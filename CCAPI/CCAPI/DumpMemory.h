#ifndef _DUMPMEMORY_
#define _DUMPMEMORY_

#include "DumpTask.h"
#include <mutex>

using namespace std;

typedef vector<pair<Task*, pair<unsigned long long, unsigned long long>>> ProgressList;

class DumpMemory
{
public:
	DumpMemory(string ip, int ccapiVersion, unsigned long long offset, unsigned long long length);
	~DumpMemory();
	unsigned int process();
	void cancel();
	void progressCallBack(DumpTask* task, long long current, long long total);
	float getProgress();
	string getStatus() { return m_status; }
	void resetStatus() { m_status = "INIT"; }
	unsigned int getError() { return m_error; }
	void stopThreads() { for (unsigned int i=0; i<m_threadList.size(); i++) { m_threadList.at(i)->join(); } }
	bool isRunning() { return m_running; }
	DumpData consolideDump();

private:

	void calculateThreads();

	unsigned long long m_offset, m_length;
	string m_ip;
	string m_status;
	unsigned int m_error;

	int m_threadCount;
	ProgressList m_threadProgressList;
	vector<shared_ptr<DumpTask>> m_threadList;

	bool m_cancel;
	bool m_running;
	mutex cancelLock;
	mutex statusLock;

	int m_ccapiHostVersion;

};
#endif