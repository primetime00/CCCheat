#ifndef _INFOWINDOW_
#define _INFOWINDOW_

#include "rkWindow.h"
#include <FL/Fl_Text_Display.H>
#include "Common.h"

using namespace std;


class InfoWindow : public rkWindow
{
public:
	InfoWindow(int X, int Y, int W, int H, const char *l=0);
	InfoWindow(int W, int H, const char *l=0);

	static char info[];

private:
	Fl_Text_Display *display;



};


#endif
