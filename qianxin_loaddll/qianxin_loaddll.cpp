#include <windows.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "service.hpp"
#include "GHCORE64.sys.h"

using namespace std;

int _cdecl wmain(int argc, wchar_t* argv[])
{
  if (argc < 2) {
    wcout << "[-] need param\n";
    return 0;
  }  
    bool isloadorunload; //load
    wcout << "[+] " << argv[1] << endl;
    if (!_wcsicmp(argv[1], L"-i"))
        isloadorunload = true;
    else if (!_wcsicmp(argv[1], L"-u"))
        isloadorunload = false;
    else {
        wcout << "[-] Error command line option\n";
        return 0;
    }
    if (isloadorunload && argc < 4) {
        wcout << "[-] Need 4 params\n";
        return 0;
    }
    if (!isloadorunload) {
        service::StopAndRemove(L"GHCORE64.sys");
        return 0;
     }

    std::wstring dll_path32 = argv[2];
    std::wstring dll_path64 = argv[3];
    if (!std::filesystem::exists(dll_path32) || !std::filesystem::exists(dll_path64)) {
        wcout << "[-] Invalid dll path\n";
        return 0;
    }

    wstring servicekey = L"SYSTEM\\CurrentControlSet\\Services\\";
    HKEY okey;

    // 将要注入的DLL写入注入表
    LSTATUS s = RegCreateKeyW(HKEY_LOCAL_MACHINE, (servicekey + L"GHCORE64.sys").c_str(), &okey);
    if (s != ERROR_SUCCESS) {
        wcout << "[+] RegCreateKeyW GHCORE64.sys failed\n";
        return 0;
    }

    s = RegSetKeyValueW(okey, NULL, L"ghdll32", REG_SZ, dll_path32.c_str(), dll_path32.size()*2);
    s|= RegSetKeyValueW(okey, NULL, L"ghdll64", REG_SZ, dll_path64.c_str(), dll_path64.size()*2);
    if (s != ERROR_SUCCESS) {
        wcout << "[+] RegSetKeyValueW GHCORE64.sys failed\n";
        return 0;
    }


    std::filesystem::path current_path("./");
    std::filesystem::path canonical_path;
    canonical_path = std::filesystem::canonical(current_path);

    std::wstring driver_path(canonical_path.append(L"GHCORE64.sys"));

    HANDLE h = CreateFileW(driver_path.c_str(), FILE_ALL_ACCESS, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (h == INVALID_HANDLE_VALUE) {
        auto error = GetLastError();
        wcout << "[-] CreateFile failed\n";
        return 0;
    }

    DWORD lpNumberOfBytesWritten;
    bool b =WriteFile(h, GHCORE64_sys, sizeof(GHCORE64_sys), &lpNumberOfBytesWritten, NULL);
    if (!b)
    {
        wcout << "[-] WriteFile failed\n";
        return 0;
    }

    CloseHandle(h);     //要先关闭文件句柄
    
    //写入自己的DLL的路径

    bool load = service::RegisterAndStart(driver_path);
    if (load)
        wcout << "[+] Driver loading success\n";
    wcout << "[+] delete drive " << driver_path << endl;
    bool is_removed = filesystem::remove(driver_path);
    if (!is_removed) wcout << "[+] driver not removed\n";

    return 0;
}

