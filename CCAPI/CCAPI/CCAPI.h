#ifndef _CCAPI_
#define _CCAPI_

#if defined(_WIN32) || defined(WIN32)
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <string>
#include <vector>
#include "Common.h"


#define CCAPI_PORT_20 1977
#define CCAPI_PORT_25 1978

#define CCAPI_MAX_BYTE_REQUEST 0x10000
#define CCAPI_DATA_BUFFER 0x15000

#define CCAPI_COMMAND_GETPROCESSLIST		0x04
#define CCAPI_COMMAND_GETPROCESSNAME		0x05
#define CCAPI_COMMAND_READPROCESSMEMORY		0x02
#define CCAPI_COMMAND_WRITEPROCESSMEMORY	0x03

#define CCAPI_SIZE_GETPROCESSLIST			0x10
#define CCAPI_SIZE_GETPROCESSNAME			0x14
#define CCAPI_SIZE_READPROCESSMEMORY		0x20
#define CCAPI_SIZE_WRITEPROCESSMEMORY		0x20

#define CCAPI_ERROR_NO_ERROR	0x00
#define CCAPI_ERROR_NO_CONNECT	0x01
#define CCAPI_ERROR_NO_ATTACH	0x02
#define CCAPI_ERROR_CANCEL	0x03
#define CCAPI_ERROR_FILE_FAIL	0x04

#define PS3_INT_SIZE	4



using namespace std;

class CCAPI
{
public:
	CCAPI(string ip);
	~CCAPI() {delete [] dataBuffer;}
	static void startup() {
		if (!initialized)
		{
#if defined(_WIN32) || defined(WIN32)
			WSADATA wsaData;
			WSAStartup(0x0202, &wsaData);
#endif
			initialized = true;
		}
	}
	static unsigned char bitConv[];
	static bool initialized;

	int connect(void);
	int disconnect(void);
	int receiveData(void);
	int requestProcessIDList(void);
	int requestProcessName(unsigned int processID);
	int requestReadMemory(unsigned int processID, unsigned int offset, unsigned int length);
	int requestWriteMemory(unsigned int processID, unsigned int offset, unsigned int length, char *data);
	int attach(void);
	int readMemory(unsigned int offset, unsigned int length);
	int validateMemory(unsigned int offset);
	int writeMemory(unsigned int offset, unsigned int length, char *data);

	char *getData(unsigned int &length) { length = getDataSize(dataBuffer)-16; return &dataBuffer[16]; }
	bool insertData(unsigned int pos, char *data, unsigned int size);

	bool isConnected() { return connected; }
	bool isAttached() { return attached; }
	void setHostVersion(int ver) { hostVersion = ver; }


private:
	unsigned int _conv32(unsigned int val) {
		//return BSWAP32(  (bitConv[((char*)&val)[0]] << 24) + (bitConv[((char*)&val)[1]] << 16) + (bitConv[((char*)&val)[2]] << 8) + (bitConv[((char*)&val)[3]] << 0) );
		return (  (bitConv[((unsigned char*)&val)[0]] << 24) + (bitConv[((unsigned char*)&val)[1]] << 16) + (bitConv[((unsigned char*)&val)[2]] << 8) + (bitConv[((unsigned char*)&val)[3]] << 0) );
	}
	unsigned short _conv16(unsigned short val) {
		//return BSWAP16(  (bitConv[((char*)val)[0]] << 8) + (bitConv[((char*)val)[1]] << 0) );
		return (  (bitConv[((unsigned char*)val)[0]] << 8) + (bitConv[((unsigned char*)val)[1]] << 0) );
	}
	unsigned char _conv8(unsigned char val) {
		return bitConv[(unsigned char)val];
	}

	unsigned int getDataCommand(char *data) { return BSWAP32( ((unsigned int*)data)[1] ); }
	unsigned int getDataSize(char *data) { return BSWAP32( ((unsigned int*)data)[0] ); }
	unsigned int getDataID(char *data) { return BSWAP32( ((unsigned int*)data)[2] ); }
	unsigned int getDataValidity(char *data) { return BSWAP32( ((unsigned int*)data)[3] ); }
	int parseProcessIDs(char *data);
	int parseProcessName(char *data);
	int parseProcessMemory(char *data);
	void constructHeader(char *buffer, unsigned int &buffersize, unsigned int size, unsigned int command, unsigned int third=1444, unsigned int fourth=1164476209);

private:
	string ipAddress;
	int sock;
	struct sockaddr_in destination;
	unsigned int attachedPID;
	char *dataBuffer;
	vector<unsigned int> processIDs;
	string processName;
	bool connected;
	bool attached;
	int hostVersion;

};

#endif
