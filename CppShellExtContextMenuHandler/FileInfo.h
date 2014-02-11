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

		//FileInfo();
		FileInfo(const wchar_t *path);		
		
		//~FileInfo();

		bool operator<(const FileInfo &other) const;
		FileInfo &operator=(FileInfo &other);

		std::wstring path() const;
		int32_t checkSum() const;
		const std::wstring getCreationTime() const;
		void FileInfo::wait();
		bool ready() const;

	private:		
		std::wstring TimeToString(const SYSTEMTIME &creationTime) const;
		std::wstring CreationTime(const wchar_t* fileName) const;

		std::wstring m_path;
		int32_t m_checkSum;	
		std::wstring m_creationTime;

		boost::mutex mut; // For checksum sleep

		bool m_ready;
		boost::condition_variable m_cond;
};