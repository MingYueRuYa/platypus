#include "PEVersionInfo.h"
#include <limits.h>
#include <time.h>
#pragma comment(lib, "version.lib")

#pragma warning(disable: 4267)
#pragma warning(disable: 4244)

#define ALIGN_SIZE_4(val) (((val) + 4 - 1) & ~(4 - 1))
#define PEVersionNameSize(str) sizeof(str)

#define PEVersionInfoSignature L"VS_VERSION_INFO"
#define PEVersionVarFileInfoSignature L"VarFileInfo"
#define PEVersionTranslationSignature L"Translation"
#define PEVersionStringFileInfoSignature L"StringFileInfo"

struct PEVersionSignature
{
	static const wchar_t version_info[];
	static const wchar_t var_file_info[];
	static const wchar_t translation[];
	static const wchar_t string_file_info[];
};

const wchar_t PEVersionSignature::version_info[] = L"VS_VERSION_INFO";
const wchar_t PEVersionSignature::var_file_info[] = L"VarFileInfo";
const wchar_t PEVersionSignature::translation[] = L"Translation";
const wchar_t PEVersionSignature::string_file_info[] = L"StringFileInfo";

#pragma pack(push)
#pragma pack(1)
struct PEVersionHeader
{
	WORD wLength;
	WORD wValueLength;
	WORD wType;
};
#pragma pack(pop)

static void GetVersionFromDWord(DWORD version, unsigned short* pMajor, unsigned short* pMinor)
{
	char buf[16] = {};
	sprintf_s(buf, "%x", HIWORD(version));
	*pMajor = atoi(buf);
	sprintf_s(buf, "%x", LOWORD(version));
	*pMinor = atoi(buf);
}
static DWORD GetDWordFromVersion(unsigned short major, unsigned short minor)
{
	char buf[16] = {};
	char* ptr;
	sprintf_s(buf, "%d", major);
	major = strtol(buf, &ptr, 10);
	sprintf_s(buf, "%d", minor);
	minor = strtol(buf, &ptr, 10);
	return MAKELPARAM(minor, major);
}
static PEVersion GetPEVersion(DWORD dwMS, DWORD dwLS)
{
	PEVersion version;
	GetVersionFromDWord(dwMS, &version.subversion1, &version.subversion2);
	GetVersionFromDWord(dwLS, &version.subversion3, &version.subversion4);
	return version;
}

static void SetPEVersion(PEVersion version, DWORD* dwMS, DWORD* dwLS)
{
	*dwMS = GetDWordFromVersion(version.subversion1, version.subversion2);
	*dwLS = GetDWordFromVersion(version.subversion3, version.subversion4);
}

static std::wstring GetPEVersionString(PEVersion version)
{
	wchar_t buf[1024];
	swprintf_s(buf, L"%hd.%hd.%hd.%hd", version.subversion1, version.subversion2, version.subversion3, version.subversion4);
	return buf;
}

static void SetPEVersionString(const std::wstring& buf, DWORD* dwMS, DWORD* dwLS)
{
	PEVersion version;
	swscanf_s(buf.data(), L"%hd.%hd.%hd.%hd", &version.subversion1, &version.subversion2, &version.subversion3, &version.subversion4);
	SetPEVersion(version, dwMS, dwLS);
}

static inline DWORD GetWCharSize(const wchar_t* str)
{
	return (wcslen(str) + 1) << 1;
}

static inline DWORD GetWCharSize(const std::wstring& str)
{
	return (str.size() + 1) << 1;
}

static std::string Unicode2Ansi(const std::wstring& str)
{
	std::string tmp;
	int size = WideCharToMultiByte(CP_ACP, 0, str.data(), str.size(), 0, 0, 0, 0);
	tmp.resize(size);
	WideCharToMultiByte(CP_ACP, 0, str.data(), str.size(), (char*)tmp.data(), tmp.size(), 0, 0);
	return std::move(tmp);
}

static const void* MovePointer(const void* pData, size_t offset)
{
	return ((const unsigned char*)pData) + offset;
}

static const wchar_t* GetVersionKey(const void* pData)
{
	return (const wchar_t*)MovePointer(pData, sizeof(PEVersionHeader));
}

static const void* GetVersionValue(const void* pData, DWORD* numOfBytes)
{
	const wchar_t* str = (const wchar_t*)MovePointer(pData, sizeof(PEVersionHeader));
	size_t len = GetWCharSize(str);
	*numOfBytes = ((PEVersionHeader*)pData)->wLength - ALIGN_SIZE_4(sizeof(PEVersionHeader) + len);
	return MovePointer(pData, ALIGN_SIZE_4(sizeof(PEVersionHeader) + len));
}

