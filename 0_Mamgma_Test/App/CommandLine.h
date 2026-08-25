#pragma once

class CCommandLine
{
public:
	static int Execute(int argc, char* argv[]);

private:
	static bool ParsePlane(const char* pszFilePath, struct SPlane& outPlane);
	static void PrintUsage();
};