#include "ChunkFuzzySearch.h"
#include <iostream>

using namespace std;

//#define CMP(_type, val1, val2, loc) (testFunction_Signed( (_type) *((_type*)val1), (_type) *((_type*)val2)))
#define CMP(_type, vari, val1, val2, loc) (testFunction_Signed( vari.convertToLongLong((char*)val1), vari.convertToLongLong((char*)val2)))
//#define CMP_UNSIGNED(_type, val1, val2, loc) (testFunction_Unsigned((unsigned _type) *((unsigned _type*)val1), (unsigned _type) *((unsigned _type*)val2)))
#define CMP_UNSIGNED(_type, vari, val1, val2, loc) (testFunction_Signed( (unsigned long long) vari.convertToLongLong((char*)val1), (unsigned long long)vari.convertToLongLong((char*)val2)))

#define DO_COMPARE_FLOAT(vari, val1, val2, loc, base)\
	if (testFunction_Float(vari.convertToFloat((char*)val1), vari.convertToFloat((char*)val2)))\
	{\
		(*resultRef)[base].push_back(make_shared<AddressObj>(loc, (unsigned long)vari.convertToLong((char*)val1), TEST_SIGN_UNKNOWN));\
	}

#define DO_COMPARE_FLOAT_NEXT(section, vari, val, item)\
	signage = TEST_SIGN_NULL;\
	if (testFunction_Float(vari.convertToFloat((char*)val), vari.convertToFloat((char*)&(item->value))))\
	{\
		signage = TEST_SIGN_UNKNOWN;\
		(item->value) = (unsigned long) vari.convertToLong((char*)val);\
		(item->sign) = signage;\
	}


#define DO_COMPARE(_type, vari, val1, val2, loc, base)\
	signage = TEST_SIGN_NULL;\
	if (CMP(_type, vari, val1, val2, loc))\
		signage |= TEST_SIGN_YES;\
	if (CMP_UNSIGNED(_type, vari, val1, val2, loc))\
		signage |= TEST_SIGN_NO;\
	if (signage != TEST_SIGN_NULL)\
	{\
		(*resultRef)[base].push_back(make_shared<AddressObj>(loc, (unsigned long) vari.convertToLong((char*)val1), signage));\
	}

#define DO_COMPARE_NEXT(_type, section, vari, val, item)\
	signage = TEST_SIGN_NULL;\
	if ((item->sign) & TEST_SIGN_YES)\
	{\
		if (CMP(_type, vari, val, (_type*)&(item->value), (item->sign)))\
			signage |= TEST_SIGN_YES;\
	}\
	if ((item->sign) & TEST_SIGN_NO)\
	{\
		if (CMP_UNSIGNED(_type, vari, val, (_type*)&(item->value), (item->sign)))\
			signage |= TEST_SIGN_NO;\
	}\
	if (signage != TEST_SIGN_NULL)\
	{\
		(item->value) = (unsigned long) vari.convertToLong((char*)val);\
		(item->sign) = signage;\
	}


