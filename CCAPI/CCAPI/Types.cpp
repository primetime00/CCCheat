#include "Common.h"
#include "Types.h"
#include <cstring>

using namespace std;

PointerObj::PointerObj(AddressOffsets b)
{ 
	pointers = b; updateCount = 0; 
	resolved = 0;
}

PointerObj::PointerObj(const unsigned long address, const PointerOffsets &pt) 
{ 
	fromPointerOffsets(address, pt); updateCount =0;
	resolved = 0;
}

PointerOffsets PointerObj::getOffsets()
{
	PointerOffsets po;
	for (auto it = pointers.begin(); it != pointers.end(); ++it)
	{
		po.push_back(it->offset);
	}
	return po;
}
void PointerObj::fromPointerOffsets(const unsigned long address, const PointerOffsets &pt) 
{ 
	pointers.clear();
	for (auto it = pt.begin(); it!= pt.end(); ++ it)
	{
		pointers.push_back(AddressOffset(0, *it));
	}
	if (pointers.size() > 0) pointers.front().address = address;
}

bool PointerObj::equal(PointerObj p) {
	auto pstart = p.pointers.begin();
	auto start = pointers.begin();
	//if (p.getBase() != getBase()) return false;
	if (p.pointers.size() != pointers.size()) return false;
	while (start != pointers.end())
	{
		if (start->offset != pstart->offset) return false;
		++start; ++pstart;
	}
	return true;
}
void PointerObj::update() 
{ 
	updateCount++; updateCount %= 0xFFFFFFF; 
}

AddressObj::AddressObj(unsigned long a, unsigned long v, char s) 
{ 
	init(a, v, SEARCH_VALUE_TYPE_4BYTE, s);
} 

AddressObj::AddressObj()
{ 
	init(0,0,SEARCH_VALUE_TYPE_4BYTE,0);
}

AddressObj::AddressObj(unsigned long a, unsigned long v, char t, char s) 
{ 
	init(a,v,t,s);
}
AddressObj::AddressObj(unsigned long a, PointerOffsets p, char t, char s) 
{ 
	address = a; type = t; sign = s; 
	pointer = make_shared<PointerObj>(a, p);
}

AddressObj::AddressObj(unsigned long a, PointerOffsets p, unsigned long v, char t, char s)
{
	address = a; type = t; sign = s; 
	pointer = make_shared<PointerObj>(a, p);
	value = store = v;
}

AddressObj::AddressObj(unsigned long a, PointerItem p, char t, char s) 
{ 
	address = a; type = t; sign = s;
	if (p)
		pointer = make_shared<PointerObj>(a, p->getOffsets());
	else
		pointer = nullptr;
}


AddressObj::AddressObj(AddressObj &item) 
{ 
	address = 0;
	pointer = nullptr;
	sign = false;
	type = 0;
	value = 0;
	*this = item;
}

void AddressObj::init(unsigned long a, unsigned long v, char t, char s)
{
	address = a; value = v; type = t; sign = s; store = v;
}
AddressObj & AddressObj::operator=(const AddressObj& t) 
{
	address = t.address; value = t.value; sign = t.sign; type = t.type; store = t.store;
	if (t.pointer != nullptr)
	{
		pointer = make_shared<PointerObj>(address, t.pointer->getOffsets());
	}
	return *this;
}

bool AddressObj::operator==(const AddressObj &t)
{
	if (t.address == address)
	{
		if ( t.pointer && pointer )
		{
			auto toff = t.pointer->getOffsets();
			auto off = pointer->getOffsets();
			if (toff.size() != off.size())
				return false;
			auto j = off.begin();
			for (auto i=toff.begin(); i != toff.end(); ++i, ++j)
			{
				if (*i != *j)
					return false;
			}
			return true;
		}
		else if (!t.pointer && !pointer)
			return true;
	}
	return false;
}

bool AddressObj::operator!=(const AddressObj &t)
{
	return !(*this == t);
}


int AddressObj::read(char *buf)
{
	int pos = 0;
	unsigned int offset;
	memcpy(&address, &buf[pos], sizeof(unsigned long)); pos += sizeof(unsigned long);
	memcpy(&value, &buf[pos], sizeof(long long)); pos += sizeof(long long);
	store = value;
	type = buf[pos]; pos += 1;
	sign = 0;
	int NumOfPointers = buf[pos]; pos += 1;
	if (NumOfPointers > 0)
	{
		PointerOffsets pt;
		for (int i=0; i < NumOfPointers; ++i)
		{
			memcpy(&offset, &buf[pos], sizeof(unsigned int)); pos += sizeof(unsigned int);
			pt.push_back(offset);
		}
		pointer = make_shared<PointerObj>(address, pt);
	}
	return pos;
}

int AddressObj::write(char *buf)
{
	int pos = 0;
	unsigned int offset;
	memcpy(&buf[pos], &address, sizeof(unsigned long)); pos += sizeof(unsigned long);
	memcpy(&buf[pos], &store, sizeof(long long)); pos += sizeof(long long);
	buf[pos] = type; pos += 1;
	buf[pos] = pointer ? pointer->getOffsets().size() : 0; pos += 1;
	if (pointer && pointer->getOffsets().size() > 0)
	{
		PointerOffsets pt = pointer->getOffsets();
		for (auto i = pt.begin(); i != pt.end(); ++i)
		{
			unsigned int v = *i;
			memcpy(&buf[pos], &v, sizeof(unsigned int)); pos += sizeof(unsigned int);
		}
	}
	return pos;
}

void AddressObj::debug()
{
	cout << "Address Obj Info:" << endl;
	cout << "Base Address: " << hex << address << endl;
	cout << "Value: " << dec << value << endl;
	cout << "Store: " << dec << store << endl;
	cout << "Type: " << dec << (int)type <<endl;
	cout << "Pointer?: " << (isPointer() ? "Yes" : "No") << endl;
	if (isPointer())
	{
		auto pt = pointer->getOffsets();
		int p = 0;
		for (auto i = pt.begin(); i != pt.end(); ++i, ++p)
		{
			cout << "Pointer Offset ["<<p<<"]: " << hex << *i << endl;
		}
	}
}

Variant::Variant(float v) 
{
	memcpy((char*)&value, (char*)&v, sizeof(float));
	pt_obj.cPt = (char*)&value;
}
Variant::Variant(char v) 
{
	memcpy((char*)&value, (char*)&v, sizeof(char));
	pt_obj.cPt = (char*)&value;
}
Variant::Variant(short v) 
{
	memcpy((char*)&value, (char*)&v, sizeof(short));
	pt_obj.cPt = (char*)&value;
}
Variant::Variant(long v) 
{
	memcpy((char*)&value, (char*)&v, sizeof(long));
	pt_obj.cPt = (char*)&value;
}
Variant::Variant(long long v) 
{
	memcpy((char*)&value, (char*)&v, sizeof(long long));
	pt_obj.cPt = (char*)&value;
}
