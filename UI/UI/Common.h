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
#include <string>
#include <string.h>
#include <memory>

#define CCCHEAT_VERSION "1.20"

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
	char type;
	bool freeze;
	char _delete;
	WidgetField *widget;
	rkCheat_Code(){
		m_address->address = 0;
		m_address->value = 0;
		m_address->sign = 1;
		type = 0;
		freeze = false;
		widget = 0;
		description = "";
		_delete = 0;
	}
	rkCheat_Code(string d, unsigned long address, PointerItem p, bool f, WidgetField *w){
		m_address->address = address;
		m_address->pointer = p;
		m_address->sign = 1;
		freeze = f;
		widget = w;
		description = d;
		_delete = 0;
	}
	rkCheat_Code(string d, AddressItem i, bool f, WidgetField *w){
		m_address = i;
		freeze = f;
		widget = w;
		description = d;
		_delete = 0;
	}
	rkCheat_Code(string d, unsigned long add, long long val, char t, bool f, WidgetField *w){
		m_address->address = add;
		m_address->value = val;
		m_address->sign = 1;
		type = t;
		freeze = f;
		widget = w;
		description = d;
		_delete = 0;
	}
	rkCheat_Code(rkCheat_Code &item){
		*this = item;
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
		if (m_address == nullptr)
			m_address = make_shared<AddressObj>();
		m_address->address = item.m_address->address;
		m_address->value = item.m_address->value;
		if (item.m_address->pointer != nullptr)
			m_address->pointer = make_shared<PointerObj>(item.m_address->address, item.m_address->pointer->getOffsets());
		type = item.type;
		freeze = item.freeze;
		widget = item.widget;
		m_address->sign = item.m_address->sign;
		m_address->type = item.m_address->type;
		_delete = item._delete;
	}
	void setSign(bool s) { m_address->sign = s; }
};

struct ResultRow {
	AddressItem item;
	unsigned long section;
	unsigned long thread;
	unsigned long searchID;
	ResultRow(AddressItem i, unsigned long s, unsigned long t, unsigned long sid) { item = i; section = s; thread = t; searchID = sid;}
};

typedef vector<ResultRow> Results;

struct rkTrainerCodeObj
{
	string name;
	bool condition;
	int  conditionType;
	long long conditionValue;
	AddressItem m_conditionAddress;
	AddressList m_addresses;

	rkTrainerCodeObj(rkTrainerCodeObj &item) {
		*this = item;
	}

	~rkTrainerCodeObj() { }
	rkTrainerCodeObj() {
		name = "";
		conditionType = 0;
		condition = false;
		conditionValue = 0;
		m_addresses.clear();
	}

	void operator=(rkTrainerCodeObj &item) {
		name = item.name;
		condition = item.condition;
		conditionType = item.conditionType;
		conditionValue = item.conditionValue;
		m_conditionAddress = make_shared<AddressObj>(item.m_conditionAddress->address, item.m_conditionAddress->pointer, item.m_conditionAddress->type, item.m_conditionAddress->sign);
		m_addresses.clear();
		for (auto i = item.m_addresses.begin(); i != item.m_addresses.end(); ++i)
			m_addresses.push_back(make_shared<AddressObj>((*i)->address, (*i)->pointer, (*i)->type, (*i)->sign));
	}

	int write(char *buf) {
		int pos = 0;
		buf[pos] = name.length(); pos+=1;
		memcpy(&buf[pos], name.c_str(), (int)name.length()); pos += (int)name.length();
		buf[pos] = condition; pos+=1;
		if (condition)
		{
			buf[pos] = conditionType; pos+=1;
			m_conditionAddress->value = m_conditionAddress->store = conditionValue;
			pos += m_conditionAddress->write(&buf[pos]);
		}
		buf[pos] = m_addresses.size(); pos+=1;
		for (auto i=m_addresses.begin(); i != m_addresses.end(); ++i)
		{
			pos += (*i)->write(&buf[pos]);
		}
		return pos;
	}

	int read(char *buf) {
		int pos = 0;
		int nameLen = buf[pos]; pos+=1;
		name = string(&buf[pos], nameLen); pos += (int)nameLen;
		condition = buf[pos] > 0 ? true : false; pos+=1;
		if (condition)
		{
			conditionType = buf[pos]; pos+=1;
			m_conditionAddress = make_shared<AddressObj>();
			pos += m_conditionAddress->read(&buf[pos]);
			conditionValue = m_conditionAddress->value;
		}
		int numOfCodes = buf[pos]; pos+=1;
		for (int i=0; i< numOfCodes; ++i)
		{
			auto code = make_shared<AddressObj>();
			pos += code->read(&buf[pos]);
			m_addresses.push_back(code);
		}
		return pos;
	}

	void debug() {
		cout << "Code Info: " << endl;
		cout << "Name: " << name << endl;
		cout << "Condition? " << (condition ? "true" : "false") << endl;
		if (condition)
		{
			cout << "Condition Type: " << conditionType << endl;
			cout << "Condition Value: " << conditionValue << endl;
			m_conditionAddress->debug();
		}
		cout << "Number of inside codes: " << m_addresses.size() << endl << endl;
		for (auto i = m_addresses.begin(); i!=m_addresses.end(); ++i)
			(*i)->debug();
	}
};

typedef shared_ptr<rkTrainerCodeObj> rkTrainerCode;


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


static bool isNotHex(int c) __attribute__ ((unused));
static bool isNotHex(int c) { return !isxdigit(c); }
static bool isNotDigit(int c) __attribute__ ((unused));
static bool isNotDigit(int c) { return !isdigit(c); }


#endif
