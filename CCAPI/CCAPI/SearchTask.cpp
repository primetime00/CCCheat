#include "SearchTask.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <cstring>
#include "Helpers.h"
#include "ChunkFuzzySearch.h"
#include "ChunkValueSearch.h"


using namespace std;

SearchTask::SearchTask(string ip, unsigned long long offset, unsigned long long length, unsigned int id, string name) : Task(id, name)
{
	m_offset = offset;
	m_length = length;
	m_ip = ip;
	m_isLastThread = false;
	m_searchValueType = SEARCH_VALUE_TYPE_4BYTE;
	m_compareType = SEARCH_FUZZY_NOTEQUAL;
	m_connected = false;
}

SearchTask::~SearchTask()
{
	if (m_ccapi != nullptr && m_ccapi->isAttached())
		m_ccapi->disconnect();
}


void SearchTask::run()
{
	unsigned int res;
	m_taskState = Task::WAIT;
	res = connectAndAttach();
	if (res == TASK_ERROR_NONE)
	{
		res = waitForFire();
		if (res == TASK_ERROR_NONE)
		{
			m_taskState = Task::WORKING;
			if (m_searchType == SEARCH_TYPE_FUZZY)
			{
				m_isFuzzy = true;
				res = doFuzzySearch();
			}
			else
			{
				m_isFuzzy = false;
				res = doValueSearch();
			}
		}
	}
}

int SearchTask::connect(int loops)
{
	if (m_ccapi->connect() == 0)
	{
		for (int i=0; i<loops; i++)
		{
			if (m_cancel)
			{
				m_ccapi->disconnect();
				return TASK_ERROR_CANCEL;
			}
			this_thread::sleep_for(chrono::milliseconds(50));
		}
	}
	else
		return TASK_ERROR_NO_CONNECT;
	return TASK_ERROR_NONE;
}

int SearchTask::attach()
{
	int res;
	res = m_ccapi->attach();
	if (res == 0)
	{
		m_ccapi->disconnect();
		m_status = "ERROR";
		m_error = TASK_ERROR_NO_ATTACH;
		m_connected = false;
		return m_error;
	}
	else if (res == -1) //disconnected!
	{
		if (m_cancel)
			return TASK_ERROR_CANCEL;
		m_status = "ERROR";
		m_error = TASK_ERROR_NO_ATTACH;
		m_connected = false;
		return m_error;
	}
	return TASK_ERROR_NONE;
}

int SearchTask::connectAndAttach(int retry)
{
	m_status = "CONNECT";
	int res = 0;
	if (m_ccapi == nullptr || !m_ccapi->isConnected())
	{
		for (int i=0; i<retry; i++)
		{
			m_ccapi = make_shared<CCAPI>(m_ip);
			res = connect();
			if (res == TASK_ERROR_NONE)
				break;
		}
		if (!m_ccapi->isConnected())
		{
			m_status = "ERROR";
			m_connected = false;
			return TASK_ERROR_NO_CONNECT;
		}
	}
	if (!m_ccapi->isAttached())
	{
		res = attach();
		if (!m_ccapi->isAttached())
		{
			m_status = "ERROR";
			m_connected = false;
			return TASK_ERROR_NO_ATTACH;
		}
	}
	m_status = "CONNECTED";
	m_connected = true;
	return TASK_ERROR_NONE;
}

unsigned int SearchTask::waitForFire()
{
	while (m_fire != true)
	{
		if (m_cancel)
		{
			m_ccapi->disconnect();
			return TASK_ERROR_CANCEL;
		}
		this_thread::sleep_for(chrono::milliseconds(50));
		if (m_errorCallback != nullptr)
		{
			unsigned int error = m_errorCallback();
			if (error != TASK_ERROR_NONE)
			{
				m_ccapi->disconnect();
				return error;
			}
		}
	}
	return TASK_ERROR_NONE;

}
int SearchTask::doFuzzySearch()
{
	int res;
	m_valueSearcher.clear();
	m_fuzzySearcher.clear();
	m_fuzzySearcher.setResultReference(m_results);
	m_fuzzySearcher.setup(m_searchValueType, m_compareType);
	if (m_results->size() == 0 && m_dumpFile.empty()) //we are doing an initial search
	{
		res = initialFuzzySearch(); //which is just a dump
	}
	else
	{
		res = continueFuzzySearch();
	}
	m_fire = false;
	return res;
}