void ChunkFuzzySearch::setup(char len, char comp)
{
	switch (len)
	{
	case SEARCH_VALUE_TYPE_1BYTE: valueByteLength = 1; break;
	case SEARCH_VALUE_TYPE_2BYTE: valueByteLength = 2; break;
	case SEARCH_VALUE_TYPE_4BYTE: valueByteLength = 4; break;
	case SEARCH_VALUE_TYPE_FLOAT: valueByteLength = 4; isFloat = true; break;
	default: valueByteLength = 4; break;
	}
	comparison = comp;
	if (valueByteLength == 1)
	{
		if (comparison == SEARCH_FUZZY_EQUAL)
		{
			testFunction_Signed = bind(&ChunkFuzzySearch::testEqual<char>, this, placeholders::_1, placeholders::_2);
			testFunction_Unsigned = bind(&ChunkFuzzySearch::testEqual<unsigned char>, this, placeholders::_1, placeholders::_2);
		}
		else if (comparison == SEARCH_FUZZY_NOTEQUAL)
		{
			testFunction_Signed = bind(&ChunkFuzzySearch::testNotEqual<char>, this, placeholders::_1, placeholders::_2);
			testFunction_Unsigned = bind(&ChunkFuzzySearch::testNotEqual<unsigned char>, this, placeholders::_1, placeholders::_2);
		}
		else if (comparison == SEARCH_FUZZY_GREATER)
		{
			testFunction_Signed = bind(&ChunkFuzzySearch::testGreater<char>, this, placeholders::_1, placeholders::_2);
			testFunction_Unsigned = bind(&ChunkFuzzySearch::testGreater<unsigned char>, this, placeholders::_1, placeholders::_2);
		}
		else
		{
			testFunction_Signed = bind(&ChunkFuzzySearch::testLess<char>, this, placeholders::_1, placeholders::_2);
			testFunction_Unsigned = bind(&ChunkFuzzySearch::testLess<unsigned char>, this, placeholders::_1, placeholders::_2);
		}
	}
	else if (valueByteLength == 2)
	{
		if (comparison == SEARCH_FUZZY_EQUAL)
		{
			testFunction_Signed = bind(&ChunkFuzzySearch::testEqual<short>, this, placeholders::_1, placeholders::_2);
			testFunction_Unsigned = bind(&ChunkFuzzySearch::testEqual<unsigned short>, this, placeholders::_1, placeholders::_2);
		}
		else if (comparison == SEARCH_FUZZY_NOTEQUAL)
		{
			testFunction_Signed = bind(&ChunkFuzzySearch::testNotEqual<short>, this, placeholders::_1, placeholders::_2);
			testFunction_Unsigned = bind(&ChunkFuzzySearch::testNotEqual<unsigned short>, this, placeholders::_1, placeholders::_2);
		}
		else if (comparison == SEARCH_FUZZY_GREATER)
		{
			testFunction_Signed = bind(&ChunkFuzzySearch::testGreater<short>, this, placeholders::_1, placeholders::_2);
			testFunction_Unsigned = bind(&ChunkFuzzySearch::testGreater<unsigned short>, this, placeholders::_1, placeholders::_2);
		}
		else
		{
			testFunction_Signed = bind(&ChunkFuzzySearch::testLess<short>, this, placeholders::_1, placeholders::_2);
			testFunction_Unsigned = bind(&ChunkFuzzySearch::testLess<unsigned short>, this, placeholders::_1, placeholders::_2);
		}
	}
	else if (valueByteLength == 4 && !isFloat)
	{
		if (comparison == SEARCH_FUZZY_EQUAL)
		{
			testFunction_Signed = bind(&ChunkFuzzySearch::testEqual<long>, this, placeholders::_1, placeholders::_2);
			testFunction_Unsigned = bind(&ChunkFuzzySearch::testEqual<unsigned long>, this, placeholders::_1, placeholders::_2);
		}
		else if (comparison == SEARCH_FUZZY_NOTEQUAL)
		{
			testFunction_Signed = bind(&ChunkFuzzySearch::testNotEqual<long>, this, placeholders::_1, placeholders::_2);
			testFunction_Unsigned = bind(&ChunkFuzzySearch::testNotEqual<unsigned long>, this, placeholders::_1, placeholders::_2);
		}
		else if (comparison == SEARCH_FUZZY_GREATER)
		{
			testFunction_Signed = bind(&ChunkFuzzySearch::testGreater<long>, this, placeholders::_1, placeholders::_2);
			testFunction_Unsigned = bind(&ChunkFuzzySearch::testGreater<unsigned long>, this, placeholders::_1, placeholders::_2);
		}
		else
		{
			testFunction_Signed = bind(&ChunkFuzzySearch::testLess<long>, this, placeholders::_1, placeholders::_2);
			testFunction_Unsigned = bind(&ChunkFuzzySearch::testLess<unsigned long>, this, placeholders::_1, placeholders::_2);
		}
	}
	else
	{
		if (comparison == SEARCH_FUZZY_EQUAL)
		{
			testFunction_Float = bind(&ChunkFuzzySearch::testFloatEqual, this, placeholders::_1, placeholders::_2);
		}
		else if (comparison == SEARCH_FUZZY_NOTEQUAL)
		{
			testFunction_Float = bind(&ChunkFuzzySearch::testFloatNotEqual, this, placeholders::_1, placeholders::_2);
		}
		else if (comparison == SEARCH_FUZZY_GREATER)
		{
			testFunction_Float = bind(&ChunkFuzzySearch::testFloatGreater, this, placeholders::_1, placeholders::_2);
		}
		else
		{
			testFunction_Float = bind(&ChunkFuzzySearch::testFloatLess, this, placeholders::_1, placeholders::_2);
		}
	}

}

