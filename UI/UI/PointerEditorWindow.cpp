#include "PointerEditorWindow.h"
#include "PointerScannerTable.h"
#include "DumpManager.h"
#include "ui_main.h"
#include <CCAPI/Helpers.h>
using namespace std;

void PointerEditorWindow::OnOKCB(Fl_Widget *w)
{
	readPointers();
	hide();
}

void PointerEditorWindow::OnCancelCB(Fl_Widget *w)
{
	currentPointer = nullptr;
	hide();

}

void PointerEditorWindow::OnAddCB(Fl_Widget *w)
{
	bool found = false;
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		if (!(*it)->visible())
		{
			found = true;
			(*it)->setOffsetOnly(0);
			(*it)->show();
			if ((*it) == groups.back())
				uiInstance->m_peAddPointerButton->deactivate();
			break;
		}
	}
	if (!found)
		uiInstance->m_peAddPointerButton->deactivate();
	uiInstance->m_peDeletePointerButton->activate();
	uiInstance->m_peResolvedAddress->setValue((unsigned long)0);
	uiInstance->m_peResolvedAddress->value("Refresh");
	uiInstance->m_peResolvedValue->setValue((unsigned long)0);
	uiInstance->m_peResolvedValue->value("Refresh");

}

void PointerEditorWindow::OnDeleteCB(Fl_Widget *w)
{
	bool found = false;
	for (int i=groups.size()-1; i>=1; --i)
	{
		if (groups[i]->visible())
		{
			found = true;
			groups[i]->setOffsetOnly(0);
			groups[i]->hide();
			if (i == 1)
				uiInstance->m_peDeletePointerButton->deactivate();
			break;
		}
	}
	if (!found) //we've deleted all we can
		uiInstance->m_peDeletePointerButton->deactivate();
	uiInstance->m_peAddPointerButton->activate();
	uiInstance->m_peResolvedAddress->setValue((unsigned long)0);
	uiInstance->m_peResolvedAddress->value("Refresh");
	uiInstance->m_peResolvedValue->setValue((unsigned long)0);
	uiInstance->m_peResolvedValue->value("Refresh");

}

void PointerEditorWindow::OnRefreshCB(Fl_Widget *w)
{
	readPointers();
	if (m_operator != 0 && currentPointer != nullptr)
	{
		uiInstance->m_peRefreshButton->deactivate();
		m_operator->setReadMemoryOperation(currentPointer, false);
		Fl::add_timeout(0.5, refreshTimeout, this);
	}
}

void PointerEditorWindow::refreshTimeout()
{
	if (currentPointer != nullptr && currentPointer->isPointer() && currentPointer->pointer->updateCount != pointerUpdateCounter)
	{
		unsigned int i=0;
		Fl::remove_timeout(refreshTimeout, this);
		pointerUpdateCounter = currentPointer->pointer->updateCount;
		for (auto it = currentPointer->pointer->pointers.begin(); it != currentPointer->pointer->pointers.end(); ++it, ++i)
		{
			if (i >= groups.size())
				break;
			groups[i]->set(it->address, it->offset);
			groups[i]->setRefresh(false);
		}
		uiInstance->m_peResolvedAddress->setValue(currentPointer->pointer->resolved);
		uiInstance->m_peResolvedValue->setValueType(-1);
		uiInstance->m_peResolvedValue->setLiteral(currentPointer->type != SEARCH_VALUE_TYPE_FLOAT);
		long long nval = Helpers::convert4BytesToType(currentPointer->value, currentPointer->type);
		uiInstance->m_peResolvedValue->setValue(nval);
		uiInstance->m_peResolvedValue->setValueType(currentPointer->type);

		uiInstance->m_peRefreshButton->activate();
		damage(FL_DAMAGE_ALL);
	}
	else if (currentPointer == nullptr)
	{
		Fl::remove_timeout(refreshTimeout, this);
		uiInstance->m_peRefreshButton->activate();
	}
	else
		Fl::repeat_timeout(0.5, refreshTimeout, this);
}

