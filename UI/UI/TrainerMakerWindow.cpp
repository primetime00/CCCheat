#include "TrainerMakerWindow.h"
#include "TrainerCreator.h"
#include "ui_main.h"

using namespace std;
void TrainerMakerWindow::init() 
{ 
	uiInstance->m_tcTrainerTitle->maximum_size(TITLELEN);
	uiInstance->m_tcAuthorName->maximum_size(AUTHORLEN);
	uiInstance->m_tcGameTitle->maximum_size(GAMETITLELEN);
	uiInstance->m_tcGameRegion->maximum_size(REGIONLEN);
	uiInstance->m_tcInformation->buffer(infoBuffer);

	checkButtonStatus();
}

void TrainerMakerWindow::show()
{
	init();
	rkWindow::show();
}

void TrainerMakerWindow::checkButtonStatus()
{
	bool createPass = true;
	bool editPass = true;
	if (uiInstance->m_tcCodeBrowser->size() == 0)
	{
		createPass = false;
		editPass = false;
	}
	if (uiInstance->m_tcCodeBrowser->value() == 0)
		editPass = false;
	if (uiInstance->m_tcTrainerTitle->size() == 0 || uiInstance->m_tcGameRegion->size() == 0 || uiInstance->m_tcGameTitle->size() == 0)
	{
		createPass = false;
	}
	if (createPass) uiInstance->m_tcCreateButton->activate(); else uiInstance->m_tcCreateButton->deactivate();
	if (editPass) uiInstance->m_tcEditCodeButton->activate(); else uiInstance->m_tcEditCodeButton->deactivate();

}

void TrainerMakerWindow::popup()
{
	m_codes.clear();
	uiInstance->m_tcCodeBrowser->clear();
	show();
	while (shown()) Fl::wait();
}
void TrainerMakerWindow::createNewCode()
{
	uiInstance->m_trainerItem->popup(uiInstance->ui_codeTable);
	auto code = uiInstance->m_trainerItem->getCode();
	if (code == nullptr)
		return;
	m_codes.push_back(code);
	uiInstance->m_tcCodeBrowser->add(code->name.c_str(), code.get());
	checkButtonStatus();

}

void TrainerMakerWindow::onSelectCode()
{
	checkButtonStatus();
}

void TrainerMakerWindow::onEdit()
{
	auto v = uiInstance->m_tcCodeBrowser->value();
	auto data = uiInstance->m_tcCodeBrowser->data(v);
	uiInstance->m_trainerItem->popup(uiInstance->ui_codeTable, (rkTrainerCodeObj*) data);
	auto code = uiInstance->m_trainerItem->getCode();
	if (code == nullptr)
		return;
	for (auto ci = m_codes.begin(); ci != m_codes.end();)
	{
		if ((*ci).get() == (rkTrainerCodeObj*) data)
			ci = m_codes.erase(ci);
		else
			++ci;
	}
	uiInstance->m_tcCodeBrowser->remove(v);
	uiInstance->m_tcCodeBrowser->insert(v, code->name.c_str(), code.get());
	m_codes.push_back(code);
	onSelectCode();
	damage(FL_DAMAGE_ALL);
}

void TrainerMakerWindow::onCreate()
{
	TrainerCreator creator;
	creator.setTitle(uiInstance->m_tcTrainerTitle->value());
	creator.setAuthor(uiInstance->m_tcAuthorName->value());
	creator.setGameTitle(uiInstance->m_tcGameTitle->value());
	creator.setInformation(uiInstance->m_tcInformation->buffer()->text());
	creator.setRegion(uiInstance->m_tcGameRegion->value());
	creator.setVersion("1.0");
	creator.setCodes(m_codes);
	creator.exportTrainer();
	fl_message("The trainer was successfully created!");
	hide();
}

void TrainerMakerWindow::capture()
{
	rkWindow::capture();
	createdW = uiInstance->m_tcCodeGroup->x() + uiInstance->m_tcCodeGroup->w() + 10;
	createdH = uiInstance->m_tcCreateButton->y() + uiInstance->m_tcCreateButton->h() + 20;

}

