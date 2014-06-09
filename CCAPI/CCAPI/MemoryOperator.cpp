#include "MemoryOperator.h"
#include <iostream>
#include <cstring>
void MemoryOperator::start()
{
	m_thread = thread(&MemoryOperator::run, this);
}

void MemoryOperator::setReadMemoryOperation(unsigned long address, char type, char *memory, bool keep)
{
	m_mutex.lock();
	if (memoryReadOperationList.count(address) > 0) //we already have it!
	{
		bool found = false;
		for (MemoryReadSet::iterator it = memoryReadOperationList[address].begin(); it != memoryReadOperationList[address].end(); ++it)
		{
			if ((*it)->memory == memory) //we have it!
			{
				found = true;
				(*it)->type = type;
				(*it)->keep |= keep;
				break;
			}
		}
		if (!found) //this is a new memory location
		{
			memoryReadOperationList[address].push_back(make_shared<MemoryReadItem>(address, type, memory, keep));
		}
	}
	else
	{
		memoryReadOperationList[address].push_back(make_shared<MemoryReadItem>(address, type, memory, keep));
	}
	m_mutex.unlock();
}

void MemoryOperator::setChunkReadMemoryOperation(unsigned long start, unsigned long size, char *memory, bool keep)
{
	m_mutex.lock();
	if (memoryChunkReadOperationList.count(start) > 0) //we already have it!
	{
		bool found = false;
		for (MemoryChunkReadSet::iterator it = memoryChunkReadOperationList[start].begin(); it != memoryChunkReadOperationList[start].end(); ++it)
		{
			if ((*it)->memory == memory) //we have it!
			{
				found = true;
				(*it)->keep |= keep;
				break;
			}
		}
		if (!found) //this is a new memory location
		{
			memoryChunkReadOperationList[start].push_back(make_shared<MemoryChunkReadItem>(start, size, memory, keep));
		}
	}
	else
	{
		memoryChunkReadOperationList[start].push_back(make_shared<MemoryChunkReadItem>(start, size, memory, keep));
	}
	m_mutex.unlock();
}

void MemoryOperator::setWriteMemoryOperation(unsigned long address, long long value, char type, bool freeze)
{
	m_mutex.lock();
	if (memoryWriteOperationList.count(address) > 0) //we already have it!
	{
		memoryWriteOperationList[address]->freeze = freeze;
		memoryWriteOperationList[address]->type = type;
		memoryWriteOperationList[address]->value = value;
	}
	else
	{
		memoryWriteOperationList[address] = make_shared<MemoryWriteItem>(address, value, type, freeze);
	}
	m_mutex.unlock();
}


void MemoryOperator::removeMemoryOperation(char command, unsigned long address)
{
	m_mutex.lock();
	switch (command)
	{
	case MEMORY_COMMAND_WRITE:
		memoryWriteOperationList.erase(address);
		break;
	case MEMORY_COMMAND_READCHUNK:
		memoryChunkReadOperationList.erase(address);
		break;
	default:
		memoryReadOperationList.erase(address);
		break;
	}
	m_mutex.unlock();
}


void MemoryOperator::run()
{
	m_status = "CONNECT";
	if (connect() != CCAPI_ERROR_NO_ERROR)
	{
		m_status = "ERROR";
		return;
	}
	m_status = "PROCESS";
	process();
	m_ccapi->disconnect();
	m_status = "EXIT";
}

void MemoryOperator::process()
{
	while (!m_exit)
	{
		this_thread::sleep_for(chrono::milliseconds(100));
		if (processWrite() != 0)
			continue;
		if (processRead() != 0)
			continue;
		if (processChunkRead() != 0)
			continue;
	}
}

