#ifndef _VALUEINPUT_
#define _VALUEINPUT_
#include <FL/Fl_Input.H>
#include <CCAPI/Common.h>

#define VAL_TYPE_DESC		0
#define VAL_TYPE_ADDRESS	1
#define VAL_TYPE_VALUE		2
#define VAL_TYPE_MISC		3



class ValueInput :
	public Fl_Input
{
public:
	ValueInput(int x, int y, int w, int h, const char *l=0);
	~ValueInput(void);
	int handle(int);

	void setCodeType(bool v) {m_codeType = v;}
	void setHex(bool hex);
	void setValueType(char type);
	unsigned long getULValue();
	long long getLLValue();
	void setLiteral(bool v) { m_literal = v; }
	void setValType(char type) { m_valType = type; }
	char getValType() { return m_valType; }

private:
	string convertHexToInt(string hex);
	string convertHexToFloat(string hex);
	string convertIntToHex(string val);
	string convertFloatToHex(string val);

	bool m_hex;
	bool m_literal;
	char m_type;
	bool m_codeType;
	char m_valType;
};

#endif
