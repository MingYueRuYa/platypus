/****************************************************************************
**
** Copyright (C) 2018 635672377@qq.com
** All rights reserved.
**
****************************************************************************/

#ifndef debughelper_h
#define debughelper_h

#include <string>
#include <windows.h>

using std::string;
using std::wstring;

namespace XIBAO {

#ifndef XIBAO_DEBUG_HELPER
#define XIBAO_DEBUG_HELPER
#endif // XIBAO_DEBUG_HELPER

#define OutDebug(msg) (XIBAO::DebugHelper::OutputDebugString(msg))

#ifndef _DEBUG
#undef XIBAO_DEBUG_HELPER
#endif // _DEBUG

/*! \brief common::debughelper
 	\author liushixiong (liushixiongcpp@163.cn)
 	\version 0.01
 	\date 2018-11-29 10:31:00
*/
class DebugHelper
{
public:	
	DebugHelper() = default;
	~DebugHelper() = default;
	DebugHelper(const DebugHelper &) = default;

	static void OutputDebugString(LPCSTR format, ...);
	static void OutputDebugString(LPCTSTR format, ...);
	static void OutputDebugString(const wstring &strbufinfo);
	static void OutputDebugString(const string &strbufinfo);
};

};

#endif // debughelper_h