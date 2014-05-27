#include "InfoWindow.h"

#include <FL/Fl_Text_Buffer.H>
using namespace std;

#define INFO_MARGIN 10
#define NL "\n"
char InfoWindow::info[] = 
	"CCCheat 1.0 was written by Primetime00." NL
	"This version may still have several bugs!" NL NL
	"NOTE: Right click on an address in the code table to view nearby addresses." NL
	"Special Thanks to:" NL
	"FM|T Enstone for CCAPI." NL
	"Dnawrkshp for NetCheat PS3";

InfoWindow::InfoWindow(int X, int Y, int W, int H, const char *l) : rkWindow(X,Y,W,H,l)
{
	begin();
	display = new Fl_Text_Display(INFO_MARGIN,INFO_MARGIN,W-INFO_MARGIN*3,H-INFO_MARGIN*3,0);
	display->wrap_mode(3,0);
	Fl_Text_Buffer *b = new Fl_Text_Buffer();
	b->insert(0, InfoWindow::info);
	display->buffer(b);
	end();
}

InfoWindow::InfoWindow(int W, int H, const char *l) : rkWindow(W,H,l)
{
	begin();
	display = new Fl_Text_Display(INFO_MARGIN,INFO_MARGIN,W-(INFO_MARGIN*3),H-INFO_MARGIN*3,0);
	display->wrap_mode(3,0);
	Fl_Text_Buffer *b = new Fl_Text_Buffer();
	b->insert(0, InfoWindow::info);
	display->buffer(b);
	end();
}