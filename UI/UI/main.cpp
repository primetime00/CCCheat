#include "ui_main.h"
#include "CCAPI/CCAPI.h"
#include "InterfaceCCAPI.h"
using namespace std;

void windowCallback(Fl_Widget*, void*) {
  if (Fl::event()==FL_SHORTCUT && Fl::event_key()==FL_Escape) 
    return; // ignore Escape
  exit(0);
}

int main() {
	CCAPI ccapi("127.0.0.1");
	rkCheatUI ui;
	InterfaceCCAPI m_interface(&ccapi, &ui);
	ui.mainWindow->callback(windowCallback);
	ui.setInterface(&m_interface);
	ui.setConnectStatus(INTERFACE_DISCONNECT);
	ui.mainWindow->show();
    return Fl::run();
}