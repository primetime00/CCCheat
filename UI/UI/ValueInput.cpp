#include "ValueInput.h"
#include "Common.h"
#include <string>
#include <algorithm>
#include <cmath>
#include <FL/Fl.H>

using namespace std;

ValueInput::ValueInput(int x, int y, int w, int h, const char *l) : Fl_Input(x,y,w,h,l)
{
	setLiteral(true);
	setValueType(SEARCH_VALUE_TYPE_1BYTE);
	setCodeType(false);
	setHex(false);
}

#define MAX_NON_HEX(t) maximum_size( (t == SEARCH_VALUE_TYPE_FLOAT) ? 20 : 10 )

void ValueInput::setValueType(char type)
{
	string text = value();
	if (text.length() == 0)
	{
		m_type = type;
		if (!m_hex) MAX_NON_HEX(type);
		return;
	}
	if (type != m_type) //we have a different type!
	{
		if ( (m_type == SEARCH_VALUE_TYPE_4BYTE && (type == SEARCH_VALUE_TYPE_1BYTE || type == SEARCH_VALUE_TYPE_2BYTE)) || 
			 (m_type == SEARCH_VALUE_TYPE_2BYTE && (type == SEARCH_VALUE_TYPE_1BYTE))) //truncate!
		{
			long long val;
			if (m_hex)
				val = stoull(text, nullptr, 16);
			else
				val = stoull(text, nullptr, 10);
			if (type == SEARCH_VALUE_TYPE_1BYTE) val = val < 0 ? (char) val : (unsigned char) val;
			else if (type == SEARCH_VALUE_TYPE_2BYTE) val = val < 0 ? (short)val : (unsigned short) val;
			value(m_hex ? convertIntToHex(to_string(val)).c_str() : to_string(val).c_str());
		}
		else if (m_type == SEARCH_VALUE_TYPE_FLOAT) //convert and trunc
		{
			if (!m_literal)
			{
				unsigned int val;
				if (m_hex)
				{
					Variant variant((long long)stoull(text, nullptr, 16));
					val = (unsigned int) variant.asFloat();
				}
				else
				{
					val = (unsigned int) stof(text);
				}
				if (type == SEARCH_VALUE_TYPE_1BYTE) val = val < 0 ? (char) val : (unsigned char) val;
				else if (type == SEARCH_VALUE_TYPE_2BYTE) val = val < 0 ? (short) val : (unsigned short) val;
				value(m_hex ? convertIntToHex(to_string(val)).c_str() : to_string(val).c_str());
			}
			else
			{
				long long val;
				if (m_hex)
				{
					val = stoull(text, nullptr, 16);
				}
				else
				{
					val = (long long) stof(text);
				}
				if (type == SEARCH_VALUE_TYPE_1BYTE) val = val < 0 ? (char) val : (unsigned char) val;
				else if (type == SEARCH_VALUE_TYPE_2BYTE) val = val < 0 ? (short) val : (unsigned short) val;
				value(m_hex ? convertIntToHex(to_string(val)).c_str() : to_string(val).c_str());
			}
		}
		else if (type == SEARCH_VALUE_TYPE_FLOAT) //convert from integer to float
		{
			if (!m_literal)
			{
				float res;
				if (m_hex)
				{
					Variant variant((long long)stoull(text, nullptr, 16));
					res = variant.asFloat();
				}
				else
				{
					Variant variant((long long)stoull(text, nullptr, 10));
					res = variant.asFloat();
				}
				value(to_string(res).c_str());
			}
			else
			{
				long long val;
				float res;
				if (m_hex)
				{
					val = stoull(text, nullptr, 16);
					res = (float) val;
				}
				else
				{
					val = stoull(text, nullptr, 10);
					res = (float) val;
				}
				string s = to_string(res);
				value(s.c_str());
			}
		}
	}
	if (!m_hex) MAX_NON_HEX(type);
	m_type = type;
}

void ValueInput::setHex(bool hex)
{ 
	string text = value();
	if (text.length() == 0)
	{
		m_hex = hex;
		if (m_hex)
			maximum_size(8);
		else
			MAX_NON_HEX(m_type);
		return;
	}
	if (m_hex && !hex) //hex to int/float
	{
		MAX_NON_HEX(m_type);
		if (m_type == SEARCH_VALUE_TYPE_FLOAT)
			value(convertHexToFloat(text).c_str());
		else
			value(convertHexToInt(text).c_str());
	}
	if (!m_hex && hex) //int/float to hex
	{
		if (m_type == SEARCH_VALUE_TYPE_FLOAT)
			value(convertFloatToHex(text).c_str());
		else
			value(convertIntToHex(text).c_str());
		maximum_size(8);
	}
	m_hex = hex;
}

