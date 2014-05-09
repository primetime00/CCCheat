#include "SearchMemory.h"
#include "Common.h"
#include <iostream>
#include <cstdio>
#include <fstream>

using namespace std;

SearchMemory::SearchMemory(string ip, unsigned long long offset, unsigned long long length)
{
	m_ip = ip;
	m_offset = offset;
	m_length = length;
	m_threadProgressList.clear();
	m_threadCount = 1;
	m_error = TASK_ERROR_NONE;
	m_status = "INIT";
	m_searchCount = 0;
	m_dump = false;
	m_running = false;
	m_reorganize = false;
}

void SearchMemory::calculateThreads()
{
	if (m_length >= 0x800000)
		m_threadCount = 4;
	else if (m_length >= 0x200000)
		m_threadCount = 2;
	else
		m_threadCount = 1;

	for (int i=0; i<m_threadCount; i++)
	{
		m_threadResults.push_back(ResultList());
	}
}

void SearchMemory::progressCallBack(SearchTask* task, long long current, long long total)
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
		m_searchCount++;
		m_dump = false;
		if (m_searchType == SEARCH_FUZZY_EQUAL || m_searchType == SEARCH_FUZZY_NOTEQUAL || m_searchType == SEARCH_FUZZY_GREATER || m_searchType == SEARCH_FUZZY_LESS || m_searchType == SEARCH_FUZZY_INIT)
		{
			if (m_searchCount == 1)
				m_dump = true;
		}
		m_running = false;
	}
}

float SearchMemory::getProgress()
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

unsigned int SearchMemory::process()
{
	shared_ptr<SearchTask> searchTask;
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
			string name = "SEARCHTASK_"+to_string(i);
			if (i == m_threadCount-1)
			{
				searchTask = make_shared<SearchTask>(m_ip, m_offset+(i*minSize), minSize+minRemain, i, name);
				searchTask->setLastThread(true);
			}
			else
				searchTask = make_shared<SearchTask>(m_ip, m_offset+(i*minSize), minSize, i, name);
			searchTask->setSearchType(m_searchType);
			searchTask->setSearchValue(m_searchValue);
			searchTask->setSearchValueType(m_searchValueType);
			searchTask->setSearchCompareType(m_compareType);
			searchTask->setResultList(&m_threadResults[i]);
			m_threadList.push_back(searchTask);
			searchTask->setProgressCallback(bind(&SearchMemory::progressCallBack, this, searchTask.get(), placeholders::_2, placeholders::_3));
			searchTask->setErrorCallback(bind(&SearchMemory::getError, this));
			searchTask->start();
			::this_thread::sleep_for(::chrono::milliseconds(500));
		}
	}
	else
	{
		for (int i = 0; i<m_threadCount; i++)
		{
			m_threadList.at(i)->join();
		}
		if (m_reorganize)
		{
			reOrganizeResults();
			m_reorganize = false;
		}
		for (int i = 0; i<m_threadCount; i++)
		{
			m_threadList.at(i)->setSearchType(m_searchType);
			m_threadList.at(i)->setSearchCompareType(m_compareType);
			m_threadList.at(i)->setSearchValue(m_searchValue);
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
		m_status = "SEARCHMEMORY";
	for (unsigned int i=0; i<m_threadList.size(); i++)
	{
		::this_thread::sleep_for(::chrono::milliseconds(100));
		//if (i != m_threadList.size()-1)
			m_threadList.at(i)->fire();
	}
	return TASK_ERROR_NONE;
}

void SearchMemory::cancel() 
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

unsigned long SearchMemory::getNumberOfResults()
{
	unsigned long res = 0;
	unsigned long threadSize = 0;
	long long min = -1;
	long long max = 0;
	for (int i=0; i<m_threadResults.size(); i++)
	{
		threadSize = 0;
		for (ResultList::iterator it = m_threadResults[i].begin(); it != m_threadResults[i].end(); ++it)
		{
			res += it->second.size();
			threadSize += it->second.size();
		}
		if (!m_dump)
		{
			if (threadSize > max)
			{
				if (min == -1)
					min = threadSize;
				max = threadSize;
			}
			else
			{
				if (threadSize <= min || min == -1)
					min = threadSize;
			}
		}
	}
	if (m_threadCount > 1 && !m_dump && res >  0 &&  ((max-min) / (float)res) > 0.40f )
		m_reorganize = true;
	return res;
}

void SearchMemory::reOrganizeResults()
{
	unsigned long resTotal = getNumberOfResults();
	unsigned long resCount = 0;
	float avg = resTotal / (float)m_threadCount;
	map<unsigned long, ResultList> resMap; //map of a group of addresses (sections)
	unsigned long size = 0;
	unsigned long thread = 0;
	for (unsigned long i=0; i<m_threadResults.size(); i++) //each thread (vector of ResultList)
	{
		for (ResultList::iterator it = m_threadResults[i].begin(); it != m_threadResults[i].end(); ++it) //Each section of a ResultList
		{
			unsigned long secTotal = it->second.size();
			resCount += secTotal;
			if (secTotal == 0)
				continue;
			size += secTotal;
			if (size < (unsigned long)avg)
			{
				resMap[thread][it->first] = it->second;
			}
			else if (thread == m_threadCount-1) //if the last thread, add the rest!
			{
				resMap[thread][it->first] = it->second;
			}
			else //size is greater than the average
			{
				if (resMap[thread].size() > 0)
				{
					thread++;
					resMap[thread][it->first] = it->second;
					size = secTotal;
				}
				else
				{
					resMap[thread][it->first] = it->second;
					size = 0;
					thread++;
				}
			}
			if (resCount == resTotal) //we've gone through all the results
				break;
		}
		if (resCount == resTotal) //we've gone through all the results
			break;
	}
	for (unsigned long i=0; i<m_threadResults.size(); i++) //each thread
	{
		m_threadResults[i].clear();
		m_threadResults[i] = resMap[i];
	}
}
