#ifndef __TRAINER__
#define __TRAINER__

#include <memory.h>
#include <memory>
#include "CCAPI/Types.h"

using namespace std;

#define TITLELEN		256
#define AUTHORLEN		128
#define INFOLEN			4096
#define CODESLEN		32768
#define GAMETITLELEN	256
#define VERSIONLEN		32
#define REGIONLEN		32

#define TITLE_TXT "TRAIN_TITLE"
#define AUTHOR_TXT "TRAIN_AUTHOR"
#define INFO_TXT "TRAIN_INFO"
#define CODES_TXT "TRAIN_CODES"
#define GAME_TITLE_TXT "TRAIN_GAME_TITLE"
#define VERSION_TXT "TRAIN_GAME_VERSION"
#define REGION_TXT "TRAIN_GAME_REGION"

extern char trainerTitle[TITLELEN];
extern char trainerAuthor[AUTHORLEN];
extern char trainerInfo[INFOLEN];
extern char trainerCodeDesc[CODESLEN];
extern char trainerGameTitle[GAMETITLELEN];
extern char trainerGameVersion[VERSIONLEN];
extern char trainerGameRegion[REGIONLEN];

#if 0
struct codeFmt
{
	string name;
	char condition;
	char conditionType;
	long long conditionValue;
	AddressItem conditionItem;
	AddressList codeItems;
	codeFmt() { name = ""; conditionItem = nullptr; codeItems.clear(); }
	~codeFmt() { conditionItem = nullptr; codeItems.clear(); }

	int write(char *buf)
	{
		int pos = 0;
		buf[pos] = name.length(); pos+=1;
		memcpy(&buf[pos], name.c_str(), (int)name.length()); pos += (int)name.length();
		buf[pos] = condition; pos+=1;
		if (condition)
		{
			buf[pos] = conditionType; pos+=1;
			conditionItem->value = conditionValue;
			pos += conditionItem->write(&buf[pos]);
		}
		buf[pos] = codeItems.size(); pos+=1;
		for (auto i=codeItems.begin(); i != codeItems.end(); ++i)
		{
			pos += (*i)->write(&buf[pos]);
		}
		return pos;
	}

	int read(char *buf)
	{
		int pos = 0;
		int nameLen = buf[pos]; pos+=1;
		name = string(&buf[pos], nameLen); pos += (int)nameLen;
		condition = buf[pos]; pos+=1;
		if (condition)
		{
			conditionType = buf[pos]; pos+=1;
			conditionItem = make_shared<AddressObj>();
			pos += conditionItem->read(&buf[pos]);
		}
		int numOfCodes = buf[pos]; pos+=1;
		for (int i=0; i< numOfCodes; ++i)
		{
			auto code = make_shared<AddressObj>();
			pos += code->read(&buf[pos]);
			codeItems.push_back(code);
		}
		return pos;
	}

};

typedef shared_ptr<codeFmt> codeFmtItem;
#endif

#endif