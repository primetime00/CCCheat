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
	void setWriteMemoryOperation(unsigned long address, long long value, char type, bool freeze);
	void removeMemoryOperation(char command, unsigned long address);

private:
	MemoryOperator() {}
	void run();
	void process();
	int processRead();
	int processWrite();

	unsigned int getLength(char type) { if (type == SEARCH_VALUE_TYPE_1BYTE) return 1; if (type == SEARCH_VALUE_TYPE_2BYTE) return 2; return 4; }

	int connect();

	thread m_thread;
	shared_ptr<CCAPI> m_ccapi;
	string m_ip;
	MemoryReadItemList memoryReadOperationList;
	MemoryWriteItemList memoryWriteOperationList;
	bool m_exit;
	string m_status;
	mutex m_mutex;



};

#endif