void PointerEditorWindow::readPointers()
{
	unsigned long address;
	PointerOffsets po;
	address = uiInstance->m_peAddressSet0->getAddress();
	po.push_back(uiInstance->m_peAddressSet0->getOffset());
	for (unsigned int i=1; i<groups.size(); ++i)
	{
		if (!groups[i]->visible())
			break;
		po.push_back(groups[i]->getOffset());
	}
	PointerItem p = make_shared<PointerObj>(address, po);
	currentPointer = make_shared<AddressObj>();
	currentPointer->address = address;
	currentPointer->pointer = p;
	currentPointer->type = uiInstance->m_peValueType->getValue();
	pointerUpdateCounter = 0;
}

void PointerEditorWindow::reset() 
{ 
	createGroup();
	for (auto it = groups.begin(); it != groups.end(); ++it)
	{
		(*it)->reset();
		(*it)->setAddressEditable(false);
	}
	groups[0]->setAddressEditable(true);
	uiInstance->m_peResolvedAddress->setHex(true);
	uiInstance->m_peResolvedAddress->setValue((unsigned long)0);
	uiInstance->m_peResolvedAddress->value("");
	uiInstance->m_peResolvedAddress->deactivate();
	uiInstance->m_peValueType->reset();
	uiInstance->m_peResolvedValue->setValueType(uiInstance->m_peValueType->getValue());
	uiInstance->m_peResolvedValue->setHex(false);
	uiInstance->m_peResolvedValue->setValue((unsigned long)0);
	uiInstance->m_peResolvedValue->value("Refresh");
	uiInstance->m_peResolvedValue->setLiteral(false);
	uiInstance->m_peResolvedValue->deactivate();
	currentPointer = nullptr;
	m_resAddress = 0;
}

void PointerEditorWindow::OnTypeChangedCB(Fl_Widget *w)
{
	char type = uiInstance->m_peValueType->getValue();
	if (currentPointer != nullptr && currentPointer->type != type)
	{
		uiInstance->m_peResolvedValue->setLiteral(type != SEARCH_VALUE_TYPE_FLOAT);
		long long nval = Helpers::convert4BytesToType(currentPointer->value, type);
		uiInstance->m_peResolvedValue->setValue(nval);
		uiInstance->m_peResolvedValue->setValueType(type);
		currentPointer->type = type;
	}
}

void PointerEditorWindow::show()
{
	rkWindow::show();
}

AddressItem PointerEditorWindow::popup(unsigned long address, PointerOffsets offsets)
{
	reset();
	setPointer(address, offsets);
	show();
	while (shown()) Fl::wait();
	return currentPointer;
}

AddressItem PointerEditorWindow::popup(AddressItem p)
{
	reset();
	setPointer(p);
	show();
	while (shown()) Fl::wait();
	return currentPointer;
}
void PointerEditorWindow::hide()
{
	//if (visible())
//		uiInstance->m_valueTable->stopMemoryRead();
	rkWindow::hide();
}

void PointerEditorWindow::setPointer(unsigned long address, PointerOffsets offsets)
{
	unsigned int i=0;
	for (auto it = offsets.begin(); it!=offsets.end(); ++it, ++i)
	{
		if (i == 0)
			groups[i]->set(address, *it);
		else if (i<groups.size())
		{
			groups[i]->setOffsetOnly(*it);
		}
	}
	if (m_resAddress == 0)
		uiInstance->m_peResolvedAddress->value("Refresh");
	else
		uiInstance->m_peResolvedAddress->setValue(m_resAddress);
	currentPointer = make_shared<AddressObj>(address, offsets, SEARCH_VALUE_TYPE_4BYTE, 0);
}

void PointerEditorWindow::setPointer(AddressItem p)
{
	PointerOffsets po = p->pointer->getOffsets();
	setPointer(p->address, po);
}

void PointerEditorWindow::createGroup()
{
	groups.clear();
	groups.push_back(uiInstance->m_peAddressSet0);
	groups.push_back(uiInstance->m_peAddressSet1);
	groups.push_back(uiInstance->m_peAddressSet2);
	groups.push_back(uiInstance->m_peAddressSet3);
	groups.push_back(uiInstance->m_peAddressSet4);
}
