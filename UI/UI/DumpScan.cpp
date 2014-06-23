#include "DumpScan.h"

#define CHECK_CANCEL if (m_cancel) return DUMP_SCAN_STATUS_CANCEL
DumpScan::DumpScan(unsigned long address, unsigned int maxOffset, DumpDataList &dumps, PointerOffsets &offsets, unsigned int maxResults)
{
	m_address = address;
	m_dumps = dumps;
	m_offsets = offsets;
	m_maxOffset = maxOffset;
	m_maxResults = maxResults;
	m_cancel = false;
}

DumpScan::~DumpScan()
{
	m_results.clear();
}

int DumpScan::startProcess()
{
	return process(m_dumps.begin(), 0);
}
int DumpScan::continueProcess()
{
	m_results.clear();
	return process(m_currentDump, m_currentByte);
}
int DumpScan::process(DumpDataList::iterator startDump, unsigned long startByte)
{
	unsigned long min = m_address - m_maxOffset;
	unsigned long max = m_address;
	if (m_maxOffset > m_address)
		min = 0;
	m_counter = 0;
	for (DumpDataList::iterator it = startDump; it!= m_dumps.end(); ++it)
	{
		char *data = (*it)->data;
		CHECK_CANCEL;
		for (unsigned long i=startByte; i<((*it)->header.end - (*it)->header.begin)-3; ++i)
		{
			unsigned long currentAddress = BSWAP32(*(unsigned long*)(&data[i]));
			CHECK_CANCEL;
			if ( currentAddress >= min && currentAddress <= max) //we want this
			{
				PointerOffsets po = m_offsets;
				po.push_back(m_address - currentAddress);
				m_results.push_back(ScanResult(i+(*it)->header.begin, po));
				if (++m_counter >= m_maxResults)
				{
					m_counter = 0;
					m_currentDump = it;
					m_currentByte = i+1;
					return DUMP_SCAN_STATUS_CONTINUE;
				}
			}
		}
	}
	return DUMP_SCAN_STATUS_DONE;
}