/* Halo Sandbox — Copyright (c) 2026 Ghost7134 — licence MIT. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <xinput.h>
#include <string.h>

typedef DWORD (WINAPI *xinput_get_state_fn)(DWORD, XINPUT_STATE *);

static HMODULE g_self;
static volatile LONG g_started;

static int menu_is_open(void) {
    wchar_t path[MAX_PATH];
    char text[8] = {0};
    DWORD read = 0;
    HANDLE file;
    GetModuleFileNameW(g_self, path, MAX_PATH);
    wchar_t *slash = wcsrchr(path, L'\\');
    if (!slash) return 0;
    wcscpy_s(slash + 1, MAX_PATH - (size_t)(slash + 1 - path), L"menu_state.txt");
    file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) return 0;
    ReadFile(file, text, sizeof(text) - 1, &read, NULL);
    CloseHandle(file);
    return read > 0 && text[0] == '1';
}

static int game_has_focus(void) {
    DWORD pid = 0;
    HWND window = GetForegroundWindow();
    if (!window) return 0;
    GetWindowThreadProcessId(window, &pid);
    return pid == GetCurrentProcessId();
}

static void press_key(WORD key) {
    INPUT input[2];
    memset(input, 0, sizeof(input));
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = key;
    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = key;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, input, sizeof(INPUT));
}

static DWORD WINAPI controller_thread(void *unused) {
    HMODULE xinput = LoadLibraryW(L"xinput1_4.dll");
    xinput_get_state_fn get_state;
    WORD previous = 0;
    (void)unused;
    if (!xinput) xinput = LoadLibraryW(L"xinput1_3.dll");
    if (!xinput) xinput = LoadLibraryW(L"xinput9_1_0.dll");
    get_state = xinput ? (xinput_get_state_fn)GetProcAddress(xinput, "XInputGetState") : NULL;
    while (get_state) {
        XINPUT_STATE state;
        WORD buttons = 0;
        if (get_state(0, &state) == ERROR_SUCCESS) buttons = state.Gamepad.wButtons;
        if (game_has_focus()) {
            WORD pressed = (WORD)(buttons & (WORD)~previous);
            int chord = (buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) && (buttons & XINPUT_GAMEPAD_DPAD_RIGHT);
            int old_chord = (previous & XINPUT_GAMEPAD_RIGHT_SHOULDER) && (previous & XINPUT_GAMEPAD_DPAD_RIGHT);
            if (chord && !old_chord) press_key(VK_F4);
            if (menu_is_open()) {
                if (pressed & XINPUT_GAMEPAD_DPAD_UP) press_key(VK_UP);
                if (pressed & XINPUT_GAMEPAD_DPAD_DOWN) press_key(VK_DOWN);
                /* Enter est consomme par Halo sur l'hote coop. F12 est une
                 * touche reservee au mod et arrive donc toujours a UE4SS. */
                if (pressed & XINPUT_GAMEPAD_A) press_key(VK_F12);
                if (pressed & XINPUT_GAMEPAD_B) press_key(VK_BACK);
            }
        }
        previous = buttons;
        Sleep(20);
    }
    if (xinput) FreeLibrary(xinput);
    return 0;
}

__declspec(dllexport) int halosandbox_controller_start(void *lua_state) {
    HANDLE thread;
    (void)lua_state;
    if (InterlockedCompareExchange(&g_started, 1, 0) == 0) {
        thread = CreateThread(NULL, 0, controller_thread, NULL, 0, NULL);
        if (thread) CloseHandle(thread);
    }
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        g_self = instance;
        DisableThreadLibraryCalls(instance);
    }
    return TRUE;
}
