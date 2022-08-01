/****************************************************************************
**
** Copyright (C) 2018 635672377@qq.com
** All rights reserved.
**
****************************************************************************/

#ifndef registerhelper_h
#define registerhelper_h

#include "utili.hpp"

#include <map>
#include <string>
#include <memory>
#include <windows.h>

using std::map;
using std::wstring;
using std::shared_ptr;
using std::make_shared;
using std::make_shared_array;

namespace XIBAO {

/*! \brief common::registerhelper
 	\author liushixiong (liushixiongcpp@163.cn)
 	\version 0.03
	\description 注册表读取助手
 	\date 2018-12-13 20:13:42
	\description 增加遍历key，增加模板支持 2018-12-23 11:23:40
*/
class RegisterHelper
{
private:
	template <typename T>
	class RegDataType {
		public:
			explicit RegDataType(DWORD regType, const T &tValue)
				:	mRegType(regType),
					mValue(tValue),
					mByteSize(MAX_PATH*2)
			{ 
				// 静态断言
				static_assert(! std::is_pointer<T>::value, 
								"Not support pinter type.");
			}

			explicit RegDataType(DWORD regType)
				:	mRegType(regType),
					mValue(T()),
					mByteSize(MAX_PATH*2)
			{ 
				// 静态断言
				static_assert(! std::is_pointer<T>::value, 
								"Not support pinter type.");
			}

			~RegDataType() {}
			RegDataType(const RegDataType &right) = default;
			RegDataType& operator=(const RegDataType &right) = default;

			unsigned int ValueSize() const { return sizeof(T); }
			const T *GetValueAddress() const { return &mValue; }
			DWORD GetType() const { return mRegType; }
			DWORD *GetTypeAddress() const { return &mRegType; }
			
			BYTE *GetByte()
			{
				if (nullptr == mshByte.get()) {
					mshByte.reset(new BYTE[mByteSize]);
				} 
				return mshByte.get();
			}

			DWORD *GetByteSize() { return &mByteSize; }

			T GetValue() 
			{
				mValue = *(reinterpret_cast<T *>(mshByte.get()));
				return mValue;
			}

		public:
			DWORD mRegType;
			DWORD mByteSize;
			T	  mValue;
			shared_ptr<BYTE> mshByte;
	};

	template <>
	class RegDataType<wstring> {
	public:
		RegDataType(DWORD regType, const wstring &tValue)
			: mRegType(regType),
			  mValue(tValue),
			  mByteSize(MAX_PATH*2)
		{ 
		}

		RegDataType(DWORD regType)
			: mRegType(regType),
			  mValue(wstring(L"")),
			  mByteSize(MAX_PATH*2)
		{ 
		}

		~RegDataType() {}

		RegDataType(const RegDataType &right) = default;
		RegDataType& operator=(const RegDataType &right) = default;

		unsigned int ValueSize() const 
		{ return mValue.length() * sizeof(wchar_t); }
		const wchar_t *GetValueAddress() const { return mValue.c_str(); }
		DWORD GetType() const { return mRegType; }
		DWORD *GetTypeAddress() { return &mRegType; }

		wchar_t *GetByte()
		{
			if (nullptr == mshByte.get()) {
				mshByte = make_shared_array<wchar_t>(mByteSize);
			} 
			return mshByte.get();
		}

		DWORD *GetByteSize() { return &mByteSize; }

		wstring GetValue() 
		{
			mValue = wstring(mshByte.get());
			return mValue;
		}

	public:
		DWORD	mRegType;
		wstring	mValue;
		DWORD mByteSize;
		shared_ptr<wchar_t> mshByte;
	};

	// TODO 优化：HKEY不用每次都打开，可以考虑将HKEY作为成员变量
public:
	RegisterHelper(HKEY key, const wstring &subPath, REGSAM regSam);
	RegisterHelper(const RegisterHelper &right);
	RegisterHelper& operator=(const RegisterHelper &right);
	~RegisterHelper();

	DWORD SetDWORD(const wstring &keyName, DWORD dwValue);
	DWORD GetDWORD(const wstring &keyName, DWORD &dwValue) const;

	DWORD SetQWORD(const wstring &keyName, int64_t dwValue);
	DWORD GetQWORD(const wstring &keyName, int64_t &dwValue) const;

	DWORD SetSZ(const wstring &keyName, const wstring &strValue);
	DWORD GetSZ(const wstring &keyName, wstring &strValue) const;

	DWORD SetMultiSZ(const wstring &keyName, const wstring &strValue);
	DWORD GetMultiSZ(const wstring &keyName, wstring &strValue) const;

	DWORD SetExpandSZ(const wstring &keyName, const wstring &strValue);
	DWORD GetExpandSZ(const wstring &keyName, wstring &strValue) const;

	DWORD DeleteValue(const wstring &keyName);
	DWORD DeleteAllValues();
	DWORD DeleteKey();
	// DWORD DeleteAllKey();
	BOOL RegDelnode(/*HKEY hKeyRoot, LPCTSTR lpSubKey*/);

