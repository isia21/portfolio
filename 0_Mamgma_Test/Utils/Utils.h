#pragma once

namespace Utils {
	extern const bool WRITE_LOG_FILE;
	extern HWND hWnd;
	
	void Init(HWND h);

	int Message(const char* str, ...);
	void ODS(const char* str, ...);
}

#define SAFEDELETE(op)         if (op) { delete op;op=NULL;}
#define SAFEDELETEARRAY(op)    if (op) { delete [] op;op=NULL;}
#define ASSERT(op) \
	if (!(op)) { \
		Utils::Message("[ASSERT]\n\"%s\"-- \"%s\"() on \"%s\" %d(Line).\n",#op,__FUNCTION__,__FILE__,__LINE__); \
		assert(op); \
	}