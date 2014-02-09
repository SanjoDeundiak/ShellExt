#include "FileInfo.h"
#include <locale> //for char_type toupper(char_type)

FileInfo::FileInfo(const FileInfo& other) :
	m_checkSum(other.m_checkSum),
	m_creationTime(other.m_creationTime),
	m_ready(other.m_ready),
	loc(other.loc)
{
	cond = new boost::condition_variable;
	size_t length;
	StringCchLengthW(other.m_path, MAX_PATH, &length);
	m_path = new wchar_t[length + 1];
	StringCchCopyW(m_path, length + 1, other.m_path);
}

FileInfo::FileInfo(const wchar_t *path) : m_checkSum(0),
	m_path(NULL),
	m_ready(false),
	loc("")
{
	cond = new boost::condition_variable;
	size_t length;
	StringCchLengthW(path, MAX_PATH, &length);
		
	m_path = new wchar_t[length + 1];
	StringCchCopyW(m_path, length + 1, path);

	m_creationTime = CreationTime(m_path);	
}

bool FileInfo::operator<(const FileInfo &other) const
{
	const wchar_t *st1=this->path(), *st2=other.path();
	int i=0;
	size_t n1 = 0, n2 = 0;

	StringCchLengthW(st1, MAX_PATH, &n1);
	StringCchLengthW(st1, MAX_PATH, &n2);	
	
	while (i<n1 && i<n2 && std::toupper(st1[i], loc) == std::toupper(st2[i], loc))
		i++;
	return (std::toupper(st1[i], loc) < std::toupper(st2[i], loc));
}

FileInfo &FileInfo::operator=(FileInfo &other)	
{
	loc = other.loc;
	m_ready = other.m_ready;
	m_checkSum = other.m_checkSum;
	m_creationTime = other.m_creationTime;	
	cond = new boost::condition_variable;

	size_t length;
	StringCchLengthW(m_path, MAX_PATH, &length);
	other.m_path = new wchar_t[length + 1];
	StringCchCopyW(other.m_path, length + 1, m_path);

	return *this;
}

FileInfo::~FileInfo()
{
	delete m_path;
	m_path = nullptr;

	delete cond;
	cond = nullptr;
}

bool FileInfo::ready() const
{
	return m_ready;
}

const wchar_t *FileInfo::path() const
{
	return m_path;
}

int32_t FileInfo::checkSum() const
{
	return m_checkSum;
}

const std::wstring FileInfo::getCreationTime() const
{
	return m_creationTime;
}


std::wstring FileInfo::TimeToString(const SYSTEMTIME creationTime) const
{
	std::wstring tempSt, creationTimeSt;	
	
	creationTimeSt.clear();

	tempSt = std::to_wstring(DWORDLONG(creationTime.wDay));
	creationTimeSt += (creationTime.wDay>9) ? tempSt : L"0" + tempSt;	
	
	tempSt = std::to_wstring(DWORDLONG(creationTime.wMonth));
	creationTimeSt += L"/" + ((creationTime.wMonth>9) ? tempSt : L"0" + tempSt);
	
	tempSt = std::to_wstring(DWORDLONG(creationTime.wYear));
	creationTimeSt += L"/" + tempSt + L" ";
	
	tempSt = std::to_wstring(DWORDLONG(creationTime.wHour));
	creationTimeSt += (creationTime.wHour>9) ? tempSt : L"0" + tempSt;
	
	tempSt = std::to_wstring(DWORDLONG(creationTime.wMinute));
	creationTimeSt += L":" + ((creationTime.wMinute>9) ? tempSt : L"0" + tempSt);
	
	tempSt = std::to_wstring(DWORDLONG(creationTime.wSecond));
	creationTimeSt += L":" + ((creationTime.wSecond>9) ? tempSt : L"0" + tempSt);
	
	return creationTimeSt;
}

std::wstring FileInfo::CreationTime(const wchar_t* fileName) const
{
	WIN32_FILE_ATTRIBUTE_DATA fileInformation;
	GetFileAttributesExW(fileName, GetFileExInfoStandard, &fileInformation);
	
	FILETIME localFileTime;
	FileTimeToLocalFileTime(&fileInformation.ftCreationTime, &localFileTime);

	SYSTEMTIME creationTime;
	FileTimeToSystemTime(&localFileTime, &creationTime);

	return TimeToString(creationTime);
}