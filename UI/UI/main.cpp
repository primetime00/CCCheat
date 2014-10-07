#include "ui_main.h"
#include "CCAPI/CCAPI.h"
#include "InterfaceCCAPI.h"
#include "TrainerSrc.h"

#include <vector>
#include <algorithm>
#include <fstream>
using namespace std;



void windowCallback(Fl_Widget*, void*) {
  if (Fl::event()==FL_SHORTCUT && Fl::event_key()==FL_Escape) 
    return; // ignore Escape
  if (InterfaceCCAPI::instance != 0)
	  InterfaceCCAPI::instance->disconnect();
  exit(0);
}

void exportTrainer(string name, string filename)
{
	vector<unsigned char> s;
	char cTitle[] = "TRAIN_TITLE";
	s.assign(trainSrc, trainSrc+397312);
	auto it = search(s.begin(), s.end(), cTitle, cTitle+strlen(cTitle));
	if (it == s.end()) return;
	for (auto nit = name.begin(); nit != name.end(); ++nit)
	{
		*it = *nit;
		++it;
	}
	ofstream outFile(filename, ofstream::binary);
	outFile.write((char*)s.data(), 397312);
	outFile.close();
}

int main() {
	CCAPI ccapi("127.0.0.1");
	rkCheatUI ui;
	InterfaceCCAPI m_interface(&ccapi, &ui);
	ui.mainWindow->callback(windowCallback);
	ui.m_trainerMakerWindow->callback(windowCallback);
	ui.m_pointerScannerWindow->callback(windowCallback);
	ui.setVersion(CCCHEAT_VERSION);
	ui.setInterface(&m_interface);
	ui.setConnectStatus(INTERFACE_DISCONNECT);
	ui.mainWindow->show();
    return Fl::run();
}
