#ifndef _VVWINDOW_
#define _VVWINDOW_

#include "rkWindow.h"
#include "Common.h"

using namespace std;

class ValueViewerWindow : public rkWindow
{
public:
	ValueViewerWindow(int X, int Y, int W, int H, const char *l=0) : rkWindow(X,Y,W,H,l) {}
	ValueViewerWindow(int W, int H, const char *l=0) : rkWindow(W,H,l) {}

	void TypeChangedCB(Fl_Widget *w);
	void SignedChangedCB(Fl_Widget *w);
	void AddCodeCB(Fl_Widget *w);
	void setCodeData(rkCheat_Code *item);

	void show();
	void hide();

private:
	void reset();

};


#endif
