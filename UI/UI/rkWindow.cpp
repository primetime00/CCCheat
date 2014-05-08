#include "rkWindow.h"
#include <FL/Fl.H>


vector<IDeactivate *> rkWindow::objs;

int rkWindow::handle(int evt)
{
	int res = Fl_Double_Window::handle(evt);
	if (evt == FL_PUSH)
	{
		for (int i=0; i<objs.size(); i++)
		{
			if (Fl::pushed() != objs.at(i)->getWidget())
				objs.at(i)->doDeactivate();
		}
	}
	return res;
}