string ValueInput::convertHexToInt(string hex)
{
	long long x;
	x = stoull(hex, nullptr, 16);
	return to_string(x);
}
string ValueInput::convertHexToFloat(string hex)
{
	float v;
	Variant variant((long long)stoull(hex, nullptr, 16));
	v = variant.asFloat();
#if defined(_WIN32) || defined(WIN32)
	if (_isnan(v))
#else
	if (isnan(v))
#endif
		return "0";
	return to_string(v);
}
string ValueInput::convertIntToHex(string val)
{
	char buf[50];
	sprintf(buf, "%lX", stoul(val, nullptr, 10));
	return string(buf);
}
string ValueInput::convertFloatToHex(string val)
{
	char buf[50];
	Variant variant(stof(val));
	sprintf(buf, "%X", (unsigned int)variant.asLong());
	return string(buf);
}

int ValueInput::handle(int e)
{
	string val;
	string txt = value();
	switch ( e ) 
	{
		case FL_PASTE:
			val = Fl::event_text();
			std::transform(val.begin(), val.end(), val.begin(), ::toupper);
			if (m_hex && val.length() > 1 && val[0] == '0' && val[1] == 'X')
			{
				val.erase(val.begin(), val.begin()+2);
			}
			if (m_hex)
				val.erase( remove_if(val.begin(), val.end(), isNotHex), val.end() );
			else
				val.erase( remove_if(val.begin(), val.end(), isNotDigit), val.end() );
			if ((signed)val.length() > maximum_size())
				val.erase(val.begin()+maximum_size(), val.end());
			value(val.c_str());
			if (m_codeType)
			{
				do_callback();
				return 1;

			}
			return 1;
			break;
		case FL_KEYUP:
		case FL_KEYDOWN: {
			val = Fl::event_text();
			if (val[0] == 27) //escape key pressed
			{
				val = value();
				value(val.c_str());
				return(Fl_Input::handle(e));
			}
			if (val.empty())
				return(Fl_Input::handle(e));
			std::transform(val.begin(), val.end(), val.begin(), ::tolower);
			if (m_hex)
			{
				if (val == "a" || val == "b" || val == "c" || val == "d" || val == "e" || val == "f")
					return(Fl_Input::handle(e));
				else if (isdigit(val[0]))
					return(Fl_Input::handle(e));
				else if (isprint(val[0]))
					return 1;
			}
			else
			{
				if (isdigit(val[0]))
					return(Fl_Input::handle(e));
				else if (val == "-" && txt.size() == 0)
					return(Fl_Input::handle(e));
				else if (val == "." && m_type == SEARCH_VALUE_TYPE_FLOAT && txt.find(".") == string::npos)
					return(Fl_Input::handle(e));
				else if (isprint(val[0]))
					return 1;
			}
			return(Fl_Input::handle(e));
			break;
		}
		default: return(Fl_Input::handle(e)); break;
	}
    return 1;
}

unsigned long ValueInput::getULValue()
{
	string v = value();
	if (v.empty())
		return 0;
	if (m_type == SEARCH_VALUE_TYPE_FLOAT)
	{
		if (m_hex)
			return stoul(value(), nullptr, 16);
		else
		{
			string val = convertFloatToHex(value());
			return stoul(val, nullptr, 16);
		}
	}
	else
	{
		if (m_hex)
			return stoul(value(), nullptr, 16);
		else
			return stoul(value(), nullptr, 10);
	}
}

long long ValueInput::getLLValue()
{
	string v = value();
	if (v.empty())
		return 0;
	if (m_type == SEARCH_VALUE_TYPE_FLOAT)
	{
		if (m_hex)
			return stoul(value(), nullptr, 16);
		else
		{
			string val = convertFloatToHex(value());
			return stoul(val, nullptr, 16);
		}
	}
	else
	{
		if (m_hex)
			return stoll(value(), nullptr, 16);
		else
			return stoll(value(), nullptr, 10);
	}
}

void ValueInput::setValue(unsigned long ivalue)
{
	string strVal;
	if (m_hex)
		strVal = convertIntToHex(to_string(ivalue));
	else
	{
		MAX_NON_HEX(m_type);
		if (m_type == SEARCH_VALUE_TYPE_PT_FLOAT)
		{
			setValue(*(float*) (&ivalue)[0]);
			return;
		}
		strVal = to_string(ivalue);
	}
	
	value(strVal.c_str());
}

void ValueInput::setValue(long long ivalue)
{
	string strVal;
	if (m_hex)
		strVal = convertIntToHex(to_string(ivalue));
	else
	{
		MAX_NON_HEX(m_type);
		if (m_type == SEARCH_VALUE_TYPE_PT_FLOAT)
		{
			setValue(*(float*) (&ivalue)[0]);
			return;
		}
		strVal = to_string(ivalue);
	}
	
	value(strVal.c_str());
}

void ValueInput::setValue(float ivalue)
{
	string strVal;
	if (m_hex)
		strVal = convertFloatToHex(to_string(ivalue));
	else
	{
		MAX_NON_HEX(m_type);
		strVal = to_string(ivalue);
	}

	value(strVal.c_str());
}

void ValueInput::setValue(long long value, char type, bool lit)
{
	auto tmp_lit = m_literal;
	setLiteral(lit);
	setValue(value);
	setValueType(type);
	setLiteral(tmp_lit);
}

ValueInput::~ValueInput(void)
{
}
