#include "PointerScannerWindow.h"
#include "PointerScannerTable.h"
#include "DumpManager.h"
#include "ui_main.h"

using namespace std;

int PointerScannerWindow::doNewLoadDialog(bool load)
{
	string fname = "";
	Fl_Native_File_Chooser fnfc;
	if (load)
	{
		fnfc.title("Open Pointer Scan");
		fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
	}
	else
	{
		fnfc.title("Save Pointer Scan");
		fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	}
	fnfc.filter("Point Scan Files\t*.pts");
	fnfc.directory("/var/tmp");
  	switch ( fnfc.show() ) {
  		case -1: printf("ERROR: %s\n", fnfc.errmsg());    break;  // ERROR
  		case  1: printf("CANCEL\n");                      break;  // CANCEL
  		default: 
  		fname = fnfc.filename();
  		if (fnfc.filter_value() == 0)
  		{
  			if (fname.find(".pts", fname.size()-4) == string::npos)
  				fname += ".pts";
  		}
  	}
	if (!fname.empty())
	{
		if (load)
		{
			m_inputFile = fname;
			ifstream iFile(fname);
			if (iFile)
				return 0;
			else
				return 2;
		}
		else
			m_scanFile = fname;
		return 0;
	}
	return 1;
}
void PointerScannerWindow::NewScanCB(Fl_Widget *w)
{
	if (doNewLoadDialog(false) == 0)
		setScanState(PS_STATE_NEW);
}

void PointerScannerWindow::LoadScanCB(Fl_Widget *w)
{
	int res = doNewLoadDialog(true);
	if (res == 2) //the file does not exist
	{
		fl_alert("That pointer scan file does not exist.");
		setScanState(PS_STATE_NEWLOAD);
	}
	else if (res == 1)
		return;
	res = doNewLoadDialog(false);
	while (m_scanFile == m_inputFile)
	{
		fl_alert("Please choose another file to write pointers.\nI don't want to erase previous scans.");
		res = doNewLoadDialog(false);
		if (res == 1)
			return;
	}
	if (res == 0)
		setScanState(PS_STATE_LOAD);
}

void PointerScannerWindow::AddressChangeCB(Fl_Widget *w)
{
	if (string(uiInstance->m_psNewAddress->value()).size() > 0)
		uiInstance->m_psNewStartButton->activate();
	else
		uiInstance->m_psNewStartButton->deactivate();
}

void PointerScannerWindow::StartNewScanCB(Fl_Widget *w)
{
	if (m_psState == PS_STATE_NEW || m_psState == PS_STATE_COMPLETE || m_psState == PS_STATE_CANCEL || m_psState == PS_STATE_LOAD)
	{
		setScanState(PS_STATE_DUMPING);
		InterfaceCCAPI::dumpMemory();
	}
	else if (m_psState == PS_STATE_DUMPING) //cancel
	{
		InterfaceCCAPI::cancelDumpMemory();
		setScanState(m_isLoading ? PS_STATE_LOAD : PS_STATE_NEW);
	}
	else if (m_psState == PS_STATE_SCANNING) //cancel
	{
		if (dumpManager == nullptr)
			setScanState(m_isLoading ? PS_STATE_LOAD : PS_STATE_NEW);
		else
		{
			dumpManager->cancel();
			dumpManager->join();
			setScanState(PS_STATE_CANCEL);
		}
	}
}

void PointerScannerWindow::AddPointerToListCB(Fl_Widget *w)
{
	PointerOffsets po;
	po.push_back(1);
	po.push_back(2);
	PointerItem b = uiInstance->m_peWindow->popup(0x5000, po);
	if (b == nullptr || m_codeTable == 0)
		return;

}

void PointerScannerWindow::setScanState(char v) 
{ 
	if (m_psState != v)
	{
		switch (v)
		{
		case PS_STATE_NEWLOAD:
			uiInstance->m_psNewGroup->hide();
			uiInstance->m_psLoadNewGroup->show();
			break;
		case PS_STATE_NEW:
			m_isLoading = false;
			uiInstance->m_psNewCancelButton->activate();
			uiInstance->m_psNewDumpProgress->hide();
			uiInstance->m_psScanStateLabelGroup->hide();
			uiInstance->m_psNewScanTable->reset();
			uiInstance->m_psNewStartButton->label("Start");
			uiInstance->m_psNewDumpProgress->minimum(0.0f);
			uiInstance->m_psNewDumpProgress->maximum(1.0f);
			uiInstance->m_psLoadNewGroup->hide();
			if (string(uiInstance->m_psNewAddress->value()).size() > 0)
				uiInstance->m_psNewStartButton->activate();
			else
				uiInstance->m_psNewStartButton->deactivate();
			uiInstance->m_psNewDepthSpinner->activate();
			uiInstance->m_psOffset->activate();
			uiInstance->m_psNewGroup->show();
			break;
		case PS_STATE_LOAD:
			m_isLoading = true;
			uiInstance->m_psNewCancelButton->activate();
			uiInstance->m_psNewDumpProgress->hide();
			uiInstance->m_psScanStateLabelGroup->hide();
			uiInstance->m_psNewScanTable->reset();
			uiInstance->m_psNewStartButton->label("Start");
			uiInstance->m_psNewDumpProgress->minimum(0.0f);
			uiInstance->m_psNewDumpProgress->maximum(1.0f);
			uiInstance->m_psLoadNewGroup->hide();
			if (string(uiInstance->m_psNewAddress->value()).size() > 0)
				uiInstance->m_psNewStartButton->activate();
			else
				uiInstance->m_psNewStartButton->deactivate();
			uiInstance->m_psNewDepthSpinner->deactivate();
			uiInstance->m_psOffset->deactivate();
			uiInstance->m_psNewGroup->show();
			break;
		case PS_STATE_DUMPING:
			uiInstance->m_psNewCancelButton->deactivate();
			uiInstance->m_psNewStartButton->label("Stop");
			uiInstance->m_psScanStateLabelGroup->hide();
			uiInstance->m_psNewDumpProgress->show();
			break;
		case PS_STATE_SCANNING:
			uiInstance->m_psNewCancelButton->deactivate();
			uiInstance->m_psScanningLabel->label("Scanning...");
			uiInstance->m_psNewScanTable->setResultAddress(uiInstance->m_psNewAddress->getULValue());
			uiInstance->m_psNewDumpProgress->hide();
			uiInstance->m_psScanningLabel->labelcolor(FL_BLUE);
			uiInstance->m_psScanningNumberLabel->label("0");
			uiInstance->m_psScanningResultsLabel->labelcolor(FL_BLUE);
			uiInstance->m_psScanStateLabelGroup->show();
			break;
		case PS_STATE_COMPLETE:
			uiInstance->m_psNewCancelButton->activate();
			uiInstance->m_psScanningLabel->label("Done!");
			uiInstance->m_psNewStartButton->label("Start");
			break;
		case PS_STATE_CANCEL:
			uiInstance->m_psNewCancelButton->activate();
			uiInstance->m_psScanningLabel->label("Canceled!");
			uiInstance->m_psNewStartButton->label("Start");
			break;
		}
	}
	m_psState = v;
}

