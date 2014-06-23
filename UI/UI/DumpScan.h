#ifndef __DUMPSCAN__
#define __DUMPSCAN__

#include "CCAPI/Common.h"

#define DUMP_SCAN_STATUS_DONE		0
#define DUMP_SCAN_STATUS_CONTINUE	1
#define DUMP_SCAN_STATUS_CANCEL		2

using namespace std;

#define MAX_SCAN 4

struct ScanResult
{
	unsigned long address;
	PointerOffsets offsets;
	ScanResult(unsigned long a, PointerOffsets po) { address = a; offsets = po; }
	ScanResult &operator=(ScanResult t) { address = t.address; offsets = t.offsets; return *this;}
};
typedef list<ScanResult> ScanResultList;

class DumpScan
{
public:
	DumpScan(unsigned long address, unsigned int maxOffset, DumpDataList &dumps, PointerOffsets &offsets, unsigned int maxReults);
	~DumpScan();

	int process(DumpDataList::iterator startDump, unsigned long startByte);
	int startProcess();
	int continueProcess();
	int getDepth() { return m_offsets.size()+1; }
	ScanResultList getResults() { return m_results; }
	void cancel() { m_cancel = true; }

private:
	unsigned long m_address;
	DumpDataList m_dumps;
	PointerOffsets m_offsets;
	unsigned int m_maxOffset;
	DumpDataList::iterator m_currentDump;
	unsigned long m_currentByte;

	unsigned int m_counter;

	ScanResultList m_results;
	unsigned int m_maxResults;
	bool m_cancel;
};

#endif