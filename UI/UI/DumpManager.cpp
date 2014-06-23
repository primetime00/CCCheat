#include "DumpManager.h"
#include "Common.h"
#include "FL/Fl.H"
#include <string>

DumpManager::DumpManager(unsigned long address, unsigned int maxOffset, unsigned int depth, DumpDataList &dumps, PointerOffsets &offsets, string filename) : m_outFile(filename, ofstream::out)
{
	m_continue = false;
	m_address = address;
	m_dumps = dumps;
	m_offsets = offsets;
	m_maxOffset = maxOffset;
	m_depth = depth;
	m_cancel = false;
	m_status = DATAMANAGER_STATUS_IDLE;
	unsigned int tmp[] = { 6, 3, 2, 1, 1 };
	maxResList = vector<unsigned int>(tmp, tmp+5);
}
DumpManager::DumpManager(unsigned long address, DumpDataList &dumps, string inputFname, string scanFname) : m_inFile(inputFname, ifstream::in), m_outFile(scanFname, ofstream::out)
{
	m_continue = true;
	m_address = address;
	m_dumps = dumps;
	m_cancel = false;
	m_status = DATAMANAGER_STATUS_IDLE;
}

DumpManager::~DumpManager()
{
	m_dumps.clear();
	m_offsets.clear();
}


void DumpManager::start()
{
	m_status = DATAMANAGER_STATUS_WORKING;
	if (m_outFile) //we didn't create the file
		m_outFile << CCCHEAT_VERSION << '\n';
	m_thread = thread(&DumpManager::run, this);
}

void DumpManager::readInFile()
{
	string version;
	ScanResultList ls;
	m_inFile.seekg(0);
	m_inFile >> version;
	while (1)
	{
		unsigned long address;
		unsigned int offset;
		unsigned int depth;
		PointerOffsets po;
		if (m_cancel) return;
		m_inFile >> hex >> address >> depth;
		if (m_inFile.eof())
			break;
		for (unsigned int i=0; i<depth; ++i)
		{
			m_inFile >> hex >> offset;
			po.push_back(offset);
		}
		unsigned long val = refreshAddress(address, po); //lets check the current value of the pointer
		if (val != 0 && val == m_address) //this pointer still points to where we expect, lets add it to the list
		{
			ls.push_back(ScanResult(address, po));
			if (ls.size() == 4)
			{
				addResults(ls);
				ls.clear();
			}
		}
	}
	if (ls.size() > 0)
	{
		addResults(ls);
		ls.clear();
	}
}

unsigned long DumpManager::refreshAddress(unsigned long address, PointerOffsets offsets)
{
	unsigned int currentOffset = offsets.back();
	offsets.pop_back();
	for (DumpDataList::const_iterator it = m_dumps.begin(); it != m_dumps.end(); ++it)
	{
		if (address >= (*it)->header.begin && address <= (*it)->header.end)
		{
			if (m_cancel) return 0;
			char *data = (*it)->data;
			unsigned long newAddress = BSWAP32(*(unsigned long*) (&data[address - (*it)->header.begin]));
			newAddress += currentOffset;
			if (offsets.size() == 0)
				return newAddress;
			return refreshAddress(newAddress, offsets);
		}
	}
	return 0;
}

void DumpManager::run()
{
	int res = 1;
	if (m_continue) //read in file info
	{
		readInFile();
	}
	else
	{
		while (res && !m_cancel)
		{
			res = process();
		}
	}
	if (m_outFile)
		m_outFile.close();
	if (m_cancel)
		m_status = DATAMANAGER_STATUS_CANCELED;
	else
		m_status = DATAMANAGER_STATUS_COMPLETE;
}

void DumpManager::cancel()
{
	lock_guard<mutex> lock(m_addDumpScanLock);
	m_cancel = true;
	for (auto it = m_scannerList.begin(); it!= m_scannerList.end(); ++it)
	{
		(*it)->cancel();
	}
}

void DumpManager::addResults(ScanResultList &ls)
{
	lock_guard<mutex> lock(m_totalResultLock);
	//m_totalResults.insert(m_totalResults.end(), ls.begin(), ls.end());
	for (ScanResultList::const_iterator it = ls.begin(); it != ls.end(); ++it)
	{
		m_totalResults.push_back(*it);
		if (m_outFile)
		{
			m_outFile << hex << it->address << " " << it->offsets.size() << " ";
			for (PointerOffsets::const_iterator pt = it->offsets.begin(); pt != it->offsets.end(); ++pt)
				m_outFile << hex << *pt << " ";
			m_outFile << '\n';
		}
	}
}

