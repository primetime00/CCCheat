#ifndef _SEARCHMEMORY_
#define _SEARCHMEMORY_

#include "SearchTask.h"
#include <mutex>

using namespace std;

typedef vector<pair<Task*, pair<unsigned long long, unsigned long long>>> ProgressList;

class SearchMemory
{
public:
	SearchMemory(string ip, int ccapiVersion, unsigned long long offset, unsigned long long length);
	~SearchMemory() {}

	unsigned int process();
	void cancel();
	void progressCallBack(SearchTask* task, long long current, long long total);
	float getProgress();
	string getStatus() { return m_status; }
	void resetStatus() { m_status = "INIT"; }
	unsigned int getError() { return m_error; }
	void stopThreads() { for (unsigned int i=0; i<m_threadList.size(); i++) { m_threadList.at(i)->join(); } }

	void setSearchType(char type) { m_searchType = type; }
	void setSearchValueType(char type) { m_searchValueType = type; }
	void setSearchCompareType(char type) { m_compareType = type; }
	void setSearchValue(long long val) { m_searchValue = val; }

	char getSearchType() { return m_searchType; }
	char getSearchValueType() { return m_searchValueType; }
	long long getSearchValue() { return m_searchValue; }
	vector<ResultList> *getResults() { return &m_threadResults; }
	bool isDump() { return m_dump; }
	bool isRunning() { return m_running; }
	unsigned long getNumberOfResults();
	void reOrganizeResults();

private:

	void calculateThreads();

	unsigned long long m_offset, m_length;
	string m_ip;
	string m_status;
	unsigned int m_error;

	int m_threadCount;
	ProgressList m_threadProgressList;
	vector<shared_ptr<SearchTask>> m_threadList;
	vector<ResultList> m_threadResults;

	char m_searchType;
	char m_searchValueType;
	char m_compareType;
	long long m_searchValue;
	unsigned int m_searchCount;
	bool m_dump;
	bool m_cancel;
	bool m_running;
	bool m_reorganize;
	mutex cancelLock;

	int m_ccapiHostVersion;

};
#endif