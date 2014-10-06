#include "MemoryOperator.h"
#include "Helpers.h"
#include <iostream>
#include <cstring>
void MemoryOperator::start()
{
	m_thread = thread(&MemoryOperator::run, this);
}

void MemoryOperator::setReadMemoryOperation(AddressItem item, bool keep)
{
	lock_guard<mutex> lock(m_mutex);
	if (memoryReadOperationList.count(item->address) > 0) //we already have it!
	{
		bool found = false;
		for (MemoryReadSet::iterator it = memoryReadOperationList[item->address].begin(); it != memoryReadOperationList[item->address].end(); ++it)
		{
			if (item.get() == (*it)->item.get())
			{
				found = true;
				(*it)->keep |= keep;
				break;
			}
		}
		if (!found) //this is a new memory location
		{
			memoryReadOperationList[item->address].push_back(make_shared<MemoryReadItem>(item, keep));
		}
	}
	else
	{
		memoryReadOperationList[item->address].push_back(make_shared<MemoryReadItem>(item, keep));
	}
}

void MemoryOperator::setChunkReadMemoryOperation(unsigned long start, unsigned long size, char *memory, bool keep)
{
	lock_guard<mutex> lock(m_mutex);
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
}

void MemoryOperator::setWriteMemoryOperation(AddressItem item, long long value, bool freeze)
{
	lock_guard<mutex> lock(m_mutex);
	bool found = false;
	if (memoryWriteOperationList.count(item->address) > 0) //we already have it!
	{
		for (auto it = memoryWriteOperationList[item->address].begin(); it != memoryWriteOperationList[item->address].end(); ++it)
		{
			if ((*it)->item.get() == item.get())
			{
				found = true;
				(*it)->freeze = freeze;
				(*it)->item->store = value;
				break;
			}
		}
		if (!found) //we have a new address
		{
			memoryWriteOperationList[item->address].push_back(make_shared<MemoryWriteItem>(item, value, freeze));
		}
	}
	else
	{
		memoryWriteOperationList[item->address].push_back(make_shared<MemoryWriteItem>(item, value, freeze));
	}
}

void MemoryOperator::removeMemoryOperation(char command, AddressItem item)
{
	lock_guard<mutex> lock(m_mutex);
	switch (command)
	{
	case MEMORY_COMMAND_CONDITIONAL:
		if (memoryConditionalWriteOperationList.count(item->address) == 0) return;
		for (auto it = memoryConditionalWriteOperationList[item->address].begin(); it != memoryConditionalWriteOperationList[item->address].end();)
		{
			if ((*it)->conditionItem.get() != item.get())
			{
				++it;
				continue;
			}
			m_mutex.unlock();
			removeMemoryOperation(MEMORY_COMMAND_READ, item);
			m_mutex.lock();
			for (auto writes = (*it)->writeItems.begin(); writes != (*it)->writeItems.end(); ++writes)
			{
				m_mutex.unlock();
				removeMemoryOperation(MEMORY_COMMAND_READ, (*writes));
				m_mutex.lock();
			}
			it = memoryConditionalWriteOperationList[item->address].erase(it);
		}
		if (memoryConditionalWriteOperationList.count(item->address) > 0 && memoryConditionalWriteOperationList[item->address].size() == 0)
			memoryConditionalWriteOperationList.erase(item->address);
		break;
	case MEMORY_COMMAND_WRITE:
		if (memoryWriteOperationList.count(item->address) == 0) return;
		for (auto it = memoryWriteOperationList[item->address].begin(); it != memoryWriteOperationList[item->address].end();)
		{
			if ((*it)->item.get() == item.get())
			{
				it = memoryWriteOperationList[item->address].erase(it);
				break;
			}
			++it;
		}
		if (memoryWriteOperationList.count(item->address) > 0 && memoryWriteOperationList[item->address].size() == 0)
			memoryWriteOperationList.erase(item->address);
		break;
	default:
		if (memoryReadOperationList.count(item->address) == 0) return;
		for (auto it = memoryReadOperationList[item->address].begin(); it != memoryReadOperationList[item->address].end();)
		{
			if ((*it)->item.get() == item.get())
			{
				it = memoryReadOperationList[item->address].erase(it);
				break;
			}
			++it;
		}
		if (memoryReadOperationList.count(item->address) > 0 && memoryReadOperationList[item->address].size() == 0)
			memoryReadOperationList.erase(item->address);
		break;
	}
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
		if (processConditional() != 0)
			continue;
		if (processRead() != 0)
			continue;
		if (processChunkRead() != 0)
			continue;
	}
}