int SearchTask::doValueSearch()
{
	m_valueSearcher.clear();
	m_fuzzySearcher.clear();
	m_valueSearcher.setResultReference(m_results);
	m_valueSearcher.setup(m_searchValueType, m_compareType, m_searchValue);
	if (m_dumpFile.empty()) //we are doing an initial search
	{
		initialValueSearch(); //collect all of our values
	}
	else
	{
		continueValueSearch();
	}
	m_fire = false;
	return 0;
}

int SearchTask::initialFuzzySearch() //THIS WILL GENERATE A DUMP FILE!
{
	ofstream dumpFile;
	m_dumpFile = NumberToString(m_offset)+"-"+NumberToString(m_length)+".bin";
	if (m_offset+m_length-3 < 0x100000000)
	{
		if (!m_isLastThread)
		{
			switch(m_searchValueType) //this will allow us to read into the next threads area just a little bit
			{
			case SEARCH_VALUE_TYPE_2BYTE: m_length += 1; break;
			case SEARCH_VALUE_TYPE_4BYTE:
			case SEARCH_VALUE_TYPE_FLOAT: m_length += 3; break;
			default: break;
			}
		}
	}
	unsigned long rounds, remains;
	unsigned char resFileType = RESULT_FILE_TYPE_DUMP;
	char *memData;
	unsigned int length;
	calculateReads(RANGE_INTERVAL, rounds, remains);
	dumpFile.open(DUMP_DIR+m_dumpFile, ios::out | ios::binary | ios::trunc);
	dumpFile.write((char*)&resFileType, sizeof(unsigned char));
	for (unsigned long long i=0; i<rounds; i++)
	{
		if (m_cancel)
		{
			dumpFile.close();
			return TASK_ERROR_CANCEL;
		}
		dumpDataToFile(dumpFile, m_offset+(i*RANGE_INTERVAL), RANGE_INTERVAL);
		progressCallback(this, i*RANGE_INTERVAL, m_length);
	}
	if (remains > 0)
	{
		dumpDataToFile(dumpFile, m_offset+m_length-remains, remains);
	}
	dumpFile.close();
	m_taskState = Task::COMPLETE;
	progressCallback(this, m_length, m_length);
	return TASK_ERROR_NONE;
}
void SearchTask::dumpDataToFile(ostream &dumpFile, unsigned long location, unsigned long length)
{
	unsigned int readSize; 
	char *data;
	if (m_ccapi->readMemory(location, length) == 0)
	{
		data = m_ccapi->getData(readSize);
		dumpFile.write(data, readSize);
	}
}

int SearchTask::initialValueSearch() //this will generate a result file
{
	unsigned int readSize; 
	unsigned long i;
	char *data;
	bool isUnsigned = false;
	m_results->clear();
	switch (m_searchValueType)
	{
	case SEARCH_VALUE_TYPE_1BYTE: isUnsigned = ((unsigned char)m_searchValue > 0x7F); break;
	case SEARCH_VALUE_TYPE_2BYTE: isUnsigned = ((unsigned short)m_searchValue > 0x7FFF); break;
	case SEARCH_VALUE_TYPE_4BYTE: isUnsigned = ((unsigned long)m_searchValue > 0x7FFFFFFF); break;
	}
	m_dumpFile = NumberToString(m_offset)+"-"+NumberToString(m_length)+".bin";
	if (m_offset+m_length-3 < 0x100000000)
	{
		if (!m_isLastThread)
		{
			switch(m_searchValueType) //this will allow us to read into the next threads area just a little bit
			{
			case SEARCH_VALUE_TYPE_2BYTE: m_length += 1; break;
			case SEARCH_VALUE_TYPE_4BYTE:
			case SEARCH_VALUE_TYPE_FLOAT: m_length += 3; break;
			default: break;
			}
		}
	}
	unsigned long rounds, remains;
	unsigned char resFileType = RESULT_FILE_TYPE_LIST;
	char *memData;
	unsigned int length;
	calculateReads(RANGE_INTERVAL, rounds, remains);
	for (i=0; i<rounds; i++)
	{
		if (m_cancel)
			return TASK_ERROR_CANCEL;
		if (m_ccapi->readMemory(m_offset+(i*RANGE_INTERVAL), RANGE_INTERVAL) == 0)
		{
			data = m_ccapi->getData(readSize);
			m_valueSearcher.digest(data, readSize, m_offset+(i*RANGE_INTERVAL));
			progressCallback(this, i*RANGE_INTERVAL, m_length);
		}
	}
	if (remains > 0)
	{
		if (m_ccapi->readMemory(m_offset+m_length-remains, remains) == 0)
		{
			data = m_ccapi->getData(readSize);
			m_valueSearcher.digest(data, readSize, m_offset+m_length-remains);
		}
	}
	//else if (rounds > 0)
	//	m_valueSearcher.recordResults(m_offset+(i*RANGE_INTERVAL));
	if (m_cancel)
		return TASK_ERROR_CANCEL;

	m_taskState = Task::COMPLETE;
	progressCallback(this, m_length, m_length);
	return TASK_ERROR_NONE;
}
int SearchTask::continueFuzzySearch()
{
	ifstream resFile(DUMP_DIR+m_dumpFile, ios::in | ios::binary);
	unsigned char resFileType;
	int res;
	if (resFile)
	{
		resFile.read((char*)&resFileType, sizeof(unsigned char));
		if (resFileType == RESULT_FILE_TYPE_DUMP) //we are comparing to a dump file
		{
			res = fuzzySearchDumpFile(resFile);
		}
	}
	else if (m_results->size() > 0)
	{
		res = fuzzySearchResults();
	}
	else //there are no results!
	{
		res = TASK_ERROR_NONE;
		m_taskState = Task::COMPLETE;
		progressCallback(this, 1, 1);
	}
	cout << "THREAD " << m_name << " HAS " << getNumberOfResults(*m_results) << " Results" << endl;
	return res;
}


