#include "Common.h"
#include "Types.h"
#include <cstring>

using namespace std;

PointerObj::PointerObj(AddressOffsets b)
{ 
	pointers = b; updateCount = 0; 
}

PointerObj::PointerObj(const unsigned long address, const PointerOffsets &pt) 
{ 
	fromPointerOffsets(address, pt); updateCount =0;
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

void AddressObj::init(unsigned long a, unsigned long v, char t, char s)
{
	address = a; value = v; type = t; sign = s; 
}
AddressObj & AddressObj::operator=(AddressObj t) 
{
	address = t.address; value = t.value; sign = t.sign; type = t.type; 
	if (pointer != nullptr)
	{
		pointer = make_shared<PointerObj>(address, t.pointer->getOffsets());
	}
	return *this;
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
