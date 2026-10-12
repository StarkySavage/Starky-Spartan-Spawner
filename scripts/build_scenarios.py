"""Reproduce E10/D40 scenario edits using the owner's stock CU4 game data.
No game binaries/assets are stored in this repository. Outputs stay in build/.
"""
from pathlib import Path
import argparse, hashlib, json, struct, subprocess
from scenario_reader import Scenario, section

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--retoc', required=True, help='retoc executable from Blamforge 0.7.0')
parser.add_argument('--game-folder', type=Path, required=True)
args = parser.parse_args()
out = root / 'build/scenarios'
out.mkdir(parents=True, exist_ok=True)
specs = [
    ('Bonus', 'pakchunk510-Windows.utoc', 'ec571ee68a6a2fb600000002', 191, 200,
     '05df792ff3b473daac50530b0bd7e5daf827e0db65321892e1c4127345b22a1d'),
    ('Maw', 'pakchunk440-Windows.utoc', '5459424c9d647e6200000002', 194, 192,
     '14cdfa14a6707504e04e0b0dc0da02fd68156f34bb9ec6b08556d5094397d4de')
]
sha = lambda data: hashlib.sha256(data).hexdigest()
report = {}
for name, container_name, chunk, count, bulk_size_offset, original_hash in specs:
    raw = out / name / 'raw'
    (raw / 'chunks').mkdir(parents=True, exist_ok=True)
    (raw / 'manifest.json').write_text(json.dumps({'chunk_paths': {}, 'version': 'ReplaceIoChunkHashWithIoHash', 'mount_point': '../../../'}))
    container = args.game_folder / 'Meteorite/Content/Paks' / container_name
    export = chunk[:-1] + '1'
    for key in [chunk, export]:
        subprocess.run([args.retoc, 'get', str(container), key, str(raw / 'chunks' / key)], check=True)
    original = (raw / 'chunks' / chunk).read_bytes()
    if sha(original) != original_hash:
        raise SystemExit(name + ': unsupported/modified scenario; original hash differs')
    scenario = Scenario(original)
    squad = scenario.squads()
    packed, wrappers = scenario.squad_parts()
    assert squad['count'] == count and squad['element_size'] == 108
    empty = section('tgbl', 0, struct.pack('<II', 0, 0))
    cell = section('tgst', len(empty), empty)
    children = empty + empty + section('tgsi', 0, b'') + cell + cell
    wrapper = section('tgst', len(children), children)
    records = []
    for i in range(16 if name == 'Bonus' else 20):
        if name == 'Bonus':
            label = f'nova_sandbox_enemy_{i}' if i < 12 else f'starky_free_marine_{i-12}'
            team = 3 if i < 4 else 8 if i < 8 else 9 if i < 12 else 2
        else:
            label = f'starky_maw_squad_{i}'
            team = 2 if i < 8 else 3 if i < 12 else 8 if i < 16 else 9
        row = bytearray(108)
        row[:len(label)] = label.encode()
        struct.pack_into('<h', row, 36, team)
        for offset in [38, 40, 42, 44, 46]:
            struct.pack_into('<h', row, offset, -1)
        struct.pack_into('<i', row, 80, -1)
        records.append(bytes(row))
    total = count + len(records)
    replacement = section('tgbl', 0, struct.pack('<II', total, 0) + packed + b''.join(records) + b''.join(wrappers) + wrapper * len(records))
    a = squad['section']
    b = a + 12 + squad['length']
    delta = len(replacement) - (b-a)
    edited = bytearray(original[:a] + replacement + original[b:])
    struct.pack_into('<I', edited, squad['packed_offset'], total)
    sizes = [0x48, scenario.bdat+8, scenario.root_block+8, scenario.root_wrapper+4, scenario.root_wrapper+8]
    for offset in sizes:
        struct.pack_into('<I', edited, offset, scenario.u32(offset)+delta)
    check = Scenario(bytes(edited))
    np, nw = check.squad_parts()
    assert check.squads()['count'] == total and np[:len(packed)] == packed and nw[:count] == wrappers
    reverse = bytearray(edited[:a] + original[a:b] + edited[a+len(replacement):])
    for offset in [squad['packed_offset']] + sizes:
        struct.pack_into('<I', reverse, offset, scenario.u32(offset))
    assert bytes(reverse) == original
    (raw / 'chunks' / chunk).write_bytes(edited)
    zen = bytearray((raw / 'chunks' / export).read_bytes())
    assert struct.unpack_from('<Q', zen, bulk_size_offset)[0] == len(original)
    struct.pack_into('<Q', zen, bulk_size_offset, len(edited))
    (raw / 'chunks' / export).write_bytes(zen)
    package = out / 'package/Meteorite/Content/Paks/~mods'
    package.mkdir(parents=True, exist_ok=True)
    basename = 'StarkySpartanSpawner_' + name + '_P'
    utoc = package / (basename + '.utoc')
    subprocess.run([args.retoc, 'pack-raw', str(raw), str(utoc)], check=True)
    # Empty companion pak: mount/index/footer only, no game assets or executable content.
    (package / (basename + '.pak')).write_bytes(bytes.fromhex((root / 'scripts/empty-companion-pak.hex').read_text()))
    for key in [chunk, export]:
        roundtrip = out / name / (key + '.roundtrip')
        subprocess.run([args.retoc, 'get', str(utoc), key, str(roundtrip)], check=True)
        assert roundtrip.read_bytes() == (raw / 'chunks' / key).read_bytes()
    report[name] = {'original_sha256': sha(original), 'edited_sha256': sha(edited), 'original_squads': count, 'new_squads': total, 'exact_reversal': True, 'packed_roundtrip': True}
(out / 'verification.json').write_text(json.dumps(report, indent=2))
print('Both scenarios rebuilt, reversed and roundtrip-verified. Game installation unchanged.')

