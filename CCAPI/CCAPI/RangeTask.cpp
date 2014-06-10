#include "RangeTask.h"
#include "RangeMemory.h"
#include <chrono>
#include <iostream>


using namespace std;

RangeTask::RangeTask(string ip, unsigned long long offset, unsigned long long length, unsigned int id, string name) : Task(id, name)
{
	m_offset = offset;
	m_length = length;
	m_ip = ip;
	m_lastGoodCheck = false;
}

void RangeTask::run()
{
	m_pair = RangePair(-1,-1);//make_pair(-1,-1);
	m_firstValidOffset = -1;
	unsigned long long currentLength = m_length;
	m_status = "CONNECT";
	m_ccapi = make_shared<CCAPI>(m_ip);
	m_ccapi->setHostVersion(m_ccapiHostVersion);
	if (m_ccapi->connect() == 0)
	{
		for (int i=0; i<10; i++)
		{
			if (m_cancel)
			{
				m_ccapi->disconnect();
				return;
			}
			this_thread::sleep_for(chrono::milliseconds(50));
		}
		if (m_ccapi->attach() == 0)
		{
			m_ccapi->disconnect();
			m_status = "ERROR";
			m_error = TASK_ERROR_NO_ATTACH;
			return;
		}
		m_status = "CONNECTED";
	}
	else
	{
		m_status = "ERROR";
		m_error = TASK_ERROR_NO_CONNECT;
		return;
	}
	while (m_fire != true)
	{
		if (m_cancel)
		{
			m_ccapi->disconnect();
			return;
		}
		this_thread::sleep_for(chrono::milliseconds(50));
		if (m_errorCallback != nullptr)
		{
			if (m_errorCallback() != RANGEMEMORY_ERROR_NONE)
			{
				m_ccapi->disconnect();
				return;
			}
		}
	}
	m_status = "FINDRANGE";

	for (unsigned long long i=m_offset; i<m_offset+m_length; i+=RANGE_INTERVAL)
	{
		if (m_cancel)
			return;
		int check = m_ccapi->validateMemory(i);
		if (check)
		{
			if (!m_lastGoodCheck)
			{
				m_pair.first(i);
				m_lastGoodCheck = true;
			}
		}
		else
		{
			if (m_lastGoodCheck)
			{
				m_pair.second(i);
				m_lastGoodCheck = false;
				m_intervals.push_back(RangePair(m_pair.first(), m_pair.second()));
				m_pair.first(-1);
				m_pair.second(-1);
			}
		}
		if (m_callback != nullptr)
		{
			m_callback(this, i-m_offset, m_length);
		}
	}
	if (m_pair.first() != -1 && m_pair.second() == -1) //we have a continuing pair
		m_intervals.push_back(RangePair(m_pair.first(), m_pair.second()));
	if (m_callback != nullptr)
	{
		m_callback(this, m_length, m_length);
	}
	m_status = "DONE";
}
