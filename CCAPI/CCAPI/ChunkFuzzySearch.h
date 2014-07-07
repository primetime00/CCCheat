#ifndef _CHUNKFUZZYSEARCH_
#define _CHUNKFUZZYSEARCH_

#include <functional>
#include "Common.h"

class ChunkFuzzySearch
{
public:
	ChunkFuzzySearch():resultRef(0) { first = true; isFloat = false; prevAddress = 0;}
	~ChunkFuzzySearch() {}

	void setup(char len, char comp);
	void digest(char *newMemory, char *prevMemory, unsigned long length, unsigned long address);
	bool digestValue(char* memory, AddressItem item, unsigned long section);
	void digestValue(char* memory, unsigned long prevVal, unsigned long address, char sign = TEST_SIGN_BOTH);
	void clear() { first = true; prevAddress = 0; }

	void setResultReference(ResultList *ref) { resultRef = ref; }

	std::function<bool(long long, long long)> testFunction_Signed;
	std::function<bool(long long, long long)> testFunction_Unsigned;
	std::function<bool(float, float)> testFunction_Float;

	template<typename T>
	bool testEqual(T currentVal, T prevVal) { return currentVal == prevVal; }

	template<typename T>
	bool testNotEqual(T currentVal, T prevVal) { return currentVal != prevVal; }

	template<typename T>
	bool testGreater(T currentVal, T prevVal) { return currentVal > prevVal; }

	template<typename T>
	bool testLess(T currentVal, T prevVal) { return currentVal < prevVal; }

	bool testFloatEqual(float currentVal, float prevVal) { return currentVal == *(float*)&prevVal; }

	bool testFloatNotEqual(float currentVal, float prevVal) { return  (long)(currentVal*1000) != (long)((*(float*)&prevVal)*1000); }

	bool testFloatGreater(float currentVal, float prevVal) { return (long)(currentVal*1000) > (long) ((*(float*)&prevVal)*1000); }

	bool testFloatLess(float currentVal, float prevVal) { return (long)(currentVal*1000) < (long)((*(float*)&prevVal)*1000); }


private:
	template<typename T>
	char test(T currentVal, T prevVal);

	char valueByteLength;
	char comparison;

	unsigned long newBytes[6];
	unsigned long prevBytes[6];
	bool first;
	bool isFloat;

	ResultList *resultRef;
	unsigned long prevAddress;

};

#endif