	// 保存数组数据时，需要泛型的迭代器
	template<typename TContainer>
	DWORD TraverseKey(TContainer &containter)
	{
		HKEY hKey = 0;
		DWORD errorcode = RegOpenKeyExW(mRootKey, mSubPath.c_str(), 0, 
										mSamDesired, &hKey);
		if (ERROR_SUCCESS != errorcode) { return errorcode; }
		DWORD dwIndex = 0;
		do {
			wchar_t lpName[MAX_PATH]	= {0};
			DWORD cchName				= MAX_PATH;
			errorcode = RegEnumKeyExW(hKey, dwIndex, lpName, &cchName, 
										NULL, NULL, NULL, NULL);
			if (errorcode != ERROR_SUCCESS) { break; }
			
			dwIndex++;
			
			containter.insert(wstring(lpName));

#ifdef XIBAO_DEBUG_HELPER
			DebugHelper::OutputDebugStringW(wstring(lpName) + L"\r\n");
#endif // XIBAO_DEBUG_HELPER
		} while (errorcode != ERROR_NO_MORE_ITEMS);

		if (errorcode == ERROR_NO_MORE_ITEMS) { errorcode = ERROR_SUCCESS; }

		RegCloseKey(hKey);	
		return errorcode;
	}

	// container内需要pair类型（key, value）
	template<typename TContainer>
	DWORD TraverseValue(TContainer &container)
	{
		HKEY hKey = 0;
		DWORD errorcode = RegOpenKeyExW(mRootKey, mSubPath.c_str(), 0, 
										mSamDesired, &hKey);
		DWORD dwIndex = 0;
		BYTE *lpData = new BYTE[MAX_PATH];
		do {
			ZeroMemory(lpData, MAX_PATH);
			wchar_t wchvalue[MAX_PATH]	= {0};
			DWORD cchcount				= MAX_PATH; 
			DWORD type = 0;
			errorcode = RegEnumValueW(hKey, dwIndex, wchvalue, &cchcount, 
										0, 0, 0, 0);
			if (errorcode != ERROR_SUCCESS) { break; }

			DWORD cbData = MAX_PATH;
			errorcode = RegQueryValueExW(hKey, wchvalue, 0, &type, lpData, &cbData);
			if (errorcode != ERROR_SUCCESS) { continue; }

			wstring value = L"";
			DWORD dwvalue = 0;
			switch (type) {
			case REG_SZ:
			case REG_MULTI_SZ:
			case REG_EXPAND_SZ:
				value = reinterpret_cast<wchar_t *>(lpData);	
			case REG_DWORD:
				dwvalue = *(reinterpret_cast<DWORD *>(lpData));	
				value = to_wstring(dwvalue);
			default:
				break;
			}
			
			dwIndex++;
#ifdef XIBAO_DEBUG_HELPER
			DebugHelper::OutputDebugStringW(L"%s:%s\r\n", wchvalue, 
												value.c_str());
#endif // XIBAO_DEBUG_HELPER
			
			container.insert(std::pair<wstring, wstring>(wchvalue, value));

		} while(errorcode != ERROR_NO_MORE_ITEMS);
		delete [] lpData;

		if (errorcode == ERROR_NO_MORE_ITEMS) { errorcode = ERROR_SUCCESS; }

		RegCloseKey(hKey);	
		return errorcode;
	}

private:
	template<typename TRetValue>
	DWORD GetValue(DWORD regType, 
					const wstring &keyName, 
					TRetValue &value) const
	{
		HKEY hKey		= 0;
		DWORD result	= NOERROR;

		RegDataType<TRetValue> regdata(regType);

		long lRet = RegOpenKeyExW(mRootKey, mSubPath.c_str(), 0,
									mSamDesired, &hKey);
		if (lRet != ERROR_SUCCESS) { return lRet; }

		DWORD regtype;
		lRet = RegQueryValueExW(hKey, 
								keyName.c_str(), 
								NULL, 
								&regtype, 
								(BYTE *)regdata.GetByte(),
								regdata.GetByteSize()
								);
		
		if (lRet != ERROR_SUCCESS) { return lRet; }
		value = regdata.GetValue();
		return 0;
	}

	template<typename TRetValue>
	DWORD SetValue(DWORD regType,
					const wstring &keyName,
					const TRetValue &value
					)
	{
		RegDataType<TRetValue> datatype(regType, value);

		HKEY hKey			= 0;
		BYTE *lpData		= nullptr;
		DWORD result		= NOERROR;

		long lRet = RegOpenKeyExW(mRootKey, mSubPath.c_str(), 0, 
											mSamDesired, &hKey);
		if (lRet != ERROR_SUCCESS) {
			if (ERROR_SUCCESS != (lRet = RegCreateKeyW(mRootKey, 
												mSubPath.c_str(), 
												&hKey))) {
				result = lRet;
				goto CreateError;
			}
		}

		lpData = new BYTE[datatype.ValueSize()];
		CopyMemory(lpData, datatype.GetValueAddress(), datatype.ValueSize());
		lRet = RegSetValueEx(hKey, keyName.c_str(), 0, datatype.GetType(), 
								lpData, datatype.ValueSize());
		result = lRet;
		RegCloseKey(hKey);
		delete[] lpData;
		return result;	

	CreateError:
		RegCloseKey(hKey);
		delete[] lpData;
		return result;
	}

private:
	void _CopyValue(const RegisterHelper &right);
	BOOL _RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey);

private:
	// 主键
	HKEY mRootKey;
	// 副键
	wstring mSubPath;
	// 读取权限
	REGSAM mSamDesired;
	
};

}

#endif // registerhelper_h