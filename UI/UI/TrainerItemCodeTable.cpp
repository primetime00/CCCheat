#include "TrainerItemCodeTable.h"
#include "CCAPI/Common.h"
#include "CCAPI/Helpers.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <FL/fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Widget.H>
#include "TrainerItemWindow.h"
#include "ui_main.h"

using namespace std;

#define DESC_COL		0
#define VALUE_COL		1

#define CODE_COLS		2

TrainerItemCodeTable::TrainerItemCodeTable(int X, int Y, int W, int H, const char *l) : Fl_Table_Row(X,Y,W,H,l) {

	last_cell_widget = 0;
	rkWindow::addObject(this);

	rows(0);
	row_header(0);
	row_height_all(20);
	row_resize(0);

	cols(CODE_COLS);
	col_header(1);
	col_width_all(110);
	col_width(DESC_COL, 330);
	col_width(VALUE_COL, 220);
	col_resize(1);
	unsigned int width=0;
	for (int i=0; i<cols(); ++i)
		width += col_width(i);
	w(width+4);

	m_menu = 0;
	selectedRow = 0;

	type(SELECT_MULTI);
	end();
	callback(tableClickedCB);
	parentWindow = 0;
}

void TrainerItemCodeTable::setCodeList(const rkCheat_CodeList &list)
{
	int pos = 0;
	codeList.clear();
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		codeList.push_back(*it);
	}
	if (m_menu)
	{
		for (auto it = trainList.begin(); it != trainList.end(); ++it)
		{
			(*it)->choice->menu(0);
		}
		delete [] m_menu;
		m_menu = 0;
	}
	m_menu = new Fl_Menu_Item[55];//codeList.size()+1];
	memset(m_menu, 0, sizeof(Fl_Menu_Item)*(codeList.size()+1));
	((Fl_Menu_Item*)m_menu)->add("Code...", 0, 0, 0, 0);
	m_menu[pos].labeltype(FL_NORMAL_LABEL);
	m_menu[pos].labelsize(14);
	pos++;

	for (auto it = codeList.begin(); it != codeList.end(); ++it)
	{
		((Fl_Menu_Item*)m_menu)->add(TrainerItemWindow::getDescriptionOrCode((*it)).c_str(), 0, 0, 0, 0);
		m_menu[pos].user_data((*it)->m_address.get());
		m_menu[pos].labeltype(FL_NORMAL_LABEL);
		m_menu[pos].labelsize(14);
		pos++;
	}

	if (trainList.size() == 0)
		createEntry();
}

void TrainerItemCodeTable::init()
{
	last_cell_widget = 0;
	rows(0);
	m_menu = 0;
	selectedRow = 0;
	trainList.clear();
}
void TrainerItemCodeTable::createEntry()
{
	int x,y,w,h;

	if (trainList.size() > 0 && trainList[trainList.size()-1]->choice->mvalue() == &(trainList[trainList.size()-1]->menu[0]))
		return;
	rows(trainList.size()+1);
	begin();

	Fl_Menu_Item *item_menu = copyMenu(m_menu);
	find_cell(Fl_Table::CONTEXT_TABLE, trainList.size(), DESC_COL, x,y,w,h);
	Fl_Choice *desc_input = new Fl_Choice(x,y,w,h);
	desc_input->menu(item_menu);
	desc_input->callback((Fl_Callback*)codeDescriptionChangedCB);
	desc_input->when(FL_WHEN_ENTER_KEY_CHANGED);
	//desc_input->hide();

	find_cell(Fl_Table::CONTEXT_TABLE, trainList.size(), VALUE_COL, x,y,w,h);
	ValueInput *value_input = new ValueInput(x,y,w,h);
	value_input->setHex(false);
	value_input->setLiteral(true);
	value_input->setValueType(SEARCH_VALUE_TYPE_4BYTE);
	value_input->callback((Fl_Callback*)codeValueChangedCB);
	value_input->when(FL_WHEN_RELEASE);
	value_input->setCodeType(true);
	value_input->hide();

	end();
	CodeItem cItem = make_shared<ItemField>(desc_input, value_input, item_menu);
	trainList.push_back(cItem);

	desc_input->user_data(cItem.get());
	value_input->user_data(cItem.get());

	desc_input->take_focus();
	row_position(rows()-1);
	rebuildTable();
	TrainerItemWindow::OnCodeTableUpdate(this, true);
}


