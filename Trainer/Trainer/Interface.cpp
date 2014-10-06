#include "Interface.h"
#include "ui_trainer.h"
#include "CCAPI/Common.h"
#include "FL/Fl.H"
#include <sstream>

#if defined(_WIN32) || defined(WIN32)
#pragma warning(disable:4503)
#endif

Interface *Interface::instance = 0;


void Interface::connect(string ip)
{
	m_ip = ip;
	ConnectionManager::startUp();
	connection = make_shared<ConnectionManager>();
	memoryOperator = new MemoryOperator(ip);
	int ver = get_user_data(int, TrainerClass::uiInstance->ui_ccapiChoice->mvalue()->user_data());
	memoryOperator->setHostCCAPIVersion(ver);
	memoryOperator->start();
	Fl::add_timeout(0.2, Interface::connectCheck);
}

void Interface::_connectCheck()
{
	if (memoryOperator != 0)
	{
		string stat = memoryOperator->getStatus();
		if (stat == "CONNECT") //we are connecting still
		{
			Fl::repeat_timeout(0.2, Interface::connectCheck);
			TrainerClass::uiInstance->ui_trainerTable->deactivate();
		}
		else if (stat == "ERROR") //we failed!
		{
			Fl::remove_timeout(Interface::connectCheck);
			TrainerClass::uiInstance->setConnectStatus(INTERFACE_CONNECT_FAIL);
			TrainerClass::uiInstance->ui_trainerTable->deactivate();
			disconnect();
		}
		else
		{
			Fl::remove_timeout(Interface::connectCheck);
			TrainerClass::uiInstance->setConnectStatus(INTERFACE_CONNECT_SUCCESS);
			TrainerClass::uiInstance->ui_trainerTable->activate();
		}
	}
}

void Interface::disconnect()
{
	TrainerClass::uiInstance->ui_trainerTable->deactivate();
	if (memoryOperator != 0)
	{
		delete memoryOperator;
	}
	memoryOperator = 0;
	connection = nullptr;
}

void Interface::disable(rkTrainerCodeObj *item)
{
	if (!item) return;
	if (item->m_conditionAddress)
		memoryOperator->removeMemoryOperation(MEMORY_COMMAND_CONDITIONAL, item->m_conditionAddress);
	else
	{
		for (auto i = item->m_addresses.begin(); i != item->m_addresses.end(); ++i)
			memoryOperator->removeMemoryOperation(MEMORY_COMMAND_WRITE, (*i));
	}
}

void Interface::enable(rkTrainerCodeObj *item)
{
	if (!item) return;
	if (item->m_conditionAddress)
	{
		memoryOperator->setConditionalWriteMemoryOperation(item->m_conditionAddress, item->conditionType, item->conditionValue, item->m_addresses);
	}	
	else
	{
		for (auto i = item->m_addresses.begin(); i != item->m_addresses.end(); ++i)
		{
			if ((*i)->isPointer())
				memoryOperator->setReadMemoryOperation((*i), true);
			memoryOperator->setWriteMemoryOperation((*i), (*i)->value, true);
		}
	}
}