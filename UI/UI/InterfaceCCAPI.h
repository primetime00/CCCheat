#ifndef _INTERFACECCAPI_
#define _INTERFACECCAPI_
#include <thread>
#include "CCAPI/CCAPI.h"
#include "CCAPI/Common.h"
#include "Common.h"
#include "CCAPI/RangeMemory.h"
#include "CCAPI/SearchMemory.h"
#include "CCAPI/MemoryOperator.h"
#include "ui_main.h"

#ifndef _HOME
#define PS3_IP	"127.0.0.1"
#else
#define PS3_IP	"192.168.137.2"
#endif

#define INTERFACE_CONNECT_SUCCESS	0x00
#define INTERFACE_CONNECT_FAIL		0x01
#define INTERFACE_DISCONNECT		0x02
#define INTERFACE_CONNECTING		0x03

class rkCheatUI;

class InterfaceCCAPI {
public:
	InterfaceCCAPI(CCAPI *ccapi, rkCheatUI *ui) { m_ccapi = ccapi; m_ui = ui; instance = this; lastSearchType = SEARCH_TYPE_UNKNOWN; currentSearchIndex = 0; memoryOperator = 0; ranger = 0;}

	static void findAddresses();
	void _findAddresses();

	static void cancelFindAddresses();
	void _cancelFindAddresses();

	static void findAddressProgress(void *);
	bool _findAddressProgress();

	static void searchProgress(void *);
	bool _searchProgress(SearchMemory *);


	void _launchFindAddress();
	void _processAddresses();

	bool _searchInProgress() { return searchList.size() > 0; }
	void _resetSearch();

	void _launchSearch();
	void _processSearchResults(SearchMemory *);

	static void startNewSearch();
	void _startNewSearch();

	static void continueSearch();
	void _continueSearch();

	void _cycleSearch();

	bool _isSearching() { if (searcher) return searcher->isRunning(); else return false; }

	void _cancelSearch();

	static void connectCheck(void *v) { if (instance != 0) { instance->_connectCheck(); } }
	void _connectCheck();
	void connect(string ip);
	void disconnect();

	static InterfaceCCAPI *instance;

	bool searchInProgress() { return searchList.size() > 0; }

	char getSearchValueType() { return searcher ? searcher->getSearchValueType() : 0; }

	unsigned long getNumberOfResults();
private:
	bool isSearchFinished();
	SearchMemory *getCurrentSearch();

	rkCheatUI *m_ui;
	CCAPI *m_ccapi;

	thread m_launcher;
	RangeMemory *ranger;
	SearchMemory *searcher;
	MemoryOperator *memoryOperator;
	
	vector<SearchMemory*> searchList;

	rkCheat_Results searchResults;

	char lastSearchType;
	int currentSearchIndex;
	string m_ip;
	

};
#endif