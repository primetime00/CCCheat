#ifndef _PSWINDOW_
#define _PSWINDOW_

#include <string>
#include <memory>
#include "DumpManager.h"
#include "rkWindow.h"
#include "Common.h"
#include "CodeTable.h"
#include "CCAPI/Common.h"
#include "FL/Fl.H"

using namespace std;

#define PS_STATE_NEWLOAD	0
#define PS_STATE_NEW		1
#define PS_STATE_LOAD		2
#define PS_STATE_DUMPING	3
#define PS_STATE_SCANNING	4
#define PS_STATE_COMPLETE	5
#define PS_STATE_CANCEL		6

class PointerScannerWindow : public rkWindow
{
public:
	PointerScannerWindow(int X, int Y, int W, int H, const char *l=0) : rkWindow(X,Y,W,H,l) 
	{
		reset();
	}
	PointerScannerWindow(int W, int H, const char *l=0) : rkWindow(W,H,l) 
	{
		reset();
	}

	void reset();
	void NewScanCB(Fl_Widget *w);
	void LoadScanCB(Fl_Widget *w);
	void AddressChangeCB(Fl_Widget *w);
	void StartNewScanCB(Fl_Widget *w);
	void AddPointerToListCB(Fl_Widget *w);

	void progressTimeout();
	static void progressTimeout(void *instance) { ((PointerScannerWindow*)instance)->progressTimeout(); }


	static void StartScanning(void *w) 
	{ 
		Fl::remove_timeout(PointerScannerWindow::StartScanning, w);
		((PointerScannerWindow*)w)->startScanning(); 
	}
	void setDumpList(DumpDataList dl) { dumpList = dl; }

	void setDumpProgress(string s, float percent);

	void show();
	void hide();
	void setCodeTable(CodeTable *t) { m_codeTable = t; }

private:
	void setScanState(char v);
	void setNumberOfResults(unsigned long res);
	void startScanning();
	int doNewLoadDialog(bool load);

	string m_scanFile;
	string m_inputFile;
	char m_psState;
	DumpDataList dumpList;
	bool m_isLoading;
	shared_ptr<DumpManager> dumpManager;

	CodeTable *m_codeTable;

};


#endif
