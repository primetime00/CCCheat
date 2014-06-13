#include "DumpMemory.h"
#include "Common.h"
#include <iostream>
#include <cstdio>
#include <fstream>

using namespace std;

DumpMemory::DumpMemory(string ip, int ccapiVersion, unsigned long long offset, unsigned long long length)
{
	m_ip = ip;
	m_offset = offset;
	m_length = length;
	m_threadProgressList.clear();
	m_threadCount = 1;
	m_error = TASK_ERROR_NONE;
	m_status = "INIT";
	m_running = false;
	m_ccapiHostVersion = ccapiVersion;
}

DumpMemory::~DumpMemory()
{
	for (unsigned int i=0; i<m_threadList.size(); i++)
	{
		m_threadList.at(i).get()->cancel();
	}
	for (unsigned int j=0; j<m_threadList.size(); j++)
		m_threadList.at(j)->join();
	m_threadList.clear(); //should remove all the thread pointers!
}
void DumpMemory::calculateThreads()
{
	if (m_length >= 0x800000)
		m_threadCount = 4;
	else if (m_length >= 0x200000)
		m_threadCount = 2;
	else
		m_threadCount = 1;
}

void DumpMemory::progressCallBack(DumpTask* task, long long current, long long total)
{
	bool done = true;
	bool found = false;
	bool check = false;
	for (ProgressList::iterator it = m_threadProgressList.begin(); it != m_threadProgressList.end(); ++it)
	{
		if (task == it->first) //we have it!
		{
			found = true;
			it->second.first = current;
			it->second.second = total;
			break;
		}
	}
	if (!found)
		m_threadProgressList.push_back(make_pair(task, make_pair(current, total)));
	check = task->getState() == Task::COMPLETE;
	if (!check)
		return;
	for (unsigned int i=0; i<m_threadList.size(); i++)
	{
		if (m_threadList.at(i).get()->getState() != Task::COMPLETE)
		{
			done = false;
			break;
		}
	}
	if (done)
	{
		m_status = "DONE";
		m_running = false;
	}
}

float DumpMemory::getProgress()
{
	unsigned long long current=0, total = 0;
	if (m_threadProgressList.size() != m_threadList.size())
		return 0.0f;
	if (m_status == "INIT")
		return 0.0f;
	for (ProgressList::iterator it = m_threadProgressList.begin(); it != m_threadProgressList.end(); ++it)
	{
		current+= it->second.first;
		total += it->second.second;
	}
	return (float) (current / (double)total);
}

unsigned int DumpMemory::process()
{
	shared_ptr<DumpTask> dumpTask;
	m_status = "CONNECTING";
	m_cancel = false;
	m_running = true;
	m_threadProgressList.clear();
	if (m_threadList.empty())
	{
		calculateThreads();
		unsigned long long minSize = (unsigned long long) (m_length / (double)m_threadCount);
		unsigned long long minRemain = (unsigned long long) (m_length % m_threadCount);
		for (int i = 0; i<m_threadCount; i++)
		{
			if (m_cancel)
			{
				m_running = false;
				return false;
			}
			string name = "DUMPTASK_"+to_string(i);
			if (i == m_threadCount-1)
			{
				dumpTask = make_shared<DumpTask>(m_ip, m_offset+(i*minSize), minSize+minRemain, i, name);
			}
			else
				dumpTask = make_shared<DumpTask>(m_ip, m_offset+(i*minSize), minSize, i, name);
			dumpTask->setHostCCAPIVersion(m_ccapiHostVersion);			
			m_threadList.push_back(dumpTask);
			dumpTask->setProgressCallback(bind(&DumpMemory::progressCallBack, this, dumpTask.get(), placeholders::_2, placeholders::_3));
			dumpTask->setErrorCallback(bind(&DumpMemory::getError, this));
			dumpTask->start();
			::this_thread::sleep_for(::chrono::milliseconds(500));
		}
	}
	else
	{
		for (int i = 0; i<m_threadCount; i++)
		{
			m_threadList.at(i)->join();
		}
		for (int i = 0; i<m_threadCount; i++)
		{
			m_threadList.at(i)->start();
			::this_thread::sleep_for(::chrono::milliseconds(500));
		}
	}
	while (true)
	{
		bool ready = true;
		for (unsigned int i=0; i<m_threadList.size(); i++)
		{
			if (m_threadList.at(i)->getStatus() != "CONNECTED")
			{
				if (m_threadList.at(i)->getStatus() == "ERROR")
				{
					switch (m_threadList.at(i)->getError())
					{
					case TASK_ERROR_NO_ATTACH: m_error = TASK_ERROR_NO_ATTACH; break;
					case TASK_ERROR_NO_CONNECT: m_error = TASK_ERROR_NO_CONNECT; break;
					case TASK_ERROR_CANCEL: m_error = TASK_ERROR_CANCEL; break;
					default: break;
					}
					for (unsigned int j=0; j<m_threadList.size(); j++)
						m_threadList.at(j)->join();
					m_threadList.clear();
					m_running = false;
					return m_error;
				}
				ready = false;
				break;
			}
		}
		if (ready) break;
		::this_thread::sleep_for(::chrono::milliseconds(100));
	}
	if (m_status != "DONE")
		m_status = "DUMPMEMORY";
	for (unsigned int i=0; i<m_threadList.size(); i++)
	{
		::this_thread::sleep_for(::chrono::milliseconds(50));
		cancelLock.lock();
		if (m_cancel)
		{
			cancelLock.unlock();
			return TASK_ERROR_CANCEL;
		}
		cancelLock.unlock();
		//if (i != m_threadList.size()-1)
			m_threadList.at(i)->fire();
	}
	return TASK_ERROR_NONE;
}

void DumpMemory::cancel() 
{
	cancelLock.lock();
	m_cancel = true;
	cancelLock.unlock();
	cout << "CANCELED!" << endl;
	for (unsigned int i=0; i<m_threadList.size(); i++)
	{
		m_threadList.at(i).get()->cancel();
	}
	for (unsigned int j=0; j<m_threadList.size(); j++)
		m_threadList.at(j)->join();
	m_threadList.clear();
	m_status = "CANCEL";
}

int DumpMemory::writeDump(string fname)
{
	ofstream oFile(fname, ofstream::binary);
	if (!oFile)
		return TASK_ERROR_FILE_FAIL;
	DumpHeader header(m_offset, m_offset+m_length, 0);
	oFile.write((char*)&header, sizeof(DumpHeader));
	for (int i = 0; i<m_threadCount; i++)
	{
		char *b = m_threadList.at(i)->getBuffer();
		oFile.write(b, m_threadList.at(i)->getLength());
	}
	oFile.close();
}