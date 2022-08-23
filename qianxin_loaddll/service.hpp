#pragma once
#include <windows.h>
#include <string>
#include <filesystem>

namespace service
{
	bool RegisterAndStart(const std::wstring& driver_path);
	bool StopAndRemove(const std::wstring& driver_name);
};