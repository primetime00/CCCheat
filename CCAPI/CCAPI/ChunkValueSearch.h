#ifndef _CHUNKVALUESEARCH_
#define _CHUNKVALUESEARCH_

#include <functional>
#include "Helpers.h"
#include "Common.h"

using namespace std;


class ChunkValueSearch
{
public:
	ChunkValueSearch():resultRef(0) { first = true; isFloat = false; prevAddress = 0; isUnsigned = true;}
	~ChunkValueSearch() {}

	void setup(char len, char comp, long long val);
	void digest(char *memory, unsigned long length, unsigned long address);
	bool digestValue(char* memory, AddressItem item, unsigned long section);
	void clear() { first = true; prevAddress = 0;}
	void setResultReference(ResultList *ref) { resultRef = ref; }
	long long getValue() { return value; }

	std::function<bool(long long)> testFunction_Signed;
	std::function<bool(long long)> testFunction_Unsigned;
	std::function<bool(float)> testFunction_Float;

private:
	template<typename T>
	bool test(T val);

	template<typename T>
	bool testEqual(T val) { return val == (T)value; }

	template<typename T>
	bool testNotEqual(T val) { return val != (T)value; }

	template<typename T>
	bool testGreater(T val) { return val > (T)value; }

	template<typename T>
	bool testLess(T val) { return val < (T)value; }

	bool testFloatEqual(float val) { return val == m_variant.convertToFloat((char*)&value); }

	bool testFloatNotEqual(float val) { return val != m_variant.convertToFloat((char*)&value); }

	bool testFloatGreater(float val) { return val > m_variant.convertToFloat((char*)&value); }

	bool testFloatLess(float val) { return val < m_variant.convertToFloat((char*)&value); }



	char valueByteLength;
	Variant m_variant;
	unsigned char comparison;
	long long value;
	bool isUnsigned;

	unsigned long prev[6];
	bool first;
	bool isFloat;

	ResultList *resultRef;
	unsigned long prevAddress;

};

#endif
