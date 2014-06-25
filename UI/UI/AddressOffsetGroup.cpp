#include "AddressOffsetGroup.h"
#include <CCAPI/Common.h>

using namespace std;


AddressOffsetGroup::AddressOffsetGroup(int X, int Y, int W, int H, const char *l) : Fl_Group(X,Y,W,H,l)
{
	const int pad = 10;
	const int space = 60;
	const int labelSpace = 50;
	int addressX = X+pad+labelSpace;
	float addresBoxWidth = (W - pad*2 - space - labelSpace*2) * 0.6f;
	int offsetX = addressX + addresBoxWidth + space + labelSpace;
	float offsetBoxWidth = (W - pad*2 - space - labelSpace*2) * 0.4f;
	const int regHeight = 25;
	const int regY = Y+(int)((H - regHeight)/2.0f);

	begin();
	m_address = new ValueInput(addressX, regY, addresBoxWidth, regHeight, "Address:");
	m_offset = new ValueInput(offsetX, regY, offsetBoxWidth, regHeight, "Offset:");
	end();
	m_address->setHex(true);
	m_offset->setHex(true);
	m_address->setValType(SEARCH_VALUE_TYPE_4BYTE);
	m_offset->setValType(SEARCH_VALUE_TYPE_2BYTE);
	reset();
}

void AddressOffsetGroup::set(unsigned long address, unsigned int offset)
{
	m_address->setValue(address);
	m_offset->setValue((unsigned long)offset);
	show();
}

void AddressOffsetGroup::setOffsetOnly(unsigned int offset)
{
	setRefresh(true);
	m_address->deactivate();
	m_offset->setValue((unsigned long)offset);
	show();
}
  
void AddressOffsetGroup::reset()
{
	m_address->setValue((unsigned long)0);
	m_offset->setValue((unsigned long)0);
	m_address->value("");
	m_offset->value("");
	m_address->activate();
	m_addressEditable = false;
	setRefresh(true);
	hide();
}

void AddressOffsetGroup::draw()
{
	if (!m_addressEditable && !m_needsRefresh)
	{
		m_address->setValue((unsigned long)m_address->getULValue());
	}
	else if (!m_addressEditable)
	{
		m_address->setValue((unsigned long)0);
		m_address->value("Refresh");
	}
	Fl_Group::draw();
}

void AddressOffsetGroup::setRefresh(bool refresh)
{
	m_needsRefresh = refresh;
}