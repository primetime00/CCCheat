#ifndef _PEWINDOW_
#define _PEWINDOW_

#include <string>
#include <memory>
#include <vector>
#include "AddressOffsetGroup.h"
#include "rkWindow.h"
#include "Common.h"
#include "CCAPI/Common.h"
#include "CCAPI/MemoryOperator.h"
#include "FL/Fl.H"

using namespace std;

class PointerEditorWindow : public rkWindow
{
public:
	PointerEditorWindow(int X, int Y, int W, int H, const char *l=0) : rkWindow(X,Y,W,H,l) 
	{
		m_resAddress = 0;
		m_operator = 0;
		pointerUpdateCounter = 0;
	}
	PointerEditorWindow(int W, int H, const char *l=0) : rkWindow(W,H,l) 
	{
		m_resAddress = 0;
		m_operator = 0;
		pointerUpdateCounter = 0;
	}

	void reset();
	void OnOKCB(Fl_Widget *w);
	void OnCancelCB(Fl_Widget *w);
	void OnRefreshCB(Fl_Widget *w);
	void OnAddCB(Fl_Widget *w);
	void OnDeleteCB(Fl_Widget *w);
	void OnTypeChangedCB(Fl_Widget *w);

	static void refreshTimeout(void *t) { ((PointerEditorWindow*)t)->refreshTimeout(); }
	void refreshTimeout();


	void show();
	void hide();
	PointerItem popup(unsigned long address, PointerOffsets offsets);

	void setPointer(unsigned long address, PointerOffsets offsets);
	void setResolvedAddress(unsigned long address) { m_resAddress = address; }
	void setMemoryOperator(MemoryOperator *op) { m_operator = op; }

private:
	void readPointers();
	void createGroup();

	unsigned long m_resAddress;
	MemoryOperator *m_operator;
	PointerItem currentPointer;
	unsigned int pointerUpdateCounter;

	vector<AddressOffsetGroup *> groups;

};


#endif
