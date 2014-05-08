#ifndef _VALUEINPUT_
#define _VALUEINPUT_
#include <FL/Fl_Input.H>
#include <CCAPI/Common.h>


class ValueInput :
	public Fl_Input
{
public:
	ValueInput(int x, int y, int w, int h, const char *l=0);
	~ValueInput(void);
	int handle(int);

	void setHex(bool hex);
	void setValueType(char type);
	unsigned long getULValue();
	long long getLLValue();
	void setLiteral(bool v) { m_literal = v; }

private:
	string convertHexToInt(string hex);
	string convertHexToFloat(string hex);
	string convertIntToHex(string val);
	string convertFloatToHex(string val);

	bool m_hex;
	bool m_literal;
	char m_type;
};

#endif
