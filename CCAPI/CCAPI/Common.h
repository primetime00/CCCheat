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
#define SEARCH_VALUE_TYPE_PT_1BYTE	0x04
#define SEARCH_VALUE_TYPE_PT_2BYTE	0x05
#define SEARCH_VALUE_TYPE_PT_4BYTE	0x06
#define SEARCH_VALUE_TYPE_PT_FLOAT	0x07

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
#define SEARCH_POINTER			0x09

#define RESULT_FILE_TYPE_DUMP	0x00
#define RESULT_FILE_TYPE_LIST	0x01

#define MEMORY_COMMAND_READ			0x00
#define MEMORY_COMMAND_WRITE		0x01
#define MEMORY_COMMAND_READCHUNK	0x02


struct AddressItem
{
	unsigned long address;
	unsigned long value;
	char sign;
	char type;
	AddressItem() { address = 0; value = 0; sign = 0; }
	AddressItem(unsigned long a, unsigned long v, char t, char s) { address = a; value = v; type = t; sign = s; }
	AddressItem(unsigned long a, unsigned long v, char s) { address = a; value = v; type = SEARCH_VALUE_TYPE_4BYTE; sign = s; }
	AddressItem &operator=(AddressItem t) { address = t.address; value = t.value; sign = t.sign; type = t.type; return *this;}
};


struct AddressOffset
{
	unsigned long address;
	unsigned int offset;
	AddressOffset(unsigned long a, unsigned int o) { address = a; offset = o; }
};

typedef list<unsigned int> PointerOffsets;
typedef list<AddressOffset> AddressOffsets;

struct PointerObj
{
	AddressOffsets pointers;	

	AddressItem m_address;

	unsigned long getBase() { return (pointers.size() > 0) ? pointers.front().address : 0; }
	unsigned int updateCount;
	void fromPointerOffsets(const unsigned long address, const PointerOffsets &pt) { 
		pointers.clear();
		for (auto it = pt.begin(); it!= pt.end(); ++ it)
		{
			pointers.push_back(AddressOffset(0, *it));
		}
		if (pointers.size() > 0) pointers.front().address = address;
	}
	bool equal(PointerObj p) {
		auto pstart = p.pointers.begin();
		auto start = pointers.begin();
		if (p.getBase() != getBase()) return false;
		if (p.pointers.size() != pointers.size()) return false;
		while (start != pointers.end())
		{
			if (start->offset != pstart->offset) return false;
			++start; ++pstart;
		}
		return true;
	}
	void update() { updateCount++; updateCount %= 0xFFFFFFF; }
	PointerObj(AddressOffsets b) { pointers = b; m_address.address = 0; updateCount = 0; m_address.type = SEARCH_VALUE_TYPE_4BYTE; }
	PointerObj(const unsigned long address, const PointerOffsets &pt) { fromPointerOffsets(address, pt); updateCount =0; m_address.type = SEARCH_VALUE_TYPE_4BYTE;}
};
typedef shared_ptr<PointerObj> PointerItem;

struct DumpHeader
{
	unsigned long begin, end;
	char misc;
	DumpHeader(unsigned long b, unsigned long e, char m) { begin = b; end = e; misc = m; }
	DumpHeader() { begin = 0; end = 0; misc = 0; }
	DumpHeader &operator=(DumpHeader t) { begin = t.begin; end = t.end; misc = t.misc; return *this;}
};

struct _DumpData
{
	DumpHeader header;
	char *data;
	_DumpData() { data=0;}
	_DumpData(DumpHeader h, char *d) { header = h, data = d;}
	_DumpData(_DumpData &s) { data = s.data; header = s.header; }
	~_DumpData() { if (data !=0) delete[] data; data=0; }
};
typedef shared_ptr<_DumpData> DumpData;
typedef vector<DumpData> DumpDataList;

typedef vector<AddressItem> AddressList;
typedef map<unsigned long, AddressList> ResultList;

//typedef pair<long long, long long> RangePair;
struct RangePair
{
	unsigned long m_first, m_second;
	RangePair(unsigned long first, unsigned long second) { set(first,second); }
	RangePair() { set(-1,-1); }
	void set(unsigned long first, unsigned long second) { m_first = first; m_second = second; }
	unsigned long first() { return m_first; }
	unsigned long second() { return m_second; }
	void first(unsigned v) { m_first = v; }
	void second(unsigned v) { m_second = v; }
	RangePair& operator=(RangePair p) { m_first = p.m_first; m_second = p.m_second; return *this;}
};
typedef vector<RangePair> RangeList;

struct MemoryReadItem{
	unsigned int address;
	char type;
	char *memory;
	bool keep;
	MemoryReadItem(unsigned int a, char t, char *m, bool k) { address = a; type = t; memory = m; keep = k;}
};

struct MemoryChunkReadItem{
	unsigned int address;
	unsigned long length;
	char *memory;
	bool keep;
	MemoryChunkReadItem(unsigned int a, unsigned long l, char *m, bool k) { address = a; length = l; memory = m; keep = k;}
};

struct PointerReadItem{
	PointerItem pointer;
	bool keep;
	PointerReadItem(PointerItem p, bool k) { pointer = p; keep = k;}
};


struct MemoryWriteItem{
	unsigned int address;
	long long value;
	char type;
	bool freeze;
	MemoryWriteItem(unsigned int a, long long v, char t, bool f) { address = a; value = v; type = t; freeze = f;}
};

typedef list<shared_ptr<MemoryReadItem>> MemoryReadSet;

typedef list<shared_ptr<MemoryChunkReadItem>> MemoryChunkReadSet;

typedef list<shared_ptr<PointerReadItem>> PointerReadSet;


typedef map<unsigned long, MemoryReadSet> MemoryReadItemList;
typedef map<unsigned long, MemoryChunkReadSet> MemoryChunkReadItemList;
typedef map<unsigned long, PointerReadSet> PointerReadItemList;
typedef map<unsigned long, shared_ptr<MemoryWriteItem>> MemoryWriteItemList;

#endif
