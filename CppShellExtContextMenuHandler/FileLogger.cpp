#include "FileLogger.h"
#include "boost/filesystem.hpp"
#include <fstream>

FileLogger::FileLogger(std::list<FileInfo> &filesL) : 
	filesList(filesL),
	loc("")
{
	start();
}

void FileLogger::start()
{
	//Detecting folder where the first file is
	boost::filesystem::path dir(filesList.begin()->path());
	dir = dir.parent_path();		

	// Creating and opening log file
	std::wofstream logFile;	
	logFile.open(dir.wstring() + L"\\log.txt", std::ios::out); 
		
	// Sort
	filesList.sort();

	logFile.imbue(loc);
	if (logFile.good())
		logFile << L"Total " << filesList.size() << L" file(s)!" << std::endl;													
		
	CheckSumMultiThread threads(filesList);	

	for (auto j = filesList.begin(); 
		logFile.good() && j != filesList.end();
		++j)
	{																								
		j->wait();
		
		logFile << j->path()
				<< L" CheckSum: " << ShowCheckSum(j->checkSum())
				<< L" Size: " << boost::filesystem::file_size(j->path()) << L" bytes"					
				<< L" Creation time: " << j->getCreationTime()
				<< std::endl;			
	}				
	
	logFile.close();
}

std::wstring FileLogger::ShowCheckSum(int32_t checkSum)
{
	std::wstringstream ss;
	ss << L"0x" << std::uppercase << std::hex << std::setfill(L'0')
	   << std::setw(8) << checkSum;	
	return ss.str();
}