#ifndef _RKWINDOW_
#define _RKWINDOW_

#include <vector>
#include <FL/Fl_Double_Window.H>
#include "IDeactivate.h"

using namespace std;

class rkWindow : public Fl_Double_Window
{
public:
	rkWindow(int X, int Y, int W, int H, const char *l=0) : Fl_Double_Window(X,Y,W,H,l), createdW(W), createdH(H) {}
	rkWindow(int W, int H, const char *l=0) : Fl_Double_Window(W,H,l), createdW(W), createdH(H) {}

	static void addObject(IDeactivate *d) { objs.push_back(d); }
	static vector<IDeactivate *> objs;


	int handle(int evt);

	void show();
	void hide();

private:
	unsigned int createdW, createdH;
};


#endif
