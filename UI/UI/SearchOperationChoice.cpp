#include "CCAPI/Common.h"
#include "SearchOperationChoice.h"

using namespace std;

Fl_Menu_Item SearchOperationChoice::items[] = {
		{"EXACT VALUE", 0,  0, (void*)(SEARCH_VALUE_EXACT), 0, FL_NORMAL_LABEL, 0, 14, 0},
		{"LESS THAN VALUE", 0,  0, (void*)(SEARCH_VALUE_LESS), 0, FL_NORMAL_LABEL, 0, 14, 0},
		{"GREATER THAN VALUE", 0,  0, (void*)(SEARCH_VALUE_GREATER), 0, FL_NORMAL_LABEL, 0, 14, 0},
		{"NOT EQUAL TO VALUE", 0,  0, (void*)(SEARCH_VALUE_NOTEXACT), 0, FL_NORMAL_LABEL, 0, 14, 0},
		{"FUZZY", 0,  0, (void*)(SEARCH_FUZZY_INIT), 0, FL_NORMAL_LABEL, 0, 14, 0},
		{"DECREASED", 0,  0, (void*)(SEARCH_FUZZY_LESS), 16, FL_NORMAL_LABEL, 0, 14, 0},
		{"INCREASED", 0,  0, (void*)(SEARCH_FUZZY_GREATER), 16, FL_NORMAL_LABEL, 0, 14, 0},
		{"CHANGED", 0,  0, (void*)(SEARCH_FUZZY_NOTEQUAL), 16, FL_NORMAL_LABEL, 0, 14, 0},
		{"UNCHANGED", 0,  0, (void*)(SEARCH_FUZZY_EQUAL), 16, FL_NORMAL_LABEL, 0, 14, 0},
		{0,0,0,0,0,0,0,0,0}
	};

SearchOperationChoice::SearchOperationChoice(int X, int Y, int W, int H, const char *l) : Fl_Choice(X,Y,W,H,l)
{
	Fl_Menu_Item *it = &items[0];
	while (it->label() != 0)
	{
		menuMap[(int)it->user_data()] = it;
		it++;
	}
	menu(items);
	reset();
}


SearchOperationChoice::~SearchOperationChoice(void)
{
}


void SearchOperationChoice::reset()
{
	menuMap[SEARCH_VALUE_EXACT]->show();
	menuMap[SEARCH_VALUE_LESS]->show();
	menuMap[SEARCH_VALUE_GREATER]->show();
	menuMap[SEARCH_VALUE_NOTEXACT]->show();
	menuMap[SEARCH_FUZZY_INIT]->show();
	menuMap[SEARCH_FUZZY_LESS]->hide();
	menuMap[SEARCH_FUZZY_GREATER]->hide();
	menuMap[SEARCH_FUZZY_NOTEQUAL]->hide();
	menuMap[SEARCH_FUZZY_EQUAL]->hide();
	if (isFuzzy())
		value(menuMap[SEARCH_FUZZY_INIT]);
}

void SearchOperationChoice::firstValue()
{
	menuMap[SEARCH_VALUE_EXACT]->show();
	menuMap[SEARCH_VALUE_LESS]->show();
	menuMap[SEARCH_VALUE_GREATER]->show();
	menuMap[SEARCH_VALUE_NOTEXACT]->show();
	menuMap[SEARCH_FUZZY_INIT]->hide();
	menuMap[SEARCH_FUZZY_LESS]->show();
	menuMap[SEARCH_FUZZY_GREATER]->show();
	menuMap[SEARCH_FUZZY_NOTEQUAL]->show();
	menuMap[SEARCH_FUZZY_EQUAL]->show();
		if (isFuzzy() && getValue() == SEARCH_FUZZY_INIT)
		value(menuMap[SEARCH_FUZZY_GREATER]);

}

void SearchOperationChoice::firstFuzzy()
{
	firstValue();
}

bool SearchOperationChoice::isFuzzy()
{
	char v = (char)mvalue()->user_data();
	if (v == SEARCH_FUZZY_INIT || v == SEARCH_FUZZY_LESS || v == SEARCH_FUZZY_GREATER || v == SEARCH_FUZZY_NOTEQUAL || v == SEARCH_FUZZY_EQUAL)
		return true;
	return false;
}

char SearchOperationChoice::getValue()
{
	return (char)mvalue()->user_data();
}