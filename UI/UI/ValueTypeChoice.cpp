#include "CCAPI/Common.h"
#include "Common.h"
#include "ValueTypeChoice.h"
#include <iostream>

using namespace std;

Fl_Menu_Item ValueTypeChoice::items[] = {
 {"1 Byte", 0,  0, (void*)(SEARCH_VALUE_TYPE_1BYTE), 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"2 Bytes", 0,  0, (void*)(SEARCH_VALUE_TYPE_2BYTE), 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"4 Bytes", 0,  0, (void*)(SEARCH_VALUE_TYPE_4BYTE), 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Float", 0,  0, (void*)(SEARCH_VALUE_TYPE_FLOAT), 0, FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0}
};

ValueTypeChoice::ValueTypeChoice(int X, int Y, int W, int H, const char *l) : Fl_Choice(X,Y,W,H,l)
{
	Fl_Menu_Item *it = &items[0];
	while (it->label() != 0)
	{
		menuMap[get_user_data(int, it->user_data())] = it;
		it++;
	}
	menu(items);
	reset();
}


ValueTypeChoice::~ValueTypeChoice(void)
{
}


void ValueTypeChoice::reset()
{
	menuMap[SEARCH_VALUE_TYPE_1BYTE]->show();
	menuMap[SEARCH_VALUE_TYPE_2BYTE]->show();
	menuMap[SEARCH_VALUE_TYPE_4BYTE]->show();
	menuMap[SEARCH_VALUE_TYPE_FLOAT]->show();
	value(menuMap[SEARCH_VALUE_TYPE_4BYTE]);
}


char ValueTypeChoice::getValue()
{
	return get_user_data(char, mvalue()->user_data());
}
