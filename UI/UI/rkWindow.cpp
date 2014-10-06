#include "rkWindow.h"
#include <iostream>
#include <FL/Fl.H>

using namespace std;

vector<IDeactivate *> rkWindow::objs;

rkWindow::rkWindow(int X, int Y, int W, int H, const char *l) : Fl_Double_Window(X,Y,W,H,l), createdX(X), createdY(Y), createdW(W), createdH(H), first(false) 
{
}
rkWindow::rkWindow(int W, int H, const char *l) : Fl_Double_Window(W,H,l), createdW(W), createdH(H), first(false)
{ 
	resize(x(), y(), createdW, createdH);
	createdX = x();
	createdY = y();
}


int rkWindow::handle(int evt)
{
	int res = Fl_Double_Window::handle(evt);
	if (evt == FL_PUSH)
	{
		for (unsigned int i=0; i<objs.size(); i++)
		{
			if (Fl::pushed() != objs.at(i)->getWidget())
				objs.at(i)->doDeactivate();
		}
	}
	return res;
}

void rkWindow::show()
{
	if (visible())
	{
		resize(x(), y(), createdW, createdH);
	}
	else
	{
		if (!first)
		{
			first = true;
			capture();
		}
		Fl_Window::show();
		resize(createdX, createdY, createdW, createdH);
	}
	Fl_Window::show();
}
void rkWindow::hide()
{
	Fl_Window::hide();
}

void rkWindow::capture() 
{
	Fl_Window::show();
	createdX = x();
	createdY = y();
	createdW = w();
	createdH = h();
	Fl_Window::hide();
}

void rkWindow::manual_resize(int w, int h)
{ 
	Fl_Window::hide(); 
	createdW = w; 
	createdH = h; 
	Fl_Window::show(); 
	resize(x(), y(), w, h); 
}
