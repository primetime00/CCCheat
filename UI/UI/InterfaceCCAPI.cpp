#include "InterfaceCCAPI.h"
#include "CCAPI/Common.h"
#include "FL/Fl.H"
#include <sstream>

InterfaceCCAPI *InterfaceCCAPI::instance = 0;


void InterfaceCCAPI::connect(string ip)
{
	m_ip = ip;
	memoryOperator = new MemoryOperator(ip);
	int ver = get_user_data(int, uiInstance->ui_ccapiChoice->mvalue()->user_data());
	memoryOperator->setHostCCAPIVersion(ver);
	memoryOperator->start();
	Fl::add_timeout(0.2, InterfaceCCAPI::connectCheck);
}

void InterfaceCCAPI::_connectCheck()
{
	if (memoryOperator != 0)
	{
		string stat = memoryOperator->getStatus();
		if (stat == "CONNECT") //we are connecting still
		{
			Fl::repeat_timeout(0.2, InterfaceCCAPI::connectCheck);
		}
		else if (stat == "ERROR") //we failed!
		{
			Fl::remove_timeout(InterfaceCCAPI::connectCheck);
			uiInstance->setConnectStatus(INTERFACE_CONNECT_FAIL);
			disconnect();
		}
		else
		{
			Fl::remove_timeout(InterfaceCCAPI::connectCheck);
			uiInstance->setConnectStatus(INTERFACE_CONNECT_SUCCESS);
			uiInstance->ui_codeTable->setMemoryOperator(memoryOperator);
			uiInstance->m_peWindow->setMemoryOperator(memoryOperator);
			uiInstance->m_pointerScannerWindow->setCodeTable(uiInstance->ui_codeTable);
			uiInstance->ui_resultTable->setMemoryOperator(memoryOperator);
			uiInstance->m_valueTable->setMemoryOperator(memoryOperator);
			uiInstance->m_valueTable->setCodeTable(uiInstance->ui_codeTable);
		}
	}
}

void InterfaceCCAPI::disconnect()
{
	uiInstance->ui_codeTable->setMemoryOperator(0);
	uiInstance->ui_resultTable->setMemoryOperator(0);
	uiInstance->m_valueTable->setMemoryOperator(0);
	uiInstance->m_peWindow->setMemoryOperator(0);
	uiInstance->m_valueTable->setCodeTable(0);
	uiInstance->m_pointerScannerWindow->setCodeTable(0);
	if (memoryOperator != 0)
	{
		delete memoryOperator;
	}
	memoryOperator = 0;
	if (ranger != 0)
	{
		ranger->cancel();
	}
	for (vector<SearchMemory*>::iterator it = searchList.begin(); it != searchList.end(); ++it)
	{
		(*it)->cancel();
	}
}

void InterfaceCCAPI::findAddressProgress(void *)
{
	if (InterfaceCCAPI::instance)
	{
		if (!InterfaceCCAPI::instance->_findAddressProgress())
			Fl::repeat_timeout(1.0, InterfaceCCAPI::findAddressProgress);
		else
			Fl::remove_timeout(InterfaceCCAPI::findAddressProgress);
	}
}

bool InterfaceCCAPI::_findAddressProgress()
{
	string status = ranger->getStatus();
	float progress = ranger->getProgress();
	m_ui->setRangeProgress(progress, "Finding Range...", false);
	if (status == "DONE")
	{
		if (m_launcher.joinable())
			m_launcher.join();
		_processAddresses();
		m_ui->setRangeProgress(1.0f, "Finished", true);
		delete ranger;
		ranger = 0;
		m_ui->findRangeStopped();
		return true;
	}
	else if (status == "CANCEL")
	{
		if (m_launcher.joinable())
			m_launcher.join();
		m_ui->setRangeProgress(0.0f, "Canceled", true);
		delete ranger;
		ranger = 0;
		m_ui->findRangeStopped();
		return true;
	}
	return false;
}
void InterfaceCCAPI::findAddresses()
{
	if (InterfaceCCAPI::instance)
		InterfaceCCAPI::instance->_findAddresses();
}

void InterfaceCCAPI::cancelFindAddresses()
{
	if (InterfaceCCAPI::instance)
		InterfaceCCAPI::instance->_cancelFindAddresses();
}

void InterfaceCCAPI::_findAddresses()
{
	string c_ip = m_ui->ui_ipInput->getIP();
	m_ui->findRangeStarted();
	m_ui->setRangeProgress(0.0f, "Initializing...", false);
	int ver = get_user_data(int, uiInstance->ui_ccapiChoice->mvalue()->user_data());
	ranger = new RangeMemory(c_ip, ver, 0, 0x100000000);
	m_launcher = thread(&InterfaceCCAPI::_launchFindAddress, this);
	Fl::add_timeout(1.0, InterfaceCCAPI::findAddressProgress, ranger);
	cout << "FINDING ADDRESSES!" << endl;
}

