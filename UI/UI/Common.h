#ifndef _COMMON_RKCHEAT_
#define _COMMON_RKCHEAT_

#include <vector>
#include <deque>
#include <utility>
#include <tuple>
#include <CCAPI/Common.h>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include "ValueInput.h"
#include <iostream>
#include <memory>

#define SEARCH_STOPPED_CANCEL 0x00
#define SEARCH_STOPPED_PROPER 0x01
#define SEARCH_STOPPED_RESET 0x02
#define SEARCH_STOPPED_NO_RESULTS 0x03

#define get_user_data(type, data) (type)((unsigned long)data)

using namespace std;

struct WidgetField
{
	Fl_Input *description;
	ValueInput *address_input;
	ValueInput *value_input;
	Fl_Check_Button *freeze;
	Fl_Choice *type;
	WidgetField(Fl_Input *d, ValueInput *a, ValueInput *v, Fl_Choice *c, Fl_Check_Button *f) { description = d; address_input = a; value_input = v; freeze = f; type = c; }
	WidgetField() { freeze = 0; type = 0; value_input = 0; description = 0;}
	~WidgetField() {
		delete value_input; 
		delete address_input;
		delete freeze; delete type;
		delete description;
		address_input = 0;
		freeze = 0;
		type = 0;
		value_input = 0;
		description = 0;
	}
};

struct rkCheat_Code
{
	string description;
	unsigned long address;
	long long value;
	char type;
	bool freeze;
	bool sign;
	char _delete;
	WidgetField *widget;
	rkCheat_Code(){
		address = 0;
		value = 0;
		type = 0;
		freeze = false;
		widget = 0;
		description = "";
		sign = true;
		_delete = 0;
	}
	rkCheat_Code(string d, unsigned long add, long long val, char t, bool f, WidgetField *w){
		address = add;
		value = val;
		type = t;
		freeze = f;
		widget = w;
		description = d;
		sign = true;
		_delete = 0;
	}
	~rkCheat_Code() {
		if (widget)
		{
			delete widget;
			widget = 0;
		}
	}
	void operator=(rkCheat_Code &item) {
		description = item.description;
		address = item.address;
		value = item.value;
		type = item.type;
		freeze = item.freeze;
		widget = item.widget;
		sign = item.sign;
		_delete = item._delete;
	}
	void setSign(bool s) { sign = s; }
};

#ifdef _WIN32
	#include <direct.h>
    #define GetCurrentDir _getcwd
#else
	#include <unistd.h>
    #define GetCurrentDir getcwd
#endif

typedef map<unsigned long, vector<ResultList> *> rkCheat_Results;

typedef pair<unsigned long, unsigned long> rkCheat_RangeItem;
typedef vector<rkCheat_RangeItem> rkCheat_RangeList;

typedef pair<unsigned long, unsigned long> rkCheat_ResultItem;
typedef vector<rkCheat_ResultItem> rkCheat_ResultList;

typedef shared_ptr<rkCheat_Code> rkCheat_CodeItem;
typedef deque<rkCheat_CodeItem> rkCheat_CodeList;


static bool isNotHex(int c) { return !isxdigit(c); }
static bool isNotDigit(int c) { return !isdigit(c); }


#endif
