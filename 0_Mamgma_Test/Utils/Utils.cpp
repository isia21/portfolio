#include "stdafx.h"

namespace {
	CRITICAL_SECTION g_logCriticalSection;
	INIT_ONCE g_logInitOnce = INIT_ONCE_STATIC_INIT;
	FILE* g_logFile = nullptr;

	BOOL CALLBACK InitLogFile(PINIT_ONCE, PVOID, PVOID*)
	{
		InitializeCriticalSection(&g_logCriticalSection);

		if (!Utils::WRITE_LOG_FILE)
			return TRUE;

		_mkdir("log");

		time_t now = time(nullptr);
		tm localTime = {};
		localtime_s(&localTime, &now);

		char filename[MAX_PATH] = {};
		sprintf_s(
			filename,
			"log/%04d_%02d_%02d_%02d_%02d.txt",
			localTime.tm_year + 1900,
			localTime.tm_mon + 1,
			localTime.tm_mday,
			localTime.tm_hour,
			localTime.tm_min);

		fopen_s(&g_logFile, filename, "a");
		return TRUE;
	}

	void WriteLogFile(const char* message)
	{
		if (!Utils::WRITE_LOG_FILE)
			return;

		InitOnceExecuteOnce(&g_logInitOnce, InitLogFile, nullptr, nullptr);
		if (!g_logFile)
			return;

		EnterCriticalSection(&g_logCriticalSection);
		fputs(message, g_logFile);
		fputc('\n', g_logFile);
		fflush(g_logFile);
		LeaveCriticalSection(&g_logCriticalSection);
	}
}

namespace Utils {
	const bool WRITE_LOG_FILE = true;
	HWND hWnd = nullptr;

	int Message(const char* str, ...)
	{
		char szMsg[40960];

		va_list va;
		va_start(va, str);
		vsprintf_s(szMsg, str, va);
		va_end(va);

		return MessageBox(hWnd == nullptr? nullptr : hWnd, szMsg, "HINT", MB_OK);
	}
	void ODS(const char* format, ...)
	{
		va_list args, args_copy;
		va_start(args, format);
		va_copy(args_copy, args);
		int len = vsnprintf(nullptr, 0, format, args_copy);
		va_end(args_copy);

		if (len <= 0)
		{
			va_end(args);
			return;
		}
		std::string buffer(len + 2, '\0');
		vsnprintf(&buffer[0], len + 1, format, args);
		WriteLogFile(buffer.c_str());
		buffer[len] = '\n';
		OutputDebugStringA(buffer.c_str());

		va_end(args);
	}

	void Init(HWND h) { hWnd = h; }
}