void TrainerItemCodeTable::clearTable()
{
	trainList[0]->choice->value(0);
}

void TrainerItemCodeTable::deleteEntry(unsigned int row)
{
	if (trainList.size() == 1) //we are getting rid of the last entry
	{
		clearTable();
	}
	else
	{
		if (trainList.at(row)->data == nullptr)
			return;
		this->remove(*trainList.at(row)->choice);
		this->remove(*trainList.at(row)->value_input);
		trainList.erase(trainList.begin()+row, trainList.begin()+row+1);
		rebuildTable();
		rows(trainList.size());
	}
	TrainerItemWindow::OnCodeTableUpdate(this, true);
}

void TrainerItemCodeTable::deleteEntry(vector<int> &indexRows)
{
	rkCheat_CodeList cl;
	if (indexRows.size() >= trainList.size())
	{
		clearTable();
	}
	else
	{
		for (vector<int>::iterator it = indexRows.begin(); it!= indexRows.end(); ++it) //iterate through index		
		{
			trainList[*it]->_delete = 1;
		}
		for (CodeItems::iterator it = trainList.begin(); it != trainList.end();)
		{
			if ((*it)->_delete == 1)
			{
				remove(*(*it)->choice);
				remove(*(*it)->value_input);
				it = trainList.erase(it);
			}
			else
				it++;
		}
		rebuildTable();
		rows(trainList.size());
	}
	TrainerItemWindow::OnCodeTableUpdate(this, true);
}

void TrainerItemCodeTable::reInsert()
{
	int j=0;
	for (unsigned int i=0; i<trainList.size(); i++)
	{
		insert( *((Fl_Widget*)trainList[i]->choice), j++);
		insert( *((Fl_Widget*)trainList[i]->value_input), j++);
	}
}

void TrainerItemCodeTable::rebuildTable()
{
	int x,y,w,h;
	for (unsigned int i=0; i<trainList.size(); i++)
	{
		find_cell(CONTEXT_TABLE, i, DESC_COL, x,y,w,h);
		((Fl_Widget*)trainList[i]->choice)->position(x, y);
		find_cell(CONTEXT_TABLE, i, VALUE_COL, x,y,w,h);
		((Fl_Widget*)trainList[i]->value_input)->position(x, y);
	}
}

bool TrainerItemCodeTable::hasSelection()
{
	for (int i=0; i<rows(); ++i)
	{
		if (row_selected(i))
			return true;
	}
	return false;
/*	int t,b,l,r;
	get_selection(t,l,b,r);
	return (t > -1 && b > -1);*/
}

