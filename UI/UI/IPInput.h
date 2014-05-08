#ifndef __IPINPUT__
#define __IPINPUT__

#include <string>
#include <FL/Fl_Group.H>
#include "NumberInput.h"

using namespace std;

class IPInput : public Fl_Group
{
public:
	IPInput(int X, int Y, int W, int H, const char *l=0);
	~IPInput();

	string getIP();
	void setIP(string ip);

	static void boxInputCB(Fl_Widget *w, void *p);
	NumberInput *getNextBox(NumberInput *n);
	void checkValid();

private:
	NumberInput *inp_box[4];
};
#endif