void InterfaceCCAPI::_cancelFindAddresses()
{
	if (ranger)
	{
		ranger->cancel();
	}
}


void InterfaceCCAPI::_launchFindAddress()
{
	ranger->process();
}

void InterfaceCCAPI::_processAddresses()
{
	rkCheat_RangeList rList;
	RangeList ranges = ranger->getRanges();
	m_ui->clearRanges();
	for ( RangeList::iterator it = ranges.begin(); it != ranges.end(); ++it)
	{
		m_ui->addRange(it->first(), it->second());
	}
}


void InterfaceCCAPI::startNewSearch()
{
	if (InterfaceCCAPI::instance)
	{
		if (InterfaceCCAPI::instance->_searchInProgress())
		{
			if (InterfaceCCAPI::instance->_isSearching())
				InterfaceCCAPI::instance->_cancelSearch();
			else
				InterfaceCCAPI::instance->_resetSearch();
		}
		else
			InterfaceCCAPI::instance->_startNewSearch();
	}
}
void InterfaceCCAPI::_startNewSearch()
{
	char searchType = get_user_data(char, m_ui->ui_searchType->mvalue()->user_data());
	char valueType = get_user_data(char, m_ui->ui_valueType->mvalue()->user_data());
	string c_ip = m_ui->ui_ipInput->getIP();
	m_ui->ui_resultTable->clear();
	rkCheat_RangeList rList = m_ui->ui_rangeTable->getSelectedRanges();
	lastSearchType = SEARCH_TYPE_UNKNOWN;
	int ver = get_user_data(int, uiInstance->ui_ccapiChoice->mvalue()->user_data());
	
	//add the ranges to the search list
	for (rkCheat_RangeList::iterator it = rList.begin(); it != rList.end(); ++it)
	{
		SearchMemory *s = new SearchMemory(c_ip, ver, it->first, it->second - it->first);
		searchList.push_back(s);

		if (searchType == SEARCH_FUZZY_INIT)
		{
			s->setSearchType(SEARCH_TYPE_FUZZY);
			s->setSearchCompareType(SEARCH_FUZZY_EQUAL);
		}
		else
		{
			s->setSearchType(SEARCH_TYPE_VALUE);
			s->setSearchCompareType(get_user_data(char, m_ui->ui_searchType->mvalue()->user_data()));
			s->setSearchValue(m_ui->ui_valueInput->getLLValue());
		}
		s->setSearchValueType(valueType);
	}
	
	//set the UI to seach mode
	m_ui->searchStarted();
	m_ui->setSearchProgress(0.0f, "Initializing...", false);

	//launch the searching thread!
	SearchMemory *s = getCurrentSearch();
	Fl::add_timeout(1.0, InterfaceCCAPI::searchProgress, s);
	m_launcher = thread(&InterfaceCCAPI::_launchSearch, this);

}

void InterfaceCCAPI::cancelDumpMemory()
{
	InterfaceCCAPI::instance->_cancelDumpMemory();
}
void InterfaceCCAPI::_cancelDumpMemory()
{
	for (auto it = dumpList.begin(); it != dumpList.end(); ++it)
	{
		(*it)->cancel();
	}
}

void InterfaceCCAPI::dumpMemory()
{
	InterfaceCCAPI::instance->_dumpMemory();
}
void InterfaceCCAPI::_dumpMemory()
{
	string c_ip = m_ui->ui_ipInput->getIP();
	rkCheat_RangeList rList = m_ui->ui_rangeTable->getSelectedRanges();
	int ver = get_user_data(int, uiInstance->ui_ccapiChoice->mvalue()->user_data());
	
	//add the ranges to the search list
	dumpList.clear();
	for (rkCheat_RangeList::iterator it = rList.begin(); it != rList.end(); ++it)
	{
		DumpMemoryItem s = make_shared<DumpMemory>(c_ip, ver, it->first, it->second - it->first);
		dumpList.push_back(s);
	}
	
	//launch the searching thread!
	DumpMemoryItem s = getCurrentDump();
	m_ui->m_pointerScannerWindow->setDumpProgress("", 0.0f);
	Fl::add_timeout(1.0, InterfaceCCAPI::dumpProgress, s.get());
	m_launcher = thread(&InterfaceCCAPI::_launchDump, this);
}


void InterfaceCCAPI::continueSearch()
{
	if (InterfaceCCAPI::instance)
	{
		InterfaceCCAPI::instance->_continueSearch();
	}
}

