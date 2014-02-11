#pragma once

#include "boost/thread.hpp"
#include "boost/thread/thread.hpp"
#include "boost/cstdint.hpp"
#include "FileInfo.h"
#include <fstream>
#include <list>

class CheckSumMultiThread
{
	public:		
		CheckSumMultiThread(std::list<FileInfo> &pfileList);
		~CheckSumMultiThread();

	private:
		void start();
		void threadFunc();
		int32_t findCheckSum(std::wstring path);		

		boost::thread_group trGroup;
		boost::mutex mtx;

		std::list<FileInfo> &fileList;
		std::list<FileInfo>::iterator currentCheckSum;		
};