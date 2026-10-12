/* Halo Sandbox — Copyright (c) 2026 Ghost7134 — licence MIT. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <math.h>
#include <xinput.h>
#include <intrin.h>

#if defined(_MSC_VER) && !defined(_Alignas)
#define _Alignas(x) __declspec(align(x))
#endif

#define MINIMUM_IMAGE_SIZE 0x00664000u

typedef void (__fastcall *placement_init_fn)(void *, int32_t, int32_t, const void *);
typedef int32_t (__fastcall *object_new_fn)(void *);
typedef void (__fastcall *object_delete_fn)(int32_t);
typedef void (__fastcall *object_get_origin_fn)(int32_t, float *);
typedef void (__fastcall *object_get_orientation_fn)(int32_t, float *, float *);
typedef float (__fastcall *unit_get_vitality_fn)(int32_t);
typedef void (__fastcall *unit_set_body_vitality_fn)(int32_t, float, uint8_t);
typedef void (__fastcall *unit_set_shield_vitality_fn)(int32_t, float);
typedef void (__fastcall *unit_set_enterable_fn)(int32_t, int32_t);
typedef uint8_t (__fastcall *unit_submit_action_fn)(int32_t, void *);
typedef void (__fastcall *unit_enter_vehicle_immediate_fn)(int32_t, int32_t, uint32_t, uint8_t);
typedef void (__fastcall *object_set_velocity_fn)(int32_t, float, float, float);
typedef void (__fastcall *object_teleport_fn)(int32_t, const float *, const float *, const float *, uint8_t);
typedef void (__fastcall *object_hide_fn)(int32_t, uint8_t);
typedef void (__fastcall *damage_owner_from_object_fn)(int32_t, void *);
typedef void (__fastcall *weapon_update_fn)(int32_t);
typedef void (__fastcall *weapon_fire_fn)(int32_t, int16_t, uint8_t);
typedef void (__fastcall *weapon_fire_pending_fn)(
    int32_t, const void *, uint8_t, int32_t);
typedef float (__fastcall *weapon_interval_fn)(int32_t, int16_t, uint8_t);
typedef void (__fastcall *weapon_barrel_reset_fn)(int32_t, int16_t);
typedef void (__fastcall *weapon_set_age_fn)(int32_t, float);
typedef void (__fastcall *weapon_set_ammo_fn)(int32_t, int32_t, uint16_t);
typedef void (__fastcall *actor_set_team_fn)(int32_t, int32_t);
typedef void (__fastcall *actor_clear_secondary_link_fn)(int32_t);
typedef void *(__fastcall *tag_get_fn)(uint32_t, int32_t);
typedef int32_t (__fastcall *tag_load_fn)(uint32_t, const char *);
typedef int (__fastcall *find_squad_group_fn)(void *, const char *);
typedef int (__fastcall *find_squad_fn)(void *, const char *);
typedef void (__fastcall *ai_place_fn)(uint32_t, int, uint8_t);
typedef void (__fastcall *ai_place_squad_fn)(int32_t, void *);
typedef int32_t (__fastcall *actor_attach_to_unit_fn)(int32_t, int32_t, int32_t, uint16_t, uint16_t, const void *);
typedef void (__fastcall *actor_create_equipment_fn)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
typedef void (__fastcall *actor_migrate_to_squad_fn)(int32_t, int32_t);
typedef void (__fastcall *runtime_flags_set_fn)(const uint64_t *);
typedef void (__fastcall *ai_iterator_init_fn)(uint32_t, void *);
typedef void *(__fastcall *ai_iterator_next_fn)(void *);
typedef int32_t (__fastcall *data_new_fn)(void *);
typedef void (__fastcall *fireteam_add_actor_fn)(int32_t, int32_t);
typedef void (__fastcall *fireteam_remove_actor_at_fn)(int32_t, int32_t);
typedef uint8_t (__fastcall *fireteam_remove_squad_fn)(int32_t);
typedef uint8_t (__fastcall *squad_activate_fn)(int32_t);
typedef void (__fastcall *squad_runtime_initialize_fn)(void *);
typedef HRESULT (WINAPI *get_thread_description_fn)(HANDLE, PWSTR *);

enum command_kind {
    COMMAND_NONE,
    COMMAND_HEALTH,
    COMMAND_SPAWN,
    COMMAND_SPAWN_AT,
    COMMAND_AI_CONTROL,
    COMMAND_OBSERVER,
    COMMAND_ESCORT_STATUS,
    COMMAND_SPAWN_ENTER,
    COMMAND_VELOCITY,
    COMMAND_ENTERABLE,
    COMMAND_ENTER,
    COMMAND_SEATINFO,
    COMMAND_UNLOCKSEATS,
    COMMAND_LISTTAGS,
    COMMAND_SEATDUMP,
    COMMAND_MEMDUMP,
    COMMAND_TAGENTRY,
    COMMAND_FINDTAG,
    COMMAND_LOADTAG
    ,COMMAND_ORIGIN
    ,COMMAND_TELEPORT
    ,COMMAND_SEATFLAGS
    ,COMMAND_ENTER_IMMEDIATE
    ,COMMAND_PARENT
    ,COMMAND_EXIT_FORCE
    ,COMMAND_AI_PROBE
    ,COMMAND_AI_PLACE_GROUP
    ,COMMAND_AI_ATTACH
    ,COMMAND_AI_SPAWN_AT
    ,COMMAND_AI_FOLLOW_WATCH
    ,COMMAND_SPAWN_ENTER_RELATIVE
    ,COMMAND_SPAWN_RELATIVE
    ,COMMAND_DAMAGEPROOF
    ,COMMAND_REFILL_AMMO
    ,COMMAND_PLAYER_POWERS
    ,COMMAND_AMMO_MAINTENANCE
    ,COMMAND_PHASE_FORWARD
    ,COMMAND_NOCLIP
    ,COMMAND_NOCLIP_TICK
    ,COMMAND_NOCLIP_RECOVER
    ,COMMAND_TAGDUMP
    ,COMMAND_TAGBLOCK
    ,COMMAND_MODELMARKERS
    ,COMMAND_ATTACHSPAWN
    ,COMMAND_WEAPON_FIRE
    ,COMMAND_OBJECT_DELETE
    ,COMMAND_PELICAN_ARM
    ,COMMAND_PELICAN_DISARM
    ,COMMAND_PELICAN_INPUT_TICK
    ,COMMAND_PELICAN_STATUS
    ,COMMAND_PELICAN_PATCH
    ,COMMAND_PELICAN_BOOST
    ,COMMAND_PELICAN_ARM_BUILTIN
#ifdef HALOSANDBOX_RUNTIME_TEST
    ,COMMAND_PELICAN_RUNTIME_TEST_ARM
#endif
};

struct native_command {
    enum command_kind kind;
    LONG dispatch_epoch;
    int32_t a;
    int32_t b;
    int32_t c;
    int32_t d;
    int32_t e;
    int32_t marine_mode; /* 0: follow; 1: free combat; 2: initial search */
    uintptr_t address;
    float x;
    float y;
    float z;
    float fx;
    float fy;
    float fz;
    float ux;
    float uy;
    float uz;
    char path[256];
};

HMODULE g_self;
void *g_resume_rip;
static uint8_t *g_blam;
static volatile LONG g_state;
enum native_dispatch_state {
    NATIVE_STATE_IDLE = 0,
    NATIVE_STATE_QUEUED = 1,
    NATIVE_STATE_RESULT_READY = 2,
    NATIVE_STATE_EXECUTING = 3,
    NATIVE_STATE_RESERVED = 4,
    NATIVE_STATE_EXECUTING_QUIESCED = 5,
    NATIVE_STATE_RESULT_WRITING = 6,
    NATIVE_STATE_RESULT_WRITING_QUIESCED = 7,
    NATIVE_STATE_QUIESCE_RESETTING = 8,
    NATIVE_STATE_RESERVED_QUIESCED = 9,
    NATIVE_STATE_QUEUED_QUIESCED = 10
};
static volatile LONG g_worker_started;
static volatile LONG g_dispatch_epoch;
static volatile LONG g_pelican_worker_started;
static volatile LONG g_ammo_worker_started;
static volatile LONG g_ammo_maintenance_enabled;
static volatile LONG g_ammo_maintenance_unit = -1;
static volatile LONG g_ammo_maintenance_epoch;
static volatile LONG g_process_detaching;
static volatile LONG g_internal_dispatch;
static volatile LONG g_external_pending;
static volatile LONG g_self_pinned;
__declspec(align(8)) static volatile LONG64 g_menu_state_checked_at;
static volatile LONG g_menu_state_cached;
static volatile LONG g_menu_state_direct;
static struct native_command g_command;
static char g_result[65536];
static DWORD g_last_main_thread_id;
static DWORD g_last_main_thread_candidates;

static void finish_quiesced_owner(LONG expected_state);
static void abandon_reserved_state(void);
static void abandon_queued_state(void);

#define NOCLIP_MAX_ZONE_WORDS 1024u

struct zone_limit_snapshot {
    void *scenario;
    uint32_t *soft_words;
    uint32_t *kill_words;
    uint32_t soft_count;
    uint32_t trigger_count;
    uint32_t trigger_token;
    uint32_t soft_word_count;
    uint32_t kill_word_count;
    uint32_t soft_original[NOCLIP_MAX_ZONE_WORDS];
    uint32_t kill_original[NOCLIP_MAX_ZONE_WORDS];
    uint32_t soft_disable_mask[NOCLIP_MAX_ZONE_WORDS];
    uint32_t kill_disable_mask[NOCLIP_MAX_ZONE_WORDS];
    int valid;
};

static volatile LONG g_noclip_enabled;
static int32_t g_noclip_datum = -1;
static int32_t g_noclip_protected_vehicle = -1;
static volatile LONG g_noclip_anchor_valid;
static void *g_noclip_anchor_scenario;
static float g_noclip_anchor_position[3];
static float g_noclip_anchor_forward[3];
static float g_noclip_anchor_up[3];
static struct zone_limit_snapshot g_zone_limits;

struct pelican_direct_gate {
    int32_t weapon;
    int32_t last_attempt_tick;
    int32_t last_success_tick;
    int32_t recovery_until_tick;
    uint16_t shots_in_hold;
    uint8_t was_down;
    uint8_t had_success;
    uint8_t recovery_active;
    uint8_t has_attempt_in_hold;
};

#define PELICAN_LOCAL_INPUT_HOLD_TICKS 2
#define PELICAN_PILOT_STALE_TICKS 2

/* Relais de tir du pilote Pelican. Le module-arme reste un vrai objet Blam
 * visible et attache au vehicule : il porte le canon ventral original, son
 * projectile, ses effets et son son retail. */
static volatile LONG g_pelican_armed;
static volatile LONG g_pelican_requested_primary;
static volatile LONG g_pelican_applied_primary;
static volatile LONG g_pelican_driver_present;
static volatile LONG g_pelican_local_input_hold_ticks = 0;
static volatile LONG g_pelican_pilot_absence_ticks = 0;
static volatile LONG g_pelican_last_driver_datum = -1;
/* Conserve la toute derniere cause de desarmement automatique meme apres la
 * remise a -1 des datums. Sans ce snapshot, pelican_status ne pouvait montrer
 * que armed=0 et masquait la garde exacte ayant refuse le module. */
#define PELICAN_RELEASE_VEHICLE_MISSING       (1u << 0)
#define PELICAN_RELEASE_WEAPON_MISSING        (1u << 1)
#define PELICAN_RELEASE_MODULE_MISSING        (1u << 2)
#define PELICAN_RELEASE_MODULE_PARENT         (1u << 3)
#define PELICAN_RELEASE_INVENTORY              (1u << 4)
#define PELICAN_RELEASE_BARREL                  (1u << 5)
#define PELICAN_RELEASE_EXPLICIT                (1u << 6)
#define PELICAN_RELEASE_PILOT_MISSING           (1u << 7)
#define PELICAN_BARREL_DETAIL_OBJECT            (1u << 0)
#define PELICAN_BARREL_DETAIL_RANGE             (1u << 1)
#define PELICAN_BARREL_DETAIL_PARENT            (1u << 2)
#define PELICAN_BARREL_DETAIL_TAG               (1u << 3)
#define PELICAN_BARREL_DETAIL_BLOCK             (1u << 4)
#define PELICAN_BARREL_DETAIL_MINMAX            (1u << 5)
#define PELICAN_BARREL_DETAIL_RATE_SCALE        (1u << 6)
static volatile LONG g_pelican_last_release_reason;
static volatile LONG g_pelican_release_count;
static int32_t g_pelican_last_release_vehicle = -1;
static int32_t g_pelican_last_release_module = -1;
static int32_t g_pelican_last_release_weapon = -1;
static volatile LONG g_pelican_module_inventory_missing_observed;
static volatile LONG g_pelican_rate_scale_nonfinite_observed;
static volatile LONG g_pelican_last_barrel_failure_detail;
static int32_t g_pelican_last_barrel_actual_parent = -1;
static int32_t g_pelican_last_barrel_expected_parent = -1;
static int32_t g_pelican_last_barrel_weapon_tag = -1;
static uint32_t g_pelican_last_barrel_count;
static uint32_t g_pelican_last_barrel_min_bits;
static uint32_t g_pelican_last_barrel_max_bits;
static uint32_t g_pelican_last_barrel_rate_bits;
static int32_t g_pelican_local_pilot = -1;
static int32_t g_pelican_vehicle = -1;
static int32_t g_pelican_chin_module = -1;
static int32_t g_pelican_chin_weapon = -1;
static int32_t g_pelican_audio_module = -1;
static int32_t g_pelican_audio_weapon = -1;
static uint32_t g_pelican_audio_fire_events;
static uint32_t g_pelican_audio_fire_failures;
static volatile LONG g_pelican_audio_signal_state = -1;
static volatile LONG g_pelican_audio_signal_sequence;
/* Le proxy Unreal et la simulation Blam n'utilisent pas le meme repere.
 * Lua publie donc le point vise dans le repere LOCAL du vrai Pelican UE;
 * le game thread le reconstruit dans le repere Blam courant juste avant le
 * tir. Aucun fichier ni aucune file de commandes n'est touche par ce pont. */
struct pelican_camera_target_bridge {
    volatile LONG sequence;
    volatile LONG valid;
    int32_t vehicle;
    float local_target[3];
    ULONGLONG updated_at_ms;
    uint32_t accepted_samples;
    uint32_t rejected_samples;
};
static struct pelican_camera_target_bridge g_pelican_camera_target;
static uint32_t g_pelican_retail_context_shots;
static uint32_t g_pelican_retail_context_failures;
static float g_pelican_last_target_point[3];
static volatile LONG g_pelican_last_target_from_camera;
static volatile LONG g_pelican_fire_interval_ms = 100;
static volatile LONG g_pelican_damage_owner_kind = -1;
static volatile LONG g_pelican_damage_owner_cache_active;
static volatile LONG g_pelican_damage_owner_verified;
static int32_t g_pelican_damage_owner_pilot = -1;
static int32_t g_pelican_damage_owner_module = -1;
static uint8_t g_pelican_damage_owner_bytes[12];
static struct pelican_direct_gate g_pelican_direct_gate = {
    -1, 0, 0, 0, 0, 0, 0, 0, 0
};
static uint32_t g_pelican_completed_shots;
static uint8_t g_pelican_last_shot_serial;
static volatile LONG g_pelican_last_target_ok;
static volatile LONG g_pelican_last_aim_ok;
/* Preuve du dernier projectile cree depuis le rayon du reticule. */
struct pelican_projectile_probe {
    uint32_t attempts;
    uint32_t created;
    uint32_t fx_suppressed;
    uint32_t fx_suppression_failures;
    int32_t last_datum;
    int32_t last_tag;
    int32_t last_impact_damage;
    int32_t last_weapon;
    int32_t last_parent;
    int32_t last_source_root;
    uint16_t last_barrel;
    uint8_t owner_match;
    uint8_t failure_stage;
    uint8_t placement_variant;
    uint8_t reserved;
    float authored_speed;
    float final_speed;
    float aim[3];
    float position[3];
    float forward[3];
    float velocity[3];
};
static struct pelican_projectile_probe g_pelican_projectile_probe;
/* Les references d'effet injectees directement dans le tag projectile ne
 * participent pas au graphe de dependances de tag_release. La 1.5.58 pouvait
 * donc laisser le simulateur consommer une reference liberee apres une longue
 * rafale. Les impacts utilisent desormais le vrai projectile authored du
 * Warthog deja resident via le proxy physique, sans aucune mutation de tag. */
static int32_t g_pelican_aux_impact_projectile_tag = -1;
static uint32_t g_pelican_aux_impact_created;
static uint32_t g_pelican_aux_impact_failures;
/* Le tag d'origine du canon ventral utilise le projectile et les degats du
 * Pelican, mais son firing-effect Falcon ne fournit pas la presentation retail
 * attendue dans Campaign Evolved. Tous les clients chargent donc le meme
 * firing-effect humain du Warthog, puis seul le datum de presentation du barrel
 * est remplace. Le projectile, le marker, la cadence et les degats restent ceux
 * du vrai canon ventral. Cette modification est locale aux tags de la mission;
 * elle ne donne jamais le droit de tirer a un passager. */
static int32_t g_pelican_presentation_weapon_tag = -1;
static int32_t g_pelican_presentation_original_effect = -1;
static int32_t g_pelican_presentation_effect = -1;
static uint32_t g_pelican_presentation_patch_count;
static int g_pelican_presentation_patch_active;
static int g_pelican_presentation_patch_error;
/* Le projectile du canon ventral conserve ses degats de detonation retail,
 * mais ses deux references d'impact visuel sont vides. Le moteur ne fait
 * aucun fallback entre l'impact decor (+0x254) et l'impact objet (+0x264) :
 * chacun doit donc recevoir explicitement un effet, sans jamais toucher aux
 * champs de degats +0x1C8/+0x274. */
static int32_t g_pelican_impact_projectile_tag = -1;
static int32_t g_pelican_impact_original_effects[2] = { -1, -1 };
static int32_t g_pelican_impact_effect = -1;
static uint8_t g_pelican_impact_original_valid_mask;
static float g_pelican_impact_original_material_radius;
static float g_pelican_impact_material_radius;
static int g_pelican_impact_original_material_radius_valid;
static uint32_t g_pelican_impact_patch_count;
static int g_pelican_impact_patch_active;
static int g_pelican_impact_patch_error;
/* Le moteur lit le uint16 "rounds between tracers" du premier barrel a
 * barrel+0x3E. La valeur zero force le traceur retail sur chaque projectile,
 * sans modifier le projectile, le secondary, la crate ni les effets de tir.
 * Le patch appartient strictement au weapon tag de l'arme physique armee. */
static int32_t g_pelican_tracer_weapon_tag = -1;
static uint16_t g_pelican_tracer_original_rounds;
static uint32_t g_pelican_tracer_patch_count;
static int g_pelican_tracer_original_valid;
static int g_pelican_tracer_patch_active;
static int g_pelican_tracer_patch_error;
/* Le Pelican garde integralement sa physique et sa conduite originales. Seule
 * la plage de pitch de la camera du siege pilote est elargie : le controleur
 * human-plane peut alors suivre le regard jusqu'a presque la verticale sans
 * recevoir le moindre bloc du Spirit. Le dump exact HCE du siege pilote donne
 * -45/+5 degres a +0x98/+0x9C; +0x78/+0x7C appartient au bloc de mouvement et
 * son utilisation dans la 1.5.55 faisait refuser pelican_patch avant le spawn. */
#define PELICAN_PILOT_SEAT_STRIDE 328u
#define PELICAN_PILOT_PITCH_MIN_OFFSET 0x98u
#define PELICAN_PILOT_PITCH_MAX_OFFSET 0x9Cu
#define PELICAN_PILOT_PITCH_LIMIT_RAD 1.55334306f /* 89 deg */
static int32_t g_pelican_pitch_patch_tag = -1;
static float g_pelican_pitch_original_min;
static float g_pelican_pitch_original_max;
static uint32_t g_pelican_pitch_patch_count;
static int g_pelican_pitch_patch_active;
static int g_pelican_pitch_patch_error;
/* Releve runtime HCE 1.111.2544.0, confirme sur les deux tags residents :
 * - Banshee: flags +0x14C = 0x08002000, courbe +0x470 = 2.5/1/.3/.1/.5/.5
 * - Pelican: flags +0x14C = 0x00002001, courbe +0x470 = 3/0/0/0/0/0
 * Le Pelican garde integralement son bloc et son controleur human-plane.
 * Seul "unit has boost" est active pour laisser LT/Maj suivre l'entree native. */
#define PELICAN_UNIT_DEFINITION_FLAGS_OFFSET 0x14Cu
#define PELICAN_RETAIL_UNIT_FLAGS 0x00002001u
#define UNIT_HAS_BOOST_FLAG 0x08000000u
#define UNIT_BOOST_PEAK_POWER_OFFSET 0x470u
#define UNIT_BOOST_CURVE_SIZE 24u
static int32_t g_pelican_boost_patch_tag = -1;
static uint32_t g_pelican_boost_original_flags;
static uint32_t g_pelican_boost_last_before;
static uint32_t g_pelican_boost_last_after;
static uint32_t g_pelican_boost_patch_count;
static int g_pelican_boost_patch_active;
static int g_pelican_boost_patch_error;
#define PELICAN_MAX_FIRING_EFFECT_RECORDS 4u
struct pelican_presentation_record_patch {
    int32_t original_effect;
    uint8_t slot_offset;
    uint8_t valid;
};
static struct pelican_presentation_record_patch
    g_pelican_presentation_records[PELICAN_MAX_FIRING_EFFECT_RECORDS];
static uint32_t g_pelican_presentation_record_count;
static uint32_t g_pelican_presentation_secondary_mask;
static const char g_pelican_chin_weapon_path[] =
    "objects\\vehicles\\human\\pelican\\attachments\\pelican_chin_gun\\weapons\\pelican_chin_gun";
static const char g_pelican_human_firing_effect_path[] =
    "objects\\vehicles\\human\\warthog\\fx\\weapon\\chaingun\\firing";
static const char g_pelican_human_impact_effect_path[] =
    "objects\\vehicles\\human\\warthog\\fx\\weapon\\chaingun\\impact";
static const char g_pelican_human_weapon_path[] =
    "objects\\vehicles\\human\\warthog\\weapons\\warthog_chaingun\\weapon\\warthog_chaingun";
#ifdef HALOSANDBOX_RUNTIME_TEST
/* Injection uniquement pour le banc de validation local. Cette branche n'est
 * jamais compilee dans les DLL distribuees. Le pipeline principal conserve
 * ensuite ses gardes : un vrai pilote assis dans le Pelican reste obligatoire. */
static volatile LONG g_pelican_runtime_test_primary;
struct pelican_runtime_shot_probe {
    uint16_t flags_pre;
    uint16_t flags_post_mask;
    uint16_t flags_post_fire;
    uint16_t shots_pre;
    uint16_t shots_post;
    int16_t record_pre;
    int16_t record_post;
    int16_t remaining_pre;
    int16_t remaining_post;
    uint8_t serial_pre;
    uint8_t serial_post;
    int32_t instance_pre;
    int32_t instance_post;
};
static struct pelican_runtime_shot_probe g_pelican_runtime_shot_probe;
__declspec(dllexport) void halosandbox_pelican_runtime_test_primary(int down) {
    InterlockedExchange(&g_pelican_runtime_test_primary, down ? 1 : 0);
}
#endif
static int32_t g_damageproof_unit = -1;
static int32_t g_damageproof_parent = -1;
static int32_t g_bottomless_player_datum = -1;
static uint8_t *g_bottomless_player_table;
static uint8_t *g_bottomless_player_entries;
static uint8_t g_bottomless_original_trait = 1;
static int g_bottomless_snapshot_valid;

extern void halosandbox_dispatch_stub(void);
#define PELICAN_DISPATCH_STUB_WINDOW_BYTES 0x900u
static int ai_runtime_context(uint8_t **thread_out, uint8_t **state_out,
                              uint64_t **flags_out, uint64_t *value_out);
static int is_live_player_unit(int32_t unit_datum);
static int pelican_publish_audio_signal(int active);

static int bytes_equal(const uint8_t *actual, const uint8_t *expected, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) if (actual[i] != expected[i]) return 0;
    return 1;
}

/* Temporary AI24 diagnostics. Observe the verified CU4 fatal allocator fault;
 * never swallow the exception, change its context, or retry engine work. */
static wchar_t ai24_capture_path[MAX_PATH];
static volatile LONG ai24_capture_busy;
static PVOID ai24_handler;
static unsigned char ai24_capture_buffer[65536];

static void ai24_write(HANDLE f, const void *data, DWORD size) {
    DWORD written;
    WriteFile(f,data,size,&written,NULL);
}
static DWORD ai24_memory(HANDLE f, uintptr_t address, DWORD requested, DWORD kind) {
    MEMORY_BASIC_INFORMATION m; SIZE_T got=0,available; uint64_t header[2];
    if(!address || !VirtualQuery((void *)address,&m,sizeof(m)) ||
       m.State!=MEM_COMMIT || (m.Protect&(PAGE_NOACCESS|PAGE_GUARD))) return 0;
    available=(uintptr_t)m.BaseAddress+m.RegionSize-address;
    if(requested>available)requested=(DWORD)available;
    if(requested>sizeof(ai24_capture_buffer))requested=sizeof(ai24_capture_buffer);
    if(!ReadProcessMemory(GetCurrentProcess(),(void *)address,ai24_capture_buffer,requested,&got) || !got)return 0;
    header[0]=address;header[1]=((uint64_t)kind<<32)|(DWORD)got;
    ai24_write(f,header,sizeof(header));ai24_write(f,ai24_capture_buffer,(DWORD)got);
    return (DWORD)got;
}
static LONG CALLBACK ai24_loader_fault(PEXCEPTION_POINTERS e) {
    HANDLE f; CONTEXT *c; uint64_t header[6],pointers[512]; SIZE_T got=0; DWORD i;
    if(!g_blam || !e || !e->ExceptionRecord || !e->ContextRecord ||
       e->ExceptionRecord->ExceptionCode!=EXCEPTION_ACCESS_VIOLATION ||
       (uintptr_t)e->ExceptionRecord->ExceptionAddress!=(uintptr_t)g_blam+0x430A2 ||
       !ai24_capture_path[0] || InterlockedCompareExchange(&ai24_capture_busy,1,0))
        return EXCEPTION_CONTINUE_SEARCH;
    f=CreateFileW(ai24_capture_path,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(f==INVALID_HANDLE_VALUE)return EXCEPTION_CONTINUE_SEARCH;
    c=e->ContextRecord;
    header[0]=0x3130545041433432ULL; /* 24CAPT01 */
    header[1]=(uintptr_t)g_blam;header[2]=(uintptr_t)g_self;
    header[3]=GetCurrentThreadId();header[4]=sizeof(CONTEXT);header[5]=1;
    ai24_write(f,header,sizeof(header));ai24_write(f,c,sizeof(*c));
    ai24_memory(f,c->Rsp,65536,1);
    { uint64_t regs[]={c->Rax,c->Rcx,c->Rdx,c->Rbx,c->Rbp,c->Rsi,c->Rdi,c->R8,c->R9,c->R10,c->R11,c->R12,c->R13,c->R14,c->R15};
      for(i=0;i<sizeof(regs)/sizeof(regs[0]);i++)ai24_memory(f,regs[i],4096,2); }
    if(ReadProcessMemory(GetCurrentProcess(),(void *)c->Rsp,pointers,sizeof(pointers),&got))
      for(i=0;i<got/sizeof(pointers[0]);i++) {
        uintptr_t a=(uintptr_t)pointers[i];
        if(a>0x10000 && !(a>=(uintptr_t)g_blam && a<(uintptr_t)g_blam+0x4000000))ai24_memory(f,a,256,3);
      }
    FlushFileBuffers(f);CloseHandle(f);
    return EXCEPTION_CONTINUE_SEARCH;
}
static void ai24_install_diagnostic(void) {
    DWORD n; wchar_t *end;
    if(ai24_handler)return;
    n=GetModuleFileNameW(g_self,ai24_capture_path,MAX_PATH);
    if(!n || n>=MAX_PATH){ai24_capture_path[0]=0;return;}
    end=wcsrchr(ai24_capture_path,L'\\');
    if(!end || (size_t)(end-ai24_capture_path)+26>=MAX_PATH){ai24_capture_path[0]=0;return;}
    wcscpy(end+1,L"AI24-loader-crash.bin");
    ai24_handler=AddVectoredExceptionHandler(1,ai24_loader_fault);
}

static int readable_range(const void *address, size_t size) {
    MEMORY_BASIC_INFORMATION info;
    uintptr_t begin = (uintptr_t)address;
    if (!address || !VirtualQuery(address, &info, sizeof(info))) return 0;
    if (info.State != MEM_COMMIT || (info.Protect & (PAGE_NOACCESS | PAGE_GUARD))) return 0;
    return begin + size >= begin && begin + size <= (uintptr_t)info.BaseAddress + info.RegionSize;
}

static int writable_range(const void *address, size_t size) {
    MEMORY_BASIC_INFORMATION info;
    uintptr_t begin = (uintptr_t)address;
    DWORD protection;
    if (!address || !VirtualQuery(address, &info, sizeof(info))) return 0;
    if (info.State != MEM_COMMIT || (info.Protect & (PAGE_NOACCESS | PAGE_GUARD))) return 0;
    protection = info.Protect & 0xFFu;
    if (protection != PAGE_READWRITE && protection != PAGE_WRITECOPY &&
        protection != PAGE_EXECUTE_READWRITE && protection != PAGE_EXECUTE_WRITECOPY) return 0;
    return begin + size >= begin && begin + size <= (uintptr_t)info.BaseAddress + info.RegionSize;
}

static uint8_t *resolve_seats(int32_t tag_index, uint32_t *count_out) {
    uint8_t *definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(0x69686576u, tag_index);
    uint32_t count;
    uint32_t token;
    uint8_t **pages;
    uint8_t *seats;
    if (!definition || !readable_range(definition + 1076, 12)) return NULL;
    count = *(uint32_t *)(definition + 1076);
    token = *(uint32_t *)(definition + 1080);
    if (!count || count > 64) return NULL;
    pages = (uint8_t **)(g_blam + 0x2C2CCC0);
    seats = pages[token >> 28] + (uint64_t)token * 4;
    if (!readable_range(seats, (size_t)count * 328u)) return NULL;
    *count_out = count;
    return seats;
}

/* Diagnostics strictement en lecture seule pour les tags vehicule de la
 * build courante. Ils permettent de verifier les armes, les attaches et le
 * type de vol du Pelican avant toute modification. Un tag_block contient un
 * nombre d'elements et un token exprime en unites de quatre octets. */
static uint8_t *resolve_vehicle_tag_block(int32_t tag_index, uint32_t offset,
                                          uint32_t stride, uint32_t *count_out) {
    uint8_t *definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
        0x69686576u, tag_index);
    uint8_t **pages = (uint8_t **)(g_blam + 0x2C2CCC0);
    uint32_t count;
    uint32_t token;
    uint32_t page;
    uint8_t *records;
    if (!definition || !stride || offset > 1604u - 12u ||
        !readable_range(definition + offset, 12)) return NULL;
    count = *(uint32_t *)(definition + offset);
    token = *(uint32_t *)(definition + offset + 4u);
    if (!count || count > 256u || count > 4096u / stride) return NULL;
    page = token >> 28;
    if (!readable_range(pages + page, sizeof(void *)) || !pages[page]) return NULL;
    records = pages[page] + (uint64_t)token * 4u;
    if (!readable_range(records, (size_t)count * stride)) return NULL;
    *count_out = count;
    return records;
}

static void pelican_forget_pitch_patch(int reset_counter) {
    g_pelican_pitch_patch_tag = -1;
    g_pelican_pitch_original_min = 0.0f;
    g_pelican_pitch_original_max = 0.0f;
    g_pelican_pitch_patch_active = 0;
    if (reset_counter) g_pelican_pitch_patch_count = 0u;
}

static int pelican_pitch_is_target(float minimum, float maximum) {
    return minimum == -PELICAN_PILOT_PITCH_LIMIT_RAD &&
        maximum == PELICAN_PILOT_PITCH_LIMIT_RAD;
}

/* Restaure les deux scalaires seulement s'ils portent encore notre valeur. */
static int pelican_restore_pitch_patch(void) {
    uint8_t *seats;
    uint32_t seat_count = 0u;
    float *minimum;
    float *maximum;
    DWORD old_protect = 0;
    DWORD ignored_protect = 0;
    int protection_restored;
    if (!g_pelican_pitch_patch_active) return 1;
    seats = resolve_seats(g_pelican_pitch_patch_tag, &seat_count);
    if (!seats || !seat_count) {
        pelican_forget_pitch_patch(0);
        return 1;
    }
    minimum = (float *)(seats + PELICAN_PILOT_PITCH_MIN_OFFSET);
    maximum = (float *)(seats + PELICAN_PILOT_PITCH_MAX_OFFSET);
    if (!readable_range(minimum, sizeof(*minimum)) ||
        !readable_range(maximum, sizeof(*maximum))) {
        g_pelican_pitch_patch_error = 10;
        return 0;
    }
    if (!pelican_pitch_is_target(*minimum, *maximum)) {
        pelican_forget_pitch_patch(0);
        return 1;
    }
    if (!VirtualProtect(seats, PELICAN_PILOT_SEAT_STRIDE,
                        PAGE_READWRITE, &old_protect)) {
        g_pelican_pitch_patch_error = 11;
        return 0;
    }
    *minimum = g_pelican_pitch_original_min;
    *maximum = g_pelican_pitch_original_max;
    MemoryBarrier();
    protection_restored = VirtualProtect(
        seats, PELICAN_PILOT_SEAT_STRIDE, old_protect,
        &ignored_protect) != 0;
    if (!protection_restored ||
        *minimum != g_pelican_pitch_original_min ||
        *maximum != g_pelican_pitch_original_max) {
        g_pelican_pitch_patch_error = 12;
        return 0;
    }
    pelican_forget_pitch_patch(0);
    return 1;
}

static int pelican_patch_pilot_pitch(int32_t pelican_tag) {
    uint8_t *seats;
    uint32_t seat_count = 0u;
    float *minimum;
    float *maximum;
    float original_minimum;
    float original_maximum;
    DWORD old_protect = 0;
    DWORD ignored_protect = 0;
    int protection_restored;
    g_pelican_pitch_patch_error = 0;
    if (g_pelican_pitch_patch_active &&
        g_pelican_pitch_patch_tag != pelican_tag &&
        !pelican_restore_pitch_patch()) {
        g_pelican_pitch_patch_error = 1;
        return 0;
    }
    seats = resolve_seats(pelican_tag, &seat_count);
    if (!seats || !seat_count) {
        g_pelican_pitch_patch_error = 2;
        return 0;
    }
    minimum = (float *)(seats + PELICAN_PILOT_PITCH_MIN_OFFSET);
    maximum = (float *)(seats + PELICAN_PILOT_PITCH_MAX_OFFSET);
    if (!readable_range(minimum, sizeof(*minimum)) ||
        !readable_range(maximum, sizeof(*maximum)) ||
        !isfinite(*minimum) || !isfinite(*maximum) ||
        *minimum < -3.2f || *minimum > 3.2f ||
        *maximum < -3.2f || *maximum > 3.2f) {
        g_pelican_pitch_patch_error = 3;
        return 0;
    }
    if (g_pelican_pitch_patch_active &&
        g_pelican_pitch_patch_tag == pelican_tag) {
        if (pelican_pitch_is_target(*minimum, *maximum)) return 1;
        g_pelican_pitch_patch_error = 4;
        return 0;
    }
    original_minimum = *minimum;
    original_maximum = *maximum;
    if (!VirtualProtect(seats, PELICAN_PILOT_SEAT_STRIDE,
                        PAGE_READWRITE, &old_protect)) {
        g_pelican_pitch_patch_error = 5;
        return 0;
    }
    *minimum = -PELICAN_PILOT_PITCH_LIMIT_RAD;
    *maximum = PELICAN_PILOT_PITCH_LIMIT_RAD;
    MemoryBarrier();
    protection_restored = VirtualProtect(
        seats, PELICAN_PILOT_SEAT_STRIDE, old_protect,
        &ignored_protect) != 0;
    if (!protection_restored ||
        !pelican_pitch_is_target(*minimum, *maximum)) {
        g_pelican_pitch_patch_error = 6;
        return 0;
    }
    g_pelican_pitch_patch_tag = pelican_tag;
    g_pelican_pitch_original_min = original_minimum;
    g_pelican_pitch_original_max = original_maximum;
    g_pelican_pitch_patch_active = 1;
    ++g_pelican_pitch_patch_count;
    return 1;
}

static void pelican_forget_boost_patch(int reset_counter) {
    g_pelican_boost_patch_tag = -1;
    g_pelican_boost_original_flags = 0u;
    g_pelican_boost_patch_active = 0;
    if (reset_counter) {
        g_pelican_boost_last_before = 0u;
        g_pelican_boost_last_after = 0u;
        g_pelican_boost_patch_count = 0u;
    }
}

/* Restaure uniquement le bit que nous possedons et preserve toute autre
 * modification eventuelle du moteur ou d'un autre mod. */
static int pelican_restore_boost_patch(void) {
    uint8_t *definition;
    uint32_t *flags;
    uint32_t restored;
    DWORD old_protect = 0;
    DWORD ignored_protect = 0;
    int protection_restored;
    if (!g_pelican_boost_patch_active) return 1;
    definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
        0x69686576u, g_pelican_boost_patch_tag);
    if (!definition || !readable_range(definition, 1604u)) {
        pelican_forget_boost_patch(0);
        return 1;
    }
    flags = (uint32_t *)(definition + PELICAN_UNIT_DEFINITION_FLAGS_OFFSET);
    if (!(*flags & UNIT_HAS_BOOST_FLAG)) {
        pelican_forget_boost_patch(0);
        return 1;
    }
    restored = *flags & ~UNIT_HAS_BOOST_FLAG;
    if (!VirtualProtect(definition, 1604u, PAGE_READWRITE, &old_protect)) {
        g_pelican_boost_patch_error = 10;
        return 0;
    }
    *flags = restored;
    MemoryBarrier();
    protection_restored = VirtualProtect(
        definition, 1604u, old_protect, &ignored_protect) != 0;
    if (!protection_restored || (*flags & UNIT_HAS_BOOST_FLAG)) {
        g_pelican_boost_patch_error = 11;
        return 0;
    }
    pelican_forget_boost_patch(0);
    return 1;
}

static int pelican_patch_native_boost(int32_t pelican_tag) {
    uint8_t *definition;
    uint32_t *flags;
    const float *curve;
    uint32_t before;
    uint32_t after;
    DWORD old_protect = 0;
    DWORD ignored_protect = 0;
    int protection_restored;
    unsigned int i;
    g_pelican_boost_patch_error = 0;
    if (g_pelican_boost_patch_active &&
        g_pelican_boost_patch_tag != pelican_tag &&
        !pelican_restore_boost_patch()) {
        g_pelican_boost_patch_error = 1;
        return 0;
    }
    definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
        0x69686576u, pelican_tag);
    if (!definition || !readable_range(definition, 1604u)) {
        g_pelican_boost_patch_error = 2;
        return 0;
    }
    flags = (uint32_t *)(definition + PELICAN_UNIT_DEFINITION_FLAGS_OFFSET);
    curve = (const float *)(definition + UNIT_BOOST_PEAK_POWER_OFFSET);
    /* Empreinte exacte du bloc original du Pelican HCE 1.111.2544.0 :
     * peak=3 et tous les temporisateurs a zero. Refuser une autre empreinte
     * evite de transformer par erreur un autre vehicule ou une autre build. */
    if (!readable_range(curve, UNIT_BOOST_CURVE_SIZE) || curve[0] != 3.0f) {
        g_pelican_boost_patch_error = 3;
        return 0;
    }
    for (i = 1u; i < UNIT_BOOST_CURVE_SIZE / sizeof(float); ++i) {
        if (curve[i] != 0.0f) {
            g_pelican_boost_patch_error = 4;
            return 0;
        }
    }
    before = *flags;
    if (before != PELICAN_RETAIL_UNIT_FLAGS &&
        before != (PELICAN_RETAIL_UNIT_FLAGS | UNIT_HAS_BOOST_FLAG)) {
        g_pelican_boost_patch_error = 7;
        return 0;
    }
    if (before & UNIT_HAS_BOOST_FLAG) {
        g_pelican_boost_last_before = before;
        g_pelican_boost_last_after = before;
        return 1;
    }
    if (g_pelican_boost_patch_active &&
        g_pelican_boost_patch_tag == pelican_tag) {
        pelican_forget_boost_patch(0);
    }
    after = before | UNIT_HAS_BOOST_FLAG;
    if (!VirtualProtect(definition, 1604u, PAGE_READWRITE, &old_protect)) {
        g_pelican_boost_patch_error = 5;
        return 0;
    }
    *flags = after;
    MemoryBarrier();
    protection_restored = VirtualProtect(
        definition, 1604u, old_protect, &ignored_protect) != 0;
    if (!protection_restored || *flags != after) {
        g_pelican_boost_patch_error = 6;
        return 0;
    }
    g_pelican_boost_patch_tag = pelican_tag;
    g_pelican_boost_original_flags = before;
    g_pelican_boost_last_before = before;
    g_pelican_boost_last_after = after;
    g_pelican_boost_patch_active = 1;
    ++g_pelican_boost_patch_count;
    return 1;
}

/* Resolve les marqueurs du squelette utilise par le modele d'un vehicule.
 * Les references de tags gardent leur FourCC logique dans les donnees, alors
 * que tag_get attend le FourCC en ordre memoire (hlmt -> tmlh, skel -> leks). */
static uint8_t *resolve_vehicle_model_markers(int32_t vehicle_tag,
                                              int32_t *model_tag_out,
                                              int32_t *skeleton_tag_out,
                                              uint32_t *count_out) {
    tag_get_fn tag_get = (tag_get_fn)(g_blam + 0x201480);
    uint8_t **pages = (uint8_t **)(g_blam + 0x2C2CCC0);
    uint8_t *vehicle = (uint8_t *)tag_get(0x69686576u, vehicle_tag);
    uint8_t *model;
    uint8_t *skeleton;
    int32_t model_tag;
    int32_t skeleton_tag;
    uint32_t count;
    uint32_t token;
    uint32_t page;
    uint8_t *groups;
    if (!vehicle || !readable_range(vehicle + 84, 16)) return NULL;
    model_tag = *(int32_t *)(vehicle + 96);
    if (model_tag == -1) return NULL;
    model = (uint8_t *)tag_get(0x746D6C68u, model_tag);
    if (!model || !readable_range(model + 48, 16)) return NULL;
    skeleton_tag = *(int32_t *)(model + 60);
    if (skeleton_tag == -1) return NULL;
    skeleton = (uint8_t *)tag_get(0x6C656B73u, skeleton_tag);
    if (!skeleton || !readable_range(skeleton + 24, 12)) return NULL;
    count = *(uint32_t *)(skeleton + 24);
    token = *(uint32_t *)(skeleton + 28);
    if (!count || count > 4096u) return NULL;
    page = token >> 28;
    if (!readable_range(pages + page, sizeof(void *)) || !pages[page]) return NULL;
    groups = pages[page] + (uint64_t)token * 4u;
    if (!readable_range(groups, (size_t)count * 16u)) return NULL;
    if (model_tag_out) *model_tag_out = model_tag;
    if (skeleton_tag_out) *skeleton_tag_out = skeleton_tag;
    if (count_out) *count_out = count;
    return groups;
}

static uint8_t *resolve_scenario_squad(void *scenario, int32_t squad_index) {
    uint32_t squad_count;
    uint32_t token;
    uint32_t record_token;
    uint8_t **pages;
    uint8_t *record;
    if (!scenario || squad_index < 0 || !readable_range((uint8_t *)scenario + 0x348, 8)) return NULL;
    squad_count = *(uint32_t *)((uint8_t *)scenario + 0x348);
    token = *(uint32_t *)((uint8_t *)scenario + 0x34C);
    if ((uint32_t)squad_index >= squad_count) return NULL;
    record_token = token + (uint32_t)squad_index * 0x1Bu;
    pages = (uint8_t **)(g_blam + 0x2C2CCC0);
    if (!readable_range(pages + (record_token >> 28), sizeof(void *)) ||
        !pages[record_token >> 28]) return NULL;
    record = pages[record_token >> 28] + (uint64_t)record_token * 4u;
    return readable_range(record, 0x6C) ? record : NULL;
}

static uint8_t *tag_table_entries(uint32_t *count_out) {
    uint8_t *table = *(uint8_t **)(g_blam + 0x182D1E8);
    uint8_t *entries;
    uint32_t count;
    if (!table || !readable_range(table + 0x44, 0x14)) return NULL;
    count = *(uint32_t *)(table + 0x44);
    entries = *(uint8_t **)(table + 0x50);
    if (!count || count > 65536 || !readable_range(entries, (size_t)count * 48u)) return NULL;
    *count_out = count;
    return entries;
}

/* Halo's per-thread engine state is stored in the loader/static TLS vector
 * addressed by GS:[0x58][game_tls_index].  The game itself uses that exact
 * sequence in the AI/object routines.  TlsGetValue(game_tls_index) addresses
 * Win32 dynamic TLS slots instead and becomes observably wrong once the
 * current build assigns an index above the inline slot range (82 here). */
static uint8_t *blam_thread_state(void) {
    DWORD tls_index;
    uint8_t **tls_slots;
    if (!g_blam) return NULL;
    tls_index = *(DWORD *)(g_blam + 0xD72730);
    tls_slots = (uint8_t **)__readgsqword(0x58);
    if (!tls_slots || !readable_range(tls_slots + tls_index, sizeof(void *))) return NULL;
    return tls_slots[tls_index];
}

static uint8_t *object_data(int32_t datum_index) {
    uint8_t *thread_state;
    uint8_t *table;
    uint8_t *entries;
    uint8_t *entry;
    uint8_t *object;
    uint32_t slot;
    if (datum_index == -1) return NULL;
    thread_state = blam_thread_state();
    if (!thread_state || !readable_range(thread_state + 0x14, 0x15) || thread_state[0x14] == 0) return NULL;
    table = *(uint8_t **)(thread_state + 0x20);
    if (!table || !readable_range(table + 0x50, 8)) return NULL;
    entries = *(uint8_t **)(table + 0x50);
    slot = (uint16_t)datum_index;
    entry = entries ? entries + (size_t)slot * 24u : NULL;
    if (!entry || !readable_range(entry, 24u) ||
        *(uint16_t *)entry != (uint16_t)((uint32_t)datum_index >> 16))
        return NULL;
    object = *(uint8_t **)(entry + 0x10);
    /* Do not require the remainder of the object to live in one VirtualQuery
       region here. Freshly allocated objects may straddle two committed heap
       regions even though every field that we use is valid. Each caller must
       validate the exact field range it reads. */
    return object && readable_range(object, sizeof(void *)) ? object : NULL;
}

/* Resolve un tag_block a partir d'une definition deja validee. Cette version
 * generique sert uniquement a retrouver l'effet de bouche original de l'arme
 * physique du Pelican; aucun tag ni projectile n'est cree ici. */
static uint8_t *resolve_tag_block_raw(uint8_t *owner, uint32_t offset,
                                      uint32_t stride, uint32_t max_count,
                                      uint32_t *count_out) {
    uint32_t count;
    uint32_t token;
    uint32_t page;
    uint8_t **pages = (uint8_t **)(g_blam + 0x2C2CCC0);
    uint8_t *records;
    if (!owner || !stride || !readable_range(owner + offset, 12u)) return NULL;
    count = *(uint32_t *)(owner + offset);
    token = *(uint32_t *)(owner + offset + 4u);
    if (!count || count > max_count || (size_t)count > SIZE_MAX / stride)
        return NULL;
    page = token >> 28;
    if (!readable_range(pages + page, sizeof(*pages)) || !pages[page])
        return NULL;
    records = pages[page] + (uint64_t)token * 4u;
    if (!readable_range(records, (size_t)count * stride)) return NULL;
    if (count_out) *count_out = count;
    return records;
}

#define TAG_GROUP_WEAPON 0x77656170u
#define TAG_GROUP_EFFECT 0x65666665u
#define TAG_GROUP_PROJECTILE 0x70726F6Au

/* Layout retail 1.111.2544.0 : barrels +0x3E4, stride 0x188;
 * firing-effects +0x17C, stride 0xC4. Aucun pointeur n'est conserve entre
 * deux appels ni au changement de mission. */
static uint8_t *pelican_primary_barrel_for_weapon_tag(int32_t weapon_tag) {
    uint8_t *definition;
    uint8_t *barrels;
    uint32_t barrel_count = 0;
    if (weapon_tag == -1) return NULL;
    definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
        TAG_GROUP_WEAPON, weapon_tag);
    barrels = resolve_tag_block_raw(
        definition, 0x3E4u, 0x188u, 2u, &barrel_count);
    return barrels && barrel_count ? barrels : NULL;
}

static uint8_t *pelican_firing_effect_records(
        int32_t weapon_tag, uint8_t **barrel_out, uint32_t *count_out) {
    uint8_t *barrels;
    uint8_t *effects;
    uint32_t effect_count = 0;
    if (barrel_out) *barrel_out = NULL;
    if (count_out) *count_out = 0;
    barrels = pelican_primary_barrel_for_weapon_tag(weapon_tag);
    if (!barrels) return NULL;
    effects = resolve_tag_block_raw(
        barrels, 0x17Cu, 0xC4u,
        PELICAN_MAX_FIRING_EFFECT_RECORDS, &effect_count);
    if (!effects || !effect_count) return NULL;
    if (barrel_out) *barrel_out = barrels;
    if (count_out) *count_out = effect_count;
    return effects;
}

/* weapon_fire incremente le compteur weapon+0x23A avant de choisir la
 * presentation. Si le record possede un secondary firing-effect, le moteur
 * choisit alors record+0x40 meme pour le premier coup; sinon il choisit le
 * primary a record+0x10. Ne jamais rendre un secondary -1 valide, car cela
 * changerait la branche retail. */
static int pelican_effect_record_selected_slot(
        uint8_t *record, int32_t **slot_out, int32_t *effect_out,
        uint8_t *offset_out) {
    int32_t *slot;
    uint8_t offset;
    if (slot_out) *slot_out = NULL;
    if (effect_out) *effect_out = -1;
    if (offset_out) *offset_out = 0;
    if (!record || !readable_range(record, 0x44u)) return 0;
    offset = *(int32_t *)(record + 0x40u) != -1 ? 0x40u : 0x10u;
    /* Le group stocke dans cette TagDependency n'est pas consulte par le
     * selecteur retail; le moteur resout le vrai group depuis le datum global.
     * Il peut donc s'agir d'un ancien snd! meme quand nous greffons un effe.
     * Seul le datum de quatre octets est remplace. */
    slot = (int32_t *)(record + offset);
    if (slot_out) *slot_out = slot;
    if (effect_out) *effect_out = *slot;
    if (offset_out) *offset_out = offset;
    return 1;
}

/* Compatibilite diagnostic : renvoie le slot effectivement selectionne pour
 * le premier record, et non plus systematiquement son primary. */
static int pelican_primary_firing_effect_slot(
        int32_t weapon_tag, int32_t **slot_out, int32_t *effect_out) {
    uint8_t *effects;
    uint32_t effect_count = 0;
    effects = pelican_firing_effect_records(
        weapon_tag, NULL, &effect_count);
    if (!effects || !effect_count) return 0;
    return pelican_effect_record_selected_slot(
        effects, slot_out, effect_out, NULL);
}

static int pelican_write_presentation_datum(
        int32_t *slot, int32_t expected, int32_t value) {
    DWORD old_protect = 0;
    DWORD ignored_protect = 0;
    int wrote = 0;
    int protection_restored;
    if (!slot || !readable_range(slot, sizeof(*slot)) ||
        !VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &old_protect))
        return 0;
    if (*slot == expected) {
        *slot = value;
        MemoryBarrier();
        wrote = *slot == value;
    }
    protection_restored = VirtualProtect(
        slot, sizeof(*slot), old_protect, &ignored_protect) != 0;
    return wrote && protection_restored &&
        readable_range(slot, sizeof(*slot)) && *slot == value;
}

static int pelican_write_presentation_float(
        float *slot, float expected, float value) {
    DWORD old_protect = 0;
    DWORD ignored_protect = 0;
    int wrote = 0;
    int protection_restored;
    if (!slot || !readable_range(slot, sizeof(*slot)) ||
        !VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &old_protect))
        return 0;
    if (*slot == expected) {
        *slot = value;
        MemoryBarrier();
        wrote = *slot == value;
    }
    protection_restored = VirtualProtect(
        slot, sizeof(*slot), old_protect, &ignored_protect) != 0;
    return wrote && protection_restored &&
        readable_range(slot, sizeof(*slot)) && *slot == value;
}

#define PELICAN_BARREL_PROJECTILE_TAG_OFFSET 0x114u
#define PELICAN_PROJECTILE_MATERIAL_EFFECT_RADIUS_OFFSET 0x220u
#define PELICAN_PROJECTILE_IMPACT_EFFECT_OFFSET 0x254u
#define PELICAN_PROJECTILE_OBJECT_IMPACT_EFFECT_OFFSET 0x264u
#define PROJECTILE_IMPACT_DAMAGE_TAG_OFFSET 0x274u

static uint8_t *pelican_projectile_definition_for_weapon_tag(
        int32_t weapon_tag, int32_t *projectile_tag_out) {
    uint8_t *barrel;
    int32_t projectile_tag;
    if (projectile_tag_out) *projectile_tag_out = -1;
    barrel = pelican_primary_barrel_for_weapon_tag(weapon_tag);
    if (!barrel || !readable_range(
            barrel + PELICAN_BARREL_PROJECTILE_TAG_OFFSET,
            sizeof(projectile_tag))) return NULL;
    projectile_tag = *(int32_t *)(
        barrel + PELICAN_BARREL_PROJECTILE_TAG_OFFSET);
    if (projectile_tag == -1) return NULL;
    if (projectile_tag_out) *projectile_tag_out = projectile_tag;
    return (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
        TAG_GROUP_PROJECTILE, projectile_tag);
}

static void pelican_forget_impact_patch(int reset_counter) {
    g_pelican_impact_projectile_tag = -1;
    g_pelican_impact_original_effects[0] = -1;
    g_pelican_impact_original_effects[1] = -1;
    g_pelican_impact_effect = -1;
    g_pelican_impact_original_valid_mask = 0u;
    g_pelican_impact_original_material_radius = 0.0f;
    g_pelican_impact_material_radius = 0.0f;
    g_pelican_impact_original_material_radius_valid = 0;
    g_pelican_impact_patch_active = 0;
    if (reset_counter) g_pelican_impact_patch_count = 0u;
}

/* Restaure seulement les slots qui portent encore notre effet. Un autre mod
 * ou le moteur reste proprietaire de toute valeur changee entre-temps. */
static int pelican_restore_impact_patch(void) {
    static const uint32_t offsets[2] = {
        PELICAN_PROJECTILE_IMPACT_EFFECT_OFFSET,
        PELICAN_PROJECTILE_OBJECT_IMPACT_EFFECT_OFFSET
    };
    uint8_t *definition;
    uint32_t index;
    if (!g_pelican_impact_patch_active) return 1;
    definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
        TAG_GROUP_PROJECTILE, g_pelican_impact_projectile_tag);
    if (!definition) {
        pelican_forget_impact_patch(0);
        return 1;
    }
    for (index = 0; index < 2u; ++index) {
        int32_t *slot;
        if ((g_pelican_impact_original_valid_mask & (1u << index)) == 0u)
            continue;
        slot = (int32_t *)(definition + offsets[index]);
        if (!readable_range(slot, sizeof(*slot))) continue;
        if (*slot == g_pelican_impact_effect &&
            !pelican_write_presentation_datum(
                slot, g_pelican_impact_effect,
                g_pelican_impact_original_effects[index])) return 0;
    }
    if (g_pelican_impact_original_material_radius_valid) {
        float *radius = (float *)(definition +
            PELICAN_PROJECTILE_MATERIAL_EFFECT_RADIUS_OFFSET);
        if (!readable_range(radius, sizeof(*radius))) return 0;
        if (*radius == g_pelican_impact_material_radius &&
            !pelican_write_presentation_float(
                radius, g_pelican_impact_material_radius,
                g_pelican_impact_original_material_radius)) return 0;
    }
    pelican_forget_impact_patch(0);
    return 1;
}

/* Force l'effet retail Warthog dans les deux chemins distincts du moteur :
 * decor/BSP et objet. Le projectile Pelican contient deja une reference de
 * decor non vide mais sans presentation visible dans Campaign Evolved; la
 * conserver expliquait impact_patch=1 sans decal ni etincelles au sol. Les
 * degats, reponses materiau et physique restent strictement intacts. */
static int pelican_patch_impact_for_weapon_tag(int32_t weapon_tag) {
    static const uint32_t offsets[2] = {
        PELICAN_PROJECTILE_IMPACT_EFFECT_OFFSET,
        PELICAN_PROJECTILE_OBJECT_IMPACT_EFFECT_OFFSET
    };
    uint8_t *definition;
    uint8_t *source_definition;
    int32_t *slots[2] = { NULL, NULL };
    float *radius_slot;
    int32_t originals[2] = { -1, -1 };
    float original_radius;
    float replacement_radius;
    uint8_t changed_mask = 0u;
    int radius_changed = 0;
    uint32_t index;
    int32_t projectile_tag = -1;
    int32_t source_projectile_tag = -1;
    int32_t source_weapon_tag;
    int32_t replacement;

    g_pelican_impact_patch_error = 0;
    if (weapon_tag == -1) {
        g_pelican_impact_patch_error = 1;
        return 0;
    }
    definition = pelican_projectile_definition_for_weapon_tag(
        weapon_tag, &projectile_tag);
    if (!definition || projectile_tag == -1) {
        g_pelican_impact_patch_error = 2;
        return 0;
    }
    replacement = ((tag_load_fn)(g_blam + 0x197F60))(
        TAG_GROUP_EFFECT, g_pelican_human_impact_effect_path);
    if (replacement == -1 ||
        !((tag_get_fn)(g_blam + 0x201480))(
            TAG_GROUP_EFFECT, replacement)) {
        g_pelican_impact_patch_error = 3;
        return 0;
    }
    /* Le rayon retail de certaines chainguns vaut zero : zero signifie que
     * seul le point exact est echantillonne, ce qui ne produit aucun decal
     * visible avec le projectile Pelican. Utiliser un rayon materiau borne de
     * 0,10 unite; une valeur retail strictement positive reste prioritaire.
     * Ce correctif de presentation ne doit jamais bloquer l'armement. */
    replacement_radius = 0.10f;
    source_weapon_tag = ((tag_load_fn)(g_blam + 0x197F60))(
        TAG_GROUP_WEAPON, g_pelican_human_weapon_path);
    source_definition = source_weapon_tag == -1 ? NULL :
        pelican_projectile_definition_for_weapon_tag(
            source_weapon_tag, &source_projectile_tag);
    if (source_definition && source_projectile_tag != -1 &&
        readable_range(source_definition +
            PELICAN_PROJECTILE_MATERIAL_EFFECT_RADIUS_OFFSET,
            sizeof(float))) {
        float authored_radius = *(float *)(source_definition +
            PELICAN_PROJECTILE_MATERIAL_EFFECT_RADIUS_OFFSET);
        if (isfinite(authored_radius) && authored_radius > 0.0001f &&
            authored_radius <= 10.0f)
            replacement_radius = authored_radius;
    }
    if (g_pelican_impact_patch_active) {
        if (g_pelican_impact_projectile_tag != projectile_tag ||
            g_pelican_impact_effect != replacement ||
            g_pelican_impact_material_radius != replacement_radius) {
            if (!pelican_restore_impact_patch()) {
                g_pelican_impact_patch_error = 4;
                return 0;
            }
        } else {
            for (index = 0; index < 2u; ++index) {
                int32_t *slot = (int32_t *)(definition + offsets[index]);
                if ((g_pelican_impact_original_valid_mask &
                     (1u << index)) != 0u &&
                    (!readable_range(slot, sizeof(*slot)) ||
                     *slot != replacement)) {
                    g_pelican_impact_patch_error = 5;
                    return 0;
                }
            }
            if (g_pelican_impact_original_material_radius_valid) {
                float *radius = (float *)(definition +
                    PELICAN_PROJECTILE_MATERIAL_EFFECT_RADIUS_OFFSET);
                if (!readable_range(radius, sizeof(*radius)) ||
                    *radius != replacement_radius) {
                    g_pelican_impact_patch_error = 5;
                    return 0;
                }
            }
            return 1;
        }
    }
    for (index = 0; index < 2u; ++index) {
        slots[index] = (int32_t *)(definition + offsets[index]);
        if (!readable_range(slots[index], sizeof(*slots[index]))) {
            g_pelican_impact_patch_error = 6;
            return 0;
        }
        originals[index] = *slots[index];
        if (originals[index] != replacement)
            changed_mask |= (uint8_t)(1u << index);
    }
    radius_slot = (float *)(definition +
        PELICAN_PROJECTILE_MATERIAL_EFFECT_RADIUS_OFFSET);
    if (!readable_range(radius_slot, sizeof(*radius_slot))) {
        g_pelican_impact_patch_error = 6;
        return 0;
    }
    original_radius = *radius_slot;
    if (!isfinite(original_radius)) {
        g_pelican_impact_patch_error = 6;
        return 0;
    }
    radius_changed = original_radius != replacement_radius;
    for (index = 0; index < 2u; ++index) {
        if ((changed_mask & (1u << index)) == 0u) continue;
        if (!pelican_write_presentation_datum(
                slots[index], originals[index], replacement)) {
            goto impact_patch_rollback;
        }
    }
    if (radius_changed && !pelican_write_presentation_float(
            radius_slot, original_radius, replacement_radius)) {
        /* Les deux effets d'impact restent valides meme si ce champ optionnel
         * est protege par une autre couche. Ne jamais desarmer la tourelle
         * pour un echec purement visuel. Si la valeur a tout de meme ete
         * ecrite, en conserver la propriete afin de pouvoir la restaurer. */
        if (!readable_range(radius_slot, sizeof(*radius_slot)) ||
            *radius_slot != replacement_radius)
            radius_changed = 0;
    }

    g_pelican_impact_projectile_tag = projectile_tag;
    g_pelican_impact_original_effects[0] = originals[0];
    g_pelican_impact_original_effects[1] = originals[1];
    g_pelican_impact_effect = replacement;
    g_pelican_impact_original_valid_mask = changed_mask;
    g_pelican_impact_original_material_radius = original_radius;
    g_pelican_impact_material_radius = replacement_radius;
    g_pelican_impact_original_material_radius_valid = radius_changed;
    g_pelican_impact_patch_active = changed_mask != 0u || radius_changed;
    if (changed_mask & 1u) ++g_pelican_impact_patch_count;
    if (changed_mask & 2u) ++g_pelican_impact_patch_count;
    if (radius_changed) ++g_pelican_impact_patch_count;
    return 1;

impact_patch_rollback:
    /* Une ecriture peut avoir reussi alors que la remise de protection a
     * echoue. Restaurer tout champ encore egal a notre remplacement, puis
     * publier les rares champs restants avant une seconde tentative. */
    for (index = 0; index < 2u; ++index) {
        if ((changed_mask & (1u << index)) != 0u &&
            readable_range(slots[index], sizeof(*slots[index])) &&
            *slots[index] == replacement)
            (void)pelican_write_presentation_datum(
                slots[index], replacement, originals[index]);
    }
    if (radius_changed && readable_range(radius_slot, sizeof(*radius_slot)) &&
        *radius_slot == replacement_radius)
        (void)pelican_write_presentation_float(
            radius_slot, replacement_radius, original_radius);
    {
        uint8_t remaining_mask = 0u;
        int remaining_radius = 0;
        for (index = 0; index < 2u; ++index) {
            if ((changed_mask & (1u << index)) != 0u &&
                readable_range(slots[index], sizeof(*slots[index])) &&
                *slots[index] == replacement)
                remaining_mask |= (uint8_t)(1u << index);
        }
        remaining_radius = radius_changed &&
            readable_range(radius_slot, sizeof(*radius_slot)) &&
            *radius_slot == replacement_radius;
        if (remaining_mask != 0u || remaining_radius) {
            g_pelican_impact_projectile_tag = projectile_tag;
            g_pelican_impact_original_effects[0] = originals[0];
            g_pelican_impact_original_effects[1] = originals[1];
            g_pelican_impact_effect = replacement;
            g_pelican_impact_original_valid_mask = remaining_mask;
            g_pelican_impact_original_material_radius = original_radius;
            g_pelican_impact_material_radius = replacement_radius;
            g_pelican_impact_original_material_radius_valid =
                remaining_radius;
            g_pelican_impact_patch_active = 1;
            (void)pelican_restore_impact_patch();
        } else {
            pelican_forget_impact_patch(0);
        }
    }
    g_pelican_impact_patch_error = 7;
    return 0;
}

static int pelican_patch_impact_for_weapon(int32_t weapon_datum) {
    uint8_t *weapon = object_data(weapon_datum);
    if (!weapon || !readable_range(weapon, sizeof(int32_t))) {
        g_pelican_impact_patch_error = 1;
        return 0;
    }
    return pelican_patch_impact_for_weapon_tag(*(int32_t *)weapon);
}

/* Selectionne le projectile Warthog deja resident par l'arme physique du
 * proxy audio. Ses reponses materiau, decals et effets objet/BSP restent donc
 * entierement authored et geres par le moteur. Aucun datum de definition
 * Pelican n'est modifie et tag_release voit une dependance normale. */
static int pelican_prepare_aux_impact_projectile(int32_t weapon_datum) {
    uint8_t *weapon = object_data(weapon_datum);
    uint8_t *definition;
    int32_t projectile_tag = -1;
    int32_t ground_effect;
    int32_t object_effect;
    int32_t impact_damage;
    g_pelican_aux_impact_projectile_tag = -1;
    if (!weapon || !readable_range(weapon, sizeof(int32_t))) return 0;
    definition = pelican_projectile_definition_for_weapon_tag(
        *(int32_t *)weapon, &projectile_tag);
    if (!definition || projectile_tag == -1 ||
        !readable_range(definition + PELICAN_PROJECTILE_IMPACT_EFFECT_OFFSET,
                        sizeof(int32_t)) ||
        !readable_range(
            definition + PELICAN_PROJECTILE_OBJECT_IMPACT_EFFECT_OFFSET,
            sizeof(int32_t)) ||
        !readable_range(definition + PROJECTILE_IMPACT_DAMAGE_TAG_OFFSET,
                        sizeof(int32_t))) return 0;
    ground_effect = *(int32_t *)(
        definition + PELICAN_PROJECTILE_IMPACT_EFFECT_OFFSET);
    object_effect = *(int32_t *)(
        definition + PELICAN_PROJECTILE_OBJECT_IMPACT_EFFECT_OFFSET);
    impact_damage = *(int32_t *)(
        definition + PROJECTILE_IMPACT_DAMAGE_TAG_OFFSET);
    /* Certaines balles retail portent leurs reponses visuelles dans le graphe
     * de degats/materiaux et laissent ces deux references directes vides. Le
     * dommage authored non vide est la preuve determinante qu'il s'agit bien
     * du projectile physique utilisable. */
    if (impact_damage == -1 && ground_effect == -1 && object_effect == -1)
        return 0;
    g_pelican_aux_impact_projectile_tag = projectile_tag;
    return 1;
}

/* Ecriture compare-and-swap du seul champ retail rounds-between-tracers.
 * VirtualProtect est restaure dans tous les chemins apres son acquisition. */
static int pelican_write_tracer_rounds(
        uint16_t *slot, uint16_t expected, uint16_t value) {
    DWORD old_protect = 0;
    DWORD ignored_protect = 0;
    SHORT observed;
    int wrote;
    int protection_restored;
    if (!slot || ((uintptr_t)slot & (sizeof(*slot) - 1u)) != 0u ||
        !readable_range(slot, sizeof(*slot)) ||
        !VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &old_protect))
        return 0;
    observed = InterlockedCompareExchange16(
        (volatile SHORT *)slot, (SHORT)value, (SHORT)expected);
    MemoryBarrier();
    wrote = (uint16_t)observed == expected &&
        readable_range(slot, sizeof(*slot)) && *slot == value;
    protection_restored = VirtualProtect(
        slot, sizeof(*slot), old_protect, &ignored_protect) != 0;
    return wrote && protection_restored &&
        readable_range(slot, sizeof(*slot)) && *slot == value;
}

static void pelican_forget_tracer_patch(int reset_counter) {
    g_pelican_tracer_weapon_tag = -1;
    g_pelican_tracer_original_rounds = 0u;
    g_pelican_tracer_original_valid = 0;
    g_pelican_tracer_patch_active = 0;
    if (reset_counter) g_pelican_tracer_patch_count = 0;
}

/* Restauration symetrique : seul notre zero est remplace par l'original. Si
 * un autre systeme a pris possession du champ, sa valeur n'est jamais ecrasee. */
static int pelican_restore_tracer_patch(void) {
    uint8_t *barrel;
    uint16_t *slot;
    uint16_t current;
    if (!g_pelican_tracer_patch_active) return 1;
    barrel = pelican_primary_barrel_for_weapon_tag(
        g_pelican_tracer_weapon_tag);
    if (!barrel || !g_pelican_tracer_original_valid ||
        !readable_range(barrel + 0x3Eu, sizeof(*slot))) {
        g_pelican_tracer_patch_error = 7;
        return 0;
    }
    slot = (uint16_t *)(barrel + 0x3Eu);
    current = *slot;
    if (current == 0u && !pelican_write_tracer_rounds(
            slot, 0u, g_pelican_tracer_original_rounds)) {
        g_pelican_tracer_patch_error = 8;
        return 0;
    }
    /* current != 0 signifie soit deja restaure, soit mutation tierce. Dans
     * les deux cas nous abandonnons la propriete sans ecriture destructive. */
    pelican_forget_tracer_patch(0);
    return 1;
}

static int pelican_patch_tracer_for_weapon_tag(int32_t weapon_tag) {
    uint8_t *barrel;
    uint16_t *slot;
    uint16_t original;
    g_pelican_tracer_patch_error = 0;
    if (weapon_tag == -1) {
        g_pelican_tracer_patch_error = 1;
        return 0;
    }
    if (g_pelican_tracer_patch_active) {
        if (g_pelican_tracer_weapon_tag != weapon_tag) {
            if (!pelican_restore_tracer_patch()) {
                g_pelican_tracer_patch_error = 4;
                return 0;
            }
        } else {
            barrel = pelican_primary_barrel_for_weapon_tag(weapon_tag);
            if (!barrel || !readable_range(
                    barrel + 0x3Eu, sizeof(*slot))) {
                g_pelican_tracer_patch_error = 2;
                return 0;
            }
            slot = (uint16_t *)(barrel + 0x3Eu);
            if (*slot == 0u) return 1;
            if (!g_pelican_tracer_original_valid ||
                *slot != g_pelican_tracer_original_rounds) {
                /* Une mutation tierce n'est pas notre patch actif. */
                pelican_forget_tracer_patch(0);
                g_pelican_tracer_patch_error = 3;
                return 0;
            }
            /* Restauration externe a la valeur d'origine : reprendre un
             * snapshot neuf au lieu de reutiliser un etat devenu stale. */
            pelican_forget_tracer_patch(0);
        }
    }
    barrel = pelican_primary_barrel_for_weapon_tag(weapon_tag);
    if (!barrel || !readable_range(barrel + 0x3Eu, sizeof(*slot))) {
        g_pelican_tracer_patch_error = 2;
        return 0;
    }
    slot = (uint16_t *)(barrel + 0x3Eu);
    original = *slot;
    if (!pelican_write_tracer_rounds(slot, original, 0u)) {
        /* Le CAS peut avoir reussi alors que la restauration de protection a
         * echoue. Enregistrer temporairement la propriete permet un rollback
         * symetrique au lieu de laisser silencieusement le zero en place. */
        if (readable_range(slot, sizeof(*slot)) && *slot == 0u) {
            g_pelican_tracer_weapon_tag = weapon_tag;
            g_pelican_tracer_original_rounds = original;
            g_pelican_tracer_original_valid = 1;
            g_pelican_tracer_patch_active = 1;
            if (!pelican_restore_tracer_patch()) return 0;
        }
        g_pelican_tracer_patch_error = 5;
        return 0;
    }
    g_pelican_tracer_weapon_tag = weapon_tag;
    g_pelican_tracer_original_rounds = original;
    g_pelican_tracer_original_valid = 1;
    g_pelican_tracer_patch_active = 1;
    if (original != 0u) ++g_pelican_tracer_patch_count;
    return 1;
}

static int pelican_patch_tracer_for_weapon(int32_t weapon_datum) {
    uint8_t *weapon = object_data(weapon_datum);
    if (!weapon || !readable_range(weapon, sizeof(int32_t))) {
        g_pelican_tracer_patch_error = 1;
        return 0;
    }
    return pelican_patch_tracer_for_weapon_tag(*(int32_t *)weapon);
}

static void pelican_forget_presentation_patch(int reset_counter) {
    g_pelican_presentation_weapon_tag = -1;
    g_pelican_presentation_original_effect = -1;
    g_pelican_presentation_effect = -1;
    g_pelican_presentation_record_count = 0;
    g_pelican_presentation_secondary_mask = 0;
    g_pelican_presentation_patch_active = 0;
    memset(g_pelican_presentation_records, 0,
           sizeof(g_pelican_presentation_records));
    if (reset_counter) g_pelican_presentation_patch_count = 0;
}

/* Lorsqu'un autre weapon tag devient le vrai canon actif dans la meme mission,
 * restaurer les datums du precedent avant de greffer le nouveau. Les slots sont
 * re-resolus par index; aucun pointeur de definition n'est conserve. Une valeur
 * modifiee par un autre systeme n'est jamais ecrasee. */
static int pelican_restore_presentation_patch(void) {
    uint8_t *effects;
    uint32_t effect_count = 0;
    uint32_t i;
    if (!g_pelican_presentation_patch_active) return 1;
    effects = pelican_firing_effect_records(
        g_pelican_presentation_weapon_tag, NULL, &effect_count);
    if (!effects) {
        pelican_forget_presentation_patch(0);
        return 1;
    }
    for (i = 0; i < g_pelican_presentation_record_count &&
                i < effect_count; ++i) {
        struct pelican_presentation_record_patch *saved =
            &g_pelican_presentation_records[i];
        int32_t *slot;
        if (!saved->valid ||
            (saved->slot_offset != 0x10u &&
             saved->slot_offset != 0x40u)) continue;
        slot = (int32_t *)(effects + (size_t)i * 0xC4u +
                          saved->slot_offset);
        if (!readable_range(slot, sizeof(*slot))) continue;
        if (*slot == g_pelican_presentation_effect &&
            !pelican_write_presentation_datum(
                slot, g_pelican_presentation_effect,
                saved->original_effect)) return 0;
    }
    pelican_forget_presentation_patch(0);
    return 1;
}

/* Greffe exclusivement la presentation humaine de la mitrailleuse Warthog.
 * weapon_fire continue ainsi son pipeline retail unique (effet + son) tandis
 * que le projectile et tous les parametres de gameplay du Pelican restent
 * intacts. Le tag_load est celui du moteur et rend l'effet disponible aussi
 * dans les missions qui ne contiennent aucun Warthog. */
static int pelican_patch_human_presentation_for_weapon_tag(
        int32_t weapon_tag) {
    struct pending_patch {
        int32_t *slot;
        int32_t original;
        uint8_t offset;
        uint8_t changed;
        uint8_t original_known;
    } pending[PELICAN_MAX_FIRING_EFFECT_RECORDS];
    uint8_t *effects;
    uint32_t effect_count = 0;
    uint32_t secondary_mask = 0;
    uint32_t applied_count = 0;
    uint32_t i;
    int reuse_originals;
    int32_t replacement;
    g_pelican_presentation_patch_error = 0;
    if (g_pelican_presentation_patch_active &&
        g_pelican_presentation_weapon_tag != weapon_tag &&
        !pelican_restore_presentation_patch()) {
        g_pelican_presentation_patch_error = 7;
        return 0;
    }
    replacement = ((tag_load_fn)(g_blam + 0x197F60))(
        TAG_GROUP_EFFECT, g_pelican_human_firing_effect_path);
    if (replacement == -1 ||
        !((tag_get_fn)(g_blam + 0x201480))(
            TAG_GROUP_EFFECT, replacement)) {
        g_pelican_presentation_patch_error = 1;
        return 0;
    }
    effects = pelican_firing_effect_records(
        weapon_tag, NULL, &effect_count);
    if (!effects || !effect_count ||
        effect_count > PELICAN_MAX_FIRING_EFFECT_RECORDS) {
        g_pelican_presentation_patch_error = 2;
        return 0;
    }
    memset(pending, 0, sizeof(pending));
    reuse_originals = g_pelican_presentation_patch_active &&
        g_pelican_presentation_weapon_tag == weapon_tag &&
        g_pelican_presentation_effect == replacement &&
        g_pelican_presentation_record_count == effect_count;
    for (i = 0; i < effect_count; ++i) {
        uint8_t *record = effects + (size_t)i * 0xC4u;
        int32_t current = -1;
        if (!pelican_effect_record_selected_slot(
                record, &pending[i].slot, &current, &pending[i].offset) ||
            !pending[i].slot) {
            g_pelican_presentation_patch_error = 3;
            return 0;
        }
        if (pending[i].offset == 0x40u)
            secondary_mask |= 1u << i;
        if (reuse_originals &&
            g_pelican_presentation_records[i].slot_offset !=
                pending[i].offset) {
            g_pelican_presentation_patch_error = 9;
            return 0;
        }
        if (reuse_originals) {
            if (current != replacement) {
                /* Une mutation tierce apparue apres notre patch reste
                 * proprietaire de ce slot : ne jamais l'ecraser. */
                g_pelican_presentation_patch_error = 8;
                return 0;
            }
            if (g_pelican_presentation_records[i].valid) {
                pending[i].original =
                    g_pelican_presentation_records[i].original_effect;
                pending[i].original_known = 1;
            }
        } else if (current != replacement) {
            pending[i].original = current;
            pending[i].original_known = 1;
        }
        pending[i].changed = current != replacement;
    }
    for (i = 0; i < effect_count; ++i) {
        uint32_t rollback;
        if (!pending[i].changed) continue;
        if (!pelican_write_presentation_datum(
                pending[i].slot, pending[i].original, replacement)) {
            for (rollback = 0; rollback <= i; ++rollback) {
                if (pending[rollback].changed &&
                    pending[rollback].original_known)
                    (void)pelican_write_presentation_datum(
                        pending[rollback].slot,
                        replacement,
                        pending[rollback].original);
            }
            g_pelican_presentation_patch_error = 4;
            return 0;
        }
        ++applied_count;
    }
    g_pelican_presentation_weapon_tag = weapon_tag;
    g_pelican_presentation_original_effect =
        pending[0].original_known ? pending[0].original : -1;
    g_pelican_presentation_effect = replacement;
    memset(g_pelican_presentation_records, 0,
           sizeof(g_pelican_presentation_records));
    for (i = 0; i < effect_count; ++i) {
        g_pelican_presentation_records[i].original_effect =
            pending[i].original;
        g_pelican_presentation_records[i].slot_offset = pending[i].offset;
        g_pelican_presentation_records[i].valid =
            pending[i].original_known;
    }
    g_pelican_presentation_record_count = effect_count;
    g_pelican_presentation_secondary_mask = secondary_mask;
    g_pelican_presentation_patch_count += applied_count;
    g_pelican_presentation_patch_active = 1;
    return 1;
}

static int pelican_patch_human_presentation_for_weapon(
        int32_t weapon_datum) {
    uint8_t *weapon = object_data(weapon_datum);
    if (!weapon || !readable_range(weapon, sizeof(int32_t))) {
        g_pelican_presentation_patch_error = 5;
        return 0;
    }
    return pelican_patch_human_presentation_for_weapon_tag(
        *(int32_t *)weapon);
}

/* L'armement est une transaction presentation -> impact -> traceur. Si le
 * patch d'impact echoue, rendre d'abord tout slot partiellement acquis, puis
 * la presentation. L'appelant reste desarme dans tous les cas. */
static void pelican_rollback_failed_impact_arm_patch(void) {
    int impact_error = g_pelican_impact_patch_error;
    if (g_pelican_impact_patch_active &&
        !pelican_restore_impact_patch())
        impact_error = g_pelican_impact_patch_error;
    if (!pelican_restore_presentation_patch())
        g_pelican_presentation_patch_error = 12;
    g_pelican_impact_patch_error = impact_error ? impact_error : 8;
}

/* Si le dernier patch (traceur) echoue, restaurer dans l'ordre inverse et
 * conserver son code d'erreur initial pour le diagnostic. */
static void pelican_rollback_failed_tracer_arm_patch(void) {
    int tracer_error = g_pelican_tracer_patch_error;
    if (g_pelican_tracer_patch_active &&
        !pelican_restore_tracer_patch())
        tracer_error = g_pelican_tracer_patch_error;
    if (!pelican_restore_presentation_patch())
        g_pelican_presentation_patch_error = 11;
    g_pelican_tracer_patch_error = tracer_error ? tracer_error : 6;
}

/* Chemin client-local utilise par pelican_patch avant meme qu'une instance ne
 * soit creee. Le module ventral charge normalement ce weapon en dependance;
 * tag_load sert aussi de repli deterministe sur les missions sans Pelican. */
static int pelican_patch_original_chin_presentation(void) {
    int32_t weapon_tag = ((tag_load_fn)(g_blam + 0x197F60))(
        TAG_GROUP_WEAPON, g_pelican_chin_weapon_path);
    if (weapon_tag == -1 ||
        !((tag_get_fn)(g_blam + 0x201480))(
            TAG_GROUP_WEAPON, weapon_tag)) {
        g_pelican_presentation_patch_error = 6;
        return 0;
    }
    return pelican_patch_human_presentation_for_weapon_tag(weapon_tag);
}

/* Valide le vrai WeaponDatum du module ventral et calcule une cadence a partir
 * du barrel 0 tel qu'il est authoré dans le tag. Le tir complet est un appel
 * one-shot : ce garde-fou empeche donc les doubles tirs, les index de barrel
 * invalides et une cadence superieure a celle prevue par le jeu. */
static int pelican_primary_barrel_info(int32_t weapon_datum,
                                       int32_t expected_parent,
                                       uint32_t *interval_ms_out,
                                       uint8_t *serial_out) {
    uint8_t *object = object_data(weapon_datum);
    uint8_t *definition;
    uint8_t *barrels;
    uint32_t barrel_count = 0;
    int32_t parent;
    float rounds_per_second_min;
    float rounds_per_second_max;
    float rounds_per_second;
    float rate_scale;
    float quantized_steps;
    uint32_t interval_ms;
    tag_get_fn tag_get = (tag_get_fn)(g_blam + 0x201480);
    if (!object || !readable_range(object, sizeof(int32_t)) ||
        !readable_range(object + 0x14, sizeof(int32_t)) ||
        !readable_range(object + 0x244, sizeof(float)) ||
        !readable_range(object + 0x25E, sizeof(uint8_t))) return 0;
    parent = *(int32_t *)(object + 0x14);
    if (expected_parent != -1 && parent != expected_parent) return 0;
    definition = (uint8_t *)tag_get(0x77656170u, *(int32_t *)object);
    if (!definition) return 0;
    /* weapon_definition: barrels +0x3E4/+0x3E8, stride 0x188. */
    barrels = resolve_tag_block_raw(
        definition, 0x3E4u, 0x188u, 2u, &barrel_count);
    if (!barrels || !barrel_count) return 0;
    /* Le sous-struct firing commence a barrel+4. */
    rounds_per_second_min = *(float *)(barrels + 4u);
    rounds_per_second_max = *(float *)(barrels + 8u);
    rate_scale = *(float *)(object + 0x244);
    if (!isfinite(rounds_per_second_min) ||
        !isfinite(rounds_per_second_max)) return 0;
    /* La boucle retail reecrit ce scale dynamique a chaque tick. Ses chemins
     * d'intervalle et de tracer traitent une valeur unordered/NaN comme zero;
     * elle ne signifie donc pas que le WeaponDatum ou son barrel a disparu.
     * Conserver la meme valeur de repli evite un faux desarmement tout en
     * gardant les gardes structurelles parent/tag/barrel ci-dessus. */
    if (!isfinite(rate_scale)) {
        memcpy(&g_pelican_last_barrel_rate_bits,
               &rate_scale, sizeof(g_pelican_last_barrel_rate_bits));
        InterlockedExchange(
            &g_pelican_rate_scale_nonfinite_observed, 1);
        rate_scale = 0.0f;
    }
    if (rate_scale < 0.0f) rate_scale = 0.0f;
    else if (rate_scale > 1.0f) rate_scale = 1.0f;
    rounds_per_second = rounds_per_second_min +
        (rounds_per_second_max - rounds_per_second_min) * rate_scale;
    /* Halo quantifie la cadence sur une base de 30 Hz : 20 RPS devient donc
     * un coup toutes les deux subdivisions, soit 15 RPS. Le worker ne fait
     * qu'approcher cette cadence; le predicate runtime ci-dessous reste
     * l'autorite finale avant chaque coup. */
    if (rounds_per_second > 0.0001f && rounds_per_second < 1000.0f) {
        quantized_steps = ceilf(30.0f / rounds_per_second);
        interval_ms = (uint32_t)ceilf(1000.0f * quantized_steps / 30.0f);
    } else {
        interval_ms = 50u;
    }
    /* 20 appels/s maximum garde le relais main-thread fluide; un tag plus
     * lent conserve sa cadence integralement. */
    if (interval_ms < 50u) interval_ms = 50u;
    if (interval_ms > 5000u) interval_ms = 5000u;
    if (interval_ms_out) *interval_ms_out = interval_ms;
    if (serial_out) *serial_out = object[0x25E];
    return 1;
}

/* Snapshot read-only execute uniquement lorsqu'une validation de barrel
 * echoue sur le tick d'entree. Il persiste apres release afin que STATUS
 * distingue une parenté rompue, un tag detruit ou un simple champ numerique. */
static uint32_t pelican_primary_barrel_failure_detail(
        int32_t weapon_datum, int32_t expected_parent,
        int32_t *actual_parent_out, int32_t *weapon_tag_out,
        uint32_t *barrel_count_out, uint32_t *min_bits_out,
        uint32_t *max_bits_out, uint32_t *rate_bits_out) {
    uint32_t detail = 0u;
    uint8_t *object = object_data(weapon_datum);
    uint8_t *definition = NULL;
    uint8_t *barrels = NULL;
    uint32_t barrel_count = 0u;
    int32_t actual_parent = -1;
    int32_t weapon_tag = -1;
    float rounds_per_second_min = 0.0f;
    float rounds_per_second_max = 0.0f;
    float rate_scale = 0.0f;
    uint32_t min_bits = 0u;
    uint32_t max_bits = 0u;
    uint32_t rate_bits = 0u;

    if (!object) {
        detail |= PELICAN_BARREL_DETAIL_OBJECT;
        goto done;
    }
    if (!readable_range(object, sizeof(int32_t)) ||
        !readable_range(object + 0x14, sizeof(int32_t)) ||
        !readable_range(object + 0x244, sizeof(float)) ||
        !readable_range(object + 0x25E, sizeof(uint8_t))) {
        detail |= PELICAN_BARREL_DETAIL_RANGE;
        goto done;
    }
    weapon_tag = *(int32_t *)object;
    actual_parent = *(int32_t *)(object + 0x14);
    if (expected_parent != -1 && actual_parent != expected_parent)
        detail |= PELICAN_BARREL_DETAIL_PARENT;
    definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
        0x77656170u, weapon_tag);
    if (!definition) {
        detail |= PELICAN_BARREL_DETAIL_TAG;
        goto done;
    }
    barrels = resolve_tag_block_raw(
        definition, 0x3E4u, 0x188u, 2u, &barrel_count);
    if (!barrels || !barrel_count) {
        detail |= PELICAN_BARREL_DETAIL_BLOCK;
        goto done;
    }
    rounds_per_second_min = *(float *)(barrels + 4u);
    rounds_per_second_max = *(float *)(barrels + 8u);
    rate_scale = *(float *)(object + 0x244);
    memcpy(&min_bits, &rounds_per_second_min, sizeof(min_bits));
    memcpy(&max_bits, &rounds_per_second_max, sizeof(max_bits));
    memcpy(&rate_bits, &rate_scale, sizeof(rate_bits));
    if (!isfinite(rounds_per_second_min) ||
        !isfinite(rounds_per_second_max))
        detail |= PELICAN_BARREL_DETAIL_MINMAX;
    if (!isfinite(rate_scale))
        detail |= PELICAN_BARREL_DETAIL_RATE_SCALE;

done:
    if (actual_parent_out) *actual_parent_out = actual_parent;
    if (weapon_tag_out) *weapon_tag_out = weapon_tag;
    if (barrel_count_out) *barrel_count_out = barrel_count;
    if (min_bits_out) *min_bits_out = min_bits;
    if (max_bits_out) *max_bits_out = max_bits;
    if (rate_bits_out) *rate_bits_out = rate_bits;
    return detail;
}

struct pelican_direct_authored {
    uint32_t interval_ticks;
    uint32_t recovery_ticks;
    int16_t min_shots;
    int16_t max_shots;
};

struct pelican_direct_fire_info {
    int32_t tick;
    uint8_t serial;
    struct pelican_direct_authored authored;
};

static int game_time_snapshot(int32_t *tick_out, int16_t *rate_out) {
    uint8_t *thread_state = blam_thread_state();
    uint8_t *game_time;
    int16_t rate;
    if (!tick_out || !rate_out || !thread_state ||
        !readable_range(thread_state + 0x98, sizeof(game_time))) return 0;
    game_time = *(uint8_t **)(thread_state + 0x98);
    if (!game_time || !readable_range(game_time + 6, sizeof(int16_t)) ||
        !readable_range(game_time + 0x0C, sizeof(int32_t))) return 0;
    rate = *(int16_t *)(game_time + 6);
    if (rate <= 0 || rate > 120) return 0;
    *rate_out = rate;
    *tick_out = *(int32_t *)(game_time + 0x0C);
    return 1;
}

static int pelican_tick_elapsed(int32_t now, int32_t then,
                                uint32_t interval) {
    return ((uint32_t)now - (uint32_t)then) >= interval;
}

static int pelican_tick_reached(int32_t now, int32_t deadline) {
    return (int32_t)((uint32_t)now - (uint32_t)deadline) >= 0;
}

static void pelican_direct_gate_clear(int32_t weapon_datum) {
    memset(&g_pelican_direct_gate, 0, sizeof(g_pelican_direct_gate));
    g_pelican_direct_gate.weapon = weapon_datum;
}

/* Snapshot authoré du barrel 0. L'intervalle provient directement de la
 * routine retail 0x5C97D0; le recovery total est reserve au relachement. */
static int pelican_direct_fire_snapshot(
        int32_t weapon_datum, int32_t expected_parent,
        struct pelican_direct_fire_info *out) {
    uint8_t *weapon = object_data(weapon_datum);
    uint8_t *definition;
    uint8_t *barrels;
    uint32_t barrel_count = 0;
    int32_t current_tick;
    int16_t tick_rate;
    float interval_float;
    float recovery_seconds;
    float recovery_base;
    if (!out || !weapon ||
        !readable_range(weapon, sizeof(int32_t)) ||
        !readable_range(weapon + 0x14, sizeof(int32_t)) ||
        !readable_range(weapon + 0x235, 3u) ||
        !readable_range(weapon + 0x23A, sizeof(uint16_t)) ||
        !readable_range(weapon + 0x25E, sizeof(uint8_t)) ||
        !readable_range(weapon + 0x2EC, sizeof(int32_t)) ||
        (expected_parent != -1 &&
         *(int32_t *)(weapon + 0x14) != expected_parent)) return 0;
    definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
        0x77656170u, *(int32_t *)weapon);
    barrels = resolve_tag_block_raw(
        definition, 0x3E4u, 0x188u, 2u, &barrel_count);
    if (!barrels || barrel_count < 1u ||
        !game_time_snapshot(&current_tick, &tick_rate)) return 0;
    interval_float = ((weapon_interval_fn)(g_blam + 0x5C97D0))(
        weapon_datum, 0, 1);
    recovery_seconds = *(float *)(barrels + 0x28);
    if (!isfinite(interval_float) || !isfinite(recovery_seconds) ||
        interval_float > 100000.0f || interval_float < 0.0f) return 0;
    out->authored.interval_ticks = interval_float > 0.0f
        ? (uint32_t)ceilf(interval_float) : 1u;
    if (!out->authored.interval_ticks)
        out->authored.interval_ticks = 1u;
    recovery_base = fmaxf(1.0f, recovery_seconds * (float)tick_rate);
    if (!isfinite(recovery_base) || recovery_base > 100000.0f) return 0;
    out->authored.recovery_ticks = (uint32_t)ceilf(recovery_base);
    out->authored.min_shots = *(int16_t *)(barrels + 0x24);
    out->authored.max_shots = *(int16_t *)(barrels + 0x26);
    out->tick = current_tick;
    out->serial = weapon[0x25E];
    return 1;
}

/* Prepare une instance neuve ou restauree. Un etat 1 indique un tir natif
 * concurrent; tous les etats stale sont remis idle une seule fois avant arm. */
static int pelican_prepare_direct_barrel(int32_t weapon_datum) {
    uint8_t *weapon = object_data(weapon_datum);
    if (!weapon || !readable_range(weapon + 0x235, 3u) ||
        !pelican_primary_barrel_info(
            weapon_datum, -1, NULL, NULL)) return 0;
    if (*(int8_t *)(weapon + 0x235) != 0 ||
        *(int16_t *)(weapon + 0x236) != 0) {
        ((weapon_barrel_reset_fn)(g_blam + 0x5CE2D0))(weapon_datum, 0);
        weapon = object_data(weapon_datum);
    }
    return weapon && readable_range(weapon + 0x235, 3u) &&
        *(int8_t *)(weapon + 0x235) == 0 &&
        *(int16_t *)(weapon + 0x236) == 0;
}

/* Met a jour la pression sans ecrire les compteurs runtime. Le reset natif au
 * relachement nettoie +0x23A et les flags exactement comme une vraie arme. */
static int pelican_direct_gate_update(
        int32_t weapon_datum, int trigger_down,
        const struct pelican_direct_fire_info *info) {
    uint8_t *weapon = object_data(weapon_datum);
    int32_t engine_last;
    if (!info || !weapon ||
        !readable_range(weapon + 0x235, 3u) ||
        !readable_range(weapon + 0x23A, sizeof(uint16_t)) ||
        !readable_range(weapon + 0x2EC, sizeof(int32_t))) return 0;
    if (g_pelican_direct_gate.weapon != weapon_datum)
        pelican_direct_gate_clear(weapon_datum);
    if (!trigger_down) {
        if (g_pelican_direct_gate.was_down) {
            g_pelican_direct_gate.was_down = 0;
            if (g_pelican_direct_gate.had_success) {
                g_pelican_direct_gate.recovery_until_tick =
                    info->tick + (int32_t)info->authored.recovery_ticks;
                g_pelican_direct_gate.recovery_active = 1;
            }
            if (g_pelican_direct_gate.had_success ||
                *(uint16_t *)(weapon + 0x23A) != 0)
                ((weapon_barrel_reset_fn)(g_blam + 0x5CE2D0))(
                    weapon_datum, 0);
            g_pelican_direct_gate.shots_in_hold = 0;
            g_pelican_direct_gate.had_success = 0;
            g_pelican_direct_gate.has_attempt_in_hold = 0;
        }
        return 0;
    }
    if (!g_pelican_direct_gate.was_down) {
        g_pelican_direct_gate.was_down = 1;
        g_pelican_direct_gate.shots_in_hold = 0;
        g_pelican_direct_gate.had_success = 0;
        g_pelican_direct_gate.has_attempt_in_hold = 0;
    }
    if (g_pelican_direct_gate.recovery_active) {
        if (!pelican_tick_reached(
                info->tick, g_pelican_direct_gate.recovery_until_tick))
            return 0;
        g_pelican_direct_gate.recovery_active = 0;
    }
    if (info->authored.max_shots > 0 &&
        (g_pelican_direct_gate.shots_in_hold >=
             (uint16_t)info->authored.max_shots ||
         *(uint16_t *)(weapon + 0x23A) >=
             (uint16_t)info->authored.max_shots)) {
        /* Une arme a rafales doit entrer en recovery puis recommencer meme si
         * le pilote garde la detente enfoncee. Sans ce reset, max_shots
         * bloquerait toute la pression apres la premiere rafale. */
        ((weapon_barrel_reset_fn)(g_blam + 0x5CE2D0))(weapon_datum, 0);
        g_pelican_direct_gate.recovery_until_tick =
            info->tick + (int32_t)info->authored.recovery_ticks;
        g_pelican_direct_gate.recovery_active = 1;
        g_pelican_direct_gate.shots_in_hold = 0;
        g_pelican_direct_gate.had_success = 0;
        g_pelican_direct_gate.has_attempt_in_hold = 0;
        return 0;
    }
    /* Direct-only : state 0/timer 0 obligatoires. State 1 appartient deja au
     * pipeline moteur et provoquerait un double tir. Le plafond de rafale est
     * traite avant afin de pouvoir nettoyer cet etat retail. */
    if (*(int8_t *)(weapon + 0x235) != 0 ||
        *(int16_t *)(weapon + 0x236) != 0) return 0;
    engine_last = *(int32_t *)(weapon + 0x2EC);
    if (engine_last != -1 && !pelican_tick_elapsed(
            info->tick, engine_last, info->authored.interval_ticks)) return 0;
    if (g_pelican_direct_gate.has_attempt_in_hold &&
        !pelican_tick_elapsed(
            info->tick, g_pelican_direct_gate.last_attempt_tick,
            info->authored.interval_ticks)) return 0;
    g_pelican_direct_gate.last_attempt_tick = info->tick;
    g_pelican_direct_gate.has_attempt_in_hold = 1;
    return 1;
}

static void pelican_direct_gate_commit(int32_t tick, int succeeded) {
    if (!succeeded) return;
    g_pelican_direct_gate.last_success_tick = tick;
    g_pelican_direct_gate.had_success = 1;
    if (g_pelican_direct_gate.shots_in_hold != UINT16_MAX)
        ++g_pelican_direct_gate.shots_in_hold;
}

/* La boucle retail weapon_update efface ces deux latches juste avant un tir :
 * bit 7 autorise le nouvel effet de bouche/son et bit 0 autorise le nouvel
 * evenement de canon. weapon_barrel_reset ne nettoie pas le bit 7; sans cette
 * etape, l'appel direct continue a creer projectile et degats mais les FX du
 * premier coup restent verrouilles pour les suivants. Aucun autre flag du
 * barrel n'est touche. */
static int pelican_prepare_direct_shot(int32_t weapon_datum) {
    uint8_t *weapon = object_data(weapon_datum);
    uint16_t *barrel_flags;
    if (!weapon || !writable_range(
            weapon + 0x238, sizeof(uint16_t)) ||
        !pelican_primary_barrel_info(
            weapon_datum, -1, NULL, NULL)) return 0;
    barrel_flags = (uint16_t *)(weapon + 0x238);
    *barrel_flags = (uint16_t)(*barrel_flags & (uint16_t)0xFF7Eu);
    return 1;
}

static void pelican_direct_gate_abandon(int32_t weapon_datum) {
    uint8_t *weapon = object_data(weapon_datum);
    if (weapon && readable_range(weapon + 0x23A, sizeof(uint16_t)) &&
        pelican_primary_barrel_info(weapon_datum, -1, NULL, NULL) &&
        (g_pelican_direct_gate.was_down ||
         *(uint16_t *)(weapon + 0x23A) != 0))
        ((weapon_barrel_reset_fn)(g_blam + 0x5CE2D0))(weapon_datum, 0);
    pelican_direct_gate_clear(-1);
}

/* Meme etat que weapon_set_primary_barrel_firing dans la build retail : le
 * bit 1 du word weapon+0x1BA commande le canon primaire, puis 0x59F460 marque
 * l'objet et sa chaine de parents comme modifies. */
static int set_weapon_primary_firing(int32_t weapon_datum, int enabled) {
    uint8_t *weapon = object_data(weapon_datum);
    uint16_t value;
    if (!weapon || !writable_range(weapon + 0x1BA, sizeof(uint16_t))) return 0;
    value = *(uint16_t *)(weapon + 0x1BA);
    value = enabled ? (uint16_t)(value | 2u) : (uint16_t)(value & ~2u);
    *(uint16_t *)(weapon + 0x1BA) = value;
    ((weapon_update_fn)(g_blam + 0x59F460))(weapon_datum);
    return object_data(weapon_datum) != NULL;
}

static int32_t first_unit_weapon(int32_t unit_datum) {
    uint8_t *unit = object_data(unit_datum);
    int slot;
    if (!unit || !readable_range(unit + 0x344, 4u * sizeof(int32_t))) return -1;
    for (slot = 0; slot < 4; ++slot) {
        int32_t datum = *(int32_t *)(unit + 0x344 + slot * 4);
        uint8_t *item = object_data(datum);
        if (item && readable_range(item, sizeof(int32_t)) &&
            ((tag_get_fn)(g_blam + 0x201480))(0x77656170u, *(int32_t *)item))
            return datum;
    }
    return -1;
}

/* Disposition UnitDatum de Halo Campaign Evolved 1.111.2544.0, verifiee dans
 * HaloSimulation_tag_release.dll. Elle differe fortement de Halo CE retail :
 * reutiliser les anciens offsets ecrasait le bloc UnitAnimation. */
#define UNIT_FLAGS_OFFSET                    0x1B4u
#define OBJECT_DAMAGE_OWNER_OFFSET           0x0F0u
#define UNIT_DESIRED_AIMING_VECTOR_OFFSET    0x1D4u
#define UNIT_AIMING_VECTOR_OFFSET            0x1E0u
#define UNIT_AIMING_HISTORY_VECTOR_OFFSET    0x1ECu
#define UNIT_DESIRED_LOOKING_VECTOR_OFFSET   0x1F8u
#define UNIT_LOOKING_VECTOR_OFFSET           0x204u
#define PELICAN_1549_DESIRED_AIM_OFFSET      0x1E0u
#define PELICAN_1549_CURRENT_AIM_OFFSET      0x1ECu
#define PELICAN_1549_DESIRED_LOOK_OFFSET     0x204u
#define PELICAN_1549_CURRENT_LOOK_OFFSET     0x210u
#define UNIT_LOOKING_HISTORY_VECTOR_OFFSET   0x210u
#define UNIT_SECONDARY_AIM_HISTORY_OFFSET    0x21Cu
#define UNIT_AIM_VECTOR_BLOCK_SIZE           0x054u
#define UNIT_DAMAGE_OWNER_TEAM_OFFSET        0x1BAu
#define UNIT_CONTROL_FLAGS_OFFSET            0x1C8u
#define UNIT_DRIVER_PRIMARY_TRIGGER_OFFSET   0x240u
#define UNIT_CONTROL_AIM_VECTOR_OFFSET        0x250u
#define UNIT_CONTROL_AIM_VECTOR_SIZE          0x00Cu
#define UNIT_CONTROL_AIM_SCALAR_OFFSET        0x274u
#define UNIT_VEHICLE_PRIMARY_TRIGGER_OFFSET  0x2A0u
#define UNIT_CURRENT_WEAPON_INDEX_OFFSET     0x33Eu
#define UNIT_DESIRED_WEAPON_INDEX_OFFSET     0x33Fu
#define UNIT_WEAPON_INVENTORY_OFFSET         0x344u
#define UNIT_DRIVER_OBJECT_OFFSET            0x390u
#define UNIT_GUNNER_OBJECT_OFFSET            0x394u
#define UNIT_FLAG_ACTIVELY_CONTROLLED         (1u << 0)
#define UNIT_FLAG_CHANGED_AIMING_ANGLES       (1u << 1)
#define UNIT_FLAG_DESIRED_WEAPON_INVALID      (1u << 2)
#define UNIT_CONTROL_PRIMARY_TRIGGER_BIT      ((uint64_t)1u << 11)
#define UNIT_CONTROL_VEHICLE_PRIMARY_BIT      ((uint64_t)1u << 32)
#define WEAPON_OWNER_UNIT_OFFSET               0x320u
#define OBJECT_PARENT_OFFSET                    0x014u
#define OBJECT_LINEAR_VELOCITY_OFFSET           0x068u
#define PROJECTILE_SOURCE_ROOT_OFFSET           0x1B4u
#define PROJECTILE_WEAPON_OFFSET                0x1C8u
#define PROJECTILE_BARREL_OFFSET                0x24Au
#define WEAPON_BARREL_PROJECTILE_TAG_OFFSET     0x114u
#define WEAPON_BARREL_SECONDARY_TAG_OFFSET      0x124u
#define WEAPON_BARREL_DAMAGE_TAG_OFFSET         0x134u
#define WEAPON_BARREL_CRATE_TAG_OFFSET          0x144u

static int unit_weapon_slot(int32_t unit_datum, int32_t weapon_datum) {
    uint8_t *unit = object_data(unit_datum);
    int slot;
    if (!unit || !readable_range(unit + UNIT_WEAPON_INVENTORY_OFFSET,
                                 4u * sizeof(int32_t))) return -1;
    for (slot = 0; slot < 4; ++slot)
        if (*(int32_t *)(unit + UNIT_WEAPON_INVENTORY_OFFSET + slot * 4u) ==
            weapon_datum) return slot;
    return -1;
}

/* Selectionne l'arme integree du vehicule par les memes champs runtime que
 * les vehicules Halo armes. Sans cette selection, forcer uniquement le bit du
 * WeaponDatum ne fait avancer ni le cycle de detente, ni les effets, ni le son.
 * Tout est limite a l'instance du Pelican cree par le mod. */
static int select_unit_vehicle_weapon(int32_t unit_datum, int32_t weapon_datum,
                                      int *slot_out, uint16_t *current_before_out,
                                      uint16_t *desired_before_out,
                                      uint32_t *flags_before_out) {
    uint8_t *unit = object_data(unit_datum);
    int slot = unit_weapon_slot(unit_datum, weapon_datum);
    uint16_t current_before;
    uint16_t desired_before;
    uint32_t flags_before;
    if (!unit || slot < 0 ||
        !writable_range(unit + UNIT_FLAGS_OFFSET, sizeof(uint32_t)) ||
        !writable_range(unit + UNIT_CURRENT_WEAPON_INDEX_OFFSET, sizeof(uint8_t)) ||
        !writable_range(unit + UNIT_DESIRED_WEAPON_INDEX_OFFSET, sizeof(uint8_t)))
        return 0;
    flags_before = *(uint32_t *)(unit + UNIT_FLAGS_OFFSET);
    current_before = *(uint8_t *)(unit + UNIT_CURRENT_WEAPON_INDEX_OFFSET);
    desired_before = *(uint8_t *)(unit + UNIT_DESIRED_WEAPON_INDEX_OFFSET);
    *(uint32_t *)(unit + UNIT_FLAGS_OFFSET) =
        (flags_before | UNIT_FLAG_ACTIVELY_CONTROLLED |
         UNIT_FLAG_CHANGED_AIMING_ANGLES) &
        ~UNIT_FLAG_DESIRED_WEAPON_INVALID;
    *(uint8_t *)(unit + UNIT_CURRENT_WEAPON_INDEX_OFFSET) = (uint8_t)slot;
    /* +0x33E et +0x33F sont les deux canaux d'arme du vehicule. Le moteur
     * traite le canal 0 avec bits 32..35/+0x2A0, puis le canal 1 avec
     * bits 37..40/+0x2A4. Dupliquer le slot dans les deux canaux faisait que
     * le second passage reecrivait l'etat du WeaponDatum sans detente et
     * annulait le tir primaire. Ce module n'a qu'une arme : canal 0 actif,
     * canal 1 explicitement invalide. */
    *(uint8_t *)(unit + UNIT_DESIRED_WEAPON_INDEX_OFFSET) = 0xFFu;
    if (slot_out) *slot_out = slot;
    if (current_before_out) *current_before_out = current_before;
    if (desired_before_out) *desired_before_out = desired_before;
    if (flags_before_out) *flags_before_out = flags_before;
    return 1;
}

/* Retrouve le vehicule Warthog invisible qui possede l'arme proxy audio et
 * prepare son canal 0. Aucun projectile de cette arme ne sera publie : ses
 * dependances sont neutralisees autour de chaque weapon_fire synchrone. */
static int prepare_pelican_audio_proxy(int32_t weapon_datum,
                                       int32_t *module_out) {
    uint8_t *weapon = object_data(weapon_datum);
    int32_t candidates[2] = { -1, -1 };
    int index;
    if (module_out) *module_out = -1;
    if (weapon_datum == -1 || !weapon) return 0;
    if (readable_range(weapon + WEAPON_OWNER_UNIT_OFFSET,
                       sizeof(int32_t)))
        candidates[0] = *(int32_t *)(weapon + WEAPON_OWNER_UNIT_OFFSET);
    if (readable_range(weapon + OBJECT_PARENT_OFFSET, sizeof(int32_t)))
        candidates[1] = *(int32_t *)(weapon + OBJECT_PARENT_OFFSET);
    for (index = 0; index < 2; ++index) {
        if (candidates[index] != -1 &&
            unit_weapon_slot(candidates[index], weapon_datum) >= 0 &&
            select_unit_vehicle_weapon(
                candidates[index], weapon_datum, NULL, NULL, NULL, NULL)) {
            /* attachspawn masque le vehicule proxy, mais son WeaponDatum est
             * un objet rendu separement. Sans ce second hide, la chaingun
             * Warthog reste visible sous le Pelican et donne l'impression que
             * la tourelle est dedoublee. Le datum et son acteur Unreal restent
             * vivants : seuls leurs pixels sont masques, donc le son retail
             * Chaingun_Fire et ses quatre couches continuent de fonctionner. */
            ((object_hide_fn)(g_blam + 0x5A5FD0))(
                candidates[index], 1);
            ((object_hide_fn)(g_blam + 0x5A5FD0))(weapon_datum, 1);
            if (module_out) *module_out = candidates[index];
            return 1;
        }
    }
    return 0;
}

/* Reconstitue le contexte qu'aurait cree le retail si le module ventral avait
 * ete instancie avec son conducteur. L'arme reste possedee par le module afin
 * que weapon_get_inventory_index retrouve son vrai slot. Le damage_owner du
 * module, lui, est celui du conducteur reel : joueur, equipe et attribution
 * sont ainsi recopies tels quels par projectile_new et damage_apply. */
static int bind_pelican_weapon_damage_owner(int32_t pilot_datum,
                                            int32_t module_datum,
                                            int32_t weapon_datum) {
    uint8_t owner[12];
    uint8_t resolved_owner[12];
    uint8_t *pilot = object_data(pilot_datum);
    uint8_t *module = object_data(module_datum);
    uint8_t *weapon = object_data(weapon_datum);
    int physics_kind;
    int cache_active = 0;
    if (!pilot || !module || !weapon ||
        !readable_range(pilot + UNIT_DAMAGE_OWNER_TEAM_OFFSET,
                        sizeof(uint16_t)) ||
        !writable_range(module + OBJECT_DAMAGE_OWNER_OFFSET, sizeof(owner)) ||
        !writable_range(module + UNIT_DAMAGE_OWNER_TEAM_OFFSET,
                        sizeof(uint16_t)) ||
        !writable_range(weapon + WEAPON_OWNER_UNIT_OFFSET, sizeof(int32_t)))
        return 0;
    ((damage_owner_from_object_fn)(g_blam + 0x63EA70))(
        pilot_datum, owner);
    /* Sortir puis remonter dans le meme Pelican ne change pas le proprietaire
     * des degats. Campaign Evolved peut en revanche faire pointer sa routine
     * de resolution vers le parent pendant une transition de siege. Le
     * snapshot deja verifie lors du premier ARM reste alors la reference
     * exacte et evite qu'un rebind transitoire bloque toute la detente. */
    if (g_pelican_damage_owner_pilot == pilot_datum &&
        g_pelican_damage_owner_module == module_datum &&
        InterlockedCompareExchange(
            &g_pelican_damage_owner_verified, 0, 0) != 0 &&
        memcmp(g_pelican_damage_owner_bytes, owner, sizeof(owner)) == 0) {
        int cached_physics_kind;
        /* Une sortie de siege peut vider le damage-owner du module et son
         * miroir physique tout en conservant les datums. Restaurer ici le
         * snapshot deja verifie, et pas seulement WEAPON_OWNER_UNIT. */
        memcpy(module + OBJECT_DAMAGE_OWNER_OFFSET,
               g_pelican_damage_owner_bytes, sizeof(owner));
        *(uint16_t *)(module + UNIT_DAMAGE_OWNER_TEAM_OFFSET) =
            *(const uint16_t *)(pilot + UNIT_DAMAGE_OWNER_TEAM_OFFSET);
        cached_physics_kind = ((int (__fastcall *)(int32_t))(
            g_blam + 0x624510))(module_datum);
        if (cached_physics_kind == 6 &&
            writable_range(module + 0xAACu, sizeof(owner)) &&
            writable_range(module + 0xAB8u, sizeof(uint8_t))) {
            memcpy(module + 0xAACu,
                   g_pelican_damage_owner_bytes, sizeof(owner));
            MemoryBarrier();
            module[0xAB8u] = 1u;
            InterlockedExchange(&g_pelican_damage_owner_cache_active, 1);
        } else if (cached_physics_kind == 10 &&
                   writable_range(module + 0xAB4u, sizeof(owner)) &&
                   writable_range(module + 0xAC0u, sizeof(uint8_t))) {
            memcpy(module + 0xAB4u,
                   g_pelican_damage_owner_bytes, sizeof(owner));
            MemoryBarrier();
            module[0xAC0u] = 1u;
            InterlockedExchange(&g_pelican_damage_owner_cache_active, 1);
        }
        InterlockedExchange(
            &g_pelican_damage_owner_kind, cached_physics_kind);
        *(int32_t *)(weapon + WEAPON_OWNER_UNIT_OFFSET) = module_datum;
        MemoryBarrier();
        return 1;
    }
    memcpy(module + OBJECT_DAMAGE_OWNER_OFFSET, owner, sizeof(owner));
    *(uint16_t *)(module + UNIT_DAMAGE_OWNER_TEAM_OFFSET) =
        *(const uint16_t *)(pilot + UNIT_DAMAGE_OWNER_TEAM_OFFSET);

    /* damage_owner_from_object appelle unit_get_cached_damage_owner pour les
     * vehicules physiques de familles 6/10 avant de suivre le conducteur ou
     * +0xF0. Un module cree sans pilote possede le bloc, mais son indicateur
     * reste inactif. Initialiser le bloc puis publier son indicateur reproduit
     * exactement l'etat retail cree par une tourelle deja occupee. */
    physics_kind = ((int (__fastcall *)(int32_t))(g_blam + 0x624510))(
        module_datum);
    if (physics_kind == 6) {
        if (!writable_range(module + 0xAACu, sizeof(owner)) ||
            !writable_range(module + 0xAB8u, sizeof(uint8_t))) return 0;
        memcpy(module + 0xAACu, owner, sizeof(owner));
        MemoryBarrier();
        module[0xAB8u] = 1u;
        cache_active = 1;
    } else if (physics_kind == 10) {
        if (!writable_range(module + 0xAB4u, sizeof(owner)) ||
            !writable_range(module + 0xAC0u, sizeof(uint8_t))) return 0;
        memcpy(module + 0xAB4u, owner, sizeof(owner));
        MemoryBarrier();
        module[0xAC0u] = 1u;
        cache_active = 1;
    }
    ((damage_owner_from_object_fn)(g_blam + 0x63EA70))(
        module_datum, resolved_owner);
    InterlockedExchange(&g_pelican_damage_owner_kind, physics_kind);
    InterlockedExchange(&g_pelican_damage_owner_cache_active, cache_active);
    InterlockedExchange(&g_pelican_damage_owner_verified,
                        memcmp(resolved_owner, owner, sizeof(owner)) == 0);
    if (memcmp(resolved_owner, owner, sizeof(owner)) != 0) return 0;
    g_pelican_damage_owner_pilot = pilot_datum;
    g_pelican_damage_owner_module = module_datum;
    memcpy(g_pelican_damage_owner_bytes, owner, sizeof(owner));
    *(int32_t *)(weapon + WEAPON_OWNER_UNIT_OFFSET) = module_datum;
    return 1;
}

struct pelican_suppressed_projectile_refs {
    uint8_t *barrel;
    DWORD old_protect;
    int32_t primary;
    int32_t secondary;
    int32_t firing_damage;
    int32_t crate;
    int active;
};

static void pelican_projectile_probe_reset(void) {
    memset(&g_pelican_projectile_probe, 0,
           sizeof(g_pelican_projectile_probe));
    g_pelican_projectile_probe.last_datum = -1;
    g_pelican_projectile_probe.last_tag = -1;
    g_pelican_projectile_probe.last_impact_damage = -1;
    g_pelican_projectile_probe.last_weapon = -1;
    g_pelican_projectile_probe.last_parent = -1;
    g_pelican_projectile_probe.last_source_root = -1;
    g_pelican_projectile_probe.last_barrel = 0xFFFFu;
}

/* Conserve weapon_fire et toute sa presentation, mais neutralise uniquement
 * les quatre dependances qui pourraient creer un second objet/deuxieme degat.
 * La fenetre n'existe que pendant l'appel synchrone sur game_MAIN_THREAD. */
static int pelican_suppress_native_projectile_begin(
        int32_t weapon_datum,
        struct pelican_suppressed_projectile_refs *saved) {
    uint8_t *weapon = object_data(weapon_datum);
    uint8_t *barrel;
    DWORD ignored_protect = 0;
    if (!saved) return 0;
    memset(saved, 0, sizeof(*saved));
    if (!weapon || !readable_range(weapon, sizeof(int32_t))) return 0;
    barrel = pelican_primary_barrel_for_weapon_tag(*(int32_t *)weapon);
    if (!barrel || !readable_range(
            barrel + WEAPON_BARREL_PROJECTILE_TAG_OFFSET,
            WEAPON_BARREL_CRATE_TAG_OFFSET + sizeof(int32_t) -
                WEAPON_BARREL_PROJECTILE_TAG_OFFSET)) return 0;
    saved->barrel = barrel;
    saved->primary = *(int32_t *)(
        barrel + WEAPON_BARREL_PROJECTILE_TAG_OFFSET);
    saved->secondary = *(int32_t *)(
        barrel + WEAPON_BARREL_SECONDARY_TAG_OFFSET);
    saved->firing_damage = *(int32_t *)(
        barrel + WEAPON_BARREL_DAMAGE_TAG_OFFSET);
    saved->crate = *(int32_t *)(
        barrel + WEAPON_BARREL_CRATE_TAG_OFFSET);
    if (saved->primary == -1 ||
        !((tag_get_fn)(g_blam + 0x201480))(
            TAG_GROUP_PROJECTILE, saved->primary) ||
        !VirtualProtect(
            barrel + WEAPON_BARREL_PROJECTILE_TAG_OFFSET,
            WEAPON_BARREL_CRATE_TAG_OFFSET + sizeof(int32_t) -
                WEAPON_BARREL_PROJECTILE_TAG_OFFSET,
            PAGE_READWRITE, &saved->old_protect)) return 0;
    InterlockedExchange((volatile LONG *)(
        barrel + WEAPON_BARREL_PROJECTILE_TAG_OFFSET), -1);
    InterlockedExchange((volatile LONG *)(
        barrel + WEAPON_BARREL_SECONDARY_TAG_OFFSET), -1);
    InterlockedExchange((volatile LONG *)(
        barrel + WEAPON_BARREL_DAMAGE_TAG_OFFSET), -1);
    InterlockedExchange((volatile LONG *)(
        barrel + WEAPON_BARREL_CRATE_TAG_OFFSET), -1);
    MemoryBarrier();
    if (*(int32_t *)(barrel + WEAPON_BARREL_PROJECTILE_TAG_OFFSET) != -1 ||
        *(int32_t *)(barrel + WEAPON_BARREL_SECONDARY_TAG_OFFSET) != -1 ||
        *(int32_t *)(barrel + WEAPON_BARREL_DAMAGE_TAG_OFFSET) != -1 ||
        *(int32_t *)(barrel + WEAPON_BARREL_CRATE_TAG_OFFSET) != -1) {
        InterlockedExchange((volatile LONG *)(
            barrel + WEAPON_BARREL_PROJECTILE_TAG_OFFSET), saved->primary);
        InterlockedExchange((volatile LONG *)(
            barrel + WEAPON_BARREL_SECONDARY_TAG_OFFSET), saved->secondary);
        InterlockedExchange((volatile LONG *)(
            barrel + WEAPON_BARREL_DAMAGE_TAG_OFFSET), saved->firing_damage);
        InterlockedExchange((volatile LONG *)(
            barrel + WEAPON_BARREL_CRATE_TAG_OFFSET), saved->crate);
        VirtualProtect(
            barrel + WEAPON_BARREL_PROJECTILE_TAG_OFFSET,
            WEAPON_BARREL_CRATE_TAG_OFFSET + sizeof(int32_t) -
                WEAPON_BARREL_PROJECTILE_TAG_OFFSET,
            saved->old_protect, &ignored_protect);
        return 0;
    }
    saved->active = 1;
    return 1;
}

static int pelican_suppress_native_projectile_end(
        struct pelican_suppressed_projectile_refs *saved) {
    uint8_t *barrel;
    DWORD ignored_protect = 0;
    int restored = 1;
    int protection_restored;
    if (!saved || !saved->active || !saved->barrel) return 0;
    barrel = saved->barrel;
    if (InterlockedCompareExchange((volatile LONG *)(
            barrel + WEAPON_BARREL_PROJECTILE_TAG_OFFSET),
            saved->primary, -1) != -1) restored = 0;
    if (InterlockedCompareExchange((volatile LONG *)(
            barrel + WEAPON_BARREL_SECONDARY_TAG_OFFSET),
            saved->secondary, -1) != -1) restored = 0;
    if (InterlockedCompareExchange((volatile LONG *)(
            barrel + WEAPON_BARREL_DAMAGE_TAG_OFFSET),
            saved->firing_damage, -1) != -1) restored = 0;
    if (InterlockedCompareExchange((volatile LONG *)(
            barrel + WEAPON_BARREL_CRATE_TAG_OFFSET),
            saved->crate, -1) != -1) restored = 0;
    MemoryBarrier();
    protection_restored = VirtualProtect(
        barrel + WEAPON_BARREL_PROJECTILE_TAG_OFFSET,
        WEAPON_BARREL_CRATE_TAG_OFFSET + sizeof(int32_t) -
            WEAPON_BARREL_PROJECTILE_TAG_OFFSET,
        saved->old_protect, &ignored_protect) != 0;
    saved->active = 0;
    return restored && protection_restored &&
        readable_range(barrel + WEAPON_BARREL_PROJECTILE_TAG_OFFSET,
                       sizeof(int32_t)) &&
        *(int32_t *)(barrel + WEAPON_BARREL_PROJECTILE_TAG_OFFSET) ==
            saved->primary;
}

/* Le vrai son Chaingun_Fire est emis par l'acteur Unreal de l'arme Warthog.
 * Le relais Blam direct ne garantit pas la publication des delegates UE de ce
 * Blueprint. On publie donc uniquement les fronts de rafale vers Lua; celui-ci
 * appelle les deux UFunctions audio retail sur l'acteur lie au datum proxy. */
static int pelican_emit_audio_proxy_shot(void) {
    if (g_pelican_audio_weapon == -1 || g_pelican_audio_module == -1 ||
        !object_data(g_pelican_audio_weapon) ||
        !object_data(g_pelican_audio_module)) {
        ++g_pelican_audio_fire_failures;
        return 0;
    }
    if (!pelican_publish_audio_signal(1)) {
        ++g_pelican_audio_fire_failures;
        return 0;
    }
    return 1;
}

static void pelican_stop_audio_proxy(void) {
    (void)pelican_publish_audio_signal(0);
}

static int pelican_vector_normalize(float vector[3]) {
    float length_sq;
    float inverse;
    if (!vector || !isfinite(vector[0]) || !isfinite(vector[1]) ||
        !isfinite(vector[2])) return 0;
    length_sq = vector[0] * vector[0] + vector[1] * vector[1] +
        vector[2] * vector[2];
    /* Les directions camera->cible et bouche->cible peuvent mesurer 2048 m
     * avant normalisation. L'ancienne limite (1000 m au carre) rejetait donc
     * chaque tir avant object_new. Conserver une borne monde explicite, mais
     * assez large pour les rayons authored de Campaign Evolved. */
    if (!isfinite(length_sq) || length_sq < 0.000001f ||
        length_sq > 1000000000000.0f) return 0;
    inverse = 1.0f / sqrtf(length_sq);
    vector[0] *= inverse;
    vector[1] *= inverse;
    vector[2] *= inverse;
    return 1;
}

/* Ordre restaure depuis le premier tir direct fonctionnel (build du 17:11) :
 * le projectile lisait d'abord le current aiming vector +0x1E0 du Pelican,
 * puis seulement les vecteurs desired/looking en repli. Cet ordre est celui
 * qui faisait coincider le tir avec le reticule dans la version de reference. */
static int pelican_unit_reticle_vector(int32_t unit_datum, float aim[3]) {
    static const uint32_t offsets[] = {
        UNIT_AIMING_VECTOR_OFFSET,
        UNIT_DESIRED_AIMING_VECTOR_OFFSET,
        UNIT_LOOKING_VECTOR_OFFSET,
        UNIT_DESIRED_LOOKING_VECTOR_OFFSET
    };
    uint8_t *unit = object_data(unit_datum);
    size_t index;
    float fallback_up[3];
    if (!unit || !aim) return 0;
    for (index = 0; index < sizeof(offsets) / sizeof(offsets[0]); ++index) {
        if (!readable_range(unit + offsets[index], 3u * sizeof(float)))
            continue;
        memcpy(aim, unit + offsets[index], 3u * sizeof(float));
        if (pelican_vector_normalize(aim)) return 1;
    }
    ((object_get_orientation_fn)(g_blam + 0x5A6AE0))(
        unit_datum, aim, fallback_up);
    return pelican_vector_normalize(aim);
}

/* Contexte retail consomme par weapon_fire_pending (0x5BE600). Le moteur
 * cherche l'entree du barrel, resout son marqueur de bouche, puis converge
 * lui-meme vers target_point avant projectile_new. Garder ce layout exact est
 * ce qui preserve le projectile, les degats, la trainee et tous les FX Halo. */
struct blam_fire_target {
    int32_t barrel;
    uint8_t target_valid;
    uint8_t disable_convergence;
    uint8_t pad06[2];
    float target_point[3];
    int32_t target_object;
    uint8_t pad18[4];
};

struct blam_fire_context {
    uint8_t header[8];
    struct blam_fire_target targets[4];
    int32_t target_count;
};

_Static_assert(sizeof(struct blam_fire_target) == 0x1Cu,
               "blam_fire_target layout");
_Static_assert(sizeof(struct blam_fire_context) == 0x7Cu,
               "blam_fire_context layout");

static void pelican_fill_fire_context(
        struct blam_fire_context *context, const float target_point[3]) {
    memset(context, 0, sizeof(*context));
    context->targets[0].barrel = 0;
    context->targets[0].target_valid = 1u;
    context->targets[0].disable_convergence = 0u;
    memcpy(context->targets[0].target_point,
           target_point, sizeof(context->targets[0].target_point));
    context->targets[0].target_object = -1;
    context->target_count = 1;
}

/* Convertit le point local publie par la camera Unreal vers le monde Blam du
 * Pelican au DERNIER instant. Cette reconstruction locale supprime le
 * parallaxe camera/bouche meme si le Pelican translate, tangue ou roule entre
 * deux echantillons. En cas de camera momentanement indisponible, on conserve
 * un rayon de secours loin devant l'aim moteur sans interrompre le canon. */
static int pelican_resolve_camera_target(
        int32_t vehicle_datum, int32_t module_datum,
        float target_point[3]) {
    float local_target[3];
    float vehicle_origin[3];
    float module_origin[3];
    float forward[3];
    float up[3];
    float right[3];
    ULONGLONG updated_at = 0;
    ULONGLONG now = GetTickCount64();
    int32_t bridged_vehicle = -1;
    int bridged = 0;
    int attempt;
    int index;

    for (attempt = 0; attempt < 4; ++attempt) {
        LONG sequence_before = InterlockedCompareExchange(
            &g_pelican_camera_target.sequence, 0, 0);
        LONG sequence_after;
        if (sequence_before & 1) continue;
        MemoryBarrier();
        bridged_vehicle = g_pelican_camera_target.vehicle;
        memcpy(local_target, g_pelican_camera_target.local_target,
               sizeof(local_target));
        updated_at = g_pelican_camera_target.updated_at_ms;
        MemoryBarrier();
        sequence_after = InterlockedCompareExchange(
            &g_pelican_camera_target.sequence, 0, 0);
        if (sequence_before == sequence_after && !(sequence_after & 1)) {
            bridged = InterlockedCompareExchange(
                &g_pelican_camera_target.valid, 0, 0) != 0;
            break;
        }
    }

    if (bridged && bridged_vehicle == vehicle_datum && updated_at <= now &&
        now - updated_at <= 250u &&
        isfinite(local_target[0]) && isfinite(local_target[1]) &&
        isfinite(local_target[2]) &&
        fabsf(local_target[0]) <= 10000.0f &&
        fabsf(local_target[1]) <= 10000.0f &&
        fabsf(local_target[2]) <= 10000.0f) {
        ((object_get_origin_fn)(g_blam + 0x5A6780))(
            vehicle_datum, vehicle_origin);
        ((object_get_origin_fn)(g_blam + 0x5A6780))(
            module_datum, module_origin);
        ((object_get_orientation_fn)(g_blam + 0x5A6AE0))(
            vehicle_datum, forward, up);
        if (pelican_vector_normalize(forward) &&
            pelican_vector_normalize(up)) {
            /* Repere Halo : avec forward=(1,0,0), up=(0,0,1), le cote
             * droit vaut (0,1,0), donc right = up x forward. */
            right[0] = up[1] * forward[2] - up[2] * forward[1];
            right[1] = up[2] * forward[0] - up[0] * forward[2];
            right[2] = up[0] * forward[1] - up[1] * forward[0];
            if (pelican_vector_normalize(right)) {
                float distance_sq = 0.0f;
                for (index = 0; index < 3; ++index) {
                    float delta;
                    target_point[index] = vehicle_origin[index] +
                        forward[index] * local_target[0] +
                        right[index] * local_target[1] +
                        up[index] * local_target[2];
                    delta = target_point[index] - module_origin[index];
                    distance_sq += delta * delta;
                }
                /* Lua publie deja le point de collision du rayon de controle
                 * qui pilote le reticule HUD. Le reprojeter sur le desired-aim
                 * Blam remplacait ce rayon exact par un vecteur interpole de la
                 * coque : a courte distance le tir tombait sous le reticule et
                 * pendant un mouvement il restait aussi en retard lateral.
                 * Conserver le point reconstruit tel quel; weapon_fire_pending
                 * calcule ensuite l'unique convergence bouche -> collision. */
                if (isfinite(distance_sq) && distance_sq >= 0.25f &&
                    distance_sq <= 100000000.0f) {
                    memcpy(g_pelican_last_target_point, target_point,
                           sizeof(g_pelican_last_target_point));
                    InterlockedExchange(
                        &g_pelican_last_target_from_camera, 1);
                    return 1;
                }
            }
        }
    }

    ((object_get_origin_fn)(g_blam + 0x5A6780))(
        module_datum, module_origin);
    if (!pelican_unit_reticle_vector(vehicle_datum, forward)) return 0;
    for (index = 0; index < 3; ++index)
        target_point[index] = module_origin[index] + forward[index] * 2000.0f;
    memcpy(g_pelican_last_target_point, target_point,
           sizeof(g_pelican_last_target_point));
    InterlockedExchange(&g_pelican_last_target_from_camera, 0);
    return 1;
}

/* Reproduction exacte de la convergence du premier projectile Pelican qui
 * visait correctement : point a 2048 m depuis l'origine du pilote, sur le
 * current aiming vector du vrai Pelican. Le contexte retail fait ensuite la
 * meme soustraction bouche->point que l'ancien spawn direct, tout en gardant
 * aujourd'hui ses degats, ses FX et sa gestion d'impact fiables. */
static int pelican_resolve_original_reticle_target(
        int32_t pilot_datum, int32_t vehicle_datum, int32_t module_datum,
        float target_point[3]) {
    float pilot_origin[3];
    float module_origin[3];
    float aim[3];
    int index;

    if (!object_data(pilot_datum) || !object_data(vehicle_datum) ||
        !object_data(module_datum) || !target_point)
        return pelican_resolve_camera_target(
            vehicle_datum, module_datum, target_point);
    ((object_get_origin_fn)(g_blam + 0x5A6780))(
        pilot_datum, pilot_origin);
    ((object_get_origin_fn)(g_blam + 0x5A6780))(
        module_datum, module_origin);
    if (!pelican_unit_reticle_vector(vehicle_datum, aim) &&
        !pelican_unit_reticle_vector(pilot_datum, aim))
        return pelican_resolve_camera_target(
            vehicle_datum, module_datum, target_point);
    for (index = 0; index < 3; ++index) {
        if (!isfinite(pilot_origin[index]) ||
            !isfinite(module_origin[index]) || !isfinite(aim[index]))
            return pelican_resolve_camera_target(
                vehicle_datum, module_datum, target_point);
        target_point[index] = pilot_origin[index] + aim[index] * 2048.0f;
    }
    memcpy(g_pelican_last_target_point, target_point,
           sizeof(g_pelican_last_target_point));
    InterlockedExchange(&g_pelican_last_target_from_camera, 0);
    return 1;
}

/* Pipeline de tir principal: on force d'abord la cible reconstruite depuis le
 * pont UE4SS quand elle est valide, puis on retombe sur le vecteur retail
 * "original_reticle" uniquement si la donnée camera manque/est stale. */
static int pelican_resolve_effective_reticle_target(
        int32_t pilot_datum, int32_t vehicle_datum, int32_t module_datum,
        float target_point[3]) {
    if (pelican_resolve_camera_target(vehicle_datum, module_datum, target_point))
        return 1;
    return pelican_resolve_original_reticle_target(
        pilot_datum, vehicle_datum, module_datum, target_point);
}

static int pelican_projectile_up_vector(
        const float forward[3], float up[3]) {
    float reference[3] = {0.0f, 0.0f, 1.0f};
    float right[3];
    if (!forward || !up) return 0;
    if (fabsf(forward[2]) > 0.95f) {
        reference[1] = 1.0f;
        reference[2] = 0.0f;
    }
    right[0] = reference[1] * forward[2] -
        reference[2] * forward[1];
    right[1] = reference[2] * forward[0] -
        reference[0] * forward[2];
    right[2] = reference[0] * forward[1] -
        reference[1] * forward[0];
    if (!pelican_vector_normalize(right)) return 0;
    up[0] = forward[1] * right[2] - forward[2] * right[1];
    up[1] = forward[2] * right[0] - forward[0] * right[2];
    up[2] = forward[0] * right[1] - forward[1] * right[0];
    return pelican_vector_normalize(up);
}

static int32_t pelican_spawn_reticle_projectile(
        int32_t pilot_datum, int32_t vehicle_datum,
        int32_t module_datum, int32_t weapon_datum,
        int32_t projectile_tag, const float target_point[3]) {
    _Alignas(16) uint8_t placement[0x330];
    uint8_t owner[12];
    uint8_t *pilot = object_data(pilot_datum);
    uint8_t *vehicle = object_data(vehicle_datum);
    uint8_t *module = object_data(module_datum);
    uint8_t *projectile_definition;
    uint8_t *projectile;
    float module_origin[3];
    float direction[3];
    float up[3];
    float inherited_velocity[3] = {0.0f, 0.0f, 0.0f};
    int32_t projectile_datum;
    int32_t impact_damage;
    int index;

    ++g_pelican_projectile_probe.attempts;
    g_pelican_projectile_probe.failure_stage = 1u;
    g_pelican_projectile_probe.placement_variant = 0u;
    g_pelican_projectile_probe.last_tag = projectile_tag;
    g_pelican_projectile_probe.last_weapon = weapon_datum;
    if (!pilot || !vehicle || !module || !object_data(weapon_datum) ||
        projectile_tag == -1 || !target_point) return -1;
    g_pelican_projectile_probe.failure_stage = 2u;
    projectile_definition = (uint8_t *)((tag_get_fn)(
        g_blam + 0x201480))(TAG_GROUP_PROJECTILE, projectile_tag);
    if (!projectile_definition) return -1;
    g_pelican_projectile_probe.failure_stage = 3u;
    if (!readable_range(
            projectile_definition + PROJECTILE_IMPACT_DAMAGE_TAG_OFFSET,
            sizeof(int32_t))) return -1;
    impact_damage = *(int32_t *)(
        projectile_definition + PROJECTILE_IMPACT_DAMAGE_TAG_OFFSET);
    ((damage_owner_from_object_fn)(g_blam + 0x63EA70))(
        pilot_datum, owner);
    ((object_get_origin_fn)(g_blam + 0x5A6780))(
        module_datum, module_origin);
    for (index = 0; index < 3; ++index) {
        if (!isfinite(module_origin[index]) ||
            !isfinite(target_point[index])) return -1;
        direction[index] = target_point[index] - module_origin[index];
    }
    g_pelican_projectile_probe.failure_stage = 4u;
    if (!pelican_vector_normalize(direction)) return -1;
    memcpy(g_pelican_projectile_probe.aim, direction, sizeof(direction));
    for (index = 0; index < 3; ++index) {
        if (!isfinite(module_origin[index])) return -1;
        /* Sortir le projectile de la collision du module sans modifier son
         * rayon : la direction reste exactement celle du reticule. */
        module_origin[index] += direction[index] * 0.85f;
    }
    g_pelican_projectile_probe.failure_stage = 5u;
    if (!pelican_projectile_up_vector(direction, up)) return -1;
    if (readable_range(vehicle + OBJECT_LINEAR_VELOCITY_OFFSET,
                       sizeof(inherited_velocity))) {
        memcpy(inherited_velocity,
               vehicle + OBJECT_LINEAR_VELOCITY_OFFSET,
               sizeof(inherited_velocity));
        for (index = 0; index < 3; ++index)
            if (!isfinite(inherited_velocity[index]) ||
                fabsf(inherited_velocity[index]) > 10000.0f)
                inherited_velocity[index] = 0.0f;
    }
    ((placement_init_fn)(g_blam + 0x5EE570))(
        placement, projectile_tag, module_datum, owner);
    *(uint32_t *)(placement + 0x18) |= 0x400u;
    memcpy(placement + 0x1C, module_origin, 3u * sizeof(float));
    memcpy(placement + 0x28, direction, 3u * sizeof(float));
    memcpy(placement + 0x34, up, 3u * sizeof(float));
    /* object_new ajoute la vitesse initiale authored le long du forward. */
    memcpy(placement + 0x40, inherited_velocity,
           sizeof(inherited_velocity));
    g_pelican_projectile_probe.failure_stage = 6u;
    g_pelican_projectile_probe.placement_variant = 1u;
    projectile_datum = ((object_new_fn)(g_blam + 0x5A0FC0))(placement);
    if (projectile_datum == -1) {
        /* Tous les placements generiques valides de cette build passent -1
         * comme source. Ne tenter cette variante qu'apres le refus atomique
         * de la variante module : aucun double projectile n'est possible. */
        ((placement_init_fn)(g_blam + 0x5EE570))(
            placement, projectile_tag, -1, owner);
        *(uint32_t *)(placement + 0x18) |= 0x400u;
        memcpy(placement + 0x1C, module_origin, 3u * sizeof(float));
        memcpy(placement + 0x28, direction, 3u * sizeof(float));
        memcpy(placement + 0x34, up, 3u * sizeof(float));
        memcpy(placement + 0x40, inherited_velocity,
               sizeof(inherited_velocity));
        g_pelican_projectile_probe.placement_variant = 2u;
        projectile_datum = ((object_new_fn)(
            g_blam + 0x5A0FC0))(placement);
    }
    projectile = object_data(projectile_datum);
    if (projectile_datum == -1 || !projectile ||
        !readable_range(projectile, sizeof(int32_t)) ||
        *(int32_t *)projectile != projectile_tag) {
        if (projectile_datum != -1 && projectile)
            ((object_delete_fn)(g_blam + 0x5A2090))(projectile_datum);
        return -1;
    }
    g_pelican_projectile_probe.failure_stage = 7u;
    if (!writable_range(projectile + PROJECTILE_WEAPON_OFFSET,
                        sizeof(int32_t)) ||
        !writable_range(projectile + PROJECTILE_BARREL_OFFSET,
                        sizeof(uint16_t))) {
        ((object_delete_fn)(g_blam + 0x5A2090))(projectile_datum);
        return -1;
    }
    *(int32_t *)(projectile + PROJECTILE_WEAPON_OFFSET) = weapon_datum;
    *(uint16_t *)(projectile + PROJECTILE_BARREL_OFFSET) = 0u;
    MemoryBarrier();
    g_pelican_projectile_probe.failure_stage = 8u;
    if (*(int32_t *)(projectile + PROJECTILE_WEAPON_OFFSET) != weapon_datum ||
        *(uint16_t *)(projectile + PROJECTILE_BARREL_OFFSET) != 0u) {
        ((object_delete_fn)(g_blam + 0x5A2090))(projectile_datum);
        return -1;
    }

    /* Conserver exactement la norme de vitesse creee par le tag, mais retirer
     * toute composante laterale heritee du vehicule. Sans cette projection, un
     * Pelican en translation faisait partir la balle a cote du rayon affiche
     * meme si son orientation etait deja celle du reticule. */
    if (writable_range(projectile + OBJECT_LINEAR_VELOCITY_OFFSET,
                       3u * sizeof(float))) {
        float current_velocity[3];
        float speed_sq;
        float speed;
        memcpy(current_velocity,
               projectile + OBJECT_LINEAR_VELOCITY_OFFSET,
               sizeof(current_velocity));
        speed_sq = current_velocity[0] * current_velocity[0] +
            current_velocity[1] * current_velocity[1] +
            current_velocity[2] * current_velocity[2];
        if (isfinite(speed_sq) && speed_sq > 0.000001f) {
            speed = sqrtf(speed_sq);
            g_pelican_projectile_probe.authored_speed = speed;
            for (index = 0; index < 3; ++index)
                current_velocity[index] = direction[index] * speed;
            memcpy(projectile + OBJECT_LINEAR_VELOCITY_OFFSET,
                   current_velocity, sizeof(current_velocity));
            MemoryBarrier();
        }
    }

    ++g_pelican_projectile_probe.created;
    g_pelican_projectile_probe.failure_stage = 0u;
    g_pelican_projectile_probe.last_datum = projectile_datum;
    g_pelican_projectile_probe.last_tag = projectile_tag;
    g_pelican_projectile_probe.last_impact_damage = impact_damage;
    g_pelican_projectile_probe.last_weapon = weapon_datum;
    g_pelican_projectile_probe.last_parent =
        readable_range(projectile + OBJECT_PARENT_OFFSET, sizeof(int32_t))
            ? *(int32_t *)(projectile + OBJECT_PARENT_OFFSET) : -1;
    g_pelican_projectile_probe.last_source_root =
        readable_range(projectile + PROJECTILE_SOURCE_ROOT_OFFSET,
                       sizeof(int32_t))
            ? *(int32_t *)(projectile + PROJECTILE_SOURCE_ROOT_OFFSET) : -1;
    g_pelican_projectile_probe.last_barrel =
        *(uint16_t *)(projectile + PROJECTILE_BARREL_OFFSET);
    g_pelican_projectile_probe.owner_match =
        readable_range(projectile + OBJECT_DAMAGE_OWNER_OFFSET,
                       sizeof(owner)) &&
        memcmp(projectile + OBJECT_DAMAGE_OWNER_OFFSET,
               owner, sizeof(owner)) == 0;
    ((object_get_origin_fn)(g_blam + 0x5A6780))(
        projectile_datum, g_pelican_projectile_probe.position);
    {
        float ignored_up[3];
        ((object_get_orientation_fn)(g_blam + 0x5A6AE0))(
            projectile_datum, g_pelican_projectile_probe.forward,
            ignored_up);
    }
    if (readable_range(projectile + OBJECT_LINEAR_VELOCITY_OFFSET,
                       sizeof(g_pelican_projectile_probe.velocity)))
        memcpy(g_pelican_projectile_probe.velocity,
               projectile + OBJECT_LINEAR_VELOCITY_OFFSET,
               sizeof(g_pelican_projectile_probe.velocity));
    {
        float speed_sq =
            g_pelican_projectile_probe.velocity[0] *
                g_pelican_projectile_probe.velocity[0] +
            g_pelican_projectile_probe.velocity[1] *
                g_pelican_projectile_probe.velocity[1] +
            g_pelican_projectile_probe.velocity[2] *
                g_pelican_projectile_probe.velocity[2];
        if (isfinite(speed_sq) && speed_sq >= 0.0f)
            g_pelican_projectile_probe.final_speed = sqrtf(speed_sq);
    }
    return projectile_datum;
}

/* Copie la visee deja resolue par unit_control_data. Quand le joueur conduit,
 * le moteur ecrit le reticule/camera sur le vrai Pelican controle, pas sur le
 * biped assis (dont +0x1D4 reste stale). Le bloc +0x250 et le scalaire +0x274
 * sont les auxiliaires de controle utilises par le yaw/pitch des tourelles. */
static int sync_unit_vehicle_aim(int32_t source_datum,
                                 int32_t target_datum) {
    uint8_t *source = object_data(source_datum);
    uint8_t *target = object_data(target_datum);
    uint64_t target_control;
    if (!source || !target ||
        !readable_range(source + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
                        UNIT_AIM_VECTOR_BLOCK_SIZE) ||
        !readable_range(source + UNIT_CONTROL_AIM_VECTOR_OFFSET,
                        UNIT_CONTROL_AIM_VECTOR_SIZE) ||
        !readable_range(source + UNIT_CONTROL_AIM_SCALAR_OFFSET,
                        sizeof(uint32_t)) ||
        !writable_range(target + UNIT_FLAGS_OFFSET, sizeof(uint32_t)) ||
        !writable_range(target + UNIT_CONTROL_FLAGS_OFFSET, sizeof(uint64_t)) ||
        !writable_range(target + UNIT_VEHICLE_PRIMARY_TRIGGER_OFFSET, sizeof(float)) ||
        !writable_range(target + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
                        UNIT_AIM_VECTOR_BLOCK_SIZE) ||
        !writable_range(target + UNIT_CONTROL_AIM_VECTOR_OFFSET,
                        UNIT_CONTROL_AIM_VECTOR_SIZE) ||
        !writable_range(target + UNIT_CONTROL_AIM_SCALAR_OFFSET,
                        sizeof(uint32_t)))
        return 0;
    memcpy(target + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
           source + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
           UNIT_AIM_VECTOR_BLOCK_SIZE);
    memcpy(target + UNIT_CONTROL_AIM_VECTOR_OFFSET,
           source + UNIT_CONTROL_AIM_VECTOR_OFFSET,
           UNIT_CONTROL_AIM_VECTOR_SIZE);
    memcpy(target + UNIT_CONTROL_AIM_SCALAR_OFFSET,
           source + UNIT_CONTROL_AIM_SCALAR_OFFSET,
           sizeof(uint32_t));
    target_control = *(uint64_t *)(target + UNIT_CONTROL_FLAGS_OFFSET);
    target_control &= ~UNIT_CONTROL_VEHICLE_PRIMARY_BIT;
    *(uint64_t *)(target + UNIT_CONTROL_FLAGS_OFFSET) = target_control;
    *(float *)(target + UNIT_VEHICLE_PRIMARY_TRIGGER_OFFSET) = 0.0f;
    /* Ce bit demande au moteur de recalculer les ecrans de visee Euler et les
     * noeuds yaw/pitch. Sans lui, les projectiles suivent la visee mais le
     * maillage de la tourelle reste visuellement fixe. */
    *(uint32_t *)(target + UNIT_FLAGS_OFFSET) |=
        UNIT_FLAG_ACTIVELY_CONTROLLED | UNIT_FLAG_CHANGED_AIMING_ANGLES;
    ((weapon_update_fn)(g_blam + 0x59F460))(target_datum);
    return 1;
}

/* A courte distance, copier seulement le rayon camera rend l'axe du canon
 * parallele au reticule : l'ecart vertical entre cockpit et bouche devient
 * alors visible. Une fois le point camera reconstruit, orienter les vecteurs
 * de visee du module depuis l'origine de sa vraie WeaponDatum vers ce point.
 * Le fire_context retail garde ensuite la convergence exacte du marqueur de
 * bouche et tout son pipeline projectile/FX/degats. */
static int sync_unit_vehicle_aim_to_target(
        int32_t source_datum, int32_t target_datum,
        int32_t weapon_datum, const float target_point[3]) {
    static const uint32_t vector_offsets[] = {
        UNIT_DESIRED_AIMING_VECTOR_OFFSET,
        UNIT_AIMING_VECTOR_OFFSET,
        UNIT_DESIRED_LOOKING_VECTOR_OFFSET,
        UNIT_LOOKING_VECTOR_OFFSET,
        UNIT_CONTROL_AIM_VECTOR_OFFSET
    };
    uint8_t *target;
    float origin[3];
    float direction[3];
    size_t index;
    if (!target_point || !sync_unit_vehicle_aim(
            source_datum, target_datum)) return 0;
    target = object_data(target_datum);
    if (!target) return 0;
    if (object_data(weapon_datum))
        ((object_get_origin_fn)(g_blam + 0x5A6780))(
            weapon_datum, origin);
    else
        ((object_get_origin_fn)(g_blam + 0x5A6780))(
            target_datum, origin);
    for (index = 0; index < 3u; ++index) {
        if (!isfinite(origin[index]) || !isfinite(target_point[index]))
            return 0;
        direction[index] = target_point[index] - origin[index];
    }
    if (!pelican_vector_normalize(direction)) return 0;
    for (index = 0; index < sizeof(vector_offsets) /
                            sizeof(vector_offsets[0]); ++index) {
        uint8_t *slot = target + vector_offsets[index];
        if (!writable_range(slot, sizeof(direction))) return 0;
        memcpy(slot, direction, sizeof(direction));
    }
    *(uint32_t *)(target + UNIT_FLAGS_OFFSET) |=
        UNIT_FLAG_ACTIVELY_CONTROLLED | UNIT_FLAG_CHANGED_AIMING_ANGLES;
    MemoryBarrier();
    ((weapon_update_fn)(g_blam + 0x59F460))(target_datum);
    return 1;
}

/* Alimente la vraie entree vehicule du canal 0. La boucle de simulation Halo
 * resout ensuite l'arme par son owner/slot d'inventaire et cree elle-meme le
 * projectile, les degats et la presentation. Appeler weapon_fire directement
 * sur une arme dont object+0x14 vaut -1 ne suffit pas : le compteur de serie
 * avance, mais aucun projectile exploitable n'est emis. */
static int set_unit_vehicle_primary_control(int32_t pilot_datum,
                                            int32_t aim_source_datum,
                                            int32_t target_datum,
                                            int enabled) {
    uint8_t *pilot = object_data(pilot_datum);
    uint8_t *aim_source = object_data(aim_source_datum);
    uint8_t *target = object_data(target_datum);
    uint64_t pilot_control;
    uint64_t vehicle_control;
    float trigger;
    int requested;
    if (!pilot || !aim_source || !target ||
        !readable_range(pilot + UNIT_CONTROL_FLAGS_OFFSET, sizeof(uint64_t)) ||
        !readable_range(pilot + UNIT_DRIVER_PRIMARY_TRIGGER_OFFSET, sizeof(float)) ||
        !readable_range(aim_source + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
                        UNIT_AIM_VECTOR_BLOCK_SIZE) ||
        !readable_range(aim_source + UNIT_CONTROL_AIM_VECTOR_OFFSET,
                        UNIT_CONTROL_AIM_VECTOR_SIZE) ||
        !readable_range(aim_source + UNIT_CONTROL_AIM_SCALAR_OFFSET,
                        sizeof(uint32_t)) ||
        !writable_range(target + UNIT_FLAGS_OFFSET, sizeof(uint32_t)) ||
        !writable_range(target + UNIT_CONTROL_FLAGS_OFFSET, sizeof(uint64_t)) ||
        !writable_range(target + UNIT_VEHICLE_PRIMARY_TRIGGER_OFFSET, sizeof(float)) ||
        !writable_range(target + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
                        UNIT_AIM_VECTOR_BLOCK_SIZE) ||
        !writable_range(target + UNIT_CONTROL_AIM_VECTOR_OFFSET,
                        UNIT_CONTROL_AIM_VECTOR_SIZE) ||
        !writable_range(target + UNIT_CONTROL_AIM_SCALAR_OFFSET,
                        sizeof(uint32_t)))
        return 0;
    pilot_control = *(uint64_t *)(pilot + UNIT_CONTROL_FLAGS_OFFSET);
    trigger = *(float *)(pilot + UNIT_DRIVER_PRIMARY_TRIGGER_OFFSET);
    if (!isfinite(trigger) || trigger < 0.0f) trigger = 0.0f;
    if (trigger > 1.0f) trigger = 1.0f;
    requested = enabled ||
        (pilot_control & UNIT_CONTROL_PRIMARY_TRIGGER_BIT) != 0 ||
        trigger > 0.05f;
    if (enabled && trigger < 1.0f) trigger = 1.0f;
    if (!requested) trigger = 0.0f;
    memcpy(target + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
           aim_source + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
           UNIT_AIM_VECTOR_BLOCK_SIZE);
    memcpy(target + UNIT_CONTROL_AIM_VECTOR_OFFSET,
           aim_source + UNIT_CONTROL_AIM_VECTOR_OFFSET,
           UNIT_CONTROL_AIM_VECTOR_SIZE);
    memcpy(target + UNIT_CONTROL_AIM_SCALAR_OFFSET,
           aim_source + UNIT_CONTROL_AIM_SCALAR_OFFSET,
           sizeof(uint32_t));
    vehicle_control = *(uint64_t *)(target + UNIT_CONTROL_FLAGS_OFFSET);
    if (requested) vehicle_control |= UNIT_CONTROL_VEHICLE_PRIMARY_BIT;
    else vehicle_control &= ~UNIT_CONTROL_VEHICLE_PRIMARY_BIT;
    *(uint64_t *)(target + UNIT_CONTROL_FLAGS_OFFSET) = vehicle_control;
    *(float *)(target + UNIT_VEHICLE_PRIMARY_TRIGGER_OFFSET) = trigger;
    *(uint32_t *)(target + UNIT_FLAGS_OFFSET) |=
        UNIT_FLAG_ACTIVELY_CONTROLLED | UNIT_FLAG_CHANGED_AIMING_ANGLES;
    ((weapon_update_fn)(g_blam + 0x59F460))(target_datum);
    return 1;
}

/* Chemin terrain de reference 1.5.49 : aucun weapon_fire manuel et aucune
 * conversion camera. Les quatre slots historiques du pilote sont recopies
 * octet pour octet; le bit 36 laisse ensuite Halo creer seul le projectile,
 * ses degats et l'animation de la tourelle. */
static int set_pelican_1549_engine_control(
        int32_t pilot_datum, int32_t module_datum, int enabled) {
    static const uint32_t offsets[] = {
        PELICAN_1549_DESIRED_AIM_OFFSET,
        PELICAN_1549_CURRENT_AIM_OFFSET,
        PELICAN_1549_DESIRED_LOOK_OFFSET,
        PELICAN_1549_CURRENT_LOOK_OFFSET
    };
    uint8_t *pilot = object_data(pilot_datum);
    uint8_t *module = object_data(module_datum);
    uint64_t pilot_control;
    uint64_t module_control;
    float trigger;
    int requested;
    size_t index;

    if (!pilot || !module ||
        !readable_range(pilot + UNIT_CONTROL_FLAGS_OFFSET, sizeof(uint64_t)) ||
        !readable_range(pilot + UNIT_DRIVER_PRIMARY_TRIGGER_OFFSET,
                        sizeof(float)) ||
        !writable_range(module + UNIT_FLAGS_OFFSET, sizeof(uint32_t)) ||
        !writable_range(module + UNIT_CONTROL_FLAGS_OFFSET, sizeof(uint64_t)) ||
        !writable_range(module + UNIT_VEHICLE_PRIMARY_TRIGGER_OFFSET,
                        sizeof(float))) return 0;
    for (index = 0; index < sizeof(offsets) / sizeof(offsets[0]); ++index) {
        if (!readable_range(pilot + offsets[index], 3u * sizeof(float)) ||
            !writable_range(module + offsets[index], 3u * sizeof(float)))
            return 0;
    }
    pilot_control = *(uint64_t *)(pilot + UNIT_CONTROL_FLAGS_OFFSET);
    trigger = *(float *)(pilot + UNIT_DRIVER_PRIMARY_TRIGGER_OFFSET);
    if (!isfinite(trigger) || trigger < 0.0f) trigger = 0.0f;
    if (trigger > 1.0f) trigger = 1.0f;
    requested = enabled ||
        (pilot_control & UNIT_CONTROL_PRIMARY_TRIGGER_BIT) != 0 ||
        trigger > 0.05f;
    if (enabled && trigger < 1.0f) trigger = 1.0f;
    if (!requested) trigger = 0.0f;
    for (index = 0; index < sizeof(offsets) / sizeof(offsets[0]); ++index)
        memcpy(module + offsets[index], pilot + offsets[index],
               3u * sizeof(float));
    module_control = *(uint64_t *)(module + UNIT_CONTROL_FLAGS_OFFSET);
    if (requested) module_control |= UNIT_CONTROL_VEHICLE_PRIMARY_BIT;
    else module_control &= ~UNIT_CONTROL_VEHICLE_PRIMARY_BIT;
    *(uint64_t *)(module + UNIT_CONTROL_FLAGS_OFFSET) = module_control;
    *(float *)(module + UNIT_VEHICLE_PRIMARY_TRIGGER_OFFSET) = trigger;
    *(uint32_t *)(module + UNIT_FLAGS_OFFSET) |=
        UNIT_FLAG_ACTIVELY_CONTROLLED | UNIT_FLAG_CHANGED_AIMING_ANGLES;
    MemoryBarrier();
    ((weapon_update_fn)(g_blam + 0x59F460))(module_datum);
    return 1;
}

/* Rayon secondaire d'impact uniquement. Le projectile principal ne consomme
 * jamais ce point : il reste entierement produit par engine_control_pipeline. */
static int pelican_resolve_1549_impact_target(
        int32_t pilot_datum, int32_t vehicle_datum, int32_t module_datum,
        float target_point[3]) {
    uint8_t *module = object_data(module_datum);
    float origin[3];
    float aim[3];
    int index;
    if (!module || !target_point || !readable_range(
            module + PELICAN_1549_DESIRED_AIM_OFFSET, sizeof(aim)))
        return pelican_resolve_effective_reticle_target(
            pilot_datum, vehicle_datum, module_datum, target_point);
    memcpy(aim, module + PELICAN_1549_DESIRED_AIM_OFFSET, sizeof(aim));
    if (!pelican_vector_normalize(aim))
        return pelican_resolve_effective_reticle_target(
            pilot_datum, vehicle_datum, module_datum, target_point);
    ((object_get_origin_fn)(g_blam + 0x5A6780))(module_datum, origin);
    for (index = 0; index < 3; ++index) {
        if (!isfinite(origin[index]))
            return pelican_resolve_effective_reticle_target(
                pilot_datum, vehicle_datum, module_datum, target_point);
        target_point[index] = origin[index] + aim[index] * 2048.0f;
    }
    return 1;
}

static int unit_primary_trigger_requested(int32_t unit_datum) {
    uint8_t *unit = object_data(unit_datum);
    uint64_t control;
    float trigger;
    if (!unit ||
        !readable_range(unit + UNIT_CONTROL_FLAGS_OFFSET, sizeof(uint64_t)) ||
        !readable_range(unit + UNIT_DRIVER_PRIMARY_TRIGGER_OFFSET, sizeof(float)))
        return 0;
    control = *(uint64_t *)(unit + UNIT_CONTROL_FLAGS_OFFSET);
    trigger = *(float *)(unit + UNIT_DRIVER_PRIMARY_TRIGGER_OFFSET);
    return (control & UNIT_CONTROL_PRIMARY_TRIGGER_BIT) != 0 ||
        (isfinite(trigger) && trigger > 0.05f);
}

/* Le controle du vehicule est le signal le plus fiable pour un pilote distant
 * sur le listen-server. Le controle du UnitDatum ci-dessus reste le repli pour
 * le pilote local et les builds ou le bit vehicule n'est pas replique. */
static int vehicle_primary_trigger_requested(int32_t vehicle_datum) {
    uint8_t *vehicle = object_data(vehicle_datum);
    uint64_t control;
    float trigger;
    if (!vehicle ||
        !readable_range(vehicle + UNIT_CONTROL_FLAGS_OFFSET, sizeof(uint64_t)) ||
        !readable_range(vehicle + UNIT_VEHICLE_PRIMARY_TRIGGER_OFFSET, sizeof(float)))
        return 0;
    control = *(uint64_t *)(vehicle + UNIT_CONTROL_FLAGS_OFFSET);
    trigger = *(float *)(vehicle + UNIT_VEHICLE_PRIMARY_TRIGGER_OFFSET);
    return (control & UNIT_CONTROL_VEHICLE_PRIMARY_BIT) != 0 ||
        (isfinite(trigger) && trigger > 0.05f);
}

/* Coupe uniquement la detente integree. Cette fonction est utilisee quand le
 * pilote quitte son siege; elle ne touche a aucune commande de vol et permet
 * donc de remonter puis de reprendre immediatement le controle du Pelican. */
static int clear_unit_vehicle_primary_control(int32_t vehicle_datum) {
    uint8_t *vehicle = object_data(vehicle_datum);
    uint64_t control;
    if (!vehicle ||
        !writable_range(vehicle + UNIT_CONTROL_FLAGS_OFFSET, sizeof(uint64_t)) ||
        !writable_range(vehicle + UNIT_VEHICLE_PRIMARY_TRIGGER_OFFSET, sizeof(float)))
        return 0;
    control = *(uint64_t *)(vehicle + UNIT_CONTROL_FLAGS_OFFSET);
    control &= ~UNIT_CONTROL_VEHICLE_PRIMARY_BIT;
    *(uint64_t *)(vehicle + UNIT_CONTROL_FLAGS_OFFSET) = control;
    *(float *)(vehicle + UNIT_VEHICLE_PRIMARY_TRIGGER_OFFSET) = 0.0f;
    ((weapon_update_fn)(g_blam + 0x59F460))(vehicle_datum);
    return 1;
}

static void pelican_reset_camera_target(int reset_statistics) {
    InterlockedIncrement(&g_pelican_camera_target.sequence);
    MemoryBarrier();
    InterlockedExchange(&g_pelican_camera_target.valid, 0);
    g_pelican_camera_target.vehicle = -1;
    memset(g_pelican_camera_target.local_target, 0,
           sizeof(g_pelican_camera_target.local_target));
    g_pelican_camera_target.updated_at_ms = 0;
    if (reset_statistics) {
        g_pelican_camera_target.accepted_samples = 0u;
        g_pelican_camera_target.rejected_samples = 0u;
        g_pelican_retail_context_shots = 0u;
        g_pelican_retail_context_failures = 0u;
        memset(g_pelican_last_target_point, 0,
               sizeof(g_pelican_last_target_point));
    }
    InterlockedExchange(&g_pelican_last_target_from_camera, 0);
    MemoryBarrier();
    InterlockedIncrement(&g_pelican_camera_target.sequence);
}

static void pelican_reset_validation_diagnostics(void) {
    InterlockedExchange(&g_pelican_module_inventory_missing_observed, 0);
    InterlockedExchange(&g_pelican_rate_scale_nonfinite_observed, 0);
    InterlockedExchange(&g_pelican_last_barrel_failure_detail, 0);
    g_pelican_last_barrel_actual_parent = -1;
    g_pelican_last_barrel_expected_parent = -1;
    g_pelican_last_barrel_weapon_tag = -1;
    g_pelican_last_barrel_count = 0u;
    g_pelican_last_barrel_min_bits = 0u;
    g_pelican_last_barrel_max_bits = 0u;
    g_pelican_last_barrel_rate_bits = 0u;
    g_pelican_audio_fire_events = 0u;
    g_pelican_audio_fire_failures = 0u;
    pelican_reset_camera_target(1);
    pelican_projectile_probe_reset();
}

/* Desarme completement l'ancien relais sur game_MAIN_THREAD. Cette routine
 * est aussi utilisee juste avant un nouvel ARM : aucun datum, etat de barrel
 * ou bit de detente de l'ancien Pelican ne peut ainsi contaminer le nouveau. */
static void pelican_release_relay_main_thread(uint32_t reason) {
    int32_t old_weapon = g_pelican_chin_weapon;
    int32_t old_vehicle = g_pelican_vehicle;
    int32_t old_module = g_pelican_chin_module;
    int32_t old_audio_module = g_pelican_audio_module;
    int32_t old_audio_weapon = g_pelican_audio_weapon;
    if (reason != 0u) {
        g_pelican_last_release_vehicle = old_vehicle;
        g_pelican_last_release_module = old_module;
        g_pelican_last_release_weapon = old_weapon;
        InterlockedExchange(
            &g_pelican_last_release_reason, (LONG)reason);
        InterlockedIncrement(&g_pelican_release_count);
    }
    InterlockedExchange(&g_pelican_armed, 0);
    InterlockedExchange(&g_pelican_requested_primary, 0);
    InterlockedExchange(&g_pelican_applied_primary, 0);
    InterlockedExchange(&g_pelican_driver_present, 0);
    InterlockedExchange(&g_pelican_local_input_hold_ticks, 0);
    InterlockedExchange(&g_pelican_pilot_absence_ticks, 0);
    InterlockedExchange(&g_pelican_last_driver_datum, -1);
    InterlockedExchange(&g_pelican_last_target_ok, 0);
    InterlockedExchange(&g_pelican_last_aim_ok, 0);
    MemoryBarrier();
    pelican_stop_audio_proxy();
    pelican_direct_gate_abandon(old_weapon);
    if (old_weapon != -1 && object_data(old_weapon))
        set_weapon_primary_firing(old_weapon, 0);
    if (old_vehicle != -1 && object_data(old_vehicle))
        clear_unit_vehicle_primary_control(old_vehicle);
    if (old_module != -1 && old_module != old_vehicle &&
        object_data(old_module))
        clear_unit_vehicle_primary_control(old_module);
    if (old_audio_weapon != -1 && object_data(old_audio_weapon)) {
        set_weapon_primary_firing(old_audio_weapon, 0);
        ((weapon_barrel_reset_fn)(g_blam + 0x5CE2D0))(
            old_audio_weapon, 0);
    }
    if (old_audio_module != -1 && object_data(old_audio_module))
        clear_unit_vehicle_primary_control(old_audio_module);
    /* Ordre inverse de l'ARM transactionnel : traceur, impact, presentation. */
    (void)pelican_restore_tracer_patch();
    if (!pelican_restore_impact_patch())
        g_pelican_impact_patch_error = 10;
    if (!pelican_restore_presentation_patch())
        g_pelican_presentation_patch_error = 10;
    g_pelican_local_pilot = -1;
    g_pelican_vehicle = -1;
    g_pelican_chin_module = -1;
    g_pelican_chin_weapon = -1;
    g_pelican_audio_module = -1;
    g_pelican_audio_weapon = -1;
    g_pelican_aux_impact_projectile_tag = -1;
    pelican_reset_camera_target(0);
    g_pelican_completed_shots = 0;
    g_pelican_aux_impact_created = 0;
    g_pelican_aux_impact_failures = 0;
    g_pelican_last_shot_serial = 0;
    InterlockedExchange(&g_pelican_fire_interval_ms, 100);
}

static int unit_has_weapon_datum(int32_t unit_datum, int32_t weapon_datum) {
    uint8_t *unit = object_data(unit_datum);
    int slot;
    if (!unit || !readable_range(unit + UNIT_WEAPON_INVENTORY_OFFSET,
                                 4u * sizeof(int32_t))) return 0;
    for (slot = 0; slot < 4; ++slot)
        if (*(int32_t *)(unit + UNIT_WEAPON_INVENTORY_OFFSET + slot * 4u) ==
            weapon_datum) return 1;
    return 0;
}

/* Le bit runtime 7 est celui que les tests de degats Blam consultent. Le
 * wrapper HaloScript a 0x65D3E0 ne prend PAS un datum objet : il prend un
 * datum de liste d'objets et parcourt ensuite tous ses membres. Lui passer un
 * datum de biped/vehicule pouvait donc dereferencer un membre inexistant
 * (crash confirme a 0x65D4A0 apres entree dans un vehicule). Ecrire le bit de
 * l'objet exact est a la fois le comportement voulu ici et le seul chemin
 * borne : aucun parcours de liste, aucun autre acteur rendu invincible. */
static int set_object_damageproof(int32_t datum_index, int enabled) {
    uint8_t *object = object_data(datum_index);
    uint32_t flags;
    volatile LONG *runtime_flags;
    const LONG mask = (LONG)(1u << 7);
    if (!object || !writable_range(object + 0x128, sizeof(flags))) return 0;
    flags = *(uint32_t *)(object + 0x128);
    if (((flags >> 7) & 1u) == (enabled ? 1u : 0u)) return 1;
    runtime_flags = (volatile LONG *)(object + 0x128);
    if (enabled) InterlockedOr(runtime_flags, mask);
    else InterlockedAnd(runtime_flags, ~mask);
    MemoryBarrier();
    object = object_data(datum_index);
    if (!object || !readable_range(object + 0x128, sizeof(flags))) return 0;
    flags = *(uint32_t *)(object + 0x128);
    return ((flags >> 7) & 1u) == (enabled ? 1u : 0u);
}

static int is_live_vehicle_object(int32_t datum_index) {
    uint8_t *object = object_data(datum_index);
    if (!object || !readable_range(object, sizeof(int32_t))) return 0;
    return ((tag_get_fn)(g_blam + 0x201480))(
               0x69686576u, *(int32_t *)object) != NULL;
}

/* L'invincibilite joueur suit aussi le vehicule actuellement occupe. Le
 * Spartan peut autrement rester protege tandis que son Pelican explose, ce
 * qui donne l'impression que l'option ne fonctionne pas. Les anciens datums
 * sont verifies par leur salt dans object_data avant toute restauration. */
static int set_player_damageproof(int32_t unit_datum, int enabled) {
    uint8_t *unit = object_data(unit_datum);
    int32_t parent = -1;
    int ok;
    /* Une reference UObject transitoire ne suffit pas : en coop ou pendant un
     * GameState, elle peut momentanement exposer le datum d'une IA. On exige
     * le player record sale pour toute activation. Une desactivation reste
     * autorisee pour notre propre ancien datum afin de toujours pouvoir le
     * nettoyer apres une mort. */
    if (!unit || (enabled && !is_live_player_unit(unit_datum)) ||
        (!enabled && unit_datum != g_damageproof_unit &&
         !is_live_player_unit(unit_datum))) return 0;
    if (readable_range(unit + 0x14, sizeof(parent)))
        parent = *(int32_t *)(unit + 0x14);
    if (g_damageproof_unit != -1 && g_damageproof_unit != unit_datum &&
        object_data(g_damageproof_unit))
        set_object_damageproof(g_damageproof_unit, 0);
    if (g_damageproof_parent != -1 &&
        (!enabled || g_damageproof_parent != parent) &&
        object_data(g_damageproof_parent))
        set_object_damageproof(g_damageproof_parent, 0);
    g_damageproof_parent = -1;
    ok = set_object_damageproof(unit_datum, enabled);
    if (enabled && parent != -1 && is_live_vehicle_object(parent)) {
        ok = set_object_damageproof(parent, 1) && ok;
        if (ok) g_damageproof_parent = parent;
    }
    g_damageproof_unit = enabled && ok ? unit_datum : -1;
    return ok;
}

/* Les bipeds de modele peuvent heriter de drapeaux runtime transitoires. Une
 * IA creee par le sandbox doit toujours rester mortelle : ne touche au moteur
 * que si le bit 7 est effectivement pose, puis confirme sa suppression. */
static int ensure_object_damageable(int32_t datum_index, int *cleared_out) {
    uint8_t *object = object_data(datum_index);
    uint32_t flags;
    if (cleared_out) *cleared_out = 0;
    if (!object || !readable_range(object + 0x128, sizeof(flags))) return 0;
    flags = *(uint32_t *)(object + 0x128);
    if ((flags & (1u << 7)) == 0) return 1;
    if (!set_object_damageproof(datum_index, 0)) return 0;
    if (cleared_out) *cleared_out = 1;
    return 1;
}

/* Trait joueur retail "infinite ammo". Contrairement a la recharge
 * periodique, le mode 3 est consulte directement par tous les chemins Halo :
 * cartouches, reserve et batterie ne sont jamais consommees. */
static uint8_t *player_ammo_trait(int32_t unit_datum,
                                  int32_t *player_datum_out,
                                  uint8_t **table_out,
                                  uint8_t **entries_out) {
    uint8_t *unit = object_data(unit_datum);
    uint8_t *thread_state = blam_thread_state();
    uint8_t *table;
    uint8_t *entries;
    uint8_t *record;
    int32_t player_datum;
    int32_t capacity;
    int32_t high;
    uint32_t slot;
    uint32_t *allocation_bits;
    if (!unit || !readable_range(unit + 0x1BC, sizeof(player_datum)) ||
        !thread_state || !readable_range(thread_state + 0x30, sizeof(void *)))
        return NULL;
    player_datum = *(int32_t *)(unit + 0x1BC);
    if (player_datum == -1) return NULL;
    table = *(uint8_t **)(thread_state + 0x30);
    if (!table || !readable_range(table + 0x20, 0x40u) ||
        *(uint64_t *)(table + 0x20) != 0x4B0u ||
        *(uint32_t *)(table + 0x28) != 0x64407440u) return NULL;
    capacity = *(int32_t *)(table + 0x2C);
    high = *(int32_t *)(table + 0x44);
    entries = *(uint8_t **)(table + 0x50);
    allocation_bits = *(uint32_t **)(table + 0x58);
    slot = (uint16_t)player_datum;
    if (capacity != 0x10 || high < 0 || high > capacity ||
        slot >= (uint32_t)high || !entries || !allocation_bits ||
        !readable_range(allocation_bits, sizeof(uint32_t)) ||
        (allocation_bits[slot >> 5] & (1u << (slot & 31u))) == 0 ||
        !readable_range(entries + (size_t)slot * 0x4B0u, 0x352u))
        return NULL;
    record = entries + (size_t)slot * 0x4B0u;
    /* Les player records sont des datums salts comme les objets/acteurs. */
    if (*(uint16_t *)record !=
        (uint16_t)((uint32_t)player_datum >> 16))
        return NULL;
    if (!writable_range(record + 0x351, sizeof(uint8_t))) return NULL;
    if (player_datum_out) *player_datum_out = player_datum;
    if (table_out) *table_out = table;
    if (entries_out) *entries_out = entries;
    return record + 0x351;
}

static int is_live_player_unit(int32_t unit_datum) {
    int32_t player_datum = -1;
    return player_ammo_trait(unit_datum, &player_datum, NULL, NULL) != NULL &&
           player_datum != -1;
}

static int set_player_bottomless_trait(int32_t unit_datum, int enabled,
                                       int *changed_out,
                                       int *new_snapshot_out) {
    int32_t player_datum = -1;
    uint8_t *table = NULL;
    uint8_t *entries = NULL;
    uint8_t *trait = player_ammo_trait(
        unit_datum, &player_datum, &table, &entries);
    int changed = 0;
    int new_snapshot = 0;
    if (!trait) return 0;
    if (enabled) {
        if (!g_bottomless_snapshot_valid ||
            g_bottomless_player_datum != player_datum ||
            g_bottomless_player_table != table ||
            g_bottomless_player_entries != entries) {
            g_bottomless_player_datum = player_datum;
            g_bottomless_player_table = table;
            g_bottomless_player_entries = entries;
            g_bottomless_original_trait = *trait;
            g_bottomless_snapshot_valid = 1;
            new_snapshot = 1;
        }
        if (*trait != 3u) {
            *trait = 3u;
            changed = 1;
        }
    } else {
        /* Ne restaure que notre propre ecriture. Sans snapshot correspondant,
         * ou si le moteur/un autre mod a change le trait entre-temps, OFF ne
         * doit jamais ecraser l'etat legitime 1/2/3 du joueur. */
        if (g_bottomless_snapshot_valid &&
            g_bottomless_player_datum == player_datum &&
            g_bottomless_player_table == table &&
            g_bottomless_player_entries == entries && *trait == 3u &&
            *trait != g_bottomless_original_trait) {
            *trait = g_bottomless_original_trait;
            changed = 1;
        }
        g_bottomless_player_datum = -1;
        g_bottomless_player_table = NULL;
        g_bottomless_player_entries = NULL;
        g_bottomless_original_trait = 1u;
        g_bottomless_snapshot_valid = 0;
    }
    if (changed_out) *changed_out = changed;
    if (new_snapshot_out) *new_snapshot_out = new_snapshot;
    return 1;
}

/* Recharge les quatre emplacements d'inventaire du joueur par les setters
 * retail. 0x5C6570 borne les cartouches chargees a la capacite du chargeur;
 * 0x5C6660 recoit la reserve maximale lue dans la definition du chargeur.
 * 0x5C64F0 remet aussi l'age a zero, donc les armes a energie Covenant sont
 * rechargees sans supposer qu'elles possedent un chargeur classique. */
static int refill_unit_ammo(int32_t unit_datum, int reserve_only,
                            int *weapons_out,
                            int *magazines_out, int *grenades_out) {
    uint8_t *unit = object_data(unit_datum);
    weapon_set_age_fn set_age = (weapon_set_age_fn)(g_blam + 0x5C64F0);
    weapon_set_ammo_fn set_loaded = (weapon_set_ammo_fn)(g_blam + 0x5C6570);
    weapon_set_ammo_fn set_reserve = (weapon_set_ammo_fn)(g_blam + 0x5C6660);
    uint8_t **pages = (uint8_t **)(g_blam + 0x2C2CCC0);
    int weapons = 0;
    int magazines = 0;
    int grenades = 0;
    int slot;

    if (!unit || !readable_range(unit + 0x344, 16)) return 0;
    for (slot = 0; slot < 4; ++slot) {
        int32_t weapon_datum = *(int32_t *)(unit + 0x344 + slot * 4);
        uint8_t *weapon;
        uint8_t *definition;
        uint32_t magazine_count;
        uint32_t token;
        uint8_t *magazine;
        uint16_t loaded_max;
        uint16_t reserve_max;
        uint16_t loaded_current;
        uint16_t reserve_current;
        uint16_t loaded_predicted;
        uint16_t reserve_predicted;
        uint8_t sync_state;
        float age_current;
        float age_predicted;
        if (weapon_datum == -1) continue;
        weapon = object_data(weapon_datum);
        if (!weapon || !readable_range(weapon, sizeof(int32_t))) continue;
        definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
            0x77656170u, *(int32_t *)weapon);
        if (!definition || !readable_range(definition + 0x3CC, 8)) continue;

        /* Ne marque jamais une arme deja pleine comme modifiee. Les setters
         * propagent leur dirty state dans la simulation et leur ancien appel
         * inconditionnel etait la source des impulsions de frame-time. */
        if (!readable_range(weapon + 0x1C3, sizeof(uint8_t)) ||
            !readable_range(weapon + 0x1D4, 2u * sizeof(float))) continue;
        sync_state = weapon[0x1C3];
        age_predicted = *(float *)(weapon + 0x1D4);
        age_current = *(float *)(weapon + 0x1D8);
        if ((isfinite(age_current) && age_current > 0.0f) ||
            (isfinite(age_predicted) && age_predicted > 0.0f)) {
            set_age(weapon_datum, 0.0f);
            /* Le setter retail ne copie le miroir predit que lorsque +0x1C3
             * vaut zero. Or c'est bien +0x1D4 que le chemin de tir batterie
             * incremente. Pour l'arme portee par le joueur cible, remettre ce
             * seul miroir a zero est necessaire en coop lorsque l'objet est
             * encore dans un etat de prediction reseau non nul. La chaleur et
             * le verrou de surchauffe vivent ailleurs et restent intacts. */
            if (sync_state != 0 && isfinite(age_predicted) &&
                age_predicted > 0.0f &&
                writable_range(weapon + 0x1D4, sizeof(float)))
                *(float *)(weapon + 0x1D4) = 0.0f;
        }
        ++weapons;

        magazine_count = *(uint32_t *)(definition + 0x3CC);
        token = *(uint32_t *)(definition + 0x3D0);
        if (!magazine_count || magazine_count > 16 ||
            !readable_range(pages + (token >> 28), sizeof(void *)) ||
            !pages[token >> 28]) continue;
        magazine = pages[token >> 28] + (uint64_t)token * 4u;
        if (!readable_range(magazine, 0x0E)) continue;
        if (!readable_range(weapon + 0x2BC, 3u * sizeof(uint16_t))) continue;
        loaded_max = *(uint16_t *)(magazine + 0x0A);
        reserve_max = *(uint16_t *)(magazine + 0x0C);
        reserve_predicted = *(uint16_t *)(weapon + 0x2BA);
        reserve_current = *(uint16_t *)(weapon + 0x2BC);
        loaded_predicted = *(uint16_t *)(weapon + 0x2BE);
        loaded_current = *(uint16_t *)(weapon + 0x2C0);
        if (!reserve_only &&
            ((int16_t)loaded_current < (int16_t)loaded_max ||
             (int16_t)loaded_predicted < (int16_t)loaded_max)) {
            /* Le deuxieme argument est un emplacement ABI reserve : ces deux
             * routines retail lisent la valeur dans R8W et gerent le premier
             * magazine runtime de l'arme. */
            set_loaded(weapon_datum, 0, loaded_max);
            /* +0x2BE est le compteur effectivement soustrait par
             * weapon_magazine_update. Le setter le laisse volontairement
             * intact pour un objet predit (+0x1C3 != 0); le garde de munitions
             * doit donc synchroniser ce miroir pour l'arme inventoriee du
             * joueur, sinon l'affichage courant est plein mais le tir reste
             * vide. */
            if (sync_state != 0 &&
                (int16_t)loaded_predicted < (int16_t)loaded_max &&
                writable_range(weapon + 0x2BE, sizeof(uint16_t)))
                *(uint16_t *)(weapon + 0x2BE) = loaded_max;
            ++magazines;
        }
        if ((int16_t)reserve_current < (int16_t)reserve_max ||
            (int16_t)reserve_predicted < (int16_t)reserve_max) {
            set_reserve(weapon_datum, 0, reserve_max);
            if (sync_state != 0 &&
                (int16_t)reserve_predicted < (int16_t)reserve_max &&
                writable_range(weapon + 0x2BA, sizeof(uint16_t)))
                *(uint16_t *)(weapon + 0x2BA) = reserve_max;
            ++magazines;
        }
    }

    /* Deux types de grenades existent dans la campagne CE. */
    if (writable_range(unit + 0x382, 2)) {
        if (unit[0x382] < 4) unit[0x382] = 4;
        if (unit[0x383] < 4) unit[0x383] = 4;
        grenades = 8;
    }
    if (weapons_out) *weapons_out = weapons;
    if (magazines_out) *magazines_out = magazines;
    if (grenades_out) *grenades_out = grenades;
    return 1;
}

/* Les limites de campagne de cette build sont separees des collisions BSP :
 * - scenario+0x248 decrit les soft surfaces, dont le bit d'activation vit
 *   dans le bitset TLS +0x430 ;
 * - scenario+0x278 decrit les trigger volumes. Leur index de kill volume est
 *   le short +0x78 et le bit "desactive" vit dans le bitset TLS +0x340.
 *
 * Ce sont exactement les donnees manipulees par les wrappers retail
 * soft_ceiling_enable (RVA 0x1B4670) et kill_volume_disable
 * (RVA 0x1B2540). Le snapshot restaure bit pour bit l'etat de la mission a la
 * sortie du noclip et n'altère jamais la collision normale du BSP. */
/* Les trigger volumes sont contigus dans une page de tags. L'ancienne boucle
 * appelait VirtualQuery une fois par volume, puis recommencait au second
 * passage d'activation et a chaque maintenance. Resoudre et valider tout le
 * bloc une seule fois retire ce pic du thread principal. Le stride est stocke
 * en mots de quatre octets dans le token Blam. */
static uint8_t *scenario_block_records(uint32_t token, uint32_t count,
                                       uint32_t stride_words,
                                       size_t readable_record_size) {
    uint8_t **pages = (uint8_t **)(g_blam + 0x2C2CCC0);
    uint64_t last_token;
    uint64_t total_size;
    uint32_t page;
    uint8_t *records;
    if (!count) return NULL;
    if (!stride_words || !readable_record_size) return NULL;
    last_token = (uint64_t)token +
        (uint64_t)(count - 1u) * (uint64_t)stride_words;
    if (last_token > UINT32_MAX ||
        ((uint32_t)last_token >> 28) != (token >> 28)) return NULL;
    total_size = (uint64_t)(count - 1u) *
        (uint64_t)stride_words * 4u + readable_record_size;
    if (total_size > SIZE_MAX) return NULL;
    page = token >> 28;
    if (!readable_range(pages + page, sizeof(void *)) || !pages[page])
        return NULL;
    records = pages[page] + (uint64_t)token * 4u;
    return readable_range(records, (size_t)total_size) ? records : NULL;
}

static void zone_limits_forget(void) {
    memset(&g_zone_limits, 0, sizeof(g_zone_limits));
}

static int zone_limits_current_context(void **scenario_out,
                                       uint32_t **soft_words_out,
                                       uint32_t **kill_words_out) {
    uint8_t *thread_state = blam_thread_state();
    void *scenario;
    uint32_t *soft_words;
    uint32_t *kill_words;
    if (!thread_state || !readable_range(thread_state + 0x430, sizeof(void *)) ||
        !readable_range(thread_state + 0x340, sizeof(void *)) ||
        !readable_range(g_blam + 0x10C3558, sizeof(void *))) return 0;
    scenario = *(void **)(g_blam + 0x10C3558);
    soft_words = *(uint32_t **)(thread_state + 0x430);
    kill_words = *(uint32_t **)(thread_state + 0x340);
    if (!scenario || !readable_range((uint8_t *)scenario + 0x248, 0x38)) return 0;
    *scenario_out = scenario;
    *soft_words_out = soft_words;
    *kill_words_out = kill_words;
    return 1;
}

static int zone_limits_snapshot_and_disable(void) {
    void *scenario;
    uint32_t *soft_words;
    uint32_t *kill_words;
    uint32_t soft_count, soft_token;
    uint32_t trigger_count, trigger_token;
    uint32_t soft_word_count;
    uint32_t kill_word_count = 0;
    int32_t max_kill_index = -1;
    uint8_t *trigger_records = NULL;
    uint32_t i;

    if (!zone_limits_current_context(&scenario, &soft_words, &kill_words)) return 0;
    soft_count = *(uint32_t *)((uint8_t *)scenario + 0x248);
    soft_token = *(uint32_t *)((uint8_t *)scenario + 0x24C);
    trigger_count = *(uint32_t *)((uint8_t *)scenario + 0x278);
    trigger_token = *(uint32_t *)((uint8_t *)scenario + 0x27C);
    if (soft_count > NOCLIP_MAX_ZONE_WORDS * 32u || trigger_count > 16384u) return 0;
    soft_word_count = (soft_count + 31u) / 32u;

    if (trigger_count) {
        trigger_records = scenario_block_records(
            trigger_token, trigger_count, 0x1Fu, 0x7Au);
        if (!trigger_records) return 0;
    }

    for (i = 0; i < trigger_count; ++i) {
        uint8_t *trigger = trigger_records + (size_t)i * 0x7Cu;
        int32_t kill_index;
        kill_index = *(int16_t *)(trigger + 0x78);
        if (kill_index > max_kill_index) max_kill_index = kill_index;
    }
    if (max_kill_index >= (int32_t)(NOCLIP_MAX_ZONE_WORDS * 32u)) return 0;
    if (max_kill_index >= 0) kill_word_count = ((uint32_t)max_kill_index + 32u) / 32u;
    if ((soft_word_count && (!soft_words || !readable_range(soft_words, soft_word_count * sizeof(uint32_t)) ||
                             !writable_range(soft_words, soft_word_count * sizeof(uint32_t)))) ||
        (kill_word_count && (!kill_words || !readable_range(kill_words, kill_word_count * sizeof(uint32_t)) ||
                             !writable_range(kill_words, kill_word_count * sizeof(uint32_t))))) return 0;

    zone_limits_forget();
    g_zone_limits.scenario = scenario;
    g_zone_limits.soft_words = soft_words;
    g_zone_limits.kill_words = kill_words;
    g_zone_limits.soft_count = soft_count;
    g_zone_limits.trigger_count = trigger_count;
    g_zone_limits.trigger_token = trigger_token;
    g_zone_limits.soft_word_count = soft_word_count;
    g_zone_limits.kill_word_count = kill_word_count;
    if (soft_word_count)
        memcpy(g_zone_limits.soft_original, soft_words,
               soft_word_count * sizeof(uint32_t));
    if (kill_word_count)
        memcpy(g_zone_limits.kill_original, kill_words,
               kill_word_count * sizeof(uint32_t));
    g_zone_limits.valid = 1;

    /* Construire une fois les masques : un soft ceiling est actif quand son
     * bit vaut 1; un kill volume est desactive quand son bit vaut 1. */
    for (i = 0; i < soft_count; ++i)
        g_zone_limits.soft_disable_mask[i >> 5] |=
            1u << (i & 31u);
    for (i = 0; i < trigger_count; ++i) {
        uint8_t *trigger = trigger_records + (size_t)i * 0x7Cu;
        int32_t kill_index = *(int16_t *)(trigger + 0x78);
        if (kill_index >= 0)
            g_zone_limits.kill_disable_mask[(uint32_t)kill_index >> 5] |=
                1u << ((uint32_t)kill_index & 31u);
    }
    for (i = 0; i < soft_word_count; ++i) {
        uint32_t desired = soft_words[i] &
            ~g_zone_limits.soft_disable_mask[i];
        if (soft_words[i] != desired) soft_words[i] = desired;
    }
    for (i = 0; i < kill_word_count; ++i) {
        uint32_t desired = kill_words[i] |
            g_zone_limits.kill_disable_mask[i];
        if (kill_words[i] != desired) kill_words[i] = desired;
    }
    MemoryBarrier();
    return 1;
}

static int zone_limits_enforce_disabled(void) {
    void *scenario;
    uint32_t *soft_words;
    uint32_t *kill_words;
    uint32_t soft_count, trigger_count, trigger_token;
    uint32_t i;
    if (!g_zone_limits.valid ||
        !zone_limits_current_context(&scenario, &soft_words, &kill_words) ||
        scenario != g_zone_limits.scenario ||
        soft_words != g_zone_limits.soft_words ||
        kill_words != g_zone_limits.kill_words) {
        return zone_limits_snapshot_and_disable();
    }
    soft_count = *(uint32_t *)((uint8_t *)scenario + 0x248);
    trigger_count = *(uint32_t *)((uint8_t *)scenario + 0x278);
    trigger_token = *(uint32_t *)((uint8_t *)scenario + 0x27C);
    if (soft_count != g_zone_limits.soft_count ||
        trigger_count != g_zone_limits.trigger_count ||
        trigger_token != g_zone_limits.trigger_token)
        return zone_limits_snapshot_and_disable();
    /* Aucun scan de tag et aucune ecriture si Halo conserve deja les bits. */
    for (i = 0; i < g_zone_limits.soft_word_count; ++i) {
        uint32_t desired = soft_words[i] &
            ~g_zone_limits.soft_disable_mask[i];
        if (soft_words[i] != desired) soft_words[i] = desired;
    }
    for (i = 0; i < g_zone_limits.kill_word_count; ++i) {
        uint32_t desired = kill_words[i] |
            g_zone_limits.kill_disable_mask[i];
        if (kill_words[i] != desired) kill_words[i] = desired;
    }
    MemoryBarrier();
    return 1;
}

static void zone_limits_restore(void) {
    void *scenario;
    uint32_t *soft_words;
    uint32_t *kill_words;
    if (g_zone_limits.valid &&
        zone_limits_current_context(&scenario, &soft_words, &kill_words) &&
        scenario == g_zone_limits.scenario &&
        soft_words == g_zone_limits.soft_words &&
        kill_words == g_zone_limits.kill_words) {
        if (g_zone_limits.soft_word_count &&
            writable_range(soft_words,
                           g_zone_limits.soft_word_count * sizeof(uint32_t)))
            memcpy(soft_words, g_zone_limits.soft_original,
                   g_zone_limits.soft_word_count * sizeof(uint32_t));
        if (g_zone_limits.kill_word_count &&
            writable_range(kill_words,
                           g_zone_limits.kill_word_count * sizeof(uint32_t)))
            memcpy(kill_words, g_zone_limits.kill_original,
                   g_zone_limits.kill_word_count * sizeof(uint32_t));
        MemoryBarrier();
    }
    zone_limits_forget();
}

static void noclip_restore_player(int keep_damageproof) {
    int32_t datum = g_noclip_datum;
    uint8_t *object = object_data(datum);
    InterlockedExchange(&g_noclip_enabled, 0);
    if (object) {
        set_object_damageproof(datum, keep_damageproof != 0);
    }
    if (g_noclip_protected_vehicle != -1 &&
        object_data(g_noclip_protected_vehicle))
        set_object_damageproof(g_noclip_protected_vehicle, 0);
    zone_limits_restore();
    g_noclip_datum = -1;
    g_noclip_protected_vehicle = -1;
}

static void noclip_refresh_vehicle_protection(int32_t datum) {
    uint8_t *object = object_data(datum);
    int32_t parent = -1;
    if (object && readable_range(object + 0x14, sizeof(int32_t)))
        parent = *(int32_t *)(object + 0x14);
    if (parent == g_noclip_protected_vehicle) return;
    if (g_noclip_protected_vehicle != -1 &&
        object_data(g_noclip_protected_vehicle))
        set_object_damageproof(g_noclip_protected_vehicle, 0);
    g_noclip_protected_vehicle = -1;
    if (parent != -1 && object_data(parent)) {
        set_object_damageproof(parent, 1);
        g_noclip_protected_vehicle = parent;
    }
}

/* Le point de secours n'est jamais utilise automatiquement. Il permet au
 * joueur de revenir volontairement au dernier endroit valide s'il a coupe
 * les limites alors qu'il etait deja hors de la zone jouable. */
static int noclip_capture_anchor(int32_t datum) {
    uint8_t *object = object_data(datum);
    int32_t root = datum;
    void *scenario;
    float position[3], forward[3], up[3];
    if (!object || !readable_range(g_blam + 0x10C3558, sizeof(void *))) return 0;
    if (readable_range(object + 0x14, sizeof(int32_t))) {
        int32_t parent = *(int32_t *)(object + 0x14);
        if (parent != -1 && object_data(parent)) root = parent;
    }
    scenario = *(void **)(g_blam + 0x10C3558);
    if (!scenario || !object_data(root)) return 0;
    ((object_get_origin_fn)(g_blam + 0x5A6780))(root, position);
    ((object_get_orientation_fn)(g_blam + 0x5A6AE0))(root, forward, up);
    if (!isfinite(position[0]) || !isfinite(position[1]) || !isfinite(position[2]) ||
        !isfinite(forward[0]) || !isfinite(forward[1]) || !isfinite(forward[2]) ||
        !isfinite(up[0]) || !isfinite(up[1]) || !isfinite(up[2])) return 0;
    memcpy(g_noclip_anchor_position, position, sizeof(position));
    memcpy(g_noclip_anchor_forward, forward, sizeof(forward));
    memcpy(g_noclip_anchor_up, up, sizeof(up));
    g_noclip_anchor_scenario = scenario;
    InterlockedExchange(&g_noclip_anchor_valid, 1);
    return 1;
}

static int noclip_recover_anchor(int32_t datum, int keep_damageproof,
                                 int32_t *root_out) {
    uint8_t *object = object_data(datum);
    int32_t root = datum;
    void *scenario;
    if (InterlockedCompareExchange(&g_noclip_anchor_valid, 0, 0) == 0 ||
        !object || !readable_range(g_blam + 0x10C3558, sizeof(void *))) return 0;
    scenario = *(void **)(g_blam + 0x10C3558);
    if (!scenario || scenario != g_noclip_anchor_scenario) {
        InterlockedExchange(&g_noclip_anchor_valid, 0);
        return 0;
    }
    if (readable_range(object + 0x14, sizeof(int32_t))) {
        int32_t parent = *(int32_t *)(object + 0x14);
        if (parent != -1 && object_data(parent)) root = parent;
    }
    if (!object_data(root)) return 0;
    set_object_damageproof(datum, 1);
    if (root != datum) set_object_damageproof(root, 1);
    ((object_teleport_fn)(g_blam + 0x351610))(
        root, g_noclip_anchor_position, g_noclip_anchor_forward,
        g_noclip_anchor_up, 1);
    noclip_restore_player(keep_damageproof);
    /* Le retour reste disponible apres une desactivation normale de F10 :
     * dans ce cas g_noclip_datum vaut deja -1, donc restaurer explicitement
     * les drapeaux du joueur et du vehicule courant. */
    set_object_damageproof(datum, keep_damageproof != 0);
    if (root != datum) set_object_damageproof(root, 0);
    InterlockedExchange(&g_noclip_anchor_valid, 0);
    if (root_out) *root_out = root;
    return 1;
}

static int noclip_enable_player(int32_t datum) {
    uint8_t *object;
    int already_enabled =
        InterlockedCompareExchange(&g_noclip_enabled, 0, 0) != 0 &&
        g_noclip_datum == datum;
    if (!is_live_player_unit(datum)) return 0;
    if (InterlockedCompareExchange(&g_noclip_enabled, 0, 0) != 0 &&
        g_noclip_datum != datum) noclip_restore_player(0);
    object = object_data(datum);
    if (!object) return 0;
    if (already_enabled) {
        set_object_damageproof(datum, 1);
        noclip_refresh_vehicle_protection(datum);
        return zone_limits_enforce_disabled();
    }
    g_noclip_datum = datum;
    if (!noclip_capture_anchor(datum)) {
        g_noclip_datum = -1;
        return 0;
    }
    set_object_damageproof(datum, 1);
    noclip_refresh_vehicle_protection(datum);
    if (!zone_limits_snapshot_and_disable()) {
        set_object_damageproof(datum, 0);
        if (g_noclip_protected_vehicle != -1 &&
            object_data(g_noclip_protected_vehicle))
            set_object_damageproof(g_noclip_protected_vehicle, 0);
        g_noclip_protected_vehicle = -1;
        g_noclip_datum = -1;
        InterlockedExchange(&g_noclip_anchor_valid, 0);
        return 0;
    }
    InterlockedExchange(&g_noclip_enabled, 1);
    return 1;
}

static int noclip_tick(void) {
    int32_t datum = g_noclip_datum;
    if (InterlockedCompareExchange(&g_noclip_enabled, 0, 0) == 0 ||
        datum == -1 || !object_data(datum)) return 0;
    if (!is_live_player_unit(datum)) {
        noclip_restore_player(0);
        return 0;
    }
    set_object_damageproof(datum, 1);
    noclip_refresh_vehicle_protection(datum);
    return zone_limits_enforce_disabled();
}

static int valid_tag_name(const char *name) {
    size_t i;
    if (!name || !readable_range(name, 1)) return 0;
    for (i = 0; i < 255; ++i) {
        unsigned char c;
        if (!readable_range(name + i, 1)) return 0;
        c = (unsigned char)name[i];
        if (c == 0) return i > 0;
        if (c < 32 || c > 126) return 0;
    }
    return 0;
}

static int tag_name_equals(const char *name, const char *expected) {
    size_t length = 0;
    size_t i;
    if (!name || !expected) return 0;
    while (length < 255 && expected[length]) ++length;
    if (!length || length >= 255) return 0;
    /* Le nom attendu donne la longueur exacte : une seule validation de la
     * region memoire suffit dans le cas normal. L'ancienne implementation
     * appelait VirtualQuery pour chaque caractere de chaque tag et pouvait
     * immobiliser le thread de simulation pendant un chargement de carte. */
    if (readable_range(name, length + 1u)) {
        for (i = 0; i < length; ++i) {
            unsigned char a = (unsigned char)name[i];
            unsigned char b = (unsigned char)expected[i];
            if (a >= 'A' && a <= 'Z') a = (unsigned char)(a + ('a' - 'A'));
            if (b >= 'A' && b <= 'Z') b = (unsigned char)(b + ('a' - 'A'));
            if (a != b) return 0;
        }
        return name[length] == 0;
    }
    return valid_tag_name(name) && _stricmp(name, expected) == 0;
}

#include "phase1_profile.h"

static int validate_module(uint8_t **base_out, char *error, size_t error_size) {
    HMODULE module = GetModuleHandleW(L"HaloSimulation_tag_release.dll");
    IMAGE_DOS_HEADER *dos;
    IMAGE_NT_HEADERS64 *nt;
    static const uint8_t sig_init[] = {0x40,0x53,0x55,0x56,0x57,0x41,0x55,0x41};
    static const uint8_t sig_new[] = {0x48,0x89,0x4C,0x24,0x08,0x41,0x54,0x41};
    static const uint8_t sig_health[] = {0x83,0xF9,0xFF,0x0F,0x84,0xA3,0x01,0x00};
    static const uint8_t sig_enter_immediate[] = {0x83,0xF9,0xFF,0x0F,0x84,0x5D,0x03,0x00};
    static const uint8_t sig_submit_action[] = {0x48,0x8B,0xC4,0x55,0x56,0x57,0x41,0x57};
    static const uint8_t sig_weapon_interval[] = {0x48,0x8B,0xC4,0x41,0x57,0x48,0x83,0xEC};
    static const uint8_t sig_weapon_fire[] = {0x48,0x8B,0xC4,0x44,0x88,0x40,0x18,0x66};
    static const uint8_t sig_weapon_fire_pending[] = {
        0x40,0x53,0x55,0x56,0x57,0x41,0x54,0x41,0x55,
        0x41,0x56,0x41,0x57,0x48,0x83,0xEC,0x28};
    static const uint8_t sig_weapon_reset[] = {0x40,0x53,0x55,0x56,0x57,0x41,0x54,0x41};
    if (!module) {
        snprintf(error, error_size, "module HaloSimulation_tag_release.dll absent");
        return 0;
    }
    dos = (IMAGE_DOS_HEADER *)module;
    nt = (IMAGE_NT_HEADERS64 *)((uint8_t *)module + dos->e_lfanew);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || nt->Signature != IMAGE_NT_SIGNATURE ||
        nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC ||
        nt->OptionalHeader.SizeOfImage < MINIMUM_IMAGE_SIZE) {
        snprintf(error, error_size, "module Halo PE64 invalide");
        return 0;
    }
    /* Les editions Steam et Xbox App 1.111.2544.0 ont des empreintes et des
     * metadonnees PE differentes. La compatibilite est donc etablie avec les
     * instructions de chacune des fonctions natives reellement appelees. */
    if (!bytes_equal((uint8_t *)module + 0x5EE570, sig_init, sizeof(sig_init)) ||
        !bytes_equal((uint8_t *)module + 0x5A0FC0, sig_new, sizeof(sig_new)) ||
        !bytes_equal((uint8_t *)module + 0x662470, sig_health, sizeof(sig_health)) ||
        !bytes_equal((uint8_t *)module + 0x662BF0, sig_enter_immediate, sizeof(sig_enter_immediate)) ||
        !bytes_equal((uint8_t *)module + 0x6286F0, sig_submit_action, sizeof(sig_submit_action)) ||
        !bytes_equal((uint8_t *)module + 0x5C97D0, sig_weapon_interval, sizeof(sig_weapon_interval)) ||
        !bytes_equal((uint8_t *)module + 0x5CC1F0, sig_weapon_fire, sizeof(sig_weapon_fire)) ||
        !bytes_equal((uint8_t *)module + 0x5BE600,
                     sig_weapon_fire_pending,
                     sizeof(sig_weapon_fire_pending)) ||
        !bytes_equal((uint8_t *)module + 0x5CE2D0, sig_weapon_reset, sizeof(sig_weapon_reset))) {
        snprintf(error, error_size,
                 "signatures natives incompatibles ts=%08lX image=%08lX",
                 (unsigned long)nt->FileHeader.TimeDateStamp,
                 (unsigned long)nt->OptionalHeader.SizeOfImage);
        return 0;
    }
    if (nt->FileHeader.TimeDateStamp != 0x6A7A740Au ||
        nt->OptionalHeader.SizeOfImage != 0x02CE1000u ||
        !phase1_file_matches(module)) {
        snprintf(error,error_size,"phase1_requires_exact_CU4_simulation_file");
        return 0;
    }
    if (!phase1_code_matches((uint8_t *)module,error,error_size)) return 0;
    *base_out = (uint8_t *)module;
    return 1;
}

static void sibling_path(wchar_t *out, size_t capacity, const wchar_t *filename) {
    DWORD count = GetModuleFileNameW(g_self, out, (DWORD)capacity);
    wchar_t *slash;
    if (!count || count >= capacity) { out[0] = L'\0'; return; }
    slash = wcsrchr(out, L'\\');
    if (!slash) { out[0] = L'\0'; return; }
    *(slash + 1) = L'\0';
    wcsncat(out, filename, capacity - wcslen(out) - 1);
}

static int read_text(const wchar_t *path, char *buffer, DWORD capacity) {
    HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD read = 0;
    if (file == INVALID_HANDLE_VALUE) return 0;
    if (!ReadFile(file, buffer, capacity - 1, &read, NULL)) { CloseHandle(file); return 0; }
    CloseHandle(file);
    buffer[read] = '\0';
    return 1;
}

static void write_result(const char *text) {
    wchar_t path[MAX_PATH];
    HANDLE file;
    DWORD written;
    sibling_path(path, MAX_PATH, L"native_result.txt");
    file = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return;
    WriteFile(file, text, (DWORD)strlen(text), &written, NULL);
    CloseHandle(file);
}

/* Pont minuscule et deterministe vers le Blueprint audio. Lua lit ces atomiques
 * directement par halosandbox_pelican_bridge : aucune ouverture/ecriture de
 * fichier n'est permise sur le chemin temps reel du tir. */
static int pelican_publish_audio_signal(int active) {
    LONG normalized = active ? 1 : 0;
    LONG previous = InterlockedExchange(
        &g_pelican_audio_signal_state, normalized);
    if (previous == normalized) return 1;
    InterlockedIncrement(&g_pelican_audio_signal_sequence);
    if (normalized) ++g_pelican_audio_fire_events;
    return 1;
}

/* UE4SS peut liberer une bibliotheque chargee par Lua avant la fin complete
 * du processus. Nos relais XInput/resultat vivent sur des threads natifs : le
 * module doit donc rester resident jusqu'a ExitProcess, sinon un thread peut
 * reprendre dans du code deja demappe pendant la fermeture du jeu. */
static void pin_module_lifetime(void) {
    HMODULE pinned = NULL;
    if (InterlockedCompareExchange(&g_self_pinned, 1, 0) != 0) return;
    if (!GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_PIN,
            (LPCWSTR)(const void *)&g_self, &pinned))
        InterlockedExchange(&g_self_pinned, 0);
}

/* Une réservation appartient au worker qui a obtenu l'état 4. Quiesce ne la
 * remet jamais directement à zéro : il la marque 9 et laisse ce propriétaire
 * terminer le reset, ce qui évite qu'une ancienne commande écrase la suivante
 * après un changement de mission. */
static void abandon_reserved_state(void) {
    LONG previous = InterlockedCompareExchange(
        &g_state, NATIVE_STATE_IDLE, NATIVE_STATE_RESERVED);
    if (previous == NATIVE_STATE_RESERVED_QUIESCED)
        finish_quiesced_owner(NATIVE_STATE_RESERVED_QUIESCED);
}

/* Même principe après l'injection du stub : l'état 10 ne peut être libéré que
 * par le stub lui-même ou par le producteur si l'injection a échoué. */
static void abandon_queued_state(void) {
    LONG previous = InterlockedCompareExchange(
        &g_state, NATIVE_STATE_IDLE, NATIVE_STATE_QUEUED);
    if (previous == NATIVE_STATE_QUEUED_QUIESCED)
        finish_quiesced_owner(NATIVE_STATE_QUEUED_QUIESCED);
}

static DWORD WINAPI result_worker(void *unused) {
    (void)unused;
    for (;;) {
        if (InterlockedCompareExchange(&g_process_detaching, 0, 0) != 0)
            break;
        /* Réserver l'écriture avant de toucher au fichier. L'ancien test 2/2
         * suivi d'un Exchange(0) pouvait subir un ABA : quiesce annulait 2,
         * une nouvelle commande réservait l'état, puis l'ancien worker
         * écrasait cette nouvelle réservation avec zéro. */
        if (InterlockedCompareExchange(
                &g_state, NATIVE_STATE_RESULT_WRITING,
                NATIVE_STATE_RESULT_READY) == NATIVE_STATE_RESULT_READY) {
            LONG previous;
            write_result(g_result);
            previous = InterlockedCompareExchange(
                &g_state, NATIVE_STATE_IDLE, NATIVE_STATE_RESULT_WRITING);
            if (previous == NATIVE_STATE_RESULT_WRITING_QUIESCED)
                finish_quiesced_owner(NATIVE_STATE_RESULT_WRITING_QUIESCED);
            Sleep(1);
        } else {
            /* Le resultat est lu par Lua toutes les 20 ms. Se reveiller 200
             * fois/s lorsque le mod est inactif ne reduisait donc aucune
             * latence observable. */
            Sleep(20);
        }
    }
    return 0;
}

/* The game build uses Win32Async for UE4SS bindings.  Poll XInput outside the
 * Unreal game thread and translate only the sandbox-menu controls to virtual
 * keyboard events.  The Lua menu writes menu_state.txt, so ordinary gameplay
 * buttons are never translated. */
typedef DWORD (WINAPI *xinput_get_state_fn)(DWORD, XINPUT_STATE *);

static int menu_is_open(void) {
    wchar_t path[MAX_PATH];
    char text[16];
    LONG64 now = (LONG64)GetTickCount64();
    LONG64 previous = InterlockedCompareExchange64(
        &g_menu_state_checked_at, 0, 0);
    /* Le Lua de cette version publie directement l'etat dans la DLL. Le
     * fichier reste uniquement le repli des anciennes versions du script et
     * du relais XInput separe. */
    if (InterlockedCompareExchange(&g_menu_state_direct, 0, 0) != 0)
        return InterlockedCompareExchange(&g_menu_state_cached, 0, 0) != 0;
    if (previous != 0 && now - previous >= 0 && now - previous < 100)
        return InterlockedCompareExchange(&g_menu_state_cached, 0, 0) != 0;
    if (InterlockedCompareExchange64(
            &g_menu_state_checked_at, now, previous) != previous)
        return InterlockedCompareExchange(&g_menu_state_cached, 0, 0) != 0;
    sibling_path(path, MAX_PATH, L"menu_state.txt");
    InterlockedExchange(&g_menu_state_cached,
        read_text(path, text, sizeof(text)) && text[0] == '1');
    return InterlockedCompareExchange(&g_menu_state_cached, 0, 0) != 0;
}

static int halo_window_focused(void) {
    DWORD pid = 0;
    HWND window = GetForegroundWindow();
    if (!window) return 0;
    GetWindowThreadProcessId(window, &pid);
    return pid == GetCurrentProcessId();
}

static DWORD find_main_thread(void) {
    DWORD pid = GetCurrentProcessId();
    DWORD found = 0;
    ULARGE_INTEGER newest_creation = {0};
    DWORD candidates = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 entry = {0};
    get_thread_description_fn get_description = (get_thread_description_fn)GetProcAddress(
        GetModuleHandleW(L"kernel32.dll"), "GetThreadDescription");
    if (snapshot == INVALID_HANDLE_VALUE) return 0;
    if (!get_description) {
        CloseHandle(snapshot);
        return 0;
    }
    entry.dwSize = sizeof(entry);
    if (Thread32First(snapshot, &entry)) {
        do {
            if (entry.th32OwnerProcessID == pid) {
                HANDLE thread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, entry.th32ThreadID);
                PWSTR description = NULL;
                if (thread && SUCCEEDED(get_description(thread, &description)) && description) {
                    if (wcscmp(description, L"game_MAIN_THREAD") == 0) {
                        FILETIME creation_time, exit_time, kernel_time, user_time;
                        ULARGE_INTEGER creation_value = {0};
                        ++candidates;
                        if (GetThreadTimes(thread, &creation_time, &exit_time, &kernel_time, &user_time)) {
                            creation_value.LowPart = creation_time.dwLowDateTime;
                            creation_value.HighPart = creation_time.dwHighDateTime;
                        }
                        /* Un changement/rechargement de mission peut laisser un ancien
                         * thread portant le meme nom. Le premier de l'enumeration est
                         * alors inactif et la commande reste eternellement "queued".
                         * Le thread cree le plus recemment appartient a la carte active. */
                        if (!found || creation_value.QuadPart >= newest_creation.QuadPart) {
                            found = entry.th32ThreadID;
                            newest_creation = creation_value;
                        }
                    }
                    LocalFree(description);
                }
                if (thread) CloseHandle(thread);
            }
        } while (Thread32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    g_last_main_thread_id = found;
    g_last_main_thread_candidates = candidates;
    return found;
}

static int dispatch_once(char *error, size_t error_size) {
    DWORD main_tid = g_last_main_thread_id;
    DWORD previous_suspend_count;
    HANDLE main_thread;
    CONTEXT context = {0};
    unsigned int stub_retry_count = 0;
    /* La mission garde le meme game_MAIN_THREAD. Le scan Toolhelp de tous les
     * threads systeme n'est necessaire qu'au premier appel; quiesce invalide
     * explicitement ce cache lors de chaque LoadMap/InitGameState. */
    if (!main_tid) main_tid = find_main_thread();
    if (!main_tid) {
        snprintf(error, error_size, "thread game_MAIN_THREAD introuvable");
        return 0;
    }
    if (main_tid == GetCurrentThreadId()) {
        snprintf(error, error_size, "commande deja sur game_MAIN_THREAD");
        return 0;
    }
retry_main_thread:
    main_thread = NULL;
    memset(&context, 0, sizeof(context));
    main_thread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT |
                             THREAD_SET_CONTEXT | THREAD_QUERY_INFORMATION,
                             FALSE, main_tid);
    if (!main_thread && g_last_main_thread_id == main_tid) {
        g_last_main_thread_id = 0;
        main_tid = find_main_thread();
        if (main_tid)
            main_thread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT |
                                     THREAD_SET_CONTEXT | THREAD_QUERY_INFORMATION,
                                     FALSE, main_tid);
    }
    if (!main_thread) {
        snprintf(error, error_size, "ouverture game_MAIN_THREAD impossible");
        return 0;
    }
    previous_suspend_count = SuspendThread(main_thread);
    if (previous_suspend_count == (DWORD)-1) {
        CloseHandle(main_thread);
        snprintf(error, error_size, "suspension game_MAIN_THREAD impossible");
        return 0;
    }
    if (previous_suspend_count != 0) {
        ResumeThread(main_thread);
        CloseHandle(main_thread);
        snprintf(error, error_size, "game_MAIN_THREAD deja suspendu count=%lu",
                 (unsigned long)previous_suspend_count);
        return 0;
    }
    context.ContextFlags = CONTEXT_CONTROL;
    if (!GetThreadContext(main_thread, &context)) {
        ResumeThread(main_thread);
        CloseHandle(main_thread);
        snprintf(error, error_size, "lecture contexte game_MAIN_THREAD impossible");
        return 0;
    }
    /* Le handler C peut publier son résultat avant la toute dernière
     * restauration assembleur. Ne jamais imbriquer un second stub dans cette
     * fenêtre : son premier push écraserait la continuation du premier. */
    if ((uintptr_t)context.Rip >= (uintptr_t)&halosandbox_dispatch_stub &&
        (uintptr_t)context.Rip <
        ((uintptr_t)&halosandbox_dispatch_stub + PELICAN_DISPATCH_STUB_WINDOW_BYTES)) {
        ResumeThread(main_thread);
        CloseHandle(main_thread);
        ++stub_retry_count;
        if (stub_retry_count <= 16u) {
            Sleep(stub_retry_count <= 4u ? 0u : 1u);
            goto retry_main_thread;
        }
        snprintf(error, error_size,
                 "restauration game_MAIN_THREAD encore active");
        return 0;
    }
    g_resume_rip = (void *)(uintptr_t)context.Rip;
    MemoryBarrier();
    context.Rip = (DWORD64)(uintptr_t)&halosandbox_dispatch_stub;
    if (!SetThreadContext(main_thread, &context)) {
        ResumeThread(main_thread);
        CloseHandle(main_thread);
        snprintf(error, error_size, "ecriture contexte game_MAIN_THREAD impossible");
        return 0;
    }
    ResumeThread(main_thread);
    CloseHandle(main_thread);
    return 1;
}

/* Le trait retail bottomless reste le chemin prefere, mais certaines sessions
 * campagne ne l'honorent pas apres leur initialisation reseau. Ce garde de
 * secours ne programme qu'une lecture courte par seconde sur game_MAIN_THREAD
 * et refill_unit_ammo n'appelle ensuite un setter que si une valeur a baisse.
 * L'epoch rend inoffensif un tick deja reserve lorsque Lua annonce un LoadMap
 * ou un InitGameState. */
#define AMMO_MAINTENANCE_INTERVAL_MS 1000u

static DWORD WINAPI ammo_maintenance_worker(void *unused) {
    ULONGLONG last_maintenance = GetTickCount64();
    (void)unused;
    for (;;) {
        ULONGLONG now;
        LONG unit_datum;
        LONG epoch;
        LONG dispatch_epoch;
        char error[128];
        if (InterlockedCompareExchange(&g_process_detaching, 0, 0) != 0)
            break;
        now = GetTickCount64();
        dispatch_epoch = InterlockedCompareExchange(&g_dispatch_epoch, 0, 0);
        if (InterlockedCompareExchange(
                &g_ammo_maintenance_enabled, 0, 0) != 0 &&
            halo_window_focused() &&
            now - last_maintenance >= AMMO_MAINTENANCE_INTERVAL_MS &&
            InterlockedCompareExchange(&g_external_pending, 0, 0) == 0 &&
            InterlockedCompareExchange(&g_state, 4, 0) == 0) {
            if (dispatch_epoch !=
                InterlockedCompareExchange(&g_dispatch_epoch, 0, 0)) {
                abandon_reserved_state();
                continue;
            }
            unit_datum = InterlockedCompareExchange(
                &g_ammo_maintenance_unit, -1, -1);
            epoch = InterlockedCompareExchange(
                &g_ammo_maintenance_epoch, 0, 0);
            last_maintenance = now;
            if (unit_datum != -1 &&
                InterlockedCompareExchange(
                    &g_ammo_maintenance_enabled, 0, 0) != 0 &&
                InterlockedCompareExchange(
                    &g_process_detaching, 0, 0) == 0) {
                memset(&g_command, 0, sizeof(g_command));
                g_command.kind = COMMAND_AMMO_MAINTENANCE;
                g_command.dispatch_epoch = dispatch_epoch;
                g_command.a = unit_datum;
                g_command.b = epoch;
                InterlockedExchange(&g_internal_dispatch, 1);
                MemoryBarrier();
                if (InterlockedCompareExchange(
                        &g_state, NATIVE_STATE_QUEUED,
                        NATIVE_STATE_RESERVED) != NATIVE_STATE_RESERVED) {
                    InterlockedExchange(&g_internal_dispatch, 0);
                    finish_quiesced_owner(NATIVE_STATE_RESERVED_QUIESCED);
                } else if (!dispatch_once(error, sizeof(error))) {
                    InterlockedExchange(&g_internal_dispatch, 0);
                    abandon_queued_state();
                }
            } else {
                abandon_reserved_state();
            }
        }
        Sleep(InterlockedCompareExchange(
                  &g_ammo_maintenance_enabled, 0, 0) != 0 ? 100 : 500);
    }
    return 0;
}

static int start_ammo_maintenance_worker(void) {
    HANDLE thread;
    if (InterlockedCompareExchange(&g_ammo_worker_started, 1, 0) != 0)
        return 1;
    thread = CreateThread(NULL, 0, ammo_maintenance_worker, NULL, 0, NULL);
    if (!thread) {
        InterlockedExchange(&g_ammo_worker_started, 0);
        return 0;
    }
    CloseHandle(thread);
    return 1;
}

static int enable_ammo_maintenance(int32_t unit_datum) {
    if (unit_datum == -1 || !is_live_player_unit(unit_datum) ||
        !start_ammo_maintenance_worker()) return 0;
    InterlockedExchange(&g_ammo_maintenance_enabled, 0);
    InterlockedIncrement(&g_ammo_maintenance_epoch);
    InterlockedExchange(&g_ammo_maintenance_unit, unit_datum);
    MemoryBarrier();
    InterlockedExchange(&g_ammo_maintenance_enabled, 1);
    return 1;
}

static void disable_ammo_maintenance(void) {
    InterlockedExchange(&g_ammo_maintenance_enabled, 0);
    InterlockedIncrement(&g_ammo_maintenance_epoch);
    MemoryBarrier();
    InterlockedExchange(&g_ammo_maintenance_unit, -1);
}

/* La gachette est lue hors du thread du jeu, puis l'etat de l'arme est
 * applique exclusivement sur game_MAIN_THREAD. Cette separation evite tout
 * acces concurrent aux objets Blam et laisse le moteur publier normalement
 * le tir dans la simulation coop. Clic gauche et gachette droite commandent
 * la vraie arme du module, jamais l'arme tenue par le Spartan. */
static DWORD WINAPI pelican_input_worker(void *unused) {
    HMODULE xinput = NULL;
    xinput_get_state_fn get_state = NULL;
    ULONGLONG last_validation = 0;
    int last_local_requested = 0;
    (void)unused;
    xinput = LoadLibraryW(L"xinput1_4.dll");
    if (!xinput) xinput = LoadLibraryW(L"xinput1_3.dll");
    if (!xinput) xinput = LoadLibraryW(L"xinput9_1_0.dll");
    if (xinput) get_state = (xinput_get_state_fn)GetProcAddress(xinput, "XInputGetState");
    for (;;) {
        LONG input_hold_ticks = 0;
        int requested = 0;
        int raw_requested = 0;
        ULONGLONG now = GetTickCount64();
        LONG dispatch_epoch = InterlockedCompareExchange(
            &g_dispatch_epoch, 0, 0);
        if (InterlockedCompareExchange(&g_process_detaching, 0, 0) != 0)
            break;
        if (InterlockedCompareExchange(&g_pelican_armed, 0, 0) != 0 &&
            halo_window_focused() && !menu_is_open()) {
            XINPUT_STATE state;
            raw_requested = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            if (get_state) {
                memset(&state, 0, sizeof(state));
                if (get_state(0, &state) == ERROR_SUCCESS &&
                    state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
                    raw_requested = 1;
            }
        }
#ifdef HALOSANDBOX_RUNTIME_TEST
        /* Le banc peut maintenir la detente sans voler le focus Windows. Le
         * game thread verifiera encore le conducteur, son parent et l'arme. */
        if (InterlockedCompareExchange(&g_pelican_armed, 0, 0) != 0 &&
            !menu_is_open() &&
            InterlockedCompareExchange(
                &g_pelican_runtime_test_primary, 0, 0) != 0)
            raw_requested = 1;
#endif
        if (InterlockedCompareExchange(&g_pelican_armed, 0, 0) == 0) {
            InterlockedExchange(&g_pelican_local_input_hold_ticks, 0);
            requested = 0;
        } else if (raw_requested) {
            InterlockedExchange(&g_pelican_local_input_hold_ticks,
                                (LONG)PELICAN_LOCAL_INPUT_HOLD_TICKS);
            input_hold_ticks = PELICAN_LOCAL_INPUT_HOLD_TICKS;
            requested = 1;
        } else {
            input_hold_ticks = InterlockedCompareExchange(
                &g_pelican_local_input_hold_ticks, 0, 0);
            if (input_hold_ticks > 0) {
                --input_hold_ticks;
                requested = 1;
            }
            InterlockedExchange(&g_pelican_local_input_hold_ticks, input_hold_ticks);
        }
        InterlockedExchange(&g_pelican_requested_primary, requested);
        /* Une pression locale est transmise immediatement. Pendant un tir, le
         * rythme vient du barrel d'origine (avec plafond de securite a 20 Hz).
         * Un conducteur distant, y compris apres une sortie/remontee, est
         * detecte sous 250 ms par le listen-server. */
        {
            int driver_present = InterlockedCompareExchange(
                &g_pelican_driver_present, 0, 0) != 0;
            int applied = InterlockedCompareExchange(
                &g_pelican_applied_primary, 0, 0) != 0;
            ULONGLONG interval;
            (void)applied;
            /* Le moteur cadence lui-meme les coups avec le barrel authored.
             * Ce worker ne fait que relayer entree et visee a 30 Hz. Les
             * fronts de gachette restent immediats via le test ci-dessus. */
            interval = driver_present ? 33u : 250u;
        if (InterlockedCompareExchange(&g_pelican_armed, 0, 0) != 0 &&
            (requested != last_local_requested ||
             now - last_validation >= interval) &&
            InterlockedCompareExchange(&g_external_pending, 0, 0) == 0 &&
            InterlockedCompareExchange(
                &g_state, NATIVE_STATE_RESERVED, NATIVE_STATE_IDLE) ==
                    NATIVE_STATE_IDLE) {
            char error[128];
            /* La reservation est la frontiere de synchronisation. Revalider
             * ensuite tous les verrous ferme la fenetre ou un quiesce peut
             * finir entre la lecture de armed et le CAS ci-dessus. */
            if (InterlockedCompareExchange(&g_pelican_armed, 0, 0) == 0 ||
                InterlockedCompareExchange(&g_process_detaching, 0, 0) != 0 ||
                InterlockedCompareExchange(&g_external_pending, 0, 0) != 0 ||
                dispatch_epoch !=
                    InterlockedCompareExchange(&g_dispatch_epoch, 0, 0)) {
                abandon_reserved_state();
                continue;
            }
            memset(&g_command, 0, sizeof(g_command));
            g_command.kind = COMMAND_PELICAN_INPUT_TICK;
            g_command.dispatch_epoch = dispatch_epoch;
            InterlockedExchange(&g_internal_dispatch, 1);
            MemoryBarrier();
            if (InterlockedCompareExchange(
                    &g_state, NATIVE_STATE_QUEUED,
                    NATIVE_STATE_RESERVED) != NATIVE_STATE_RESERVED) {
                InterlockedExchange(&g_internal_dispatch, 0);
                finish_quiesced_owner(NATIVE_STATE_RESERVED_QUIESCED);
            } else if (!dispatch_once(error, sizeof(error))) {
                InterlockedExchange(&g_internal_dispatch, 0);
                abandon_queued_state();
            } else {
                last_validation = now;
                last_local_requested = requested;
            }
        }
        }
        Sleep(InterlockedCompareExchange(
                  &g_pelican_armed, 0, 0) != 0 ? 15 : 250);
    }
    if (xinput) FreeLibrary(xinput);
    return 0;
}

static void start_pelican_worker(void) {
    if (InterlockedCompareExchange(&g_pelican_worker_started, 1, 0) == 0) {
        HANDLE thread = CreateThread(NULL, 0, pelican_input_worker, NULL, 0, NULL);
        if (thread) CloseHandle(thread);
        else InterlockedExchange(&g_pelican_worker_started, 0);
    }
}

#ifdef HALOSANDBOX_RUNTIME_TEST
/* Retrouve uniquement, pour le banc local, le joueur vivant qui occupe le
 * siege conducteur puis le module arme physiquement attache a son vehicule.
 * Aucune heuristique de tag n'est conservee dans la version de production. */
static void pelican_runtime_test_arm_main_thread(void) {
    uint8_t *thread_state = blam_thread_state();
    uint8_t *table;
    uint8_t *entries;
    uint32_t *allocation_bits;
    int32_t capacity;
    int32_t high;
    int32_t pilot_datum = -1;
    int32_t vehicle_datum = -1;
    int32_t module_datum = -1;
    int32_t weapon_datum = -1;
    int32_t index;
    uint32_t fire_interval_ms = 0;
    uint8_t shot_serial = 0;
    int weapon_slot = -1;
    uint16_t current_before = 0xFFFFu;
    uint16_t desired_before = 0xFFFFu;
    uint32_t unit_flags_before = 0;
    int presentation_patched = 0;
    int impact_patched = 0;
    int tracer_patched = 0;

    if (!thread_state || !readable_range(thread_state + 0x20, sizeof(void *))) {
        snprintf(g_result, sizeof(g_result),
                 "error pelican_runtime_test_arm object_tls_missing\n");
        return;
    }
    table = *(uint8_t **)(thread_state + 0x20);
    if (!table || !readable_range(table, 0x60u) ||
        *(uint64_t *)(table + 0x20) != 24u) {
        snprintf(g_result, sizeof(g_result),
                 "error pelican_runtime_test_arm object_table_invalid\n");
        return;
    }
    capacity = *(int32_t *)(table + 0x2C);
    high = *(int32_t *)(table + 0x44);
    entries = *(uint8_t **)(table + 0x50);
    allocation_bits = *(uint32_t **)(table + 0x58);
    if (capacity <= 0 || capacity > 65536 || high < 0 || high > capacity ||
        !entries || !allocation_bits ||
        !readable_range(allocation_bits,
            (((size_t)capacity + 31u) / 32u) * sizeof(uint32_t))) {
        snprintf(g_result, sizeof(g_result),
                 "error pelican_runtime_test_arm object_metadata_invalid capacity=%d high=%d\n",
                 capacity, high);
        return;
    }

    for (index = 0; index < high; ++index) {
        uint32_t slot = (uint32_t)index;
        uint8_t *entry;
        uint8_t *unit;
        int32_t datum;
        int32_t parent;
        uint8_t *vehicle;
        if (!(allocation_bits[slot >> 5] & (1u << (slot & 31u)))) continue;
        entry = entries + (size_t)slot * 24u;
        if (!readable_range(entry, 24u)) continue;
        datum = (int32_t)(((uint32_t)*(uint16_t *)entry << 16) | slot);
        unit = object_data(datum);
        if (!unit || !is_live_player_unit(datum) ||
            !readable_range(unit + 0x14, sizeof(parent))) continue;
        parent = *(int32_t *)(unit + 0x14);
        vehicle = object_data(parent);
        if (!vehicle || !is_live_vehicle_object(parent) ||
            !readable_range(vehicle + UNIT_DRIVER_OBJECT_OFFSET,
                            sizeof(int32_t)) ||
            *(int32_t *)(vehicle + UNIT_DRIVER_OBJECT_OFFSET) != datum)
            continue;
        pilot_datum = datum;
        vehicle_datum = parent;
        break;
    }
    if (pilot_datum == -1 || vehicle_datum == -1) {
        snprintf(g_result, sizeof(g_result),
                 "error pelican_runtime_test_arm no_live_driver\n");
        return;
    }

    for (index = 0; index < high; ++index) {
        uint32_t slot = (uint32_t)index;
        uint8_t *entry;
        uint8_t *object;
        int32_t datum;
        int32_t parent;
        int32_t weapon;
        if (!(allocation_bits[slot >> 5] & (1u << (slot & 31u)))) continue;
        entry = entries + (size_t)slot * 24u;
        if (!readable_range(entry, 24u)) continue;
        datum = (int32_t)(((uint32_t)*(uint16_t *)entry << 16) | slot);
        if (datum == pilot_datum || is_live_player_unit(datum)) continue;
        object = object_data(datum);
        if (!object || !readable_range(object + 0x14, sizeof(parent))) continue;
        parent = *(int32_t *)(object + 0x14);
        if (parent != vehicle_datum) continue;
        weapon = first_unit_weapon(datum);
        if (weapon == -1 || !pelican_primary_barrel_info(
                weapon, -1, &fire_interval_ms, &shot_serial)) continue;
        module_datum = datum;
        weapon_datum = weapon;
        break;
    }
    if (module_datum == -1 || weapon_datum == -1) {
        snprintf(g_result, sizeof(g_result),
                 "error pelican_runtime_test_arm no_attached_armed_module pilot=%d vehicle=%d\n",
                 pilot_datum, vehicle_datum);
        return;
    }
    if (!select_unit_vehicle_weapon(
            module_datum, weapon_datum, &weapon_slot,
            &current_before, &desired_before, &unit_flags_before)) {
        snprintf(g_result, sizeof(g_result),
                 "error pelican_runtime_test_arm weapon_selection_failed module=%d weapon=%d\n",
                 module_datum, weapon_datum);
        return;
    }
    pelican_release_relay_main_thread(0u);
    set_weapon_primary_firing(weapon_datum, 0);
    clear_unit_vehicle_primary_control(module_datum);
    if (!pelican_prepare_direct_barrel(weapon_datum)) {
        snprintf(g_result, sizeof(g_result),
                 "error pelican_runtime_test_arm barrel_not_idle weapon=%d\n",
                 weapon_datum);
        return;
    }
    presentation_patched =
        pelican_patch_human_presentation_for_weapon(weapon_datum);
    if (!presentation_patched) {
        snprintf(g_result, sizeof(g_result),
                 "error pelican_runtime_test_arm presentation_patch_failed module=%d weapon=%d code=%d\n",
                 module_datum, weapon_datum,
                 g_pelican_presentation_patch_error);
        return;
    }
    impact_patched = 0;
    g_pelican_aux_impact_projectile_tag = -1;
    tracer_patched = pelican_patch_tracer_for_weapon(weapon_datum);
    if (!tracer_patched) {
        pelican_rollback_failed_tracer_arm_patch();
        snprintf(g_result, sizeof(g_result),
                 "error pelican_runtime_test_arm tracer_patch_failed module=%d weapon=%d code=%d active=%d\n",
                 module_datum, weapon_datum,
                 g_pelican_tracer_patch_error,
                 g_pelican_tracer_patch_active);
        return;
    }
    ((object_hide_fn)(g_blam + 0x5A5FD0))(module_datum, 0);
    sync_unit_vehicle_aim(vehicle_datum, module_datum);
    g_pelican_local_pilot = pilot_datum;
    g_pelican_vehicle = vehicle_datum;
    g_pelican_chin_module = module_datum;
    g_pelican_chin_weapon = weapon_datum;
    pelican_direct_gate_clear(weapon_datum);
    g_pelican_completed_shots = 0;
    g_pelican_last_shot_serial = shot_serial;
    InterlockedExchange(&g_pelican_fire_interval_ms,
                        (LONG)fire_interval_ms);
    InterlockedExchange(&g_pelican_requested_primary, 0);
    InterlockedExchange(&g_pelican_applied_primary, 0);
    InterlockedExchange(&g_pelican_driver_present, 1);
    InterlockedExchange(&g_pelican_local_input_hold_ticks, 0);
    InterlockedExchange(&g_pelican_pilot_absence_ticks, 0);
    InterlockedExchange(&g_pelican_last_driver_datum, pilot_datum);
    InterlockedExchange(&g_pelican_last_target_ok, 0);
    InterlockedExchange(&g_pelican_last_aim_ok, 0);
    MemoryBarrier();
    InterlockedExchange(&g_pelican_last_release_reason, 0);
    pelican_reset_validation_diagnostics();
    InterlockedExchange(&g_pelican_armed, 1);
    start_pelican_worker();
    snprintf(g_result, sizeof(g_result),
             "ok pelican_runtime_test_arm pilot=%d vehicle=%d module=%d weapon=%d slot=%d interval_ms=%u serial=%u presentation_patch=%d presentation_error=%d presentation_effect=%d impact_patch=%d impact_error=%d impact_projectile=%d impact_effect=%d impact_slots=%02X impact_material_radius=%.4f tracer_patch=%d tracer_error=%d tracer_original=%u tracer_current=0\n",
             pilot_datum, vehicle_datum, module_datum, weapon_datum,
             weapon_slot, fire_interval_ms, (unsigned)shot_serial,
             presentation_patched, g_pelican_presentation_patch_error,
             g_pelican_presentation_effect, impact_patched,
             g_pelican_impact_patch_error,
             g_pelican_impact_projectile_tag,
             g_pelican_impact_effect,
             (unsigned)g_pelican_impact_original_valid_mask,
             (double)g_pelican_impact_material_radius,
             tracer_patched,
             g_pelican_tracer_patch_error,
             (unsigned)g_pelican_tracer_original_rounds);
}
#endif

static int unlock_vehicle_seats(int32_t tag_index, uint32_t *count_out) {
    uint32_t count = 0;
    uint8_t *seats = resolve_seats(tag_index, &count);
    DWORD old_protect;
    uint32_t i;
    const uint32_t clear_mask = (1u << 1) | (1u << 13) | (1u << 15) | (1u << 26);
    if (!seats) return 0;
    if (!VirtualProtect(seats, (size_t)count * 328u, PAGE_READWRITE, &old_protect)) return 0;
    for (i = 0; i < count; ++i) {
        uint8_t *seat = seats + i * 328u;
        *(uint32_t *)seat &= ~clear_mask;
        if (*(float *)(seat + 284) < 4.0f) *(float *)(seat + 284) = 4.0f;
    }
    VirtualProtect(seats, (size_t)count * 328u, old_protect, &old_protect);
    if (count_out) *count_out = count;
    return 1;
}

static int valid_transform(const struct native_command *command) {
    const float values[] = {
        command->x, command->y, command->z,
        command->fx, command->fy, command->fz,
        command->ux, command->uy, command->uz
    };
    float forward_length_sq;
    float up_length_sq;
    size_t i;
    for (i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
        if (!isfinite(values[i]) || values[i] < -1000000.0f || values[i] > 1000000.0f) return 0;
    }
    forward_length_sq = command->fx * command->fx + command->fy * command->fy + command->fz * command->fz;
    up_length_sq = command->ux * command->ux + command->uy * command->uy + command->uz * command->uz;
    return forward_length_sq > 0.25f && forward_length_sq < 2.25f &&
           up_length_sq > 0.25f && up_length_sq < 2.25f;
}

static int32_t spawn_at_transform(const struct native_command *command) {
    _Alignas(16) uint8_t placement[0x330];
    float *position = (float *)(placement + 0x1C);
    float *forward = (float *)(placement + 0x28);
    float *up = (float *)(placement + 0x34);
    ((placement_init_fn)(g_blam + 0x5EE570))(placement, command->b, -1, NULL);
    position[0] = command->x;
    position[1] = command->y;
    position[2] = command->z;
    forward[0] = command->fx;
    forward[1] = command->fy;
    forward[2] = command->fz;
    up[0] = command->ux;
    up[1] = command->uy;
    up[2] = command->uz;
    return ((object_new_fn)(g_blam + 0x5A0FC0))(placement);
}

/* Cree un enfant deja attache dans son placement initial. HCE lit ces trois
 * champs pendant object_new et execute lui-meme l'attache avant le premier
 * snapshot de l'objet. C'est indispensable en coop : un object_attach appele
 * apres la creation peut laisser le client observer un module libre. */
static int32_t spawn_attached_at_transform(const struct native_command *command,
                                           int32_t parent_datum,
                                           uint32_t parent_marker,
                                           uint32_t child_marker) {
    _Alignas(16) uint8_t placement[0x330];
    float *position = (float *)(placement + 0x1C);
    float *forward = (float *)(placement + 0x28);
    float *up = (float *)(placement + 0x34);
    ((placement_init_fn)(g_blam + 0x5EE570))(
        placement, command->b, -1, NULL);
    position[0] = command->x;
    position[1] = command->y;
    position[2] = command->z;
    forward[0] = command->fx;
    forward[1] = command->fy;
    forward[2] = command->fz;
    up[0] = command->ux;
    up[1] = command->uy;
    up[2] = command->uz;
    *(int32_t *)(placement + 0x2C4) = parent_datum;
    *(uint32_t *)(placement + 0x2C8) = parent_marker;
    *(uint32_t *)(placement + 0x2CC) = child_marker;
    return ((object_new_fn)(g_blam + 0x5A0FC0))(placement);
}

static uint8_t submit_enter_action(int32_t unit, int32_t vehicle, int32_t seat) {
    _Alignas(16) uint8_t action[0x50] = {0};
    *(int32_t *)(action + 0x00) = 0x13;
    *(int32_t *)(action + 0x04) = vehicle;
    *(uint8_t *)(action + 0x08) = 7;
    *(int16_t *)(action + 0x0A) = (int16_t)seat;
    *(uint8_t *)(action + 0x0C) = 1;
    return ((unit_submit_action_fn)(g_blam + 0x6286F0))(unit, action);
}

/* Meme action que HaloScript unit_exit_vehicle et ai_vehicle_exit. Elle suit
 * les validations de siege et le second handler de simulation, contrairement
 * a l'action 0x19 qui est une ejection forcee reservee au diagnostic. */
static uint8_t submit_normal_exit_action(int32_t unit) {
    _Alignas(16) uint8_t action[0x50] = {0};
    *(int32_t *)(action + 0x00) = 0x0F;
    *(uint8_t *)(action + 0x05) = 1;
    return ((unit_submit_action_fn)(g_blam + 0x6286F0))(unit, action);
}

static uint8_t submit_force_exit_action(int32_t unit) {
    _Alignas(16) uint8_t action[0x50] = {0};
    *(int32_t *)(action + 0x00) = 0x19;
    *(uint8_t *)(action + 0x06) = 1;
    return ((unit_submit_action_fn)(g_blam + 0x6286F0))(unit, action);
}

static int enter_vehicle_immediate(int32_t unit, int32_t vehicle, int32_t tag_index, int32_t seat) {
    uint32_t seat_count = 0;
    uint8_t *seats = resolve_seats(tag_index, &seat_count);
    uint32_t seat_label;
    if (!seats || seat < 0 || (uint32_t)seat >= seat_count) return 0;
    seat_label = *(uint32_t *)(seats + (size_t)seat * 328u + 4u);
    if (!seat_label) return 0;
    ((unit_set_enterable_fn)(g_blam + 0x6053D0))(vehicle, 1);
    ((unit_enter_vehicle_immediate_fn)(g_blam + 0x662BF0))(unit, vehicle, seat_label, 1);
    return 1;
}

static int safe_ai_group_name(const char *name) {
    size_t i;
    if (!name || !name[0]) return 0;
    for (i = 0; i < 32 && name[i]; ++i) {
        unsigned char c = (unsigned char)name[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_')) return 0;
    }
    return i > 0 && i < 32 && name[i] == '\0';
}

static int validate_ai_bridge(void **scenario_out, int *group_count_out, int *squad_count_out,
                              char *error, size_t error_size) {
    static const uint8_t finder_sig[] = {0x48,0x83,0xEC,0x08,0x4C,0x8B,0xDA,0x48,0x85,0xC9};
    static const uint8_t place_sig[] = {0x48,0x89,0x5C,0x24,0x10,0x56,0x48,0x83,0xEC,0x70};
    static const uint8_t iterator_init_sig[] = {0x48,0x89,0x5C,0x24,0x08,0x48,0x89,0x74,0x24,0x10};
    static const uint8_t iterator_next_sig[] = {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x79,0x09,0x00};
    void *scenario;
    int group_count, squad_count;
    if (!bytes_equal(g_blam + 0x0054EE0, finder_sig, sizeof(finder_sig)) ||
        !bytes_equal(g_blam + 0x0054FE0, finder_sig, sizeof(finder_sig)) ||
        !bytes_equal(g_blam + 0x00FD810, place_sig, sizeof(place_sig)) ||
        !bytes_equal(g_blam + 0x00FA680, iterator_init_sig, sizeof(iterator_init_sig)) ||
        !bytes_equal(g_blam + 0x00FA780, iterator_next_sig, sizeof(iterator_next_sig))) {
        snprintf(error, error_size, "signatures IA incompatibles");
        return 0;
    }
    if (!readable_range(g_blam + 0x010C3558, sizeof(void *))) {
        snprintf(error, error_size, "pointeur scenario illisible");
        return 0;
    }
    scenario = *(void **)(g_blam + 0x010C3558);
    if (!scenario || !readable_range((uint8_t *)scenario + 0x33C, 20)) {
        snprintf(error, error_size, "scenario non charge");
        return 0;
    }
    group_count = *(int *)((uint8_t *)scenario + 0x33C);
    squad_count = *(int *)((uint8_t *)scenario + 0x348);
    if (group_count < 0 || group_count > 4096) {
        snprintf(error, error_size, "groupes IA indisponibles count=%d", group_count);
        return 0;
    }
    if (squad_count <= 0 || squad_count > 16384) {
        snprintf(error, error_size, "escouades IA indisponibles count=%d", squad_count);
        return 0;
    }
    *scenario_out = scenario;
    *group_count_out = group_count;
    *squad_count_out = squad_count;
    return 1;
}

struct ai_object_record {
    int32_t datum;
    uint8_t *actor;
};

static char g_actor_lookup_detail[320];

static uint8_t *actor_data_raw(int32_t actor_datum) {
    DWORD tls_index;
    uint8_t *thread_state;
    uint8_t *table;
    uint8_t *entries;
    uint8_t *actor;
    uint32_t slot;
    g_actor_lookup_detail[0] = '\0';
    if (actor_datum == -1) {
        snprintf(g_actor_lookup_detail, sizeof(g_actor_lookup_detail),
                 "datum=-1");
        return NULL;
    }
    tls_index = *(DWORD *)(g_blam + 0xD72730);
    thread_state = blam_thread_state();
    if (!thread_state || !readable_range(thread_state + 0x28, sizeof(void *))) {
        snprintf(g_actor_lookup_detail, sizeof(g_actor_lookup_detail),
                 "tls=%lu thread=%p thread_plus_28_readable=%d",
                 (unsigned long)tls_index, thread_state,
                 thread_state ? readable_range(thread_state + 0x28, sizeof(void *)) : 0);
        return NULL;
    }
    table = *(uint8_t **)(thread_state + 0x28);
    if (!table || !readable_range(table + 0x50, sizeof(void *))) {
        snprintf(g_actor_lookup_detail, sizeof(g_actor_lookup_detail),
                 "tls=%lu thread=%p table=%p table_plus_50_readable=%d",
                 (unsigned long)tls_index, thread_state, table,
                 table ? readable_range(table + 0x50, sizeof(void *)) : 0);
        return NULL;
    }
    entries = *(uint8_t **)(table + 0x50);
    slot = (uint16_t)actor_datum;
    /* La table des acteurs HaloSimulation de cette build utilise une entree
     * fixe de 0xD10 octets. Les champs generiques +0x20/+0x44 ne decrivent pas
     * son stride/count et rejetaient donc a tort les acteurs bien crees. */
    if (!entries || slot >= 16384u) {
        snprintf(g_actor_lookup_detail, sizeof(g_actor_lookup_detail),
                 "tls=%lu thread=%p table=%p entries=%p datum=%08X slot=%u",
                 (unsigned long)tls_index, thread_state, table, entries,
                 (unsigned)actor_datum, (unsigned)slot);
        return NULL;
    }
    actor = entries + (size_t)slot * 0xD10u;
    if (!readable_range(actor, 0x20)) {
        MEMORY_BASIC_INFORMATION info;
        SIZE_T queried = VirtualQuery(actor, &info, sizeof(info));
        snprintf(g_actor_lookup_detail, sizeof(g_actor_lookup_detail),
                 "tls=%lu thread=%p table=%p entries=%p datum=%08X slot=%u actor=%p vq=%zu state=%08lX protect=%08lX base=%p size=%zu",
                 (unsigned long)tls_index, thread_state, table, entries,
                 (unsigned)actor_datum, (unsigned)slot, actor, (size_t)queried,
                 queried ? (unsigned long)info.State : 0ul,
                 queried ? (unsigned long)info.Protect : 0ul,
                 queried ? info.BaseAddress : NULL,
                 queried ? (size_t)info.RegionSize : 0u);
        return NULL;
    }
    if (*(uint16_t *)actor !=
        (uint16_t)((uint32_t)actor_datum >> 16)) {
        snprintf(g_actor_lookup_detail, sizeof(g_actor_lookup_detail),
                 "stale datum=%08X slot=%u expected_salt=%04X actual_salt=%04X",
                 (unsigned)actor_datum, (unsigned)slot,
                 (unsigned)((uint32_t)actor_datum >> 16),
                 (unsigned)*(uint16_t *)actor);
        return NULL;
    }
    snprintf(g_actor_lookup_detail, sizeof(g_actor_lookup_detail),
             "ok tls=%lu thread=%p table=%p entries=%p datum=%08X slot=%u actor=%p salt=%04X unit=%d",
             (unsigned long)tls_index, thread_state, table, entries,
             (unsigned)actor_datum, (unsigned)slot, actor,
             (unsigned)*(uint16_t *)actor, *(int32_t *)(actor + 0x1C));
    return actor;
}

static uint8_t *actor_data(int32_t actor_datum, int32_t unit_datum) {
    uint8_t *actor;
    uint8_t *unit;
    if (actor_datum == -1 || unit_datum == -1) return NULL;
    actor = actor_data_raw(actor_datum);
    unit = object_data(unit_datum);
    if (!actor || !unit || !readable_range(unit + 0x1AC, sizeof(actor_datum))) return NULL;
    if (*(int32_t *)(actor + 0x1C) != unit_datum ||
        *(int32_t *)(unit + 0x1AC) != actor_datum) return NULL;
    return actor;
}

enum ai_loadout_profile {
    AI_LOADOUT_KEEP_DEFAULT = 0,
    AI_LOADOUT_HUMAN = 1,
    AI_LOADOUT_ELITE = 2,
    AI_LOADOUT_GRUNT = 3,
    AI_LOADOUT_JACKAL = 4,
    AI_LOADOUT_FLOOD_COMBAT = 5,
    AI_LOADOUT_BRUTE = 6
};

struct weighted_weapon_entry {
    const char *path;
    uint16_t weight;
};

/* Ces listes ne contiennent que des armes portables originales du jeu. Les
 * poids gardent les armes lourdes et speciales rares, comme dans une escouade
 * de campagne, tout en garantissant que chaque espece utilise un arsenal
 * compatible avec ses animations. */
static const struct weighted_weapon_entry g_fixed_weapons[] = {
{"objects\\weapons\\rifle\\assault_rifle\\assault_rifle",1},
{"objects\\weapons\\pistol\\magnum\\magnum",1},
{"objects\\weapons\\rifle\\shotgun\\shotgun",1},
{"objects\\weapons\\rifle\\sniper_rifle\\sniper_rifle",1},
{"objects\\weapons\\rifle\\plasma_rifle\\plasma_rifle",1},
{"objects\\weapons\\rifle\\plasma_rifle_red\\plasma_rifle_red",1},
{"objects\\weapons\\pistol\\needler\\needler",1},
{"objects\\weapons\\melee\\energy_sword\\energy_sword",1},
{"objects\\weapons\\pistol\\plasma_pistol\\plasma_pistol",1},
{"objects\\weapons\\rifle\\needle_rifle\\needle_rifle",1},
{"objects\\weapons\\rifle\\assault_rifle\\smg",1},
{"objects\\weapons\\rifle\\battle_rifle\\battle_rifle",1},
{"objects\\weapons\\rifle\\beam_rifle\\beam_rifle",1},
{"objects\\weapons\\support_high\\flak_cannon\\flak_cannon",1},
{"objects\\weapons\\support_high\\rocket_launcher\\rocket_launcher",1},
{"objects\\weapons\\support_low\\sentinel_gun\\sentinel_gun",1},
};
static const struct weighted_weapon_entry g_human_weapons[] = {
{"objects\\weapons\\rifle\\assault_rifle\\smg",20},
    { "objects\\weapons\\rifle\\assault_rifle\\assault_rifle", 38 },
    { "objects\\weapons\\pistol\\magnum\\magnum", 22 },
    { "objects\\weapons\\rifle\\shotgun\\shotgun", 18 },
    { "objects\\weapons\\rifle\\sniper_rifle\\sniper_rifle", 12 },

};
static const struct weighted_weapon_entry g_brute_weapons[] = {
{"objects\\weapons\\rifle\\plasma_rifle\\plasma_rifle",45},
{"objects\\weapons\\pistol\\needler\\needler",30},
{"objects\\weapons\\rifle\\needle_rifle\\needle_rifle",25},
};
static const struct weighted_weapon_entry g_elite_weapons[] = {
    { "objects\\weapons\\rifle\\plasma_rifle\\plasma_rifle", 45 },
    { "objects\\weapons\\rifle\\plasma_rifle_red\\plasma_rifle_red", 20 },
    { "objects\\weapons\\pistol\\needler\\needler", 25 },
    { "objects\\weapons\\melee\\energy_sword\\energy_sword", 10 }
};
static const struct weighted_weapon_entry g_grunt_weapons[] = {
    { "objects\\weapons\\pistol\\plasma_pistol\\plasma_pistol", 72 },
    { "objects\\weapons\\pistol\\needler\\needler", 28 }
};
static const struct weighted_weapon_entry g_jackal_weapons[] = {
    { "objects\\weapons\\pistol\\plasma_pistol\\plasma_pistol", 68 },
    { "objects\\weapons\\pistol\\needler\\needler", 32 }
};
static const struct weighted_weapon_entry g_flood_weapons[] = {
    { "objects\\weapons\\rifle\\assault_rifle\\assault_rifle", 24 },
    { "objects\\weapons\\rifle\\shotgun\\shotgun", 20 },
    { "objects\\weapons\\pistol\\magnum\\magnum", 14 },
    { "objects\\weapons\\rifle\\plasma_rifle\\plasma_rifle", 24 },
    { "objects\\weapons\\pistol\\needler\\needler", 18 }
};

static uint32_t g_ai_loadout_rng = 0x7B4A91D3u;

static uint32_t ai_loadout_random(uint32_t mix) {
    uint32_t value = g_ai_loadout_rng ^ mix ^ (uint32_t)GetTickCount();
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    if (!value) value = 0xA341316Cu;
    g_ai_loadout_rng = value;
    return value;
}

static int load_original_weapon_checked(const char *path, int32_t *tag_out) {
    int32_t tag;
    if (tag_out) *tag_out = -1;
    if (!valid_tag_name(path)) return 0;
    tag = ((tag_load_fn)(g_blam + 0x197F60))(0x77656170u, path);
    if (tag == -1 || !((tag_get_fn)(g_blam + 0x201480))(0x77656170u, tag)) {
        return 0;
    }
    if (tag_out) *tag_out = tag;
    return 1;
}

static const struct weighted_weapon_entry *weapons_for_loadout(
    int profile, size_t *count_out) {
    if (count_out) *count_out = 0;
    if (profile>=100 && profile<=115) { if(count_out)*count_out=1;return &g_fixed_weapons[profile-100]; }
    switch (profile) {
    case AI_LOADOUT_HUMAN:
        if (count_out) *count_out = sizeof(g_human_weapons) / sizeof(g_human_weapons[0]);
        return g_human_weapons;
    case AI_LOADOUT_BRUTE:
        if (count_out) *count_out=sizeof(g_brute_weapons)/sizeof(g_brute_weapons[0]);
        return g_brute_weapons;
    case AI_LOADOUT_ELITE:
        if (count_out) *count_out = sizeof(g_elite_weapons) / sizeof(g_elite_weapons[0]);
        return g_elite_weapons;
    case AI_LOADOUT_GRUNT:
        if (count_out) *count_out = sizeof(g_grunt_weapons) / sizeof(g_grunt_weapons[0]);
        return g_grunt_weapons;
    case AI_LOADOUT_JACKAL:
        if (count_out) *count_out = sizeof(g_jackal_weapons) / sizeof(g_jackal_weapons[0]);
        return g_jackal_weapons;
    case AI_LOADOUT_FLOOD_COMBAT:
        if (count_out) *count_out = sizeof(g_flood_weapons) / sizeof(g_flood_weapons[0]);
        return g_flood_weapons;
    default:
        return NULL;
    }
}

static int unit_has_weapon_tag(uint8_t *unit, int32_t tag) {
    int slot;
    if (!unit || !readable_range(unit + 0x344, 4u * sizeof(int32_t))) return 0;
    for (slot = 0; slot < 4; ++slot) {
        int32_t item_datum = *(int32_t *)(unit + 0x344 + slot * 4);
        uint8_t *item = object_data(item_datum);
        if (item && readable_range(item, sizeof(int32_t)) &&
            *(int32_t *)item == tag &&
            ((tag_get_fn)(g_blam + 0x201480))(0x77656170u, tag)) {
            return 1;
        }
    }
    return 0;
}

/* Remplace uniquement les objets d'inventaire qui sont reellement des tags
 * weapon, par le chemin moteur utilise lors du placement normal d'un acteur.
 * En cas d'echec, l'arme d'origine est recreee avant de rendre la main afin de
 * ne jamais laisser une IA sans arme a cause du mod. */
static int equip_random_faction_weapon(int32_t actor_variant,
                                       int32_t actor_datum,
                                       int32_t unit_datum,
                                       int profile,
                                       int32_t *weapon_tag_out,
                                       int *removed_out,
                                       int *restored_out,
                                       char *path_out,
                                       size_t path_out_size) {
    const struct weighted_weapon_entry *entries;
    size_t count = 0;
    size_t selected = 0;
    size_t attempt;
    uint32_t total = 0;
    uint32_t roll;
    int32_t selected_tag = -1;
    int32_t original_tag = -1;
    int32_t items[4] = { -1, -1, -1, -1 };
    int removed = 0;
    int restored = 0;
    int slot;
    uint8_t *unit;

    if (weapon_tag_out) *weapon_tag_out = -1;
    if (removed_out) *removed_out = 0;
    if (restored_out) *restored_out = 0;
    if (path_out && path_out_size) path_out[0] = '\0';
    if (profile == AI_LOADOUT_KEEP_DEFAULT) return 1;
    if (!actor_data(actor_datum, unit_datum)) return 0;
    entries = weapons_for_loadout(profile, &count);
    if (!entries || !count) return 0;
    for (attempt = 0; attempt < count; ++attempt) total += entries[attempt].weight;
    if (!total) return 0;
    roll = ai_loadout_random((uint32_t)unit_datum ^ ((uint32_t)actor_datum << 1)) % total;
    for (selected = 0; selected < count; ++selected) {
        if (roll < entries[selected].weight) break;
        roll -= entries[selected].weight;
    }
    if (selected >= count) selected = 0;
    for (attempt = 0; attempt < count; ++attempt) {
        size_t candidate = (selected + attempt) % count;
        if (load_original_weapon_checked(entries[candidate].path, &selected_tag)) {
            selected = candidate;
            break;
        }
    }
    if (selected_tag == -1) return 0;

    unit = object_data(unit_datum);
    if (!unit || !readable_range(unit + 0x344, 4u * sizeof(int32_t))) return 0;
    for (slot = 0; slot < 4; ++slot) {
        int32_t item_datum = *(int32_t *)(unit + 0x344 + slot * 4);
        uint8_t *item = object_data(item_datum);
        if (!item || !readable_range(item, sizeof(int32_t))) continue;
        if (((tag_get_fn)(g_blam + 0x201480))(0x77656170u, *(int32_t *)item)) {
            if (original_tag == -1) original_tag = *(int32_t *)item;
            items[slot] = item_datum;
        }
    }
    for (slot = 0; slot < 4; ++slot) {
        if (items[slot] == -1 || !object_data(items[slot])) continue;
        ((object_delete_fn)(g_blam + 0x350960))(items[slot]);
        ++removed;
    }
    MemoryBarrier();
    ((actor_create_equipment_fn)(g_blam + 0x56660))(
        actor_variant, unit_datum, selected_tag, -1, -1, -1);
    MemoryBarrier();
    unit = object_data(unit_datum);
    if (!unit_has_weapon_tag(unit, selected_tag)) {
        if (original_tag != -1) {
            ((actor_create_equipment_fn)(g_blam + 0x56660))(
                actor_variant, unit_datum, original_tag, -1, -1, -1);
            MemoryBarrier();
            restored = unit_has_weapon_tag(object_data(unit_datum), original_tag);
        }
        if (removed_out) *removed_out = removed;
        if (restored_out) *restored_out = restored;
        return 0;
    }
    if (weapon_tag_out) *weapon_tag_out = selected_tag;
    if (removed_out) *removed_out = removed;
    if (restored_out) *restored_out = restored;
    if (path_out && path_out_size) {
        snprintf(path_out, path_out_size, "%s", entries[selected].path);
    }
    return 1;
}

#define AI_FOLLOW_WATCH_CAPACITY 128
struct ai_follow_watch_entry {
    uint8_t *actor_table;
    int32_t actor_datum;
    int32_t unit_datum;
    int32_t player_unit_datum;
    int32_t last_player_vehicle;
    int32_t exit_vehicle_datum;
    float last_position[3];
    uint8_t has_position;
    uint8_t stagnant_samples;
    uint8_t cooldown_samples;
    uint8_t exit_retry_samples;
    uint8_t exit_attempts;
    uint8_t navigation_sample_phase;
};

static struct ai_follow_watch_entry g_ai_follow_watch[AI_FOLLOW_WATCH_CAPACITY];
static uint32_t g_ai_follow_watch_cursor;

static uint8_t *current_actor_table(void) {
    uint8_t *thread_state = blam_thread_state();
    if (!thread_state || !readable_range(thread_state + 0x28, sizeof(void *))) return NULL;
    return *(uint8_t **)(thread_state + 0x28);
}

static void register_ai_follow_watch(int32_t actor_datum,
                                     int32_t unit_datum,
                                     int32_t player_unit_datum) {
    uint8_t *actor_table = current_actor_table();
    uint32_t index;
    uint32_t chosen = AI_FOLLOW_WATCH_CAPACITY;
    if (!actor_table || !actor_data(actor_datum, unit_datum) ||
        !is_live_player_unit(player_unit_datum)) return;
    for (index = 0; index < AI_FOLLOW_WATCH_CAPACITY; ++index) {
        if (g_ai_follow_watch[index].actor_table == actor_table &&
            g_ai_follow_watch[index].actor_datum == actor_datum &&
            g_ai_follow_watch[index].unit_datum == unit_datum) {
            chosen = index;
            break;
        }
        if (chosen == AI_FOLLOW_WATCH_CAPACITY &&
            !g_ai_follow_watch[index].actor_table) chosen = index;
    }
    if (chosen == AI_FOLLOW_WATCH_CAPACITY) {
        chosen = g_ai_follow_watch_cursor++ % AI_FOLLOW_WATCH_CAPACITY;
    }
    memset(&g_ai_follow_watch[chosen], 0, sizeof(g_ai_follow_watch[chosen]));
    g_ai_follow_watch[chosen].actor_table = actor_table;
    g_ai_follow_watch[chosen].actor_datum = actor_datum;
    g_ai_follow_watch[chosen].unit_datum = unit_datum;
    g_ai_follow_watch[chosen].player_unit_datum = player_unit_datum;
    g_ai_follow_watch[chosen].last_player_vehicle = -1;
    g_ai_follow_watch[chosen].exit_vehicle_datum = -1;
    {
        uint8_t *player = object_data(player_unit_datum);
        if (player && readable_range(player + 0x14, sizeof(int32_t))) {
            int32_t parent = *(int32_t *)(player + 0x14);
            if (parent == -1 || is_live_vehicle_object(parent))
                g_ai_follow_watch[chosen].last_player_vehicle = parent;
        }
    }
}

static void execute_ai_follow_watch(void) {
    uint8_t *actor_table = current_actor_table();
    int tracked = 0;
    int checked = 0;
    int stagnant = 0;
    int recovered = 0;
    int invalidated = 0;
    int vehicle_passengers = 0;
    int exit_requested = 0;
    int exit_rejected = 0;
    int exit_completed = 0;
    int exit_cancelled = 0;
    int exit_exhausted = 0;
    uint32_t index;
    for (index = 0; index < AI_FOLLOW_WATCH_CAPACITY; ++index) {
        struct ai_follow_watch_entry *entry = &g_ai_follow_watch[index];
        uint8_t *unit;
        uint8_t *player;
        float position[3];
        float player_position[3];
        float forward[3];
        float up[3];
        float dx, dy, dz, distance_sq, movement_sq;
        int32_t unit_parent;
        int32_t player_parent;
        int16_t unit_seat_state;
        if (!entry->actor_table) continue;
        if (!actor_table || entry->actor_table != actor_table ||
            !actor_data(entry->actor_datum, entry->unit_datum) ||
            !is_live_player_unit(entry->player_unit_datum)) {
            memset(entry, 0, sizeof(*entry));
            ++invalidated;
            continue;
        }
        unit = object_data(entry->unit_datum);
        player = object_data(entry->player_unit_datum);
        if (!unit || !player ||
            !readable_range(unit + 0x14, sizeof(int32_t)) ||
            !readable_range(unit + 0x328, sizeof(int16_t)) ||
            !readable_range(player + 0x14, sizeof(int32_t))) {
            memset(entry, 0, sizeof(*entry));
            ++invalidated;
            continue;
        }
        unit_parent = *(int32_t *)(unit + 0x14);
        player_parent = *(int32_t *)(player + 0x14);
        unit_seat_state = *(int16_t *)(unit + 0x328);
        if (player_parent != -1 && !is_live_vehicle_object(player_parent))
            player_parent = -1;
        ++tracked;
        /* Le garde ne suit que les allies crees par le mod. Si le parent du
         * joueur change, il vient de sortir de son ancien vehicule (ou d'en
         * changer directement). Seuls les allies encore assis dans cet ancien
         * vehicule recoivent l'action retail normale de sortie. */
        if (player_parent != entry->last_player_vehicle) {
            int32_t old_vehicle = entry->last_player_vehicle;
            entry->last_player_vehicle = player_parent;
            if (entry->exit_vehicle_datum == -1 && old_vehicle != -1 &&
                unit_parent == old_vehicle &&
                is_live_vehicle_object(old_vehicle)) {
                entry->exit_vehicle_datum = old_vehicle;
                entry->exit_retry_samples = 0;
                entry->exit_attempts = 0;
            }
        }
        if (entry->exit_vehicle_datum != -1) {
            /* Si le joueur remonte avant la fin de l'animation, garder ses
             * passagers est plus juste que les expulser au tick suivant. */
            if (player_parent == entry->exit_vehicle_datum) {
                entry->exit_vehicle_datum = -1;
                entry->exit_retry_samples = 0;
                entry->exit_attempts = 0;
                ++exit_cancelled;
            } else if (!is_live_vehicle_object(entry->exit_vehicle_datum)) {
                entry->exit_vehicle_datum = -1;
                entry->exit_retry_samples = 0;
                entry->exit_attempts = 0;
                ++exit_cancelled;
            } else if (unit_parent == entry->exit_vehicle_datum) {
                ++vehicle_passengers;
                if (unit_seat_state == -1) {
                    /* L'unite est encore parentee mais le siege est deja en
                     * transition : laisser le moteur terminer sans doubler
                     * l'action. */
                } else if (entry->exit_retry_samples) {
                    --entry->exit_retry_samples;
                } else if (entry->exit_attempts >= 3u) {
                    entry->exit_vehicle_datum = -1;
                    entry->exit_attempts = 0;
                    ++exit_exhausted;
                } else {
                    if (submit_normal_exit_action(entry->unit_datum))
                        ++exit_requested;
                    else
                        ++exit_rejected;
                    ++entry->exit_attempts;
                    entry->exit_retry_samples = 1;
                }
                entry->has_position = 0;
                entry->stagnant_samples = 0;
                entry->navigation_sample_phase = 0;
                continue;
            } else {
                entry->exit_vehicle_datum = -1;
                entry->exit_retry_samples = 0;
                entry->exit_attempts = 0;
                ++exit_completed;
            }
        }
        /* Une IA dans un vehicule ne doit jamais etre teletransportee hors de
         * son siege, meme si le vehicule reste immobile plusieurs secondes. */
        if (unit_parent != -1) {
            entry->has_position = 0;
            entry->stagnant_samples = 0;
            entry->navigation_sample_phase = 0;
            continue;
        }
        /* La sortie vehicule est verifiee toutes les 1,5 s. La recuperation
         * de navigation reste, elle, echantillonnee toutes les 3 s afin de
         * conserver le delai historique d'environ 12 secondes. */
        entry->navigation_sample_phase ^= 1u;
        if (entry->navigation_sample_phase) continue;
        ((object_get_origin_fn)(g_blam + 0x5A6780))(entry->unit_datum, position);
        ((object_get_origin_fn)(g_blam + 0x5A6780))(entry->player_unit_datum, player_position);
        ++checked;
        dx = position[0] - player_position[0];
        dy = position[1] - player_position[1];
        dz = position[2] - player_position[2];
        distance_sq = dx * dx + dy * dy + dz * dz;
        if (!entry->has_position) {
            memcpy(entry->last_position, position, sizeof(position));
            entry->has_position = 1;
            continue;
        }
        dx = position[0] - entry->last_position[0];
        dy = position[1] - entry->last_position[1];
        dz = position[2] - entry->last_position[2];
        movement_sq = dx * dx + dy * dy + dz * dz;
        memcpy(entry->last_position, position, sizeof(position));
        if (entry->cooldown_samples) --entry->cooldown_samples;
        if (distance_sq <= 144.0f || movement_sq >= 0.25f) {
            entry->stagnant_samples = 0;
            continue;
        }
        if (entry->stagnant_samples < 255) ++entry->stagnant_samples;
        ++stagnant;
        if (entry->stagnant_samples < 4 || entry->cooldown_samples) continue;

        ((object_get_orientation_fn)(g_blam + 0x5A6AE0))(
            entry->player_unit_datum, forward, up);
        {
            float horizontal = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);
            if (horizontal > 0.001f) {
                forward[0] /= horizontal;
                forward[1] /= horizontal;
            } else {
                forward[0] = 1.0f;
                forward[1] = 0.0f;
            }
            forward[2] = 0.0f;
            up[0] = 0.0f;
            up[1] = 0.0f;
            up[2] = 1.0f;
        }
        player_position[0] -= forward[0] * 1.20f;
        player_position[1] -= forward[1] * 1.20f;
        player_position[2] += 0.20f;
        ((object_teleport_fn)(g_blam + 0x351610))(
            entry->unit_datum, player_position, forward, up, 1);
        MemoryBarrier();
        memcpy(entry->last_position, player_position, sizeof(player_position));
        entry->stagnant_samples = 0;
        entry->cooldown_samples = 5;
        ++recovered;
    }
    snprintf(g_result, sizeof(g_result),
             "ok ai_follow_watch tracked=%d checked=%d stagnant=%d recovered=%d invalidated=%d vehicle_passengers=%d exit_requested=%d exit_rejected=%d exit_completed=%d exit_cancelled=%d exit_exhausted=%d threshold_m=12 samples=4 nav_sample_ms=3000 exit_attempts_max=3 vehicle_safe=1 player_exit_normal=1\n",
             tracked, checked, stagnant, recovered, invalidated,
             vehicle_passengers, exit_requested, exit_rejected,
             exit_completed, exit_cancelled, exit_exhausted);
}

/* Equivalent runtime de HaloScript ai_disregard(..., true). Dans cette
 * build, le wrapper retail (RVA 0x1C3DC0) pose uniquement le bit 0x20 de
 * object+0x1B4 pour chacune des unites de la liste. L'Oracle/Guilty Spark
 * appartient a l'equipe 12 dans la campagne : le marquer ainsi empeche les
 * IA ajoutees par le mod de le choisir comme cible, sans changer les relations
 * globales entre equipes ni les scripts de mission. */
static int disregard_actor_units_for_team(int team, int *changed_out,
                                          char *error, size_t error_size) {
    uint8_t *thread_state = blam_thread_state();
    uint8_t *table;
    uint8_t *entries;
    uint32_t *allocation_bits;
    int32_t capacity;
    int32_t high;
    int32_t index;
    int changed = 0;

    if (changed_out) *changed_out = 0;
    if (!thread_state || !readable_range(thread_state + 0x28, sizeof(void *))) {
        snprintf(error, error_size, "TLS acteurs indisponible");
        return 0;
    }
    table = *(uint8_t **)(thread_state + 0x28);
    if (!table || !readable_range(table, 0x60)) {
        snprintf(error, error_size, "table acteurs invalide");
        return 0;
    }
    capacity = *(int32_t *)(table + 0x2C);
    high = *(int32_t *)(table + 0x44);
    entries = *(uint8_t **)(table + 0x50);
    allocation_bits = *(uint32_t **)(table + 0x58);
    if (capacity <= 0 || capacity > 16384 || high < 0 || high > capacity ||
        !entries || !allocation_bits ||
        !readable_range(allocation_bits,
            (((size_t)capacity + 31u) / 32u) * sizeof(uint32_t))) {
        snprintf(error, error_size, "metadonnees acteurs invalides");
        return 0;
    }
    for (index = 0; index < high; ++index) {
        uint32_t slot = (uint32_t)index;
        uint8_t *actor;
        int32_t unit_datum;
        uint8_t *unit;
        if (!(allocation_bits[slot >> 5] & (1u << (slot & 31u)))) continue;
        actor = entries + (size_t)slot * 0xD10u;
        if (!readable_range(actor + 0x2C, 1) ||
            (int8_t)actor[0x2C] != (int8_t)team ||
            !readable_range(actor + 0x1C, sizeof(unit_datum))) continue;
        unit_datum = *(int32_t *)(actor + 0x1C);
        unit = object_data(unit_datum);
        if (!unit || !writable_range(unit + 0x1B4, sizeof(uint32_t))) continue;
        *(uint32_t *)(unit + 0x1B4) |= 0x20u;
        MemoryBarrier();
        if (*(uint32_t *)(unit + 0x1B4) & 0x20u) ++changed;
    }
    if (changed_out) *changed_out = changed;
    snprintf(error, error_size, "ok ai_disregard team=%d units=%d", team, changed);
    return 1;
}

/* Reproduit exactement l'effet utile de la commande HaloScript
 * ai_force_active_by_unit pour un acteur deja valide. Le wrapper retail
 * (RVA 0x1C5270) ne fait lui-meme qu'activer le bit 0x20 de actor+0x06
 * lorsque le bit d'acteur vivant 0x10 est present. Cette operation reste
 * strictement dans l'entree runtime de l'acteur: aucune definition de
 * scenario ni aucun pointeur de checkpoint n'est modifie. */
static int force_actor_active_by_unit(int32_t actor_datum,
                                      int32_t unit_datum,
                                      uint8_t *flags_before_out,
                                      uint8_t *flags_after_out) {
    uint8_t *actor = actor_data(actor_datum, unit_datum);
    uint8_t flags_before;
    uint8_t flags_after;
    if (!actor || !writable_range(actor + 0x06, 1)) return 0;
    flags_before = actor[0x06];
    if (!(flags_before & 0x10u)) return 0;
    actor[0x06] = (uint8_t)(flags_before | 0x20u);
    MemoryBarrier();
    flags_after = actor[0x06];
    if (flags_before_out) *flags_before_out = flags_before;
    if (flags_after_out) *flags_after_out = flags_after;
    return (flags_after & 0x30u) == 0x30u;
}

/* Retourne un enregistrement de la table runtime des squads. Cette table est
 * propre a la partie courante (TLS+0x58, stride 0xEC) : contrairement au bloc
 * de definitions du scenario, elle peut etre consultee sans rendre les
 * checkpoints dependants d'une allocation du mod. */
static uint8_t *live_squad_data(uint8_t *thread_state, int32_t squad_index) {
    uint8_t *table;
    uint8_t *entries;
    uint32_t *allocation_bits;
    int32_t capacity;
    uint32_t slot;

    if (!thread_state || squad_index < 0 ||
        !readable_range(thread_state + 0x58, sizeof(void *))) return NULL;
    table = *(uint8_t **)(thread_state + 0x58);
    if (!table || !readable_range(table, 0x60) ||
        *(uint64_t *)(table + 0x20) != 0xECu) return NULL;
    capacity = *(int32_t *)(table + 0x2C);
    entries = *(uint8_t **)(table + 0x50);
    allocation_bits = *(uint32_t **)(table + 0x58);
    slot = (uint32_t)squad_index;
    if (capacity <= 0 || capacity > 16384 || slot >= (uint32_t)capacity ||
        !entries || !allocation_bits ||
        !readable_range(allocation_bits + (slot >> 5), sizeof(uint32_t)) ||
        !(allocation_bits[slot >> 5] & (1u << (slot & 31u)))) return NULL;
    entries += (size_t)slot * 0xECu;
    return readable_range(entries, 0xECu) ? entries : NULL;
}

/* Cree une definition minimale uniquement sur la pile, puis laisse
 * l'initialiseur retail (RVA 0x48300) allouer et initialiser l'enregistrement
 * du live squad.  Contrairement a l'ancien prototype, aucun tag_block du
 * scenario n'est agrandi et aucun pointeur temporaire n'est conserve dans le
 * scenario ou dans un checkpoint.  L'initialiseur ne lit que flags/team/
 * parent lorsque team est non nul; ces trois champs sont donc les seuls que
 * nous renseignons dans la fausse definition ephemere. */
static int create_transient_live_squad(int team,
                                       int32_t *squad_index_out,
                                       char *error, size_t error_size) {
    static const uint8_t initialize_sig[] =
        {0x40,0x53,0x56,0x57,0x41,0x54,0x41,0x55};
    uint8_t *thread_state = blam_thread_state();
    uint8_t *table;
    uint8_t *entries;
    uint32_t *allocation_bits;
    uint8_t *squad;
    int32_t capacity;
    int32_t high;
    int32_t next;
    int32_t index;
    uint32_t slot;
    _Alignas(16) uint8_t definition[0x28];

    if (team <= 0 || team > 15 ||
        !bytes_equal(g_blam + 0x0048300,
                     initialize_sig, sizeof(initialize_sig))) {
        snprintf(error, error_size,
                 "initialiseur live squad incompatible team=%d", team);
        return 0;
    }
    if (!thread_state ||
        !readable_range(thread_state + 0x58, sizeof(void *))) {
        snprintf(error, error_size, "TLS live squad indisponible");
        return 0;
    }
    table = *(uint8_t **)(thread_state + 0x58);
    if (!table || !readable_range(table, 0x60) ||
        *(uint64_t *)(table + 0x20) != 0xECu) {
        snprintf(error, error_size, "table live squad invalide");
        return 0;
    }
    capacity = *(int32_t *)(table + 0x2C);
    next = *(int32_t *)(table + 0x40);
    high = *(int32_t *)(table + 0x44);
    entries = *(uint8_t **)(table + 0x50);
    allocation_bits = *(uint32_t **)(table + 0x58);
    if (capacity <= 0 || capacity > 16384 ||
        high < 0 || high > capacity ||
        next < 0 || next > high || !entries || !allocation_bits ||
        !readable_range(allocation_bits,
            (((size_t)capacity + 31u) / 32u) * sizeof(uint32_t))) {
        snprintf(error, error_size,
                 "metadonnees live squad invalides");
        return 0;
    }

    /* Meme choix de slot que data_new (RVA 0x43FF0). L'initialiseur retail
     * ne controle pas le retour -1, donc il est essentiel de prouver qu'un
     * emplacement existe avant de l'appeler. */
    index = -1;
    for (slot = (uint32_t)next; slot < (uint32_t)high; ++slot) {
        if (!(allocation_bits[slot >> 5] &
              (1u << (slot & 31u)))) {
            index = (int32_t)slot;
            break;
        }
    }
    if (index < 0 && high < capacity) index = high;
    if (index < 0 || index >= capacity ||
        !writable_range(entries + (size_t)(uint32_t)index * 0xECu,
                        0xECu) ||
        !writable_range(table + 0x40, 0x0Cu) ||
        !writable_range(allocation_bits + ((uint32_t)index >> 5),
                        sizeof(uint32_t))) {
        snprintf(error, error_size,
                 "aucun slot live squad libre high=%d capacity=%d",
                 high, capacity);
        return 0;
    }

    memset(definition, 0, sizeof(definition));
    definition[0x24] = (uint8_t)team;
    *(int16_t *)(definition + 0x26) = -1;
    ((squad_runtime_initialize_fn)(g_blam + 0x0048300))(definition);
    MemoryBarrier();

    squad = live_squad_data(blam_thread_state(), index);
    if (!squad || (int8_t)squad[0xE7] != (int8_t)team ||
        *(uint16_t *)(squad + 0x20) != 0 ||
        *(int16_t *)(squad + 0x94) != -1 ||
        *(int16_t *)(squad + 0x96) != -1 ||
        *(int32_t *)(squad + 0xA0) != -1 ||
        *(int32_t *)(squad + 0xA4) != -1 ||
        *(int32_t *)(squad + 0xB0) != -1 ||
        *(int32_t *)(squad + 0xB4) != -1) {
        snprintf(error, error_size,
                 "initialisation live squad non confirmee index=%d team=%d",
                 index, team);
        return 0;
    }
    if (squad_index_out) *squad_index_out = index;
    snprintf(error, error_size,
             "ok transient_runtime_squad=%d team=%d", index, team);
    return 1;
}

/* Les live squads retail sont limites a seize acteurs. Une escouade runtime
 * ennemie est donc reutilisee jusqu'a cette limite, puis une nouvelle est
 * creee. Le pointeur de table et le salt de l'entree rendent le cache caduc
 * automatiquement apres un changement/rechargement de mission. */
struct transient_squad_cache_entry {
    uint8_t *table;
    int32_t index;
    uint16_t salt;
};

static struct transient_squad_cache_entry g_transient_enemy_squads[16];


/* AI4: E10's 191 original scenario definitions cover indices 0..190.
 * Runtime-only enemy index 191 has no matching definition. Use only the
 * four named, empty definitions appended by the companion E10 package.
 * Return -1 for other missions, 0 to deny, 1 for a validated free slot.
 * This function only reads; it never creates or edits a squad. */
/* AI23: The Maw cannot safely use runtime-only squads. Never fall back
 * to those indices when the scenario is D40. Follow and independent humans
 * have separate reserved slots; existing mission AI and diplomacy are intact. */
static int ai23_maw_squad(int team, int independent, int32_t preferred,
        int32_t *index_out, int32_t *count_out, char *error, size_t error_size) {
    uint8_t *scenario, *first, *last; int total,i,start;
    if (!g_blam || !readable_range(g_blam+0x10C3558,sizeof(void *))) return 0;
    scenario=*(uint8_t **)(g_blam+0x10C3558);
    if (!scenario || !readable_range(scenario+0x348,8)) return 0;
    first=resolve_scenario_squad(scenario,0);
    if (!first || !readable_range(first,32)) return 0;
    if (strncmp((const char *)first,"enc6_1_2_elites",32)) return -1;
    total=*(int32_t *)(scenario+0x348);
    last=resolve_scenario_squad(scenario,193);
    if(total!=214 || !last || strncmp((const char *)last,"enc6_1_3_1_sents_1",32)) {
        snprintf(error,error_size,"Maw support missing or old checkpoint: restart mission from mission select");return 0;
    }
    if(team!=2 && team!=3 && team!=8 && team!=9) return 0;
    for(i=194;i<214;i++) {
        uint8_t *d=resolve_scenario_squad(scenario,i);char name[32];
        int n=i-194,expected=n<8?2:n<12?3:n<16?8:9;
        snprintf(name,sizeof(name),"starky_maw_squad_%d",n);
        if(!d || !readable_range(d,108) || strncmp((const char *)d,name,32) ||
           *(uint32_t*)(d+0x20)!=0 || *(int16_t*)(d+0x24)!=expected ||
           *(int16_t*)(d+0x26)!=-1 || *(int16_t*)(d+0x28)!=-1 ||
           *(int16_t*)(d+0x2A)!=-1 || *(int16_t*)(d+0x2C)!=-1 ||
           *(uint32_t*)(d+0x30)!=0 || *(uint32_t*)(d+0x3C)!=0 ||
           *(int32_t*)(d+0x50)!=-1 || *(uint32_t*)(d+0x54)!=0 || *(uint32_t*)(d+0x60)!=0) {
            snprintf(error,error_size,"Maw reserved definition invalid index=%d",i);return 0;
        }
    }
    start=team==2?(independent?198:194):team==3?202:team==8?206:210;
    for(i=start;i<start+4;i++) {
        uint8_t *s=live_squad_data(blam_thread_state(),i);int ft,count;
        if(!s || (int8_t)s[0xE7]!=team) {
            snprintf(error,error_size,"Maw live squad unavailable: restart mission");return 0;
        }
        ft=*(int32_t*)(s+0xB0);count=*(uint16_t*)(s+0x20);
        if(team==2 && !independent) {if(ft!=-1 && ft!=preferred)continue;}
        else if(ft!=-1 || *(int16_t*)(s+0x94)!=-1 || *(int16_t*)(s+0x96)!=-1) continue;
        if(*(int32_t*)(s+0xA4)!=-1 || count>=16)continue;
        *index_out=i;*count_out=count;return 1;
    }
    snprintf(error,error_size,"Maw reserved squads full or unavailable for selected mode (64-unit capacity)");return 0;
}

static int select_e10_defined_enemy_squad(
        int team, int32_t *index_out, int32_t *count_out,
        char *error, size_t error_size) {
    uint8_t *scenario, *first, *last;
    int32_t total, i, start;
    if (team != 2 && team != 3 && team != 8 && team != 9) return -1;
    start = team == 2 ? 203 : team == 3 ? 191 : team == 8 ? 195 : 199;
    if (!g_blam || !readable_range(g_blam + 0x10C3558, sizeof(void *))) {
        snprintf(error, error_size, "ai4 scenario pointer unavailable"); return 0;
    }
    scenario = *(uint8_t **)(g_blam + 0x10C3558);
    if (!scenario || !readable_range(scenario + 0x348, 8)) {
        snprintf(error, error_size, "ai4 scenario unavailable"); return 0;
    }
    total = *(int32_t *)(scenario + 0x348);
    if (total <= 0 || total > 16384) {
        snprintf(error, error_size, "ai4 invalid scenario squad count"); return 0;
    }
    first = resolve_scenario_squad(scenario, 0);
    if (!first) {
        snprintf(error, error_size, "ai4 first squad unreadable"); return 0;
    }
    if (strncmp((const char *)first, "sq_testing_facility_wv1_brutes_", 32)) return -1;
    last = resolve_scenario_squad(scenario, 190);
    if (!last || strncmp((const char *)last, "sq_dojo_elite_reins", 32) || (total != 203 && total != 207)) {
        snprintf(error, error_size, "ai4 E10 companion scenario missing or incompatible"); return 0;
    }
    if (team == 2 && total != 207) {
        snprintf(error,error_size,"Free Combat needs AI22 bonus companion and mission restart"); return 0;
    }
    /* Validate every reserved definition before choosing one. */
    for (i = 191; i < total; ++i) {
        char expected[32];
        uint8_t *definition = resolve_scenario_squad(scenario, i);
        snprintf(expected, sizeof(expected), i < 203 ? "nova_sandbox_enemy_%d" : "starky_free_marine_%d", (int)(i < 203 ? i-191 : i-203));
        if (!definition || strncmp((const char *)definition, expected, 32) ||
            *(uint32_t *)(definition + 0x20) != 0 ||
            *(int16_t *)(definition + 0x24) != (i < 195 ? 3 : i < 199 ? 8 : i < 203 ? 9 : 2) ||
            *(int16_t *)(definition + 0x26) != -1 ||
            *(int16_t *)(definition + 0x28) != -1 ||
            *(int16_t *)(definition + 0x2A) != -1 ||
            *(int16_t *)(definition + 0x2C) != -1 ||
            *(uint32_t *)(definition + 0x30) != 0 ||
            *(uint32_t *)(definition + 0x3C) != 0 ||
            *(int32_t *)(definition + 0x50) != -1 ||
            *(uint32_t *)(definition + 0x54) != 0 ||
            *(uint32_t *)(definition + 0x60) != 0) {
            snprintf(error, error_size, "ai4 E10 reserved squad definition invalid index=%d", (int)i); return 0;
        }
    }
    for (i = start; i < start + 4; ++i) {
        uint8_t *squad = live_squad_data(blam_thread_state(), i);
        int32_t count;
        if (!squad || (int8_t)squad[0xE7] != team ||
            *(int32_t *)(squad + 0xB0) != -1 ||
            *(int32_t *)(squad + 0xA4) != -1 ||
            *(int16_t *)(squad + 0x94) != -1 ||
            *(int16_t *)(squad + 0x96) != -1) {
            snprintf(error, error_size,
                "E10 live squad index=%d present=%d team=%d expected=%d fireteam=%d parent=%d order=%d state=%d; E10 squad validation failed",
                (int)i, squad!=NULL, squad?(int)(int8_t)squad[0xE7]:-1, team,
                squad?*(int32_t *)(squad+0xB0):-1, squad?*(int32_t *)(squad+0xA4):-1,
                squad?*(int16_t *)(squad+0x94):-1, squad?*(int16_t *)(squad+0x96):-1); return 0;
        }
        count = *(uint16_t *)(squad + 0x20);
        if (count < 16) {
            *index_out = i; *count_out = count;
            return 1;
        }
    }
    snprintf(error, error_size, "E10 reserved squads full (64 units for this faction/mode)"); return 0;
}

static int select_or_create_transient_enemy_squad(
        int team, int32_t *squad_index_out, int32_t *count_out,
        int *created_out, char *error, size_t error_size) {
    uint8_t *thread_state = blam_thread_state();
    uint8_t *table;
    uint8_t *squad;
    struct transient_squad_cache_entry *cached;
    int32_t index;

    if (created_out) *created_out = 0;
    {
        int defined = select_e10_defined_enemy_squad(team, squad_index_out, count_out, error, error_size);
        if (defined >= 0) return defined;
    }
    if (team <= 0 || team > 15 || !thread_state ||
        !readable_range(thread_state + 0x58, sizeof(void *))) {
        snprintf(error, error_size, "cache squad ennemi indisponible team=%d", team);
        return 0;
    }
    table = *(uint8_t **)(thread_state + 0x58);
    cached = &g_transient_enemy_squads[team];
    if (cached->table == table && cached->index >= 0) {
        squad = live_squad_data(thread_state, cached->index);
        if (squad && *(uint16_t *)squad == cached->salt &&
            (int8_t)squad[0xE7] == (int8_t)team &&
            *(uint16_t *)(squad + 0x20) < 16 &&
            *(int32_t *)(squad + 0xB0) == -1) {
            if (squad_index_out) *squad_index_out = cached->index;
            if (count_out) *count_out = (int32_t)*(uint16_t *)(squad + 0x20);
            return 1;
        }
    }
    if (!create_transient_live_squad(team, &index, error, error_size)) return 0;
    squad = live_squad_data(blam_thread_state(), index);
    if (!squad) {
        snprintf(error, error_size, "squad ennemi runtime perdu index=%d", index);
        return 0;
    }
    cached->table = table;
    cached->index = index;
    cached->salt = *(uint16_t *)squad;
    if (squad_index_out) *squad_index_out = index;
    if (count_out) *count_out = 0;
    if (created_out) *created_out = 1;
    return 1;
}

/* Lecture seule du datum de fireteam deja associe au joueur. Elle permet de
 * reutiliser le meme squad pour les apparitions suivantes, sans rattacher un
 * squad qui appartient a un autre joueur en cooperation. L'absence de
 * fireteam n'est pas une erreur : le wrapper officiel en cree une ensuite. */
static int32_t existing_player_fireteam_datum(int32_t player_unit_datum) {
    uint8_t *thread_state;
    uint8_t *player_unit;
    uint8_t *links;
    int32_t player_datum;
    int32_t index;

    player_unit = object_data(player_unit_datum);
    if (!player_unit || !readable_range(player_unit + 0x1BC, sizeof(player_datum))) return -1;
    player_datum = *(int32_t *)(player_unit + 0x1BC);
    thread_state = blam_thread_state();
    if (player_datum == -1 || !thread_state ||
        !readable_range(thread_state + 0x188, sizeof(void *))) return -1;
    links = *(uint8_t **)(thread_state + 0x188);
    if (!links || !readable_range(links, 8u * 0x3Cu)) return -1;
    for (index = 0; index < 8; ++index) {
        uint8_t *link = links + (size_t)index * 0x3Cu;
        if (*(int32_t *)link == player_datum &&
            readable_range(link + 0x20, sizeof(int32_t))) {
            return *(int32_t *)(link + 0x20);
        }
    }
    return -1;
}

/* Selectionne un squad deja initialise par le moteur et de la meme equipe.
 * Les squads avec des acteurs vivants sont preferes car ils possedent deja un
 * contexte de zone/objectifs valide. Un squad non lie a une autre fireteam est
 * prefere pour ne jamais detourner les compagnons d'un autre joueur en coop. */
/* AI22: only consulted during a spawn, never on a recurring scan.
 * Runtime-only, unlinked human squads and named E10 Free Combat definitions
 * must not be borrowed by the existing Follow/escort path. */
static int ai22_independent_squad(int32_t index, int32_t fireteam) {
    uint8_t *scenario, *definition;
    int32_t count;
    if (!g_blam || !readable_range(g_blam+0x10C3558,sizeof(void *))) return 1;
    scenario=*(uint8_t **)(g_blam+0x10C3558);
    if (!scenario || !readable_range(scenario+0x348,4)) return 1;
    count=*(int32_t *)(scenario+0x348);
    if (count<=0 || count>16384 || index<0) return 1;
    if (index>=count) return fireteam==-1;
    definition=resolve_scenario_squad(scenario,index);
    return definition && readable_range(definition,32) &&
        !strncmp((const char *)definition,"starky_free_marine_",18);
}

static int select_live_squad_for_team(int team,
                                      int32_t preferred_fireteam,
                                      int32_t *squad_index_out,
                                      int32_t *actor_count_out,
                                      int32_t *fireteam_out,
                                      char *error, size_t error_size) {
    uint8_t *thread_state = blam_thread_state();
    uint8_t *table;
    uint8_t *entries;
    uint32_t *allocation_bits;
    int32_t capacity;
    int32_t high;
    int32_t index;
    int32_t best_index = -1;
    int32_t best_count = 0;
    int32_t best_fireteam = -1;
    int best_score = INT_MIN;

    if (!thread_state || !readable_range(thread_state + 0x58, sizeof(void *))) {
        snprintf(error, error_size, "TLS live squad indisponible");
        return 0;
    }
    table = *(uint8_t **)(thread_state + 0x58);
    if (!table || !readable_range(table, 0x60) ||
        *(uint64_t *)(table + 0x20) != 0xECu) {
        snprintf(error, error_size, "table live squad invalide");
        return 0;
    }
    capacity = *(int32_t *)(table + 0x2C);
    high = *(int32_t *)(table + 0x44);
    entries = *(uint8_t **)(table + 0x50);
    allocation_bits = *(uint32_t **)(table + 0x58);
    if (capacity <= 0 || capacity > 16384 || high < 0 || high > capacity ||
        !entries || !allocation_bits ||
        !readable_range(allocation_bits,
            (((size_t)capacity + 31u) / 32u) * sizeof(uint32_t))) {
        snprintf(error, error_size, "metadonnees live squad invalides");
        return 0;
    }

    for (index = 0; index < high; ++index) {
        uint8_t *squad;
        int32_t count;
        int32_t fireteam;
        int16_t order;
        int16_t order_state;
        int score;
        uint32_t slot = (uint32_t)index;
        if (!(allocation_bits[slot >> 5] & (1u << (slot & 31u)))) continue;
        squad = entries + (size_t)slot * 0xECu;
        if (!readable_range(squad, 0xECu) ||
            (int8_t)squad[0xE7] != (int8_t)team) continue;
        count = (int32_t)*(uint16_t *)(squad + 0x20);
        fireteam = *(int32_t *)(squad + 0xB0);
        if (team == 2 && ai22_independent_squad(index, fireteam)) continue;
        order = *(int16_t *)(squad + 0x94);
        order_state = *(int16_t *)(squad + 0x96);
        if (count < 0 || count > 256) continue;

        /* Le conteneur d'acteurs d'un live squad retail est limite a seize
         * membres. ai_migrate accepte momentanement un dix-septieme acteur,
         * puis le tick suivant le moteur le detruit pour restaurer la limite.
         * Ne jamais choisir un squad plein : un squad vide de la meme equipe
         * sera ensuite chaine a la meme fireteam par le chemin normal. */
        if (count >= 16) continue;

        /* Ne jamais emprunter le squad d'une autre fireteam en coop. */
        if (fireteam != -1 && fireteam != preferred_fireteam) continue;
        score = 0;
        /* 0x19/1 est l'ordre retail de suivi du joueur observe sur les
         * escouades de Marines qui accompagnent reellement le joueur. Les IA
         * du mod qui avaient rejoint ce type de squad se deplacent et suivent
         * deja correctement; il doit donc passer avant les squads statiques
         * lies par les anciens essais de fireteam. */
        if (count > 0 && order == 0x19 && order_state == 1) score += 10000;
        if (preferred_fireteam != -1 && fireteam == preferred_fireteam) score += 4000;
        if (fireteam == -1) score += 1000;
        /* Un squad vide evite de detourner les IA originales de la mission.
         * 0x04F6A0 l'activera apres le lien a la fireteam. */
        if (count == 0 && *(int32_t *)(squad + 0xA0) == -1) score += 2000;
        if (count > 0) score += 100;
        if (*(int32_t *)(squad + 0xA0) != -1) score += 50;
        if (*(int16_t *)(squad + 0x94) != -1) score += 20;
        if (*(int16_t *)(squad + 0x96) != -1) score += 10;
        /* Stabilise le choix : a score egal, conserver le plus petit index. */
        if (score > best_score) {
            best_score = score;
            best_index = index;
            best_count = count;
            best_fireteam = fireteam;
        }
    }
    if (best_index < 0 || !live_squad_data(thread_state, best_index)) {
        snprintf(error, error_size,
                 "aucun live squad equipe=%d high=%d", team, high);
        return 0;
    }
    if (squad_index_out) *squad_index_out = best_index;
    if (actor_count_out) *actor_count_out = best_count;
    if (fireteam_out) *fireteam_out = best_fireteam;
    return 1;
}

/* Appelle le chemin officiel utilise par HaloScript ai_migrate pour un acteur
 * individuel (RVA 0x05E2B0), puis controle les deux invariants essentiels :
 * actor+0x3C pointe vers le squad cible et le compteur du squad a augmente.
 * Aucune table n'est ecrite directement par le mod. */
static int migrate_actor_to_live_squad(int32_t actor_datum,
                                       int32_t unit_datum,
                                       int team,
                                       int32_t preferred_fireteam,
                                       int require_transient_squad,
                                       int32_t *squad_index_out,
                                       int32_t *count_after_out,
                                       int *runtime_squad_created_out,
                                       char *error, size_t error_size) {
    uint8_t *thread_state;
    uint8_t *actor;
    uint8_t *squad;
    int32_t squad_index;
    int32_t count_before;
    int32_t linked_fireteam;
    int32_t count_after;

    if (runtime_squad_created_out) *runtime_squad_created_out = 0;
    int maw=ai23_maw_squad(team,team==2 && require_transient_squad,preferred_fireteam,
        &squad_index,&count_before,error,error_size);
    if(maw==0)return 0;
    if(maw==1) {linked_fireteam=-1;}
    else if (require_transient_squad) {
        if (!select_or_create_transient_enemy_squad(
                team, &squad_index, &count_before,
                runtime_squad_created_out, error, error_size)) return 0;
        linked_fireteam = -1;
    } else if (!select_live_squad_for_team(team, preferred_fireteam,
                                           &squad_index, &count_before,
                                           &linked_fireteam,
                                           error, error_size)) {
        if (strncmp(error, "aucun live squad equipe=",
                    sizeof("aucun live squad equipe=") - 1u) ||
            !create_transient_live_squad(team, &squad_index,
                                         error, error_size)) {
            return 0;
        }
        count_before = 0;
        linked_fireteam = -1;
        if (runtime_squad_created_out) *runtime_squad_created_out = 1;
    }
    thread_state = blam_thread_state();
    actor = actor_data(actor_datum, unit_datum);
    squad = live_squad_data(thread_state, squad_index);
    if (!actor || !squad || !readable_range(actor + 0x3C, sizeof(int32_t))) {
        snprintf(error, error_size,
                 "acteur/squad perdu avant migration actor=%d squad=%d",
                 actor_datum, squad_index);
        return 0;
    }
    ((actor_migrate_to_squad_fn)(g_blam + 0x05E2B0))(
        actor_datum, squad_index);
    MemoryBarrier();

    actor = actor_data(actor_datum, unit_datum);
    squad = live_squad_data(blam_thread_state(), squad_index);
    if (!actor || !squad ||
        *(int32_t *)(actor + 0x3C) != squad_index) {
        snprintf(error, error_size,
                 "migration non confirmee actor=%d squad=%d actor_squad=%d",
                 actor_datum, squad_index,
                 actor && readable_range(actor + 0x3C, sizeof(int32_t))
                    ? *(int32_t *)(actor + 0x3C) : -2);
        return 0;
    }
    count_after = (int32_t)*(uint16_t *)(squad + 0x20);
    if (count_after < count_before || count_after > 256) {
        snprintf(error, error_size,
                 "compteur squad incoherent squad=%d before=%d after=%d",
                 squad_index, count_before, count_after);
        return 0;
    }
    if (squad_index_out) *squad_index_out = squad_index;
    if (count_after_out) *count_after_out = count_after;
    return 1;
}

/* actor_set_team peut appeler le nettoyeur 0x8BFF0. Celui-ci dereference
 * actor+0x5A4 lorsque actor+0x5AE contient encore un emplacement. Le chemin
 * de creation directe peut momentanement y enregistrer le biped modele. Il
 * faut donc retirer ce lien pendant que cet objet existe encore, sinon sa
 * destruction laisse exactement l'acces invalide observe a RVA 0x8C0DD. */
static int clear_actor_secondary_link(int32_t actor_datum, uint8_t *actor,
                                      int *cleared_out,
                                      char *error, size_t error_size) {
    int32_t linked_unit;
    int16_t linked_slot;
    uint8_t *linked_object;
    if (cleared_out) *cleared_out = 0;
    if (!actor || !readable_range(actor + 0x5A4, 12)) {
        snprintf(error, error_size, "champs lien secondaire illisibles actor=%d", actor_datum);
        return 0;
    }
    linked_slot = *(int16_t *)(actor + 0x5AE);
    if (linked_slot == -1) return 1;
    linked_unit = *(int32_t *)(actor + 0x5A4);
    if (linked_unit == -1 || linked_slot < 0 || linked_slot > 63) {
        snprintf(error, error_size, "lien secondaire incoherent actor=%d unit=%d slot=%d",
                 actor_datum, linked_unit, (int)linked_slot);
        return 0;
    }
    linked_object = object_data(linked_unit);
    if (!linked_object ||
        !readable_range(linked_object + 0xB60 + (size_t)linked_slot * 4u, sizeof(int32_t)) ||
        !readable_range(linked_object + 0xC60 + (size_t)linked_slot * 4u, sizeof(int32_t))) {
        snprintf(error, error_size, "objet lien secondaire absent actor=%d unit=%d slot=%d",
                 actor_datum, linked_unit, (int)linked_slot);
        return 0;
    }
    ((actor_clear_secondary_link_fn)(g_blam + 0x08BFF0))(actor_datum);
    MemoryBarrier();
    actor = actor_data_raw(actor_datum);
    if (!actor || !readable_range(actor + 0x5A4, 12) ||
        *(int32_t *)(actor + 0x5A4) != -1 ||
        *(int16_t *)(actor + 0x5AE) != -1) {
        snprintf(error, error_size, "nettoyage lien secondaire non confirme actor=%d", actor_datum);
        return 0;
    }
    if (cleared_out) *cleared_out = 1;
    return 1;
}

static uint8_t *fireteam_data(uint8_t *thread_state, int32_t fireteam_datum) {
    uint8_t *table;
    uint8_t *entries;
    uint8_t *entry;
    uint32_t slot;
    if (!thread_state || fireteam_datum == -1 ||
        !readable_range(thread_state + 0x138, sizeof(void *))) return NULL;
    table = *(uint8_t **)(thread_state + 0x138);
    if (!table || !readable_range(table + 0x50, sizeof(void *))) return NULL;
    entries = *(uint8_t **)(table + 0x50);
    slot = (uint16_t)fireteam_datum;
    if (!entries || slot >= 1024u) return NULL;
    entry = entries + (size_t)slot * 0xD4u;
    if (!readable_range(entry, 0xD4u) ||
        *(uint16_t *)entry !=
            (uint16_t)((uint32_t)fireteam_datum >> 16))
        return NULL;
    return entry;
}

static int fireteam_has_actor(uint8_t *fireteam, int32_t actor_datum) {
    int32_t count;
    int32_t index;
    if (!fireteam || !readable_range(fireteam + 0xA8, sizeof(count))) return 0;
    count = *(int32_t *)(fireteam + 0xA8);
    if (count < 0 || count > 8 || !readable_range(fireteam + 0x28, (size_t)count * 0x10u)) return 0;
    for (index = 0; index < count; ++index) {
        if (*(int32_t *)(fireteam + 0x2C + (size_t)index * 0x10u) == actor_datum) return 1;
    }
    return 0;
}

/* Reproduit uniquement la partie transitoire de ai_player_add_fireteam_squad:
 * retrouver (ou creer) la fireteam du joueur, puis utiliser la fonction moteur
 * qui ajoute un acteur. Aucun bloc du scenario ni aucune definition de tag
 * n'est modifie, donc aucun pointeur temporaire ne peut entrer dans un
 * checkpoint. */
static int add_actor_to_player_fireteam(int32_t player_unit_datum,
                                        int32_t actor_datum,
                                        int32_t squad_index,
                                        int32_t *fireteam_datum_out,
                                        int32_t *member_count_out,
                                        int *squad_linked_out,
                                        int *actor_member_out,
                                        int32_t *evicted_actor_out,
                                        char *error, size_t error_size) {
    uint8_t *thread_state;
    uint8_t *player_unit;
    uint8_t *links;
    uint8_t *link;
    uint8_t *fireteam_table;
    uint8_t *fireteam;
    uint8_t *live_squad;
    int32_t player_datum;
    int32_t fireteam_datum;
    int32_t link_index = -1;
    int32_t count_before;
    int32_t count_after;
    int32_t current_squad_fireteam;
    int32_t old_squad_head;
    int32_t cursor;
    int32_t index;
    int32_t chain_guard;
    int mission_follow_squad = 0;

    if (squad_linked_out) *squad_linked_out = 0;
    if (actor_member_out) *actor_member_out = 0;
    if (evicted_actor_out) *evicted_actor_out = -1;

    player_unit = object_data(player_unit_datum);
    if (!player_unit || !readable_range(player_unit + 0x1BC, sizeof(player_datum))) {
        snprintf(error, error_size, "unite joueur invalide=%d", player_unit_datum);
        return 0;
    }
    player_datum = *(int32_t *)(player_unit + 0x1BC);
    if (player_datum == -1) {
        snprintf(error, error_size, "datum joueur absent unit=%d", player_unit_datum);
        return 0;
    }
    if (!actor_data_raw(actor_datum)) {
        snprintf(error, error_size, "acteur invalide=%d", actor_datum);
        return 0;
    }

    thread_state = blam_thread_state();
    if (!thread_state || !readable_range(thread_state + 0x188, sizeof(void *)) ||
        !readable_range(thread_state + 0x138, sizeof(void *))) {
        snprintf(error, error_size, "TLS fireteam indisponible");
        return 0;
    }
    links = *(uint8_t **)(thread_state + 0x188);
    if (!links || !readable_range(links, 8u * 0x3Cu)) {
        snprintf(error, error_size, "liens joueurs indisponibles");
        return 0;
    }
    for (index = 0; index < 8; ++index) {
        if (*(int32_t *)(links + (size_t)index * 0x3Cu) == player_datum) {
            link_index = index;
            break;
        }
    }
    if (link_index < 0) {
        snprintf(error, error_size, "lien fireteam joueur introuvable player=%d", player_datum);
        return 0;
    }
    link = links + (size_t)link_index * 0x3Cu;
    if (!readable_range(link + 0x20, 8)) {
        snprintf(error, error_size, "lien fireteam joueur illisible index=%d", link_index);
        return 0;
    }
    fireteam_datum = *(int32_t *)(link + 0x20);
    fireteam_table = *(uint8_t **)(thread_state + 0x138);
    if (!fireteam_table || !readable_range(fireteam_table + 0x50, sizeof(void *))) {
        snprintf(error, error_size, "table fireteam indisponible");
        return 0;
    }

    if (fireteam_datum == -1) {
        uint8_t *entry;
        int16_t old_head;
        if (!writable_range(link + 0x20, 8)) {
            snprintf(error, error_size, "lien fireteam joueur non modifiable");
            return 0;
        }
        fireteam_datum = ((data_new_fn)(g_blam + 0x043FF0))(fireteam_table);
        if (fireteam_datum == -1) {
            snprintf(error, error_size, "allocation fireteam refusee");
            return 0;
        }
        entry = fireteam_data(thread_state, fireteam_datum);
        if (!entry || !writable_range(entry + 4, 0xD0)) {
            snprintf(error, error_size, "nouvelle fireteam invalide=%d", fireteam_datum);
            return 0;
        }
        *(int32_t *)(entry + 0x04) = -1;
        *(int32_t *)(entry + 0x08) = 0x0008FFFF;
        *(int32_t *)(entry + 0x0C) = -1;
        *(int32_t *)(entry + 0x10) = 0;
        *(uint64_t *)(entry + 0x14) = UINT64_MAX;
        *(uint64_t *)(entry + 0x1C) = UINT64_MAX;
        *(int32_t *)(entry + 0x24) = 0;
        *(int32_t *)(entry + 0xA8) = 0;
        *(int16_t *)(entry + 0xAC) = 0;
        *(uint64_t *)(entry + 0xB0) = UINT64_MAX;
        *(int32_t *)(entry + 0xB8) = -1;
        *(int16_t *)(entry + 0xBC) = 0;
        *(uint64_t *)(entry + 0xC0) = UINT64_MAX;
        *(uint64_t *)(entry + 0xC8) = UINT64_MAX;
        entry[0xD0] = 0;

        old_head = *(int16_t *)(entry + 0x08);
        *(int16_t *)(link + 0x24) = old_head;
        *(int16_t *)(entry + 0x08) = (int16_t)link_index;
        *(int32_t *)(link + 0x20) = fireteam_datum;
        MemoryBarrier();
    }

    fireteam = fireteam_data(thread_state, fireteam_datum);
    if (!fireteam || !readable_range(fireteam + 0xA8, sizeof(count_before))) {
        snprintf(error, error_size, "fireteam joueur invalide=%d", fireteam_datum);
        return 0;
    }

    /* Reproduit la liaison runtime du wrapper retail
     * ai_player_add_fireteam_squad lorsqu'un live squad existe. Certaines
     * missions, notamment La Bibliotheque, ne definissent aucun squad de
     * Marines. Dans ce cas squad_index vaut -1 : le moteur autorise tout de
     * meme l'ajout direct de l'acteur a la fireteam (0x2A26F0 ne consulte que
     * l'acteur et la fireteam). On laisse donc la tete de squads a -1 et on
     * utilise le meme chemin officiel d'ajout d'acteur plus bas. */
    if (squad_index >= 0) {
        live_squad = live_squad_data(thread_state, squad_index);
        if (!live_squad || !readable_range(live_squad + 0xB0, 8) ||
            !readable_range(fireteam + 0x04, sizeof(int32_t))) {
            snprintf(error, error_size,
                     "squad/fireteam illisible squad=%d ft=%d",
                     squad_index, fireteam_datum);
            return 0;
        }
        mission_follow_squad = *(uint16_t *)(live_squad + 0x20) > 0 &&
            *(int16_t *)(live_squad + 0x94) == 0x19 &&
            *(int16_t *)(live_squad + 0x96) == 1;
        /* Une escouade de mission qui possede deja l'ordre retail de suivi
         * conserve son contexte. Sinon on la chaine a la fireteam. */
        if (!mission_follow_squad) {
    current_squad_fireteam = *(int32_t *)(live_squad + 0xB0);
    if (current_squad_fireteam != fireteam_datum) {
        if (current_squad_fireteam != -1) {
            ((fireteam_remove_squad_fn)(g_blam + 0x2A2D60))(squad_index);
            MemoryBarrier();
            live_squad = live_squad_data(blam_thread_state(), squad_index);
            if (!live_squad || *(int32_t *)(live_squad + 0xB0) != -1) {
                snprintf(error, error_size,
                         "detachement ancien squad refuse squad=%d old_ft=%d",
                         squad_index, current_squad_fireteam);
                return 0;
            }
        }
        old_squad_head = *(int32_t *)(fireteam + 0x04);
        if (old_squad_head != -1 &&
            !live_squad_data(blam_thread_state(), old_squad_head)) {
            snprintf(error, error_size,
                     "tete squads fireteam invalide ft=%d head=%d",
                     fireteam_datum, old_squad_head);
            return 0;
        }
        cursor = old_squad_head;
        chain_guard = 0;
        while (cursor != -1) {
            uint8_t *linked = live_squad_data(blam_thread_state(), cursor);
            if (!linked || ++chain_guard > 512) {
                snprintf(error, error_size,
                         "chaine squads fireteam corrompue ft=%d cursor=%d",
                         fireteam_datum, cursor);
                return 0;
            }
            if (cursor == squad_index) {
                snprintf(error, error_size,
                         "squad deja chaine sans proprietaire squad=%d ft=%d",
                         squad_index, fireteam_datum);
                return 0;
            }
            cursor = *(int32_t *)(linked + 0xB4);
        }
        if (!writable_range(live_squad + 0xB0, 8) ||
            !writable_range(fireteam + 0x04, sizeof(int32_t))) {
            snprintf(error, error_size,
                     "liaison squad/fireteam non modifiable squad=%d ft=%d",
                     squad_index, fireteam_datum);
            return 0;
        }
        *(int32_t *)(live_squad + 0xB4) = old_squad_head;
        *(int32_t *)(fireteam + 0x04) = squad_index;
        *(int32_t *)(live_squad + 0xB0) = fireteam_datum;
        MemoryBarrier();
        }
        ((squad_activate_fn)(g_blam + 0x04F6A0))(squad_index);
        MemoryBarrier();
        fireteam = fireteam_data(blam_thread_state(), fireteam_datum);
        live_squad = live_squad_data(blam_thread_state(), squad_index);
        if (!fireteam || !live_squad ||
            *(int32_t *)(live_squad + 0xB0) != fireteam_datum) {
            snprintf(error, error_size,
                     "liaison squad/fireteam non confirmee squad=%d ft=%d",
                     squad_index, fireteam_datum);
            return 0;
        }
        if (squad_linked_out) *squad_linked_out = 1;
        }
    }

    count_before = *(int32_t *)(fireteam + 0xA8);
    if (count_before < 0 || count_before > 8) {
        snprintf(error, error_size, "compteur fireteam invalide=%d", count_before);
        return 0;
    }
    if (!fireteam_has_actor(fireteam, actor_datum)) {
        if (count_before >= 8) {
            int32_t evicted_actor = *(int32_t *)(fireteam + 0x2C);
            /* La structure retail ne contient que huit entrees de 0x10 octets
             * entre +0x28 et le compteur +0xA8. La fonction officielle
             * fireteam_remove_actor_at (RVA 0x2A2BE0) compacte cette liste.
             * On libere donc une place avant l'ajout, sans jamais depasser la
             * capacite et sans ecrire directement dans le tableau. Ce premier
             * chemin de rotation sert aussi a verifier si l'ordre de suivi
             * reste attache a l'acteur evince. */
            ((fireteam_remove_actor_at_fn)(g_blam + 0x2A2BE0))(
                fireteam_datum, 0);
            MemoryBarrier();
            fireteam = fireteam_data(blam_thread_state(), fireteam_datum);
            if (!fireteam || !readable_range(fireteam + 0xA8, sizeof(count_before)) ||
                *(int32_t *)(fireteam + 0xA8) != 7) {
                snprintf(error, error_size,
                         "rotation fireteam refusee ft=%d before=%d actor=%d",
                         fireteam_datum, count_before, evicted_actor);
                return 0;
            }
            count_before = 7;
            if (evicted_actor_out) *evicted_actor_out = evicted_actor;
        }
        ((fireteam_add_actor_fn)(g_blam + 0x2A26F0))(fireteam_datum, actor_datum);
        MemoryBarrier();
    }
    fireteam = fireteam_data(thread_state, fireteam_datum);
    if (!fireteam || !readable_range(fireteam + 0xA8, sizeof(count_after))) {
        snprintf(error, error_size, "fireteam perdue apres ajout");
        return 0;
    }
    count_after = *(int32_t *)(fireteam + 0xA8);
    if (count_after < 0 || count_after > 8 || !fireteam_has_actor(fireteam, actor_datum)) {
        snprintf(error, error_size, "ajout acteur refuse ft=%d before=%d after=%d actor=%d",
                 fireteam_datum, count_before, count_after, actor_datum);
        return 0;
    }
    if (fireteam_datum_out) *fireteam_datum_out = fireteam_datum;
    if (member_count_out) *member_count_out = count_after;
    if (actor_member_out) *actor_member_out = 1;
    return 1;
}

static size_t collect_ai_objects(uint32_t ai_reference, struct ai_object_record *out, size_t capacity) {
    _Alignas(16) uint8_t iterator[0x80];
    ai_iterator_init_fn init = (ai_iterator_init_fn)(g_blam + 0x00FA680);
    ai_iterator_next_fn next = (ai_iterator_next_fn)(g_blam + 0x00FA780);
    size_t count = 0;
    void *actor;
    memset(iterator, 0, sizeof(iterator));
    init(ai_reference, iterator);
    while (count < capacity && (actor = next(iterator)) != NULL) {
        int32_t datum;
        size_t i;
        int duplicate = 0;
        if (!readable_range((uint8_t *)actor + 0x1C, sizeof(datum))) break;
        datum = *(int32_t *)((uint8_t *)actor + 0x1C);
        if (datum == -1) continue;
        for (i = 0; i < count; ++i) if (out[i].datum == datum) { duplicate = 1; break; }
        if (!duplicate) {
            out[count].datum = datum;
            out[count].actor = (uint8_t *)actor;
            ++count;
        }
    }
    return count;
}

static int datum_in_list(int32_t datum, const struct ai_object_record *list, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) if (list[i].datum == datum) return 1;
    return 0;
}

static uint8_t *actor_for_unit(int32_t unit_datum, int32_t *actor_datum_out) {
    uint8_t *unit;
    uint8_t *actor;
    int32_t actor_datum;
    unit = object_data(unit_datum);
    if (!unit || !readable_range(unit + 0x1AC, sizeof(actor_datum))) return NULL;
    actor_datum = *(int32_t *)(unit + 0x1AC);
    actor = actor_data(actor_datum, unit_datum);
    if (!actor) return NULL;
    if (actor_datum_out) *actor_datum_out = actor_datum;
    return actor;
}

static int ai_runtime_context(uint8_t **thread_out, uint8_t **state_out,
                              uint64_t **flags_out, uint64_t *value_out) {
    DWORD tls_index;
    uint8_t **tls_slots;
    uint8_t *thread_state;
    uint8_t *runtime_state;
    uint64_t *flags;
    if (!g_blam) return 0;
    tls_index = *(DWORD *)(g_blam + 0xD72730);
    tls_slots = (uint8_t **)__readgsqword(0x58);
    if (!tls_slots || !readable_range(tls_slots + tls_index, sizeof(void *))) return 0;
    thread_state = tls_slots[tls_index];
    if (!thread_state || !readable_range(thread_state + 0x60, sizeof(void *))) return 0;
    runtime_state = *(uint8_t **)(thread_state + 0x60);
    if (!runtime_state || !readable_range(runtime_state + 0x1EBE0, sizeof(uint64_t))) return 0;
    flags = (uint64_t *)(runtime_state + 0x1EBE0);
    if (thread_out) *thread_out = thread_state;
    if (state_out) *state_out = runtime_state;
    *flags_out = flags;
    *value_out = *flags;
    return 1;
}

static int ai_runtime_flags(uint64_t **flags_out, uint64_t *value_out) {
    return ai_runtime_context(NULL, NULL, flags_out, value_out);
}

static void execute_ai_probe(void) {
    void *scenario = NULL;
    uint8_t *thread_state = NULL;
    uint8_t *runtime_state = NULL;
    uint8_t *actor_globals = NULL;
    uint64_t *runtime_flags_address = NULL;
    uint64_t runtime_flags = 0;
    int group_count = 0, squad_count = 0;
    char error[256];
    if (!validate_ai_bridge(&scenario, &group_count, &squad_count, error, sizeof(error))) {
        snprintf(g_result, sizeof(g_result), "error ai_probe %s thread=%lu\n",
                 error, (unsigned long)GetCurrentThreadId());
        return;
    }
    if (!ai_runtime_context(&thread_state, &runtime_state, &runtime_flags_address, &runtime_flags)) {
        snprintf(g_result, sizeof(g_result),
                 "ok ai_bridge_ready groups=%d squads=%d scenario=%p thread=%lu dispatcher=game_MAIN_THREAD exact_datum=1 ai_runtime_flags=unavailable\n",
                 group_count, squad_count, scenario, (unsigned long)GetCurrentThreadId());
        return;
    }
    if (readable_range(thread_state + 0x40, sizeof(void *))) {
        actor_globals = *(uint8_t **)(thread_state + 0x40);
        if (!readable_range(actor_globals, 4)) actor_globals = NULL;
    }
    snprintf(g_result, sizeof(g_result),
             "ok ai_bridge_ready groups=%d squads=%d scenario=%p thread=%lu dispatcher=game_MAIN_THREAD exact_datum=1 ai_runtime_state=%u,%u,%u ai_actor_context=%u,%u,%u,%u ai_runtime_flags=%016llX ai_place_ready=%d\n",
             group_count, squad_count, scenario, (unsigned long)GetCurrentThreadId(),
             (unsigned)runtime_state[0], (unsigned)runtime_state[1], (unsigned)runtime_state[0x10],
             actor_globals ? (unsigned)actor_globals[0] : 255,
             actor_globals ? (unsigned)actor_globals[1] : 255,
             actor_globals ? (unsigned)actor_globals[2] : 255,
             actor_globals ? (unsigned)actor_globals[3] : 255,
             (unsigned long long)runtime_flags,
             (runtime_flags & 0x0000002000000000ULL) ? 1 : 0);
}

static void execute_ai_place_group(const struct native_command *command) {
    void *scenario = NULL;
    int group_count = 0, squad_count = 0;
    int reference_index, reference_count;
    uint32_t ai_reference;
    uint64_t *runtime_flags_address = NULL;
    uint64_t runtime_flags = 0;
    uint64_t forced_runtime_flags = 0;
    uint8_t *thread_state = NULL;
    uint8_t *runtime_state = NULL;
    uint8_t *actor_globals = NULL;
    uint8_t actor_context_original = 0;
    uint8_t ai_phase_original = 0;
    uint8_t *squad_record = NULL;
    int runtime_flag_forced = 0;
    int actor_context_forced = 0;
    int ai_phase_forced = 0;
    struct ai_object_record before[64], after[64];
    _Alignas(16) uint8_t placement_result[0x20];
    int32_t created[64];
    size_t before_count, after_count, created_count = 0, team_changed = 0, team_skipped = 0, i;
    size_t used;
    char error[256];
    const char *reference_kind = command->a == 1 ? "squad" : "group";
    if (!safe_ai_group_name(command->path) || (command->a != 1 && command->a != 2) ||
        command->b < 1 || command->b > 16 || command->c < -1 || command->c > 15) {
        snprintf(g_result, sizeof(g_result), "error ai_place commande refusee\n");
        return;
    }
    if (!validate_ai_bridge(&scenario, &group_count, &squad_count, error, sizeof(error))) {
        snprintf(g_result, sizeof(g_result), "error ai_place %s\n", error);
        return;
    }
    if (command->a == 1) {
        reference_index = ((find_squad_fn)(g_blam + 0x0054EE0))(scenario, command->path);
        reference_count = squad_count;
        ai_reference = 0x20000000u | ((uint32_t)reference_index & 0xFFFFu);
    } else {
        reference_index = ((find_squad_group_fn)(g_blam + 0x0054FE0))(scenario, command->path);
        reference_count = group_count;
        ai_reference = 0x40000000u | ((uint32_t)reference_index & 0xFFFFu);
    }
    if (reference_index < 0 || reference_index >= reference_count || reference_index > 0xFFFF) {
        snprintf(g_result, sizeof(g_result), "error ai_place %s absent name=%s index=%d count=%d\n",
                 reference_kind, command->path, reference_index, reference_count);
        return;
    }
    if (command->a == 1) {
        squad_record = resolve_scenario_squad(scenario, reference_index);
        if (!squad_record || !readable_range(squad_record + 0x48, sizeof(uint16_t))) {
            snprintf(g_result, sizeof(g_result), "error ai_place definition escouade illisible\n");
            return;
        }
    }
    if (!ai_runtime_context(&thread_state, &runtime_state, &runtime_flags_address, &runtime_flags)) {
        snprintf(g_result, sizeof(g_result), "error ai_place etat runtime IA indisponible\n");
        return;
    }
    if (!readable_range(thread_state + 0x40, sizeof(void *))) {
        snprintf(g_result, sizeof(g_result), "error ai_place contexte acteurs IA indisponible\n");
        return;
    }
    actor_globals = *(uint8_t **)(thread_state + 0x40);
    if (!actor_globals || !readable_range(actor_globals, 4) || !writable_range(actor_globals + 1, 1)) {
        snprintf(g_result, sizeof(g_result), "error ai_place contexte acteurs IA illisible\n");
        return;
    }
    actor_context_original = actor_globals[1];
    if (!actor_context_original) {
        actor_globals[1] = 1;
        MemoryBarrier();
        actor_context_forced = 1;
    }
    /* The scenario placement filter (RVA 0x48BE0) accepts spawn definitions
       only during the engine's AI update phase, identified by state byte 0x10
       being 2. HaloScript normally establishes that phase around ai_place.
       A command injected between frames sees phase 1, so every valid starting
       location is filtered out before object_new is ever reached. Recreate the
       narrow phase for this synchronous call and restore it immediately. */
    ai_phase_original = runtime_state[0x10];
    if (ai_phase_original != 2) {
        if (!writable_range(runtime_state + 0x10, 1)) {
            if (actor_context_forced) actor_globals[1] = actor_context_original;
            snprintf(g_result, sizeof(g_result), "error ai_place phase IA non modifiable\n");
            return;
        }
        runtime_state[0x10] = 2;
        MemoryBarrier();
        ai_phase_forced = 1;
    }
    before_count = collect_ai_objects(ai_reference, before, 64);
    if (!(runtime_flags & 0x0000002000000000ULL)) {
        if (!writable_range(runtime_flags_address, sizeof(*runtime_flags_address))) {
            if (ai_phase_forced) runtime_state[0x10] = ai_phase_original;
            if (actor_context_forced) actor_globals[1] = actor_context_original;
            snprintf(g_result, sizeof(g_result), "error ai_place masque runtime IA non modifiable\n");
            return;
        }
        forced_runtime_flags = runtime_flags | 0x0000002000000000ULL;
        ((runtime_flags_set_fn)(g_blam + 0x209FC0))(&forced_runtime_flags);
        MemoryBarrier();
        runtime_flag_forced = 1;
    }
    /* ai_place's squad branch constructs this exact 0x18-byte request before
       delegating to the real scenario placement routine at RVA 0x48EC0.
       Keep it visible here so failures expose the engine-selected location
       and request flags instead of collapsing to the unhelpful "0 actors". */
    memset(placement_result, 0, sizeof(placement_result));
    *(int32_t *)(placement_result + 0x00) = -1;
    *(int32_t *)(placement_result + 0x04) = -1;
    *(int16_t *)(placement_result + 0x08) = -1;
    *(int16_t *)(placement_result + 0x0A) = (int16_t)command->b;
    *(uint8_t *)(placement_result + 0x0C) = 2;
    *(uint8_t *)(placement_result + 0x0D) = 0;
    *(int16_t *)(placement_result + 0x0E) = 0;
    *(uint64_t *)(placement_result + 0x10) = 0;
    /* Bit 0 signifie « aucune variante/remix imposée ». Il conserve le chemin
       local cells/choices/starting_locations du squad. Le supprimer ferait
       sélectionner à 0x48EC0 un template de mission sans rapport avec la
       définition runtime. */
    if (command->a == 1) {
        ((ai_place_squad_fn)(g_blam + 0x0048EC0))(reference_index, placement_result);
    } else {
        ((ai_place_fn)(g_blam + 0x00FD810))(ai_reference, command->b, 0);
    }
    if (runtime_flag_forced) {
        MemoryBarrier();
        ((runtime_flags_set_fn)(g_blam + 0x209FC0))(&runtime_flags);
        MemoryBarrier();
    }
    if (ai_phase_forced) {
        runtime_state[0x10] = ai_phase_original;
        MemoryBarrier();
    }
    after_count = collect_ai_objects(ai_reference, after, 64);
    for (i = 0; i < after_count; ++i) {
        if (!datum_in_list(after[i].datum, before, before_count)) {
            int32_t actor_datum = -1;
            uint8_t *validated_actor;
            created[created_count++] = after[i].datum;
            if (command->c >= 0) {
                validated_actor = actor_for_unit(after[i].datum, &actor_datum);
                if (validated_actor && validated_actor == after[i].actor && actor_datum != -1) {
                    ((actor_set_team_fn)(g_blam + 0x0059870))(actor_datum, command->c);
                    ++team_changed;
                } else {
                    ++team_skipped;
                }
            }
        }
    }
    if (actor_context_forced) {
        MemoryBarrier();
        actor_globals[1] = actor_context_original;
        MemoryBarrier();
    }
    if (!created_count) {
        ai_runtime_flags(&runtime_flags_address, &runtime_flags);
        snprintf(g_result, sizeof(g_result),
                 "error ai_place aucun nouvel acteur kind=%s name=%s index=%d before=%zu after=%zu placement=%d,%d,%d,%d,%u,%u thread=%lu ai_runtime_state=%u,%u,%u ai_actor_context=%u,%u,%u,%u ai_runtime_flags=%016llX ai_place_ready=%d runtime_forced=%d actor_context_forced=%d ai_phase_forced=%d\n",
                 reference_kind, command->path, reference_index, before_count, after_count,
                 *(int32_t *)(placement_result + 0x00), *(int32_t *)(placement_result + 0x04),
                 (int)*(int16_t *)(placement_result + 0x08), (int)*(int16_t *)(placement_result + 0x0A),
                 (unsigned)*(uint8_t *)(placement_result + 0x0C),
                 (unsigned)*(uint8_t *)(placement_result + 0x0D),
                 (unsigned long)GetCurrentThreadId(),
                 (unsigned)runtime_state[0],
                 (unsigned)runtime_state[1], (unsigned)runtime_state[0x10],
                 (unsigned)actor_globals[0], (unsigned)actor_globals[1],
                 (unsigned)actor_globals[2], (unsigned)actor_globals[3],
                 (unsigned long long)runtime_flags,
                 (runtime_flags & 0x0000002000000000ULL) ? 1 : 0, runtime_flag_forced,
                  actor_context_forced, ai_phase_forced);
        return;
    }
    used = (size_t)snprintf(g_result, sizeof(g_result),
                            "ok ai_place kind=%s name=%s index=%d ref=%08X requested=%d new=%zu team=%d team_changed=%zu team_skipped=%zu placement=%d,%d,%d,%d,%u,%u runtime_forced=%d actor_context_forced=%d ai_phase_forced=%d datums=",
                            reference_kind, command->path, reference_index, ai_reference,
                            command->b, created_count, command->c, team_changed, team_skipped,
                            *(int32_t *)(placement_result + 0x00), *(int32_t *)(placement_result + 0x04),
                            (int)*(int16_t *)(placement_result + 0x08), (int)*(int16_t *)(placement_result + 0x0A),
                            (unsigned)*(uint8_t *)(placement_result + 0x0C),
                            (unsigned)*(uint8_t *)(placement_result + 0x0D), runtime_flag_forced,
                            actor_context_forced, ai_phase_forced);
    for (i = 0; i < created_count && used < sizeof(g_result) - 40; ++i) {
        used += (size_t)snprintf(g_result + used, sizeof(g_result) - used,
                                 "%s%d", i ? "," : "", created[i]);
    }
    snprintf(g_result + used, sizeof(g_result) - used,
             " thread=%lu\n", (unsigned long)GetCurrentThreadId());
}

#if 0
#pragma pack(push, 1)
/* Ancien prototype d'escouade injectee dans le scenario. Conserve uniquement
 * comme documentation de recherche, mais exclu du binaire: un checkpoint peut
 * serialiser la definition ajoutee alors que son stockage n'existe plus au
 * processus suivant. Ne jamais reactiver ce bloc. */
struct rt_tag_block {
    int32_t count;
    uint32_t data_token;
    uint32_t definition_token;
};

struct rt_squad_definition {
    char name[0x20];
    uint32_t flags;
    int16_t team;
    int16_t parent;
    int16_t initial_zone;
    int32_t initial_task_area;
    int16_t initial_objective;
    struct rt_tag_block spawn_formations;
    struct rt_tag_block starting_locations;
    uint16_t allowed_placement;
    uint16_t unknown_4A;
    int32_t unknown_4C;
    int32_t template_index;
    struct rt_tag_block cells;
    struct rt_tag_block template_cells;
};

struct rt_cell_definition {
    uint8_t unknown_00[0x14];
    int16_t spawn_count;
    uint8_t unknown_16[2];
    int16_t placement_mode;
    uint8_t unknown_1A[2];
    struct rt_tag_block choices;
    uint8_t unknown_28[0x2A];
    int16_t unknown_52;
    uint8_t unknown_54[0x24];
};

struct rt_character_reference {
    uint32_t group;
    uint8_t unknown_04[8];
    int32_t tag_datum;
};

struct rt_character_choice {
    uint8_t unknown_00[0x10];
    int16_t character_reference;
    int16_t weight;
};
#pragma pack(pop)

_Static_assert(sizeof(struct rt_tag_block) == 0x0C, "rt_tag_block");
_Static_assert(sizeof(struct rt_squad_definition) == 0x6C, "rt_squad_definition");
_Static_assert(offsetof(struct rt_squad_definition, starting_locations) == 0x3C,
               "rt_squad starting_locations");
_Static_assert(offsetof(struct rt_squad_definition, cells) == 0x54,
               "rt_squad cells");
_Static_assert(sizeof(struct rt_cell_definition) == 0x78, "rt_cell_definition");
_Static_assert(offsetof(struct rt_cell_definition, choices) == 0x1C,
               "rt_cell choices");
_Static_assert(sizeof(struct rt_character_reference) == 0x10,
               "rt_character_reference");
_Static_assert(sizeof(struct rt_character_choice) == 0x14,
               "rt_character_choice");

typedef int32_t (__fastcall *rt_tag_block_add_fn)(struct rt_tag_block *);
typedef uint8_t (__fastcall *rt_tag_block_resize_fn)(struct rt_tag_block *, int32_t);
typedef int32_t (__fastcall *rt_tag_load_fn)(uint32_t, const char *);
typedef uint8_t (__fastcall *rt_add_location_fn)(
    const char *, const char *, int32_t, const float *);
typedef void (__fastcall *rt_squad_initialize_fn)(struct rt_squad_definition *);

enum rt_build_result {
    RT_BUILD_FAILED = 0,
    RT_BUILD_OK = 1,
    RT_BUILD_LIVE_RETAINED = -1
};

struct rt_squad_transaction {
    void *scenario;
    struct rt_squad_definition *squad;
    int32_t squad_index;
    int32_t squad_count_before;
    int32_t zone_count_before;
    int32_t character_count_before;
    int32_t cell_count_before;
    int32_t location_count_before;
    int team;
    int squad_added;
    int zone_added;
    int character_added;
    int cell_added;
    int location_added;
    int live_initialized;
    char name[32];
};

static int rt_signatures_valid(void) {
    static const uint8_t add_sig[] =
        {0x40,0x53,0x48,0x83,0xEC,0x20,0x48,0x8B,0xD9};
    static const uint8_t resize_sig[] =
        {0x48,0x89,0x5C,0x24,0x18,0x48,0x89,0x74,0x24,0x20};
    static const uint8_t load_sig[] =
        {0x40,0x53,0xB8,0x40,0x10,0x00,0x00};
    static const uint8_t location_sig[] =
        {0x48,0x89,0x5C,0x24,0x18,0x55,0x56,0x41,0x56};
    static const uint8_t initialize_sig[] =
        {0x40,0x53,0x56,0x57,0x41,0x54,0x41,0x55};
    static const uint8_t place_sig[] =
        {0x48,0x89,0x54,0x24,0x10,0x89,0x4C,0x24,0x08};

    return bytes_equal(g_blam + 0x23BB00, add_sig, sizeof(add_sig)) &&
           bytes_equal(g_blam + 0x23B790, resize_sig, sizeof(resize_sig)) &&
           bytes_equal(g_blam + 0x197F60, load_sig, sizeof(load_sig)) &&
           bytes_equal(g_blam + 0x179090, location_sig, sizeof(location_sig)) &&
           bytes_equal(g_blam + 0x0048300, initialize_sig, sizeof(initialize_sig)) &&
           bytes_equal(g_blam + 0x0048EC0, place_sig, sizeof(place_sig));
}

static uint8_t *rt_block_record(const struct rt_tag_block *block,
                                int32_t index,
                                size_t stride,
                                size_t required_size) {
    uint64_t units;
    uint32_t page;
    uint8_t **pages;
    uint8_t *base;
    uintptr_t address;

    if (!block || !readable_range(block, sizeof(*block)) ||
        index < 0 || index >= block->count ||
        !stride || (stride & 3u)) {
        return NULL;
    }
    units = (uint64_t)block->data_token +
            (uint64_t)(uint32_t)index * (stride >> 2);
    if (units > UINT32_MAX) return NULL;
    page = (uint32_t)(units >> 28);
    if (page >= 16) return NULL;
    pages = (uint8_t **)(g_blam + 0x2C2CCC0);
    if (!readable_range(pages + page, sizeof(void *))) return NULL;
    base = pages[page];
    if (!base) return NULL;
    address = (uintptr_t)base + (uintptr_t)(units * 4u);
    if (address < (uintptr_t)base) return NULL;
    return readable_range((void *)address, required_size)
        ? (uint8_t *)address : NULL;
}

/* Reproduit uniquement la sélection de slot de data_new (RVA 0x43FF0).
 * L'initialiseur retail 0x48300 ne contrôle jamais le retour -1 : ne jamais
 * l'appeler sans avoir prouvé que son prochain slot est l'index du squad. */
static int rt_data_array_predict_next(void *table, size_t expected_stride) {
    uint8_t *data = (uint8_t *)table;
    uint32_t *bits;
    int32_t capacity;
    int32_t next;
    int32_t high;
    int32_t index;

    if (!data || !readable_range(data, 0x60) ||
        *(uint64_t *)(data + 0x20) != expected_stride) {
        return -1;
    }
    capacity = *(int32_t *)(data + 0x2C);
    next = *(int32_t *)(data + 0x40);
    high = *(int32_t *)(data + 0x44);
    bits = *(uint32_t **)(data + 0x58);
    if (capacity <= 0 || capacity > 65536 ||
        high < 0 || high > capacity ||
        next < 0 || next > high ||
        !bits ||
        !readable_range(bits,
            (((size_t)capacity + 31u) / 32u) * sizeof(uint32_t))) {
        return -1;
    }
    for (index = next; index < high; ++index) {
        if (!(bits[(uint32_t)index >> 5] &
              (1u << ((uint32_t)index & 31u)))) {
            return index;
        }
    }
    return high < capacity ? high : -1;
}

static uint8_t *rt_live_squad_record(void *table, int32_t index) {
    uint8_t *data = (uint8_t *)table;
    uint8_t *records;
    uint32_t *bits;
    int32_t capacity;

    if (!data || index < 0 || !readable_range(data, 0x60) ||
        *(uint64_t *)(data + 0x20) != 0xEC) {
        return NULL;
    }
    capacity = *(int32_t *)(data + 0x2C);
    records = *(uint8_t **)(data + 0x50);
    bits = *(uint32_t **)(data + 0x58);
    if (index >= capacity || !records || !bits ||
        !readable_range(bits + ((uint32_t)index >> 5), sizeof(uint32_t)) ||
        !(bits[(uint32_t)index >> 5] &
          (1u << ((uint32_t)index & 31u)))) {
        return NULL;
    }
    records += (size_t)index * 0xECu;
    return readable_range(records, 0xEC) ? records : NULL;
}

static int rt_data_array_slot_writable(void *table,
                                       int32_t slot,
                                       size_t expected_stride) {
    uint8_t *data = (uint8_t *)table;
    uint8_t *records;
    uint32_t *bits;
    int32_t capacity;
    uintptr_t record_address;

    if (!data || slot < 0 || !readable_range(data, 0x60) ||
        *(uint64_t *)(data + 0x20) != expected_stride) {
        return 0;
    }
    capacity = *(int32_t *)(data + 0x2C);
    records = *(uint8_t **)(data + 0x50);
    bits = *(uint32_t **)(data + 0x58);
    if (capacity <= 0 || slot >= capacity ||
        !records || !bits ||
        !writable_range(data + 0x40, 0x10) ||
        !writable_range(bits + ((uint32_t)slot >> 5),
                        sizeof(uint32_t))) {
        return 0;
    }
    record_address = (uintptr_t)records +
        (uintptr_t)(uint32_t)slot * expected_stride;
    if (record_address < (uintptr_t)records) return 0;
    return writable_range((void *)record_address,
                          expected_stride);
}

static int rt_rollback_definition(struct rt_squad_transaction *tx) {
    rt_tag_block_resize_fn resize =
        (rt_tag_block_resize_fn)(g_blam + 0x23B790);
    struct rt_tag_block *squads;
    struct rt_tag_block *zones;
    struct rt_tag_block *characters;
    int ok = 1;

    if (!tx || tx->live_initialized || !tx->scenario ||
        !readable_range(g_blam + 0x010C3558, sizeof(void *)) ||
        *(void **)(g_blam + 0x010C3558) != tx->scenario) {
        return 0;
    }
    squads = (struct rt_tag_block *)((uint8_t *)tx->scenario + 0x348);
    zones = (struct rt_tag_block *)((uint8_t *)tx->scenario + 0x354);
    characters = (struct rt_tag_block *)((uint8_t *)tx->scenario + 0x390);

    if (tx->squad && tx->location_added &&
        tx->squad->starting_locations.count ==
            tx->location_count_before + 1) {
        ok &= resize(&tx->squad->starting_locations,
                     tx->location_count_before) != 0;
    }
    if (tx->squad && tx->cell_added &&
        tx->squad->cells.count == tx->cell_count_before + 1) {
        ok &= resize(&tx->squad->cells, tx->cell_count_before) != 0;
    }
    if (tx->character_added &&
        characters->count == tx->character_count_before + 1) {
        ok &= resize(characters, tx->character_count_before) != 0;
    }
    if (tx->zone_added &&
        zones->count == tx->zone_count_before + 1) {
        ok &= resize(zones, tx->zone_count_before) != 0;
    }
    if (tx->squad_added &&
        squads->count == tx->squad_count_before + 1) {
        ok &= resize(squads, tx->squad_count_before) != 0;
    }
    return ok;
}

static int rt_build_squad(const char *character_path,
                          int32_t known_character_tag,
                          const float position[3],
                          int team,
                          struct rt_squad_transaction *tx,
                          char *error,
                          size_t error_size) {
    rt_tag_block_add_fn add =
        (rt_tag_block_add_fn)(g_blam + 0x23BB00);
    rt_tag_block_resize_fn resize =
        (rt_tag_block_resize_fn)(g_blam + 0x23B790);
    rt_tag_load_fn load =
        (rt_tag_load_fn)(g_blam + 0x197F60);
    rt_add_location_fn add_location =
        (rt_add_location_fn)(g_blam + 0x179090);
    rt_squad_initialize_fn initialize =
        (rt_squad_initialize_fn)(g_blam + 0x0048300);
    struct rt_tag_block *squads;
    struct rt_tag_block *zones;
    struct rt_tag_block *characters;
    struct rt_cell_definition *cell;
    struct rt_character_choice *choice;
    uint8_t *reference;
    uint8_t *thread_state = NULL;
    uint8_t *runtime_state = NULL;
    uint8_t *live_table;
    uint8_t *live_squad;
    uint64_t *runtime_flags = NULL;
    uint64_t runtime_flags_value = 0;
    int32_t character_tag;
    int32_t character_reference = -1;
    int32_t cell_index;
    int32_t zone_index;
    int32_t index;
    int attempt;
    char location_name[32];

#define RT_ERROR(text) do { \
    snprintf(error, error_size, "%s", (text)); \
    goto failed; \
} while (0)

    if (!tx || !character_path || !position ||
        !valid_tag_name(character_path) ||
        team < 1 || team > 15 ||
        !isfinite(position[0]) || !isfinite(position[1]) ||
        !isfinite(position[2]) ||
        !rt_signatures_valid()) {
        snprintf(error, error_size,
                 "parametres/signatures runtime invalides");
        return RT_BUILD_FAILED;
    }

    memset(tx, 0, sizeof(*tx));
    tx->squad_index = -1;
    tx->team = team;
    tx->scenario = *(void **)(g_blam + 0x010C3558);
    if (!tx->scenario) {
        snprintf(error, error_size, "scenario absent");
        return RT_BUILD_FAILED;
    }

    squads = (struct rt_tag_block *)((uint8_t *)tx->scenario + 0x348);
    zones = (struct rt_tag_block *)((uint8_t *)tx->scenario + 0x354);
    characters = (struct rt_tag_block *)((uint8_t *)tx->scenario + 0x390);
    if (!writable_range(squads, sizeof(*squads)) ||
        !writable_range(zones, sizeof(*zones)) ||
        !writable_range(characters, sizeof(*characters)) ||
        squads->count < 0 || squads->count >= 0x1FFF ||
        zones->count < 0 || zones->count >= INT16_MAX ||
        characters->count < 0 || characters->count >= INT16_MAX) {
        snprintf(error, error_size, "blocs scenario invalides");
        return RT_BUILD_FAILED;
    }

    /* tag_load prend le FourCC logique 'char'; tag_get prend sa forme mémoire. */
    character_tag = known_character_tag;
    if (character_tag == -1)
        character_tag = load(0x63686172u, character_path);
    if (character_tag == -1 ||
        !((tag_get_fn)(g_blam + 0x201480))(0x72616863u,
                                           character_tag)) {
        snprintf(error, error_size, "tag character non charge");
        return RT_BUILD_FAILED;
    }
    if (*(void **)(g_blam + 0x010C3558) != tx->scenario) {
        snprintf(error, error_size,
                 "scenario change pendant chargement");
        return RT_BUILD_FAILED;
    }

    tx->squad_count_before = squads->count;
    tx->zone_count_before = zones->count;
    tx->character_count_before = characters->count;
    for (attempt = 0; attempt < 16; ++attempt) {
        unsigned long serial =
            (unsigned long)InterlockedIncrement(&g_rt_squad_serial);
        snprintf(tx->name, sizeof(tx->name),
                 "hs_rt_%02X_%04X_%08lX",
                 (unsigned)team,
                 (unsigned)tx->squad_count_before,
                 serial);
        if (((find_squad_fn)(g_blam + 0x54EE0))(
                tx->scenario, tx->name) == -1) {
            break;
        }
    }
    if (attempt == 16) RT_ERROR("nom runtime non unique");

    tx->squad_index = add(squads);
    if (tx->squad_index != tx->squad_count_before)
        RT_ERROR("allocation squad refusee");
    tx->squad_added = 1;

    zone_index = add(zones);
    if (zone_index != tx->zone_count_before ||
        zone_index > INT16_MAX) {
        RT_ERROR("allocation zone refusee");
    }
    tx->zone_added = 1;

    tx->squad = (struct rt_squad_definition *)
        rt_block_record(squads, tx->squad_index,
                        sizeof(struct rt_squad_definition),
                        sizeof(struct rt_squad_definition));
    if (!tx->squad ||
        !writable_range(tx->squad, sizeof(*tx->squad))) {
        RT_ERROR("definition squad illisible");
    }

    memcpy(tx->squad->name, tx->name, sizeof(tx->squad->name));
    tx->squad->flags |= 0x40u;
    tx->squad->team = (int16_t)team;
    tx->squad->parent = -1;
    tx->squad->initial_zone = (int16_t)zone_index;
    tx->squad->initial_task_area = -1;
    tx->squad->initial_objective = -1;
    /* « none » force l'utilisation des cells/choices/locations locales et
       interdit à ai_place_squad de substituer un template remix de la map. */
    tx->squad->allowed_placement = 0x0001u;
    tx->squad->unknown_4C = 0;
    tx->squad->template_index = -1;
    if (!resize(&tx->squad->spawn_formations, 0))
        RT_ERROR("initialisation formations refusee");

    for (index = 0; index < characters->count; ++index) {
        reference = rt_block_record(
            characters, index,
            sizeof(struct rt_character_reference),
            sizeof(struct rt_character_reference));
        if (!reference) RT_ERROR("palette character invalide");
        if (*(int32_t *)(reference + 0x0C) == character_tag) {
            character_reference = index;
            break;
        }
    }
    if (character_reference == -1) {
        character_reference = add(characters);
        if (character_reference != tx->character_count_before ||
            character_reference > INT16_MAX) {
            RT_ERROR("ajout palette character refuse");
        }
        tx->character_added = 1;
        reference = rt_block_record(
            characters, character_reference,
            sizeof(struct rt_character_reference),
            sizeof(struct rt_character_reference));
        if (!reference || !writable_range(reference, 0x10))
            RT_ERROR("reference character illisible");
        *(uint32_t *)(reference + 0x00) = 0x63686172u;
        *(int32_t *)(reference + 0x0C) = character_tag;
    }

    tx->cell_count_before = tx->squad->cells.count;
    cell_index = add(&tx->squad->cells);
    if (cell_index != tx->cell_count_before ||
        cell_index > INT16_MAX) {
        RT_ERROR("ajout cellule refuse");
    }
    tx->cell_added = 1;
    cell = (struct rt_cell_definition *)
        rt_block_record(&tx->squad->cells, cell_index,
                        sizeof(struct rt_cell_definition),
                        sizeof(struct rt_cell_definition));
    if (!cell || !writable_range(cell, sizeof(*cell)))
        RT_ERROR("cellule illisible");
    if (!resize(&cell->choices, 1))
        RT_ERROR("choix character refuse");
    choice = (struct rt_character_choice *)
        rt_block_record(&cell->choices, 0,
                        sizeof(struct rt_character_choice),
                        sizeof(struct rt_character_choice));
    if (!choice || !writable_range(choice, sizeof(*choice)))
        RT_ERROR("choix character illisible");
    choice->character_reference = (int16_t)character_reference;
    choice->weight = 1;
    cell->unknown_52 = -1;
    cell->spawn_count = 1;
    cell->placement_mode = 3;

    tx->location_count_before =
        tx->squad->starting_locations.count;
    snprintf(location_name, sizeof(location_name),
             "hs_l_%04X_%04X",
             (unsigned)tx->squad_index,
             (unsigned)tx->location_count_before);
    if (!add_location(tx->name, location_name,
                      cell_index, position)) {
        RT_ERROR("ajout position refuse");
    }
    if (tx->squad->starting_locations.count !=
            tx->location_count_before + 1) {
        RT_ERROR("compteur position incoherent");
    }
    tx->location_added = 1;

    if (!ai_runtime_context(
            &thread_state, &runtime_state,
            &runtime_flags, &runtime_flags_value) ||
        !readable_range(thread_state + 0x58, sizeof(void *))) {
        RT_ERROR("contexte live squad absent");
    }
    live_table = *(uint8_t **)(thread_state + 0x58);
    if (!live_table ||
        rt_data_array_predict_next(live_table, 0xEC) !=
            tx->squad_index ||
        !rt_data_array_slot_writable(live_table,
                                     tx->squad_index,
                                     0xEC)) {
        RT_ERROR("slot live squad non aligne");
    }

    initialize(tx->squad);
    tx->live_initialized = 1;
    MemoryBarrier();
    live_table = *(uint8_t **)(thread_state + 0x58);
    live_squad = rt_live_squad_record(
        live_table, tx->squad_index);
    if (!live_squad || *(uint16_t *)live_squad == 0 ||
        live_squad[0xE7] != (uint8_t)team ||
        *(int32_t *)(live_squad + 0xB0) != -1 ||
        *(int32_t *)(live_squad + 0xB4) != -1) {
        snprintf(error, error_size,
                 "live squad incoherent; definition conservee");
        return RT_BUILD_LIVE_RETAINED;
    }
    snprintf(error, error_size, "ok");
    return RT_BUILD_OK;

failed:
    if (!rt_rollback_definition(tx)) {
        size_t used = strlen(error);
        if (used < error_size) {
            snprintf(error + used, error_size - used,
                     "; rollback incomplet");
        }
    }
    return RT_BUILD_FAILED;
#undef RT_ERROR
}

static int rt_place_squad(struct rt_squad_transaction *tx,
                          int32_t *unit_out,
                          int32_t *actor_out) {
    struct native_command placement;
    struct ai_object_record before[8];
    struct ai_object_record after[8];
    size_t before_count;
    size_t after_count;
    size_t index;
    int32_t actor_datum = -1;
    uint8_t *actor;
    uint32_t reference;

    if (!tx || !tx->live_initialized ||
        tx->squad_index < 0 || tx->squad_index > 0x1FFF) {
        return 0;
    }
    reference = 0x20000000u |
        ((uint32_t)tx->squad_index & 0x1FFFu);
    before_count = collect_ai_objects(reference, before, 8);

    memset(&placement, 0, sizeof(placement));
    placement.kind = COMMAND_AI_PLACE_GROUP;
    placement.a = 1;
    placement.b = 1;
    placement.c = -1;
    memcpy(placement.path, tx->name, sizeof(tx->name));
    execute_ai_place_group(&placement);

    after_count = collect_ai_objects(reference, after, 8);
    for (index = 0; index < after_count; ++index) {
        if (datum_in_list(after[index].datum,
                          before, before_count)) {
            continue;
        }
        actor = actor_for_unit(after[index].datum,
                               &actor_datum);
        if (!actor || actor != after[index].actor ||
            actor_datum == -1) {
            return 0;
        }
        if (!readable_range(actor + 0x2C, 1)) {
            return 0;
        }
        if ((int8_t)actor[0x2C] != (int8_t)tx->team) {
            ((actor_set_team_fn)(g_blam + 0x59870))(
                actor_datum, tx->team);
            actor = actor_for_unit(after[index].datum,
                                   &actor_datum);
            if (!actor ||
                !readable_range(actor + 0x2C, 1) ||
                (int8_t)actor[0x2C] != (int8_t)tx->team) {
                return 0;
            }
        }
        if (unit_out) *unit_out = after[index].datum;
        if (actor_out) *actor_out = actor_datum;
        return 1;
    }
    return 0;
}

static void execute_ai_runtime_squad(
    const struct native_command *command) {
    struct rt_squad_transaction tx;
    float position[3] = {
        command->x, command->y, command->z
    };
    char error[256];
    char placement_detail[768];
    int result;
    int placement_ok;
    int32_t unit_datum = -1;
    int32_t actor_datum = -1;

    result = rt_build_squad(command->path, command->d, position,
                            command->b, &tx,
                            error, sizeof(error));
    if (result == RT_BUILD_FAILED) {
        snprintf(g_result, sizeof(g_result),
                 "error ai_runtime_squad build=%s team=%d path=%s\n",
                 error, command->b, command->path);
        return;
    }
    if (result == RT_BUILD_LIVE_RETAINED) {
        snprintf(g_result, sizeof(g_result),
                 "error ai_runtime_squad live_retenu=%s squad=%d name=%s team=%d\n",
                 error, tx.squad_index, tx.name, tx.team);
        return;
    }
    placement_ok = rt_place_squad(&tx, &unit_datum,
                                  &actor_datum);
    snprintf(placement_detail, sizeof(placement_detail), "%s", g_result);
    if (!placement_ok) {
        snprintf(g_result, sizeof(g_result),
                 "error ai_runtime_squad placement squad=%d name=%s team=%d definition_live_conservee=1 detail={%s}\n",
                 tx.squad_index, tx.name, tx.team,
                 placement_detail);
        return;
    }
    snprintf(g_result, sizeof(g_result),
             "ok ai_runtime_squad squad=%d name=%s unit=%d actor=%d team=%d true_squad=1 duplicate_body=0 follow_pending=1\n",
             tx.squad_index, tx.name, unit_datum,
             actor_datum, tx.team);
}
#endif

struct ai1_profile { const char *biped; const char *character; int team; };
static const struct ai1_profile ai1_profiles[] = {
{"objects\\characters\\johnson\\johnsonn","objects\\characters\\johnson\\ai\\johnson",2},
{"objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_general",3},
{"objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute",3},
{"objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_captain",3},
{"objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_weapon",3},
{"objects\\characters\\brute\\brute","objects\\characters\\brute\\ai\\brute_chieftain_armor",3},

{"objects\\characters\\floodcombat_human\\floodcombat_human","objects\\characters\\floodcombat_human\\ai\\flood_combat_human",8},
{"objects\\characters\\floodcombat_elite\\floodcombat_elite","objects\\characters\\floodcombat_elite\\ai\\floodcombat_elite",8},
{"objects\\characters\\floodcarrier\\floodcarrier","objects\\characters\\floodcarrier\\ai\\flood_carrier",8},
{"objects\\characters\\flood_tank\\flood_tank","objects\\characters\\flood_tank\\ai\\flood_pureform_tank",8},
{"objects\\characters\\sentinel_aggressor\\sentinel_aggressor","objects\\characters\\sentinel_aggressor\\ai\\sentinel_aggressor",9},
{"objects\\characters\\sentinel_aggressor\\sentinel_aggressor","objects\\characters\\sentinel_aggressor\\ai\\sentinel_aggressor_major",9},

{"objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_officer",3},
{"objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_major",3},
{"objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_minor",3},
{"objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_specops",3},
{"objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_sacristan_zealot",3},
{"objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_specops",3},
{"objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite_ultra",3},
{"objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt_heavy",3},
{"objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt_major",3},
{"objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt_sacristan_major",3},
{"objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt_sacristan_specops",3},
{"objects\\characters\\jackal\\jackal","objects\\characters\\jackal\\ai\\jackal_major",3},
{"objects\\characters\\jackal\\jackal","objects\\characters\\jackal\\ai\\jackal_sacristan_major",3},
{"objects\\characters\\jackal\\jackal","objects\\characters\\jackal\\ai\\jackal_sacristan_minor",3},
{"objects\\characters\\marine\\marine","objects\\characters\\marine\\ai\\trooper",2},
{"objects\\characters\\marine_female\\marine_female","objects\\characters\\marine_female\\ai\\trooper_female",2},
{"objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt",3},
{"objects\\characters\\grunt\\grunt","objects\\characters\\grunt\\ai\\grunt_sacristan_minor",3},
{"objects\\characters\\jackal\\jackal","objects\\characters\\jackal\\ai\\jackal",3},
{"objects\\characters\\elite\\elite","objects\\characters\\elite\\ai\\elite",3},
{"objects\\characters\\hunter\\hunter","objects\\characters\\hunter\\ai\\hunter",3},
};
struct ai6_item { const char *path; uint32_t group; };
static const struct ai6_item ai6_items[] = {
{"objects\\weapons\\rifle\\assault_rifle\\smg",0x77656170u},
{"objects\\vehicles\\covenant\\banshee\\banshee",0x76656869u},
{"objects\\vehicles\\covenant\\ghost\\ghost",0x76656869u},
{"objects\\vehicles\\human\\scorpion\\scorpion",0x76656869u},
{"objects\\vehicles\\covenant\\turrets\\shade\\shade",0x76656869u},
{"objects\\vehicles\\human\\warthog\\warthog",0x76656869u},
{"objects\\vehicles\\covenant\\wraith\\wraith",0x76656869u},
{"objects\\weapons\\rifle\\assault_rifle\\assault_rifle",0x77656170u},
{"objects\\weapons\\rifle\\battle_rifle\\battle_rifle",0x77656170u},
{"objects\\weapons\\rifle\\beam_rifle\\beam_rifle",0x77656170u},
{"objects\\weapons\\melee\\energy_sword\\energy_sword",0x77656170u},
{"objects\\weapons\\support_high\\flak_cannon\\flak_cannon",0x77656170u},
{"objects\\weapons\\pistol\\magnum\\magnum",0x77656170u},
{"objects\\weapons\\rifle\\needle_rifle\\needle_rifle",0x77656170u},
{"objects\\weapons\\pistol\\needler\\needler",0x77656170u},
{"objects\\weapons\\pistol\\plasma_pistol\\plasma_pistol",0x77656170u},
{"objects\\weapons\\rifle\\plasma_rifle\\plasma_rifle",0x77656170u},
{"objects\\weapons\\rifle\\plasma_rifle_red\\plasma_rifle_red",0x77656170u},
{"objects\\weapons\\support_high\\rocket_launcher\\rocket_launcher",0x77656170u},
{"objects\\weapons\\support_low\\sentinel_gun\\sentinel_gun",0x77656170u},
{"objects\\weapons\\rifle\\shotgun\\shotgun",0x77656170u},
{"objects\\weapons\\rifle\\sniper_rifle\\sniper_rifle",0x77656170u},
};
static int ai6_item_path_allowed(uint32_t group, const char *path) {
    size_t i;
    if (!path) return 0;
    for (i=0;i<sizeof(ai6_items)/sizeof(ai6_items[0]);++i)
        if (group==ai6_items[i].group && tag_name_equals(path,ai6_items[i].path)) return 1;
    return 0;
}
static int ai1_path_allowed(uint32_t group, const char *path) {
    if (ai6_item_path_allowed(group,path)) return 1;
    size_t i;
    for (i=0;i<sizeof(ai1_profiles)/sizeof(ai1_profiles[0]);++i)
        if ((group==0x62697064u && !strcmp(path,ai1_profiles[i].biped)) ||
            (group==0x63686172u && !strcmp(path,ai1_profiles[i].character))) return 1;
    return 0;
}
static const char *ai1_tag_path(int32_t datum, uint32_t group) {
    uint32_t count=0, slot=(uint32_t)datum & 0xffffu;
    uint8_t *entries=tag_table_entries(&count), *entry;
    if (!entries || datum==-1 || slot>=count) return NULL;
    entry=entries+slot*48u;
    if (!*(uint16_t *)entry || *(uint16_t *)entry!=(uint16_t)((uint32_t)datum>>16) ||
        *(uint32_t *)(entry+4)!=group) return NULL;
    return *(const char **)(entry+0x10);
}

/* Matches the ai_enable/ai_enabled evaluators in this exact CU4 binary:
 * initialized TLS+0x14; AI globals pointer at TLS+0x40; bool at +0.
 * Spawn setup's temporary actor-context flag is the distinct byte +1. */
/* CU4 ai_disregard evaluator at 0x1C3DD0: player-only bit operation.
 * No global AI pause, team edits, damage immunity, or recurring scans. */
static int ai21_observer(int32_t player, int enabled) {
    static const uint8_t expected[]={0x8B,0x81,0xB4,0x01,0x00,0x00,0x45,0x84,0xD2,0x74,0x05,0x83,0xC8,0x20,0xEB,0x03,0x83,0xE0,0xDF,0x89,0x81,0xB4,0x01,0x00,0x00};
    uint8_t *unit;uint32_t before,after;
    if(enabled<0 || enabled>1 || !is_live_player_unit(player))return 0;
    if(!bytes_equal(g_blam+0x1C3EE1,expected,sizeof(expected)))return 0;
    unit=object_data(player);
    if(!unit || !readable_range(unit+0x1B4,4) || !writable_range(unit+0x1B4,4))return 0;
    before=*(uint32_t *)(unit+0x1B4);
    after=enabled?(before|0x20u):(before&~0x20u);
    *(uint32_t *)(unit+0x1B4)=after;MemoryBarrier();
    return *(uint32_t *)(unit+0x1B4)==after;
}

static int ai6_control(int32_t player, int enabled, int *before_out) {
    uint8_t *tls, *globals;
    if ((enabled != 0 && enabled != 1) || !is_live_player_unit(player)) return 0;
    tls = blam_thread_state();
    if (!tls || !readable_range(tls + 0x14, 1) || !tls[0x14] ||
        !readable_range(tls + 0x40, sizeof(void *))) return 0;
    globals = *(uint8_t **)(tls + 0x40);
    if (!globals || !readable_range(globals, 1) || !writable_range(globals, 1) || globals[0] > 1) return 0;
    *before_out = globals[0];
    globals[0] = (uint8_t)enabled;
    MemoryBarrier();
    return globals[0] == (uint8_t)enabled;
}
static int ai6_item_spawn_allowed(const struct native_command *command) {
    const char *weapon, *vehicle;
    float origin[3], dx, dy, dz;
    if (!is_live_player_unit(command->a) || !valid_transform(command)) return 0;
    weapon = ai1_tag_path(command->b, 0x77656170u);
    vehicle = ai1_tag_path(command->b, 0x76656869u);
    if (!ai6_item_path_allowed(0x77656170u, weapon) &&
        !ai6_item_path_allowed(0x76656869u, vehicle)) return 0;
    ((object_get_origin_fn)(g_blam + 0x5A6780))(command->a, origin);
    dx=command->x-origin[0];dy=command->y-origin[1];dz=command->z-origin[2];
    return isfinite(dx) && isfinite(dy) && isfinite(dz) && dx*dx+dy*dy+dz*dz <= 900.0f;
}

static int ai13_fixed_allowed(const char *biped,int profile) {
    size_t n=0,i;int base=0;const struct weighted_weapon_entry *entries;
    if(profile<100 || profile>115 || !biped)return 0;
    if(tag_name_equals(biped,"objects\\characters\\marine\\marine"))base=1;
    if(tag_name_equals(biped,"objects\\characters\\marine_female\\marine_female"))base=1;
    if(tag_name_equals(biped,"objects\\characters\\elite\\elite"))base=2;
    if(tag_name_equals(biped,"objects\\characters\\grunt\\grunt"))base=3;
    if(tag_name_equals(biped,"objects\\characters\\jackal\\jackal"))base=4;
    if(tag_name_equals(biped,"objects\\characters\\brute\\brute"))base=6;
    if(base==1)return profile!=107; /* Manual Marines: all fixed loadouts except energy sword. */
    entries=weapons_for_loadout(base,&n);
    for(i=0;entries && i<n;i++)if(tag_name_equals(entries[i].path,g_fixed_weapons[profile-100].path))return 1;
    return 0;
}
static int ai5_loadout_allowed(const char *biped, int profile) {
    if(tag_name_equals(biped,"objects\\characters\\johnson\\johnsonn"))return profile==AI_LOADOUT_HUMAN;
    if(profile>=100)return ai13_fixed_allowed(biped,profile);
    if (tag_name_equals(biped,"objects\\characters\\brute\\brute")) return profile == AI_LOADOUT_BRUTE;
    if (profile == AI_LOADOUT_KEEP_DEFAULT) return 1;
    if (!biped) return 0;
    if (tag_name_equals(biped, "objects\\characters\\marine\\marine") ||
        tag_name_equals(biped, "objects\\characters\\marine_female\\marine_female")) return profile == AI_LOADOUT_HUMAN;
    if (tag_name_equals(biped, "objects\\characters\\elite\\elite")) return profile == AI_LOADOUT_ELITE;
    if (tag_name_equals(biped, "objects\\characters\\grunt\\grunt")) return profile == AI_LOADOUT_GRUNT;
    if (tag_name_equals(biped, "objects\\characters\\jackal\\jackal")) return profile == AI_LOADOUT_JACKAL;
    return 0;
}
static void ai1_trace(const char *stage);
static char ai10_spawn_denial[320];
/* Only supply the missing player/human allegiance on C45 and D20.
 * Existing entries, including broken alliances, are never reset.
 * Runs on a validated human spawn, not on a timer. */
typedef void (__fastcall *ai25_defaults_fn)(int,int,uint8_t*,uint8_t*,int16_t*,int16_t*);
typedef void (__fastcall *ai25_add_fn)(int,uint8_t,int,uint8_t,int16_t,int16_t,uint8_t);
static int ai25_pair(const uint8_t *table,int count) {
    int i;
    for(i=0;i<count;i++) {
        const uint8_t *entry=table+4+i*20;
        if((entry[2]==1 && entry[3]==2)||(entry[2]==2 && entry[3]==1))return i;
    }
    return -1;
}
static int ai25_missing_human_alliance(int team,char *error,size_t capacity) {
    uint8_t *scenario,*first,*last,*tls,*table;int count,map=0,index;
    uint8_t a=0,b=0;int16_t x=0,y=0;char log[240];
    if(team!=2)return 1;
    if(!g_blam || !readable_range(g_blam+0x10C3558,sizeof(void *)))return 0;
    scenario=*(uint8_t **)(g_blam+0x10C3558);
    if(!scenario || !readable_range(scenario+0x348,8))return 0;
    first=resolve_scenario_squad(scenario,0);
    if(!first || !readable_range(first,32))return 0;
    if(!strncmp((char *)first,"sq_e61_a_empa",32)) {
        last=resolve_scenario_squad(scenario,250);
        if(last && readable_range(last,32) && !strncmp((char *)last,"sq_e6_a_gmna_vignette",32))map=45;
    } else if(!strncmp((char *)first,"sq_enc7_1_top_left_gf",32)) {
        last=resolve_scenario_squad(scenario,196);
        if(last && readable_range(last,32) && !strncmp((char *)last,"sq_enc3_4_flood_infectors",32))map=20;
    }
    if(!map)return 1;
    tls=blam_thread_state();
    if(!tls || !readable_range(tls+0x130,sizeof(void *)))return 0;
    table=*(uint8_t **)(tls+0x130);
    if(!table || !readable_range(table,0x1c4))return 0;
    count=*(int16_t *)table;
    if(count<0 || count>16){snprintf(error,capacity,"alliance table invalid");return 0;}
    index=ai25_pair(table,count);
    if(index>=0) {
        snprintf(log,sizeof(log),"AI25 alliance existing mission=%d pair=1,2 entry=%d state=%u preserved=1",map,index,table[4+index*20+10]);
        ai1_trace(log);return 1;
    }
    if(count==16){snprintf(error,capacity,"alliance table full; human spawn held");return 0;}
    /* Exact CU4 profile already validated; retain explicit call-site guards. */
    static const uint8_t defaults_sig[]={0x48,0x89,0x5c,0x24,0x18,0x48,0x89,0x6c,0x24,0x20,0x56,0x57,0x41,0x57,0x48,0x83,0xec,0x40};
    static const uint8_t add_sig[]={0x40,0x53,0x55,0x56,0x57,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x83,0xec,0x28};
    if(!bytes_equal(g_blam+0x26BBE0,defaults_sig,sizeof(defaults_sig)) ||
       !bytes_equal(g_blam+0x26C050,add_sig,sizeof(add_sig))) {
        snprintf(error,capacity,"alliance function signature mismatch");return 0;
    }
    /* Same default thresholds and argument order as ai_allegiance(player,human).
     * No remove call is needed because the pair is absent. */
    ((ai25_defaults_fn)(g_blam+0x26BBE0))(1,2,&a,&b,&x,&y);
    ((ai25_add_fn)(g_blam+0x26C050))(1,a,2,b,x,y,0);
    count=*(int16_t *)table;
    if(count<1 || count>16 || (index=ai25_pair(table,count))<0 ||
       table[4+index*20+10]!=0 ||
       !(*(uint32_t *)(table+0x184+1*4)&(1u<<2)) ||
       !(*(uint32_t *)(table+0x184+2*4)&(1u<<1))) {
        snprintf(error,capacity,"player-human alliance setup did not verify");return 0;
    }
    snprintf(log,sizeof(log),"AI25 alliance added mission=%d pair=1,2 defaults=%u,%u thresholds=%d,%d entries=%d engine_script_equivalent=1",map,a,b,x,y,count);
    ai1_trace(log);return 1;
}

static int ai1_spawn_allowed(const struct native_command *command) {
    const char *biped=ai1_tag_path(command->b,0x62697064u);
    ai10_spawn_denial[0]=0;
    const char *character=ai1_tag_path(command->d,0x63686172u);
    size_t i;
    if (command->marine_mode != 0 &&
        ((command->marine_mode < 0 || command->marine_mode > 2) || command->c != 2 || !biped ||
         (!tag_name_equals(biped,"objects\\characters\\marine\\marine") &&
          !tag_name_equals(biped,"objects\\characters\\marine_female\\marine_female")))) return 0;
    if (!ai5_loadout_allowed(biped, command->e) || !is_live_player_unit(command->a)) return 0;
    {
        int32_t reserved_index,reserved_count;
        int maw=ai23_maw_squad(command->c,command->marine_mode!=0,
            command->c==2?existing_player_fireteam_datum(command->a):-1,
            &reserved_index,&reserved_count,ai10_spawn_denial,sizeof(ai10_spawn_denial));
        if(maw==0)return 0;
    }
    if ((command->c == 2 && command->marine_mode != 0) || command->c == 3 || command->c == 8 || command->c == 9) {
        int32_t reserved_index, reserved_count;
        char reason[160];
        if (select_e10_defined_enemy_squad(command->c, &reserved_index, &reserved_count, reason, sizeof(reason)) == 0) {
            snprintf(ai10_spawn_denial,sizeof(ai10_spawn_denial),"%s",reason);
            ai1_trace(reason);
            return 0;
        }
    }
    for (i=0;i<sizeof(ai1_profiles)/sizeof(ai1_profiles[0]);++i)
        if (command->c==ai1_profiles[i].team &&
            tag_name_equals(biped,ai1_profiles[i].biped) &&
            tag_name_equals(character,ai1_profiles[i].character)) return 1;
    return 0;
}
static void ai1_trace(const char *stage) {
    wchar_t path[MAX_PATH];
    char text[4096];
    DWORD written;
    HANDLE file;
    int n;
    sibling_path(path,MAX_PATH,L"ai_trace.txt");
    file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if (file==INVALID_HANDLE_VALUE) return;
    n=snprintf(text,sizeof(text),"%llu tid=%lu %s\r\n",GetTickCount64(),GetCurrentThreadId(),stage);
    if (n>0) {
        DWORD length=(DWORD)(n<(int)sizeof(text)?n:(int)sizeof(text)-1);
        WriteFile(file,text,length,&written,NULL);
    }
    FlushFileBuffers(file);
    CloseHandle(file);
}


/* CU4 character schema: unit tag reference at +0x14, datum at +0x20.
 * Only the two explicitly selected Chieftain profiles may temporarily use
 * the tested Brute body. Restore immediately after synchronous creation. */
static int ai12_chieftain_begin(int32_t character, int32_t body,
                               int32_t **field_out, int32_t *saved_out) {
    const char *name=ai1_tag_path(character,0x63686172u);
    uint8_t *definition;
    int32_t *field;
    *field_out=NULL;
    if (!tag_name_equals(name,"objects\\characters\\brute\\ai\\brute_chieftain_weapon") &&
        !tag_name_equals(name,"objects\\characters\\brute\\ai\\brute_chieftain_armor")) return 0;
    if (!tag_name_equals(ai1_tag_path(body,0x62697064u),"objects\\characters\\brute\\brute")) return -1;
    definition=((tag_get_fn)(g_blam+0x201480))(0x63686172u,character);
    if (!definition || !readable_range(definition+0x14,16) ||
        *(uint32_t *)(definition+0x14)!=0x62697064u || !writable_range(definition+0x20,4)) return -1;
    field=(int32_t *)(definition+0x20);
    if (!tag_name_equals(ai1_tag_path(*field,0x62697064u),"objects\\characters\\brute_chieftain\\brute_chieftain")) return -1;
    *saved_out=*field;*field_out=field;*field=body;MemoryBarrier();return 1;
}
static void ai12_chieftain_end(int32_t *field, int32_t saved) {
    if (field) {*field=saved;MemoryBarrier();}
}


static int ai15_marine_alive(int32_t datum) {
    uint8_t *unit=object_data(datum);float health;
    if (!unit) return 0;
    if (!readable_range(unit,sizeof(int32_t)) ||
        !tag_name_equals(ai1_tag_path(*(int32_t *)unit,0x62697064u),"objects\\characters\\marine\\marine")) return -1;
    health=((unit_get_vitality_fn)(g_blam+0x5B7E60))(datum);
    return isfinite(health) ? (health>0.0f) : -1;
}
static int ai20_johnson_alive(int32_t datum) {
    uint8_t *unit=object_data(datum);float health;
    if (!unit) return 0;
    if (!readable_range(unit,sizeof(int32_t)) ||
        !tag_name_equals(ai1_tag_path(*(int32_t *)unit,0x62697064u),"objects\\characters\\johnson\\johnsonn")) return -1;
    health=((unit_get_vitality_fn)(g_blam+0x5B7E60))(datum);
    return isfinite(health) ? (health>0.0f) : -1;
}

/* Experimental initial hunt: bounded, spawn-time enemy awareness through the
 * same perception calls as ai_magically_see. No timer or world-object scan. */
static int ai27_search_seed(int32_t actor_id,int32_t unit_id) {
    typedef int32_t (__fastcall *find_fn)(int32_t,int32_t);
    typedef int32_t (__fastcall *create_fn)(int32_t,int32_t,int);
    typedef int32_t (__fastcall *ticks_fn)(float);
    uint8_t *actor=actor_data(actor_id,unit_id),*tls=blam_thread_state(),*table,*entries,*relations,*perception,*knowledge;
    uint32_t *bits;int high,capacity,i,j,n=0,added=0;int32_t group,targets[8];float distances[8],origin[3];
    static const uint8_t find_sig[]={0x40,0x53,0x55,0x56,0x57,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x83,0xec};
    static const uint8_t create_sig[]={0x40,0x53,0x55,0x56,0x57,0x48,0x81,0xec,0x58,0x01,0x00,0x00,0x41,0x8b,0xe8,0xc7};
    if(!actor || !readable_range(actor,0x80) || actor[0x2c]!=2 || !tls || !readable_range(tls,0x138))return -1;
    if(!bytes_equal(g_blam+0x7C540,find_sig,sizeof(find_sig)) || !bytes_equal(g_blam+0x7BF30,create_sig,sizeof(create_sig)))return -1;
    group=*(int32_t *)(actor+0x7c);if(group==-1)return -1;
    perception=*(uint8_t **)(tls+0xa0);knowledge=*(uint8_t **)(tls+0x70);
    if(!perception || !knowledge || !readable_range(perception,0x60) || !readable_range(knowledge,0x60))return -1;
    {uint8_t *groups=*(uint8_t **)(perception+0x50);int cap=*(int32_t *)(perception+0x2c);unsigned slot=(uint16_t)group;
     if(!groups || cap<=0 || cap>16384 || slot>=(unsigned)cap || !readable_range(groups+slot*0x10c,0x10c) || *(uint16_t *)(groups+slot*0x10c)!=(uint16_t)((uint32_t)group>>16))return -1;}
    table=current_actor_table();relations=*(uint8_t **)(tls+0x130);
    if(!table || !readable_range(table,0x60) || !relations || !readable_range(relations,0x1c4))return -1;
    capacity=*(int32_t *)(table+0x2c);high=*(int32_t *)(table+0x44);entries=*(uint8_t **)(table+0x50);bits=*(uint32_t **)(table+0x58);
    if(capacity<=0 || capacity>16384 || high<0 || high>capacity || !entries || !bits || !readable_range(bits,((capacity+31u)/32u)*4))return -1;
    ((object_get_origin_fn)(g_blam+0x5A6780))(unit_id,origin);
    /* Native actor slots only, at most 512; keep the closest eight enemies
     * within 60 world units, never player, humans or neutral actors. */
    for(i=0;i<high && i<512;i++) {
        uint8_t *a;int team;int32_t u,id;float p[3],dx,dy,dz,dist;
        if(!(bits[i>>5]&(1u<<(i&31))))continue;
        a=entries+(size_t)i*0xd10;
        if(!readable_range(a,0x80) || (a[6]&2))continue;
        team=a[0x2c];if(team!=3 && team!=8 && team!=9)continue;
        if(*(uint32_t *)(relations+0x184+2*4)&(1u<<team))continue;
        u=*(int32_t *)(a+0x1c);id=(int32_t)(((uint32_t)*(uint16_t *)a<<16)|(unsigned)i);
        if(!actor_data(id,u) || ((unit_get_vitality_fn)(g_blam+0x5B7E60))(u)<=0.0f)continue;
        ((object_get_origin_fn)(g_blam+0x5A6780))(u,p);dx=p[0]-origin[0];dy=p[1]-origin[1];dz=p[2]-origin[2];dist=dx*dx+dy*dy+dz*dz;
        if(!isfinite(dist) || dist>3600.0f)continue;
        if(n==8 && distances[7]<=dist)continue;
        j=n<8?n++:7;
        while(j>0 && distances[j-1]>dist){if(j<8){distances[j]=distances[j-1];targets[j]=targets[j-1];}j--;}
        distances[j]=dist;targets[j]=u;
    }
    for(i=0;i<n;i++) {
        int32_t datum=((find_fn)(g_blam+0x7C540))(group,targets[i]);
        if(datum==-1)datum=((create_fn)(g_blam+0x7BF30))(group,targets[i],6);
        if(datum!=-1) {
            unsigned slot=(uint16_t)datum;int cap=*(int32_t *)(knowledge+0x2c);uint8_t *data=*(uint8_t **)(knowledge+0x50),*entry;
            if(!data || cap<=0 || cap>16384 || slot>=(unsigned)cap)continue;
            entry=data+(size_t)slot*0xb0;
            if(!readable_range(entry,0x40) || *(uint16_t *)entry!=(uint16_t)((uint32_t)datum>>16) || !writable_range(entry+0x3c,2))continue;
            int32_t duration=((ticks_fn)(g_blam+0x210840))(30.0f);
            if(duration<0 || duration>32767)continue;
            if(*(int16_t *)(entry+0x3c)<duration)*(int16_t *)(entry+0x3c)=(int16_t)duration;
            added++;
        }
    }
    return added;
}

static void execute_ai_attach(const struct native_command *command,
                              int32_t known_tag_datum,
                              int32_t known_actor_variant,
                              const float *known_origin,
                              int32_t follow_player_unit) {
    uint8_t *unit;
    uint8_t *definition;
    uint8_t *thread_state = NULL;
    uint8_t *runtime_state = NULL;
    uint8_t *actor_globals = NULL;
    uint8_t *actor;
    uint64_t *runtime_flags_address = NULL;
    uint64_t runtime_flags = 0;
    uint64_t forced_runtime_flags = 0;
    uint8_t actor_context_original;
    uint8_t ai_phase_original;
    int actor_context_forced = 0;
    int ai_phase_forced = 0;
    int runtime_flag_forced = 0;
    int32_t tag_datum;
    int32_t actor_variant;
    int32_t returned_actor;
    int32_t active_unit;
    int search_targets = -1;
    int32_t fireteam_datum = -1;
    int32_t fireteam_members = 0;
    int32_t preferred_fireteam = -1;
    int32_t live_squad_index = -1;
    int32_t live_squad_members = 0;
    int runtime_squad_created = 0;
    int follow_attached = 0;
    int fireteam_squad_linked = 0;
    int fireteam_actor_member = 0;
    int32_t fireteam_evicted_actor = -1;
    int squad_attached = 0;
    int actor_force_active = 0;
    int damageproof_cleared = 0;
    int protected_special_units = 0;
    int special_units_disregarded = 0;
    int weapon_override = 0;
    int weapon_removed = 0;
    int weapon_restored = 0;
    int32_t random_weapon_tag = -1;
    int template_destroy_requested = 0;
    int secondary_link_cleared = 0;
    char follow_error[256] = {0};
    char squad_error[256] = {0};
    char secondary_error[256] = {0};
    char disregard_error[256] = {0};
    char random_weapon_path[192] = {0};
    uint16_t variant_name;
    uint8_t actor_flags_before = 0;
    uint8_t actor_flags_after = 0;
    _Alignas(16) uint8_t origin[0x80];

    if (command->a == -1 || command->b < 0 || command->b > 15) {
        snprintf(g_result, sizeof(g_result), "error ai_attach commande refusee\n");
        return;
    }
    unit = object_data(command->a);
    if ((!unit || !readable_range(unit, 0x1B0)) && known_tag_datum == -1) {
        snprintf(g_result, sizeof(g_result), "error ai_attach unite absente datum=%d\n", command->a);
        return;
    }
    if (unit && readable_range(unit + 0x1AC, sizeof(int32_t)) && *(int32_t *)(unit + 0x1AC) != -1) {
        snprintf(g_result, sizeof(g_result), "error ai_attach acteur deja present unit=%d actor=%d\n",
                 command->a, *(int32_t *)(unit + 0x1AC));
        return;
    }
    tag_datum = known_tag_datum != -1 ? known_tag_datum : *(int32_t *)unit;
    ai1_trace("before biped tag lookup");
    definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(0x64706962u, tag_datum);
    if (!definition) {
        snprintf(g_result, sizeof(g_result), "error ai_attach biped invalide unit=%d tag=%d\n",
                 command->a, tag_datum);
        return;
    }
    variant_name = readable_range(definition + 0x6F8, sizeof(variant_name))
        ? *(uint16_t *)(definition + 0x6F8) : 0;
    if (known_actor_variant != -1) {
        uint8_t *actor_variant_definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
            0x72616863u, known_actor_variant);
        if (!actor_variant_definition || !readable_range(actor_variant_definition, sizeof(void *))) {
            snprintf(g_result, sizeof(g_result),
                     "error ai_attach variante acteur invalide unit=%d tag=%d actor_variant=%d\n",
                     command->a, tag_datum, known_actor_variant);
            return;
        }
        actor_variant = known_actor_variant;
    } else {
        if (!readable_range(definition + 0x4BC, 1) ||
            !readable_range(definition + 0x6F4, sizeof(actor_variant))) {
            snprintf(g_result, sizeof(g_result), "error ai_attach definition biped incomplete unit=%d tag=%d\n",
                     command->a, tag_datum);
            return;
        }
        actor_variant = *(int32_t *)(definition + 0x6F4);
        if (!(definition[0x4BC] & 0x10u) || actor_variant == -1) {
            snprintf(g_result, sizeof(g_result),
                     "error ai_attach aucun acteur par defaut unit=%d tag=%d flags=%02X variant=%d\n",
                     command->a, tag_datum, (unsigned)definition[0x4BC], actor_variant);
            return;
        }
    }
    if (!ai_runtime_context(&thread_state, &runtime_state, &runtime_flags_address, &runtime_flags) ||
        !readable_range(thread_state + 0x40, sizeof(void *))) {
        snprintf(g_result, sizeof(g_result), "error ai_attach contexte IA indisponible\n");
        return;
    }
    actor_globals = *(uint8_t **)(thread_state + 0x40);
    if (!actor_globals || !readable_range(actor_globals, 4) || !writable_range(actor_globals + 1, 1) ||
        !writable_range(runtime_state + 0x10, 1)) {
        snprintf(g_result, sizeof(g_result), "error ai_attach contexte IA illisible\n");
        return;
    }
    actor_context_original = actor_globals[1];
    ai_phase_original = runtime_state[0x10];
    if (!actor_context_original) {
        actor_globals[1] = 1;
        actor_context_forced = 1;
    }
    if (ai_phase_original != 2) {
        runtime_state[0x10] = 2;
        ai_phase_forced = 1;
    }
    if (!(runtime_flags & 0x0000002000000000ULL)) {
        if (!writable_range(runtime_flags_address, sizeof(*runtime_flags_address))) {
            if (ai_phase_forced) runtime_state[0x10] = ai_phase_original;
            if (actor_context_forced) actor_globals[1] = actor_context_original;
            snprintf(g_result, sizeof(g_result), "error ai_attach masque runtime non modifiable\n");
            return;
        }
        forced_runtime_flags = runtime_flags | 0x0000002000000000ULL;
        ((runtime_flags_set_fn)(g_blam + 0x209FC0))(&forced_runtime_flags);
        runtime_flag_forced = 1;
    }
    memset(origin, 0, sizeof(origin));
    if (known_origin) {
        memcpy(origin, known_origin, sizeof(float) * 3u);
    } else {
        ((object_get_origin_fn)(g_blam + 0x5A6780))(command->a, (float *)origin);
    }
    int32_t *chieftain_field=NULL, chieftain_saved=-1;
    int chieftain_fix=ai12_chieftain_begin(actor_variant,tag_datum,&chieftain_field,&chieftain_saved);
    ai1_trace(chieftain_fix<0 ? "Chieftain unit-reference validation failed" : chieftain_fix>0 ? "Chieftain unit-reference temporarily mapped to Brute" : "standard actor profile");
    ai1_trace("before actor creation");
    returned_actor = chieftain_fix<0 ? -1 : ((actor_attach_to_unit_fn)(g_blam + 0x0059340))(
        command->a, -1, actor_variant, variant_name, 1, origin);
    ai12_chieftain_end(chieftain_field,chieftain_saved);
    ai1_trace("actor creation returned; character reference restored");
    if (runtime_flag_forced) ((runtime_flags_set_fn)(g_blam + 0x209FC0))(&runtime_flags);
    if (ai_phase_forced) runtime_state[0x10] = ai_phase_original;
    if (actor_context_forced) actor_globals[1] = actor_context_original;
    MemoryBarrier();

    /* 0x59870 attend un datum ACTEUR, jamais le datum de l'unite. L'ancien
     * appel pre-attachement indexait la table d'acteurs avec un datum objet et
     * provoquait le crash exact 0x8C0DD. */
    if (returned_actor == -1) {
        if (object_data(command->a)) ((object_delete_fn)(g_blam + 0x350960))(command->a);
        ai1_trace("failed actor creation: temporary body removed");
        snprintf(g_result, sizeof(g_result),
                 "error ai_attach creation acteur refusee unit=%d tag=%d variant=%d returned=%d runtime_forced=%d actor_context_forced=%d ai_phase_forced=%d\n",
                 command->a, tag_datum, actor_variant, returned_actor,
                 runtime_flag_forced, actor_context_forced, ai_phase_forced);
        return;
    }
    /* Cette fonction moteur utilise le biped fourni comme modele, mais cree
     * son propre biped actif. Le datum retourne est donc celui de l'acteur,
     * dont +0x1C contient la vraie unite controlee. L'ancienne validation
     * contre command->a rejetait cet acteur valide et laissait deux corps. */
    actor = actor_data_raw(returned_actor);
    if (!actor || !readable_range(actor + 0x2C, 1)) {
        snprintf(g_result, sizeof(g_result),
                 "error ai_attach acteur renvoye illisible template=%d actor=%d tag=%d variant=%d lookup={%s}\n",
                 command->a, returned_actor, tag_datum, actor_variant,
                 g_actor_lookup_detail);
        return;
    }
    active_unit = *(int32_t *)(actor + 0x1C);
    if (active_unit == -1 || !actor_data(returned_actor, active_unit)) {
        uint8_t *active_object = active_unit != -1 ? object_data(active_unit) : NULL;
        int32_t reverse_actor = active_object && readable_range(active_object + 0x1AC, sizeof(int32_t))
            ? *(int32_t *)(active_object + 0x1AC) : -1;
        snprintf(g_result, sizeof(g_result),
                 "error ai_attach liens acteur invalides template=%d active=%d actor=%d reverse=%d tag=%d variant=%d\n",
                 command->a, active_unit, returned_actor, reverse_actor, tag_datum, actor_variant);
        return;
    }
    ai1_trace("before secondary link cleanup");
    if (!clear_actor_secondary_link(returned_actor, actor,
                                    &secondary_link_cleared,
                                    secondary_error, sizeof(secondary_error))) {
        snprintf(g_result, sizeof(g_result),
                 "error ai_attach lien secondaire dangereux template=%d active=%d actor=%d detail={%s}\n",
                 command->a, active_unit, returned_actor, secondary_error);
        return;
    }
    actor = actor_data(returned_actor, active_unit);
    if (!actor || !readable_range(actor + 0x2C, 1)) {
        snprintf(g_result, sizeof(g_result),
                 "error ai_attach acteur perdu apres nettoyage template=%d active=%d actor=%d\n",
                 command->a, active_unit, returned_actor);
        return;
    }
    ai1_trace("before actor team check");
    if ((int8_t)actor[0x2C] != (int8_t)command->b) {
        ((actor_set_team_fn)(g_blam + 0x0059870))(returned_actor, command->b);
    }
    actor = actor_data(returned_actor, active_unit);
    if (!actor || *(int32_t *)(actor + 0x1C) != active_unit ||
        (int8_t)actor[0x2C] != (int8_t)command->b) {
        snprintf(g_result, sizeof(g_result),
                 "error ai_attach equipe acteur non confirmee template=%d active=%d actor=%d team=%d\n",
                 command->a, active_unit, returned_actor, command->b);
        return;
    }

    if (!ensure_object_damageable(active_unit, &damageproof_cleared)) {
        if (command->a != active_unit && object_data(command->a))
            ((object_delete_fn)(g_blam + 0x350960))(command->a);
        if (object_data(active_unit))
            ((object_delete_fn)(g_blam + 0x350960))(active_unit);
        snprintf(g_result, sizeof(g_result),
                 "error ai_attach unite invulnerable refusee template=%d active=%d actor=%d cleanup=1\n",
                 command->a, active_unit, returned_actor);
        return;
    }

    /* L'acteur existe maintenant avec ses liens croises valides. On peut
     * remplacer son equipement initial par une arme aleatoire de sa faction
     * en utilisant la meme routine retail que le placement d'IA du scenario. */
    if (command->e != AI_LOADOUT_KEEP_DEFAULT) {
        weapon_override = equip_random_faction_weapon(
            actor_variant, returned_actor, active_unit, command->e,
            &random_weapon_tag, &weapon_removed, &weapon_restored,
            random_weapon_path, sizeof(random_weapon_path));
    }

    /* Toute IA doit appartenir a un live squad pour recevoir la navigation,
     * les reactions de combat, la poursuite et la fuite. Les allies reutilisent
     * d'abord un squad humain de la mission lorsqu'il existe; sinon un squad
     * runtime pur est cree. Les ennemis utilisent toujours un squad runtime
     * propre au mod afin de ne jamais detourner un encounter scenarise. */
    if (command->b == 2 && follow_player_unit != -1) {
        preferred_fireteam = existing_player_fireteam_datum(follow_player_unit);
    }
    ai1_trace("before squad migration");
    squad_attached = migrate_actor_to_live_squad(
        returned_actor, active_unit, command->b, preferred_fireteam,
        command->b != 2 || command->marine_mode != 0,
        &live_squad_index, &live_squad_members,
        &runtime_squad_created,
        squad_error, sizeof(squad_error));
    if (squad_attached && command->marine_mode != 0) {
        /* Wake this independent squad through the same retail activation
         * path used by Follow, without assigning a player fireteam. */
        ((squad_activate_fn)(g_blam + 0x04F6A0))(live_squad_index);
        MemoryBarrier();
        uint8_t *free_squad=live_squad_data(blam_thread_state(),live_squad_index);
        if (!free_squad || *(int32_t *)(free_squad+0xB0)!=-1 ||
            !(*(uint16_t *)(free_squad+2)&0x80u)) {
            squad_attached=0;
            snprintf(squad_error,sizeof(squad_error),"Free Combat squad activation not confirmed");
        }
    }
    if (!squad_attached) {
        ai1_trace("before template removal");
    if (command->a != active_unit && object_data(command->a)) {
            ((object_delete_fn)(g_blam + 0x350960))(command->a);
        }
        if (object_data(active_unit)) {
            ((object_delete_fn)(g_blam + 0x350960))(active_unit);
        }
        snprintf(g_result, sizeof(g_result),
                 "error ai_attach migration squad refusee template=%d unit=%d actor=%d team=%d cleanup=1 detail={%s}\n",
                 command->a, active_unit, returned_actor, command->b,
                 squad_error);
        return;
    }

    /* La migration alliee remet momentanement actor+0x06 a zero, puis le
     * moteur reconstruit ses drapeaux au tick suivant. Pour les ennemis libres,
     * le bit vivant 0x10 est encore present et le wrapper retail autorise alors
     * le bit force-active 0x20. */
    actor = actor_data(returned_actor, active_unit);
    if (!actor || !readable_range(actor + 0x06, 1)) {
        snprintf(g_result, sizeof(g_result),
                 "error ai_attach acteur perdu apres migration template=%d active=%d actor=%d squad=%d\n",
                 command->a, active_unit, returned_actor, live_squad_index);
        return;
    }
    actor_flags_before = actor[0x06];
    actor_flags_after = actor_flags_before;
    if (actor_flags_before & 0x10u) {
        actor_force_active = force_actor_active_by_unit(
            returned_actor, active_unit,
            &actor_flags_before, &actor_flags_after);
    }
    if (command->b == 2 && follow_player_unit != -1) {
    ai1_trace("before Marine fireteam attachment");
        follow_attached = add_actor_to_player_fireteam(
            follow_player_unit, returned_actor, live_squad_index,
            &fireteam_datum, &fireteam_members,
            &fireteam_squad_linked,
            &fireteam_actor_member,
            &fireteam_evicted_actor,
            follow_error, sizeof(follow_error));
    }

    /* L'equipe 12 est celle de Guilty Spark dans la campagne. Le bit retail
     * ai_disregard doit etre pose pour tous les camps : un Marine allie ne
     * doit plus tirer sur l'Oracle et un ennemi ne doit pas interrompre sa
     * tactique pour le poursuivre. L'absence d'une unite equipe 12 dans les
     * autres missions est un succes avec un compteur nul. */
    ai1_trace("before special-unit exclusion");
    special_units_disregarded = disregard_actor_units_for_team(
        12, &protected_special_units,
        disregard_error, sizeof(disregard_error));

    /* Le biped modele n'a aucun acteur. Une fois le lien croise de l'unite
     * active confirme, il peut etre detruit par le chemin officiel de script
     * object_destroy (RVA 0x350960 -> object_delete). */
    ai1_trace("before successful template cleanup");
    if (command->a != active_unit && object_data(command->a)) {
        ((object_delete_fn)(g_blam + 0x350960))(command->a);
        template_destroy_requested = 1;
    }
    ai1_trace("successful template cleanup returned");
    if(command->marine_mode==2) {
        search_targets=ai27_search_seed(returned_actor,active_unit);
        char note[160];snprintf(note,sizeof(note),"AI27 initial search unit=%d actor=%d targets=%d",active_unit,returned_actor,search_targets);ai1_trace(note);
    }
    MemoryBarrier();

    if (command->b == 2 && follow_player_unit != -1 && !follow_attached) {
        snprintf(g_result, sizeof(g_result),
                 "error ai_attach suivi refuse template=%d unit=%d actor=%d team=%d template_destroy=%d detail={%s}\n",
                 command->a, active_unit, returned_actor, command->b,
                 template_destroy_requested, follow_error);
        return;
    }
    if (command->b == 2 && follow_player_unit != -1 && follow_attached) {
        register_ai_follow_watch(returned_actor, active_unit, follow_player_unit);
    }
    snprintf(g_result, sizeof(g_result),
             "ok ai_attach unit=%d actor=%d returned=%d template=%d tag=%d variant=%d team=%d marine_mode=%d search_targets=%d loadout_profile=%d random_weapon=%d weapon_path=%s weapon_removed=%d weapon_override=%d weapon_restored=%d live_squad_migration=%d squad=%d squad_members=%d transient_runtime_squad=%d fireteam_squad_link=%d fireteam_follow_ready=%d fireteam_membership=%d fireteam=%d members=%d fireteam_evicted_actor=%d actor_force_active_optional=%d actor_flags_before=%02X actor_flags_after=%02X damageproof_cleared=%d secondary_link_cleared=%d protected_special_units=%d ai_disregard_ready=%d template_destroy=%d scenario_mutation=0 actor_team_post_attach=1 no_actor_table_write=1 runtime_forced=%d actor_context_forced=%d ai_phase_forced=%d\n",
             active_unit, returned_actor, returned_actor, command->a,
             tag_datum, actor_variant, command->b,
             command->marine_mode, search_targets, command->e, random_weapon_tag,
             random_weapon_path[0] ? random_weapon_path : "default",
             weapon_removed, weapon_override, weapon_restored,
             squad_attached, live_squad_index, live_squad_members,
             runtime_squad_created,
             fireteam_squad_linked,
             follow_attached, fireteam_actor_member,
             fireteam_datum, fireteam_members,
             fireteam_evicted_actor,
             actor_force_active,
             (unsigned)actor_flags_before,
             (unsigned)actor_flags_after,
             damageproof_cleared,
             secondary_link_cleared,
             protected_special_units,
             special_units_disregarded,
             template_destroy_requested,
             runtime_flag_forced, actor_context_forced, ai_phase_forced);
}

/* Only three explicitly tracked escort units; no actor/world scans. */
static int ai19_catchup(int32_t player, const int32_t ids[3], const int alive[3]) {
    static int32_t owner=-1, tracked[3]={-1,-1,-1};
    static ULONGLONG last=0, far_since[3]={0,0,0};
    static float trail[3];
    float p[3],f[3],up[3],m[3];
    ULONGLONG now=GetTickCount64();
    uint8_t *unit=object_data(player);
    int i,moved=0,ready;
    if (!unit || !readable_range(unit+0x14,4) || *(int32_t *)(unit+0x14)!=-1) {
        owner=-1;last=0;return 0;
    }
    ((object_get_origin_fn)(g_blam+0x5A6780))(player,p);
    ((object_get_orientation_fn)(g_blam+0x5A6AE0))(player,f,up);
    for(i=0;i<3;i++) if(!isfinite(p[i]) || !isfinite(f[i]) || !isfinite(up[i])) {owner=-1;last=0;return 0;}
    ready=owner==player && last && now-last<=3500;
    if(!ready) for(i=0;i<3;i++){tracked[i]=-1;far_since[i]=0;}
    /* Do not recall during fast movement, drops, or a stale mission sample.
       Destination is a point recently occupied by the player, not a guessed
       point behind a wall. Engine teleport performs the actual relocation. */
    if(ready) {
        float x=p[0]-trail[0],y=p[1]-trail[1],z=p[2]-trail[2];
        ready=x*x+y*y+z*z<=16.0f && fabsf(z)<=0.4f;
    }
    for(i=0;i<3;i++) {
        float x,y,z;
        if(tracked[i]!=ids[i]) {tracked[i]=ids[i];far_since[i]=0;}
        unit=object_data(ids[i]);
        if(alive[i]!=1 || !unit || !readable_range(unit+0x14,4) || *(int32_t *)(unit+0x14)!=-1) {far_since[i]=0;continue;}
        ((object_get_origin_fn)(g_blam+0x5A6780))(ids[i],m);
        x=p[0]-m[0];y=p[1]-m[1];z=p[2]-m[2];
        if(!isfinite(x)||!isfinite(y)||!isfinite(z)||x*x+y*y+z*z<=625.0f) {far_since[i]=0;continue;}
        if(!far_since[i]) far_since[i]=now;
        if(ready && !moved && now-far_since[i]>=6000) {
            float destination[3]={trail[0],trail[1],trail[2]+0.15f};
            ((object_teleport_fn)(g_blam+0x351610))(ids[i],destination,f,up,1);
            far_since[i]=0;moved=i+1;
        }
    }
    memcpy(trail,p,sizeof(trail));owner=player;last=now;
    return moved;
}

/* Only three explicitly tracked escort units; no actor/world scans. */
static int ai20_johnson_catchup(int32_t player, const int32_t ids[3], const int alive[3]) {
    static int32_t owner=-1, tracked[3]={-1,-1,-1};
    static ULONGLONG last=0, far_since[3]={0,0,0};
    static float trail[3];
    float p[3],f[3],up[3],m[3];
    ULONGLONG now=GetTickCount64();
    uint8_t *unit=object_data(player);
    int i,moved=0,ready;
    if (!unit || !readable_range(unit+0x14,4) || *(int32_t *)(unit+0x14)!=-1) {
        owner=-1;last=0;return 0;
    }
    ((object_get_origin_fn)(g_blam+0x5A6780))(player,p);
    ((object_get_orientation_fn)(g_blam+0x5A6AE0))(player,f,up);
    for(i=0;i<3;i++) if(!isfinite(p[i]) || !isfinite(f[i]) || !isfinite(up[i])) {owner=-1;last=0;return 0;}
    ready=owner==player && last && now-last<=3500;
    if(!ready) for(i=0;i<3;i++){tracked[i]=-1;far_since[i]=0;}
    /* Do not recall during fast movement, drops, or a stale mission sample.
       Destination is a point recently occupied by the player, not a guessed
       point behind a wall. Engine teleport performs the actual relocation. */
    if(ready) {
        float x=p[0]-trail[0],y=p[1]-trail[1],z=p[2]-trail[2];
        ready=x*x+y*y+z*z<=16.0f && fabsf(z)<=0.4f;
    }
    for(i=0;i<3;i++) {
        float x,y,z;
        if(tracked[i]!=ids[i]) {tracked[i]=ids[i];far_since[i]=0;}
        unit=object_data(ids[i]);
        if(alive[i]!=1 || !unit || !readable_range(unit+0x14,4) || *(int32_t *)(unit+0x14)!=-1) {far_since[i]=0;continue;}
        ((object_get_origin_fn)(g_blam+0x5A6780))(ids[i],m);
        x=p[0]-m[0];y=p[1]-m[1];z=p[2]-m[2];
        if(!isfinite(x)||!isfinite(y)||!isfinite(z)||x*x+y*y+z*z<=625.0f) {far_since[i]=0;continue;}
        if(!far_since[i]) far_since[i]=now;
        if(ready && !moved && now-far_since[i]>=6000) {
            float destination[3]={trail[0],trail[1],trail[2]+0.15f};
            ((object_teleport_fn)(g_blam+0x351610))(ids[i],destination,f,up,1);
            far_since[i]=0;moved=i+1;
        }
    }
    memcpy(trail,p,sizeof(trail));owner=player;last=now;
    return moved;
}

void halosandbox_on_main_thread(void) {
    struct native_command command;
    LONG entered = InterlockedCompareExchange(
        &g_state, NATIVE_STATE_EXECUTING, NATIVE_STATE_QUEUED);
    if (entered != NATIVE_STATE_QUEUED) {
        if (entered == NATIVE_STATE_QUEUED_QUIESCED)
            finish_quiesced_owner(NATIVE_STATE_QUEUED_QUIESCED);
        return;
    }
    command = g_command;
    if (command.dispatch_epoch !=
        InterlockedCompareExchange(&g_dispatch_epoch, 0, 0)) {
        snprintf(g_result, sizeof(g_result),
                 "error stale_dispatch_epoch command=%ld current=%ld\n",
                 (long)command.dispatch_epoch,
                 (long)InterlockedCompareExchange(&g_dispatch_epoch, 0, 0));
        goto dispatch_complete;
    }
    if (command.kind==COMMAND_AI_SPAWN_AT) {
        ai1_trace("spawn validation");
        if (!ai1_spawn_allowed(&command)) {
            snprintf(g_result,sizeof(g_result),"error ai1_spawn_profile_or_player_invalid detail={%s}\n",ai10_spawn_denial);
            goto dispatch_complete;
        }
        if(!ai25_missing_human_alliance(command.c,ai10_spawn_denial,sizeof(ai10_spawn_denial))) {
            snprintf(g_result,sizeof(g_result),"error human_alliance_unavailable detail={%s}\n",ai10_spawn_denial);
            goto dispatch_complete;
        }
        ai1_trace("spawn validation passed");
    }
    if (command.kind == COMMAND_ESCORT_STATUS) {
        if (!is_live_player_unit(command.a) || ((unit_get_vitality_fn)(g_blam+0x5B7E60))(command.a)<=0.0f) snprintf(g_result,sizeof(g_result),"error escort_player_unavailable\n");
        else if(command.e==2) {
            const int32_t ids[3]={command.b,-1,-1};
            const int alive[3]={ai20_johnson_alive(command.b),0,0};
            int moved=alive[0]>=0?ai20_johnson_catchup(command.a,ids,alive):0;
            snprintf(g_result,sizeof(g_result),"ok escort s1=%d s2=1 s3=1 recalled=%d\n",alive[0],moved);
        } else {
            const int32_t ids[3]={command.b,command.c,command.d};
            const int alive[3]={ai15_marine_alive(command.b),ai15_marine_alive(command.c),ai15_marine_alive(command.d)};
            int moved=0;
            if(command.e==1 && alive[0]>=0 && alive[1]>=0 && alive[2]>=0) moved=ai19_catchup(command.a,ids,alive);
            snprintf(g_result,sizeof(g_result),"ok escort s1=%d s2=%d s3=%d recalled=%d\n",alive[0],alive[1],alive[2],moved);
        }
        goto dispatch_complete;
    }
    if (command.kind == COMMAND_OBSERVER) {
        if(!ai21_observer(command.a,command.b))snprintf(g_result,sizeof(g_result),"error observer_unavailable\n");
        else snprintf(g_result,sizeof(g_result),"ok observer enabled=%d player=%d\n",command.b,command.a);
        goto dispatch_complete;
    }
    if (command.kind == COMMAND_AI_CONTROL) {
        int before=-1;
        if (!ai6_control(command.a,command.b,&before)) {
            snprintf(g_result,sizeof(g_result),"error ai_control_unavailable\n");
        } else {
            snprintf(g_result,sizeof(g_result),"ok ai_control enabled=%d before=%d all_ai=1\n",command.b,before);
        }
        goto dispatch_complete;
    }
    if (command.kind == COMMAND_SPAWN_AT && !ai6_item_spawn_allowed(&command)) {
        snprintf(g_result,sizeof(g_result),"error item_spawn_profile_player_or_position_invalid\n");
        goto dispatch_complete;
    }
    if (command.kind == COMMAND_AI_PROBE) {
        execute_ai_probe();
    } else if (command.kind == COMMAND_AI_FOLLOW_WATCH) {
        execute_ai_follow_watch();
    } else if (command.kind == COMMAND_AI_PLACE_GROUP) {
        execute_ai_place_group(&command);
    } else if (command.kind == COMMAND_AI_SPAWN_AT) {
        int32_t created;
        int32_t follow_player_unit = command.marine_mode != 0 ? -1 : command.a;
        int32_t spawned_tag = command.b;
        int32_t spawned_actor_variant = command.d;
        float spawned_origin[3] = {command.x, command.y, command.z};
        if (!valid_transform(&command)) {
            snprintf(g_result, sizeof(g_result), "error ai_spawn_at transform invalide\n");
        } else {
    ai1_trace("before template spawn");
            created = spawn_at_transform(&command);
    ai1_trace("template spawn returned");
            if (created == -1) {
                snprintf(g_result, sizeof(g_result), "error ai_spawn_at creation biped refusee tag=%d\n", command.b);
            } else {
                /* Attacher l'acteur pendant le même tick empêche le moteur de
                 * supprimer le biped sans IA entre deux commandes Lua. */
                command.a = created;
                command.b = command.c;
                execute_ai_attach(&command, spawned_tag, spawned_actor_variant, spawned_origin,
                                  follow_player_unit);
            }
        }
    } else if (command.kind == COMMAND_AI_ATTACH) {
        execute_ai_attach(&command, -1, -1, NULL, -1);
    } else if (command.kind == COMMAND_HEALTH) {
        if (!is_live_player_unit(command.a)) {
            snprintf(g_result, sizeof(g_result),
                     "error health_not_live_player datum=%d\n", command.a);
        } else {
            unit_get_vitality_fn get_body = (unit_get_vitality_fn)(g_blam + 0x5B7E60);
            unit_get_vitality_fn get_shield = (unit_get_vitality_fn)(g_blam + 0x5B8230);
            float body_before = get_body(command.a);
            float shield_before = get_shield(command.a);
            ((unit_set_shield_vitality_fn)(g_blam + 0x5B8790))(command.a, command.y);
            ((unit_set_body_vitality_fn)(g_blam + 0x5B7F10))(command.a, command.x, 0);
            snprintf(g_result, sizeof(g_result),
                     "ok health unit=%d before=%.3f/%.3f requested=%.3f/%.3f after=%.3f/%.3f\n",
                     command.a, body_before, shield_before, command.x, command.y,
                     get_body(command.a), get_shield(command.a));
        }
    } else if (command.kind == COMMAND_DAMAGEPROOF) {
        if (!set_player_damageproof(command.a, command.b != 0)) {
            snprintf(g_result, sizeof(g_result),
                     "error damageproof_object_missing datum=%d\n", command.a);
        } else {
            snprintf(g_result, sizeof(g_result),
                     "ok damageproof datum=%d enabled=%d native_object_flag=7\n",
                     command.a, command.b != 0);
        }
    } else if (command.kind == COMMAND_REFILL_AMMO) {
        int weapons = 0, magazines = 0, grenades = 0;
        if (!is_live_player_unit(command.a) ||
            !refill_unit_ammo(command.a, 0,
                              &weapons, &magazines, &grenades)) {
            snprintf(g_result, sizeof(g_result),
                     "error refill_ammo_not_live_player datum=%d\n", command.a);
        } else {
            snprintf(g_result, sizeof(g_result),
                     "ok refill_ammo unit=%d weapons=%d magazines=%d grenades=%d native_setters=1\n",
                     command.a, weapons, magazines, grenades);
        }
    } else if (command.kind == COMMAND_PLAYER_POWERS) {
        int weapons = 0, magazines = 0, grenades = 0;
        int trait_changed = 0;
        int trait_snapshot = 0;
        int maintenance_ok = 1;
        /* proof=-1 signifie "ne pas toucher au drapeau". En mode munitions
         * seules, l'ancienne maintenance repassait inutilement toute la
         * propagation object_cannot_take_damage a OFF toutes les 1,5 s. */
        int live_player = is_live_player_unit(command.a);
        int proof_ok = live_player && (command.b < 0 ||
            set_player_damageproof(command.a, command.b != 0));
        /* ammo=-1 laisse le trait intact; 0 restaure sa valeur d'origine;
         * 1 active le vrai mode bottomless du player record. Une recharge
         * ponctuelle n'est faite qu'a l'activation ou si Halo a reinitialise
         * le trait, jamais dans la boucle de jeu normale. */
        int trait_ok = 1;
        int refill_ok = 1;
        if (command.c == 0) {
            /* Stoppe d'abord le garde et invalide tout tick reserve avant de
             * restaurer eventuellement le trait retail. */
            disable_ammo_maintenance();
        }
        trait_ok = live_player && (command.c < 0 || set_player_bottomless_trait(
            command.a, command.c != 0, &trait_changed, &trait_snapshot));
        if (command.c == 1) {
            /* Le test terrain montre que le trait peut etre ignore par une
             * session campagne deja initialisee. La recharge initiale et le
             * garde conditionnel constituent donc le chemin de secours, meme
             * si le player record n'est momentanement pas accessible. */
            refill_ok = live_player && refill_unit_ammo(
                command.a, 0, &weapons, &magazines, &grenades);
            if (refill_ok)
                maintenance_ok = enable_ammo_maintenance(command.a);
            else
                disable_ammo_maintenance();
        }
        if (!proof_ok || !trait_ok || !refill_ok || !maintenance_ok) {
            snprintf(g_result, sizeof(g_result),
                     "error player_powers unit_missing=%d proof_ok=%d trait_ok=%d refill_ok=%d maintenance_ok=%d\n",
                     command.a, proof_ok, trait_ok, refill_ok,
                     maintenance_ok);
        } else {
            snprintf(g_result, sizeof(g_result),
                     "ok player_powers unit=%d damageproof_mode=%d bottomless_mode=%d trait_supported=%d trait_changed=%d trait_snapshot=%d refill_once=%d maintenance_1hz=%d weapons=%d magazines=%d grenades=%d batched_main_thread=1\n",
                     command.a, command.b, command.c, trait_ok,
                     trait_changed, trait_snapshot, command.c == 1,
                     command.c == 1,
                     weapons, magazines, grenades);
        }
    } else if (command.kind == COMMAND_AMMO_MAINTENANCE) {
        int weapons = 0, magazines = 0, grenades = 0;
        LONG current_epoch = InterlockedCompareExchange(
            &g_ammo_maintenance_epoch, 0, 0);
        LONG current_unit = InterlockedCompareExchange(
            &g_ammo_maintenance_unit, -1, -1);
        int still_enabled = InterlockedCompareExchange(
            &g_ammo_maintenance_enabled, 0, 0) != 0;
        int refill_ok = 1;
        if (still_enabled && command.b == current_epoch &&
            command.a == current_unit && is_live_player_unit(command.a)) {
            refill_ok = refill_unit_ammo(
                command.a, 1, &weapons, &magazines, &grenades);
            if (!refill_ok) disable_ammo_maintenance();
        } else if (still_enabled && command.b == current_epoch &&
                   command.a == current_unit) {
            /* Mort/changement de biped : ne jamais laisser le garde suivre
             * un datum qui pourrait ensuite etre reutilise par une IA. */
            disable_ammo_maintenance();
        }
        /* Commande exclusivement interne : aucun resultat disque n'est
         * consomme. Eviter ici le formatage a chaque seconde. */
    } else if (command.kind == COMMAND_NOCLIP) {
        if (command.b != 0) {
            if (!noclip_enable_player(command.a)) {
                snprintf(g_result, sizeof(g_result),
                         "error noclip_enable_failed datum=%d seated_or_zone_state_unavailable=1\n",
                         command.a);
            } else {
                snprintf(g_result, sizeof(g_result),
                         "ok noclip datum=%d enabled=1 normal_player_controls=1 player_and_vehicle_support=1 damageproof=1 soft_ceiling_words=%u kill_volume_words=%u mission_boundaries_disabled=1 direct_game_thread_maintenance=1 injected_worker=0\n",
                         command.a, g_zone_limits.soft_word_count,
                         g_zone_limits.kill_word_count);
            }
        } else {
            int32_t restored_datum = g_noclip_datum;
            noclip_restore_player(command.c != 0);
            snprintf(g_result, sizeof(g_result),
                     "ok noclip datum=%d enabled=0 normal_player_controls=1 zone_limits_restored=1 damageproof=%d\n",
                     restored_datum, command.c != 0);
        }
    } else if (command.kind == COMMAND_NOCLIP_TICK) {
        noclip_tick();
    } else if (command.kind == COMMAND_NOCLIP_RECOVER) {
        int32_t recovered_root = -1;
        if (!noclip_recover_anchor(command.a, command.b != 0, &recovered_root)) {
            snprintf(g_result, sizeof(g_result),
                     "error noclip_recovery_unavailable datum=%d mission_changed_or_anchor_missing=1\n",
                     command.a);
        } else {
            snprintf(g_result, sizeof(g_result),
                     "ok noclip_recover datum=%d root=%d enabled=0 zone_limits_restored=1 emergency_anchor_only=1 damageproof=%d\n",
                     command.a, recovered_root, command.b != 0);
        }
    } else if (command.kind == COMMAND_PHASE_FORWARD) {
        uint8_t *object = object_data(command.a);
        float position[3], forward[3], up[3];
        uint32_t collision_flags;
        if (!is_live_player_unit(command.a) || !object ||
            !writable_range(object + 0x1B4, sizeof(uint32_t))) {
            snprintf(g_result, sizeof(g_result),
                     "error phase_not_live_player datum=%d\n", command.a);
        } else if (!isfinite(command.x) || command.x < 1.0f || command.x > 30.0f) {
            snprintf(g_result, sizeof(g_result), "error phase_distance_out_of_range\n");
        } else {
            ((object_get_origin_fn)(g_blam + 0x5A6780))(command.a, position);
            ((object_get_orientation_fn)(g_blam + 0x5A6AE0))(command.a, forward, up);
            if (!isfinite(position[0]) || !isfinite(position[1]) || !isfinite(position[2]) ||
                !isfinite(forward[0]) || !isfinite(forward[1]) || !isfinite(forward[2]) ||
                !isfinite(up[0]) || !isfinite(up[1]) || !isfinite(up[2])) {
                snprintf(g_result, sizeof(g_result), "error phase_transform_invalid\n");
            } else {
                position[0] += forward[0] * command.x + up[0] * 0.20f;
                position[1] += forward[1] * command.x + up[1] * 0.20f;
                position[2] += forward[2] * command.x + up[2] * 0.20f;

                /* Le bit 16 est celui que object_set_collideable modifie dans
                 * cette build. Il est retire uniquement pendant le teleport,
                 * puis l'etat exact d'origine est restaure avant le tick suivant. */
                collision_flags = *(uint32_t *)(object + 0x1B4);
                set_object_damageproof(command.a, 1);
                *(uint32_t *)(object + 0x1B4) = collision_flags & ~0x00010000u;
                MemoryBarrier();
                ((object_teleport_fn)(g_blam + 0x351610))(
                    command.a, position, forward, up, 1);
                if (writable_range(object + 0x1B4, sizeof(uint32_t)))
                    *(uint32_t *)(object + 0x1B4) = collision_flags;
                set_object_damageproof(command.a, command.b != 0);
                snprintf(g_result, sizeof(g_result),
                         "ok phase_forward datum=%d distance=%.3f position=%.6f,%.6f,%.6f collision_restored=1 damageproof=%d\n",
                         command.a, command.x, position[0], position[1], position[2], command.b != 0);
            }
        }
    } else if (command.kind == COMMAND_SPAWN) {
        int32_t created;
        _Alignas(16) uint8_t placement[0x330];
        float *position = (float *)(placement + 0x1C);
        float *forward = (float *)(placement + 0x28);
        float *up = (float *)(placement + 0x34);
        ((placement_init_fn)(g_blam + 0x5EE570))(placement, command.b, -1, NULL);
        ((object_get_origin_fn)(g_blam + 0x5A6780))(command.a, position);
        ((object_get_orientation_fn)(g_blam + 0x5A6AE0))(command.a, forward, up);
        position[0] += forward[0] * command.x + up[0] * command.y;
        position[1] += forward[1] * command.x + up[1] * command.y;
        position[2] += forward[2] * command.x + up[2] * command.y;
        created = ((object_new_fn)(g_blam + 0x5A0FC0))(placement);
        if (created == -1) snprintf(g_result, sizeof(g_result), "error spawn_failed\n");
        else snprintf(g_result, sizeof(g_result), "ok spawn datum=%d tag=%d\n", created, command.b);
    } else if (command.kind == COMMAND_SPAWN_AT) {
        int32_t created;
        if (!valid_transform(&command)) {
            snprintf(g_result, sizeof(g_result), "error invalid_transform\n");
        } else {
            created = spawn_at_transform(&command);
            if (created == -1) snprintf(g_result, sizeof(g_result), "error spawn_at_failed tag=%d\n", command.b);
            else snprintf(g_result, sizeof(g_result),
                          "ok spawn_at datum=%d tag=%d position=%.3f,%.3f,%.3f\n",
                          created, command.b, command.x, command.y, command.z);
        }
    } else if (command.kind == COMMAND_SPAWN_ENTER) {
        uint32_t seat_count = 0;
        int32_t created;
        int requested;
        if (!valid_transform(&command)) {
            snprintf(g_result, sizeof(g_result), "error invalid_transform\n");
        } else if (!unlock_vehicle_seats(command.b, &seat_count)) {
            snprintf(g_result, sizeof(g_result), "error vehicle_tag_not_resident_or_invalid tag=%d\n", command.b);
        } else if (command.c < 0 || (uint32_t)command.c >= seat_count) {
            snprintf(g_result, sizeof(g_result), "error seat_out_of_range seat=%d count=%u\n", command.c, seat_count);
        } else {
            created = spawn_at_transform(&command);
            if (created == -1) {
                snprintf(g_result, sizeof(g_result), "error spawn_enter_failed tag=%d\n", command.b);
            } else {
                requested = enter_vehicle_immediate(command.a, created, command.b, command.c);
                snprintf(g_result, sizeof(g_result), requested
                         ? "ok spawn_enter datum=%d tag=%d unit=%d seat=%d immediate=1\n"
                         : "error spawn_enter_immediate_rejected datum=%d tag=%d unit=%d seat=%d\n",
                         created, command.b, command.a, command.c);
            }
        }
    } else if (command.kind == COMMAND_SPAWN_ENTER_RELATIVE ||
               command.kind == COMMAND_SPAWN_RELATIVE) {
        const int enter_after_spawn =
            command.kind == COMMAND_SPAWN_ENTER_RELATIVE;
        uint32_t seat_count = 0;
        int32_t created;
        int requested;
        _Alignas(16) uint8_t placement[0x330];
        float *position = (float *)(placement + 0x1C);
        float *forward = (float *)(placement + 0x28);
        float *up = (float *)(placement + 0x34);
        if (!object_data(command.a)) {
            snprintf(g_result, sizeof(g_result),
                     "error spawn_enter_relative_player_missing unit=%d\n", command.a);
        } else if (!isfinite(command.x) || !isfinite(command.y) ||
                   command.x < -10.0f || command.x > 20.0f ||
                   command.y < -5.0f || command.y > 20.0f) {
            snprintf(g_result, sizeof(g_result),
                     "error spawn_enter_relative_offset_invalid forward=%.3f up=%.3f\n",
                     command.x, command.y);
        } else if (!unlock_vehicle_seats(command.b, &seat_count)) {
            snprintf(g_result, sizeof(g_result),
                     "error vehicle_tag_not_resident_or_invalid tag=%d\n", command.b);
        } else if (enter_after_spawn &&
                   (command.c < 0 || (uint32_t)command.c >= seat_count)) {
            snprintf(g_result, sizeof(g_result),
                     "error seat_out_of_range seat=%d count=%u\n", command.c, seat_count);
        } else {
            /* Utilise exclusivement l'origine et l'orientation Blam de l'unite.
             * Les coordonnees du proxy Unreal ne sont pas toujours dans le meme
             * repere sur les BSP interieurs (notamment La Bibliotheque). */
            memset(placement, 0, sizeof(placement));
            ((placement_init_fn)(g_blam + 0x5EE570))(placement, command.b, -1, NULL);
            ((object_get_origin_fn)(g_blam + 0x5A6780))(command.a, position);
            ((object_get_orientation_fn)(g_blam + 0x5A6AE0))(command.a, forward, up);
            position[0] += forward[0] * command.x + up[0] * command.y;
            position[1] += forward[1] * command.x + up[1] * command.y;
            position[2] += forward[2] * command.x + up[2] * command.y;
            if (!isfinite(position[0]) || !isfinite(position[1]) || !isfinite(position[2])) {
                snprintf(g_result, sizeof(g_result),
                         "error spawn_enter_relative_origin_invalid unit=%d\n", command.a);
            } else {
                created = ((object_new_fn)(g_blam + 0x5A0FC0))(placement);
                if (created == -1) {
                    snprintf(g_result, sizeof(g_result),
                             "error spawn_enter_relative_failed tag=%d\n", command.b);
                } else if (!enter_after_spawn) {
                    /* Les modules controles par le tireur parent doivent etre
                     * attaches avant l'entree dans le siege. Cette variante
                     * conserve le placement Blam exact sans initialiser le
                     * cycle de siege trop tot. */
                    snprintf(g_result, sizeof(g_result),
                             "ok spawn_relative datum=%d tag=%d unit=%d position=%.3f,%.3f,%.3f immediate=0 native_player_basis=1\n",
                             created, command.b, command.a,
                             position[0], position[1], position[2]);
                } else {
                    requested = enter_vehicle_immediate(command.a, created, command.b, command.c);
                    if (!requested) {
                        /* Ne laisse jamais un vehicule orphelin si l'entree
                         * atomique a ete refusee par le moteur. */
                        ((object_delete_fn)(g_blam + 0x350960))(created);
                        snprintf(g_result, sizeof(g_result),
                                 "error spawn_enter_relative_entry_rejected tag=%d unit=%d seat=%d cleanup=1\n",
                                 command.b, command.a, command.c);
                    } else {
                        snprintf(g_result, sizeof(g_result),
                                 "ok spawn_enter_relative datum=%d tag=%d unit=%d seat=%d position=%.3f,%.3f,%.3f immediate=1 native_player_basis=1\n",
                                 created, command.b, command.a, command.c,
                                 position[0], position[1], position[2]);
                    }
                }
            }
        }
    } else if (command.kind == COMMAND_ATTACHSPAWN) {
        struct native_command placement_command;
        float position[3], forward[3], up[3];
        int32_t created = -1;
        int32_t weapon = -1;
        uint8_t *child;
        if (!object_data(command.a) || command.b == -1) {
            snprintf(g_result, sizeof(g_result),
                     "error attachspawn parent_or_tag_missing parent=%d tag=%d\n",
                     command.a, command.b);
        } else {
            ((object_get_origin_fn)(g_blam + 0x5A6780))(command.a, position);
            ((object_get_orientation_fn)(g_blam + 0x5A6AE0))(command.a, forward, up);
            memset(&placement_command, 0, sizeof(placement_command));
            placement_command.b = command.b;
            placement_command.x = position[0];
            placement_command.y = position[1];
            placement_command.z = position[2];
            placement_command.fx = forward[0];
            placement_command.fy = forward[1];
            placement_command.fz = forward[2];
            placement_command.ux = up[0];
            placement_command.uy = up[1];
            placement_command.uz = up[2];
            created = spawn_attached_at_transform(
                &placement_command, command.a,
                (uint32_t)command.c, (uint32_t)command.d);
            child = object_data(created);
            if (child && command.e != 0)
                ((object_hide_fn)(g_blam + 0x5A5FD0))(created, 1);
            if (child && readable_range(child + 0x344, sizeof(int32_t)))
                weapon = *(int32_t *)(child + 0x344);
            if (!child || !readable_range(child + 0x14, sizeof(int32_t)) ||
                *(int32_t *)(child + 0x14) != command.a) {
                if (created != -1 && child)
                    ((object_delete_fn)(g_blam + 0x350960))(created);
                snprintf(g_result, sizeof(g_result),
                         "error attachspawn failed parent=%d tag=%d marker=%08X child=%d\n",
                         command.a, command.b, (uint32_t)command.c, created);
            } else {
                snprintf(g_result, sizeof(g_result),
                         "ok attachspawn parent=%d tag=%d parent_marker=%08X child_marker=%08X child=%d weapon=%d physical=1 hidden_model=%d atomic_initial_attachment=1\n",
                         command.a, command.b, (uint32_t)command.c,
                         (uint32_t)command.d, created, weapon, command.e != 0);
            }
        }
    } else if (command.kind == COMMAND_WEAPON_FIRE) {
        if (command.b < 0 || command.b > 1 ||
            !set_weapon_primary_firing(command.a, command.b)) {
            snprintf(g_result, sizeof(g_result),
                     "error weapon_fire datum=%d enabled=%d\n", command.a, command.b);
        } else {
            snprintf(g_result, sizeof(g_result),
                     "ok weapon_fire datum=%d enabled=%d object_dirty=1\n",
                     command.a, command.b);
        }
    } else if (command.kind == COMMAND_OBJECT_DELETE) {
        if (!object_data(command.a)) {
            snprintf(g_result, sizeof(g_result),
                     "error object_delete missing=%d\n", command.a);
        } else {
            ((object_delete_fn)(g_blam + 0x350960))(command.a);
            snprintf(g_result, sizeof(g_result),
                     "ok object_delete datum=%d\n", command.a);
        }
    } else if (command.kind == COMMAND_PELICAN_PATCH) {
        uint8_t *pelican_definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
            0x69686576u, command.a);
        uint8_t *seats = NULL;
        uint32_t seat_count = 0;
        uint32_t before_flags = 0;
        uint32_t after_flags = 0;
        DWORD old_protect = 0;
        int patched = 0;
        int presentation_patched = 0;
        int pitch_patched = 0;
        if (pelican_definition)
            seats = resolve_seats(command.a, &seat_count);
        if (!pelican_definition || !seats || seat_count == 0u) {
            snprintf(g_result, sizeof(g_result),
                     "error pelican_patch invalid pelican_tag=%d seats=%u\n",
                     command.a, seat_count);
        } else {
            unlock_vehicle_seats(command.a, &seat_count);
            seats = resolve_seats(command.a, &seat_count);
            if (seats && seat_count > 0u &&
                VirtualProtect(seats, 328u, PAGE_READWRITE, &old_protect)) {
                before_flags = *(uint32_t *)seats;
                /* Schema retail unit_seat_block : bit 2=driver, bit 3=capacite
                 * de commander l'arme, bit 4=camera 3e personne, bit 5=armes
                 * PORTEES. Ces capacites restent sur l'unique occupant du
                 * siege pilote : elles ne creent aucun second tireur. Le bit 5
                 * est retire pour que la detente commande le canon du Pelican,
                 * jamais le fusil du Spartan. */
                after_flags = (before_flags | 0x0000001Cu) & ~0x00000020u;
                *(uint32_t *)seats = after_flags;
                VirtualProtect(seats, 328u, old_protect, &old_protect);
                patched = 1;
            }
            pitch_patched = pelican_patch_pilot_pitch(command.a);
            presentation_patched =
                pelican_patch_original_chin_presentation();
            if (!patched) {
                snprintf(g_result, sizeof(g_result),
                         "error pelican_patch seat_update_failed tag=%d\n",
                         command.a);
            } else if (!pitch_patched) {
                snprintf(g_result, sizeof(g_result),
                         "error pelican_patch pilot_pitch_failed tag=%d code=%d\n",
                         command.a, g_pelican_pitch_patch_error);
            } else {
                snprintf(g_result, sizeof(g_result),
                         "ok pelican_patch pelican_tag=%d seat0_before=%08X seat0_after=%08X cross_tag_weapon_copy=0 physical_chin_module=1 character_weapon=0 original_pelican_handling=1 spirit_handling_patch=0 cross_vehicle_physics_reference=0 pelican_boost_modules=0 pilot_pitch_patch=%d pilot_pitch_error=%d pilot_pitch_min_deg=-89 pilot_pitch_max_deg=89 pitch_patch_count=%u presentation_patch=%d presentation_error=%d presentation_weapon_tag=%d presentation_original=%d presentation_effect=%d presentation_patch_count=%u\n",
                         command.a, before_flags, after_flags,
                         pitch_patched,
                         g_pelican_pitch_patch_error,
                         g_pelican_pitch_patch_count,
                         presentation_patched,
                         g_pelican_presentation_patch_error,
                         g_pelican_presentation_weapon_tag,
                         g_pelican_presentation_original_effect,
                         g_pelican_presentation_effect,
                         g_pelican_presentation_patch_count);
            }
        }
    } else if (command.kind == COMMAND_PELICAN_BOOST) {
        uint8_t *definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
            0x69686576u, command.a);
        uint32_t before = definition && readable_range(definition, 1604u)
            ? *(uint32_t *)(definition + PELICAN_UNIT_DEFINITION_FLAGS_OFFSET) : 0u;
        int boost_patched = pelican_patch_native_boost(command.a);
        uint32_t after = definition && readable_range(definition, 1604u)
            ? *(uint32_t *)(definition + PELICAN_UNIT_DEFINITION_FLAGS_OFFSET) : 0u;
        if (!boost_patched) {
            snprintf(g_result, sizeof(g_result),
                     "error pelican_boost tag=%d code=%d flags_before=%08X flags_after=%08X\n",
                     command.a, g_pelican_boost_patch_error, before, after);
        } else {
            snprintf(g_result, sizeof(g_result),
                     "ok pelican_boost tag=%d flags_before=%08X flags_after=%08X unit_has_boost_bit=%08X original_pelican_boost_curve=1 peak_power=3.0 banshee_tag_loaded=0 banshee_data_copy=0 human_plane_preserved=1 lua_tick=0 velocity_write=0 patch_count=%u\n",
                     command.a, before, after, UNIT_HAS_BOOST_FLAG,
                     g_pelican_boost_patch_count);
        }
    } else if (command.kind == COMMAND_PELICAN_ARM_BUILTIN) {
        uint8_t *pilot = object_data(command.a);
        uint8_t *vehicle = object_data(command.b);
        int32_t pilot_parent = -1;
        int32_t weapon_datum = first_unit_weapon(command.b);
        int32_t weapon_tag = -1;
        int weapon_slot = -1;
        uint16_t current_before = 0xFFFFu;
        uint16_t desired_before = 0xFFFFu;
        uint32_t unit_flags_before = 0;
        uint32_t fire_interval_ms = 0;
        uint8_t shot_serial = 0;
        int32_t driver_object = -1;
        int32_t gunner_object = -1;
        uint8_t *weapon = object_data(weapon_datum);
        int presentation_patched = 0;
        int impact_patched = 0;
        int tracer_patched = 0;
        if (pilot && readable_range(pilot + 0x14, sizeof(int32_t)))
            pilot_parent = *(int32_t *)(pilot + 0x14);
        if (weapon && readable_range(weapon, sizeof(int32_t)))
            weapon_tag = *(int32_t *)weapon;
        if (!pilot || !vehicle || pilot_parent != command.b ||
            weapon_datum == -1 || !weapon) {
            snprintf(g_result, sizeof(g_result),
                     "error pelican_arm_builtin pilot=%d parent=%d vehicle=%d weapon=%d\n",
                     command.a, pilot_parent, command.b, weapon_datum);
        } else if (!select_unit_vehicle_weapon(
                       command.b, weapon_datum, &weapon_slot,
                       &current_before, &desired_before, &unit_flags_before)) {
            snprintf(g_result, sizeof(g_result),
                     "error pelican_arm_builtin weapon_selection_failed pilot=%d vehicle=%d weapon=%d\n",
                     command.a, command.b, weapon_datum);
        } else if (!pelican_primary_barrel_info(
                       weapon_datum, -1, &fire_interval_ms,
                       &shot_serial)) {
            snprintf(g_result, sizeof(g_result),
                     "error pelican_arm_builtin invalid_primary_barrel pilot=%d vehicle=%d weapon=%d\n",
                     command.a, command.b, weapon_datum);
        } else {
            if (readable_range(vehicle + UNIT_DRIVER_OBJECT_OFFSET, sizeof(int32_t)))
                driver_object = *(int32_t *)(vehicle + UNIT_DRIVER_OBJECT_OFFSET);
            if (readable_range(vehicle + UNIT_GUNNER_OBJECT_OFFSET, sizeof(int32_t)))
                gunner_object = *(int32_t *)(vehicle + UNIT_GUNNER_OBJECT_OFFSET);
            pelican_release_relay_main_thread(0u);
            set_weapon_primary_firing(weapon_datum, 0);
            clear_unit_vehicle_primary_control(command.b);
            if (!pelican_prepare_direct_barrel(weapon_datum)) {
                snprintf(g_result, sizeof(g_result),
                         "error pelican_arm_builtin barrel_not_idle weapon=%d\n",
                         weapon_datum);
            } else {
            presentation_patched =
                pelican_patch_human_presentation_for_weapon(weapon_datum);
            if (!presentation_patched) {
                snprintf(g_result, sizeof(g_result),
                         "error pelican_arm_builtin presentation_patch_failed vehicle=%d weapon=%d code=%d\n",
                         command.b, weapon_datum,
                         g_pelican_presentation_patch_error);
                goto dispatch_complete;
            }
            impact_patched = 0;
            g_pelican_aux_impact_projectile_tag = -1;
            tracer_patched = pelican_patch_tracer_for_weapon(weapon_datum);
            if (!tracer_patched) {
                pelican_rollback_failed_tracer_arm_patch();
                snprintf(g_result, sizeof(g_result),
                         "error pelican_arm_builtin tracer_patch_failed vehicle=%d weapon=%d code=%d active=%d\n",
                         command.b, weapon_datum,
                         g_pelican_tracer_patch_error,
                         g_pelican_tracer_patch_active);
                goto dispatch_complete;
            }
            g_pelican_local_pilot = command.a;
            g_pelican_vehicle = command.b;
            g_pelican_chin_module = -1;
            g_pelican_chin_weapon = weapon_datum;
            pelican_direct_gate_clear(weapon_datum);
            g_pelican_completed_shots = 0;
            g_pelican_last_shot_serial = shot_serial;
            InterlockedExchange(&g_pelican_fire_interval_ms,
                                (LONG)fire_interval_ms);
            InterlockedExchange(&g_pelican_requested_primary, 0);
            InterlockedExchange(&g_pelican_applied_primary, 0);
            InterlockedExchange(&g_pelican_driver_present, 1);
            InterlockedExchange(&g_pelican_local_input_hold_ticks, 0);
            InterlockedExchange(&g_pelican_pilot_absence_ticks, 0);
            InterlockedExchange(&g_pelican_last_driver_datum, command.a);
            InterlockedExchange(&g_pelican_last_target_ok, 0);
            InterlockedExchange(&g_pelican_last_aim_ok, 0);
            MemoryBarrier();
            InterlockedExchange(&g_pelican_last_release_reason, 0);
            pelican_reset_validation_diagnostics();
            InterlockedExchange(&g_pelican_armed, 1);
            start_pelican_worker();
            snprintf(g_result, sizeof(g_result),
                     "ok pelican_arm_builtin pilot=%d vehicle=%d weapon=%d weapon_tag=%d slot=%d current_before=%u channel1_before=%u current_after=%u channel1_after=%u unit_flags_before=%08X unit_flags_after=%08X driver=%d gunner=%d primary=pilot_mouse1_or_right_trigger interval_ms=%u engine_vehicle_weapon_pipeline=1 channel=0 control_bit=32 channel1=255 presentation_patch=%d presentation_error=%d presentation_effect=%d impact_patch=%d impact_error=%d impact_projectile=%d impact_effect=%d impact_slots=%02X impact_material_radius=%.4f tracer_patch=%d tracer_error=%d tracer_original=%u tracer_current=0\n",
                     command.a, command.b, weapon_datum, weapon_tag, weapon_slot,
                     (unsigned)current_before, (unsigned)desired_before,
                     (unsigned)*(uint8_t *)(vehicle + UNIT_CURRENT_WEAPON_INDEX_OFFSET),
                     (unsigned)*(uint8_t *)(vehicle + UNIT_DESIRED_WEAPON_INDEX_OFFSET),
                     unit_flags_before,
                     *(uint32_t *)(vehicle + UNIT_FLAGS_OFFSET),
                     driver_object, gunner_object, fire_interval_ms,
                     presentation_patched,
                     g_pelican_presentation_patch_error,
                     g_pelican_presentation_effect, impact_patched,
                     g_pelican_impact_patch_error,
                     g_pelican_impact_projectile_tag,
                     g_pelican_impact_effect,
                     (unsigned)g_pelican_impact_original_valid_mask,
                     (double)g_pelican_impact_material_radius,
                     tracer_patched,
                     g_pelican_tracer_patch_error,
                     (unsigned)g_pelican_tracer_original_rounds);
            }
        }
    } else if (command.kind == COMMAND_PELICAN_ARM) {
        uint8_t *pilot = object_data(command.a);
        uint8_t *vehicle = object_data(command.b);
        uint8_t *module = object_data(command.c);
        uint8_t *weapon = object_data(command.d);
        int32_t pilot_parent = -1;
        int32_t module_parent = -1;
        int weapon_slot = -1;
        uint16_t current_before = 0xFFFFu;
        uint16_t desired_before = 0xFFFFu;
        uint32_t unit_flags_before = 0;
        uint32_t fire_interval_ms = 0;
        uint8_t shot_serial = 0;
        int presentation_patched = 0;
        int impact_patched = 0;
        int tracer_patched = 0;
        int audio_proxy_ready = 0;
        int32_t audio_module = -1;
        int32_t parent_driver = -1;
        int32_t parent_gunner = -1;
        if (pilot && readable_range(pilot + 0x14, sizeof(int32_t)))
            pilot_parent = *(int32_t *)(pilot + 0x14);
        if (module && readable_range(module + 0x14, sizeof(int32_t)))
            module_parent = *(int32_t *)(module + 0x14);
        if (!pilot || !vehicle || !module || !weapon ||
            pilot_parent != command.b || module_parent != command.b) {
            snprintf(g_result, sizeof(g_result),
                     "error pelican_arm invalid pilot=%d parent=%d vehicle=%d module=%d module_parent=%d weapon=%d\n",
                     command.a, pilot_parent, command.b, command.c,
                     module_parent, command.d);
        } else if (!select_unit_vehicle_weapon(
                       command.c, command.d, &weapon_slot,
                       &current_before, &desired_before, &unit_flags_before)) {
            snprintf(g_result, sizeof(g_result),
                     "error pelican_arm module_weapon_selection_failed pilot=%d vehicle=%d module=%d weapon=%d\n",
                     command.a, command.b, command.c, command.d);
        } else if (!pelican_primary_barrel_info(
                       command.d, -1, &fire_interval_ms,
                       &shot_serial)) {
            snprintf(g_result, sizeof(g_result),
                     "error pelican_arm invalid_primary_barrel pilot=%d vehicle=%d module=%d weapon=%d\n",
                     command.a, command.b, command.c, command.d);
        } else {
            pelican_release_relay_main_thread(0u);
            set_weapon_primary_firing(command.d, 0);
            clear_unit_vehicle_primary_control(command.c);
            if (!pelican_prepare_direct_barrel(command.d)) {
                snprintf(g_result, sizeof(g_result),
                         "error pelican_arm barrel_not_idle module=%d weapon=%d\n",
                         command.c, command.d);
            } else {
            if (!bind_pelican_weapon_damage_owner(
                    command.a, command.c, command.d)) {
                snprintf(g_result, sizeof(g_result),
                         "error pelican_arm damage_owner_bind_failed pilot=%d module=%d weapon=%d\n",
                         command.a, command.c, command.d);
                goto dispatch_complete;
            }
            if (command.e != -1 &&
                prepare_pelican_audio_proxy(command.e, &audio_module)) {
                set_weapon_primary_firing(command.e, 0);
                clear_unit_vehicle_primary_control(audio_module);
                audio_proxy_ready = pelican_prepare_direct_barrel(command.e);
                if (!audio_proxy_ready) audio_module = -1;
            }
            presentation_patched =
                pelican_patch_human_presentation_for_weapon(command.d);
            if (!presentation_patched) {
                snprintf(g_result, sizeof(g_result),
                         "error pelican_arm presentation_patch_failed module=%d weapon=%d code=%d\n",
                         command.c, command.d,
                         g_pelican_presentation_patch_error);
                goto dispatch_complete;
            }
            /* Les effets sol/objet sont portes par l'override IoStore, mais
             * cette tourelle attachee n'obtient aucun ProjectileDatum du
             * moteur meme quand son serial avance. Resoudre ici le projectile
             * authored exact; un objet Blam reel sera cree par front de tir. */
            /* Le proxy Unreal peut etre attache avant que son objet Blam ne
             * rende son tag projectile resolvable. L'impact est donc lie ici
             * si possible, puis retente atomiquement au premier tir. Il ne
             * doit jamais empecher l'arme principale de s'armer. */
            impact_patched = audio_proxy_ready &&
                pelican_prepare_aux_impact_projectile(command.e);
            tracer_patched = pelican_patch_tracer_for_weapon(command.d);
            if (!tracer_patched) {
                pelican_rollback_failed_tracer_arm_patch();
                snprintf(g_result, sizeof(g_result),
                         "error pelican_arm tracer_patch_failed module=%d weapon=%d code=%d active=%d\n",
                         command.c, command.d,
                         g_pelican_tracer_patch_error,
                         g_pelican_tracer_patch_active);
                goto dispatch_complete;
            }
            /* Le module anime est superpose sans offset aux marqueurs retail
             * 11C1/15F5. Il doit rester visible, ainsi que son arme, car eux
             * seuls possedent les noeuds yaw/pitch et la bouche animee. Ne
             * jamais recopier la visee dans le Pelican principal : ses champs
             * de controle pilotent aussi la montee et le tangage. */
            ((object_hide_fn)(g_blam + 0x5A5FD0))(command.c, 0);
            ((object_hide_fn)(g_blam + 0x5A5FD0))(command.d, 0);
            /* Les deux mini-IoStore 1.5.77 declarent le siege 0 du Pelican
             * driver+gunner ET ajoutent au module retail le flag manquant
             * "controlled by parent gunner". Le module conserve aussi son
             * flag retail "parent's primary weapon". Halo peut alors propager
             * nativement le controle joueur, l'aim-assist et la convergence
             * camera->bouche. Toute copie manuelle de vecteur ou de detente
             * ici recreerait precisement la parallaxe proche. */
            if (readable_range(vehicle + UNIT_DRIVER_OBJECT_OFFSET,
                               sizeof(int32_t)))
                parent_driver = *(int32_t *)(
                    vehicle + UNIT_DRIVER_OBJECT_OFFSET);
            if (readable_range(vehicle + UNIT_GUNNER_OBJECT_OFFSET,
                               sizeof(int32_t)))
                parent_gunner = *(int32_t *)(
                    vehicle + UNIT_GUNNER_OBJECT_OFFSET);
            g_pelican_local_pilot = command.a;
            g_pelican_vehicle = command.b;
            g_pelican_chin_module = command.c;
            g_pelican_chin_weapon = command.d;
            g_pelican_audio_module = audio_proxy_ready ? audio_module : -1;
            g_pelican_audio_weapon = audio_proxy_ready ? command.e : -1;
            pelican_reset_validation_diagnostics();
            InterlockedExchange(&g_pelican_audio_signal_state, -1);
            if (!pelican_publish_audio_signal(0))
                ++g_pelican_audio_fire_failures;
            pelican_direct_gate_clear(command.d);
            g_pelican_completed_shots = 0;
            g_pelican_last_shot_serial = shot_serial;
            InterlockedExchange(&g_pelican_fire_interval_ms,
                                (LONG)fire_interval_ms);
            InterlockedExchange(&g_pelican_requested_primary, 0);
            InterlockedExchange(&g_pelican_applied_primary, 0);
            InterlockedExchange(&g_pelican_driver_present, 1);
            InterlockedExchange(&g_pelican_local_input_hold_ticks, 0);
            InterlockedExchange(&g_pelican_pilot_absence_ticks, 0);
            InterlockedExchange(&g_pelican_last_driver_datum, command.a);
            InterlockedExchange(&g_pelican_last_target_ok, 0);
            InterlockedExchange(&g_pelican_last_aim_ok, 0);
            MemoryBarrier();
            InterlockedExchange(&g_pelican_last_release_reason, 0);
            InterlockedExchange(&g_pelican_armed, 1);
            start_pelican_worker();
            snprintf(g_result, sizeof(g_result),
                     "ok pelican_arm pilot=%d vehicle=%d visible_module=%d chin_weapon=%d slot=%d current_before=%u channel1_before=%u current_after=%u channel1_after=%u unit_flags_before=%08X unit_flags_after=%08X parent_driver=%d parent_gunner=%d primary=engine_control_1549 interval_ms=%u engine_control_pipeline=1549_module_driver_relay driver_seat_flags=0000011C module_tag_unit_flags=06042000 module_flags=controlled_by_parent_gunner_and_parents_primary_weapon spawned_turret_character=none weapon_tag_type=scarab_gun module_aim_copy=legacy_vectors_1E0_1EC_204_210 module_trigger_copy=vehicle_primary_bit32 manual_weapon_fire=0 physical_markers=11C1_15F5 original_visual_turret=1 animated_overlay=1 manual_attachment_offset=0 hidden_weapon_model=0 camera_convergence=original_reticle_target_2048m camera_raycast_hz=0 observer_update_hz=30 main_vehicle_aim_write=0 flight_controls_untouched=1 damage_owner=pilot_via_native_vehicle_cache owner_kind=%ld owner_cache=%ld owner_verified=%ld weapon_owner=module audio_proxy_ready=%d audio_module=%d audio_weapon=%d audio_kind=UE_WarthogChaingun_x4 character_weapon=disabled_by_seat presentation_patch=%d presentation_error=%d presentation_effect=%d impact_pipeline=authored_projectile_object_new_plus_local_iostore_detonation impact_ready=%d impact_projectile=%d impact_definition_runtime_patch=0 impact_floor_object=1 aux_projectile_per_shot=1 tracer_patch=%d tracer_error=%d tracer_original=%u tracer_current=0 reentry=1549_module_driver_relay\n",
                     command.a, command.b, command.c, command.d, weapon_slot,
                     (unsigned)current_before, (unsigned)desired_before,
                     (unsigned)*(uint8_t *)(module + UNIT_CURRENT_WEAPON_INDEX_OFFSET),
                     (unsigned)*(uint8_t *)(module + UNIT_DESIRED_WEAPON_INDEX_OFFSET),
                     unit_flags_before, *(uint32_t *)(module + UNIT_FLAGS_OFFSET),
                     parent_driver, parent_gunner,
                     fire_interval_ms,
                     InterlockedCompareExchange(
                         &g_pelican_damage_owner_kind, 0, 0),
                     InterlockedCompareExchange(
                         &g_pelican_damage_owner_cache_active, 0, 0),
                     InterlockedCompareExchange(
                         &g_pelican_damage_owner_verified, 0, 0),
                     audio_proxy_ready,
                     g_pelican_audio_module, g_pelican_audio_weapon,
                     presentation_patched,
                     g_pelican_presentation_patch_error,
                     g_pelican_presentation_effect, impact_patched,
                     g_pelican_aux_impact_projectile_tag,
                     tracer_patched,
                     g_pelican_tracer_patch_error,
                     (unsigned)g_pelican_tracer_original_rounds);
            }
        }
    }
#ifdef HALOSANDBOX_RUNTIME_TEST
    else if (command.kind == COMMAND_PELICAN_RUNTIME_TEST_ARM) {
        pelican_runtime_test_arm_main_thread();
    }
#endif
    else if (command.kind == COMMAND_PELICAN_DISARM) {
        pelican_release_relay_main_thread(PELICAN_RELEASE_EXPLICIT);
        snprintf(g_result, sizeof(g_result), "ok pelican_disarm firing=0\n");
    } else if (command.kind == COMMAND_PELICAN_INPUT_TICK) {
        int local_input_requested = InterlockedCompareExchange(
            &g_pelican_requested_primary, 0, 0) != 0;
        int builtin_weapon = g_pelican_chin_module == -1;
        uint8_t *vehicle = object_data(g_pelican_vehicle);
        uint8_t *module = builtin_weapon ? NULL : object_data(g_pelican_chin_module);
        uint8_t *weapon = object_data(g_pelican_chin_weapon);
        int32_t last_driver_datum = -1;
        int32_t driver_datum = -1;
        uint8_t *pilot = NULL;
        int32_t pilot_parent = -1;
        int32_t module_parent = -1;
        int32_t weapon_parent = -1;
        int32_t control_target = builtin_weapon
            ? g_pelican_vehicle : g_pelican_chin_module;
        int pilot_stale_ticks = 0;
        uint32_t authored_interval = 0;
        uint8_t shot_serial = 0;
        uint32_t release_reason = 0u;
        int inventory_observed = 0;
        int inventory_valid = 0;
        int association_valid = 0;
        int barrel_valid = 0;
        int target_ok = 0;
        int aim_ok = 0;
        if (vehicle && readable_range(
                vehicle + UNIT_DRIVER_OBJECT_OFFSET, sizeof(int32_t)))
            driver_datum = *(int32_t *)(vehicle + UNIT_DRIVER_OBJECT_OFFSET);
        if (driver_datum != -1)
            InterlockedExchange(&g_pelican_last_driver_datum, driver_datum);
        last_driver_datum =
            InterlockedCompareExchange(&g_pelican_last_driver_datum, 0, 0);
        pilot = object_data(driver_datum);
        if (pilot && readable_range(pilot + 0x14, sizeof(int32_t)))
            pilot_parent = *(int32_t *)(pilot + 0x14);
        if (!builtin_weapon && module && readable_range(module + 0x14, sizeof(int32_t)))
            module_parent = *(int32_t *)(module + 0x14);
        if (weapon && readable_range(weapon + 0x14, sizeof(int32_t)))
            weapon_parent = *(int32_t *)(weapon + 0x14);
        if (vehicle && weapon &&
            (builtin_weapon || (module && module_parent == g_pelican_vehicle))) {
            inventory_observed = unit_has_weapon_datum(
                control_target, g_pelican_chin_weapon);
            /* Retail retrouve le proprietaire d'une arme via son owner/slot
             * d'inventaire (+0x344); object+0x14 peut redevenir -1 au tick
             * suivant sans detacher l'arme. Exiger l'un OU l'autre conserve
             * une association exacte au meme module, puis les gardes
             * tag/barrel ci-dessous valident encore le WeaponDatum. */
            association_valid = inventory_observed ||
                weapon_parent == control_target;
            inventory_valid = association_valid;
            if (!builtin_weapon && !inventory_observed)
                InterlockedExchange(
                    &g_pelican_module_inventory_missing_observed, 1);
            barrel_valid = pelican_primary_barrel_info(
                g_pelican_chin_weapon, -1,
                &authored_interval, &shot_serial);
        }
        if (!vehicle) release_reason |= PELICAN_RELEASE_VEHICLE_MISSING;
        if (!weapon) release_reason |= PELICAN_RELEASE_WEAPON_MISSING;
        if (!builtin_weapon && !module)
            release_reason |= PELICAN_RELEASE_MODULE_MISSING;
        if (!builtin_weapon && module &&
            module_parent != g_pelican_vehicle)
            release_reason |= PELICAN_RELEASE_MODULE_PARENT;
        if (vehicle && weapon &&
            (builtin_weapon || (module && module_parent == g_pelican_vehicle))) {
            if (!inventory_valid)
                release_reason |= PELICAN_RELEASE_INVENTORY;
            if (!barrel_valid) {
                uint32_t detail = pelican_primary_barrel_failure_detail(
                    g_pelican_chin_weapon, -1,
                    &g_pelican_last_barrel_actual_parent,
                    &g_pelican_last_barrel_weapon_tag,
                    &g_pelican_last_barrel_count,
                    &g_pelican_last_barrel_min_bits,
                    &g_pelican_last_barrel_max_bits,
                    &g_pelican_last_barrel_rate_bits);
                g_pelican_last_barrel_expected_parent = -1;
                InterlockedExchange(
                    &g_pelican_last_barrel_failure_detail, (LONG)detail);
                if (detail & PELICAN_BARREL_DETAIL_RATE_SCALE)
                    InterlockedExchange(
                        &g_pelican_rate_scale_nonfinite_observed, 1);
                release_reason |= PELICAN_RELEASE_BARREL;
            }
        }
        if (pilot && pilot_parent == g_pelican_vehicle) {
            pilot_stale_ticks = 0;
            InterlockedExchange(&g_pelican_pilot_absence_ticks, 0);
        } else {
            pilot_stale_ticks = (int)InterlockedCompareExchange(
                &g_pelican_pilot_absence_ticks, 0, 0) + 1;
            if (pilot_stale_ticks > PELICAN_PILOT_STALE_TICKS)
                pilot_stale_ticks = PELICAN_PILOT_STALE_TICKS;
            InterlockedExchange(&g_pelican_pilot_absence_ticks, pilot_stale_ticks);
        }
        if (release_reason != 0u) {
            pelican_release_relay_main_thread(release_reason);
        } else if (pilot_stale_ticks >= PELICAN_PILOT_STALE_TICKS &&
                   (!pilot || pilot_parent != g_pelican_vehicle)) {
            pelican_release_relay_main_thread(
                release_reason | PELICAN_RELEASE_PILOT_MISSING);
        } else if (!pilot || pilot_parent != g_pelican_vehicle) {
            /* Un siege vide n'est pas une erreur definitive. On coupe le tir
             * sans toucher aux commandes de vol et on garde le relais arme;
             * le prochain vrai conducteur est repris automatiquement. */
            /* Le moteur coupe lui-meme la detente du siege parent. Ne jamais
             * ecrire le controle de vol ou celui du module pendant la sortie;
             * le lien parent-gunner sera reconstruit nativement a la remontee. */
            pelican_direct_gate_abandon(g_pelican_chin_weapon);
            if (!builtin_weapon && module)
                clear_unit_vehicle_primary_control(control_target);
            pelican_stop_audio_proxy();
            /* Garder le snapshot owner valide. Au front de remontee, le
             * rearmement et bind_pelican_weapon_damage_owner le recopient
             * dans le module, son cache physique et l'arme. */
            InterlockedExchange(&g_pelican_applied_primary, 0);
            InterlockedExchange(&g_pelican_driver_present, 0);
        } else {
            int requested = (driver_datum == g_pelican_local_pilot &&
                             local_input_requested) ||
                (driver_datum != -1 &&
                 (unit_primary_trigger_requested(driver_datum) ||
                  (!builtin_weapon &&
                   vehicle_primary_trigger_requested(g_pelican_vehicle)))) ||
                (last_driver_datum == g_pelican_local_pilot &&
                 local_input_requested);
            int applied = 0;
            if (builtin_weapon) {
                uint8_t serial_delta =
                    (uint8_t)(shot_serial - g_pelican_last_shot_serial);
                int control_ok = set_unit_vehicle_primary_control(
                    driver_datum, g_pelican_vehicle,
                    control_target, requested);
                if (serial_delta && serial_delta < 32u)
                    g_pelican_completed_shots += serial_delta;
                g_pelican_last_shot_serial = shot_serial;
                applied = requested && control_ok;
            } else {
                struct pelican_direct_fire_info fire_info;
                struct blam_fire_context fire_context;
                float target_point[3];
                int owner_ok = 1;
                int profile_ok;
                /* Pipeline terrain 1.5.60 restaure exactement : weapon_fire
                 * prepare le barrel et weapon_fire_pending consomme un vrai
                 * contexte de cible retail. C'est ce second appel qui publie
                 * le projectile avec collision, degats et impacts Halo. */
                if (InterlockedCompareExchange(
                        &g_pelican_driver_present, 0, 0) == 0 ||
                    g_pelican_damage_owner_pilot != driver_datum ||
                    InterlockedCompareExchange(
                        &g_pelican_damage_owner_verified, 0, 0) == 0)
                    owner_ok = bind_pelican_weapon_damage_owner(
                        driver_datum, control_target,
                        g_pelican_chin_weapon);
                profile_ok = pelican_direct_fire_snapshot(
                    g_pelican_chin_weapon, -1, &fire_info);
                target_ok = pelican_resolve_effective_reticle_target(
                    driver_datum, g_pelican_vehicle,
                    control_target, target_point);
                if (target_ok)
                    aim_ok = sync_unit_vehicle_aim_to_target(
                        g_pelican_vehicle, control_target,
                        g_pelican_chin_weapon, target_point);
                else
                    aim_ok = sync_unit_vehicle_aim(
                        g_pelican_vehicle, control_target);
                if (!aim_ok)
                    aim_ok = sync_unit_vehicle_aim(
                        g_pelican_vehicle, control_target);
                applied = requested && owner_ok && aim_ok && profile_ok &&
                    target_ok;
                if (profile_ok && pelican_direct_gate_update(
                        g_pelican_chin_weapon, applied, &fire_info)) {
                    int fired = 0;
                    uint8_t *weapon_after;
                    if (target_ok && pelican_prepare_direct_shot(
                            g_pelican_chin_weapon)) {
                        pelican_fill_fire_context(
                            &fire_context, target_point);
                        ((weapon_fire_fn)(g_blam + 0x5CC1F0))(
                            g_pelican_chin_weapon, 0, 0);
                        ((weapon_fire_pending_fn)(g_blam + 0x5BE600))(
                            g_pelican_chin_weapon, &fire_context, 1u, 0);
                        fired = 1;
                        ++g_pelican_retail_context_shots;
                        if (g_pelican_aux_impact_projectile_tag == -1 &&
                            object_data(g_pelican_audio_weapon))
                            (void)pelican_prepare_aux_impact_projectile(
                                g_pelican_audio_weapon);
                        if (g_pelican_aux_impact_projectile_tag != -1 &&
                            object_data(g_pelican_audio_weapon)) {
                            int32_t impact_projectile =
                                pelican_spawn_reticle_projectile(
                                    driver_datum, g_pelican_vehicle,
                                    control_target,
                                    g_pelican_chin_weapon,
                                    g_pelican_aux_impact_projectile_tag,
                                    target_point);
                            if (impact_projectile != -1)
                                ++g_pelican_aux_impact_created;
                            else
                                ++g_pelican_aux_impact_failures;
                        }
                        ++g_pelican_completed_shots;
                        (void)pelican_emit_audio_proxy_shot();
                    } else {
                        ++g_pelican_retail_context_failures;
                    }
                    weapon_after = object_data(g_pelican_chin_weapon);
                    if (weapon_after && readable_range(
                            weapon_after + 0x25E, sizeof(uint8_t)))
                        g_pelican_last_shot_serial = weapon_after[0x25E];
                    pelican_direct_gate_commit(fire_info.tick, fired);
                }
                if (!requested) pelican_stop_audio_proxy();
            }
            InterlockedExchange(&g_pelican_last_target_ok, target_ok ? 1L : 0L);
            InterlockedExchange(&g_pelican_last_aim_ok, aim_ok ? 1L : 0L);
            InterlockedExchange(&g_pelican_fire_interval_ms,
                                (LONG)authored_interval);
            InterlockedExchange(&g_pelican_driver_present, 1);
            InterlockedExchange(&g_pelican_applied_primary, applied);
        }
    } else if (command.kind == COMMAND_PELICAN_STATUS) {
        uint8_t *vehicle = object_data(g_pelican_vehicle);
        uint8_t *weapon = object_data(g_pelican_chin_weapon);
        struct pelican_direct_fire_info fire_info;
        int32_t driver_datum = -1;
        uint32_t authored_interval = 0;
        uint8_t shot_serial = 0;
        int state = -1;
        int timer = -1;
        unsigned int barrel_flags = 0;
        unsigned int runtime_shots = 0;
        int32_t presentation_slot_effect = -1;
        int32_t presentation_instance = -1;
        int presentation_slot_ok = 0;
        uint8_t *tracer_barrel = NULL;
        uint16_t tracer_current = 0xFFFFu;
        int tracer_current_valid = 0;
        uint8_t *presentation_barrel = NULL;
        uint8_t *presentation_records = NULL;
        uint32_t presentation_record_count = 0;
        uint32_t presentation_authored_flags = 0;
        uint16_t presentation_selector_used = 0;
        int16_t presentation_selector_record = -1;
        int16_t presentation_selector_left = -1;
        int32_t presentation_selected_effect = -1;
        uint8_t presentation_selected_slot = 0;
        int profile_ok;
        int32_t expected_parent = g_pelican_chin_module != -1
            ? g_pelican_chin_module : g_pelican_vehicle;
        int32_t weapon_parent = -1;
        int32_t weapon_owner = -1;
        int32_t weapon_owner_fallback = -1;
        unsigned int weapon_owner_fallback_enabled = 0;
        int current_weapon_index = -1;
        int desired_weapon_index = -1;
        int32_t inventory_slots[4] = { -1, -1, -1, -1 };
        float weapon_origin[3] = { 0.0f, 0.0f, 0.0f };
        float owner_origin[3] = { 0.0f, 0.0f, 0.0f };
        float vehicle_origin[3] = { 0.0f, 0.0f, 0.0f };
        float driver_aim[3] = { 0.0f, 0.0f, 0.0f };
        float vehicle_aim[3] = { 0.0f, 0.0f, 0.0f };
        float module_aim[3] = { 0.0f, 0.0f, 0.0f };
        float vehicle_current_aim[3] = { 0.0f, 0.0f, 0.0f };
        float module_current_aim[3] = { 0.0f, 0.0f, 0.0f };
        float vehicle_look[3] = { 0.0f, 0.0f, 0.0f };
        float module_look[3] = { 0.0f, 0.0f, 0.0f };
        int weapon_origin_valid = 0;
        int owner_origin_valid = 0;
        int vehicle_origin_valid = 0;
        int aim_probe_valid = 0;
        uint8_t *expected_parent_object = object_data(expected_parent);
        int association_valid = 0;
        int weapon_valid;
        if (weapon && readable_range(weapon + 0x14, sizeof(int32_t)))
            weapon_parent = *(int32_t *)(weapon + 0x14);
        if (weapon && readable_range(weapon + 0x324, sizeof(int32_t)))
            weapon_owner = *(int32_t *)(weapon + 0x320);
        if (weapon && readable_range(weapon + 0x1A8, sizeof(int32_t))) {
            weapon_owner_fallback_enabled = weapon[0x199];
            weapon_owner_fallback = *(int32_t *)(weapon + 0x1A4);
        }
        if (expected_parent_object && readable_range(
                expected_parent_object + UNIT_WEAPON_INVENTORY_OFFSET,
                4u * sizeof(int32_t))) {
            int slot;
            for (slot = 0; slot < 4; ++slot)
                inventory_slots[slot] = *(int32_t *)(
                    expected_parent_object + UNIT_WEAPON_INVENTORY_OFFSET +
                    (size_t)slot * sizeof(int32_t));
        }
        if (expected_parent_object && readable_range(
                expected_parent_object + UNIT_DESIRED_WEAPON_INDEX_OFFSET,
                sizeof(uint8_t))) {
            current_weapon_index = expected_parent_object[
                UNIT_CURRENT_WEAPON_INDEX_OFFSET];
            desired_weapon_index = expected_parent_object[
                UNIT_DESIRED_WEAPON_INDEX_OFFSET];
        }
        if (weapon) {
            ((object_get_origin_fn)(g_blam + 0x5A6780))(
                g_pelican_chin_weapon, weapon_origin);
            weapon_origin_valid = isfinite(weapon_origin[0]) &&
                isfinite(weapon_origin[1]) && isfinite(weapon_origin[2]);
        }
        if (expected_parent_object) {
            ((object_get_origin_fn)(g_blam + 0x5A6780))(
                expected_parent, owner_origin);
            owner_origin_valid = isfinite(owner_origin[0]) &&
                isfinite(owner_origin[1]) && isfinite(owner_origin[2]);
        }
        if (vehicle) {
            ((object_get_origin_fn)(g_blam + 0x5A6780))(
                g_pelican_vehicle, vehicle_origin);
            vehicle_origin_valid = isfinite(vehicle_origin[0]) &&
                isfinite(vehicle_origin[1]) && isfinite(vehicle_origin[2]);
        }
        association_valid = expected_parent != -1 &&
            (unit_has_weapon_datum(
                 expected_parent, g_pelican_chin_weapon) ||
             weapon_parent == expected_parent);
        weapon_valid = pelican_primary_barrel_info(
            g_pelican_chin_weapon, -1,
            &authored_interval, &shot_serial);
        memset(&fire_info, 0, sizeof(fire_info));
        profile_ok = pelican_direct_fire_snapshot(
            g_pelican_chin_weapon, -1, &fire_info);
        if (weapon && readable_range(weapon + 0x235, 7u)) {
            state = *(int8_t *)(weapon + 0x235);
            timer = *(int16_t *)(weapon + 0x236);
            barrel_flags = *(uint16_t *)(weapon + 0x238);
            runtime_shots = *(uint16_t *)(weapon + 0x23A);
        }
        if (weapon && readable_range(weapon, sizeof(int32_t))) {
            tracer_barrel = pelican_primary_barrel_for_weapon_tag(
                *(int32_t *)weapon);
            if (tracer_barrel && readable_range(
                    tracer_barrel + 0x3Eu, sizeof(tracer_current))) {
                tracer_current = *(uint16_t *)(tracer_barrel + 0x3Eu);
                tracer_current_valid = 1;
            }
            presentation_slot_ok = pelican_primary_firing_effect_slot(
                *(int32_t *)weapon, NULL, &presentation_slot_effect);
            presentation_records = pelican_firing_effect_records(
                *(int32_t *)weapon, &presentation_barrel,
                &presentation_record_count);
            if (presentation_barrel && readable_range(
                    presentation_barrel, sizeof(uint32_t)))
                presentation_authored_flags =
                    *(uint32_t *)presentation_barrel;
            if (readable_range(weapon + 0x23C, 6u)) {
                presentation_selector_used =
                    *(uint16_t *)(weapon + 0x23C);
                presentation_selector_record =
                    *(int16_t *)(weapon + 0x23E);
                presentation_selector_left =
                    *(int16_t *)(weapon + 0x240);
            }
            if (presentation_records &&
                presentation_selector_record >= 0 &&
                (uint32_t)presentation_selector_record <
                    presentation_record_count)
                (void)pelican_effect_record_selected_slot(
                    presentation_records +
                        (size_t)presentation_selector_record * 0xC4u,
                    NULL, &presentation_selected_effect,
                    &presentation_selected_slot);
            if (readable_range(weapon + 0x260, sizeof(int32_t)))
                presentation_instance = *(int32_t *)(weapon + 0x260);
        }
        if (vehicle && readable_range(
                vehicle + UNIT_DRIVER_OBJECT_OFFSET, sizeof(int32_t)))
            driver_datum = *(int32_t *)(vehicle + UNIT_DRIVER_OBJECT_OFFSET);
        {
            uint8_t *driver = object_data(driver_datum);
            uint8_t *module_object = object_data(g_pelican_chin_module);
            if (driver && vehicle && module_object &&
                readable_range(driver + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
                               sizeof(driver_aim)) &&
                readable_range(vehicle + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
                               sizeof(vehicle_aim)) &&
                readable_range(module_object + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
                               sizeof(module_aim)) &&
                readable_range(vehicle + UNIT_AIMING_VECTOR_OFFSET,
                               sizeof(vehicle_current_aim)) &&
                readable_range(module_object + UNIT_AIMING_VECTOR_OFFSET,
                               sizeof(module_current_aim)) &&
                readable_range(vehicle + UNIT_DESIRED_LOOKING_VECTOR_OFFSET,
                               sizeof(vehicle_look)) &&
                readable_range(module_object + UNIT_DESIRED_LOOKING_VECTOR_OFFSET,
                               sizeof(module_look))) {
                memcpy(driver_aim,
                       driver + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
                       sizeof(driver_aim));
                memcpy(vehicle_aim,
                       vehicle + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
                       sizeof(vehicle_aim));
                memcpy(module_aim,
                       module_object + UNIT_DESIRED_AIMING_VECTOR_OFFSET,
                       sizeof(module_aim));
                memcpy(vehicle_current_aim,
                       vehicle + UNIT_AIMING_VECTOR_OFFSET,
                       sizeof(vehicle_current_aim));
                memcpy(module_current_aim,
                       module_object + UNIT_AIMING_VECTOR_OFFSET,
                       sizeof(module_current_aim));
                memcpy(vehicle_look,
                       vehicle + UNIT_DESIRED_LOOKING_VECTOR_OFFSET,
                       sizeof(vehicle_look));
                memcpy(module_look,
                       module_object + UNIT_DESIRED_LOOKING_VECTOR_OFFSET,
                       sizeof(module_look));
                aim_probe_valid = 1;
            }
        }
        snprintf(g_result, sizeof(g_result),
                 "ok pelican_status armed=%ld release_reason=%ld release_count=%ld module_inventory_missing_observed=%ld rate_scale_nonfinite_observed=%ld barrel_failure_detail=%ld barrel_actual_parent=%d barrel_expected_parent=%d barrel_weapon_tag=%d barrel_count=%u barrel_min_bits=%08X barrel_max_bits=%08X barrel_rate_bits=%08X last_release_vehicle=%d last_release_module=%d last_release_weapon=%d local_pilot=%d driver=%d vehicle=%d module=%d weapon=%d weapon_parent=%d association_valid=%d weapon_valid=%d profile_ok=%d input_local=%ld input_unit=%d input_vehicle=%d pilot_absence=%ld input_hold=%ld last_driver=%ld last_target_ok=%ld last_aim_ok=%ld interval_ms=%u tick=%d interval_ticks=%u recovery_ticks=%u min_shots=%d max_shots=%d state=%d timer=%d barrel_flags=%04X runtime_shots=%u serial=%u completed_shots=%u gate_down=%u gate_shots=%u gate_recovery=%u recovery_until=%d last_success=%d applied=%ld tracer_patch=%d tracer_error=%d tracer_weapon_tag=%d tracer_original_valid=%d tracer_original=%u tracer_current_valid=%d tracer_current=%u tracer_patch_count=%u presentation_patch=%d presentation_error=%d presentation_weapon_tag=%d presentation_original=%d presentation_effect=%d presentation_patch_count=%u presentation_records=%u presentation_secondary_mask=%08X presentation_authored_flags=%08X presentation_slot_ok=%d presentation_slot_effect=%d selector_used=%u selector_record=%d selector_left=%d selected_slot=%u selected_effect=%d presentation_instance=%d\n",
                 InterlockedCompareExchange(&g_pelican_armed, 0, 0),
                 InterlockedCompareExchange(
                     &g_pelican_last_release_reason, 0, 0),
                 InterlockedCompareExchange(&g_pelican_release_count, 0, 0),
                 InterlockedCompareExchange(
                     &g_pelican_module_inventory_missing_observed, 0, 0),
                 InterlockedCompareExchange(
                     &g_pelican_rate_scale_nonfinite_observed, 0, 0),
                 InterlockedCompareExchange(
                     &g_pelican_last_barrel_failure_detail, 0, 0),
                 g_pelican_last_barrel_actual_parent,
                 g_pelican_last_barrel_expected_parent,
                 g_pelican_last_barrel_weapon_tag,
                 g_pelican_last_barrel_count,
                 g_pelican_last_barrel_min_bits,
                 g_pelican_last_barrel_max_bits,
                 g_pelican_last_barrel_rate_bits,
                 g_pelican_last_release_vehicle,
                 g_pelican_last_release_module,
                 g_pelican_last_release_weapon,
                 g_pelican_local_pilot, driver_datum, g_pelican_vehicle,
                 g_pelican_chin_module, g_pelican_chin_weapon,
                 weapon_parent, association_valid, weapon_valid, profile_ok,
                 InterlockedCompareExchange(&g_pelican_requested_primary, 0, 0),
                 unit_primary_trigger_requested(driver_datum),
                 vehicle_primary_trigger_requested(g_pelican_vehicle),
                 InterlockedCompareExchange(&g_pelican_pilot_absence_ticks, 0, 0),
                 InterlockedCompareExchange(&g_pelican_local_input_hold_ticks, 0, 0),
                 InterlockedCompareExchange(&g_pelican_last_driver_datum, 0, 0),
                 InterlockedCompareExchange(&g_pelican_last_target_ok, 0, 0),
                 InterlockedCompareExchange(&g_pelican_last_aim_ok, 0, 0),
                 authored_interval, fire_info.tick,
                 fire_info.authored.interval_ticks,
                 fire_info.authored.recovery_ticks,
                 (int)fire_info.authored.min_shots,
                 (int)fire_info.authored.max_shots, state, timer,
                 barrel_flags, runtime_shots, (unsigned)shot_serial,
                 g_pelican_completed_shots,
                 (unsigned)g_pelican_direct_gate.was_down,
                 (unsigned)g_pelican_direct_gate.shots_in_hold,
                 (unsigned)g_pelican_direct_gate.recovery_active,
                  g_pelican_direct_gate.recovery_until_tick,
                  g_pelican_direct_gate.last_success_tick,
                  InterlockedCompareExchange(&g_pelican_applied_primary, 0, 0),
                  g_pelican_tracer_patch_active,
                  g_pelican_tracer_patch_error,
                  g_pelican_tracer_weapon_tag,
                  g_pelican_tracer_original_valid,
                  (unsigned)g_pelican_tracer_original_rounds,
                  tracer_current_valid, (unsigned)tracer_current,
                  g_pelican_tracer_patch_count,
                  g_pelican_presentation_patch_active,
                 g_pelican_presentation_patch_error,
                 g_pelican_presentation_weapon_tag,
                 g_pelican_presentation_original_effect,
                 g_pelican_presentation_effect,
                 g_pelican_presentation_patch_count,
                 presentation_record_count,
                 g_pelican_presentation_secondary_mask,
                 presentation_authored_flags,
                 presentation_slot_ok, presentation_slot_effect,
                 (unsigned)presentation_selector_used,
                 (int)presentation_selector_record,
                 (int)presentation_selector_left,
                 (unsigned)presentation_selected_slot,
                  presentation_selected_effect,
                  presentation_instance);
        {
            size_t used = strlen(g_result);
            if (used < sizeof(g_result))
                snprintf(g_result + used, sizeof(g_result) - used,
                         "association_probe owner320=%d fallback199=%u fallback1A4=%d expected_owner=%d owner_matches=%d owner_kind=%ld owner_cache=%ld owner_verified=%ld current_index=%d desired_index=%d slots=%d,%d,%d,%d weapon_origin_valid=%d weapon_origin=%.3f,%.3f,%.3f owner_origin_valid=%d owner_origin=%.3f,%.3f,%.3f vehicle_origin_valid=%d vehicle_origin=%.3f,%.3f,%.3f\n",
                         weapon_owner, weapon_owner_fallback_enabled,
                         weapon_owner_fallback, expected_parent,
                         weapon_owner == expected_parent,
                         InterlockedCompareExchange(
                             &g_pelican_damage_owner_kind, 0, 0),
                         InterlockedCompareExchange(
                             &g_pelican_damage_owner_cache_active, 0, 0),
                         InterlockedCompareExchange(
                             &g_pelican_damage_owner_verified, 0, 0),
                         current_weapon_index, desired_weapon_index,
                         inventory_slots[0], inventory_slots[1],
                         inventory_slots[2], inventory_slots[3],
                         weapon_origin_valid, weapon_origin[0],
                         weapon_origin[1], weapon_origin[2],
                         owner_origin_valid, owner_origin[0],
                         owner_origin[1], owner_origin[2],
                         vehicle_origin_valid, vehicle_origin[0],
                         vehicle_origin[1], vehicle_origin[2]);
        }
        {
            size_t used = strlen(g_result);
            if (used < sizeof(g_result))
                snprintf(g_result + used, sizeof(g_result) - used,
                         "aim_probe valid=%d driver_desired=%.6f,%.6f,%.6f vehicle_desired=%.6f,%.6f,%.6f module_desired=%.6f,%.6f,%.6f vehicle_current=%.6f,%.6f,%.6f module_current=%.6f,%.6f,%.6f vehicle_look=%.6f,%.6f,%.6f module_look=%.6f,%.6f,%.6f\n",
                         aim_probe_valid,
                         driver_aim[0], driver_aim[1], driver_aim[2],
                         vehicle_aim[0], vehicle_aim[1], vehicle_aim[2],
                         module_aim[0], module_aim[1], module_aim[2],
                         vehicle_current_aim[0], vehicle_current_aim[1],
                         vehicle_current_aim[2], module_current_aim[0],
                         module_current_aim[1], module_current_aim[2],
                         vehicle_look[0], vehicle_look[1], vehicle_look[2],
                         module_look[0], module_look[1], module_look[2]);
        }
        {
            size_t used = strlen(g_result);
            if (used < sizeof(g_result))
                snprintf(g_result + used, sizeof(g_result) - used,
                         "projectile_probe attempts=%u created=%u aux_created=%u aux_failures=%u aux_tag=%d failure_stage=%u placement_variant=%u fx_suppressed=%u fx_suppression_failures=%u datum=%d tag=%d impact_damage=%d owner_match=%u weapon=%d barrel=%u parent=%d source_root=%d authored_speed=%.3f final_speed=%.3f aim=%.6f,%.6f,%.6f position=%.3f,%.3f,%.3f forward=%.6f,%.6f,%.6f velocity=%.3f,%.3f,%.3f\n",
                         g_pelican_projectile_probe.attempts,
                         g_pelican_projectile_probe.created,
                         g_pelican_aux_impact_created,
                         g_pelican_aux_impact_failures,
                         g_pelican_aux_impact_projectile_tag,
                         (unsigned)g_pelican_projectile_probe.failure_stage,
                         (unsigned)g_pelican_projectile_probe.placement_variant,
                         g_pelican_projectile_probe.fx_suppressed,
                         g_pelican_projectile_probe.fx_suppression_failures,
                         g_pelican_projectile_probe.last_datum,
                         g_pelican_projectile_probe.last_tag,
                         g_pelican_projectile_probe.last_impact_damage,
                         (unsigned)g_pelican_projectile_probe.owner_match,
                         g_pelican_projectile_probe.last_weapon,
                         (unsigned)g_pelican_projectile_probe.last_barrel,
                         g_pelican_projectile_probe.last_parent,
                         g_pelican_projectile_probe.last_source_root,
                         g_pelican_projectile_probe.authored_speed,
                         g_pelican_projectile_probe.final_speed,
                         g_pelican_projectile_probe.aim[0],
                         g_pelican_projectile_probe.aim[1],
                         g_pelican_projectile_probe.aim[2],
                         g_pelican_projectile_probe.position[0],
                         g_pelican_projectile_probe.position[1],
                         g_pelican_projectile_probe.position[2],
                         g_pelican_projectile_probe.forward[0],
                         g_pelican_projectile_probe.forward[1],
                         g_pelican_projectile_probe.forward[2],
                         g_pelican_projectile_probe.velocity[0],
                         g_pelican_projectile_probe.velocity[1],
                         g_pelican_projectile_probe.velocity[2]);
        }
        {
            size_t used = strlen(g_result);
            ULONGLONG now_ms = GetTickCount64();
            ULONGLONG updated_ms = g_pelican_camera_target.updated_at_ms;
            ULONGLONG age_ms = updated_ms <= now_ms
                ? now_ms - updated_ms : ~(ULONGLONG)0;
            if (used < sizeof(g_result))
                snprintf(g_result + used, sizeof(g_result) - used,
                         "camera_target_probe valid=%ld vehicle=%d age_ms=%llu samples=%u rejected=%u retail_context_shots=%u retail_context_failures=%u last_source_camera=%ld local=%.3f,%.3f,%.3f world=%.3f,%.3f,%.3f\n",
                         InterlockedCompareExchange(
                             &g_pelican_camera_target.valid, 0, 0),
                         g_pelican_camera_target.vehicle,
                         (unsigned long long)age_ms,
                         g_pelican_camera_target.accepted_samples,
                         g_pelican_camera_target.rejected_samples,
                         g_pelican_retail_context_shots,
                         g_pelican_retail_context_failures,
                         InterlockedCompareExchange(
                             &g_pelican_last_target_from_camera, 0, 0),
                         g_pelican_camera_target.local_target[0],
                         g_pelican_camera_target.local_target[1],
                         g_pelican_camera_target.local_target[2],
                         g_pelican_last_target_point[0],
                         g_pelican_last_target_point[1],
                         g_pelican_last_target_point[2]);
        }
        {
            size_t used = strlen(g_result);
            if (used < sizeof(g_result))
                snprintf(g_result + used, sizeof(g_result) - used,
                         "pitch_probe active=%d error=%d pelican_tag=%d original_min=%.6f original_max=%.6f target_min=%.6f target_max=%.6f patch_count=%u original_pelican_handling=1 spirit_handling_patch=0\n",
                         g_pelican_pitch_patch_active,
                         g_pelican_pitch_patch_error,
                         g_pelican_pitch_patch_tag,
                         g_pelican_pitch_original_min,
                         g_pelican_pitch_original_max,
                         -PELICAN_PILOT_PITCH_LIMIT_RAD,
                         PELICAN_PILOT_PITCH_LIMIT_RAD,
                         g_pelican_pitch_patch_count);
        }
        {
            size_t used = strlen(g_result);
            if (used < sizeof(g_result))
                snprintf(g_result + used, sizeof(g_result) - used,
                         "boost_probe active=%d error=%d pelican_tag=%d original_flags=%08X last_before=%08X last_after=%08X unit_has_boost_bit=%08X patch_count=%u original_curve=1 peak_power=3.0 banshee_copy=0 human_plane_preserved=1 lua_tick=0 velocity_write=0\n",
                         g_pelican_boost_patch_active,
                         g_pelican_boost_patch_error,
                         g_pelican_boost_patch_tag,
                         g_pelican_boost_original_flags,
                         g_pelican_boost_last_before,
                         g_pelican_boost_last_after,
                         UNIT_HAS_BOOST_FLAG,
                         g_pelican_boost_patch_count);
        }
        {
            size_t used = strlen(g_result);
            if (used < sizeof(g_result))
                snprintf(g_result + used, sizeof(g_result) - used,
                         "audio_probe module=%d module_valid=%d weapon=%d weapon_valid=%d start_edges=%u failures=%u signal=%ld sequence=%ld path=Lua_BP_WarthogChaingun_AudioOnWeaponFire\n",
                         g_pelican_audio_module,
                         object_data(g_pelican_audio_module) != NULL,
                         g_pelican_audio_weapon,
                         object_data(g_pelican_audio_weapon) != NULL,
                         g_pelican_audio_fire_events,
                         g_pelican_audio_fire_failures,
                         InterlockedCompareExchange(
                             &g_pelican_audio_signal_state, 0, 0),
                         InterlockedCompareExchange(
                             &g_pelican_audio_signal_sequence, 0, 0));
        }
#ifdef HALOSANDBOX_RUNTIME_TEST
        if (presentation_records) {
            uint32_t i;
            for (i = 0; i < presentation_record_count; ++i) {
                uint8_t *record = presentation_records +
                    (size_t)i * 0xC4u;
                size_t used = strlen(g_result);
                if (used >= sizeof(g_result)) break;
                snprintf(g_result + used, sizeof(g_result) - used,
                         "fx%u_bounds=%d,%d fx%u_primary=%08X,%d fx%u_secondary=%08X,%d\n",
                         i, (int)*(int16_t *)(record + 0u),
                         (int)*(int16_t *)(record + 2u),
                         i, *(uint32_t *)(record + 4u),
                         *(int32_t *)(record + 0x10u),
                         i, *(uint32_t *)(record + 0x34u),
                         *(int32_t *)(record + 0x40u));
            }
        }
        {
            size_t used = strlen(g_result);
            if (used < sizeof(g_result))
                snprintf(g_result + used, sizeof(g_result) - used,
                         "shot_probe flags=%04X,%04X,%04X shots=%u,%u record=%d,%d left=%d,%d serial=%u,%u instance=%d,%d\n",
                         g_pelican_runtime_shot_probe.flags_pre,
                         g_pelican_runtime_shot_probe.flags_post_mask,
                         g_pelican_runtime_shot_probe.flags_post_fire,
                         (unsigned)g_pelican_runtime_shot_probe.shots_pre,
                         (unsigned)g_pelican_runtime_shot_probe.shots_post,
                         (int)g_pelican_runtime_shot_probe.record_pre,
                         (int)g_pelican_runtime_shot_probe.record_post,
                         (int)g_pelican_runtime_shot_probe.remaining_pre,
                         (int)g_pelican_runtime_shot_probe.remaining_post,
                         (unsigned)g_pelican_runtime_shot_probe.serial_pre,
                         (unsigned)g_pelican_runtime_shot_probe.serial_post,
                         g_pelican_runtime_shot_probe.instance_pre,
                         g_pelican_runtime_shot_probe.instance_post);
        }
#endif
    } else if (command.kind == COMMAND_VELOCITY) {
        float magnitude_sq = command.x * command.x + command.y * command.y + command.z * command.z;
        if (!object_data(command.a)) {
            snprintf(g_result, sizeof(g_result), "error velocity_object_missing datum=%d\n", command.a);
        } else if (!isfinite(command.x) || !isfinite(command.y) || !isfinite(command.z) ||
            magnitude_sq > 250000.0f) {
            snprintf(g_result, sizeof(g_result), "error velocity_out_of_range\n");
        } else {
            ((object_set_velocity_fn)(g_blam + 0x663B90))(command.a, command.x, command.y, command.z);
            snprintf(g_result, sizeof(g_result), "ok velocity datum=%d vector=%.3f,%.3f,%.3f\n",
                     command.a, command.x, command.y, command.z);
        }
    } else if (command.kind == COMMAND_ORIGIN) {
        float position[3], forward[3], up[3];
        if (!object_data(command.a)) {
            snprintf(g_result, sizeof(g_result), "error origin_object_missing datum=%d\n", command.a);
        } else {
            ((object_get_origin_fn)(g_blam + 0x5A6780))(command.a, position);
            ((object_get_orientation_fn)(g_blam + 0x5A6AE0))(command.a, forward, up);
        }
        if (!object_data(command.a)) {
            /* Le message precis est deja prepare ci-dessus. */
        } else if (!isfinite(position[0]) || !isfinite(position[1]) || !isfinite(position[2])) {
            snprintf(g_result, sizeof(g_result), "error origin_invalid datum=%d\n", command.a);
        } else {
            snprintf(g_result, sizeof(g_result),
                     "ok origin datum=%d position=%.6f,%.6f,%.6f forward=%.6f,%.6f,%.6f up=%.6f,%.6f,%.6f\n",
                     command.a, position[0], position[1], position[2],
                     forward[0], forward[1], forward[2], up[0], up[1], up[2]);
        }
    } else if (command.kind == COMMAND_TELEPORT) {
        float forward[3], up[3], position[3] = { command.x, command.y, command.z };
        if (!object_data(command.a)) {
            snprintf(g_result, sizeof(g_result), "error teleport_object_missing datum=%d\n", command.a);
        } else if (!isfinite(command.x) || !isfinite(command.y) || !isfinite(command.z) ||
            fabsf(command.x) > 1000000.0f || fabsf(command.y) > 1000000.0f || fabsf(command.z) > 1000000.0f) {
            snprintf(g_result, sizeof(g_result), "error teleport_out_of_range\n");
        } else {
            ((object_get_orientation_fn)(g_blam + 0x5A6AE0))(command.a, forward, up);
            ((object_teleport_fn)(g_blam + 0x351610))(command.a, position, forward, up, 1);
            snprintf(g_result, sizeof(g_result), "ok teleport datum=%d position=%.6f,%.6f,%.6f\n",
                     command.a, command.x, command.y, command.z);
        }
    } else if (command.kind == COMMAND_SEATFLAGS) {
        uint32_t count = 0;
        uint8_t *seats = resolve_seats(command.a, &count);
        if (!seats || command.b < 0 || (uint32_t)command.b >= count) {
            snprintf(g_result, sizeof(g_result), "error seatflags invalid tag=%d seat=%d count=%u\n",
                     command.a, command.b, count);
        } else {
            uint32_t *flags = (uint32_t *)(seats + (size_t)command.b * 328u);
            uint32_t before = *flags;
            *flags = (*flags | (uint32_t)command.c) & ~(uint32_t)command.d;
            snprintf(g_result, sizeof(g_result),
                     "ok seatflags tag=%d seat=%d before=%08X after=%08X set=%08X clear=%08X\n",
                     command.a, command.b, before, *flags, (uint32_t)command.c, (uint32_t)command.d);
        }
    } else if (command.kind == COMMAND_ENTERABLE) {
        ((unit_set_enterable_fn)(g_blam + 0x6053D0))(command.a, command.b != 0);
        snprintf(g_result, sizeof(g_result), "ok enterable vehicle=%d enabled=%d\n", command.a, command.b != 0);
    } else if (command.kind == COMMAND_ENTER) {
        uint8_t accepted;
        accepted = submit_enter_action(command.a, command.b, command.c);
        snprintf(g_result, sizeof(g_result), accepted ? "ok enter accepted\n" : "error enter_rejected\n");
    } else if (command.kind == COMMAND_ENTER_IMMEDIATE) {
        int requested = enter_vehicle_immediate(command.a, command.b, command.c, command.d);
        snprintf(g_result, sizeof(g_result), requested
                 ? "ok enter_immediate unit=%d vehicle=%d tag=%d seat=%d requested=1\n"
                 : "error enter_immediate_rejected unit=%d vehicle=%d tag=%d seat=%d\n",
                 command.a, command.b, command.c, command.d);
    } else if (command.kind == COMMAND_PARENT) {
        uint8_t *object = object_data(command.a);
        if (!object || !readable_range(object + 0x14, sizeof(int32_t)) ||
            !readable_range(object + 0x328, sizeof(int16_t))) {
            snprintf(g_result, sizeof(g_result), "error parent object_unavailable=%d\n", command.a);
        }
        else snprintf(g_result, sizeof(g_result), "ok parent unit=%d parent=%d transition=%d\n",
                      command.a, *(int32_t *)(object + 0x14), *(int16_t *)(object + 0x328));
    } else if (command.kind == COMMAND_EXIT_FORCE) {
        uint8_t accepted = submit_force_exit_action(command.a);
        snprintf(g_result, sizeof(g_result), accepted
                 ? "ok exit_force unit=%d accepted=1\n"
                 : "error exit_force_rejected unit=%d\n", command.a);
    } else if (command.kind == COMMAND_SEATINFO) {
        uint32_t count = 0;
        uint8_t *seats = resolve_seats(command.a, &count);
        if (!seats) snprintf(g_result, sizeof(g_result), "error seatinfo tag_not_resident_or_invalid=%d\n", command.a);
        else {
            int used = snprintf(g_result, sizeof(g_result),
                                "ok seatinfo tag=%d count=%u seats=%p flags=",
                                command.a, count, seats);
            uint32_t i;
            for (i = 0; i < count && used > 0 && (size_t)used < sizeof(g_result) - 16; ++i) {
                used += snprintf(g_result + used, sizeof(g_result) - (size_t)used,
                                 "%s%08X", i ? "," : "", *(uint32_t *)(seats + i * 328u));
            }
            snprintf(g_result + used, sizeof(g_result) - (size_t)used, "\n");
        }
    } else if (command.kind == COMMAND_UNLOCKSEATS) {
        uint32_t count = 0;
        if (unlock_vehicle_seats(command.a, &count)) {
            snprintf(g_result, sizeof(g_result), "ok unlockseats tag=%d seats=%u process_memory_only=1\n", command.a, count);
        } else snprintf(g_result, sizeof(g_result), "error unlockseats tag_not_resident_or_invalid=%d\n", command.a);
    } else if (command.kind == COMMAND_TAGDUMP) {
        uint8_t *definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
            0x69686576u, command.a);
        size_t used = 0;
        int32_t i;
        if (!definition || command.b < 0 || command.c <= 0 ||
            command.b > 1604 || command.c > 1604 - command.b ||
            !readable_range(definition + command.b, (size_t)command.c)) {
            snprintf(g_result, sizeof(g_result),
                     "error tagdump invalid tag=%d offset=%d size=%d\n",
                     command.a, command.b, command.c);
        } else {
            used = (size_t)snprintf(g_result, sizeof(g_result),
                                    "ok tagdump tag=%d definition=%p offset=%d size=%d bytes=",
                                    command.a, definition, command.b, command.c);
            for (i = 0; i < command.c && used < sizeof(g_result) - 4; ++i)
                used += (size_t)snprintf(g_result + used, sizeof(g_result) - used,
                                         "%02X", definition[command.b + i]);
            snprintf(g_result + used, sizeof(g_result) - used, "\n");
        }
    } else if (command.kind == COMMAND_TAGBLOCK) {
        uint8_t *definition = (uint8_t *)((tag_get_fn)(g_blam + 0x201480))(
            0x69686576u, command.a);
        uint8_t *records = NULL;
        uint32_t count = 0;
        uint32_t token = 0;
        size_t used = 0;
        uint32_t i;
        if (!definition || command.b < 0 || command.b > 1604 - 12 ||
            command.c <= 0 || command.c > 1024 ||
            !readable_range(definition + command.b, 12)) {
            snprintf(g_result, sizeof(g_result),
                     "error tagblock invalid tag=%d offset=%d stride=%d\n",
                     command.a, command.b, command.c);
        } else {
            count = *(uint32_t *)(definition + command.b);
            token = *(uint32_t *)(definition + command.b + 4);
            if (count == 0) {
                snprintf(g_result, sizeof(g_result),
                         "ok tagblock tag=%d offset=%d stride=%d count=0 token=%08X bytes=\n",
                         command.a, command.b, command.c, token);
            } else {
                records = resolve_vehicle_tag_block(command.a, (uint32_t)command.b,
                                                    (uint32_t)command.c, &count);
                if (!records) {
                    snprintf(g_result, sizeof(g_result),
                             "error tagblock unreadable tag=%d offset=%d stride=%d count=%u token=%08X\n",
                             command.a, command.b, command.c, count, token);
                } else {
                    used = (size_t)snprintf(g_result, sizeof(g_result),
                                            "ok tagblock tag=%d offset=%d stride=%d count=%u token=%08X records=%p bytes=",
                                            command.a, command.b, command.c, count, token, records);
                    for (i = 0; i < count * (uint32_t)command.c &&
                                used < sizeof(g_result) - 4; ++i)
                        used += (size_t)snprintf(g_result + used,
                                                sizeof(g_result) - used,
                                                "%02X", records[i]);
                    snprintf(g_result + used, sizeof(g_result) - used, "\n");
                }
            }
        }
    } else if (command.kind == COMMAND_MODELMARKERS) {
        int32_t model_tag = -1;
        int32_t skeleton_tag = -1;
        uint32_t group_count = 0;
        uint8_t *groups = resolve_vehicle_model_markers(
            command.a, &model_tag, &skeleton_tag, &group_count);
        uint8_t **pages = (uint8_t **)(g_blam + 0x2C2CCC0);
        size_t used = 0;
        uint32_t group_index;
        if (!groups) {
            snprintf(g_result, sizeof(g_result),
                     "error modelmarkers unresolved vehicle=%d\n", command.a);
        } else {
            used = (size_t)snprintf(g_result, sizeof(g_result),
                                    "ok modelmarkers vehicle=%d model=%d skeleton=%d groups=%u\n",
                                    command.a, model_tag, skeleton_tag, group_count);
            for (group_index = 0; group_index < group_count &&
                                  used < sizeof(g_result) - 180u; ++group_index) {
                uint8_t *group = groups + (size_t)group_index * 16u;
                uint32_t string_id = *(uint32_t *)(group + 0);
                uint32_t marker_count = *(uint32_t *)(group + 4);
                uint32_t marker_token = *(uint32_t *)(group + 8);
                uint32_t page = marker_token >> 28;
                uint8_t *markers = NULL;
                uint32_t marker_index;
                used += (size_t)snprintf(g_result + used, sizeof(g_result) - used,
                                         "group=%u id=%08X count=%u token=%08X\n",
                                         group_index, string_id, marker_count, marker_token);
                if (marker_count == 0 || marker_count > 256u ||
                    !readable_range(pages + page, sizeof(void *)) || !pages[page])
                    continue;
                markers = pages[page] + (uint64_t)marker_token * 4u;
                if (!readable_range(markers, (size_t)marker_count * 52u)) continue;
                for (marker_index = 0; marker_index < marker_count &&
                                       used < sizeof(g_result) - 180u; ++marker_index) {
                    uint8_t *marker = markers + (size_t)marker_index * 52u;
                    used += (size_t)snprintf(
                        g_result + used, sizeof(g_result) - used,
                        " marker=%u region=%d permutation=%d node=%d flags=%02X "
                        "position=%.6f,%.6f,%.6f rotation=%.6f,%.6f,%.6f,%.6f "
                        "scale=%.6f direction=%.6f,%.6f,%.6f\n",
                        marker_index,
                        (int)*(int16_t *)(marker + 0), (int)*(int16_t *)(marker + 2),
                        (int)*(int16_t *)(marker + 4), marker[6],
                        *(float *)(marker + 8), *(float *)(marker + 12),
                        *(float *)(marker + 16), *(float *)(marker + 20),
                        *(float *)(marker + 24), *(float *)(marker + 28),
                        *(float *)(marker + 32), *(float *)(marker + 36),
                        *(float *)(marker + 40), *(float *)(marker + 44),
                        *(float *)(marker + 48));
                }
            }
        }
    } else if (command.kind == COMMAND_LISTTAGS) {
        uint32_t count = 0;
        uint8_t *entries = tag_table_entries(&count);
        size_t used = 0;
        uint32_t i;
        uint32_t matched = 0;
        if (!entries) {
            snprintf(g_result, sizeof(g_result), "error tag_table_unavailable\n");
        } else {
            used = (size_t)snprintf(g_result, sizeof(g_result), "ok listtags group=%08X count=%u datums=", (uint32_t)command.a, count);
            for (i = 0; i < count && used < sizeof(g_result) - 48; ++i) {
                uint8_t *entry = entries + (size_t)i * 48u;
                uint16_t salt = *(uint16_t *)entry;
                uint8_t flags = *(uint8_t *)(entry + 2);
                uint32_t group = *(uint32_t *)(entry + 4);
                uint32_t parent_group = *(uint32_t *)(entry + 8);
                uint32_t grandparent_group = *(uint32_t *)(entry + 12);
                uint32_t token = *(uint32_t *)(entry + 0x1C);
                const char *name = *(const char **)(entry + 0x10);
                uint8_t **pages = (uint8_t **)(g_blam + 0x2C2CCC0);
                uint8_t *definition = pages[token >> 28] + (uint64_t)token * 4;
                int32_t datum;
                if (!salt || (command.a != 0 && group != (uint32_t)command.a &&
                              parent_group != (uint32_t)command.a && grandparent_group != (uint32_t)command.a)) continue;
                datum = (int32_t)(((uint32_t)salt << 16) | i);
                used += (size_t)snprintf(g_result + used, sizeof(g_result) - used,
                                         "%s%d:%08X:%02X:%08X:%p:%s", matched ? "," : "", datum, group, flags,
                                         token, definition, valid_tag_name(name) ? name : "-");
                ++matched;
            }
            snprintf(g_result + used, sizeof(g_result) - used, " matched=%u\n", matched);
        }
    } else if (command.kind == COMMAND_SEATDUMP) {
        uint32_t count = 0;
        uint8_t *seats = resolve_seats(command.a, &count);
        size_t used = 0;
        uint32_t i;
        if (!seats || command.b < 0 || (uint32_t)command.b >= count) {
            snprintf(g_result, sizeof(g_result), "error seatdump invalid tag=%d seat=%d count=%u\n", command.a, command.b, count);
        } else {
            uint8_t *seat = seats + (size_t)command.b * 328u;
            used = (size_t)snprintf(g_result, sizeof(g_result), "ok seatdump tag=%d seat=%d bytes=", command.a, command.b);
            for (i = 0; i < 328 && used < sizeof(g_result) - 4; ++i) {
                used += (size_t)snprintf(g_result + used, sizeof(g_result) - used, "%02X", seat[i]);
            }
            snprintf(g_result + used, sizeof(g_result) - used, "\n");
        }
    } else if (command.kind == COMMAND_MEMDUMP) {
        size_t used = 0;
        int32_t i;
        uint8_t *memory = (uint8_t *)command.address;
        if (command.c <= 0 || command.c > 1024 || !readable_range(memory, (size_t)command.c)) {
            snprintf(g_result, sizeof(g_result), "error memdump unreadable address=%p size=%d\n", memory, command.c);
        } else {
            used = (size_t)snprintf(g_result, sizeof(g_result), "ok memdump address=%p size=%d bytes=", memory, command.c);
            for (i = 0; i < command.c && used < sizeof(g_result) - 4; ++i) {
                used += (size_t)snprintf(g_result + used, sizeof(g_result) - used, "%02X", memory[i]);
            }
            snprintf(g_result + used, sizeof(g_result) - used, "\n");
        }
    } else if (command.kind == COMMAND_TAGENTRY) {
        uint32_t count = 0;
        uint8_t *entries = tag_table_entries(&count);
        uint32_t index = (uint32_t)command.a & 0xFFFFu;
        size_t used = 0;
        uint32_t i;
        if (!entries || index >= count) {
            snprintf(g_result, sizeof(g_result), "error tagentry invalid datum=%d count=%u\n", command.a, count);
        } else {
            uint8_t *entry = entries + (size_t)index * 48u;
            used = (size_t)snprintf(g_result, sizeof(g_result), "ok tagentry datum=%d address=%p bytes=", command.a, entry);
            for (i = 0; i < 48 && used < sizeof(g_result) - 4; ++i) {
                used += (size_t)snprintf(g_result + used, sizeof(g_result) - used, "%02X", entry[i]);
            }
            snprintf(g_result + used, sizeof(g_result) - used, "\n");
        }
    } else if (command.kind == COMMAND_LOADTAG) {
        int32_t loaded = -1;
        uint32_t count = 0;
        uint8_t *entries;
        uint32_t index;
        int verified = 0;
        /* Cette fonction est le chargeur de tags utilise par le moteur lui-meme.
         * Elle permet de rendre resident un tag original qui n'appartient pas
         * au scenario courant (par exemple un Marine dans La Bibliotheque),
         * sans modifier le scenario ni ses blocs persistants. */
        loaded = ((tag_load_fn)(g_blam + 0x197F60))((uint32_t)command.a,
                                                    command.path);
        entries = tag_table_entries(&count);
        index = (uint32_t)loaded & 0xFFFFu;
        if (loaded != -1 && entries && index < count) {
            uint8_t *entry = entries + (size_t)index * 48u;
            uint16_t salt = *(uint16_t *)entry;
            uint32_t group = *(uint32_t *)(entry + 4);
            const char *name = *(const char **)(entry + 0x10);
            int32_t actual = (int32_t)(((uint32_t)salt << 16) | index);
            verified = salt && actual == loaded && group == (uint32_t)command.a &&
                       tag_name_equals(name, command.path);
        }
        if (!verified) {
            snprintf(g_result, sizeof(g_result),
                     "error loadtag_failed group=%08X path=%s returned=%d\n",
                     (uint32_t)command.a, command.path, loaded);
        } else {
            snprintf(g_result, sizeof(g_result),
                     "ok loadtag datum=%d group=%08X path=%s original_content=1 scenario_mutation=0\n",
                     loaded, (uint32_t)command.a, command.path);
        }
    } else if (command.kind == COMMAND_FINDTAG) {
        uint32_t count = 0;
        uint8_t *entries = tag_table_entries(&count);
        uint32_t i;
        int32_t found = -1;
        const char *found_name = NULL;
        if (!entries) {
            snprintf(g_result, sizeof(g_result), "error tag_table_unavailable\n");
        } else {
            for (i = 0; i < count; ++i) {
                uint8_t *entry = entries + (size_t)i * 48u;
                uint16_t salt = *(uint16_t *)entry;
                uint32_t group = *(uint32_t *)(entry + 4);
                const char *name = *(const char **)(entry + 0x10);
                if (!salt || group != (uint32_t)command.a) continue;
                if (tag_name_equals(name, command.path)) {
                    found = (int32_t)(((uint32_t)salt << 16) | i);
                    found_name = name;
                    break;
                }
            }
            if (found == -1) snprintf(g_result, sizeof(g_result), "error tag_not_found group=%08X path=%s\n", (uint32_t)command.a, command.path);
            else snprintf(g_result, sizeof(g_result), "ok findtag datum=%d group=%08X path=%s\n", found, (uint32_t)command.a, found_name);
        }
    } else {
        snprintf(g_result, sizeof(g_result), "error invalid_queued_command\n");
    }
dispatch_complete:
    if (command.kind==COMMAND_AI_SPAWN_AT) ai1_trace(g_result);
    {
        LONG internal = InterlockedExchange(&g_internal_dispatch, 0);
        LONG previous = InterlockedCompareExchange(
            &g_state,
            internal ? NATIVE_STATE_IDLE : NATIVE_STATE_RESULT_READY,
            NATIVE_STATE_EXECUTING);
        if (previous == NATIVE_STATE_EXECUTING_QUIESCED)
            finish_quiesced_owner(NATIVE_STATE_EXECUTING_QUIESCED);
    }
}

static int parse_command(const char *text, struct native_command *out) {
    char operation[32];
    int parsed_count;
    memset(out, 0, sizeof(*out));
    if (sscanf(text, "%31s", operation) != 1) return 0;
    if (!strcmp(operation,"ai_probe")) out->kind=COMMAND_AI_PROBE;
    else if (!strcmp(operation,"ai_follow_watch")) out->kind=COMMAND_AI_FOLLOW_WATCH;
    else if (!strcmp(operation,"ai_place_group")) {
        out->a = 2;
        out->c = -1;
        parsed_count = sscanf(text,"%*s %255s %i %i",out->path,&out->b,&out->c);
        if (parsed_count == 2 || parsed_count == 3) out->kind=COMMAND_AI_PLACE_GROUP;
    }
    else if (!strcmp(operation,"ai_place_squad")) {
        out->a = 1;
        out->c = -1;
        parsed_count = sscanf(text,"%*s %255s %i %i",out->path,&out->b,&out->c);
        if (parsed_count == 2 || parsed_count == 3) out->kind=COMMAND_AI_PLACE_GROUP;
    }
    else if (!strcmp(operation,"ai_attach") &&
             sscanf(text,"%*s %i %i",&out->a,&out->b)==2) out->kind=COMMAND_AI_ATTACH;
    else if (!strcmp(operation,"ai_spawn_at")) {
        parsed_count = sscanf(text,"%*s %i %i %i %i %i %f %f %f %f %f %f %f %f %f %i",
                              &out->a, &out->b, &out->d, &out->c, &out->e,
                              &out->x, &out->y, &out->z,
                              &out->fx, &out->fy, &out->fz,
                              &out->ux, &out->uy, &out->uz, &out->marine_mode);
        if (parsed_count == 14 || parsed_count == 15) {
            out->kind=COMMAND_AI_SPAWN_AT;
        } else {
            out->e = AI_LOADOUT_KEEP_DEFAULT;
            if (sscanf(text,"%*s %i %i %i %i %f %f %f %f %f %f %f %f %f",
                       &out->a, &out->b, &out->d, &out->c,
                       &out->x, &out->y, &out->z,
                       &out->fx, &out->fy, &out->fz,
                       &out->ux, &out->uy, &out->uz) == 13) {
                out->kind=COMMAND_AI_SPAWN_AT;
            }
        }
    }
    else if (!strcmp(operation,"health") && sscanf(text,"%*s %i %f %f",&out->a,&out->x,&out->y)==3) out->kind=COMMAND_HEALTH;
    else if (!strcmp(operation,"damageproof") &&
             sscanf(text,"%*s %i %i",&out->a,&out->b)==2) out->kind=COMMAND_DAMAGEPROOF;
    else if (!strcmp(operation,"refill_ammo") &&
             sscanf(text,"%*s %i",&out->a)==1) out->kind=COMMAND_REFILL_AMMO;
    else if (!strcmp(operation,"player_powers") &&
             sscanf(text,"%*s %i %i %i",&out->a,&out->b,&out->c)==3)
        out->kind=COMMAND_PLAYER_POWERS;
    else if (!strcmp(operation,"phase_forward") &&
             sscanf(text,"%*s %i %f %i",&out->a,&out->x,&out->b)==3) out->kind=COMMAND_PHASE_FORWARD;
    else if (!strcmp(operation,"noclip") &&
             sscanf(text,"%*s %i %i %i",&out->a,&out->b,&out->c)==3) out->kind=COMMAND_NOCLIP;
    else if (!strcmp(operation,"noclip_recover") &&
             sscanf(text,"%*s %i %i",&out->a,&out->b)==2) out->kind=COMMAND_NOCLIP_RECOVER;
    else if (!strcmp(operation,"spawn")) {
        int count = sscanf(text,"%*s %i %i %f %f",&out->a,&out->b,&out->x,&out->y);
        if (count >= 3) {
            if (count == 3) out->y = 0.75f;
            out->kind=COMMAND_SPAWN;
        }
    }
    else if (!strcmp(operation,"escort_status") && sscanf(text,"%*s %i %i %i %i %i",&out->a,&out->b,&out->c,&out->d,&out->e)>=4) out->kind=COMMAND_ESCORT_STATUS;
    else if (!strcmp(operation,"observer") &&
             sscanf(text,"%*s %i %i",&out->a,&out->b)==2 && (out->b==0 || out->b==1)) out->kind=COMMAND_OBSERVER;
    else if (!strcmp(operation,"ai_control") &&
             sscanf(text,"%*s %i %i",&out->a,&out->b)==2 && (out->b==0 || out->b==1)) out->kind=COMMAND_AI_CONTROL;
    else if (!strcmp(operation,"item_spawn") &&
             sscanf(text,"%*s %i %i %f %f %f %f %f %f %f %f %f",
                    &out->a,&out->b,&out->x,&out->y,&out->z,
                    &out->fx,&out->fy,&out->fz,&out->ux,&out->uy,&out->uz)==11) out->kind=COMMAND_SPAWN_AT;
    else if (!strcmp(operation,"spawn_at") &&
             sscanf(text,"%*s %i %f %f %f %f %f %f %f %f %f",
                    &out->b, &out->x, &out->y, &out->z,
                    &out->fx, &out->fy, &out->fz,
                    &out->ux, &out->uy, &out->uz) == 10) out->kind=COMMAND_SPAWN_AT;
    else if (!strcmp(operation,"spawn_enter") &&
             sscanf(text,"%*s %i %i %i %f %f %f %f %f %f %f %f %f",
                    &out->a, &out->b, &out->c,
                    &out->x, &out->y, &out->z,
                    &out->fx, &out->fy, &out->fz,
                     &out->ux, &out->uy, &out->uz) == 12) out->kind=COMMAND_SPAWN_ENTER;
    else if (!strcmp(operation,"spawn_enter_relative") &&
             sscanf(text,"%*s %i %i %i %f %f",
                    &out->a, &out->b, &out->c, &out->x, &out->y) == 5)
        out->kind=COMMAND_SPAWN_ENTER_RELATIVE;
    else if (!strcmp(operation,"spawn_relative") &&
             sscanf(text,"%*s %i %i %f %f",
                    &out->a, &out->b, &out->x, &out->y) == 4)
        out->kind=COMMAND_SPAWN_RELATIVE;
    else if (!strcmp(operation,"velocity") &&
             sscanf(text,"%*s %i %f %f %f",&out->a,&out->x,&out->y,&out->z)==4) out->kind=COMMAND_VELOCITY;
    else if (!strcmp(operation,"enterable") && sscanf(text,"%*s %i %i",&out->a,&out->b)==2) out->kind=COMMAND_ENTERABLE;
    else if (!strcmp(operation,"enter") && sscanf(text,"%*s %i %i %i",&out->a,&out->b,&out->c)==3) out->kind=COMMAND_ENTER;
    else if (!strcmp(operation,"enter_immediate") &&
             sscanf(text,"%*s %i %i %i %i",&out->a,&out->b,&out->c,&out->d)==4) out->kind=COMMAND_ENTER_IMMEDIATE;
    else if (!strcmp(operation,"parent") && sscanf(text,"%*s %i",&out->a)==1) out->kind=COMMAND_PARENT;
    else if (!strcmp(operation,"exit_force") && sscanf(text,"%*s %i",&out->a)==1) out->kind=COMMAND_EXIT_FORCE;
    else if (!strcmp(operation,"seatinfo") && sscanf(text,"%*s %i",&out->a)==1) out->kind=COMMAND_SEATINFO;
    else if (!strcmp(operation,"unlockseats") && sscanf(text,"%*s %i",&out->a)==1) out->kind=COMMAND_UNLOCKSEATS;
    else if (!strcmp(operation,"tagdump") &&
             sscanf(text,"%*s %i %i %i",&out->a,&out->b,&out->c)==3)
        out->kind=COMMAND_TAGDUMP;
    else if (!strcmp(operation,"tagblock") &&
             sscanf(text,"%*s %i %i %i",&out->a,&out->b,&out->c)==3)
        out->kind=COMMAND_TAGBLOCK;
    else if (!strcmp(operation,"modelmarkers") &&
             sscanf(text,"%*s %i",&out->a)==1)
        out->kind=COMMAND_MODELMARKERS;
    else if (!strcmp(operation,"attachspawn")) {
        unsigned int parent_marker, child_marker;
        out->e = 0;
        parsed_count = sscanf(text,"%*s %i %i %x %x %i",&out->a,&out->b,
                              &parent_marker,&child_marker,&out->e);
        if (parsed_count == 4 || parsed_count == 5) {
            out->c=(int32_t)parent_marker;
            out->d=(int32_t)child_marker;
            out->kind=COMMAND_ATTACHSPAWN;
        }
    }
    else if (!strcmp(operation,"weapon_fire") &&
             sscanf(text,"%*s %i %i",&out->a,&out->b)==2)
        out->kind=COMMAND_WEAPON_FIRE;
    else if (!strcmp(operation,"object_delete") &&
             sscanf(text,"%*s %i",&out->a)==1)
        out->kind=COMMAND_OBJECT_DELETE;
    else if (!strcmp(operation,"pelican_arm")) {
        out->e = -1;
        parsed_count = sscanf(text,"%*s %i %i %i %i %i",
                              &out->a,&out->b,&out->c,&out->d,&out->e);
        if (parsed_count == 4 || parsed_count == 5)
            out->kind=COMMAND_PELICAN_ARM;
    }
    else if (!strcmp(operation,"pelican_disarm"))
        out->kind=COMMAND_PELICAN_DISARM;
    else if (!strcmp(operation,"pelican_status"))
        out->kind=COMMAND_PELICAN_STATUS;
    else if (!strcmp(operation,"pelican_patch") &&
             sscanf(text,"%*s %i %i",&out->a,&out->b)==2)
        out->kind=COMMAND_PELICAN_PATCH;
    else if (!strcmp(operation,"pelican_boost") &&
             sscanf(text,"%*s %i",&out->a)==1)
        out->kind=COMMAND_PELICAN_BOOST;
    else if (!strcmp(operation,"pelican_arm_builtin") &&
             sscanf(text,"%*s %i %i",&out->a,&out->b)==2)
        out->kind=COMMAND_PELICAN_ARM_BUILTIN;
#ifdef HALOSANDBOX_RUNTIME_TEST
    else if (!strcmp(operation,"pelican_runtime_test_arm"))
        out->kind=COMMAND_PELICAN_RUNTIME_TEST_ARM;
#endif
    else if (!strcmp(operation,"listtags")) {
        unsigned int group;
        if (sscanf(text,"%*s %x",&group)==1) { out->a=(int32_t)group; out->kind=COMMAND_LISTTAGS; }
    }
    else if (!strcmp(operation,"seatdump") && sscanf(text,"%*s %i %i",&out->a,&out->b)==2) out->kind=COMMAND_SEATDUMP;
    else if (!strcmp(operation,"memdump")) {
        unsigned long long address;
        if (sscanf(text,"%*s %llx %i",&address,&out->c)==2) {
            out->address=(uintptr_t)address; out->kind=COMMAND_MEMDUMP;
        }
    }
    else if (!strcmp(operation,"tagentry") && sscanf(text,"%*s %i",&out->a)==1) out->kind=COMMAND_TAGENTRY;
    else if (!strcmp(operation,"findtag")) {
        unsigned int group;
        if (sscanf(text,"%*s %x %255s",&group,out->path)==2) {
            out->a=(int32_t)group; out->kind=COMMAND_FINDTAG;
        }
    }
    else if (!strcmp(operation,"loadtag")) {
        unsigned int group;
        if (sscanf(text,"%*s %x %255s",&group,out->path)==2 &&
            (group == 0x62697064u || group == 0x626C6F63u ||
             group == 0x65716970u || group == 0x76656869u ||
              group == 0x77656170u || group == 0x63686172u ||
              group == TAG_GROUP_EFFECT) &&
            valid_tag_name(out->path)) {
            out->a=(int32_t)group; out->kind=COMMAND_LOADTAG;
        }
    }
    else if (!strcmp(operation,"origin") && sscanf(text,"%*s %i",&out->a)==1) out->kind=COMMAND_ORIGIN;
    else if (!strcmp(operation,"teleport") &&
             sscanf(text,"%*s %i %f %f %f",&out->a,&out->x,&out->y,&out->z)==4) out->kind=COMMAND_TELEPORT;
    else if (!strcmp(operation,"seatflags")) {
        unsigned int set_flags, clear_flags;
        if (sscanf(text,"%*s %i %i %x %x",&out->a,&out->b,&set_flags,&clear_flags)==4) {
            out->c=(int32_t)set_flags; out->d=(int32_t)clear_flags; out->kind=COMMAND_SEATFLAGS;
        }
    }
    return out->kind != COMMAND_NONE;
}

/* package.loadlib transmet un lua_State*, mais notre DLL n'importe pas Lua.
 * UE4SS exporte son petit wrapper LuaMadeSimple; ses methodes ne dependent
 * que du premier champ (lua_State*). Les noms sont resolus dynamiquement afin
 * de ne rajouter aucune DLL ni aucune I/O au chemin camera/audio. */
typedef double (__fastcall *ue4ss_lua_get_number_fn)(const void *, int);
typedef void (__fastcall *ue4ss_lua_set_integer_fn)(const void *, int64_t);

struct ue4ss_lua_view {
    void *state;
};

static ue4ss_lua_get_number_fn g_ue4ss_lua_get_number;
static ue4ss_lua_set_integer_fn g_ue4ss_lua_set_integer;
static volatile LONG g_ue4ss_lua_bridge_resolved;

static int resolve_ue4ss_lua_bridge(void) {
    HMODULE module;
    if (InterlockedCompareExchange(
            &g_ue4ss_lua_bridge_resolved, 0, 0) != 0)
        return g_ue4ss_lua_get_number && g_ue4ss_lua_set_integer;
    module = GetModuleHandleW(L"UE4SS.dll");
    if (!module) return 0;
    g_ue4ss_lua_get_number = (ue4ss_lua_get_number_fn)GetProcAddress(
        module,
        "?get_number@Lua@LuaMadeSimple@RC@@QEBANH@Z");
    g_ue4ss_lua_set_integer = (ue4ss_lua_set_integer_fn)GetProcAddress(
        module,
        "?set_integer@Lua@LuaMadeSimple@RC@@QEBAX_J@Z");
    MemoryBarrier();
    InterlockedExchange(&g_ue4ss_lua_bridge_resolved, 1);
    return g_ue4ss_lua_get_number && g_ue4ss_lua_set_integer;
}

/* Appel direct leger depuis le game thread Lua :
 *   nativePelicanBridge(vehicle, localForward, localRight, localUp)
 * Retour : activeAudio, sequenceAudio, moduleAudio, weaponAudio.
 * LuaMadeSimple consomme la valeur lue; les arguments doivent être lus aux
 * indices 1, 2, 3 et 4. */
__declspec(dllexport) int halosandbox_pelican_bridge(void *lua_state) {
    struct ue4ss_lua_view lua;
    double raw_vehicle;
    double local_forward;
    double local_right;
    double local_up;
    int32_t vehicle;
    int accepted = 0;
    LONG sequence;
    if (!lua_state || !resolve_ue4ss_lua_bridge()) return 0;
    lua.state = lua_state;
    raw_vehicle = g_ue4ss_lua_get_number(&lua, 1);
    local_forward = g_ue4ss_lua_get_number(&lua, 2);
    local_right = g_ue4ss_lua_get_number(&lua, 3);
    local_up = g_ue4ss_lua_get_number(&lua, 4);
    if (isfinite(raw_vehicle) && raw_vehicle >= (double)INT32_MIN &&
        raw_vehicle <= (double)INT32_MAX &&
        isfinite(local_forward) && isfinite(local_right) &&
        isfinite(local_up) && fabs(local_forward) <= 10000.0 &&
        fabs(local_right) <= 10000.0 && fabs(local_up) <= 10000.0) {
        vehicle = (int32_t)raw_vehicle;
        InterlockedIncrement(&g_pelican_camera_target.sequence);
        MemoryBarrier();
        g_pelican_camera_target.vehicle = vehicle;
        g_pelican_camera_target.local_target[0] = (float)local_forward;
        g_pelican_camera_target.local_target[1] = (float)local_right;
        g_pelican_camera_target.local_target[2] = (float)local_up;
        g_pelican_camera_target.updated_at_ms = GetTickCount64();
        InterlockedExchange(&g_pelican_camera_target.valid,
                            vehicle == g_pelican_vehicle && vehicle != -1);
        MemoryBarrier();
        InterlockedIncrement(&g_pelican_camera_target.sequence);
        ++g_pelican_camera_target.accepted_samples;
        accepted = 1;
    }
    if (!accepted) ++g_pelican_camera_target.rejected_samples;

    sequence = InterlockedCompareExchange(
        &g_pelican_audio_signal_sequence, 0, 0);
    g_ue4ss_lua_set_integer(&lua, (int64_t)InterlockedCompareExchange(
        &g_pelican_audio_signal_state, 0, 0));
    g_ue4ss_lua_set_integer(&lua, (int64_t)sequence);
    g_ue4ss_lua_set_integer(&lua, (int64_t)g_pelican_audio_module);
    g_ue4ss_lua_set_integer(&lua, (int64_t)g_pelican_audio_weapon);
    return 4;
}

/* Pont audio dedie : aucun argument camera, aucune publication de cible et
 * aucun acces aux objets Blam. Lua ne lit ici que quatre atomiques deja
 * produits par le vrai serial de tir sur le thread de simulation. */
__declspec(dllexport) int halosandbox_pelican_audio_bridge(void *lua_state) {
    struct ue4ss_lua_view lua;
    LONG sequence;
    if (!lua_state || !resolve_ue4ss_lua_bridge()) return 0;
    lua.state = lua_state;
    sequence = InterlockedCompareExchange(
        &g_pelican_audio_signal_sequence, 0, 0);
    g_ue4ss_lua_set_integer(&lua, (int64_t)InterlockedCompareExchange(
        &g_pelican_audio_signal_state, 0, 0));
    g_ue4ss_lua_set_integer(&lua, (int64_t)sequence);
    g_ue4ss_lua_set_integer(&lua, (int64_t)g_pelican_audio_module);
    g_ue4ss_lua_set_integer(&lua, (int64_t)g_pelican_audio_weapon);
    return 4;
}

/* Appele directement par Lua avant LoadMap/InitGameState. Aucun acces Blam :
 * on empeche seulement les workers de programmer un nouveau tick avec les
 * datums de la mission qui est en train d'etre detruite. */
__declspec(dllexport) int halosandbox_menu_open(void *lua_state) {
    (void)lua_state;
    InterlockedExchange(&g_menu_state_cached, 1);
    InterlockedExchange(&g_menu_state_direct, 1);
    return 0;
}

__declspec(dllexport) int halosandbox_menu_closed(void *lua_state) {
    (void)lua_state;
    InterlockedExchange(&g_menu_state_cached, 0);
    InterlockedExchange(&g_menu_state_direct, 1);
    return 0;
}

/* Appel direct de Lua sur game_MAIN_THREAD : aucune suspension/injection de
 * thread n'est necessaire, donc aucune saccade periodique du noclip. */
__declspec(dllexport) int halosandbox_noclip_bridge(void *lua_state) {
    (void)lua_state;
    if (!g_blam ||
        InterlockedCompareExchange(&g_process_detaching, 0, 0) != 0 ||
        InterlockedCompareExchange(&g_noclip_enabled, 0, 0) == 0 ||
        InterlockedCompareExchange(&g_external_pending, 0, 0) != 0 ||
        InterlockedCompareExchange(&g_state, NATIVE_STATE_IDLE,
                                   NATIVE_STATE_IDLE) != NATIVE_STATE_IDLE)
        return 0;
    (void)noclip_tick();
    return 0;
}

/* Ces champs ne sont jamais des objets Unreal et leur remise à zéro ne fait
 * aucun appel Blam. Elle doit néanmoins attendre le propriétaire de la
 * commande courante : AI_FOLLOW_WATCH et les relais Pelican les consultent
 * sur game_MAIN_THREAD. */
static void reset_mission_caches_no_blam(void) {
    MemoryBarrier();
    pelican_stop_audio_proxy();
    /* Refaire cette publication APRES l'attente du proprietaire. Un ARM qui
     * etait deja EXECUTING au debut du quiesce a pu republier armed=1 avant
     * de rendre la main; aucun worker ne doit alors reutiliser ses datums. */
    InterlockedExchange(&g_pelican_armed, 0);
    InterlockedExchange(&g_pelican_requested_primary, 0);
    InterlockedExchange(&g_pelican_applied_primary, 0);
    InterlockedExchange(&g_pelican_driver_present, 0);
    InterlockedExchange(&g_pelican_local_input_hold_ticks, 0);
    InterlockedExchange(&g_pelican_pilot_absence_ticks, 0);
    InterlockedExchange(&g_pelican_last_driver_datum, -1);
    InterlockedExchange(&g_pelican_last_target_ok, 0);
    InterlockedExchange(&g_pelican_last_aim_ok, 0);
    InterlockedExchange(&g_pelican_last_release_reason, 0);
    InterlockedExchange(&g_pelican_release_count, 0);
    pelican_reset_validation_diagnostics();
    g_pelican_last_release_vehicle = -1;
    g_pelican_last_release_module = -1;
    g_pelican_last_release_weapon = -1;
    g_pelican_local_pilot = -1;
    g_pelican_vehicle = -1;
    g_pelican_chin_module = -1;
    g_pelican_chin_weapon = -1;
    g_pelican_audio_module = -1;
    g_pelican_audio_weapon = -1;
    g_pelican_aux_impact_projectile_tag = -1;
    g_pelican_damage_owner_pilot = -1;
    g_pelican_damage_owner_module = -1;
    memset(g_pelican_damage_owner_bytes, 0,
           sizeof(g_pelican_damage_owner_bytes));
    InterlockedExchange(&g_pelican_damage_owner_verified, 0);
    pelican_direct_gate_clear(-1);
    g_pelican_completed_shots = 0;
    g_pelican_aux_impact_created = 0;
    g_pelican_aux_impact_failures = 0;
    g_pelican_last_shot_serial = 0;
    InterlockedExchange(&g_pelican_fire_interval_ms, 100);
    /* Les snapshots de tags restent vivants jusqu'au retour sur le thread Lua.
     * halosandbox_quiesce les restaure avant que HaloSimulation_tag_release ne
     * libere les dependances de la mission. Les oublier ici reproduisait
     * l'assert observe dans le dump 18:56. */
#ifdef HALOSANDBOX_RUNTIME_TEST
    memset(&g_pelican_runtime_shot_probe, 0,
           sizeof(g_pelican_runtime_shot_probe));
    g_pelican_runtime_shot_probe.instance_pre = -1;
    g_pelican_runtime_shot_probe.instance_post = -1;
#endif
    g_damageproof_unit = -1;
    g_damageproof_parent = -1;
    g_bottomless_player_datum = -1;
    g_bottomless_player_table = NULL;
    g_bottomless_player_entries = NULL;
    g_bottomless_original_trait = 1u;
    g_bottomless_snapshot_valid = 0;
    g_noclip_datum = -1;
    g_noclip_protected_vehicle = -1;
    InterlockedExchange(&g_noclip_anchor_valid, 0);
    g_noclip_anchor_scenario = NULL;
    memset(g_noclip_anchor_position, 0, sizeof(g_noclip_anchor_position));
    memset(g_noclip_anchor_forward, 0, sizeof(g_noclip_anchor_forward));
    memset(g_noclip_anchor_up, 0, sizeof(g_noclip_anchor_up));
    /* Oublier le snapshot de l'ancien scénario sans tenter de le restaurer
     * pendant son teardown. Une nouvelle activation prendra un snapshot neuf. */
    memset(&g_zone_limits, 0, sizeof(g_zone_limits));
    g_last_main_thread_id = 0;
    g_last_main_thread_candidates = 0;
    memset(g_ai_follow_watch, 0, sizeof(g_ai_follow_watch));
    g_ai_follow_watch_cursor = 0;
    memset(&g_transient_enemy_squads[2],0,sizeof(g_transient_enemy_squads[2]));
    MemoryBarrier();
}

/* Appelé uniquement après avoir acquis l'état 8. Publier IDLE en dernier
 * garantit qu'aucun nouveau producteur ne peut voir des caches à moitié
 * réinitialisés. */
static void complete_quiesce_reset_locked(void) {
    InterlockedExchange(&g_internal_dispatch, 0);
    reset_mission_caches_no_blam();
    /* L'ancien propriétaire peut avoir écrit son résultat après que Lua a
     * commencé la transition. Le tronquer sous l'état 8 interdit qu'il soit
     * pris pour la réponse de la première commande de la nouvelle mission. */
    write_result("");
    InterlockedExchange(&g_external_pending, 0);
    MemoryBarrier();
    InterlockedExchange(&g_state, NATIVE_STATE_IDLE);
}

static void finish_quiesced_owner(LONG expected_state) {
    if (InterlockedCompareExchange(
            &g_state, NATIVE_STATE_QUIESCE_RESETTING,
            expected_state) == expected_state)
        complete_quiesce_reset_locked();
}

/* LoadMap ne doit pas commencer à détruire les tables Blam tant que leur
 * propriétaire exécute encore une commande. Les marqueurs évitent tout
 * deadlock de verrou; cette attente ne tient aucun thread suspendu et le
 * propriétaire publie IDLE en dernier. */
static void wait_for_quiesce_completion(void) {
    unsigned int yields = 0;
    while (InterlockedCompareExchange(
               &g_state, NATIVE_STATE_IDLE, NATIVE_STATE_IDLE) !=
           NATIVE_STATE_IDLE) {
        if (InterlockedCompareExchange(&g_process_detaching, 0, 0) != 0)
            return;
        Sleep(yields++ < 64u ? 0u : 1u);
    }
}

/* Cet appel revient sur le thread Lua/game apres l'arret de tous les workers,
 * mais avant que LoadMap/InitGameState poursuive la liberation des tags. Les
 * datums greffes sont donc retires pendant que leurs definitions et leurs
 * compteurs de dependances existent encore. */
static void pelican_restore_tag_patches_before_release(void) {
    if (!pelican_restore_presentation_patch())
        (void)pelican_restore_presentation_patch();
    if (!pelican_restore_impact_patch())
        (void)pelican_restore_impact_patch();
    if (!pelican_restore_tracer_patch())
        (void)pelican_restore_tracer_patch();
    if (!pelican_restore_pitch_patch())
        (void)pelican_restore_pitch_patch();
    if (!pelican_restore_boost_patch())
        (void)pelican_restore_boost_patch();
    pelican_forget_presentation_patch(1);
    pelican_forget_impact_patch(1);
    pelican_forget_tracer_patch(1);
    pelican_forget_pitch_patch(1);
    pelican_forget_boost_patch(1);
    g_pelican_presentation_patch_error = 0;
    g_pelican_impact_patch_error = 0;
    g_pelican_tracer_patch_error = 0;
    g_pelican_pitch_patch_error = 0;
    g_pelican_boost_patch_error = 0;
}

__declspec(dllexport) int halosandbox_quiesce(void *lua_state) {
    LONG pending;
    (void)lua_state;
    /* Invalider d'abord tout producteur préempté avant sa réservation. Même
     * s'il reprend après la fin du reset, son epoch ne correspondra plus et
     * il libérera lui-même l'état 4 sans injecter l'ancienne commande. */
    InterlockedIncrement(&g_dispatch_epoch);
    MemoryBarrier();
    /* Invalide aussi un COMMAND_AMMO_MAINTENANCE deja reserve. Son passage
     * eventuel dans le stub constatera l'epoch differente et ne dereferencera
     * aucun pointeur Blam pendant le teardown. */
    disable_ammo_maintenance();
    InterlockedExchange(&g_pelican_armed, 0);
    InterlockedExchange(&g_pelican_requested_primary, 0);
    InterlockedExchange(&g_pelican_applied_primary, 0);
    InterlockedExchange(&g_pelican_driver_present, 0);
    InterlockedExchange(&g_noclip_enabled, 0);
    /* Empêche les workers internes de réserver un nouvel état pendant le
     * choix du propriétaire chargé de finir la commande en vol. */
    InterlockedExchange(&g_external_pending, 1);
    for (;;) {
        pending = InterlockedCompareExchange(
            &g_state, NATIVE_STATE_IDLE, NATIVE_STATE_IDLE);
        if (pending == NATIVE_STATE_IDLE ||
            pending == NATIVE_STATE_RESULT_READY) {
            if (InterlockedCompareExchange(
                    &g_state, NATIVE_STATE_QUIESCE_RESETTING,
                    pending) == pending) {
                complete_quiesce_reset_locked();
                break;
            }
            continue;
        }
        if (pending == NATIVE_STATE_RESERVED) {
            if (InterlockedCompareExchange(
                    &g_state, NATIVE_STATE_RESERVED_QUIESCED,
                    NATIVE_STATE_RESERVED) == NATIVE_STATE_RESERVED) {
                wait_for_quiesce_completion();
                break;
            }
            continue;
        }
        if (pending == NATIVE_STATE_QUEUED) {
            if (InterlockedCompareExchange(
                    &g_state, NATIVE_STATE_QUEUED_QUIESCED,
                    NATIVE_STATE_QUEUED) == NATIVE_STATE_QUEUED) {
                wait_for_quiesce_completion();
                break;
            }
            continue;
        }
        if (pending == NATIVE_STATE_EXECUTING) {
            if (InterlockedCompareExchange(
                    &g_state, NATIVE_STATE_EXECUTING_QUIESCED,
                    NATIVE_STATE_EXECUTING) == NATIVE_STATE_EXECUTING) {
                wait_for_quiesce_completion();
                break;
            }
            continue;
        }
        if (pending == NATIVE_STATE_RESULT_WRITING) {
            if (InterlockedCompareExchange(
                    &g_state, NATIVE_STATE_RESULT_WRITING_QUIESCED,
                    NATIVE_STATE_RESULT_WRITING) == NATIVE_STATE_RESULT_WRITING) {
                wait_for_quiesce_completion();
                break;
            }
            continue;
        }
        /* Un autre appel quiesce a déjà transféré le reset au propriétaire. */
        wait_for_quiesce_completion();
        break;
    }
    pelican_restore_tag_patches_before_release();
    return 0;
}

__declspec(dllexport) int halosandbox_exec(void *lua_state) {
    wchar_t command_path[MAX_PATH];
    char text[512], error[192];
    struct native_command parsed;
    LONG dispatch_epoch;
    HANDLE worker;
    (void)lua_state;
    sibling_path(command_path, MAX_PATH, L"native_command.txt");
    if (!read_text(command_path,text,sizeof(text))) { write_result("error command_file_missing\n"); return 0; }
    {
        char verb[40]={0};
        sscanf(text,"%39s",verb);
        if (strcmp(verb,"probe") && strcmp(verb,"controller_start") &&
            strcmp(verb,"ai_spawn_at") && strcmp(verb,"ai_follow_watch") && strcmp(verb,"origin") &&
            strcmp(verb,"findtag") && strcmp(verb,"loadtag") &&
            strcmp(verb,"observer") && strcmp(verb,"item_spawn") && strcmp(verb,"ai_control") && strcmp(verb,"escort_status")) {
            write_result("error ai1_feature_disabled\n"); return 0;
        }
    }
    pin_module_lifetime();
    if (!validate_module(&g_blam, error, sizeof(error))) {
        snprintf(g_result, sizeof(g_result), "error incompatible %s\n", error);
        write_result(g_result); return 0;
    }
    ai24_install_diagnostic();
    if (InterlockedCompareExchange(&g_worker_started,1,0)==0) {
        worker=CreateThread(NULL,0,result_worker,NULL,0,NULL);
        if (worker) CloseHandle(worker);
        else InterlockedExchange(&g_worker_started, 0);
    }

    if (!strncmp(text,"controller_start",16)) {
        write_result("ok xinput_controller_external_relay=1 duplicate_worker=0\n");
        return 0;
    }
    if (!strcmp(text,"probe")) { write_result("ok compatible protocol=70 profile=CU4-AI6 experimental=1 ai_spawn=1 pelican=0 full_mod=0\n"); return 0; }
    if (!strncmp(text,"threadprobe",11)) {
        DWORD tid = find_main_thread();
        snprintf(g_result, sizeof(g_result), "ok threadprobe selected=%lu candidates=%lu\n",
                 (unsigned long)tid, (unsigned long)g_last_main_thread_candidates);
        write_result(g_result);
        return 0;
    }
    if (!parse_command(text,&parsed)) { write_result("error invalid_command\n"); return 0; }
    if ((parsed.kind==COMMAND_FINDTAG || parsed.kind==COMMAND_LOADTAG) &&
        !ai1_path_allowed((uint32_t)parsed.a,parsed.path)) {
        write_result("error ai1_profile_not_enabled\n"); return 0;
    }

    if (((parsed.kind == COMMAND_HEALTH || parsed.kind == COMMAND_DAMAGEPROOF ||
          parsed.kind == COMMAND_REFILL_AMMO || parsed.kind == COMMAND_PLAYER_POWERS ||
          parsed.kind == COMMAND_PHASE_FORWARD ||
          parsed.kind == COMMAND_NOCLIP || parsed.kind == COMMAND_NOCLIP_RECOVER ||
          parsed.kind == COMMAND_SPAWN || parsed.kind == COMMAND_AI_ATTACH ||
          parsed.kind == COMMAND_AI_SPAWN_AT ||
          parsed.kind == COMMAND_ATTACHSPAWN || parsed.kind == COMMAND_WEAPON_FIRE ||
          parsed.kind == COMMAND_OBJECT_DELETE || parsed.kind == COMMAND_PELICAN_ARM ||
          parsed.kind == COMMAND_PELICAN_PATCH ||
          parsed.kind == COMMAND_PELICAN_BOOST ||
          parsed.kind == COMMAND_PELICAN_ARM_BUILTIN ||
          parsed.kind == COMMAND_SPAWN_ENTER || parsed.kind == COMMAND_VELOCITY || parsed.kind == COMMAND_ORIGIN ||
          parsed.kind == COMMAND_TELEPORT || parsed.kind == COMMAND_ENTERABLE ||
          parsed.kind == COMMAND_ENTER || parsed.kind == COMMAND_ENTER_IMMEDIATE ||
          parsed.kind == COMMAND_PARENT || parsed.kind == COMMAND_EXIT_FORCE) && parsed.a == -1) ||
        ((parsed.kind == COMMAND_SPAWN || parsed.kind == COMMAND_SPAWN_AT ||
           parsed.kind == COMMAND_SPAWN_ENTER || parsed.kind == COMMAND_ENTER_IMMEDIATE ||
           parsed.kind == COMMAND_ATTACHSPAWN) && parsed.b == -1)) {
        write_result("error invalid_datum_or_tag\n"); return 0;
    }
    if (parsed.kind == COMMAND_PELICAN_ARM &&
        (parsed.b == -1 || parsed.c == -1 || parsed.d == -1)) {
        write_result("error invalid_pelican_arm_datum\n"); return 0;
    }
    if ((parsed.kind == COMMAND_PELICAN_PATCH ||
         parsed.kind == COMMAND_PELICAN_ARM_BUILTIN) && parsed.b == -1) {
        write_result("error invalid_pelican_builtin_datum_or_tag\n"); return 0;
    }
    if (parsed.kind == COMMAND_ATTACHSPAWN && (parsed.e < 0 || parsed.e > 1)) {
        write_result("error invalid_attach_hidden_flag\n"); return 0;
    }
    if ((parsed.kind == COMMAND_DAMAGEPROOF || parsed.kind == COMMAND_PHASE_FORWARD ||
         parsed.kind == COMMAND_NOCLIP || parsed.kind == COMMAND_NOCLIP_RECOVER) &&
        (parsed.b < 0 || parsed.b > 1)) {
        write_result("error invalid_player_toggle\n"); return 0;
    }
    if (parsed.kind == COMMAND_PLAYER_POWERS &&
        ((parsed.b != -1 && parsed.b != 0 && parsed.b != 1) ||
         (parsed.c != -1 && parsed.c != 0 && parsed.c != 1))) {
        write_result("error invalid_player_powers_toggle\n"); return 0;
    }
    if (parsed.kind == COMMAND_NOCLIP && (parsed.c < 0 || parsed.c > 1)) {
        write_result("error invalid_noclip_damageproof_restore\n"); return 0;
    }
    if (parsed.kind == COMMAND_AI_PLACE_GROUP &&
        (!safe_ai_group_name(parsed.path) || (parsed.a != 1 && parsed.a != 2) ||
         parsed.b < 1 || parsed.b > 16 || parsed.c < -1 || parsed.c > 15)) {
        write_result("error invalid_ai_reference_count_or_team\n"); return 0;
    }
    if (parsed.kind == COMMAND_AI_ATTACH && (parsed.b < 0 || parsed.b > 15)) {
        write_result("error invalid_ai_team\n"); return 0;
    }
    if (parsed.kind == COMMAND_AI_SPAWN_AT &&
        (parsed.marine_mode < 0 || parsed.marine_mode > 2 || (parsed.marine_mode != 0 && parsed.c != 2) ||
         parsed.d == -1 || parsed.c < 0 || parsed.c > 15 ||
         parsed.e < AI_LOADOUT_KEEP_DEFAULT || (parsed.e > AI_LOADOUT_BRUTE && (parsed.e < 100 || parsed.e > 115)))) {
        write_result("error invalid_ai_actor_variant_team_or_loadout\n"); return 0;
    }
    dispatch_epoch = InterlockedCompareExchange(&g_dispatch_epoch, 0, 0);
    /* Une commande du menu a priorité sur les ticks internes. Attendre aussi
     * les états d'écriture 2/6 d'une commande externe précédente : Lua peut
     * dépiler la commande suivante dès que le fichier est fermé. */
    InterlockedExchange(&g_external_pending, 1);
    {
        int wait_ms = 0;
        LONG pending_state;
        /* Une commande interne déjà injectée a besoin du game thread pour
         * finir. Attendre ici bloquerait précisément ce thread pendant 500 ms
         * avant de répondre busy. Seules les très brèves publications du
         * résultat externe précédent méritent une attente bornée; Lua réessaie
         * désormais les autres collisions sans perdre la commande du menu. */
        for (;;) {
            pending_state = InterlockedCompareExchange(&g_state, 0, 0);
            if (pending_state == NATIVE_STATE_IDLE) break;
            if (pending_state != NATIVE_STATE_RESULT_READY &&
                pending_state != NATIVE_STATE_RESULT_WRITING &&
                pending_state != NATIVE_STATE_RESULT_WRITING_QUIESCED)
                break;
            if (wait_ms >= 50) break;
            Sleep(1);
            ++wait_ms;
        }
    }
    if (InterlockedCompareExchange(
            &g_state, NATIVE_STATE_RESERVED,
            NATIVE_STATE_IDLE) != NATIVE_STATE_IDLE) {
        InterlockedExchange(&g_external_pending, 0);
        write_result("error busy\n"); return 0;
    }
    if (dispatch_epoch !=
        InterlockedCompareExchange(&g_dispatch_epoch, 0, 0)) {
        abandon_reserved_state();
        InterlockedExchange(&g_external_pending, 0);
        return 0;
    }
    parsed.dispatch_epoch = dispatch_epoch;
    g_command=parsed;
    MemoryBarrier();
    if (InterlockedCompareExchange(
            &g_state, NATIVE_STATE_QUEUED,
            NATIVE_STATE_RESERVED) != NATIVE_STATE_RESERVED) {
        InterlockedExchange(&g_external_pending, 0);
        finish_quiesced_owner(NATIVE_STATE_RESERVED_QUIESCED);
        return 0;
    }
    if (!dispatch_once(error, sizeof(error))) {
        /* Garder l'état 1/10 pendant l'écriture : une nouvelle commande ne
         * peut pas effacer puis se faire écraser par cet ancien résultat. */
        snprintf(g_result, sizeof(g_result), "error dispatch %s\n", error);
        write_result(g_result);
        abandon_queued_state();
        InterlockedExchange(&g_external_pending, 0);
        return 0;
    }
    InterlockedExchange(&g_external_pending, 0);
    /* Le fichier a deja ete vide par Lua. Ne pas ecrire "queued" ici : le
     * game thread peut terminer tres vite et le result_worker ecrire la vraie
     * reponse avant notre retour; "queued" l'ecraserait alors et provoquerait
     * un faux timeout de trois secondes. */
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    (void)reserved;
    if (reason==DLL_PROCESS_ATTACH) { g_self=instance; DisableThreadLibraryCalls(instance); }
    else if (reason==DLL_PROCESS_DETACH) {
        /* Aucun appel moteur/restauration sous le loader lock. Les workers ne
         * font que constater ce drapeau et quittent; le processus terminera
         * ensuite leurs handles fermes normalement. */
        InterlockedExchange(&g_process_detaching, 1);
        InterlockedExchange(&g_ammo_maintenance_enabled, 0);
        InterlockedIncrement(&g_ammo_maintenance_epoch);
        InterlockedExchange(&g_ammo_maintenance_unit, -1);
    }
    return TRUE;
}
