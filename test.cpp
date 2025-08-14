#include <Windows.h>
#include <iostream>
#include <string>
#include "MinHook.h"

#define RVA_PlayerControl_GetTrueSpeed 0x5DBCC0
#define RVA_PlayerPhysics_CalclateLightRadius 0x62AC40
#define RVA_LogicOptions_GetKillCooldown 0x7BC5E0
#define RVA_PlayerControl_CanMove 0x5C5B00
#define RVA_PlayerControl_Exiled 0x5BBA20
#define RVA_LogicOptions_GetKillDistance 0x7BC660
#define RVA_InfectedOverlay_CanUseSabotage 0x519300

bool bSpeedHack = false, bMaxVision = false, bNoKillCooldown = false,
bMoveInMeeting = false, bAntiExile = false, bLongRangeKill = false,
bNoSabotageCooldown = false;

typedef float(__cdecl* get_float_t)(void* instance);
typedef float(__cdecl* get_float_1p_t)(void* instance, void* p1);
typedef bool(__cdecl* get_bool_t)(void* instance);
typedef void(__cdecl* void_t)(void* instance);

get_float_t o_GetTrueSpeed = nullptr;
get_float_1p_t o_CalclateLightRadius = nullptr;
get_float_t o_GetKillCooldown = nullptr;
get_bool_t o_CanMove = nullptr;
void_t o_Exiled = nullptr;
get_float_t o_GetKillDistance = nullptr;
get_bool_t o_CanUseSabotage = nullptr;

float __cdecl hk_GetTrueSpeed(void* instance) { if (bSpeedHack) return 8.0f; return o_GetTrueSpeed(instance); }
float __cdecl hk_CalclateLightRadius(void* i, void* p) { if (bMaxVision) return 40.0f; return o_CalclateLightRadius(i, p); }
float __cdecl hk_GetKillCooldown(void* instance) { if (bNoKillCooldown) return 0.0f; return o_GetKillCooldown(instance); }
bool __cdecl hk_CanMove(void* instance) { if (bMoveInMeeting) return true; return o_CanMove(instance); }
void __cdecl hk_Exiled(void* instance) { if (bAntiExile) return; o_Exiled(instance); }
float __cdecl hk_GetKillDistance(void* instance) { if (bLongRangeKill) return 100.0f; return o_GetKillDistance(instance); }
bool __cdecl hk_CanUseSabotage(void* instance) { if (bNoSabotageCooldown) return true; return o_CanUseSabotage(instance); }

void UpdateConsoleStatus() {
    system("cls");
    std::cout << "[F1] Speed Hack        -> " << (bSpeedHack ? "ON" : "OFF") << " (Client)\n";
    std::cout << "[F2] Max Vision        -> " << (bMaxVision ? "ON" : "OFF") << " (Client)\n";
    std::cout << "[F3] No Kill Cooldown   -> " << (bNoKillCooldown ? "ON" : "OFF") << " (Impostor)\n";
    std::cout << "[F4] Move In Meeting    -> " << (bMoveInMeeting ? "ON" : "OFF") << " (Client)\n";
    std::cout << "[F5] Anti-Exile         -> " << (bAntiExile ? "ON" : "OFF") << " (Client)\n";
    std::cout << "[F6] Long Range Kill    -> " << (bLongRangeKill ? "ON" : "OFF") << " (Impostor)\n";
    std::cout << "[F7] No Sabotage CD     -> " << (bNoSabotageCooldown ? "ON" : "OFF") << " (Impostor)\n";
    std::cout << "[END] to Eject DLL\n";
}

DWORD WINAPI HackThread(HMODULE hModule) {
    AllocConsole(); FILE* f; freopen_s(&f, "CONOUT$", "w", stdout);

    uintptr_t base = (uintptr_t)GetModuleHandleA("GameAssembly.dll");

    if (MH_Initialize() == MH_OK) {
        MH_CreateHook((void*)(base + RVA_PlayerControl_GetTrueSpeed), &hk_GetTrueSpeed, (void**)&o_GetTrueSpeed);
        MH_CreateHook((void*)(base + RVA_PlayerPhysics_CalclateLightRadius), &hk_CalclateLightRadius, (void**)&o_CalclateLightRadius);
        MH_CreateHook((void*)(base + RVA_LogicOptions_GetKillCooldown), &hk_GetKillCooldown, (void**)&o_GetKillCooldown);
        MH_CreateHook((void*)(base + RVA_PlayerControl_CanMove), &hk_CanMove, (void**)&o_CanMove);
        MH_CreateHook((void*)(base + RVA_PlayerControl_Exiled), &hk_Exiled, (void**)&o_Exiled);
        MH_CreateHook((void*)(base + RVA_LogicOptions_GetKillDistance), &hk_GetKillDistance, (void**)&o_GetKillDistance);
        MH_CreateHook((void*)(base + RVA_InfectedOverlay_CanUseSabotage), &hk_CanUseSabotage, (void**)&o_CanUseSabotage);
        MH_EnableHook(MH_ALL_HOOKS);
    }

    UpdateConsoleStatus();

    while (!(GetAsyncKeyState(VK_END) & 1)) {
        bool changed = false;
        if (GetAsyncKeyState(VK_F1) & 1) { bSpeedHack = !bSpeedHack; changed = true; }
        if (GetAsyncKeyState(VK_F2) & 1) { bMaxVision = !bMaxVision; changed = true; }
        if (GetAsyncKeyState(VK_F3) & 1) { bNoKillCooldown = !bNoKillCooldown; changed = true; }
        if (GetAsyncKeyState(VK_F4) & 1) { bMoveInMeeting = !bMoveInMeeting; changed = true; }
        if (GetAsyncKeyState(VK_F5) & 1) { bAntiExile = !bAntiExile; changed = true; }
        if (GetAsyncKeyState(VK_F6) & 1) { bLongRangeKill = !bLongRangeKill; changed = true; }
        if (GetAsyncKeyState(VK_F7) & 1) { bNoSabotageCooldown = !bNoSabotageCooldown; changed = true; }
        if (changed) UpdateConsoleStatus();
        Sleep(150);
    }

    MH_DisableHook(MH_ALL_HOOKS); MH_Uninitialize();
    if (f) fclose(f); FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hM, DWORD r, LPVOID l) {
    if (r == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hM);
        HANDLE hT = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HackThread, hM, 0, 0);
        if (hT) CloseHandle(hT);
    }
    return TRUE;
}