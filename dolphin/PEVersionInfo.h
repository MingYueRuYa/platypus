#ifndef __PEVERSIONINFO_H__
#define __PEVERSIONINFO_H__
#include <Windows.h>
#include <vector>
#include <unordered_map>
#include <type_traits>

#pragma push_macro("PEDEFINE_ENUMS")
#ifdef PEDEFINE_ENUMS
#undef PEDEFINE_ENUMS
#endif
#define PEDEFINE_ENUMS(type) \
struct type##s \
{ \
	type##s(type val): value(val) {} \
	type##s(int val): value(val) {} \
	operator type() const { return (type)value; }; \
	int value; \
};

struct PEVersionCodepage
{
	union
	{
		struct
		{
			WORD wLangugage;
			WORD wCodepage;
		};
		DWORD value;
	};

	size_t operator()(const PEVersionCodepage& codepage) const
	{
		return std::hash<DWORD>()(codepage.value);
	}

	bool operator==(const PEVersionCodepage& codepage) const
	{
		return value == codepage.value;
	}

	bool operator!=(const PEVersionCodepage& codepage) const
	{
		return value != codepage.value;
	}
};

struct PEVersion
{
	union
	{
		struct
		{
			unsigned short subversion1;
			unsigned short subversion2;
			unsigned short subversion3;
			unsigned short subversion4;
		};
		unsigned short subversion[4];
		DWORD version[2];
	};
};

class PEVersionInfo
{
public:
	enum FileFlag
	{
		PEFlag_DEBUG = VS_FF_DEBUG,
		PEFlag_PRERELEASE = VS_FF_PRERELEASE,
		PEFlag_PATCHED = VS_FF_PATCHED,
		PEFlag_PRIVATEBUILD = VS_FF_PRIVATEBUILD,
		PEFlag_INFOINFERRED = VS_FF_INFOINFERRED,
		PEFlag_SPECIALBUILD = VS_FF_SPECIALBUILD
	};
	PEDEFINE_ENUMS(FileFlag);

	enum FileOS
	{
		PEOS_DOS_WINDOWS16 = VOS_DOS_WINDOWS16,
		PEOS_DOS_WINDOWS32 = VOS_DOS_WINDOWS32,
		PEOS_OS216_PM16 = VOS_OS216_PM16,
		PEOS_OS232_PM32 = VOS_OS232_PM32,
		PEOS_NT_WINDOWS32 = VOS_NT_WINDOWS32
	};
	PEDEFINE_ENUMS(FileOS);

	enum FileType
	{
		PEType_UNKNOWN = VFT_UNKNOWN,
		PEType_APP = VFT_APP,
		PEType_DLL = VFT_DLL,
		PEType_DRV = VFT_DRV,
		PEType_FONT = VFT_FONT,
		PEType_VXD = VFT_VXD,
		PEType_STATIC_LIB = VFT_STATIC_LIB
	};
	PEDEFINE_ENUMS(FileType);

	enum FileSubType
	{
		PESubType_UNKNOWN = VFT2_UNKNOWN,
		PESubType_DRV_PRINTER = VFT2_DRV_PRINTER,
		PESubType_DRV_KEYBOARD = VFT2_DRV_KEYBOARD,
		PESubType_DRV_LANGUAGE = VFT2_DRV_LANGUAGE,
		PESubType_DRV_DISPLAY = VFT2_DRV_DISPLAY,
		PESubType_DRV_MOUSE = VFT2_DRV_MOUSE,
		PESubType_DRV_NETWORK = VFT2_DRV_NETWORK,
		PESubType_DRV_SYSTEM = VFT2_DRV_SYSTEM,
		PESubType_DRV_INSTALLABLE = VFT2_DRV_INSTALLABLE,
		PESubType_DRV_SOUND = VFT2_DRV_SOUND,
		PESubType_DRV_COMM = VFT2_DRV_COMM,
		PESubType_DRV_INPUTMETHOD = VFT2_DRV_INPUTMETHOD,
		PESubType_DRV_VERSIONED_PRINTER = VFT2_DRV_VERSIONED_PRINTER,
		PESubType_FONT_RASTER = VFT2_FONT_RASTER,
		PESubType_FONT_TRUETYPE = VFT2_FONT_TRUETYPE,
		PESubType_FONT_VECTOR = VFT2_FONT_VECTOR
	};
	PEDEFINE_ENUMS(FileSubType);

	typedef std::unordered_map<std::wstring, std::wstring> StringTable;
	typedef std::unordered_map<PEVersionCodepage, StringTable, PEVersionCodepage> CodepageMap;

	PEVersionInfo();
	~PEVersionInfo();

	bool loadFromMemory(const void* pData);
	bool loadFromResource(HINSTANCE hInstance, LPCWSTR id = MAKEINTRESOURCEW(VS_VERSION_INFO), LPCWSTR type = MAKEINTRESOURCEW(16));
	bool loadFromPEFile(const std::wstring& path);
	bool loadFromPEFile(const std::string& path);

	void changeCodepage(PEVersionCodepage codepage);

	std::wstring getStringValue(const std::wstring& key) const;
	void setStringValue(const std::wstring& key, const std::wstring& value);

	void inspect() const;

	std::string pack() const;
	bool saveToPEFile(const std::wstring& path) const;
	bool saveToPEFile(const std::string& path) const;

	bool hasError() const;
	std::string getErrorMessage() const;
	void reset();

	std::wstring getCompanyName() const;
	void setCompanyName(const std::wstring& val);

	std::wstring getFileDescription() const;
	void setFileDescription(const std::wstring& val);

	std::wstring getFileVersion() const;
	void setFileVersion(const std::wstring& val);

	std::wstring getProductVersion() const;
	void setProductVersion(const std::wstring& val);

	std::wstring getProductName() const;
	void setProductName(const std::wstring& val);

	std::wstring getInternalName() const;
	void setInternalName(const std::wstring& val);

	std::wstring getLegalCopyright() const;
	void setLegalCopyright(const std::wstring& val);

	std::wstring getOriginalFilename() const;
	void setOriginalFilename(const std::wstring& val);

	PEVersion getFileVersionValue() const;
	void setFileVersionValue(PEVersion version);
	PEVersion getProductVersionValue() const;
	void setProductVersionValue(PEVersion version);
	void setFileVersionValue(unsigned short(&version)[4]);
	void setProductVersionValue(unsigned short(&version)[4]);

	std::wstring getSquirrelAwareVersion() const;
	void setSquirrelAwareVersion(const std::wstring& version);

	void setFileFlags(FileFlags flags);
	FileFlags getFileFlags() const;

	void setFileOS(FileOSs os);
	FileOSs getFileOS() const;
	void setFileType(FileTypes types);
	FileTypes getFileType() const;
	void setFileSubType(FileSubTypes subTypes);
	FileSubTypes getFileSubType() const;

	void setFileDate(__int64 time);
	__int64 getFileDate() const;
	void setFileDateNow();
private:
	void parse(const void* pData);
	void parseVarFileInfo(const void* pData);
	void parseVar(const void* pData);
	void parseStringFileInfo(const void* pData);
	void parseStringTable(const void* pData);
	void parseString(const void* pData, StringTable& table);

	CodepageMap m_codepageMap;
	VS_FIXEDFILEINFO m_fixedFileInfo;
	StringTable* m_pCurrentStringTable;
	DWORD m_dwErrorCode;
	PEVersion m_fileVersion;
	PEVersion m_productVersion;
};
#pragma pop_macro("PEDEFINE_ENUMS")
#endif