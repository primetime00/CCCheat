#ifndef _RKWINDOW_
#define _RKWINDOW_

#include <vector>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl.H>
#include "IDeactivate.h"

using namespace std;

class rkWindow : public Fl_Double_Window
{
public:
	rkWindow(int X, int Y, int W, int H, const char *l=0);
	rkWindow(int W, int H, const char *l=0);

	static void addObject(IDeactivate *d) { objs.push_back(d); }
	static vector<IDeactivate *> objs;


	int handle(int evt);

	void manual_resize(int w, int h);
	void show();
	void hide();

	virtual void capture();

protected:
	unsigned int createdW, createdH;
	int createdX, createdY;
	bool first;
};


#endif
