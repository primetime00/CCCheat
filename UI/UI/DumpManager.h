#ifndef __DUMPMAN__
#define __DUMPMAN__

#include "CCAPI/Common.h"
#include "DumpScan.h"
#include <fstream>
#include <thread>
#include <mutex>

using namespace std;

#define DATAMANAGER_STATUS_COMPLETE		0
#define DATAMANAGER_STATUS_WORKING		1
#define DATAMANAGER_STATUS_CANCELED		2
#define DATAMANAGER_STATUS_IDLE			3


typedef list<shared_ptr<DumpScan>> DumpScanList;

using namespace std;

class DumpManager
{
public:
	DumpManager(unsigned long address, unsigned int maxOffset, unsigned int depth, DumpDataList &dumps, PointerOffsets &offsets, string filename);
	DumpManager(unsigned long address, DumpDataList &dumps, string inputFname, string scanFname);
	~DumpManager();

	int process();
	void run();
	void start();
	void join() { if (m_thread.joinable()) m_thread.join();}
	void cancel();


	ScanResultList &getResults() { return m_totalResults; }
	mutex &getLock() { return m_totalResultLock; }

	void pushScanDump(shared_ptr<DumpScan> d) { 
//		lock_guard<mutex> lock(m_addDumpScanLock); 
//		if (!m_cancel)
			m_scannerList.push_front(d); 
	}
	char getStatus() { return m_status; }

private:
	void addResults(ScanResultList &ls);
	void readInFile();
	unsigned long refreshAddress(unsigned long address, PointerOffsets offsets);

	thread m_thread;
	mutex m_totalResultLock;
	mutex m_addDumpScanLock;

	vector<unsigned int> maxResList;

	char m_status;
	bool m_cancel;

	unsigned long m_address;
	DumpDataList m_dumps;
	PointerOffsets m_offsets;
	unsigned int m_maxOffset;
	unsigned int m_depth;

	DumpScanList m_scannerList;
	ScanResultList m_totalResults;

	DumpScanList::iterator m_currentScanner;

	bool m_continue;

	ifstream m_inFile;
	ofstream m_outFile;
};

#endif