#include "DumpTask.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <cstring>
#include "Helpers.h"


using namespace std;

DumpTask::DumpTask(string ip, unsigned long long offset, unsigned long long length, unsigned int id, string name) : Task(id, name)
{
	char buf[255];
	m_offset = offset;
	m_length = length;
	m_ip = ip;
	m_connected = false;
	char *pt __attribute__((unused));
	pt = GetCurrentDir(buf, 255);
	m_buffer = 0;
}

DumpTask::~DumpTask()
{
	if (m_buffer)
		delete [] m_buffer;
	if (m_ccapi != nullptr && m_ccapi->isAttached())
		m_ccapi->disconnect();
}


void DumpTask::run()
{
	unsigned int res;
	if (m_cancel)
		return;
	m_taskState = Task::WAIT;
	res = connectAndAttach();
	if (res == TASK_ERROR_NONE)
	{
		res = waitForFire();
		if (res == TASK_ERROR_NONE)
		{
			m_taskState = Task::WORKING;
			doDump();
		}
	}
}

int DumpTask::connect(int loops)
{
	m_ccapi->setHostVersion(m_ccapiHostVersion);
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

int DumpTask::attach()
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

int DumpTask::connectAndAttach(int retry)
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

unsigned int DumpTask::waitForFire()
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
int DumpTask::doDump()
{
	unsigned long rounds, remains, bufferPos;
	if (m_buffer)
		delete [] m_buffer;
	m_buffer = new char[(unsigned long)m_length];
	char *data;
	unsigned int readSize;
	calculateReads(RANGE_INTERVAL, rounds, remains);
	bufferPos = 0;
	for (unsigned long long i=0; i<rounds; i++)
	{
		if (m_cancel)
		{
			delete [] m_buffer;
			m_buffer = 0;
			return TASK_ERROR_CANCEL;
		}
		if (m_ccapi->readMemory((unsigned long)(m_offset+(i*RANGE_INTERVAL)), RANGE_INTERVAL) == 0)
		{
			data = m_ccapi->getData(readSize);
			memcpy(&m_buffer[bufferPos], data, readSize);
			bufferPos += readSize;
			progressCallback(this, bufferPos, m_length);
		}
	}
	if (remains > 0)
	{
		if (m_ccapi->readMemory((unsigned long)(m_offset+m_length-remains), remains) == 0)
		{
			data = m_ccapi->getData(readSize);
			memcpy(&m_buffer[bufferPos], data, readSize);
			bufferPos += readSize;
		}
	}
	if (m_cancel)
	{
		delete [] m_buffer;
		m_buffer = 0;
		return TASK_ERROR_CANCEL;
	}
	m_fire = false;
	m_taskState = Task::COMPLETE;
	progressCallback(this, m_length, m_length);
	return TASK_ERROR_NONE;
}

void DumpTask::calculateReads(unsigned long interval, unsigned long &rounds, unsigned long &remain)
{
	unsigned long long minSize = m_length / interval;
	unsigned long long minRemain = m_length % interval;

	rounds = (unsigned long) minSize;
	remain = (unsigned long) minRemain;
}


void DumpTask::disconnect() 
{
	if (m_connected)
	{
		m_ccapi->disconnect();
		m_connected = false;
	}
}

