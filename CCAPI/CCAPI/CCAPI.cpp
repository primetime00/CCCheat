#include "CCAPI.h"
#include <iostream>
#if defined(_WIN32) || defined(WIN32)
#include <intrin.h>
#define CLOSESOCKET(x) closesocket(x)
#else
#include <byteswap.h>
#include <unistd.h>
#include <cstring>
#define CLOSESOCKET(x) close(x)
#endif

using namespace std;

unsigned char CCAPI::bitConv[] = {0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA, 0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1, 0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5, 0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD, 0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};
bool CCAPI::initialized = false;

CCAPI::CCAPI(string ip)
{
	startup();
	ipAddress = ip;
	attachedPID = 0;
	destination.sin_family = AF_INET;
	destination.sin_port = htons (CCAPI_PORT_25);
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
	{
		cout << "\nSocket Creation FAILED!" << endl;
	}
	dataBuffer = new char[CCAPI_DATA_BUFFER];
	connected = attached = false;
}


int CCAPI::connect(void)
{
	if (hostVersion == 20)
		destination.sin_port = htons(CCAPI_PORT_20);
	else
		destination.sin_port = htons(CCAPI_PORT_25);
	destination.sin_addr.s_addr = inet_addr(ipAddress.c_str());
	if (::connect(sock,(struct sockaddr *)&destination,sizeof(destination))!=0)
	{
		cout << "\nSocket Connection FAILED!" << endl;
		if (sock) 
		{
			CLOSESOCKET(sock);
			return -1;
		}
	}
	connected = true;
	return 0;
}


int CCAPI::disconnect(void)
{
	connected = false;
	attached = false;
	if (sock) 
	{
		CLOSESOCKET(sock);
		return 0;
	}
	return -1;
}

int CCAPI::receiveData(void) 
{
	const unsigned int bufferSize = 4096;
	ssize_t numOfBytes = 0;
	unsigned int i, size, command;

	numOfBytes = recv(sock, dataBuffer, bufferSize, 0);
	if (numOfBytes == -1) //we've disconnected
		return -1;
	for (i=0; i<numOfBytes; i++) { 
		dataBuffer[i] = _conv8(dataBuffer[i]); 
	}
	while (numOfBytes < 16)
	{
		int _numOfBytes = recv(sock, &dataBuffer[numOfBytes], bufferSize, 0);
		if (_numOfBytes == -1) return -1;//now we really disconnected!
		for (i=numOfBytes; i<numOfBytes+_numOfBytes; i++) { dataBuffer[i] = _conv8(dataBuffer[i]); }
		numOfBytes += _numOfBytes;
	}
	size = getDataSize(dataBuffer);
	command = getDataCommand(dataBuffer);
	while (numOfBytes < size)
	{
		int _numOfBytes = recv(sock, &dataBuffer[numOfBytes], bufferSize, 0);
		if (_numOfBytes == -1) return -1;//now we really disconnected!
		for (i=numOfBytes; i<numOfBytes+_numOfBytes; i++) { dataBuffer[i] = _conv8(dataBuffer[i]); }
		numOfBytes += _numOfBytes;
	}
	switch (command)
	{
	case CCAPI_COMMAND_GETPROCESSLIST: parseProcessIDs(dataBuffer); break;
	case CCAPI_COMMAND_GETPROCESSNAME: parseProcessName(dataBuffer); break;
	case CCAPI_COMMAND_READPROCESSMEMORY: parseProcessMemory(dataBuffer); break;
	default: break;
	}
	return 0;
}

int CCAPI::parseProcessIDs(char *data)
{
	unsigned int *intData = (unsigned int*)data;
	unsigned int numberOfProcs = BSWAP32( intData[4] );
	processIDs.clear();
	if (numberOfProcs > 20)
		return -1;
	for (unsigned int i=0, startLocation=5; i<numberOfProcs; i++, startLocation++)
		processIDs.push_back( BSWAP32( intData[startLocation] ));
	return processIDs.size();
}

int CCAPI::parseProcessName(char *data)
{
	processName = "";
	for (int i=0, startLocation=16; i<5000; i++, startLocation++) //we will look for a string with a max length of 5000 characters
	{
		if (data[startLocation] != 0)
			processName.push_back( data[startLocation] );
		else
			break;
	}
	return 0;
}

int CCAPI::parseProcessMemory(char *data)
{
	return 0;
}

int CCAPI::requestProcessIDList(void)
{
	if (!sock)
		return -1;
	char header[30];
	unsigned size=0;
	constructHeader(header, size, CCAPI_SIZE_GETPROCESSLIST, CCAPI_COMMAND_GETPROCESSLIST);
	if (send(sock, header, size, 0) != size)
		return -1;
	return 0;
}


