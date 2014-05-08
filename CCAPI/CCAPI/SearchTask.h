#ifndef _SEARCHTASK_
#define _SEARCHTASK_

#include <vector>
#include <utility>
#include <map>
#include "Task.h"
#include "ChunkValueSearch.h"
#include "ChunkFuzzySearch.h"

#define DUMP_DIR	"C:/temp/"

using namespace std;

class SearchTask: public Task
{
public:
	SearchTask(string ip, unsigned long long offset, unsigned long long length, unsigned int id, string name);
	~SearchTask();

	virtual void run(void);

	void setSearchType(unsigned char type) { m_searchType = type; }
	void setSearchValueType(unsigned char type) { m_searchValueType = type; }
	void setSearchValue(long long val) { m_searchValue = val; }
	void setSearchCompareType(unsigned char type) { m_compareType = type; }
	void setResultList(ResultList *r)  { m_results = r; }
	int connectAndAttach(int retry=3);
	int doFuzzySearch();
	int doValueSearch();
	void setLastThread(bool v) { m_isLastThread = v; }

	int connect(int loops=10);
	int attach();



	void setProgressCallback(function<void(SearchTask*, long long, long long)> cb) { m_progressCallback = cb; }
	void setErrorCallback(function<unsigned int(void)> cb) { m_errorCallback = cb; }

	long long getLastOffset() { return m_length+m_offset; }
	long long getFirstOffset() { return m_offset; }
	long long getLength() { return m_length; }
	string getDumpFilename() { return m_dumpFile; }

	static unsigned long getNumberOfResults(ResultList resList);

	

protected:
	function<void(SearchTask*, long long, long long)> m_progressCallback;
	function<unsigned int(void)> m_errorCallback;

private:
	int initialFuzzySearch();
	int continueFuzzySearch();
	int fuzzySearchDumpFile(ifstream &resFile);
	int fuzzySearchResults();

	int initialValueSearch();
	int continueValueSearch();
	int valueSearchResults();
	void disconnect();

	void calculateReads(unsigned long interval, unsigned long &rounds, unsigned long &remain);
	unsigned int waitForFire();

	void progressCallback(SearchTask* task, long long c, long long t) { if (m_progressCallback != nullptr) { m_progressCallback(task,c,t); } }

	void dumpDataToFile(ostream &dumpFile, unsigned long location, unsigned long length);

	string m_dumpFile;
	ResultList *m_results;
	unsigned char m_searchType;
	unsigned char m_compareType;
	long long m_searchValue;
	unsigned char m_searchState;
	unsigned char m_searchValueType;
	bool m_isLastThread;
	ChunkValueSearch m_valueSearcher;
	ChunkFuzzySearch m_fuzzySearcher;
	bool m_connected;
	bool m_isFuzzy;
};

#endif