static const wchar_t* GetVersionStringValue(const void* pData)
{
	const wchar_t* str = (const wchar_t*)MovePointer(pData, sizeof(PEVersionHeader));
	size_t len = GetWCharSize(str);
	return (const wchar_t*)MovePointer(pData, ALIGN_SIZE_4(sizeof(PEVersionHeader) + len));
}

static const PEVersionHeader* GetVersionChildren(const void* pData)
{
	const wchar_t* str = (const wchar_t*)MovePointer(pData, sizeof(PEVersionHeader));
	size_t len = GetWCharSize(str);
	return (const PEVersionHeader*)MovePointer(pData, ALIGN_SIZE_4(sizeof(PEVersionHeader) + len));
}

static const PEVersionHeader* GetVersionInfoChildren(const void* pData)
{
	const wchar_t* str = (const wchar_t*)MovePointer(pData, sizeof(PEVersionHeader));
	size_t len = GetWCharSize(str);
	return (const PEVersionHeader*)MovePointer(pData, ALIGN_SIZE_4(sizeof(PEVersionHeader) + len) + ALIGN_SIZE_4(sizeof(VS_FIXEDFILEINFO)));
}

static bool IsVersionValid(const PEVersionHeader* pParent, const void* pData)
{
	return ((char*)(pData)) - ((char*)(pParent)) < pParent->wLength;
}

static const PEVersionHeader* NextChild(const void* pData)
{
	const PEVersionHeader* pHeader = (const PEVersionHeader*)pData;
	return (const PEVersionHeader*)MovePointer(pData, ALIGN_SIZE_4(pHeader->wLength));
}

static VS_FIXEDFILEINFO GetFixedFileInfo(const PEVersionHeader* pRoot)
{
	const wchar_t* str = (const wchar_t*)MovePointer(pRoot, sizeof(PEVersionHeader));
	size_t len = GetWCharSize(str);
	return *(const VS_FIXEDFILEINFO*)MovePointer(pRoot, ALIGN_SIZE_4(sizeof(PEVersionHeader) + len));
}


//setter

static PEVersionHeader* GetVersionHeaderFromBuffer(std::string& buffer, size_t offset)
{
	return (PEVersionHeader*)(buffer.data() + offset);
}

static void SetVersionStringKey(std::string& buffer, size_t offset, const wchar_t* key)
{
	PEVersionHeader* pHeader = GetVersionHeaderFromBuffer(buffer, offset);
	wchar_t* str = (wchar_t*)MovePointer(pHeader, sizeof(PEVersionHeader));
	std::copy(key, key + wcslen(key), str);
}

static void AddVersionSize(std::string& buffer, std::vector<size_t>& stack, size_t size)
{
	for (size_t i = 0; i < stack.size(); ++i)
	{
		PEVersionHeader* pHeader = GetVersionHeaderFromBuffer(buffer, stack[i]);
		pHeader->wLength += size;
	}
}

static void NewVersionFileInfo(std::string& buffer, std::vector<size_t>& stack, const VS_FIXEDFILEINFO& fixedInfo)
{
	stack.push_back(buffer.size());
	DWORD size = ALIGN_SIZE_4(sizeof(PEVersionHeader) + GetWCharSize(PEVersionSignature::version_info)) + ALIGN_SIZE_4(sizeof(VS_FIXEDFILEINFO));
	buffer.append(size, 0);
	*(VS_FIXEDFILEINFO*)MovePointer(buffer.data(), ALIGN_SIZE_4(sizeof(PEVersionHeader) + GetWCharSize(PEVersionSignature::version_info))) = fixedInfo;
	SetVersionStringKey(buffer, stack.back(), PEVersionSignature::version_info);
	PEVersionHeader* pHeader = GetVersionHeaderFromBuffer(buffer, stack.back());
	pHeader->wLength += size;
	pHeader->wValueLength = sizeof(fixedInfo);
}
static void NewVar(std::string& buffer, std::vector<size_t>& stack, PEVersionCodepage codepage)
{
	stack.push_back(buffer.size());
	DWORD size = ALIGN_SIZE_4(sizeof(PEVersionHeader) + GetWCharSize(PEVersionSignature::translation)) + sizeof(DWORD);
	buffer.append(size, 0);
	SetVersionStringKey(buffer, stack.back(), PEVersionSignature::translation);
	AddVersionSize(buffer, stack, size);
	PEVersionHeader* pHeader = GetVersionHeaderFromBuffer(buffer, stack.back());

	*(DWORD*)MovePointer(pHeader, ALIGN_SIZE_4(sizeof(PEVersionHeader) + GetWCharSize(PEVersionSignature::translation))) = codepage.value;
	pHeader->wValueLength = sizeof(DWORD);

	stack.pop_back();
}

