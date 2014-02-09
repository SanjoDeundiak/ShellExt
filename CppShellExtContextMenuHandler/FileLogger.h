#pragma once

#pragma warning(push)
#pragma warning(disable: 4995)

#include <fstream> //for file output
#include <iomanip> //setw, setfill

#pragma warning(pop)

#include <list>
#include "FileInfo.h"
#include "CheckSumMultiThread.h"

#include "boost/thread.hpp"
#include "boost/thread/thread.hpp"

class FileLogger
{
	public:
		FileLogger(std::list<FileInfo> &fileL);

	private:		
		void start();
		std::wstring ShowCheckSum(int32_t checkSum);
		std::list<FileInfo> &filesList;
		std::locale loc;
		boost::mutex mut; // For checksum sleep
		
};