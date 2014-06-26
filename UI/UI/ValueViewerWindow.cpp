#include "ValueViewerWindow.h"
#include "ValueViewerTable.h"
#include "ui_main.h"

void ValueViewerWindow::TypeChangedCB(Fl_Widget *w)
{
	char type = uiInstance->m_viewerTypeChoice->value();
	uiInstance->m_valueTable->setType(type);
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
	uiInstance->m_viewerTypeChoice->value(item->type);
	if (visible() && uiInstance->m_valueTable->getAddress() != item->m_address.address)
		hide();
	uiInstance->m_valueTable->setAddress(item->m_address.address);
	uiInstance->m_valueTable->setType(item->type);
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
	rkWindow::hide();
}