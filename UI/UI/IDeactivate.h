#ifndef _RKWIDGET_
#define _RKWIDGET_

#include <FL/Fl_Widget.H>

class IDeactivate
{
public:
	~IDeactivate() {}
	virtual void doDeactivate() = 0;
	virtual Fl_Widget *getWidget() = 0;

};
#endif