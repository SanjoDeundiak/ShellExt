#include "FileInfo.h"
#include <locale> //for char_type toupper(char_type)
#include "boost/algorithm/string/compare.hpp"

FileInfo::FileInfo(const FileInfo& other) :
	m_checkSum(other.m_checkSum),
	m_creationTime(other.m_creationTime),
	m_ready(other.m_ready),
	m_path(other.m_path)
{
}

FileInfo::FileInfo(const wchar_t *path) : m_checkSum(0),
	m_ready(false),
	m_path(path),
	m_creationTime(CreationTime(path))
{
}

bool FileInfo::operator<(const FileInfo &other) const
{
	return (m_path < other.m_path);
}

FileInfo &FileInfo::operator=(FileInfo &other)	
{
	m_path = other.m_path;
	m_ready = other.m_ready;
	m_checkSum = other.m_checkSum;
	m_creationTime = other.m_creationTime;	
	
	return *this;
}

/*FileInfo::~FileInfo()
{
	delete cond;
	cond = nullptr;
}*/

bool FileInfo::ready() const
{
	return m_ready;
}

std::wstring FileInfo::path() const
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

std::wstring FileInfo::TimeToString(const SYSTEMTIME &creationTime) const
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

void FileInfo::wait()
{
	while (!m_ready)
		boost::unique_lock<boost::mutex> lock(mut);
}