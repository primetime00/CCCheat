#include "TrainerItemWindow.h"
#include "ui_main.h"

using namespace std;
void TrainerItemWindow::init() 
{ 
	uiInstance->m_tciCodeName->maximum_size(TITLELEN);
	uiInstance->m_tciConditionalCheck->value(0);
	m_code = nullptr;
	m_cl.clear();
	int width = uiInstance->m_tciCodeTable->w() + uiInstance->m_tciCodeTable->x();
	uiInstance->m_tciCodeTable->setParent(this);
	uiInstance->m_tciCodeName->value("");
	uiInstance->m_tciCodeTable->init();
	w(width+10);
	createdW = width+10;
	checkButtonStatus();
}

void TrainerItemWindow::show()
{
	if (!shown())
		init();
	rkWindow::show();
}

void TrainerItemWindow::checkButtonStatus()
{
	bool okPass = true;
	bool okCondition = true;
	bool addPass = true;
	string name = uiInstance->m_tciCodeName->value();
	if (uiInstance->m_tciConditionalCheck->value() > 0)
	{
		if (uiInstance->m_tciConditionalCode->value() == 0)
			okPass = false;
		if (uiInstance->m_tciConditionalValue->size() == 0)
			okPass = false;
	}
	else
		okCondition = false;
	if (name.empty() || uiInstance->m_tciCodeTable->rows() == 0 || uiInstance->m_tciCodeTable->isEmpty() || (uiInstance->m_tciConditionalCheck->value() && uiInstance->m_tciConditionalCode->value() == 0))
	{
		okPass = false;
		addPass = false;
	}
	if (okPass) uiInstance->m_tciOKButton->activate(); else uiInstance->m_tciOKButton->deactivate();
	if (addPass) uiInstance->m_tciAddCodeButton->activate(); else uiInstance->m_tciAddCodeButton->deactivate();
	if (okCondition) uiInstance->m_tciConditionGroup->activate(); else uiInstance->m_tciConditionGroup->deactivate();

}

void TrainerItemWindow::popup(CodeTable *t, rkTrainerCodeObj *code)
{
	show();
	setupCodes(t, code);
	while (shown()) Fl::wait();
}

void TrainerItemWindow::setupCodes(CodeTable *t, rkTrainerCodeObj *code)
{
	int index = 0;
	int conditionSet = 0;
	if (code != 0)
	{
		auto name = code->name.c_str();
		uiInstance->m_tciCodeName->value(name);
		if (code->condition == true)
		{
			uiInstance->m_tciConditionalCheck->value(1);
			int i=0;
			const Fl_Menu_Item *menu = uiInstance->m_tciConditionalOperator->menu();
			while (menu[i].text == 0)
			{
				if (get_user_data(char, menu[i].user_data()) == code->conditionType)
				{
					uiInstance->m_tciConditionalOperator->value(i);
					break;
				}
				++i;
			}
			uiInstance->m_tciConditionGroup->activate();
		}

	}
	uiInstance->m_tciConditionalCode->menu(uiInstance->m_tciDefaultMenuItem);
	if (t != 0)
	{
		m_cl = t->getCodes();
		for (auto it = m_cl.begin(); it != m_cl.end(); ++it, ++index)
		{
			uiInstance->m_tciConditionalCode->add(getDescriptionOrCode((*it)).c_str(), 0, 0, (*it)->m_address.get(), 0);
			if (code != 0 && code->condition == true)
			{
				if ( *(*it)->m_address == *code->m_conditionAddress )
					conditionSet = index+1;
			}
		}
		uiInstance->m_tciConditionalCode->value(conditionSet);
		onConditionCodeChanged( (code != 0) ? code->m_conditionAddress.get() : 0);
	}
	uiInstance->m_tciCodeTable->setCodeList(t->getCodes());
	if (code != 0)
	{
		uiInstance->m_tciCodeTable->clearTable();
		uiInstance->m_tciCodeTable->addCodes(code->m_addresses);
	}
	checkButtonStatus();
}

void TrainerItemWindow::onConditionChanged()
{
	checkButtonStatus();
}

void TrainerItemWindow::onConditionCodeChanged(AddressObj *addr)
{
	if (uiInstance->m_tciConditionalCode->value() == 0)
	{
		uiInstance->m_tciConditionalValue->value("");
	}
	else
	{
		auto item = uiInstance->m_tciConditionalCode->mvalue();
		AddressObj *obj = (addr == 0) ? (AddressObj*)item->user_data() : addr;

		long long int_val = (obj->sign) ? (signed long)obj->value : (unsigned long) obj->value; 
		Variant variant((long long)obj->value);
		float float_val = variant.asFloat();

		string s = to_string(obj->value);
		uiInstance->m_tciConditionalValue->setValueType(obj->type);
		if (obj->type == SEARCH_VALUE_TYPE_FLOAT)
			uiInstance->m_tciConditionalValue->value(to_string(float_val).c_str());
		else
		{
			switch (obj->type)
			{
			case SEARCH_VALUE_TYPE_1BYTE:
				uiInstance->m_tciConditionalValue->value(obj->sign ? to_string((char)int_val).c_str() : to_string((unsigned char)int_val).c_str());
				break;
			case SEARCH_VALUE_TYPE_2BYTE:
				uiInstance->m_tciConditionalValue->value(obj->sign ? to_string((short)int_val).c_str() : to_string((unsigned short)int_val).c_str());
				break;
			default:
				uiInstance->m_tciConditionalValue->value(obj->sign ? to_string((long)int_val).c_str() : to_string((unsigned long)int_val).c_str());
				break;
			}
		}

	}
	checkButtonStatus();
}

