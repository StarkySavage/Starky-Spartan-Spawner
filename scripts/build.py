"""Build both mod DLLs locally. Does not install or start the game."""
from pathlib import Path
import argparse, hashlib, json, shutil, subprocess

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--zig', default='zig', help='Zig 0.16.0 executable')
args = parser.parse_args()
version = subprocess.check_output([args.zig, 'version'], text=True).strip()
if version != '0.16.0':
    raise SystemExit('This documented build requires Zig 0.16.0; got ' + version)
out = root / 'build'
out.mkdir(exist_ok=True)

def run(*arguments):
    subprocess.run([args.zig, 'cc', '-target', 'x86_64-windows-gnu', *arguments], cwd=root, check=True)

run('-c', 'src/halosandbox_hook.S', '-o', 'build/halosandbox_hook.obj')
run('-fms-extensions', '-O2', '-Werror', '-Wno-deprecated-declarations', '-shared',
    '-o', 'build/halosandbox_native.dll', 'src/halosandbox_native_v2.c',
    'build/halosandbox_hook.obj', '-lkernel32', '-luser32', '-lxinput1_4', '-lbcrypt')
run('-O2', '-Werror', '-shared', '-o', 'build/halosandbox_controller.dll',
    'src/halosandbox_controller.c', '-lkernel32', '-luser32')
package = out / 'package/Meteorite/Binaries/Win64/ue4ss/Mods/HaloSandbox'
shutil.copytree(root / 'mod/HaloSandbox', package, dirs_exist_ok=True)
(package / 'Native').mkdir(exist_ok=True)
hashes = {}
for name in ['halosandbox_native.dll', 'halosandbox_controller.dll']:
    shutil.copy2(out / name, package / 'Native' / name)
    hashes[name] = hashlib.sha256((out / name).read_bytes()).hexdigest()
(out / 'build-hashes.json').write_text(json.dumps(hashes, indent=2))
print('Built both DLLs and staged the mod under build/package. No game files changed.')