void InterfaceCCAPI::_continueSearch()
{
	//set the UI to seach mode
	int numResults = searchResults[currentSearchIndex]->size();
	m_ui->searchStarted();
	m_ui->setSearchProgress(0.0f, "Initializing...", false);
	m_ui->ui_resultTable->clear();

	//searchResults->clear();

	for (unsigned int i=0; i<searchList.size(); i++)
	{
		SearchMemory *s = searchList.at(i);
		if (s->isDump() && !m_ui->ui_searchType->isFuzzy() && lastSearchType == SEARCH_TYPE_FUZZY)
		{
			_cycleSearch();
			return;
		}
		s->setSearchValue(m_ui->ui_valueInput->getLLValue());
		s->setSearchType(m_ui->ui_searchType->isFuzzy() ? SEARCH_TYPE_FUZZY: SEARCH_TYPE_VALUE);
		s->setSearchCompareType(get_user_data(char, m_ui->ui_searchType->mvalue()->user_data()));
		if (numResults > 0 || s->isDump())
			s->resetStatus();
	}
	//launch the searching thread!
	SearchMemory *s = getCurrentSearch();
	if (s != 0)
	{
		Fl::add_timeout(1.0, InterfaceCCAPI::searchProgress, s);
		m_launcher = thread(&InterfaceCCAPI::_launchSearch, this);
	}
	cout << "Continuing search 4" << endl;

}


void InterfaceCCAPI::_resetSearch()
{
	Fl::remove_timeout(InterfaceCCAPI::searchProgress, searcher);
	if (m_launcher.joinable())
	{
		m_launcher.join();
	}
	for (unsigned int i=0; i<searchList.size(); i++)
	{
		delete searchList.at(i);
	}
	lastSearchType = SEARCH_TYPE_UNKNOWN;
	searchList.clear();
	searcher = 0;
	m_ui->searchStopped(SEARCH_STOPPED_RESET);
	m_ui->setRangeProgress(0.0f, "Reset", true);
	m_ui->ui_resultTable->clear();
	m_ui->ui_searchType->reset();
	//searchResults.clear();
	rkCheatUI::SearchTypeChangeCB(0, 0);
}

void InterfaceCCAPI::_cycleSearch()
{
	for (unsigned int i=0; i<searchList.size(); i++)
	{
		SearchMemory *s = searchList.at(i);
		delete s;
	}
	searchList.clear();
	_startNewSearch();
}

SearchMemory *InterfaceCCAPI::getCurrentSearch()
{
	SearchMemory *s = 0;
	currentSearchIndex = 0;
	for (vector<SearchMemory*>::iterator it = searchList.begin(); it != searchList.end(); ++it)
	{
		SearchMemory *m = *it;
		if (m->getStatus() == "DONE")
		{
			currentSearchIndex++;
			continue;
		}
		s = m;
		break;
	}
	if (s == 0)
		currentSearchIndex = 0;
	return s;
}

InterfaceCCAPI::DumpMemoryItem InterfaceCCAPI::getCurrentDump()
{
	DumpMemoryItem s;
	for (vector<DumpMemoryItem>::iterator it = dumpList.begin(); it != dumpList.end(); ++it)
	{
		DumpMemoryItem m = *it;
		if (m->getStatus() == "DONE")
		{
			//currentSearchIndex++;
			continue;
		}
		s = m;
		break;
	}
	return s;
}

void InterfaceCCAPI::_launchSearch()
{
	searcher = getCurrentSearch();
	if (searcher != 0)
		searcher->process();
}

void InterfaceCCAPI::_launchDump()
{
	DumpMemoryItem d = getCurrentDump();
	if (d != 0)
		d->process();
}


void InterfaceCCAPI::searchProgress(void * search)
{
	if (InterfaceCCAPI::instance)
	{
		InterfaceCCAPI::instance->_searchProgress((SearchMemory*)search);
	}
}

bool InterfaceCCAPI::_searchProgress(SearchMemory *search)
{
	unsigned long numResults;
	stringstream searchString;
	if (search == 0)
		false;
	string status = search->getStatus();
	float progress = search->getProgress();
	if (searchList.size() > 1)
		searchString << "Searching [" << currentSearchIndex+1 <<"/"<<searchList.size() << "]...";
	else
		searchString << "Searching...";
	m_ui->setSearchProgress(progress, searchString.str().c_str(), false);
	if (status == "DONE")
	{
		if (m_launcher.joinable())
			m_launcher.join();
		Fl::remove_timeout(InterfaceCCAPI::searchProgress, search);
		_processSearchResults(search);
		numResults = getNumberOfResults();
		m_ui->setNumberOfResultsString(to_string(numResults));
		if (!isSearchFinished()) //start the next range
		{
			searcher = getCurrentSearch();
			Fl::add_timeout(1.0, InterfaceCCAPI::searchProgress, searcher);
			m_launcher = thread(&InterfaceCCAPI::_launchSearch, this);
			return false;
		}
		lastSearchType = searcher->getSearchType();
		m_ui->setSearchProgress(1.0f, "Finished", true);
		if (numResults == 0 && m_ui->ui_searchType->getValue() != SEARCH_FUZZY_INIT) //we have no results
		{
			m_ui->searchStopped(SEARCH_STOPPED_NO_RESULTS);
		}
		else
		{
			m_ui->searchStopped(SEARCH_STOPPED_PROPER);
			m_ui->ui_searchType->firstFuzzy();
			m_ui->ui_resultTable->setResults(searchResults);
		}
		return true;
	}
	else if (status == "CANCEL")
	{
		_resetSearch();
		m_ui->setSearchProgress(0.0f, "Canceled", true);
		return true;
	}
	Fl::repeat_timeout(1.0, InterfaceCCAPI::searchProgress, search);
	return false;
}

