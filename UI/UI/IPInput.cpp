#include "IPInput.h"
#include <iostream>
#include <sstream>
#include <FL/Fl.H>

using namespace std;

IPInput::IPInput(int X, int Y, int W, int H, const char *l) : Fl_Group(X,Y,W,H,l)
{
	const int gap = 5;
	const int bWidth = (int)((W/4.0f) - (gap*3));
	const int inc = bWidth + gap;
	begin();
	inp_box[0] = new NumberInput(X,Y,bWidth,H,0); X+=inc;
	inp_box[1] = new NumberInput(X,Y,bWidth,H,0); X+=inc;
	inp_box[2] = new NumberInput(X,Y,bWidth,H,0); X+=inc;
	inp_box[3] = new NumberInput(X,Y,bWidth,H,0);
	end();
	inp_box[0]->callback(boxInputCB, this);
	inp_box[1]->callback(boxInputCB, this);
	inp_box[2]->callback(boxInputCB, this);
	inp_box[3]->callback(boxInputCB, this);

	inp_box[0]->when(FL_WHEN_CHANGED);
	inp_box[1]->when(FL_WHEN_CHANGED);
	inp_box[2]->when(FL_WHEN_CHANGED);
	inp_box[3]->when(FL_WHEN_CHANGED);

	inp_box[0]->setMaxNumber(255);
	inp_box[1]->setMaxNumber(255);
	inp_box[2]->setMaxNumber(255);
	inp_box[3]->setMaxNumber(255);

	inp_box[0]->setMinNumber(0);
	inp_box[1]->setMinNumber(0);
	inp_box[2]->setMinNumber(0);
	inp_box[3]->setMinNumber(0);

	inp_box[0]->maximum_size(3);
	inp_box[1]->maximum_size(3);
	inp_box[2]->maximum_size(3);
	inp_box[3]->maximum_size(3);

	inp_box[0]->align(FL_ALIGN_CENTER);
	inp_box[1]->align(FL_ALIGN_CENTER);
	inp_box[2]->align(FL_ALIGN_CENTER);
	inp_box[3]->align(FL_ALIGN_CENTER);

	setIP("192.168.255.1");
}

IPInput::~IPInput()
{
	clear();
}

void IPInput::boxInputCB(Fl_Widget *w, void *p)
{
	IPInput *m_IPInput = (IPInput*)p;
	if (((NumberInput*)w)->size() >= 3)
	{
		NumberInput *next = m_IPInput->getNextBox((NumberInput*)w);
		next->take_focus();
		next->position(0);
		next->mark(3);
	}
	m_IPInput->checkValid();
}

void IPInput::checkValid()
{
	bool valid = true;
	for (int i=0; i<4; ++i)
	{
		if (inp_box[i]->size() == 0)
		{
			valid = false;
			break;
		}
	}
	do_callback(this, valid);
/*	if (validFunc != nullptr)
		validFunc(valid);*/
}

NumberInput *IPInput::getNextBox(NumberInput *n)
{
	if (n == inp_box[0])
		return inp_box[1];
	if (n == inp_box[1])
		return inp_box[2];
	if (n == inp_box[2])
		return inp_box[3];
	if (n == inp_box[3])
		return inp_box[0];
	return 0;
}

string IPInput::getIP()
{
	stringstream ip;
	ip  << inp_box[0]->value() << "." << inp_box[1]->value() << "." << inp_box[2]->value() << "." << inp_box[3]->value();
	return ip.str();
}
void IPInput::setIP(string ip)
{
	int sip[4];
	for (string::iterator it= ip.begin(); it!= ip.end();)
	{
		if (*it == '.')
		{
			*it = ' '; ++it;
		}
		else if (!isdigit(*it))
		{
			it = ip.erase(it);
		}
		else
			++it;
	}
	stringstream inp(ip);
	inp >> sip[0] >> sip[1] >> sip[2] >> sip[3];
	for (int i=0; i<4; i++)
		if (sip[i] > 255 || sip[i] < 0)
			sip[i] = 0;
	inp_box[0]->value(to_string(sip[0]).c_str());
	inp_box[1]->value(to_string(sip[1]).c_str());
	inp_box[2]->value(to_string(sip[2]).c_str());
	inp_box[3]->value(to_string(sip[3]).c_str());
}
