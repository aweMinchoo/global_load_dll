#include "service.hpp"
#include "nt.hpp"

#include <iostream>
#include <algorithm>
#include <string>

using namespace std;
bool service::RegisterAndStart(const std::wstring& driver_path) {

	uint32_t ErrorControl = 0;
	uint32_t Start = 3;


	const static DWORD ServiceTypeKernel = 1;
	//
	//从驱动路径中解析出驱动文件名,带文件后缀
	//
	const std::wstring nPath = L"\\??\\" + driver_path;
	size_t pos1 = driver_path.find_last_of(L'\\')+1;
	size_t pos2 = driver_path.find(L".sys");
	if (pos1 == (wstring::npos+1) || pos2 == wstring::npos)
	{
		wcout << L"[-] Invalid driver path" << std::endl;
		return false;
	}
	const std::wstring driver_name = driver_path.substr(pos1,pos2);
	const std::wstring servicesPath = L"SYSTEM\\CurrentControlSet\\Services\\" + driver_name;
	std::wcout << L"[+] Loading the driver \"" << driver_name.c_str() << L"\"" << std::endl;
	HKEY dservice;
	LSTATUS status = RegCreateKeyW(HKEY_LOCAL_MACHINE, servicesPath.c_str(), &dservice); //Returns Ok if already exists
	if (status != ERROR_SUCCESS) {
		wcout << L"[-] Can't create service key" << std::endl;
		return false;
	}

	status = RegSetKeyValueW(dservice, NULL, L"ImagePath", REG_EXPAND_SZ, nPath.c_str(), (DWORD)(nPath.size()*sizeof(wchar_t)));
	if (status != ERROR_SUCCESS) {
		RegCloseKey(dservice);
		wcout << L"[-] Can't create 'ImagePath' registry value" << std::endl;
		return false;
	}
	status = RegSetKeyValueW(dservice, NULL, L"DisplayName", REG_SZ, driver_name.c_str(), (DWORD)(driver_name.size() * sizeof(wchar_t)));
	if (status != ERROR_SUCCESS) {
		RegCloseKey(dservice);
		wcout << L"[-] Can't create 'DisplayName' registry value" << std::endl;
		return false;
	}
	
	status = RegSetKeyValueW(dservice, NULL, L"Start", REG_DWORD, &Start, 4);
	if (status != ERROR_SUCCESS) {
		RegCloseKey(dservice);
		wcout << L"[-] Can't create 'Start' registry value" << std::endl;
		return false;
	}
	status = RegSetKeyValueW(dservice, NULL, L"Type", REG_DWORD, &ServiceTypeKernel, sizeof(DWORD));
	if (status != ERROR_SUCCESS) {
		RegCloseKey(dservice);
		wcout << L"[-] Can't create 'Type' registry value" << std::endl;
		return false;
	}
	
	RegCloseKey(dservice);

	HMODULE ntdll = GetModuleHandleA("ntdll.dll");
	if (ntdll == NULL) {
		return false;
	}

	auto RtlAdjustPrivilege = (nt::RtlAdjustPrivilege)GetProcAddress(ntdll, "RtlAdjustPrivilege");
	auto NtLoadDriver = (nt::NtLoadDriver)GetProcAddress(ntdll, "NtLoadDriver");

	ULONG SE_LOAD_DRIVER_PRIVILEGE = 10UL;
	BOOLEAN SeLoadDriverWasEnabled;
	NTSTATUS Status = RtlAdjustPrivilege(SE_LOAD_DRIVER_PRIVILEGE, TRUE, FALSE, &SeLoadDriverWasEnabled);
	if (!NT_SUCCESS(Status)) {
		wcout << L"[-]Fatal error: failed to acquire SE_LOAD_DRIVER_PRIVILEGE. Make sure you are running as administrator." << std::endl;
		return false;
	}

	std::wstring wdriver_reg_path = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" + driver_name;
	UNICODE_STRING serviceStr;
	RtlInitUnicodeString(&serviceStr, wdriver_reg_path.c_str());

	Status = NtLoadDriver(&serviceStr);
	wcout << L"[-] NtLoadDriver Status 0x" << std::hex << Status << std::endl;
	
	if (Status == 0xC000010E) {// STATUS_IMAGE_ALREADY_LOADED
		return true;
	}
	
	return true;
}

bool service::StopAndRemove(const std::wstring& driver_name) {
	HMODULE ntdll = GetModuleHandleA("ntdll.dll");
	if (ntdll == NULL)
		return false;

	auto RtlAdjustPrivilege = (nt::RtlAdjustPrivilege)GetProcAddress(ntdll, "RtlAdjustPrivilege");
	ULONG SE_LOAD_DRIVER_PRIVILEGE = 10UL;
	BOOLEAN SeLoadDriverWasEnabled;
	NTSTATUS Status = RtlAdjustPrivilege(SE_LOAD_DRIVER_PRIVILEGE, TRUE, FALSE, &SeLoadDriverWasEnabled);
	if (!NT_SUCCESS(Status)) {
		wcout << L"[-]Fatal error: failed to acquire SE_LOAD_DRIVER_PRIVILEGE. Make sure you are running as administrator." << std::endl;
		return false;
	}

	std::wstring wdriver_reg_path = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" + driver_name;
	UNICODE_STRING serviceStr;
	RtlInitUnicodeString(&serviceStr, wdriver_reg_path.c_str());

	HKEY driver_service;
	std::wstring servicesPath = L"SYSTEM\\CurrentControlSet\\Services\\" + driver_name;
	LSTATUS status = RegOpenKeyW(HKEY_LOCAL_MACHINE, servicesPath.c_str(), &driver_service);
	if (status != ERROR_SUCCESS) {
		wcout << "[-] Cant find key " << servicesPath << endl;
		return false;
	}
	RegCloseKey(driver_service);

	auto NtUnloadDriver = (nt::NtUnloadDriver)GetProcAddress(ntdll, "NtUnloadDriver");
	NTSTATUS st = NtUnloadDriver(&serviceStr);
	wcout << L"[+] NtUnloadDriver Status 0x" << std::hex << st << std::endl;
	if (st != 0x0) {
		wcout << L"[-] Driver unload Failed" << std::endl;
	}
	else
		wcout << L"[+] Driver unload Success" << endl;
	

	status = RegDeleteKeyW(HKEY_LOCAL_MACHINE, servicesPath.c_str());
	if (status != ERROR_SUCCESS) {
		return false;
	}
	return true;
}