int MemoryOperator::processRead()
{
	unsigned int length;
	char *data;
	m_mutex.lock();
	for (MemoryReadItemList::iterator it = memoryReadOperationList.begin(); it != memoryReadOperationList.end();)
	{
		if (m_exit) { m_mutex.unlock(); return 1; }
		for (MemoryReadSet::iterator setIT = it->second.begin(); setIT != it->second.end();)
		{
			length = getLength((*setIT)->type);
			if (m_ccapi->readMemory((*setIT)->address, length) == 0)
			{
				data = m_ccapi->getData(length);
				if ((*setIT)->type == SEARCH_VALUE_TYPE_2BYTE)
				{
					short tmp = BSWAP16(*(short*)&data[0]);
					((long long*)(*setIT)->memory)[0] = (long long)tmp;
				}
				else if ((*setIT)->type == SEARCH_VALUE_TYPE_4BYTE)
				{
					long tmp = BSWAP32(*(long*)&data[0]);
					((long long*)(*setIT)->memory)[0] = (long long)tmp;
				}
				else if ((*setIT)->type == SEARCH_VALUE_TYPE_FLOAT)
				{
					unsigned long tmp = BSWAP32(*(unsigned long*)&data[0]);
					((long long*)(*setIT)->memory)[0] = *(unsigned long*)&tmp;
				}
				else
				{
					((long long*)(*setIT)->memory)[0] = (long long)data[0];
				}
			}
			if ((*setIT)->keep == true)
			{
				++setIT;
			}
			else
			{
				setIT = it->second.erase(setIT);
			}
		}
		if (it->second.size() == 0)
		{
			it = memoryReadOperationList.erase(it);
		}
		else
		{
			++it;
		}
	}
	m_mutex.unlock();
	return 0;
}

int MemoryOperator::processChunkRead()
{
	unsigned int length;
	char *data;
	if (memoryChunkReadOperationList.size() == 0)
		return 0;
	m_mutex.lock();
	for (MemoryChunkReadItemList::iterator it = memoryChunkReadOperationList.begin(); it != memoryChunkReadOperationList.end();)
	{
		if (m_exit) { m_mutex.unlock(); return 1; }
		for (MemoryChunkReadSet::iterator setIT = it->second.begin(); setIT != it->second.end();)
		{
			length = (*setIT)->length;
			if (m_ccapi->readMemory((*setIT)->address, length) == 0)
			{
				data = m_ccapi->getData(length);
				memcpy((*setIT)->memory, data, length);
			}
			if ((*setIT)->keep == true)
			{
				++setIT;
			}
			else
			{
				setIT = it->second.erase(setIT);
			}
		}
		if (it->second.size() == 0)
		{
			it = memoryChunkReadOperationList.erase(it);
		}
		else
		{
			++it;
		}
	}
	m_mutex.unlock();
	return 0;
}


int MemoryOperator::processWrite()
{
	unsigned int length;
	char loc[20];
	int res;
	m_mutex.lock();
	for (MemoryWriteItemList::iterator it = memoryWriteOperationList.begin(); it != memoryWriteOperationList.end();)
	{
		if (m_exit) { m_mutex.unlock(); return 1; }
		length = getLength(it->second->type);
		memcpy(loc, (char*)&it->second->value, 8);

		if (it->second->type == SEARCH_VALUE_TYPE_2BYTE)
		{
			short tmp = BSWAP16(*(short*)&loc);
			res = m_ccapi->writeMemory(it->second->address, length, (char*)&tmp);
		}
		else if (it->second->type == SEARCH_VALUE_TYPE_4BYTE)
		{
			long tmp = BSWAP32(*(long*)&loc);
			res = m_ccapi->writeMemory(it->second->address, length, (char*)&tmp);
		}
		else if (it->second->type == SEARCH_VALUE_TYPE_FLOAT)
		{
			unsigned long tmp = BSWAP32(*(unsigned long*)&loc);
			res = m_ccapi->writeMemory(it->second->address, length, (char*)&tmp);
		}
		else
		{
			res = m_ccapi->writeMemory(it->second->address, length, loc);
		}
		if (it->second->freeze)
			++it;
		else
			it = memoryWriteOperationList.erase(it);
	}
	m_mutex.unlock();
	return 0;
}

int MemoryOperator::connect()
{
	m_ccapi = make_shared<CCAPI>(m_ip);
	m_ccapi->setHostVersion(m_ccapiHostVersion);
	if (m_ccapi->connect() == 0)
	{
		for (int i=0; i<10; i++)
		{
			if (m_exit)
			{
				m_ccapi->disconnect();
				return CCAPI_ERROR_CANCEL;
			}
			this_thread::sleep_for(chrono::milliseconds(10));
		}
		if (m_ccapi->attach() == 0)
		{
			m_ccapi->disconnect();
			return CCAPI_ERROR_NO_ATTACH;
		}
	}
	else
	{
		return CCAPI_ERROR_NO_CONNECT;	
	}
	return CCAPI_ERROR_NO_ERROR;
}