void TrainerItemCodeTable::draw_cell(TableContext context, int ROW, int COL, int X, int Y, int W, int H){
    static char s[40];
    switch ( context ) {
      case CONTEXT_STARTPAGE:                   // before page is drawn..
        fl_font(FL_HELVETICA, 14);              // set the font for our drawing operations
        return; 
	case CONTEXT_RC_RESIZE:
	{
	    int X, Y, W, H;
	    int index = 0;
	    for ( int r = 0; r<rows(); r++ )
	    {
			for ( int c = 0; c<cols(); c++ )
			{
				if ( index >= children() ) break;
				find_cell(CONTEXT_TABLE, r, c, X, Y, W, H);
				child(index)->resize(X,Y,W,H);
				index++;
			}
	    }
	    init_sizes();			// tell group children resized
	    return;
	}

      case CONTEXT_COL_HEADER:                  // Draw column headers
		switch (COL)
		{
		case DESC_COL:		DrawHeader("Code",X,Y,W,H); break;
		case VALUE_COL:		DrawHeader("Value",X,Y,W,H); break;
		default: break;
		}
        return; 
      case CONTEXT_ROW_HEADER:                  // Draw row headers
		fl_font(FL_HELVETICA, 14);              // set the font for our drawing operations
        sprintf(s,"%03d:",COL);                 // "001:", "002:", etc
        DrawHeader(s,X,Y,W,H);
        return; 
      case CONTEXT_CELL:    // Draw data in cells
		fl_font(FL_HELVETICA, 14);              // set the font for our drawing operations
		if (COL == DESC_COL && trainList[ROW]->choice->visible())
			return;
		else if (COL == VALUE_COL && trainList[ROW]->value_input->visible())
			return;
	    fl_push_clip(X, Y, W, H);
	    {
	        // BG COLOR
		fl_color( row_selected(ROW) ? selection_color() : FL_WHITE);
		fl_rectf(X, Y, W, H);

		// TEXT
		fl_color(FL_BLACK);
		if ((unsigned int)ROW < trainList.size())
		{
			Variant variant( (trainList[ROW]->data == nullptr) ? 0 : (long long)trainList[ROW]->data->value );
			switch(COL)
			{
			case DESC_COL: 
				break;
			case VALUE_COL:
				if (trainList[ROW]->choice->value() == 0)
					sprintf(s,""); 
				else
				{
					switch (trainList[ROW]->data->type)
					{
					case SEARCH_VALUE_TYPE_FLOAT: sprintf(s,"%f", variant.asFloat()); break;
					case SEARCH_VALUE_TYPE_1BYTE: 
							if (trainList[ROW]->data->sign) 
								 sprintf(s,"%d", (char)trainList[ROW]->data->value); 
							 else
								 sprintf(s,"%u", (unsigned char)trainList[ROW]->data->value); 
							 break;					
					case SEARCH_VALUE_TYPE_2BYTE: 
							if (trainList[ROW]->data->sign) 
								 sprintf(s,"%d", (short)trainList[ROW]->data->value); 
							 else
								 sprintf(s,"%u", (unsigned short)trainList[ROW]->data->value); 
							 break;					
					default:
							if (trainList[ROW]->data->sign) 
								 sprintf(s,"%ld", (long)trainList[ROW]->data->value); 
							 else
								 sprintf(s,"%lu", (unsigned long)trainList[ROW]->data->value); 
							 break;
					}
				}
/*				switch (data[ROW]->m_address->type)
				{
				case SEARCH_VALUE_TYPE_FLOAT: sprintf(s,"%f", variant.asFloat()); break;
				case SEARCH_VALUE_TYPE_1BYTE: 
						if (data[ROW]->m_address->sign) 
							 sprintf(s,"%d", (char)data[ROW]->m_address->value); 
						 else
							 sprintf(s,"%u", (unsigned char)data[ROW]->m_address->value); 
						 break;					
				case SEARCH_VALUE_TYPE_2BYTE: 
						if (data[ROW]->m_address->sign) 
							 sprintf(s,"%d", (short)data[ROW]->m_address->value); 
						 else
							 sprintf(s,"%u", (unsigned short)data[ROW]->m_address->value); 
						 break;					
				default:
						if (data[ROW]->m_address->sign) 
							 sprintf(s,"%ld", (long)data[ROW]->m_address->value); 
						 else
							 sprintf(s,"%lu", (unsigned long)data[ROW]->m_address->value); 
						 break;
				} */
				break;
			default: break;
			}
			DrawData(s,X,Y,W,H);
		}
		// BORDER
		fl_color(FL_LIGHT2); 
		fl_rect(X, Y, W, H);
	    }
	    fl_pop_clip();
        return;
      default:
        return;
    }
}


void TrainerItemCodeTable::tableClickedCB(Fl_Widget *w, void *data) {
	if (((Fl_Table*)w)->callback_context() == CONTEXT_CELL)
	{
		if (Fl::event_clicks())
			((TrainerItemCodeTable*)w)->onCellDoubleClicked(((Fl_Table*)w)->callback_row(), ((Fl_Table*)w)->callback_col());
		else if (Fl::event_button() == FL_RIGHT_MOUSE)
			((TrainerItemCodeTable*)w)->onCellRightClicked(((Fl_Table*)w)->callback_row(), ((Fl_Table*)w)->callback_col());
	}
	else
	{
		((TrainerItemCodeTable*)w)->onTableClicked();
		((TrainerItemCodeTable*)w)->select_all_rows(0);
	}
	TrainerItemWindow::OnCodeTableUpdate((TrainerItemCodeTable*)w, true);
}

