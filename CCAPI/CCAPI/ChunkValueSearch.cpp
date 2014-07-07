#include "ChunkValueSearch.h"

using namespace std;

void ChunkValueSearch::setup(char len, char comp, long long val)
{
	switch (len)
	{
	case SEARCH_VALUE_TYPE_1BYTE: valueByteLength = 1; isUnsigned = (val > 0x7F); break;
	case SEARCH_VALUE_TYPE_2BYTE: valueByteLength = 2; isUnsigned = (val > 0x7FFF); break;
	case SEARCH_VALUE_TYPE_4BYTE: valueByteLength = 4; isUnsigned = (val > 0x7FFFFFFF); break;
	case SEARCH_VALUE_TYPE_FLOAT: valueByteLength = 4; isFloat = true; break;
	default: valueByteLength = 4; break;
	}
	comparison = comp;
	value = val;
	if (valueByteLength == 1)
	{
		if (comparison == SEARCH_VALUE_EXACT)
		{
			testFunction_Signed = bind(&ChunkValueSearch::testEqual<char>, this, placeholders::_1);
			testFunction_Unsigned = bind(&ChunkValueSearch::testEqual<unsigned char>, this, placeholders::_1);
		}
		else if (comparison == SEARCH_VALUE_NOTEXACT)
		{
			testFunction_Signed = bind(&ChunkValueSearch::testNotEqual<char>, this, placeholders::_1);
			testFunction_Unsigned = bind(&ChunkValueSearch::testNotEqual<unsigned char>, this, placeholders::_1);
		}
		else if (comparison == SEARCH_VALUE_GREATER)
		{
			testFunction_Signed = bind(&ChunkValueSearch::testGreater<char>, this, placeholders::_1);
			testFunction_Unsigned = bind(&ChunkValueSearch::testGreater<unsigned char>, this, placeholders::_1);
		}
		else
		{
			testFunction_Signed = bind(&ChunkValueSearch::testLess<char>, this, placeholders::_1);
			testFunction_Unsigned = bind(&ChunkValueSearch::testLess<unsigned char>, this, placeholders::_1);
		}
	}
	else if (valueByteLength == 2)
	{
		if (comparison == SEARCH_VALUE_EXACT)
		{
			testFunction_Signed = bind(&ChunkValueSearch::testEqual<short>, this, placeholders::_1);
			testFunction_Unsigned = bind(&ChunkValueSearch::testEqual<unsigned short>, this, placeholders::_1);
		}
		else if (comparison == SEARCH_VALUE_NOTEXACT)
		{
			testFunction_Signed = bind(&ChunkValueSearch::testNotEqual<short>, this, placeholders::_1);
			testFunction_Unsigned = bind(&ChunkValueSearch::testNotEqual<unsigned short>, this, placeholders::_1);
		}
		else if (comparison == SEARCH_VALUE_GREATER)
		{
			testFunction_Signed = bind(&ChunkValueSearch::testGreater<short>, this, placeholders::_1);
			testFunction_Unsigned = bind(&ChunkValueSearch::testGreater<unsigned short>, this, placeholders::_1);
		}
		else
		{
			testFunction_Signed = bind(&ChunkValueSearch::testLess<short>, this, placeholders::_1);
			testFunction_Unsigned = bind(&ChunkValueSearch::testLess<unsigned short>, this, placeholders::_1);
		}
	}
	else if (valueByteLength == 4 && !isFloat)
	{
		if (comparison == SEARCH_VALUE_EXACT)
		{
			testFunction_Signed = bind(&ChunkValueSearch::testEqual<long>, this, placeholders::_1);
			testFunction_Unsigned = bind(&ChunkValueSearch::testEqual<unsigned long>, this, placeholders::_1);
		}
		else if (comparison == SEARCH_VALUE_NOTEXACT)
		{
			testFunction_Signed = bind(&ChunkValueSearch::testNotEqual<long>, this, placeholders::_1);
			testFunction_Unsigned = bind(&ChunkValueSearch::testNotEqual<unsigned long>, this, placeholders::_1);
		}
		else if (comparison == SEARCH_VALUE_GREATER)
		{
			testFunction_Signed = bind(&ChunkValueSearch::testGreater<long>, this, placeholders::_1);
			testFunction_Unsigned = bind(&ChunkValueSearch::testGreater<unsigned long>, this, placeholders::_1);
		}
		else
		{
			testFunction_Signed = bind(&ChunkValueSearch::testLess<long>, this, placeholders::_1);
			testFunction_Unsigned = bind(&ChunkValueSearch::testLess<unsigned long>, this, placeholders::_1);
		}
	}
	else
	{
		if (comparison == SEARCH_VALUE_EXACT)
		{
			testFunction_Float = bind(&ChunkValueSearch::testFloatEqual, this, placeholders::_1);
		}
		else if (comparison == SEARCH_VALUE_NOTEXACT)
		{
			testFunction_Float = bind(&ChunkValueSearch::testFloatNotEqual, this, placeholders::_1);
		}
		else if (comparison == SEARCH_VALUE_GREATER)
		{
			testFunction_Float = bind(&ChunkValueSearch::testFloatGreater, this, placeholders::_1);
		}
		else
		{
			testFunction_Float = bind(&ChunkValueSearch::testFloatLess, this, placeholders::_1);
		}
	}
}

