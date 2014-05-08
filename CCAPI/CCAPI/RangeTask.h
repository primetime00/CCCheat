#ifndef _RANGETASK_
#define _RANGETASK_

#include <vector>
#include <utility>
#include "Task.h"


using namespace std;

class RangeTask: public Task
{
public:
	RangeTask(string ip, unsigned long long offset, unsigned long long length, unsigned int id, string name);
	~RangeTask() {}

	virtual void run(void);


	void setCallback(function<void(RangeTask*, long long, long long)> cb) { m_callback = cb; }
	void setErrorCallback(function<unsigned int(void)> cb) { m_errorCallback = cb; }
	vector<pair<long long, long long>> getIntervals() { return m_intervals; }
	pair<long long, long long> getCurrentPair() { return m_pair; }

	long long getLastOffset() { return m_length+m_offset; }
	long long getFirstOffset() { return m_offset; }
	long long getLength() { return m_length; }

	

protected:
	function<void(RangeTask*, long long, long long)> m_callback;
	function<unsigned int(void)> m_errorCallback;

private:
	vector<pair<long long, long long>> m_intervals;
	pair<long long, long long> m_pair;
	bool m_lastGoodCheck;
	long long m_firstValidOffset;

};

#endif