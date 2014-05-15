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
	uiInstance->m_valueTable->setAddress(item->address);
	uiInstance->m_valueTable->setType(item->type);
}

void ValueViewerWindow::show()
{
	if (!visible())
		uiInstance->m_valueTable->startMemoryRead();
	Fl_Window::show();
}

void ValueViewerWindow::hide()
{
	if (visible())
		uiInstance->m_valueTable->stopMemoryRead();
	Fl_Window::hide();
}