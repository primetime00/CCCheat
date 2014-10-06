#include "ValueViewerWindow.h"
#include "ValueViewerTable.h"
#include "ui_main.h"

void ValueViewerWindow::TypeChangedCB(Fl_Widget *w)
{
	char type = uiInstance->m_viewerTypeChoice->value();
	uiInstance->m_valueTable->setType(type);
	if (uiInstance->m_viewerTypeChoice->value() == SEARCH_VALUE_TYPE_FLOAT)
		uiInstance->m_viewerSignedButton->deactivate();
	else
		uiInstance->m_viewerSignedButton->activate();
}
void ValueViewerWindow::SignedChangedCB(Fl_Widget *w)
{
	char sign = uiInstance->m_viewerSignedButton->value();
	uiInstance->m_valueTable->setSigned(sign != 0);
}

void ValueViewerWindow::AddCodeCB(Fl_Widget *w)
{
	uiInstance->m_valueTable->addSelectedAddresses();
}

void ValueViewerWindow::setCodeData(rkCheat_Code *item)
{
	uiInstance->m_viewerTypeChoice->value(item->m_address->type);
	auto resolved = item->m_address->isPointer() ? item->m_address->pointer->resolved : item->m_address->address;
	if (visible() && uiInstance->m_valueTable->getAddress() != resolved)
		hide();
	uiInstance->m_valueTable->setAddress(resolved);
	uiInstance->m_valueTable->setType(item->m_address->type);
	TypeChangedCB(this);
}

void ValueViewerWindow::show()
{
	if (!visible())
		uiInstance->m_valueTable->startMemoryRead();
	rkWindow::show();
}

void ValueViewerWindow::hide()
{
	if (visible())
		uiInstance->m_valueTable->stopMemoryRead();
	reset();
	rkWindow::hide();
}

void ValueViewerWindow::reset()
{
	uiInstance->m_valueTable->clearSelection();
	uiInstance->m_valueTable->setType(SEARCH_VALUE_TYPE_4BYTE);
	uiInstance->m_valueTable->setAddress(0);
	uiInstance->m_valueTable->setSigned(false);
	Fl_Menu_Item *it = &uiInstance->menu_ui_valueType[0];
	while (it->label() != 0)
	{
		if (get_user_data(int, it->user_data()) == SEARCH_VALUE_TYPE_4BYTE)
		{
			uiInstance->m_viewerTypeChoice->value(it);
			break;
		}
		it++;
	}
	uiInstance->m_viewerSignedButton->value(0);
}

void ValueViewerWindow::capture()
{
	Fl_Window::show();
	createdX = x();
	createdY = y();
	createdW = uiInstance->m_valueAddCodeButton->x()+uiInstance->m_valueAddCodeButton->w()+20;
	createdH = uiInstance->m_valueTable->y() + uiInstance->m_valueTable->h() + 20;
	Fl_Window::hide();
}