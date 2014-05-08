#include "RangeMemory.h"
#include <iostream>

using namespace std;

RangeMemory::RangeMemory(string ip, unsigned long long offset, unsigned long long length)
{
	m_ip = ip;
	m_offset = offset;
	m_length = length;
	memset(m_totals, 0, sizeof(long long)*MAX_THREADS);
	m_threadCount = 1;
	m_error = RANGEMEMORY_ERROR_NONE;

}

void RangeMemory::calculateThreads()
{
	if (m_length >= 0x800000)
		m_threadCount = 8;
	else if (m_length >= 0x200000)
		m_threadCount = 4;
	else
		m_threadCount = 1;
/*
#ifdef _HOME
	m_threadCount = 1;
#endif*/
}

void RangeMemory::cancel() 
{
	m_cancel = true;
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
void RangeMemory::callBack(RangeTask* task, long long current, long long size)
{
	bool done = true;
	for (unsigned int i=0; i<m_threadList.size(); i++)
	{
		if (m_threadList.at(i).get() == task)
		{
			m_totals[i] = current;
		}
		if (m_totals[i] != m_threadList.at(i)->getLength())
			done = false;
	}
	if (done)
		m_status = "DONE";
}

float RangeMemory::getProgress()
{
	long long total = 0;
	for (int i=0; i<m_threadCount; i++)
	{
		total += m_totals[i];
	}
	return (float) (total / (double)m_length);
}

RangeList RangeMemory::getRanges() 
{
	if (m_threadList.size() == 1)
	{
		m_ranges = m_threadList.at(0)->getIntervals();
		int last = m_ranges.size()-1;
		if (m_ranges.at(last).second == -1)
				m_ranges.at(last).second = m_threadList.at(0)->getLastOffset();
		if (m_ranges.at(last).first == -1)
				m_ranges.at(0).first = m_threadList.at(0)->getFirstOffset();
		return m_ranges;
	}
	for (int i = m_threadList.size()-1; i>0; i--)
	{
		RangeTask *cur = m_threadList.at(i).get();
		if (cur->getIntervals().size() == 0)
			continue;
		for (int j = i-1; j>=0; j--)
		{
			RangeTask *prev = m_threadList.at(i-1).get();
			if (prev->getIntervals().size() == 0)
				continue;
			int last = prev->getIntervals().size()-1;
			if (prev->getIntervals().at(last).second == -1 && cur->getFirstOffset() == cur->getIntervals().at(0).first) //this is a continuation!
			{
				prev->getIntervals().at(last).second = cur->getIntervals().at(0).second;
				cur->getIntervals().erase(cur->getIntervals().begin());
			}
			else if (prev->getIntervals().at(last).second == -1)
			{
				prev->getIntervals().at(last).second = cur->getFirstOffset();
				//cur->getIntervals().erase(cur->getIntervals().begin());
			}
		}
	}
	m_ranges.clear();
	for (unsigned int i = 0; i < m_threadList.size(); i++)
	{
		for (unsigned int j = 0; j < m_threadList.at(i)->getIntervals().size(); j++)
		{
			m_ranges.push_back(m_threadList.at(i)->getIntervals().at(j));
		}
	}
	return m_ranges;
}

unsigned int RangeMemory::process()
{
	shared_ptr<RangeTask> rangeTask;
	calculateThreads();
	unsigned long long minSize = (unsigned long long) (m_length / (double)m_threadCount);
	unsigned long long minRemain = (unsigned long long) (m_length % m_threadCount);
	m_cancel = false;
	m_status = "CONNECTING";
	for (int i = 0; i<m_threadCount; i++)
	{
		if (m_cancel)
			return false;
		string name = "RANGETASK_"+to_string(i);
		if (i == m_threadCount-1)
			rangeTask = make_shared<RangeTask>(m_ip, m_offset+(i*minSize), minSize+minRemain, i, name);
		else
			rangeTask = make_shared<RangeTask>(m_ip, m_offset+(i*minSize), minSize, i, name);
		m_threadList.push_back(rangeTask);
		rangeTask->setCallback(bind(&RangeMemory::callBack, this, rangeTask.get(), placeholders::_2, placeholders::_3));
		rangeTask->setErrorCallback(bind(&RangeMemory::getError, this));
		rangeTask->start();
		::this_thread::sleep_for(::chrono::milliseconds(500));
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
					case TASK_ERROR_NO_ATTACH: m_error = RANGEMEMORY_ERROR_NO_ATTACH; break;
					case TASK_ERROR_NO_CONNECT: m_error = RANGEMEMORY_ERROR_NO_CONNECT; break;
					default: break;
					}
					for (unsigned int j=0; j<m_threadList.size(); j++)
						m_threadList.at(j)->join();
					m_threadList.clear();
					return m_error;
				}
				ready = false;
				break;
			}
		}
		if (ready) break;
		::this_thread::sleep_for(::chrono::milliseconds(100));
	}
	m_status = "FINDRANGE";
	for (unsigned int i=0; i<m_threadList.size(); i++)
	{
		m_threadList.at(i)->fire();
		::this_thread::sleep_for(::chrono::milliseconds(100));
	}
	return RANGEMEMORY_ERROR_NONE;
}

int sample_main()
{
	unsigned int status;
	RangeMemory rm("127.0.0.1", 0, 0x100000000);
	status = rm.process();
	switch (status)
	{
	case RANGEMEMORY_ERROR_NO_CONNECT:
		cout << "Could not connect all threads!" << endl;
		return -1;
		break;
	case RANGEMEMORY_ERROR_NO_ATTACH:
		cout << "Could not attach all threads!" << endl;
		return -1;
		break;
	default: break;
	}
	int sec = 0;
	while (1)
	{
		::this_thread::sleep_for(::chrono::milliseconds(1000));
		cout << "PROGRESS: " << sec++ << " " << rm.getProgress() << endl;
		if (rm.getStatus() == "DONE")
			break;
	}
	vector<pair<long long, long long>> z = rm.getRanges();
	cout << "ALL DONE" << endl;
}