void PointerScannerWindow::startScanning()
{
	setScanState(PS_STATE_SCANNING);
	if (dumpList.size() == 0)
		return;
	if (!m_isLoading)
	{
		PointerOffsets po;
		dumpManager = make_shared<DumpManager>(uiInstance->m_psNewAddress->getULValue(), (unsigned int)uiInstance->m_psOffset->getULValue(), (unsigned int)uiInstance->m_psNewDepthSpinner->value(),dumpList, po, m_scanFile);
		uiInstance->m_psNewScanTable->reset();
		dumpManager->start();
	}
	else
	{
		dumpManager = make_shared<DumpManager>(uiInstance->m_psNewAddress->getULValue(), dumpList, m_inputFile, m_scanFile);
		uiInstance->m_psNewScanTable->reset();
		dumpManager->start();
	}
	Fl::add_timeout(1.0, PointerScannerWindow::progressTimeout, this);
}

void PointerScannerWindow::setDumpProgress(string s, float percent)
{
	static char st[50];
	sprintf(st, "%s", s.c_str());
	uiInstance->m_psNewDumpProgress->label(st);
	uiInstance->m_psNewDumpProgress->value(percent);
}

void PointerScannerWindow::reset() 
{ 
		m_scanFile = "";
		m_psState = -1;
		dumpManager = nullptr;
		dumpList.clear();
		uiInstance->m_psNewAddress->setValueType(SEARCH_VALUE_TYPE_4BYTE);
		if (uiInstance->ui_isHex->value()) //it is hex already
		{
			uiInstance->m_psNewAddress->setHex(true);
			uiInstance->m_psNewAddress->value(uiInstance->ui_valueInput->value());
		}
		else //not hex
		{
			uiInstance->m_psNewAddress->value(uiInstance->ui_valueInput->value());
			uiInstance->m_psNewAddress->setHex(true);
		}
		uiInstance->m_psOffset->setHex(true);
		uiInstance->m_psOffset->setValueType(SEARCH_VALUE_TYPE_2BYTE);
		uiInstance->m_psNewDepthSpinner->type(FL_INT_INPUT);
		uiInstance->m_psNewDepthSpinner->maximum(5);
		uiInstance->m_psNewDepthSpinner->minimum(1);
		uiInstance->m_psNewDepthSpinner->value(1);
		uiInstance->m_psOffset->value("");
		uiInstance->m_psNewDumpProgress->hide();
		setScanState(PS_STATE_NEWLOAD);
		m_codeTable = 0;
}

void PointerScannerWindow::show()
{
	//if (!visible())
	//	uiInstance->m_valueTable->startMemoryRead();
	rkWindow::show();
}

void PointerScannerWindow::hide()
{
	//if (visible())
//		uiInstance->m_valueTable->stopMemoryRead();
	reset();
	rkWindow::hide();
}

void PointerScannerWindow::progressTimeout()
{
	lock_guard<mutex> lock(dumpManager->getLock());
	int index=0;
	//read the total results
	ScanResultList &sr = dumpManager->getResults();
	for (auto it = sr.begin(); it != sr.end(); ++it, ++index)
	{
		uiInstance->m_psNewScanTable->addRow(it->address, it->offsets);
		if (index == DEFAULT_PT_ROWS-1)
			break;
	}
	setNumberOfResults(sr.size());
	if (dumpManager->getStatus() == DATAMANAGER_STATUS_COMPLETE)
	{
		Fl::remove_timeout(PointerScannerWindow::progressTimeout, this);
		dumpManager->join();
		setScanState(PS_STATE_COMPLETE);
	}
	else if (dumpManager->getStatus() == DATAMANAGER_STATUS_CANCELED)
	{
		Fl::remove_timeout(PointerScannerWindow::progressTimeout, this);
		dumpManager->join();
		setScanState(PS_STATE_CANCEL);
	}
	else
		Fl::repeat_timeout(1.0, PointerScannerWindow::progressTimeout, this);
}

void PointerScannerWindow::setNumberOfResults(unsigned long res)
{
	static char st[22];
	sprintf(st, "%lu", res);
	uiInstance->m_psScanningNumberLabel->label(st);
	uiInstance->m_psScanningNumberLabel->damage(FL_DAMAGE_ALL);

}
