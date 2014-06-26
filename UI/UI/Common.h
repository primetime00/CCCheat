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

#define CCCHEAT_VERSION "1.10"

#define SEARCH_STOPPED_CANCEL 0x00
#define SEARCH_STOPPED_PROPER 0x01
#define SEARCH_STOPPED_RESET 0x02
#define SEARCH_STOPPED_NO_RESULTS 0x03

#define get_user_data(type, data) (type)((unsigned long)data)

using namespace std;

class rkCheatUI;
extern rkCheatUI *uiInstance;

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
	AddressItem m_address;
	PointerItem m_pointer;
	bool m_isPointer;

/*	unsigned long address;
	long long value;
	bool sign;*/
	char type;
	bool freeze;
	char _delete;
	WidgetField *widget;
	rkCheat_Code(){
		m_isPointer = false;
		m_address.address = 0;
		m_address.value = 0;
		m_address.sign = 1;
		type = 0;
		freeze = false;
		widget = 0;
		description = "";
		_delete = 0;
	}
	rkCheat_Code(string d, PointerItem p, bool f, WidgetField *w){
		m_isPointer = true;
		m_pointer = p;
		m_address.sign = 1;
		freeze = f;
		widget = w;
		description = d;
		_delete = 0;
	}
	rkCheat_Code(string d, unsigned long add, long long val, char t, bool f, WidgetField *w){
		m_isPointer = false;
		m_address.address = add;
		m_address.value = val;
		m_address.sign = 1;
		type = t;
		freeze = f;
		widget = w;
		description = d;
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
		m_address.address = item.m_address.address;
		m_address.value = item.m_address.value;
		type = item.type;
		freeze = item.freeze;
		widget = item.widget;
		m_address.sign = item.m_address.sign;
		_delete = item._delete;
	}
	void setSign(bool s) { m_address.sign = s; }
};

struct ResultRow {
	AddressItem *item;
	unsigned long section;
	unsigned long thread;
	unsigned long searchID;
	ResultRow(AddressItem *i, unsigned long s, unsigned long t, unsigned long sid) { item = i; section = s; thread = t; searchID = sid;}
};

typedef vector<ResultRow> Results;


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