static void NewVarFileInfo(std::string& buffer, std::vector<size_t>& stack, const PEVersionInfo::CodepageMap& codeMap)
{
	stack.push_back(buffer.size());
	DWORD size = ALIGN_SIZE_4(sizeof(PEVersionHeader) + GetWCharSize(PEVersionSignature::var_file_info));
	buffer.append(size, 0);
	SetVersionStringKey(buffer, stack.back(), PEVersionSignature::var_file_info);
	GetVersionHeaderFromBuffer(buffer, stack.back())->wType = 1;
	AddVersionSize(buffer, stack, size);
	for (auto itor : codeMap)
		NewVar(buffer, stack, itor.first);
	stack.pop_back();
}

static void NewString(std::string& buffer, std::vector<size_t>& stack, const std::wstring& key, std::wstring& value)
{
	stack.push_back(buffer.size());
	DWORD size = ALIGN_SIZE_4(sizeof(PEVersionHeader) + GetWCharSize(key)) + ALIGN_SIZE_4(GetWCharSize(value));

	buffer.append(size, 0);
	AddVersionSize(buffer, stack, size);
	PEVersionHeader* pHeader = GetVersionHeaderFromBuffer(buffer, stack.back());
	pHeader->wType = 1;
	pHeader->wValueLength = value.size();
	SetVersionStringKey(buffer, stack.back(), key.data());
	wchar_t* ptr = (wchar_t*)MovePointer(buffer.data(), ALIGN_SIZE_4(sizeof(PEVersionHeader) + GetWCharSize(key)) + stack.back());
	std::copy(value.begin(), value.end(), ptr);
	stack.pop_back();
}

static void NewStringTable(std::string& buffer, std::vector<size_t>& stack, PEVersionCodepage codepage, const PEVersionInfo::StringTable& table)
{
	stack.push_back(buffer.size());
	wchar_t buf[9] = {};
	DWORD value = ((DWORD)codepage.wLangugage << 16) + codepage.wCodepage;
	swprintf_s(buf, L"%08x", value);
	DWORD size = ALIGN_SIZE_4(sizeof(PEVersionHeader) + sizeof(buf));
	buffer.append(size, 0);
	AddVersionSize(buffer, stack, size);
	SetVersionStringKey(buffer, stack.back(), buf);
	GetVersionHeaderFromBuffer(buffer, stack.back())->wType = 1;
	for (auto itor : table)
		NewString(buffer, stack, itor.first, itor.second);
	stack.pop_back();
}

static void NewStringFileInfo(std::string& buffer, std::vector<size_t>& stack, const PEVersionInfo::CodepageMap& codeMap)
{
	stack.push_back(buffer.size());
	DWORD size = ALIGN_SIZE_4(sizeof(PEVersionHeader) + GetWCharSize(PEVersionSignature::string_file_info));
	buffer.append(size, 0);
	SetVersionStringKey(buffer, stack.back(), PEVersionSignature::string_file_info);
	GetVersionHeaderFromBuffer(buffer, stack.back())->wType = 1;
	AddVersionSize(buffer, stack, size);
	for (auto itor: codeMap)
		NewStringTable(buffer, stack, itor.first, itor.second);
	stack.pop_back();
}
//setter**


PEVersionInfo::PEVersionInfo()
	:m_pCurrentStringTable(nullptr), m_dwErrorCode(0)
{
	memset(&m_fixedFileInfo, 0, sizeof(m_fixedFileInfo));
	memset(&m_fileVersion, 0, sizeof(m_fileVersion));
	memset(&m_productVersion, 0, sizeof(m_productVersion));
}

PEVersionInfo::~PEVersionInfo()
{

}

bool PEVersionInfo::loadFromMemory(const void* pData)
{
	parse(pData);
	return true;
}

