#ifndef _COMMON_
#define _COMMON_

#include <map>
#include <vector>
#include <utility>
#include <tuple>
#include <list>
#include <memory>

using namespace std;

#if defined(_WIN32) || defined(WIN32)
#define BSWAP32(x) _byteswap_ulong(x)
#define BSWAP16(x) _byteswap_ushort(x)
#else
#define BSWAP32(x) __bswap_32(x)
#define BSWAP16(x) __bswap_16(x)
#endif


#define TEST_TYPE_FIND_RANGE	0x00
#define TEST_TYPE_FIND_SEARCH	0x01

#define TEST_SIGN_YES		0x01
#define TEST_SIGN_NO		0x02
#define TEST_SIGN_BOTH		0x03
#define TEST_SIGN_UNKNOWN	0x08
#define TEST_SIGN_NULL		0x00

#define SEARCH_TYPE_VALUE	0x00
#define SEARCH_TYPE_FUZZY	0x01
#define SEARCH_TYPE_UNKNOWN	0x02

#define SEARCH_VALUE_TYPE_1BYTE		0x00
#define SEARCH_VALUE_TYPE_2BYTE		0x01
#define SEARCH_VALUE_TYPE_4BYTE		0x02
#define SEARCH_VALUE_TYPE_FLOAT		0x03

#define SEARCH_STATE_INITIAL	0x00
#define SEARCH_STATE_CONTINUE	0x01

#define SEARCH_FUZZY_EQUAL		0x00
#define SEARCH_FUZZY_NOTEQUAL	0x01
#define SEARCH_FUZZY_GREATER	0x02
#define SEARCH_FUZZY_LESS		0x03
#define SEARCH_VALUE_EXACT		0x04
#define SEARCH_VALUE_NOTEXACT	0x05
#define SEARCH_VALUE_GREATER	0x06
#define SEARCH_VALUE_LESS		0x07
#define SEARCH_FUZZY_INIT		0x08

#define RESULT_FILE_TYPE_DUMP	0x00
#define RESULT_FILE_TYPE_LIST	0x01

#define MEMORY_COMMAND_READ		0x00
#define MEMORY_COMMAND_WRITE	0x01


struct AddressItem
{
	unsigned long address;
	unsigned long value;
	char sign;
	AddressItem(unsigned long a, unsigned long v, char s) { address = a; value = v; sign = s; }
	AddressItem &operator=(AddressItem t) { address = t.address; value = t.value; sign = t.sign; return *this;}
};

//typedef tuple<unsigned long, unsigned long, char> AddressItem;
typedef vector<AddressItem> AddressList;
typedef vector<AddressItem*> AddressListPtr;
typedef map<unsigned long, AddressList> ResultList;
typedef map<unsigned long, ResultList*> ThreadResultList;

typedef pair<long long, long long> RangePair;
typedef vector<RangePair> RangeList;

struct MemoryReadItem{
	unsigned int address;
	char type;
	char *memory;
	bool keep;
	MemoryReadItem(unsigned int a, char t, char *m, bool k) { address = a; type = t; memory = m; keep = k;}
};
struct MemoryWriteItem{
	unsigned int address;
	long long value;
	char type;
	bool freeze;
	MemoryWriteItem(unsigned int a, long long v, char t, bool f) { address = a; value = v; type = t; freeze = f;}
};

typedef list<shared_ptr<MemoryReadItem>> MemoryReadSet;


typedef map<unsigned long, MemoryReadSet> MemoryReadItemList;
typedef map<unsigned long, shared_ptr<MemoryWriteItem>> MemoryWriteItemList;

#endif