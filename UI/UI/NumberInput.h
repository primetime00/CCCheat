#ifndef _NUMBERINPUT_
#define _NUMBERINPUT_
#include <FL/Fl_Input.H>
#include <functional>

class NumberInput :
	public Fl_Input
{
public:
	NumberInput(int x, int y, int w, int h, const char *l=0);
	~NumberInput(void);
	int handle(int);

	void setMaxNumber(long number) { m_max = number; }
	void setMinNumber(long number) { m_min = number; }
	long long getLLValue();



private:
	long m_max, m_min;

};

#endif