bool PEVersionInfo::loadFromResource(HINSTANCE hInstance, LPCWSTR id /*= MAKEINTRESOURCE(VS_VERSION_INFO)*/, LPCWSTR type /*= RT_VERSION*/)
{
	HRSRC src = FindResourceW(hInstance, id, type);
	if (src == nullptr)
		return false;
	HGLOBAL hGlobal = LoadResource(hInstance, src);
	if (hGlobal == nullptr)
		return false;
	void* pData = LockResource(hGlobal);
	bool result = loadFromMemory(pData);
	UnlockResource(pData);
	FreeResource(hGlobal);
	return result;
}

bool PEVersionInfo::loadFromPEFile(const std::wstring& path)
{
	DWORD dwHandle;
	DWORD dwSize = GetFileVersionInfoSizeW(path.data(), &dwHandle);
	unsigned char* buf = new unsigned char[dwSize];
	GetFileVersionInfoW(path.data(), dwHandle, dwSize, buf);
	bool result = loadFromMemory(buf);
	delete[] buf;
	return result;
}

bool PEVersionInfo::loadFromPEFile(const std::string& path)
{
	std::wstring str;
	int size = MultiByteToWideChar(CP_ACP, 0, path.data(), path.size(), 0, 0);
	str.resize(size);
	MultiByteToWideChar(CP_ACP, 0, path.data(), path.size(), (wchar_t*)str.data(), str.size());
	return loadFromPEFile(str);
}

void PEVersionInfo::changeCodepage(PEVersionCodepage codepage)
{
	auto itor = m_codepageMap.find(codepage);
	if (itor == m_codepageMap.end())
		return;
	m_pCurrentStringTable = &itor->second;
}

std::wstring PEVersionInfo::getStringValue(const std::wstring& key) const
{
	if (!m_pCurrentStringTable)
		return L"";
	auto itor = m_pCurrentStringTable->find(key);
	if (itor == m_pCurrentStringTable->end())
		return L"";
	return itor->second;
}

void PEVersionInfo::setStringValue(const std::wstring& key, const std::wstring& value)
{
	if (!m_pCurrentStringTable)
		return;
	(*m_pCurrentStringTable)[key] = value;
}

void PEVersionInfo::inspect() const
{
	for (auto itor: m_codepageMap)
	{
		printf("codepage: %d    langugage: %d\n", itor.first.wCodepage, itor.first.wLangugage);
		for (auto itor2 : itor.second)
			printf("%s : \"%s\"\n", Unicode2Ansi(itor2.first).data(), Unicode2Ansi(itor2.second).data());
	}
}

std::string PEVersionInfo::pack() const
{
	std::string buffer;
	std::vector<size_t> stack;

	NewVersionFileInfo(buffer, stack, m_fixedFileInfo);
	NewStringFileInfo(buffer, stack, m_codepageMap);
	NewVarFileInfo(buffer, stack, m_codepageMap);

	return std::move(buffer);
}

bool PEVersionInfo::saveToPEFile(const std::wstring& path) const
{
	std::string buffer = pack();
	HANDLE hRes = BeginUpdateResourceW(path.data(), FALSE);
	if (hRes == nullptr)
		return false;
	UpdateResourceW(hRes, MAKEINTRESOURCEW(16), MAKEINTRESOURCEW(VS_VERSION_INFO), m_codepageMap.begin()->first.wLangugage, (void*)buffer.data(), buffer.size());
	return EndUpdateResourceW(hRes, FALSE) != FALSE;
}

bool PEVersionInfo::saveToPEFile(const std::string& path) const
{
	wchar_t buf[1024] = {};
	MultiByteToWideChar(CP_ACP, 0, path.data(), path.size(), buf, 1024);
	return saveToPEFile(buf);
}

bool PEVersionInfo::hasError() const
{
	return m_dwErrorCode != S_OK;
}

std::string PEVersionInfo::getErrorMessage() const
{
	DWORD dwErrorCode = m_dwErrorCode;
	if (dwErrorCode == S_OK)
		dwErrorCode = GetLastError();
	char buf[1024] = {};
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, dwErrorCode, LANG_USER_DEFAULT, buf, sizeof(buf), nullptr);
	return buf;
}

void PEVersionInfo::reset()
{
	m_codepageMap.clear();
	m_dwErrorCode = 0;
	m_pCurrentStringTable = nullptr;
	memset(&m_fixedFileInfo, 0, sizeof(m_fixedFileInfo));
}

std::wstring PEVersionInfo::getCompanyName() const
{
	return getStringValue(L"CompanyName");
}

void PEVersionInfo::setCompanyName(const std::wstring& val)
{
	setStringValue(L"CompanyName", val);
}