int CCAPI::attach(void)
{
	attached = false;
	if (!sock)
		return -1;
	attachedPID = 0;
	requestProcessIDList();
	if (receiveData() == -1)
		return -1;
	for (unsigned int i=0; i<processIDs.size(); i++)
	{
		requestProcessName(processIDs.at(i));
		if (receiveData() == -1)
			return -1;
		if (processName.find("flash") == string::npos)
		{
			attachedPID = processIDs.at(i);
			attached = true;
			break;
		}
	}
	return attachedPID;
}

int CCAPI::readMemory(unsigned int offset, unsigned int length)
{
	if (!sock)
		return -1;
	if (attachedPID == 0)
		return -2;
	requestReadMemory(attachedPID, offset, length);
	if (receiveData() == -1)
		return -1;
	return 0;
}

int CCAPI::validateMemory(unsigned int offset)
{
	if (!sock)
		return -1;
	if (attachedPID == 0)
		return -2;
	requestReadMemory(attachedPID, offset, 1);
	if (receiveData() == -1)
		return -1;
	return getDataValidity(dataBuffer) == 0;
}

int CCAPI::writeMemory(unsigned int offset, unsigned int length, char *data)
{
	if (!sock)
		return -1;
	if (attachedPID == 0)
		return -2;
	//convert the memory here?
	requestWriteMemory(attachedPID, offset, length, data);
	if (receiveData() == -1)
		return -1;
	return 0;
}

int CCAPI::requestProcessName(unsigned int processID)
{
	if (!sock)
		return -1;
	char header[64];
	unsigned size=0;
	constructHeader(header, size, CCAPI_SIZE_GETPROCESSNAME, CCAPI_COMMAND_GETPROCESSNAME);
	processID = _conv32(processID);
	memcpy(&header[size], &processID, PS3_INT_SIZE); size+=PS3_INT_SIZE;
	if (send(sock, header, size, 0) != size)
		return -1;
	return 0;
}

int CCAPI::requestReadMemory(unsigned int processID, unsigned int offset, unsigned int length)
{
	if (!sock)
		return -1;
	char header[64];
	unsigned size=0;
	constructHeader(header, size, CCAPI_SIZE_READPROCESSMEMORY, CCAPI_COMMAND_READPROCESSMEMORY);
	unsigned int pack[] = { _conv32(processID), 0, _conv32(offset), _conv32(length) };
	memcpy(&header[size], pack, PS3_INT_SIZE*4); size+=PS3_INT_SIZE*4;
	if (send(sock, header, size, 0) != size)
		return -1;
	return 0;
}

int CCAPI::requestWriteMemory(unsigned int processID, unsigned int offset, unsigned int length, char *data)
{
	if (!sock)
		return -1;
	char header[64];
	unsigned size=0;
	constructHeader(header, size, CCAPI_SIZE_WRITEPROCESSMEMORY+length, CCAPI_COMMAND_WRITEPROCESSMEMORY);
	unsigned int pack[] = { _conv32(processID), 0, _conv32(offset), _conv32(length) };
	memcpy(&header[size], pack, PS3_INT_SIZE*4); size+=PS3_INT_SIZE*4;
	for (unsigned int i=0; i<length; i++)
		data[i] = bitConv[(unsigned char)data[i]];
	memcpy(&header[size], data, length); size+=length;
	if (send(sock, header, size, 0) != size)
		return -1;
	return 0;
}

void CCAPI::constructHeader(char *buffer, unsigned int &buffersize, unsigned int size, unsigned int command, unsigned int third, unsigned int fourth)
{
	size = _conv32(size);//_conv32(size);
	command = _conv32(command);
	third = _conv32(third);
	//third = _conv32(third);
	fourth = _conv32(fourth);
	unsigned int i=0;
	memcpy(&buffer[i], &size, sizeof(unsigned int)); i+=sizeof(unsigned int);
	memcpy(&buffer[i], &command, sizeof(unsigned int)); i+=sizeof(unsigned int);
	memcpy(&buffer[i], &third, sizeof(unsigned int)); i+=sizeof(unsigned int);
	memcpy(&buffer[i], &fourth, sizeof(unsigned int)); i+=sizeof(unsigned int);
	buffersize = i;
}

bool CCAPI::insertData(unsigned int pos, char *data, unsigned int size)
{
	if (pos+size >= CCAPI_DATA_BUFFER-16)
		return false;
	char *dp = &dataBuffer[16];
	memcpy(&dp[pos], data, size);
	return true;
}

