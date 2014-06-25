#ifndef _ADDRESSOFFSETGROUP_
#define _ADDRESSOFFSETGROUP_

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include "Common.h"
#include "ValueInput.h"

using namespace std;


class AddressOffsetGroup : public Fl_Group {
public:
	AddressOffsetGroup(int X, int Y, int W, int H, const char *l=0);
	~AddressOffsetGroup() { }

	void set(unsigned long address, unsigned int offset);
	void setOffsetOnly(unsigned int offset);
	void reset();
	void setAddressEditable(bool edit) { m_addressEditable = edit; }

	void setRefresh(bool refresh);

	unsigned long getAddress() { return m_address->getULValue(); }
	unsigned int getOffset() { return (unsigned int)m_offset->getULValue(); }
  
protected:
	void draw();

private:
	ValueInput *m_address;
	ValueInput *m_offset;
	bool m_addressEditable;
	bool m_needsRefresh;
};

#endif