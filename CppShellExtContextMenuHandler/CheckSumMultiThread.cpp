#include "CheckSumMultiThread.h"

#define buffSize 4096

CheckSumMultiThread::CheckSumMultiThread(std::list<FileInfo> &fileL) :
	fileList(fileL),
	currentCheckSum(fileL.begin())
{
	start();
}

CheckSumMultiThread::~CheckSumMultiThread()
{	
	trGroup.join_all();	
}

void CheckSumMultiThread::start()
{	
	int threadNumber = boost::thread::hardware_concurrency() - 1;
	if (threadNumber == 0)
		threadNumber++;

	for (int i = 0; i < threadNumber; i++)		
		trGroup.add_thread(new boost::thread(&CheckSumMultiThread::threadFunc, this));
}

int32_t CheckSumMultiThread::findCheckSum(const std::wstring &path)
{			
	std::fstream file;

	int32_t sum = 0;
	char buff[buffSize];
	int32_t temp[4], T;
	
	file.open(path, std::ios_base::binary | std::ios_base::in);

	while (file.good())
	{
		int i=0;
		file.read(buff, sizeof(buff));
		std::streamsize wasRead = file.gcount();
		while (wasRead > i)
		{
			T = 0;
			for (int j = 0; j < 4; j++, i++)
			{
				temp[j] = (i<wasRead) ? buff[i] : 0;
				T += temp[j] << (4-j);
			}
			sum = sum ^ T;
		}
	}		
	
	file.clear();
	file.close();	

	return sum;	
}

void CheckSumMultiThread::threadFunc()
{
	std::list<FileInfo>::iterator it;		
	bool flag = true;

	while (flag)
	{
		this->mtx.lock();
		if (this->currentCheckSum != this->fileList.end())						
			it = (this->currentCheckSum)++;
		else
			flag = false;
		this->mtx.unlock();
		if (!flag)
			break;
		it->m_checkSum = findCheckSum(it->m_path);
		it->m_ready = true;
		it->m_cond.notify_one();
	}
}