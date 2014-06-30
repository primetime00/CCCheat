#ifndef _MEMORYOPERATOR_
#define _MEMORYOPERATOR_

#include <thread>
#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include "Helpers.h"
#include "Common.h"
#include "CCAPI.h"

class MemoryOperator
{
public:
	MemoryOperator(string ip) { m_ip = ip; m_exit = false; }
	~MemoryOperator() {exit(); join(); }

	void start();
	void join() { 
		if (m_thread.joinable()) 
		{
			m_thread.join(); 
		}
	}
	void exit() { m_exit = true; }
	string getStatus() { return m_status; }

	void setWriteMemoryOperation(AddressItem item, long long value, bool freeze);
	void setReadMemoryOperation(AddressItem item, bool keep);
	void setChunkReadMemoryOperation(unsigned long start, unsigned long size, char *memory, bool keep);

	void removeMemoryOperation(char command, AddressItem item);
	void removePointerOperation(char command, PointerItem p);
	void removeChunkReadOperation(unsigned long address);

	void setHostCCAPIVersion(int ver) { m_ccapiHostVersion = ver; }

private:
	MemoryOperator() {}
	void run();
	void process();
	int processRead();
	int processPointers();
	int processChunkRead();
	int processWrite();

	unsigned int getLength(char type) { return Helpers::getTypeLength(type); }
	long long readAddress(unsigned long address, char type);
	void writeAddress(unsigned long address, char type, long long value);

	int connect();

	thread m_thread;
	shared_ptr<CCAPI> m_ccapi;
	string m_ip;
	MemoryReadItemList memoryReadOperationList;
	MemoryChunkReadItemList memoryChunkReadOperationList;
	MemoryWriteItemList memoryWriteOperationList;
	bool m_exit;
	string m_status;
	mutex m_mutex;
	int m_ccapiHostVersion;



};

#endif