#include "NumberInput.h"
#include <string>
#include <algorithm>
#include <FL/Fl.H>
#include <climits>
#include "Common.h"


using namespace std;

NumberInput::NumberInput(int x, int y, int w, int h, const char *l) : Fl_Input(x,y,w,h,l)
{
	m_max = LONG_MAX;
	m_min = LONG_MIN;
}

int NumberInput::handle(int e)
{
	string val, current;
	string txt = value();
	switch ( e ) 
	{
		case FL_PASTE:
			val = Fl::event_text();
			std::transform(val.begin(), val.end(), val.begin(), ::toupper);
			val.erase( remove_if(val.begin(), val.end(), isNotDigit), val.end() );
			if (stol(val, nullptr, 10) > m_max || stol(val) < m_min)
			{
				value("");
				return 1;
			}
			value(val.c_str());
			return 1;
			break;
		case FL_KEYUP:
			break;
		case FL_KEYDOWN: {
			cut();
			current = value();
			val = Fl::event_text();
			if (val.empty())
				return(Fl_Input::handle(e));
			std::transform(val.begin(), val.end(), val.begin(), ::tolower);
			if (isdigit(val[0]))
			{
				long v = stol(current+val[0], nullptr, 10);
				if (v > m_max || v < m_min)
					return 1;
				return(Fl_Input::handle(e));
			}
			else if (val == "-" && txt.size() == 0 && m_min < 0)
				return(Fl_Input::handle(e));
			else if (isprint(val[0]))
				return 1;
			return(Fl_Input::handle(e));
			break;
		}
		default: return(Fl_Input::handle(e)); break;
	}
    return 1;
}

long long NumberInput::getLLValue()
{
	string v = value();
	if (v.empty())
		return 0;
	return stoll(value(), nullptr, 10);
}


NumberInput::~NumberInput(void)
{
}