int SearchTask::fuzzySearchDumpFile(ifstream &resFile) 
{
	m_results->clear();
	unsigned long rounds, remains;
	unsigned long long i;
	char *memData;
	char resData[RANGE_INTERVAL];
	unsigned int length;
	calculateReads(RANGE_INTERVAL, rounds, remains);
	for (i=0; i<rounds; i++)
	{
		if (m_cancel)
		{
			resFile.close();
			return TASK_ERROR_CANCEL;
		}
		if (m_ccapi->readMemory(m_offset+(i*RANGE_INTERVAL), RANGE_INTERVAL) == 0)
		{
			memData = m_ccapi->getData(length);
			resFile.read(resData, RANGE_INTERVAL);
			m_fuzzySearcher.digest(memData, resData, RANGE_INTERVAL, m_offset+(i*RANGE_INTERVAL));
		}
		progressCallback(this, i*RANGE_INTERVAL, m_length);
	}
	if (remains > 0)
	{
		if (m_ccapi->readMemory(m_offset+m_length-remains, remains) == 0)
		{
			memData = m_ccapi->getData(length);
			resFile.read(resData, length);
			m_fuzzySearcher.digest(memData, resData, length, m_offset+m_length-remains);
		}
	}
	resFile.close();
	if (m_cancel)
		return TASK_ERROR_CANCEL;
	remove(string(DUMP_DIR+m_dumpFile).c_str());
	m_taskState = Task::COMPLETE;
	progressCallback(this, m_length, m_length);
	return TASK_ERROR_NONE;
}


int SearchTask::fuzzySearchResults()
{
	const int cbRate = 15;
	int scans = 0;
	int length;
	unsigned int readLen;
	unsigned long section, size;
	char finalDumpBytes[3];
	char *dataBuf;
	switch (m_searchValueType)
	{
	case SEARCH_VALUE_TYPE_1BYTE: length = 1; break;
	case SEARCH_VALUE_TYPE_2BYTE: length = 2; break;
	default: length = 4; break;
	}
	unsigned resListSize = getNumberOfResults(*m_results);
	for (ResultList::iterator it=m_results->begin(); it!=m_results->end(); ++it) //read each section
	{
		if (it->second.size() == 0)
			continue;
		AddressList keep;
		m_fuzzySearcher.clear();
		section = it->first;
		size = it->second.size();
		if (size > 10) //lets grab the chunk!
		{
			if (m_cancel)
				return TASK_ERROR_CANCEL;
			if (section+RANGE_INTERVAL < 0xFFFFFFFF-3 && m_ccapi->readMemory(section+RANGE_INTERVAL, 3) == 0)
			{
				dataBuf = m_ccapi->getData(readLen);
				memcpy(finalDumpBytes, dataBuf, readLen);
			}
			if (m_ccapi->readMemory(section, RANGE_INTERVAL) == 0)
			{
				dataBuf = m_ccapi->getData(readLen);
				if (m_ccapi->insertData(readLen, finalDumpBytes, 3))
				{
					for (AddressList::iterator addrIt = it->second.begin(); addrIt != it->second.end();) //read all addresses in a section
					{
						//if (m_fuzzySearcher.digestValue(&dataBuf[get<0>(*addrIt) - section], *addrIt, section)) //we need to keep it!
						if (m_fuzzySearcher.digestValue(&dataBuf[(addrIt->address) - section], *addrIt, section)) //we need to keep it!
							keep.push_back(*addrIt);
						addrIt++;
						if (scans % cbRate == 0) progressCallback(this, scans, resListSize);
						scans++;
					}
					it->second = keep;
				}
			}
		}
		else
		{
			for (AddressList::iterator addrIt = it->second.begin(); addrIt != it->second.end();) //read all addresses in asection
			{
				if (m_cancel)
					return TASK_ERROR_CANCEL;

				if (m_ccapi->readMemory((addrIt->address), length) == 0)
				{
					dataBuf = m_ccapi->getData(readLen);
					if (m_fuzzySearcher.digestValue(dataBuf, *addrIt, section)) //we need to keep it!
						keep.push_back(*addrIt);
					addrIt++;
					if (scans % cbRate == 0) progressCallback(this, scans, resListSize);
					scans++;
				}
			}
			it->second = keep;
		}
	}
	m_taskState = Task::COMPLETE;
	progressCallback(this, resListSize, resListSize);
	return TASK_ERROR_NONE;
}