std::wstring PEVersionInfo::getFileDescription() const
{
	return getStringValue(L"FileDescription");
}

void PEVersionInfo::setFileDescription(const std::wstring& val)
{
	setStringValue(L"FileDescription", val);
}

std::wstring PEVersionInfo::getFileVersion() const
{
	return getStringValue(L"FileVersion");
}

void PEVersionInfo::setFileVersion(const std::wstring& val)
{
	setStringValue(L"FileVersion", val);
	SetPEVersionString(val, &m_fixedFileInfo.dwFileVersionMS, &m_fixedFileInfo.dwFileVersionLS);
}

PEVersion PEVersionInfo::getFileVersionValue() const
{
	return GetPEVersion(m_fixedFileInfo.dwFileVersionMS, m_fixedFileInfo.dwFileVersionLS);
}

void PEVersionInfo::setFileVersionValue(PEVersion version)
{
	SetPEVersion(version, &m_fixedFileInfo.dwFileVersionMS, &m_fixedFileInfo.dwFileVersionLS);
	setFileVersion(GetPEVersionString(version));
}

void PEVersionInfo::setFileVersionValue(unsigned short(&version)[4])
{
	PEVersion ver;
	std::copy(version, version + 4, ver.subversion);
	setFileVersionValue(ver);
}

PEVersion PEVersionInfo::getProductVersionValue() const
{
	return GetPEVersion(m_fixedFileInfo.dwProductVersionMS, m_fixedFileInfo.dwProductVersionLS);
}

void PEVersionInfo::setProductVersionValue(PEVersion version)
{
	SetPEVersion(version, &m_fixedFileInfo.dwProductVersionMS, &m_fixedFileInfo.dwProductVersionLS);
	setProductVersion(GetPEVersionString(version));
}

void PEVersionInfo::setProductVersionValue(unsigned short(&version)[4])
{
	PEVersion ver;
	std::copy(version, version + 4, ver.subversion);
	setProductVersionValue(ver);
}

std::wstring PEVersionInfo::getSquirrelAwareVersion() const
{
	return getStringValue(L"SquirrelAwareVersion");
}

void PEVersionInfo::setSquirrelAwareVersion(const std::wstring& version)
{
	setStringValue(L"SquirrelAwareVersion", version);
}

void PEVersionInfo::setFileFlags(FileFlags flag)
{
	m_fixedFileInfo.dwFileFlags = flag;
}

PEVersionInfo::FileFlags PEVersionInfo::getFileFlags() const
{
	return m_fixedFileInfo.dwFileFlags;
}

void PEVersionInfo::setFileOS(FileOSs os)
{
	m_fixedFileInfo.dwFileOS = os;
}

PEVersionInfo::FileOSs PEVersionInfo::getFileOS() const
{
	return m_fixedFileInfo.dwFileOS;
}

void PEVersionInfo::setFileType(FileTypes types)
{
	m_fixedFileInfo.dwFileType = types;
}

PEVersionInfo::FileTypes PEVersionInfo::getFileType() const
{
	return m_fixedFileInfo.dwFileType;
}

void PEVersionInfo::setFileSubType(FileSubTypes subTypes)
{
	m_fixedFileInfo.dwFileSubtype = subTypes;
}

PEVersionInfo::FileSubTypes PEVersionInfo::getFileSubType() const
{
	return m_fixedFileInfo.dwFileSubtype;
}

void PEVersionInfo::setFileDate(__int64 time)
{
	m_fixedFileInfo.dwFileDateMS = (time & 0xFFFFFFFF00000000) >> 32;
	m_fixedFileInfo.dwFileDateLS = time & 0xFFFFFFFF;
}

__int64 PEVersionInfo::getFileDate() const
{
	__int64 val = 0;
	val |= (__int64)m_fixedFileInfo.dwFileDateMS << 32;
	val |= m_fixedFileInfo.dwFileVersionLS;
	return val;
}

void PEVersionInfo::setFileDateNow()
{
	__time64_t time = ::_time64(0);
	setFileDate(time);
}

std::wstring PEVersionInfo::getProductVersion() const
{
	return getStringValue(L"ProductVersion");
}

void PEVersionInfo::setProductVersion(const std::wstring& val)
{
	setStringValue(L"ProductVersion", val);
	SetPEVersionString(val, &m_fixedFileInfo.dwProductVersionMS, &m_fixedFileInfo.dwProductVersionLS);
}

