#ifndef _HELPERS_
#define _HELPERS_
#include <string>
#include <sstream>
#include <cmath>
#include "Common.h"

using namespace std;

template <typename T>
string NumberToString ( T Number )
{
	stringstream ss;
	ss << Number;
	return ss.str();
}

class Helpers
{
public:
	static int getTypeLength(char type)
	{
		switch (type)
		{
		case SEARCH_VALUE_TYPE_1BYTE:
			return 1;
		case SEARCH_VALUE_TYPE_2BYTE:
			return 2;
		default:
			return 4;
		}
	}

	static bool isSigned(char type, long long value)
	{
		if (value < 0)
			return true;
		int len = getTypeLength(type) * 8;
		if ( value < (unsigned long)((1 << len-1)) )
			return true;
		return false;
	}

	static long long convertValueType(long long value, char newType, char oldType, bool sign)
	{
		switch (oldType)
		{
		case SEARCH_VALUE_TYPE_1BYTE:
			if (newType == SEARCH_VALUE_TYPE_2BYTE)
				return (long) value;
			if (newType == SEARCH_VALUE_TYPE_4BYTE)
				return (long) value;
			if (newType == SEARCH_VALUE_TYPE_FLOAT)
				return *(unsigned long*)&value;
			break;
		case SEARCH_VALUE_TYPE_2BYTE:
			if (newType == SEARCH_VALUE_TYPE_1BYTE) //2 to 1
				if (sign)
					return (char) value;
				else
					return (unsigned char) value;
			if (newType == SEARCH_VALUE_TYPE_4BYTE)
				return (long) value;
			if (newType == SEARCH_VALUE_TYPE_FLOAT)
				return *(unsigned long*)&value;
			break;
		case SEARCH_VALUE_TYPE_4BYTE:
			if (newType == SEARCH_VALUE_TYPE_1BYTE) //4 to 1
				if (sign)
					return (char) value;
				else
					return (unsigned char) value;
			if (newType == SEARCH_VALUE_TYPE_2BYTE) //4 to 2
				if (sign)
					return (short) value;
				else
					return (unsigned short) value;
			if (newType == SEARCH_VALUE_TYPE_FLOAT)
				return *(unsigned long*)&value;
			break;
		default:
			if (newType == SEARCH_VALUE_TYPE_1BYTE) //float to 1
				return *(char*)&value;
			if (newType == SEARCH_VALUE_TYPE_2BYTE) //float to 2
				return *(short*)&value;
			if (newType == SEARCH_VALUE_TYPE_4BYTE) //float to 4
				return *(long*)&value;
			break;
		}
		return 0;
	}
};

#endif