#include "pch.h"
#include "../shared/shared.h"

__declspec(dllexport) void ijtdummy() {

}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      GetModuleFileNameW(NULL, exepath, 512);
      OutputDebug(L"[64]dll loaded into %ws\n", exepath);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