int DumpManager::process()
{
	int res;
	shared_ptr<DumpScan> s;
	ScanResultList sr;
	unsigned int depth=0;
	lock_guard<mutex> lock(m_addDumpScanLock);
	if (m_cancel)
	{
		m_scannerList.clear();
		return 0;
	}
	if (m_scannerList.size() == 0)
	{
		s = make_shared<DumpScan>(m_address, m_maxOffset, m_dumps, m_offsets, maxResList[depth]);
		res = s->startProcess();
		switch (res)
		{
		case DUMP_SCAN_STATUS_CONTINUE: //we must continue this one
			sr = s->getResults();
			addResults(sr);
			pushScanDump(s); //add ourself
			if (s->getDepth() < m_depth)
			{
				depth = s->getDepth();
				for (ScanResultList::iterator it = sr.begin(); it != sr.end(); ++it)
				{
					s = make_shared<DumpScan>( it->address, m_maxOffset, m_dumps, it->offsets, maxResList[depth] );
					pushScanDump(s);
				}
			}
			m_currentScanner = m_scannerList.begin();
			break;
		case DUMP_SCAN_STATUS_DONE: //this one is done!
			sr = s->getResults();
			addResults(sr);
			if (s->getDepth() < m_depth)
			{
				depth = s->getDepth();
				for (ScanResultList::iterator it = sr.begin(); it != sr.end(); ++it)
				{
					s = make_shared<DumpScan>( it->address, m_maxOffset, m_dumps, it->offsets, maxResList[depth] );
					pushScanDump(s);
				}
			}
			m_currentScanner = m_scannerList.begin();
			break;
		case DUMP_SCAN_STATUS_CANCEL: //this one is canceled
			//m_scannerList.clear();
			return 0;
			break;
		}
	}
	else //we have some scanner objects!
	{
		s = *m_currentScanner;
		DumpScanList::iterator tmp = m_currentScanner;
		bool isLastDepth = false;
		//if (++tmp != m_scannerList.end())
		if (++tmp == m_scannerList.end())
		{
			//if (s->getDepth() != (*tmp)->getDepth()) //we are at the last scanner of this depth
				isLastDepth = true;
		}

		if (s->getResults().size() == 0)
			res = s->startProcess();
		else
			res = s->continueProcess();
		switch (res)
		{
		case DUMP_SCAN_STATUS_CONTINUE: //we must continue this one
			sr = s->getResults();
			addResults(sr);
			if (s->getDepth() < m_depth)
			{
				depth = s->getDepth();
				for (ScanResultList::iterator it = sr.begin(); it != sr.end(); ++it)
				{
					s = make_shared<DumpScan>( it->address, m_maxOffset, m_dumps, it->offsets, maxResList[depth] );
					pushScanDump(s);
				}
				if (isLastDepth)
					m_currentScanner = m_scannerList.begin();
				else
				{
					m_currentScanner++;
					if (m_currentScanner == m_scannerList.end()) 
					{
						m_currentScanner = m_scannerList.begin();
					}
				}
			}
			else
			{
				m_currentScanner++;
				if (m_currentScanner == m_scannerList.end()) 
				{
					m_currentScanner = m_scannerList.begin();
				}
			}
			break;
		case DUMP_SCAN_STATUS_DONE: //this one is done!
			sr = s->getResults();
			addResults(sr);

			m_currentScanner = m_scannerList.erase(m_currentScanner);

#if 0
			//make sure we aren't cancelling our scan
			m_addDumpScanLock.lock();
			if (!m_cancel)
				m_currentScanner = m_scannerList.erase(m_currentScanner);
			m_addDumpScanLock.unlock();
#endif
			if (s->getDepth() < m_depth)
			{
				depth = s->getDepth();
				for (ScanResultList::iterator it = sr.begin(); it != sr.end(); ++it)
				{
					s = make_shared<DumpScan>( it->address, m_maxOffset, m_dumps, it->offsets, maxResList[depth] );
					pushScanDump(s);
				}
				if (isLastDepth)
					m_currentScanner = m_scannerList.begin();
				else
				{
					m_currentScanner++;
					if (m_currentScanner == m_scannerList.end()) 
					{
						m_currentScanner = m_scannerList.begin();
					}
				}
			}
			else
			{
				if (m_currentScanner == m_scannerList.end())
					m_currentScanner = m_scannerList.begin();
			}
			break;
		case DUMP_SCAN_STATUS_CANCEL: //this one is canceled
			m_scannerList.clear();
			return 0;
			break;
		}
	}
	return m_scannerList.size();
}