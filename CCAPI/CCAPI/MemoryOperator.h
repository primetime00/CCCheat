#ifndef _MEMORYOPERATOR_
#define _MEMORYOPERATOR_

#include <thread>
#include <string>
#include <memory>
#include <functional>
#include <mutex>
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

	void setReadMemoryOperation(unsigned long address, char type, char *memory, bool keep);
	void setReadPointerOperation(PointerItem pointer);

	void setWriteMemoryOperation(unsigned long address, long long value, char type, bool freeze);
	void setChunkReadMemoryOperation(unsigned long start, unsigned long size, char *memory, bool keep);
	void removeMemoryOperation(char command, unsigned long address);

	void setReadPointerOperation(unsigned long address, list <unsigned int> offsets, char *memory, bool keep);
	unsigned int getPointerListSize() { return pointerReadOperationList.size(); }

	void setHostCCAPIVersion(int ver) { m_ccapiHostVersion = ver; }

private:
	MemoryOperator() {}
	void run();
	void process();
	int processRead();
	int processPointers();
	int processChunkRead();
	int processWrite();

	unsigned int getLength(char type) { if (type == SEARCH_VALUE_TYPE_1BYTE) return 1; if (type == SEARCH_VALUE_TYPE_2BYTE) return 2; return 4; }
	unsigned int readPointer(unsigned int address, unsigned int offset);
	long long readAddress(unsigned long address, char type);

	int connect();

	thread m_thread;
	shared_ptr<CCAPI> m_ccapi;
	string m_ip;
	MemoryReadItemList memoryReadOperationList;
	MemoryChunkReadItemList memoryChunkReadOperationList;
	MemoryWriteItemList memoryWriteOperationList;
	PointerReadItemList pointerReadOperationList;
	bool m_exit;
	string m_status;
	mutex m_mutex;
	int m_ccapiHostVersion;



};

#endif