int MemoryOperator::processRead()
{
	lock_guard<mutex> lock(m_mutex);
	for (MemoryReadItemList::iterator it = memoryReadOperationList.begin(); it != memoryReadOperationList.end();)
	{
		if (m_exit) { return 1; }
		for (MemoryReadSet::iterator setIT = it->second.begin(); setIT != it->second.end();)
		{
			if ((*setIT)->item->isPointer()) //we are reading a pointer
			{
				AddressOffsets *po = &(*setIT)->item->pointer->pointers;
				unsigned long cAddress = (*setIT)->item->address;
				for (auto offsetIT = po->begin(); offsetIT != po->end(); ++offsetIT)
				{
					offsetIT->address = cAddress;
					cAddress = (unsigned long)readAddress(cAddress, SEARCH_VALUE_TYPE_4BYTE);
					if (cAddress > 0)
						cAddress += offsetIT->offset;
					else
						break;
				}
				if (cAddress == 0)
					(*setIT)->item->pointer->resolved = 0;
				else
				{
					(*setIT)->item->pointer->resolved = cAddress;
					(*setIT)->item->value = (unsigned long)readAddress((*setIT)->item->pointer->resolved, (*setIT)->item->type);
				}
				(*setIT)->item->pointer->update();
			}
			else
				(*setIT)->item->value = (unsigned long)readAddress((*setIT)->item->address, (*setIT)->item->type);
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
	return 0;
}

long long MemoryOperator::readAddress(unsigned long address, char type)
{
	unsigned int length;
	char *data;
	length = Helpers::getTypeLength(type);
	if (m_ccapi->readMemory(address, length) == 0)
	{
		data = m_ccapi->getData(length);
		if (type == SEARCH_VALUE_TYPE_2BYTE)
		{
			short tmp = BSWAP16(*(short*)&data[0]);
			return (long long)tmp;
		}
		else if (type == SEARCH_VALUE_TYPE_4BYTE)
		{
			long tmp = BSWAP32(*(long*)&data[0]);
			return (long long)tmp;
		}
		else if (type == SEARCH_VALUE_TYPE_FLOAT)
		{
			unsigned long tmp = BSWAP32(*(unsigned long*)&data[0]);
			return *(unsigned long*)&tmp;
		}
		else
		{
			return (long long)data[0];
		}
	}
	return 0;
}

int MemoryOperator::processChunkRead()
{
	unsigned int length;
	char *data;
	if (memoryChunkReadOperationList.size() == 0)
		return 0;
	lock_guard<mutex> lock(m_mutex);
	for (MemoryChunkReadItemList::iterator it = memoryChunkReadOperationList.begin(); it != memoryChunkReadOperationList.end();)
	{
		if (m_exit) { return 1; }
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
	return 0;
}


int MemoryOperator::processWrite()
{
	lock_guard<mutex> lock(m_mutex);
	for (MemoryWriteItemList::iterator it = memoryWriteOperationList.begin(); it != memoryWriteOperationList.end();)
	{
		if (m_exit) { return 1; }
		for (MemoryWriteSet::iterator setIT = it->second.begin(); setIT != it->second.end();)
		{
			Variant val((*setIT)->item->store);
			if ((*setIT)->item->isPointer())
			{
				if ((*setIT)->item->pointer->resolved != 0) //we are writing a pointer
					writeAddress((*setIT)->item->pointer->resolved, (*setIT)->item->type, val);
				else //we want to write a pointer, but it hasn't been resolved yet
				{
					++setIT; continue;
				}
			}
			else
				writeAddress((*setIT)->item->address, (*setIT)->item->type, val);
			if ((*setIT)->freeze == true)
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
			it = memoryWriteOperationList.erase(it);
		}
		else
		{
			++it;
		}
	}
	return 0;
}


int MemoryOperator::writeAddress(unsigned long address, char type, Variant value)
{
	int res = CCAPI_ERROR_NO_ERROR; 
	unsigned int length = getLength(type);

	if (type == SEARCH_VALUE_TYPE_2BYTE)
	{
		short tmp = BSWAP16(value.asShort());
		res = m_ccapi->writeMemory(address, length, (char*)&tmp);
	}
	else if (type == SEARCH_VALUE_TYPE_4BYTE)
	{
		long tmp = BSWAP32(value.asLong());
		res = m_ccapi->writeMemory(address, length, (char*)&tmp);
	}
	else if (type == SEARCH_VALUE_TYPE_FLOAT)
	{
		unsigned long tmp = BSWAP32(value.asLong());
		res = m_ccapi->writeMemory(address, length, (char*)&tmp);
	}
	else
	{
		res = m_ccapi->writeMemory(address, length, value.asPointer());
	}
	return res;
}

void MemoryOperator::removeChunkReadOperation(unsigned long address)
{
	memoryChunkReadOperationList.erase(address);
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

void MemoryOperator::setConditionalWriteMemoryOperation(AddressItem item, char conditionType, long long conditionValue, AddressList writeItems)
{
	setReadMemoryOperation(item, true);
	lock_guard<mutex> lock(m_mutex);
	bool found = false;
	if (memoryConditionalWriteOperationList.count(item->address) > 0) //we already have it!
	{
		found = true;
		for (auto it = memoryConditionalWriteOperationList[item->address].begin(); it != memoryConditionalWriteOperationList[item->address].end(); ++it)
		{
			if (writeItems.size() == (*it)->writeItems.size())
			{
				auto j = (*it)->writeItems.begin();
				for (auto i = writeItems.begin(); i != writeItems.end(); ++i, ++j)
				{
					if ((*i) != (*j))
					{
						found = false;
						break;
					}
					else
					{
						(*j)->value = (*i)->value;
					}
				}
			}
			else
			{
				found  = false;
				break;
			}
			if (!found) break;
		}
		if (!found) //we have a new address
		{
			memoryConditionalWriteOperationList[item->address].push_back(make_shared<MemoryConditionalWriteItem>(item, conditionValue, conditionType, writeItems));
			m_mutex.unlock();
			for (auto i = writeItems.begin(); i != writeItems.end(); ++i) //lets read all the write addresses, and keep reading the pointers!
			{
				setReadMemoryOperation((*i), (*i)->isPointer());
			}
			m_mutex.lock();
		}
	}
	else
	{
		memoryConditionalWriteOperationList[item->address].push_back(make_shared<MemoryConditionalWriteItem>(item, conditionValue, conditionType, writeItems));
		m_mutex.unlock();
		for (auto i = writeItems.begin(); i != writeItems.end(); ++i) //lets read all the write addresses, and keep reading the pointers!
		{
			setReadMemoryOperation((*i), (*i)->isPointer());
		}
		m_mutex.lock();
	}
}

int MemoryOperator::processConditional()
{
	lock_guard<mutex> lock(m_mutex);
	for (auto it = memoryConditionalWriteOperationList.begin(); it != memoryConditionalWriteOperationList.end();)
	{
		if (m_exit) { return 1; }
		for (auto setIT = it->second.begin(); setIT != it->second.end();)
		{
			bool check = false;
			switch ( (*setIT)->conditionType)
			{
			default:
			case SEARCH_VALUE_EXACT: check = (*setIT)->conditionItem->value == (*setIT)->conditionValue; break;
			case SEARCH_VALUE_NOTEXACT: check = (*setIT)->conditionItem->value != (*setIT)->conditionValue; break;
			case SEARCH_VALUE_GREATER: check = (*setIT)->conditionItem->value > (*setIT)->conditionValue; break;
			case SEARCH_VALUE_LESS: check = (*setIT)->conditionItem->value < (*setIT)->conditionValue; break;
			}
			if ((*setIT)->hasRead && check) //we will perform the writes!
			{
				for (auto writes = (*setIT)->writeItems.begin(); writes != (*setIT)->writeItems.end(); ++writes)
				{
					doWrite((*writes), (*writes)->store);
				}
			}
			else if (!(*setIT)->hasRead)
			{
				(*setIT)->hasRead = true;
			}
			++setIT;
		}
		++it;
	}
	return 0;

}

int MemoryOperator::doWrite(AddressItem item, long long value)
{
	Variant val(value);
	if (item->isPointer() && item->pointer->resolved != 0) //we are writing a pointer
		return writeAddress(item->pointer->resolved, item->type, val);
	else
		return writeAddress(item->address, item->type, val);
}