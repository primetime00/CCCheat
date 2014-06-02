#ifndef _TASK_
#define _TASK_

#include <thread>
#include <string>
#include <memory>
#include <functional>
#include "Common.h"
#include "CCAPI.h"

#define RANGE_INTERVAL 0x10000

#define MAX_MEMORY_INTERVAL 0x10000
#define MAX_THREADS 16

#define TASK_ERROR_NONE			0x0
#define TASK_ERROR_NO_CONNECT	CCAPI_ERROR_NO_CONNECT
#define TASK_ERROR_NO_ATTACH	CCAPI_ERROR_NO_ATTACH
#define TASK_ERROR_CANCEL		CCAPI_ERROR_CANCEL
#define TASK_ERROR_FILE_FAIL	CCAPI_ERROR_FILE_FAIL

using namespace std;
class Task
{
public:
	Task(unsigned int id, string name);
	~Task() { if (m_thread.joinable()) m_thread.join(); };

	void start();
	void join() { 
		if (m_thread.joinable()) 
			m_thread.join(); 
	}
	void cancel() { m_cancel = true; m_ccapi->disconnect();}

	void fire() { m_fire = true; }

	string getStatus() { return m_status; }

	unsigned int getError() { return m_error; }
	void setHostCCAPIVersion(int ver) { m_ccapiHostVersion =ver; }

	//void setCallback(taskCallback c) { m_callback = c; }


	virtual void run(void);

	enum TaskState{
		WAIT,
		WORKING,
		COMPLETE,
	} m_taskState;

	TaskState getState() { return m_taskState; }
	
protected:

	unsigned int m_id;
	string m_name;
	thread m_thread;
	shared_ptr<CCAPI> m_ccapi;

	unsigned long long m_offset;
	unsigned long long m_length;

	string m_ip;
	bool m_fire;
	bool m_cancel;

	unsigned int m_error;


	string m_status;
	int m_ccapiHostVersion;

private:
	Task();
};

#endif