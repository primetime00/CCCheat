#ifndef __TRAINERCREATOR__
#define __TRAINERCREATOR__

#include <string>
#include <map>
#include "Common.h"
#include "Trainer/Trainer.h"

using namespace std;

class TrainerCreator
{
public:
	TrainerCreator();
	~TrainerCreator();

	void setTitle(string title) { m_title = title; }
	void setAuthor(string author) { m_author = author; }
	void setRegion(string region) { m_region = region; }
	void setGameTitle(string title) { m_gameTitle = title; }
	void setInformation(string info) {m_info = info; }
	void setVersion(string version) { m_version = version; }

	void setCodes(vector<rkTrainerCode> &codes);

	void exportTrainer(bool debug = false);

	struct rkPair
	{
		long long location;
		int length;
		rkPair() { location = 0; length = 0; }
		rkPair(long long loc, int len) { location = loc; length = len; }
		rkPair& operator=(const rkPair &t) { location = t.location; length = t.length; return *this; };
	};

private:
	void inject(string findValue, string replaceValue);
	void injectCode(char *buffer);

	map<string, rkPair> m_locationMap;

	string m_title;
	string m_author;
	string m_region;
	string m_gameTitle;
	string m_info;
	string m_version;
	char codeBuffer[CODESLEN];
	int totalCodeSize;
};

#endif