int TrainerItemCodeTable::handle(int evt)
{
	int res = 0;
	when(FL_WHEN_NEVER);
	if (evt == FL_KEYUP)
	{
		if (Fl::event_text() && Fl::event_text()[0] == 27)
		{
			current_cell_widget = 0;
			if (current_cell_widget != last_cell_widget) //click on a new widget
			{
				if (last_cell_widget) last_cell_widget->hide();
				take_focus();
				this->window()->cursor(FL_CURSOR_DEFAULT);
			}
			last_cell_widget = current_cell_widget;
		}
	}
	res = Fl_Table_Row::handle(evt);
	return res;
}
void TrainerItemCodeTable::DrawData(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    // Draw cell bg
    //fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
    // Draw cell data
    fl_color(FL_GRAY0); fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
    // Draw box border
    fl_color(color()); fl_rect(X,Y,W,H);
fl_pop_clip();
}

void TrainerItemCodeTable::DrawHeader(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, row_header_color());
    fl_color(FL_BLACK);
    fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
fl_pop_clip();
}

void TrainerItemCodeTable::DrawCheckbox(int X, int Y, int W, int H)
{
	fl_push_clip(X,Y,W,H);

	fl_pop_clip();
}

void TrainerItemCodeTable::onDescriptionChanged(ItemField *item, AddressObj *addr, int value)
{
	if (value > 0)
	{
		item->menu[0].hide();
		if (item)
			item->data = make_shared<AddressObj>(*addr);
		else
			item->data = make_shared<AddressObj>(*addr);

		item->value_input->hide();
	}
	select_all_rows(0);
	for (int i=0; i<rows(); ++i)
	{
		if (trainList[i].get() == item)
		{
			selectRow(i);
			break;
		}
	}
	damage(FL_DAMAGE_ALL);
}

void TrainerItemCodeTable::onValueChanged(ItemField *item, AddressObj *addr, long long value)
{
	item->data->value = value;
	codeList[item->choice->value()-1]->m_address->value = value;
	/*
	if (item->m_address->value != (long long)value)
	{
		item->m_address->value = value;
		if (value < 0)
			item->setSign(true);
		else
			item->setSign(false);
		//item->setSign(Helpers::isSigned(item->type, value));
		if (m_operator != 0)
			m_operator->setWriteMemoryOperation(item->m_address, value, item->freeze);
	}
	*/
}


void TrainerItemCodeTable::onCellDoubleClicked(int row, int col)
{
	long long int_val; 
	if (trainList[row]->choice->mvalue() == &trainList[row]->menu[0])
		return;
	if (trainList[row]->data->sign)
		int_val = (signed long)trainList[row]->data->value;
	else
		int_val = (unsigned long) trainList[row]->data->value;
	unsigned long address = trainList[row]->data->address;
	Variant variant((long long)trainList[row]->data->value);
	float float_val = variant.asFloat();
	switch (col)
	{
	case DESC_COL:
		current_cell_widget = trainList[row]->choice;
		if (current_cell_widget != last_cell_widget) //click on a new widget
		{
			if (last_cell_widget) last_cell_widget->hide();
			trainList[row]->choice->show();
			trainList[row]->choice->value(0);
			trainList[row]->choice->take_focus();
		}
		last_cell_widget = current_cell_widget;
		break;
	case VALUE_COL: //input
		current_cell_widget = trainList[row]->value_input;
		if (current_cell_widget != last_cell_widget) //click on a new widget
		{
			if (last_cell_widget) last_cell_widget->hide();
			trainList[row]->value_input->show();
			trainList[row]->value_input->take_focus();
			trainList[row]->value_input->setValueType(trainList[row]->data->type);
			if (trainList[row]->data->type == SEARCH_VALUE_TYPE_FLOAT)
				trainList[row]->value_input->value(to_string(float_val).c_str());
			else
			{
				switch (trainList[row]->data->type)
				{
				case SEARCH_VALUE_TYPE_1BYTE:
					trainList[row]->value_input->value(trainList[row]->data->sign ? to_string((char)int_val).c_str() : to_string((unsigned char)int_val).c_str());
					break;
				case SEARCH_VALUE_TYPE_2BYTE:
					trainList[row]->value_input->value(trainList[row]->data->sign ? to_string((short)int_val).c_str() : to_string((unsigned short)int_val).c_str());
					break;
				default:
					trainList[row]->value_input->value(trainList[row]->data->sign ? to_string((long)int_val).c_str() : to_string((unsigned long)int_val).c_str());
					break;
				}
			}
		}
		last_cell_widget = current_cell_widget;
		break;
	}
}

