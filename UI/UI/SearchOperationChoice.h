#ifndef _SEARCHCHOICE_
#define _SEARCHCHOICE_

#include "FL/Fl_Choice.H"
#include <map>

using namespace std;


class SearchOperationChoice : public Fl_Choice
{
public:
	SearchOperationChoice(int X, int Y, int W, int H, const char *l = 0);
	~SearchOperationChoice(void);

	void reset();
	void firstValue();
	void firstFuzzy();
	bool isFuzzy();
	bool isPointer();
	char getValue();

private:
	static Fl_Menu_Item items[];
	map<int, Fl_Menu_Item*> menuMap;
};

#endif

