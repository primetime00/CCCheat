#ifndef _RANGEMEMORY_
#define _RANGEMEMORY_

#include "RangeTask.h"

using namespace std;

#define RANGEMEMORY_ERROR_NONE			0x00
#define RANGEMEMORY_ERROR_NO_CONNECT	0x01
#define RANGEMEMORY_ERROR_NO_ATTACH		0x02

class RangeMemory
{
public:
	RangeMemory(string ip, int ccapiVersion, unsigned long long offset, unsigned long long length);
	~RangeMemory() {}

	unsigned int process();
	void cancel();
	void callBack(RangeTask* task, long long current, long long size);
	float getProgress();
	string getStatus() { return m_status; }
	RangeList getRanges();
	unsigned int getError() { return m_error; }


private:

	void calculateThreads();

	unsigned long long m_offset, m_length;
	string m_ip;
	string m_status;
	unsigned int m_error;
	bool m_cancel;

	int m_threadCount;
	long long m_totals[MAX_THREADS];
	//vector<pair<long long, long long>> m_ranges;
	vector<RangePair> m_ranges;
	vector<shared_ptr<RangeTask>> m_threadList;
	int m_ccapiHostVersion;

};
#endif