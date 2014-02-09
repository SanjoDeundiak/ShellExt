#pragma once

#include "boost/cstdint.hpp"
#include "boost/thread/condition_variable.hpp"
#include <windows.h>
#include <string>
#include <strsafe.h>
#include <fstream>
 
class FileInfo
{	
	friend class CheckSumMultiThread;
	public:

		FileInfo(const FileInfo& other);

		FileInfo();
		FileInfo(const wchar_t *path);		
		
		~FileInfo();

		bool operator<(const FileInfo &other) const;
		FileInfo &operator=(FileInfo &other);

		const wchar_t *path() const;
		int32_t checkSum() const;
		const std::wstring getCreationTime() const;
		bool ready() const;
		std::locale loc; // for std::to_upper(char)
		boost::condition_variable *cond;

	private:		
		std::wstring TimeToString(const SYSTEMTIME creationTime) const;
		std::wstring CreationTime(const wchar_t* fileName) const;

		wchar_t* m_path;
		int32_t m_checkSum;	
		std::wstring m_creationTime;
		bool m_ready;
};