InterfaceCCAPI::DumpMemoryItem InterfaceCCAPI::getDumpMemoryItem(DumpMemory *m)
{
	for (auto it = dumpList.begin(); it != dumpList.end(); ++it)
	{
		if (it->get() == m)
			return *it;
	}
	return nullptr;
}

void InterfaceCCAPI::dumpProgress(void *dump)
{
	if (InterfaceCCAPI::instance)
	{
		InterfaceCCAPI::instance->_dumpProgress((DumpMemory *)dump);
	}
}

bool InterfaceCCAPI::_dumpProgress(DumpMemory* dump)
{
	unsigned long numResults;
	unsigned int i;
	stringstream dumpString;
	DumpMemoryItem s = getDumpMemoryItem(dump);
	if (s == nullptr)
		false;
	string status = s->getStatus();
	float progress = s->getProgress();
	for (i=0; i<dumpList.size(); ++i)
		if (dumpList[i] == s)
			break;

	if (dumpList.size() > 1)
		dumpString << "Dumping Memory [" << i+1 <<"/"<<dumpList.size() << "]...";
	else
		dumpString << "Dumping Memory...";
	m_ui->m_pointerScannerWindow->setDumpProgress(dumpString.str(), progress);
	if (status == "DONE")
	{
		m_ui->m_pointerScannerWindow->setDumpProgress(dumpString.str(), 1.0f);
		if (m_launcher.joinable())
			m_launcher.join();
		Fl::remove_timeout(InterfaceCCAPI::dumpProgress, s.get());
		s = getCurrentDump();
		if (s != nullptr) //start the next dump
		{
			Fl::add_timeout(1.0, InterfaceCCAPI::dumpProgress, s.get());
			m_launcher = thread(&InterfaceCCAPI::_launchDump, this);
			return false;
		}
		//all dumps are done!
		DumpDataList dList;
		for (i=0; i<dumpList.size(); ++i)
		{
			dList.push_back(dumpList[i]->consolideDump());
			//delete dumpList[i];
		}
		dumpList.clear();
		m_ui->m_pointerScannerWindow->setDumpList(dList);
		Fl::add_timeout(0.5, PointerScannerWindow::StartScanning, m_ui->m_pointerScannerWindow);
		if (m_launcher.joinable()) m_launcher.join();
		return true;
	}
	else if (status == "CANCEL")
	{
		Fl::remove_timeout(InterfaceCCAPI::dumpProgress, s.get());
		m_ui->m_pointerScannerWindow->setDumpProgress(dumpString.str(), 0.0f);
		dumpList.clear();
		if (m_launcher.joinable()) m_launcher.join();
		return true;
	}
	Fl::repeat_timeout(1.0, InterfaceCCAPI::dumpProgress, s.get());
	return false;
}


unsigned long InterfaceCCAPI::getNumberOfResults()
{
	unsigned long num = 0;
	for (vector<SearchMemory*>::iterator it = searchList.begin(); it != searchList.end(); ++it)
	{
		if ((*it)->getStatus() == "DONE")
			num+= (*it)->getNumberOfResults();
	}
	return num;
}
void InterfaceCCAPI::_processSearchResults(SearchMemory *search)
{
	searchResults[currentSearchIndex] = search->getResults();
	/*
	AddressList rList = search->getResults();
	searchResults.insert(searchResults.end(), rList.begin(), rList.end());*/
}

void InterfaceCCAPI::_cancelSearch() 
{
	if (searcher)
	{
		searcher->cancel();
	}
}

bool InterfaceCCAPI::isSearchFinished()
{
	bool done = true;
	for (vector<SearchMemory*>::iterator it = searchList.begin(); it != searchList.end(); ++it)
	{
		if ((*it)->getStatus() == "DONE")
			continue;
		done = false;
	}
	return done;
}
