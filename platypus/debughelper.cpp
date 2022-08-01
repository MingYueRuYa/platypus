/****************************************************************************
**
** Copyright (C) 2018 635672377@qq.com
** All rights reserved.
**
****************************************************************************/

#include "debughelper.h"

#include <stdio.h>

namespace XIBAO
{

void DebugHelper::OutputDebugString(LPCSTR format, ...)
{
#ifdef XIBAO_DEBUG_HELPER
	va_list args;
	va_start(args, format);
	int len = _vscprintf(format, args) + 1;
	va_end(args);

	va_start(args, format);
	char *buf = new char[len];
	_vsnprintf_s(buf, len, len, format, args);
	va_end(args);

	::OutputDebugStringA(buf);
	delete[] buf;
#endif // XIBAO_DEBUG_HELPER
}

void DebugHelper::OutputDebugString(LPCTSTR format, ...)
{
#ifdef XIBAO_DEBUG_HELPER
	va_list args;
	va_start(args, format);
	int len = _vscwprintf(format, args) + 1;
	va_end(args);

	va_start(args, format);
	wchar_t *buf = new wchar_t[len];
	_vsnwprintf_s(buf, len, len, format, args);
	va_end(args);

	::OutputDebugString(buf);
	delete[] buf;
#endif // XIBAO_DEBUG_HELPER
}

void DebugHelper::OutputDebugString(const wstring &strbufinfo)
{
#ifdef XIBAO_DEBUG_HELPER
	::OutputDebugString(strbufinfo.c_str());
#endif // XIBAO_DEBUG_HELPER
}

void DebugHelper::OutputDebugString(const string &strbufinfo)
{
#ifdef XIBAO_DEBUG_HELPER
	::OutputDebugStringA(strbufinfo.c_str());
#endif // XIBAO_DEBUG_HELPER
}

} //namespace XIBAO;

