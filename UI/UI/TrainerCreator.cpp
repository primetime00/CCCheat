#include "TrainerCreator.h"
#include "Trainer/Trainer.h"
#include "TrainerSrc.h"
#include <vector>
#include <algorithm>
#include <fstream>

using namespace std;

TrainerCreator::TrainerCreator()
{
	m_title = "";
	m_region = "";
	m_gameTitle = "";
	m_author = "";
	m_info = "";
	totalCodeSize = 0;

	//lets find the locations
	vector<unsigned char> s;
	s.assign(trainSrc, trainSrc+sizeof(trainSrc));
	auto it = search(s.begin(), s.end(), TITLE_TXT, TITLE_TXT+strlen(TITLE_TXT));			m_locationMap[TITLE_TXT] = rkPair(it - s.begin(), TITLELEN);
	it = search(s.begin(), s.end(), AUTHOR_TXT, AUTHOR_TXT+strlen(AUTHOR_TXT));				m_locationMap[AUTHOR_TXT] = rkPair(it - s.begin(), AUTHORLEN);
	it = search(s.begin(), s.end(), INFO_TXT, INFO_TXT+strlen(INFO_TXT));					m_locationMap[INFO_TXT] = rkPair(it - s.begin(), INFOLEN);
	it = search(s.begin(), s.end(), CODES_TXT, CODES_TXT+strlen(CODES_TXT));				m_locationMap[CODES_TXT] = rkPair(it - s.begin(), CODESLEN);
	it = search(s.begin(), s.end(), GAME_TITLE_TXT, GAME_TITLE_TXT+strlen(GAME_TITLE_TXT)); m_locationMap[GAME_TITLE_TXT] = rkPair(it - s.begin(), GAMETITLELEN);
	it = search(s.begin(), s.end(), VERSION_TXT, VERSION_TXT+strlen(VERSION_TXT));			m_locationMap[VERSION_TXT] = rkPair(it - s.begin(), VERSIONLEN);
	it = search(s.begin(), s.end(), REGION_TXT, REGION_TXT+strlen(REGION_TXT));				m_locationMap[REGION_TXT] = rkPair(it - s.begin(), REGIONLEN);
}

TrainerCreator::~TrainerCreator()
{
}

void TrainerCreator::setCodes(vector<rkTrainerCode> &codes)
{
	int pos = 1;
	codeBuffer[0] = codes.size();
	for (auto it = codes.begin(); it != codes.end(); ++it)
	{
		pos += (*it)->write(&codeBuffer[pos]);
	}
	totalCodeSize = pos;
}

void TrainerCreator::inject(string findValue, string replaceValue)
{
	if (m_locationMap.count(findValue) == 0)
		return;
	unsigned char *pt = &trainSrc[m_locationMap[findValue].location];
	int size = m_locationMap[findValue].length;
	memset(pt, 0, size);
	memcpy(pt, replaceValue.c_str(), replaceValue.length());
}

void TrainerCreator::injectCode(char *buffer)
{
	if (m_locationMap.count(CODES_TXT) == 0)
		return;
	unsigned char *pt = &trainSrc[m_locationMap[CODES_TXT].location];
	int size = m_locationMap[CODES_TXT].length;
	memset(pt, 0, size);
	memcpy(pt, (unsigned char*)codeBuffer, totalCodeSize);
}

void TrainerCreator::exportTrainer(bool debug)
{
	if (codeBuffer[0] == 0)
		return;
#if defined(_WIN32) || defined(WIN32)
	string fname = m_title + "-" + m_gameTitle + ".exe";
#else
	string fname = m_title + "-" + m_gameTitle;
#endif
	inject(TITLE_TXT, m_title);
	inject(AUTHOR_TXT, m_author);
	inject(INFO_TXT, m_info);
	inject(GAME_TITLE_TXT, m_gameTitle);
	inject(VERSION_TXT, m_version);
	inject(REGION_TXT, m_region);
	injectCode(codeBuffer);
	ofstream outFile(fname.c_str(), ofstream::binary);
	outFile.write((char*)trainSrc, sizeof(trainSrc));
	outFile.close();
	if (debug)
	{
		ofstream outDebugFile("dbg.dat", ofstream::binary);
		outDebugFile.write((char*)codeBuffer, totalCodeSize);
		outDebugFile.close();

	}
}