void TrainerItemWindow::onAddCode()
{
	uiInstance->m_tciCodeTable->addCode();
}

void TrainerItemWindow::onDeleteCode()
{
	uiInstance->m_tciCodeTable->deleteCode();
}

void TrainerItemWindow::onNameChanged()
{
	checkButtonStatus();
}

string TrainerItemWindow::getDescriptionOrCode(const rkCheat_CodeItem &item)
{
	char desc[300];
	string output;
	if (item->description.empty())
		sprintf(desc,"0x%08lX",item->m_address->address);
	else
		strncpy(desc,item->description.c_str(), 300);
	output = string(desc);
	return output;
}

void TrainerItemWindow::OnCodeTableUpdate(TrainerItemCodeTable *ct, bool timeout)
{
	if (timeout)
		Fl::add_timeout(0.01, UpdateTimeout, ct);
	else
		UpdateTimeout(ct);
}

void TrainerItemWindow::UpdateTimeout(void *data)
{
	string name = uiInstance->m_tciCodeName->value();
	TrainerItemCodeTable *ct = (TrainerItemCodeTable*)data;
	//find the parent, which i hope is the window
	TrainerItemWindow *win = (TrainerItemWindow *)ct->parent();
	win->checkButtonStatus();
	/*
	if (ct->isEmpty() || !ct->hasSelection())
		uiInstance->m_tciDeleteCodeButton->deactivate();
	else
		uiInstance->m_tciDeleteCodeButton->activate();
	if (ct->canAdd())
		uiInstance->m_tciAddCodeButton->activate();
	else
		uiInstance->m_tciAddCodeButton->deactivate();

	if (!name.empty() && !ct->isEmpty() && (uiInstance->m_tciConditionalCheck->value() == 0 || (uiInstance->m_tciConditionalCode->value() > 0) && uiInstance->m_tciConditionalCheck->value() == 1))
		uiInstance->m_tciOKButton->activate();
	else
		uiInstance->m_tciOKButton->deactivate();
		*/
	Fl::remove_timeout(UpdateTimeout);
}

void TrainerItemWindow::onCancel()
{
	m_code = nullptr;
	hide();
}

void TrainerItemWindow::onOK()
{
	auto codeList = uiInstance->m_tciCodeTable->getCodes();
	if (codeList.size() == 0)
	{
		hide();
		m_code = nullptr;
		return;
	}
	m_code = make_shared<rkTrainerCodeObj>();
	m_code->name = uiInstance->m_tciCodeName->value();
	m_code->condition = uiInstance->m_tciConditionalCheck->value() ? true : false;
	if (m_code->condition)
	{
		auto pos = uiInstance->m_tciConditionalCode->value() - 1;
		m_code->m_conditionAddress = make_shared<AddressObj>( *(m_cl[pos]->m_address) );
		m_code->m_conditionAddress->store = m_code->m_conditionAddress->value = uiInstance->m_tciConditionalValue->getLLValue();
		m_code->conditionValue = m_code->m_conditionAddress->store;
		m_code->conditionType = get_user_data(char, uiInstance->m_tciConditionalOperator->mvalue()->user_data());
	}
	m_code->m_addresses.clear();
	for (auto i = codeList.begin(); i != codeList.end(); ++i)
	{
		auto pos = (*i)->choice->value() - 1;
		auto val = make_shared<AddressObj>(*(m_cl[pos]->m_address));
		val->store = val->value = (*i)->value_input->getLLValue();
		m_code->m_addresses.push_back( val );
	}
	hide();
}

void TrainerItemWindow::capture()
{
	rkWindow::capture();
	createdW = uiInstance->m_tciCodeTable->x() + uiInstance->m_tciCodeTable->w() + 10;
	createdH = uiInstance->m_tciOKButton->y() + uiInstance->m_tciOKButton->h() + 20;
	int btnX = (createdW - uiInstance->m_tciOKButton->w())/2;
	uiInstance->m_tciOKButton->position( btnX - ( (uiInstance->m_tciOKButton->w()/2) + 10) , uiInstance->m_tciOKButton->y());
	uiInstance->m_tciCancelButton->position( btnX + ( (uiInstance->m_tciCancelButton->w()/2) + 10) , uiInstance->m_tciCancelButton->y());
}
