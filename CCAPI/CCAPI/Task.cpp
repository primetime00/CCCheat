#include <iostream>
#include "Task.h"

Task::Task(unsigned int id, string name)
{
	m_id = id;
	m_name = name;
	m_fire = false;
	m_error = TASK_ERROR_NONE;
	m_taskState = WAIT;
	m_cancel = false;
}

void Task::start()
{
	m_thread = thread(&Task::run, this);
}

void Task::run(void) 
{
}