void TrainerItemCodeTable::onCellRightClicked(int row, int col)
{

}


void TrainerItemCodeTable::onTableClicked()
{
	if (last_cell_widget) last_cell_widget->hide();
	last_cell_widget = 0;
}

vector<int> TrainerItemCodeTable::getSelectedRows() 
{
	vector<int> v;
	for (int i=0; i<rows(); i++)
	{
		if (row_selected(i))
			v.push_back(i);
	}
	return v;
}

void TrainerItemCodeTable::doDeactivate()
{
	 if (last_cell_widget) last_cell_widget->hide(); 
	 last_cell_widget = 0; 
	 if (!Fl::event_inside(this->parent()))
		select_all_rows(0);
}

void TrainerItemCodeTable::deleteCode()
{
	auto sel = getSelectedRows();
	int start = -1;
	int i=0;
	if (sel.size() == 0)
		return;
	auto code = trainList.begin();
	for (auto it = sel.begin(); it != sel.end(); ++it)
	{
		(*(code+*it))->_delete = 1;
	}
	for (auto it = trainList.begin(); it != trainList.end(); ++i)
	{
		if ((*it)->_delete)
		{
			if (start < 0) start = i;
			it = trainList.erase(it);
		}
		else
			++it;
	}
	rows(trainList.size());
	if (trainList.size() == 0)
	{
		createEntry();

	}
	select_all_rows(0);
	if (rows()-1 < start-1 || (start-1) < 0)
		select_row(rows()-1);
	else
		select_row(start-1);
	TrainerItemWindow::OnCodeTableUpdate(this, true);
}

Fl_Menu_Item *TrainerItemCodeTable::copyMenu(Fl_Menu_Item *m)
{
	Fl_Menu_Item *new_menu = new Fl_Menu_Item[55];//codeList.size()+1];
	memcpy(new_menu, m, sizeof(Fl_Menu_Item)*55);
	return new_menu;
}

void TrainerItemCodeTable::selectRow(int row)
{
	selectedRow = row;
	Fl::add_timeout(0.01, selectionTimeout, this);
}

void TrainerItemCodeTable::selectionTimeout(void *data)
{
	TrainerItemCodeTable *table = (TrainerItemCodeTable*)data;
	table->select_all_rows(0);
	table->select_row(table->getSelectedRow());
	Fl::remove_timeout(selectionTimeout);
	TrainerItemWindow::OnCodeTableUpdate(table);
}

bool TrainerItemCodeTable::isEmpty()
{
	if (rows() == 1 && !trainList.empty())
	{
		if (trainList[0]->menu[0].visible())
			return true;
	}
	return false;
}

bool TrainerItemCodeTable::canAdd()
{
	if (trainList.size() > 0 && trainList[trainList.size()-1]->choice->mvalue() == &(trainList[trainList.size()-1]->menu[0]))
		return false;
	return true;
}

void TrainerItemCodeTable::addCodes(AddressList codes)
{
	int j=0;
	for (auto it = codes.begin(); it !=codes.end(); ++it)
	{
		createEntry();
		auto last = trainList.back();
		last->data = (*it);
		last->value_input->setValue(last->data->value, last->data->type, false);
		j=0;
		for (auto jt = codeList.begin(); jt != codeList.end(); ++jt, ++j)
		{
			if ( *(*jt)->m_address == *(*it))
			{
				last->choice->value(j+1);
				last->menu[0].hide();
			}
		}
	}
	rebuildTable();
	TrainerItemWindow::OnCodeTableUpdate(this, true);
}