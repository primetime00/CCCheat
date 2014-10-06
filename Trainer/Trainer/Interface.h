#ifndef _TRAINERINTERFACE_
#define _TRAINERINTERFACE_

#include <thread>
#include "CCAPI/CCAPI.h"
#include "CCAPI/Common.h"
#include "CCAPI/MemoryOperator.h"
#include "UI/Common.h"
#include "Trainer.h"

#ifndef _HOME
#define PS3_IP	"127.0.0.1"
#else
#define PS3_IP	"192.168.137.2"
#endif

#define INTERFACE_CONNECT_SUCCESS	0x00
#define INTERFACE_CONNECT_FAIL		0x01
#define INTERFACE_DISCONNECT		0x02
#define INTERFACE_CONNECTING		0x03


#ifdef _WIN32
	#include <direct.h>
    #define GetCurrentDir _getcwd
#else
	#include <unistd.h>
    #define GetCurrentDir getcwd
#endif

#define get_user_data(type, data) (type)((unsigned long)data)

class TrainerClass;

class Interface {
public:
	Interface(CCAPI *ccapi, TrainerClass *ui) { 
		m_ccapi = ccapi; m_ui = ui; instance = this; memoryOperator = 0; 
	}

	static void connectCheck(void *v) { if (instance != 0) { instance->_connectCheck(); } }
	void _connectCheck();
	void connect(string ip);
	void disconnect();

	void disable(rkTrainerCodeObj *item);
	void enable(rkTrainerCodeObj *item);

	static Interface *instance;

private:

	TrainerClass *m_ui;
	CCAPI *m_ccapi;

	thread m_launcher;
	MemoryOperator *memoryOperator;
	shared_ptr<ConnectionManager> connection;
	
	string m_ip;
	

};
#endif