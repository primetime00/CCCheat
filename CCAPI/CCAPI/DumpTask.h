#ifndef _DUMPTASK_
#define _DUMPTASK_

#include <vector>
#include <utility>
#include <map>
#include "Task.h"

#ifdef _WIN32
	#include <direct.h>
    #define GetCurrentDir _getcwd
#else
	#include <unistd.h>
    #define GetCurrentDir getcwd
#endif


using namespace std;

class DumpTask: public Task
{
public:
	DumpTask(string ip, unsigned long long offset, unsigned long long length, unsigned int id, string name);
	~DumpTask();

	virtual void run(void);

	int connectAndAttach(int retry=3);

	int connect(int loops=10);
	int attach();



	void setProgressCallback(function<void(DumpTask*, long long, long long)> cb) { m_progressCallback = cb; }
	void setErrorCallback(function<unsigned int(void)> cb) { m_errorCallback = cb; }

	long long getLastOffset() { return m_length+m_offset; }
	long long getFirstOffset() { return m_offset; }
	long long getLength() { return m_length; }
	char *getBuffer() {return m_buffer;}


	

protected:
	function<void(DumpTask*, long long, long long)> m_progressCallback;
	function<unsigned int(void)> m_errorCallback;

private:
	void disconnect();

	int doDump();

	void calculateReads(unsigned long interval, unsigned long &rounds, unsigned long &remain);
	unsigned int waitForFire();

	void progressCallback(DumpTask* task, long long c, long long t) { if (m_progressCallback != nullptr) { m_progressCallback(task,c,t); } }

	bool m_connected;
	char *m_buffer;
};

#endif
