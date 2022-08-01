/****************************************************************************
**
** Copyright (C) 2018 635672377@qq.com
** All rights reserved.
**
****************************************************************************/

#include "debughelper.h"
#include "registerhelper.h"

#include <strsafe.h>
#include <set>

using std::set;

namespace XIBAO
{

RegisterHelper::RegisterHelper(HKEY key, const wstring &subPath, REGSAM regSam)
	:	mRootKey(key),
		mSubPath(subPath),
		mSamDesired(regSam)
{
}

RegisterHelper::RegisterHelper(const RegisterHelper &right)
{
	_CopyValue(right);
}

RegisterHelper& RegisterHelper::operator=(const RegisterHelper &right)
{
	if (this == &right) {
		return *this;
	}

	_CopyValue(right);
	return *this;
}

RegisterHelper::~RegisterHelper()
{
}

DWORD RegisterHelper::SetDWORD(const wstring &keyName, DWORD dwValue)
{
	return SetValue(REG_DWORD, keyName, dwValue);
}

DWORD RegisterHelper::GetDWORD(const wstring &keyName, DWORD &dwValue) const
{
	return GetValue(REG_DWORD, keyName, dwValue);
}

DWORD RegisterHelper::SetQWORD(const wstring &keyName, int64_t dwValue)
{
	return SetValue(REG_QWORD, keyName, dwValue);
}

DWORD RegisterHelper::GetQWORD(const wstring &keyName, int64_t &dwValue) const
{
	return GetValue(REG_QWORD, keyName, dwValue);
}

DWORD RegisterHelper::SetSZ(const wstring &keyName, const wstring &strValue)
{
	return SetValue(REG_SZ, keyName, strValue);
}

DWORD RegisterHelper::GetSZ(const wstring &keyName, wstring &strValue) const
{
	return GetValue(REG_SZ, keyName, strValue);
}

DWORD RegisterHelper::SetMultiSZ(const wstring &keyName, 
									const wstring &strValue)
{
	return SetValue(REG_MULTI_SZ, keyName, strValue);
}

DWORD RegisterHelper::GetMultiSZ(const wstring &keyName, 
									wstring &strValue) const
{
	return GetValue(REG_MULTI_SZ, keyName, strValue);
}

DWORD RegisterHelper::SetExpandSZ(const wstring &keyName, 
									const wstring &strValue)
{
	return SetValue(REG_EXPAND_SZ, keyName, strValue);
}

DWORD RegisterHelper::GetExpandSZ(const wstring &keyName, 
									wstring &strValue) const
{
	return GetValue(REG_EXPAND_SZ, keyName, strValue);
}

DWORD RegisterHelper::DeleteValue(const wstring &keyName)
{
	if (keyName.empty()) { return -1; }

	HKEY hKey		= 0;
	DWORD errorcode = 0;

	errorcode = RegOpenKeyExW(mRootKey, mSubPath.c_str(), 0, 
											mSamDesired, &hKey);
	if (ERROR_SUCCESS != errorcode) { return errorcode; }
	errorcode = RegDeleteValueW(hKey, keyName.c_str());
	RegCloseKey(hKey);

	return errorcode;
}

DWORD RegisterHelper::DeleteAllValues()
{
	HKEY hKey		= 0;
	DWORD errorcode = 0;

	errorcode = RegOpenKeyExW(mRootKey, mSubPath.c_str(), 0, 
											mSamDesired, &hKey);
	if (ERROR_SUCCESS != errorcode) { return errorcode; }

	errorcode = RegDeleteTreeW(hKey, NULL);
	if (ERROR_SUCCESS != errorcode) { 
		RegCloseKey(hKey);
		return errorcode; 
	}

	RegCloseKey(hKey);
	return errorcode;
}

DWORD RegisterHelper::DeleteKey()
{
	size_t pos = mSubPath.rfind(L"\\");	
	if (wstring::npos == pos) { return -1; }

	wstring partialpath = mSubPath.substr(0, pos);
	wstring keyname		= mSubPath.substr(pos+1, mSubPath.length());

	HKEY hKey = 0;
	DWORD errorcode = RegOpenKeyExW(mRootKey, partialpath.c_str(), 0, 
											mSamDesired, &hKey);
	if (ERROR_SUCCESS != errorcode) { return errorcode; }

	errorcode = RegDeleteKeyExW(hKey, keyname.c_str(), mSamDesired, 0);
	if (ERROR_SUCCESS != errorcode) { 
		RegCloseKey(hKey);
		return errorcode; 
	}

	RegCloseKey(hKey);
	return errorcode;
}


#pragma region DeleteAllKey
//DWORD RegisterHelper::DeleteAllKey()
//{
//	set<wstring> setkey;
//	if (NOERROR != TraverseKey(setkey)) {
//		return -1;
//	}
//
//	//TODO 删除所有的key
//	for (auto &ibeg : setkey) {
//		HKEY hKey = 0;
//		set<wstring> setsubkey;
//		TraverseKey(mRootKey, mSubPath + L"\\" + ibeg, mSamDesired, setsubkey);
//
//		for (auto &isubbeg : setsubkey) {
//			HKEY subKey = 0;
//			wstring path = mSubPath + L"\\"	+ ibeg + L"\\" + isubbeg;
//			DWORD errorcode = RegOpenKeyExW(mRootKey, path.c_str(), 0, 
//												mSamDesired, &subKey);
//			if (ERROR_SUCCESS != errorcode) { continue; }	
//			errorcode = RegDeleteKeyExW(subKey, isubbeg.c_str(), 
//										mSamDesired, 0);
//			RegCloseKey(subKey);
//		}
//
//		wstring subpath = mSubPath + L"\\2\\新项 #1";
//		DWORD errorcode = RegOpenKeyExW(mRootKey, subpath.c_str(), 0, 
//											mSamDesired, &hKey);
//		if (ERROR_SUCCESS != errorcode) { continue; }	
//		// errorcode = RegDeleteKeyExW(hKey, ibeg.c_str(), mSamDesired, 0);
//		errorcode = RegDeleteKeyExW(hKey, L"新项 #1", mSamDesired, 0);
//
//	}
//
//	return NOERROR;
//}

#pragma endregion

BOOL RegisterHelper::RegDelnode(/*HKEY hKeyRoot, LPCTSTR lpSubKey*/)
{
	TCHAR szDelKey[MAX_PATH*2];
    StringCchCopy (szDelKey, MAX_PATH*2, mSubPath.c_str());
    return _RegDelnodeRecurse(mRootKey, szDelKey);
}

void RegisterHelper::_CopyValue(const RegisterHelper &right)
{
	mRootKey	= right.mRootKey;
	mSubPath	= right.mSubPath;
	mSamDesired = right.mSamDesired;
}

// 关于递归删除subkey参考链接
// https://docs.microsoft.com/en-us/windows/desktop/sysinfo/deleting-a-key-with-subkeys
//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all its subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************
BOOL RegisterHelper::_RegDelnodeRecurse(HKEY hKeyRoot, LPWSTR lpSubKey)
{
	LPWSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    wchar_t szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having
    // to recurse.
    lResult = RegDeleteKeyW(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    lResult = RegOpenKeyExW(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) 
    {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            return TRUE;
        } 
        else {
            return FALSE;
        }
    }

    // Check for an ending slash and add one if it is missing.
    lpEnd = lpSubKey + lstrlenW(lpSubKey);

    if (*(lpEnd - 1) != TEXT('\\')) 
    {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    // Enumerate the keys
    dwSize = MAX_PATH;
    lResult = RegEnumKeyExW(hKey, 0, szName, &dwSize, NULL,
                           NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS) {
        do {
            *lpEnd = TEXT('\0');
            StringCchCatW(lpSubKey, MAX_PATH * 2, szName);

            if (! _RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            dwSize = MAX_PATH;

            lResult = RegEnumKeyExW(hKey, 0, szName, &dwSize, NULL,
                                   NULL, NULL, &ftWrite);

        } while (lResult == ERROR_SUCCESS);
    }

    lpEnd--;
    *lpEnd = TEXT('\0');

    RegCloseKey (hKey);

    // Try again to delete the key.
    lResult = RegDeleteKeyW(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS) 
        return TRUE;

    return FALSE;
}

};

