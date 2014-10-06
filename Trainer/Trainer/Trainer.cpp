// Trainer.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "Trainer.h"
#include "Interface.h"
#include "ui_trainer.h"

void windowCallback(Fl_Widget*, void*) {
  if (Fl::event()==FL_SHORTCUT && Fl::event_key()==FL_Escape) 
    return; // ignore Escape
  exit(0);
}

char trainerTitle[TITLELEN] = {TITLE_TXT};
char trainerAuthor[AUTHORLEN] = {AUTHOR_TXT};
char trainerInfo[INFOLEN] = {INFO_TXT};
char trainerCodeDesc[CODESLEN] = {CODES_TXT};
char trainerGameTitle[GAMETITLELEN] = {GAME_TITLE_TXT};
char trainerGameVersion[VERSIONLEN] = {VERSION_TXT};
char trainerGameRegion[REGIONLEN] = {REGION_TXT};

stringstream info;
void SetInfo()
{
	info << "Trainer: " << trainerTitle << "\n";
	info << "By: " << trainerAuthor << "\n";
	info << "Game: " << trainerGameTitle << " " << trainerGameVersion << " [" << trainerGameRegion << "]\n\n";
	info << trainerInfo;
}

int main(int argc, char* argv[])
{
	CCAPI ccapi("127.0.0.1");
	TrainerClass ui;
	ui.make_window();
	Interface iface(&ccapi, &ui);
	ui.setConnectStatus(INTERFACE_DISCONNECT);
	ui.mainWindow->label(trainerTitle);
	SetInfo();
	ui.ui_infoBuffer.append(info.str().c_str());
	ui.ui_trainerTable->setCodes(trainerCodeDesc);
	ui.ui_trainerTable->deactivate();
	ui.mainWindow->callback(windowCallback);
	ui.mainWindow->show();
    return Fl::run();

}

