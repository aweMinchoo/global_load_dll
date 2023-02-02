#include <windows.h>
#include <cstdio>

void OutputDebug(const WCHAR* strOutputString, ...) {
	WCHAR strBuffer[4096] = { 0 };
	va_list vlArgs;
	va_start(vlArgs, strOutputString);
	//in stdio.h
	_vsnwprintf_s(strBuffer, ARRAYSIZE(strBuffer) - 1, ARRAYSIZE(strBuffer) - 1, strOutputString, vlArgs);
	va_end(vlArgs);
	OutputDebugString(strBuffer);
}

static wchar_t exepath[512]{};