int SearchTask::continueValueSearch()
{
	int res;
	res = valueSearchResults();
	cout << "THREAD " << m_name << " HAS " << getNumberOfResults(*m_results) << " Results" << endl;
	return res;
}

int SearchTask::valueSearchResults()
{
	const int cbRate = 15;
	int scans = 0;
	int length;
	unsigned int readLen;
	unsigned long section, size;
	char finalDumpBytes[3];
	char *dataBuf;
	switch (m_searchValueType)
	{
	case SEARCH_VALUE_TYPE_1BYTE: length = 1; break;
	case SEARCH_VALUE_TYPE_2BYTE: length = 2; break;
	default: length = 4; break;
	}
	unsigned resListSize = getNumberOfResults(*m_results);
	for (ResultList::iterator it=m_results->begin(); it!=m_results->end(); ++it) //read each section
	{
		if (it->second.size() == 0)
			continue;
		AddressList keep;
		m_valueSearcher.clear();
		section = it->first;
		size = it->second.size();
		if (size > 10) //lets grab the chunk!
		{
			if (m_cancel)
				return TASK_ERROR_CANCEL;
			if (section+RANGE_INTERVAL < 0xFFFFFFFF-3 && m_ccapi->readMemory(section+RANGE_INTERVAL, 3) == 0)
			{
				dataBuf = m_ccapi->getData(readLen);
				memcpy(finalDumpBytes, dataBuf, readLen);
			}
			if (m_ccapi->readMemory(section, RANGE_INTERVAL) == 0)
			{
				dataBuf = m_ccapi->getData(readLen);
				if (m_ccapi->insertData(readLen, finalDumpBytes, 3))
				{
					for (AddressList::iterator addrIt = it->second.begin(); addrIt != it->second.end();) //read all addresses in a section
					{
						if (m_valueSearcher.digestValue(&dataBuf[(addrIt->address) - section], *addrIt, section)) //we need to keep it!
							keep.push_back(*addrIt);
						addrIt++;
						if (scans % cbRate == 0) progressCallback(this, scans, resListSize);
						scans++;
					}
					it->second = keep;
				}
			}
		}
		else
		{
			for (AddressList::iterator addrIt = it->second.begin(); addrIt != it->second.end();) //read all addresses in asection
			{
				if (m_cancel)
					return TASK_ERROR_CANCEL;
				if (m_ccapi->readMemory((addrIt->address), length) == 0)
				{
					dataBuf = m_ccapi->getData(readLen);
					if (m_valueSearcher.digestValue(dataBuf, *addrIt, section)) //we need to keep it!
						keep.push_back(*addrIt);
					addrIt++;
					if (scans % cbRate == 0) progressCallback(this, scans, resListSize);
					scans++;
				}
			}
			it->second = keep;
		}
	}
	m_taskState = Task::COMPLETE;
	progressCallback(this, resListSize, resListSize);
	return TASK_ERROR_NONE;
}


void SearchTask::calculateReads(unsigned long interval, unsigned long &rounds, unsigned long &remain)
{
	unsigned long long minSize = m_length / interval;
	unsigned long long minRemain = m_length % interval;

	rounds = (unsigned long) minSize;
	remain = (unsigned long) minRemain;
}


void SearchTask::disconnect() 
{
	if (m_connected)
	{
		m_ccapi->disconnect();
		m_connected = false;
	}
}

unsigned long SearchTask::getNumberOfResults(ResultList resList)
{
	unsigned long total=0;
	for (ResultList::iterator it=resList.begin(); it!=resList.end(); ++it)
	{
		total+= it->second.size();
	}
	return total;
}