#define DO_COMPARE_FLOAT(vari, val, loc, base)\
	if (testFunction_Float(vari.convertToFloat((char*)val)))\
	{\
		(*resultRef)[base].push_back(make_shared<AddressObj>(loc, (unsigned long)vari.convertToLong((char*)val), TEST_SIGN_UNKNOWN));\
	}

#define DO_COMPARE_FLOAT_NEXT(section, vari, val, item)\
	signage = TEST_SIGN_NULL;\
	if (testFunction_Float(vari.convertToFloat((char*)val)))\
	{\
		signage = TEST_SIGN_UNKNOWN;\
		(item->value) = (unsigned long) vari.convertToLong((char*)val);\
		(item->sign) = signage;\
	}

#define CMP(_type, val, loc) (testFunction_Signed(*((_type*)val)))
#define CMP_UNSIGNED(_type, val, loc) (testFunction_Unsigned(*((unsigned _type*)val)))
#define DO_COMPARE(_type, val, loc, base)\
	signage = TEST_SIGN_NULL;\
	if (CMP(_type, val, loc))\
		signage |= TEST_SIGN_YES;\
	if (CMP_UNSIGNED(_type, val, loc))\
		signage |= TEST_SIGN_NO;\
	if (signage != TEST_SIGN_NULL)\
	{\
		(*resultRef)[base].push_back(make_shared<AddressObj>(loc, (_type) *((_type*)val), signage));\
	}

#define DO_COMPARE_NEXT(_type, section, val, item)\
	signage = TEST_SIGN_NULL;\
	if ((item->sign) & TEST_SIGN_YES)\
	{\
		if (CMP(_type, val, (item->address)))\
			signage |= TEST_SIGN_YES;\
	}\
	if ((item->sign) & TEST_SIGN_NO)\
	{\
		if (CMP_UNSIGNED(_type, val, (item->address)))\
			signage |= TEST_SIGN_NO;\
	}\
	if (signage != TEST_SIGN_NULL)\
	{\
		(item->value) = (_type) *((_type*)val);\
		(item->sign) = signage;\
	}

//used for initial value search against PS3 memory
void ChunkValueSearch::digest(char *memory, unsigned long length, unsigned long address)
{
	unsigned long i;
	char signage;
	Variant variant;
	if (valueByteLength == 1) //we are a character search, we good
	{
		for (i=0; i<length; i++)
		{
			DO_COMPARE(char, &memory[i], address+i, address);
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
			prev[1] = memory[0];
			short tmp = BSWAP16(variant.convertToShort((char*)&prev[0]));	
			DO_COMPARE(short, &tmp, address-1, prevAddress);
		}
		for (i=0; i<length-1; i++)
		{
			short tmp = BSWAP16(variant.convertToShort(&memory[i]));
			DO_COMPARE(short, &tmp, address+i, address);
		}
		prev[0] = memory[length-1];
	}
	else if (valueByteLength == 4)
	{
		if (first)
		{
			first = false;
		}
		else
		{
			prev[3] = memory[0];
			prev[4] = memory[1];
			prev[5] = memory[2];
			for (int j=0; j<3; j++)
			{
				long tmp = BSWAP32(variant.convertToLong((char*)&prev[j]));
				if (isFloat)
				{
					DO_COMPARE_FLOAT(variant, &tmp, address-3+j, prevAddress);
				}
				else
				{
					DO_COMPARE(long, &tmp, address-3+j, prevAddress);
				}
			}
		}
		for (i=0; i<length-3; i++)
		{
			long tmp = BSWAP32(variant.convertToLong(&memory[i]));
			if (isFloat)
			{
				DO_COMPARE_FLOAT(variant, &tmp, address+i, address);
			}
			else
			{
				DO_COMPARE(long, &tmp, address+i, address);
			}
		}
		prev[0] = memory[length-3];
		prev[1] = memory[length-2];
		prev[2] = memory[length-1];
	}
	prevAddress = address;
}


//used for continuing value search against a result file
bool ChunkValueSearch::digestValue(char *memory, AddressItem item, unsigned long section)
{
	char signage;
	Variant variant;
	if (valueByteLength == 1)
	{
		DO_COMPARE_NEXT(char, section, &memory[0], item);
	}
	else if (valueByteLength == 2)
	{
		short tmp = BSWAP16(*(short*)&memory[0]);
		DO_COMPARE_NEXT(short, section, &tmp, item);
	}
	else if (valueByteLength == 4 && isFloat)
	{
		long tmp = BSWAP32(*(long*)&memory[0]);
		DO_COMPARE_FLOAT_NEXT(section, variant, &tmp, item);
	}
	else
	{
		long tmp = BSWAP32(*(long*)&memory[0]);
		DO_COMPARE_NEXT(long, section, &tmp, item);
	}
	return signage != TEST_SIGN_NULL;
}

template<typename T>
bool ChunkValueSearch::test(T val)
{
	switch (comparison)
	{
	case SEARCH_VALUE_EXACT:
		//return val == *((T*)&value);
		return val == value;
		break;
	case SEARCH_VALUE_NOTEXACT:
		//return val != *((T*)&value);
		return val != value;
		break;
	case SEARCH_VALUE_GREATER:
		//return val > *((T*)&value);
		return val > value;
		break;
	case SEARCH_VALUE_LESS:
		//return val < *((T*)&value);
		return val < value;
		break;
	default: return false;
	}
}