void ChunkFuzzySearch::digest(char *newMemory, char *prevMemory, unsigned long length, unsigned long address)
{
	unsigned long i;
	char signage;
	Variant variant;
	if (valueByteLength == 1) //we are a character search, we good
	{
		for (i=0; i<length; i++)
		{
			DO_COMPARE(char, variant, &newMemory[i], &prevMemory[i], address+i, address);
		}
	}
	else if (valueByteLength == 2)
	{
		if (first)
		{
			first = false;
		}
		else
		{
			newBytes[1] = newMemory[0];
			prevBytes[1] = prevMemory[0];
			short tmp1 = BSWAP16(variant.convertToShort((char*)&newBytes[0]));
			short tmp2 = BSWAP16(variant.convertToShort((char*)&prevBytes[0]));
			DO_COMPARE(short, variant, &tmp1, &tmp2, address-1, prevAddress);
		}
		for (i=0; i<length-1; i++)
		{
			short tmp1 = BSWAP16(variant.convertToShort((char*)&newMemory[i]));
			short tmp2 = BSWAP16(variant.convertToShort((char*)&prevMemory[i]));
			DO_COMPARE(short, variant, &tmp1, &tmp2, address+i, address);
		}
		newBytes[0] = newMemory[length-1];
		prevBytes[0] = prevMemory[length-1];
	}
	else if (valueByteLength == 4)
	{
		if (first)
		{
			first = false;
		}
		else
		{
			newBytes[3] = newMemory[0];
			newBytes[4] = newMemory[1];
			newBytes[5] = newMemory[2];
			prevBytes[3] = prevMemory[0];
			prevBytes[4] = prevMemory[1];
			prevBytes[5] = prevMemory[2];
			for (int j=0; j<3; j++)
			{
				long tmp1 = BSWAP32(variant.convertToLong((char*)&newBytes[j]));
				long tmp2 = BSWAP32(variant.convertToLong((char*)&prevBytes[j]));
				if (isFloat)
				{
					DO_COMPARE_FLOAT(variant, &tmp1, &tmp2, address-3+j, prevAddress);
				}
				else
				{
					DO_COMPARE(long, variant, &tmp1, &tmp2, address-3+j, prevAddress);
				}
			}
		}
		for (i=0; i<length-3; i++)
		{
			long tmp1 = BSWAP32(variant.convertToLong((char*)&newMemory[i]));
			long tmp2 = BSWAP32(variant.convertToLong((char*)&prevMemory[i]));
			if (isFloat)
			{
				DO_COMPARE_FLOAT(variant, &tmp1, &tmp2, address+i, address);
			}
			else
			{
				DO_COMPARE(long, variant, &tmp1, &tmp2, address+i, address);
			}
		}
		newBytes[0] = newMemory[length-3];
		newBytes[1] = newMemory[length-2];
		newBytes[2] = newMemory[length-1];
		prevBytes[0] = prevMemory[length-3];
		prevBytes[1] = prevMemory[length-2];
		prevBytes[2] = prevMemory[length-1];

	}
	prevAddress = address;
}

bool ChunkFuzzySearch::digestValue(char* memory, AddressItem item, unsigned long section)
{
	char signage;
	Variant variant;
	if (valueByteLength == 1)
	{
		DO_COMPARE_NEXT(char, section, variant, &memory[0], item);
	}
	else if (valueByteLength == 2)
	{
		short tmp = BSWAP16(variant.convertToShort((char*)(&memory[0])));
		DO_COMPARE_NEXT(short, section, variant, &tmp, item);
	}
	else if (valueByteLength == 4 && isFloat)
	{
		long tmp = BSWAP32(variant.convertToLong((char*)(&memory[0])));
		DO_COMPARE_FLOAT_NEXT(section, variant, &tmp, item);
	}
	else
	{
		long tmp = BSWAP32(variant.convertToLong((char*)(&memory[0])));
		DO_COMPARE_NEXT(long, section, variant, &tmp, item);
	}

	return (signage != TEST_SIGN_NULL);
}

template<typename T>
char ChunkFuzzySearch::test(T currentVal, T prevVal)
{
	switch (comparison)
	{
	case SEARCH_FUZZY_EQUAL:
		return currentVal == (T)prevVal;
		break;
	case SEARCH_FUZZY_NOTEQUAL:
		return currentVal != (T)prevVal;
		break;
	case SEARCH_FUZZY_GREATER:
		return currentVal > (T)prevVal;
		break;
	case SEARCH_FUZZY_LESS:
		return currentVal < (T)prevVal;
		break;
	default: return false;
	}
}
