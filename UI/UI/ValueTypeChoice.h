#ifndef _VALUETYPECHOICE_
#define _VALUETYPECHOICE_

#include "FL/Fl_Choice.H"
#include <map>

using namespace std;


class ValueTypeChoice : public Fl_Choice
{
public:
	ValueTypeChoice(int X, int Y, int W, int H, const char *l = 0);
	~ValueTypeChoice(void);

	void reset();
	char getValue();

private:
	static Fl_Menu_Item items[];
	map<int, Fl_Menu_Item*> menuMap;
};

#endif