std::wstring PEVersionInfo::getProductName() const
{
	return getStringValue(L"ProductName");
}

void PEVersionInfo::setProductName(const std::wstring& val)
{
	setStringValue(L"ProductName", val);
}

std::wstring PEVersionInfo::getInternalName() const
{
	return getStringValue(L"InternalName");
}

void PEVersionInfo::setInternalName(const std::wstring& val)
{
	setStringValue(L"InternalName", val);
}

std::wstring PEVersionInfo::getLegalCopyright() const
{
	return getStringValue(L"LegalCopyright");
}

void PEVersionInfo::setLegalCopyright(const std::wstring& val)
{
	setStringValue(L"LegalCopyright", val);
}

std::wstring PEVersionInfo::getOriginalFilename() const
{
	return getStringValue(L"OriginalFilename");
}

void PEVersionInfo::setOriginalFilename(const std::wstring& val)
{
	setStringValue(L"OriginalFilename", val);
}

void PEVersionInfo::parse(const void* pData)
{
	const PEVersionHeader* pRoot = (const PEVersionHeader*)pData;
	m_fixedFileInfo = GetFixedFileInfo(pRoot);

	m_fileVersion = GetPEVersion(m_fixedFileInfo.dwFileVersionMS, m_fixedFileInfo.dwFileVersionLS);
	m_productVersion = GetPEVersion(m_fixedFileInfo.dwProductVersionMS, m_fixedFileInfo.dwProductVersionLS);

	const PEVersionHeader* pChild = GetVersionInfoChildren(pRoot);
	while (IsVersionValid(pRoot, pChild))
	{
		const wchar_t* key = GetVersionKey(pChild);
		if (wcscmp(key, PEVersionSignature::string_file_info) == 0)
			parseStringFileInfo(pChild);
		else if(wcscmp(key, PEVersionSignature::var_file_info) == 0)
			parseVarFileInfo(pChild);
		pChild = NextChild(pChild);
	}

	if (m_pCurrentStringTable == nullptr && !m_codepageMap.empty())
		m_pCurrentStringTable = &m_codepageMap.begin()->second;
}

void PEVersionInfo::parseVarFileInfo(const void* pData)
{
	const PEVersionHeader* pParent = (const PEVersionHeader*)pData;
	const PEVersionHeader* pChild = GetVersionChildren(pParent);

	while (IsVersionValid(pParent, pChild))
	{
		const wchar_t* key = GetVersionKey(pChild);
		if (wcscmp(key, PEVersionSignature::translation) == 0)
			parseVar(pChild);
		pChild = NextChild(pChild);
	}
}

void PEVersionInfo::parseVar(const void* pData)
{
	DWORD numBytes;
	PEVersionCodepage codepage;
	codepage.value = *(const DWORD*)GetVersionValue(pData, &numBytes);
	auto itor = m_codepageMap.find(codepage);
	if(itor == m_codepageMap.end())
		m_codepageMap.insert(CodepageMap::value_type(codepage, StringTable()));
}

void PEVersionInfo::parseStringFileInfo(const void* pData)
{
	const PEVersionHeader* pParent = (const PEVersionHeader*)pData;
	const PEVersionHeader* pChild = GetVersionChildren(pParent);

	while (IsVersionValid(pParent, pChild))
	{
		parseStringTable(pChild);
		pChild = NextChild(pChild);
	}
}

void PEVersionInfo::parseStringTable(const void* pData)
{
	const PEVersionHeader* pParent = (const PEVersionHeader*)pData;
	const PEVersionHeader* pChild = GetVersionChildren(pParent);

	PEVersionCodepage codepage;
	wchar_t* ptr;
	DWORD value = wcstol(GetVersionKey(pParent), &ptr, 16);
	codepage.wCodepage = LOWORD(value);
	codepage.wLangugage = HIWORD(value);
	CodepageMap::iterator itor = m_codepageMap.find(codepage);
	if (itor == m_codepageMap.end())
		itor = m_codepageMap.insert(CodepageMap::value_type(codepage, StringTable())).first;
	StringTable& table = itor->second;
	while (IsVersionValid(pParent, pChild))
	{
		parseString(pChild, table);
		pChild = NextChild(pChild);
	}
}

void PEVersionInfo::parseString(const void* pData, StringTable& table)
{
	const PEVersionHeader* pNode = (const PEVersionHeader*)pData;

	table.insert(StringTable::value_type(
		GetVersionKey(pNode),
		GetVersionStringValue(pNode)
	));
}