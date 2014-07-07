#include "CodeTable.h"
#include "CCAPI/Common.h"
#include "CCAPI/Helpers.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include "ui_main.h"

using namespace std;

#define CODE_COLS		5
#define DESC_COL		0
#define ADDRESS_COL		1
#define VALUE_COL		2
#define TYPE_COL		3
#define FREEZE_COL		4


CodeTable::CodeTable(int X, int Y, int W, int H, const char *l) : Fl_Table_Row(X,Y,W,H,l) {

	last_cell_widget = 0;
	m_operator = 0;
	rkWindow::addObject(this);

	rows(0);
	row_header(0);
	row_height_all(20);
	row_resize(0);

	cols(CODE_COLS);
	col_header(1);
	col_width_all(110);
	col_width(DESC_COL, 330);
	col_width(VALUE_COL, 250);
	col_width(ADDRESS_COL, 200);
	col_width(FREEZE_COL, 90);
	col_resize(1);
	unsigned int width=0;
	for (int i=0; i<cols(); ++i)
		width += col_width(i);
	w(width+3);

	viewer = 0;

	type(SELECT_MULTI);
	end();
	callback(tableClickedCB);

	Fl::add_timeout(1.0, redraw, this);
}

void CodeTable::redraw(void *v)
{
	CodeTable *t = (CodeTable *)v;
	t->damage(FL_DAMAGE_ALL);
	Fl::repeat_timeout(1.0, CodeTable::redraw, v);
}

void CodeTable::addEntry(string desc, AddressItem item, bool freeze)
{
	bool found = false;
	if (item->address != 0)
	{
		for (unsigned int i=0; i<data.size(); i++)
		{
			if (!data.at(i)->m_address->isPointer() && !item->isPointer() && data.at(i)->m_address->address == item->address) //we have this
			{
				found = true;
				break;
			}
			else if (data.at(i)->m_address->isPointer() && item->isPointer() && data.at(i)->m_address->address == item->address && data.at(i)->m_address->pointer->equal(*item->pointer))
			{
				found = true;
				break;
			}
		}
	}
	if (!found)
	{
		int x,y,w,h;
		begin();
		rows(data.size()+1);

		find_cell(Fl_Table::CONTEXT_TABLE, data.size(), DESC_COL, x,y,w,h);
		Fl_Input *desc_input = new Fl_Input(x,y,w,h);
		desc_input->maximum_size(26);
		desc_input->callback((Fl_Callback*)codeDescriptionChangedCB);
		desc_input->when(FL_WHEN_CHANGED);
		desc_input->hide();

		ValueInput *address_input= 0;
		if (!item->isPointer())
		{
			find_cell(Fl_Table::CONTEXT_TABLE, data.size(), ADDRESS_COL, x,y,w,h);
			address_input = new ValueInput(x,y,w,h);
			address_input->setHex(true);
			address_input->maximum_size(8);
			address_input->callback((Fl_Callback*)codeAddressChangedCB);
			address_input->when(FL_WHEN_RELEASE);
			address_input->hide();
		}

		find_cell(Fl_Table::CONTEXT_TABLE, data.size(), VALUE_COL, x,y,w,h);
		ValueInput *value_input = new ValueInput(x,y,w,h);
		value_input->setHex(false);
		value_input->setLiteral(true);
		value_input->setValueType(item->type);
		value_input->callback((Fl_Callback*)codeValueChangedCB);
		value_input->when(FL_WHEN_RELEASE);
		value_input->setCodeType(true);
		value_input->hide();

		find_cell(Fl_Table::CONTEXT_TABLE, data.size(), TYPE_COL, x,y,w,h);
		Fl_Choice *type_choice = new Fl_Choice(x,y,w,h);
		type_choice->menu(rkCheatUI::menu_ui_valueType);
		type_choice->callback((Fl_Callback*)codeTypeChangedCB);
		type_choice->when(FL_WHEN_CHANGED);
		type_choice->hide();
		for (int i=0; rkCheatUI::menu_ui_valueType[i].text != 0; ++i)
		{
			if ( get_user_data(char, rkCheatUI::menu_ui_valueType[i].user_data()) == item->type )
			{
				type_choice->value(i);
				break;
			}
		}		
		find_cell(Fl_Table::CONTEXT_TABLE, data.size(), FREEZE_COL, x,y,w,h);
		Fl_Check_Button *freeze_check = new Fl_Check_Button(x+(w-20)/2,y,20,h);
		freeze_check->align(FL_ALIGN_CENTER);
		freeze_check->callback((Fl_Callback*)codeFreezeChangedCB);
		freeze_check->when(FL_WHEN_CHANGED);
		freeze_check->value(freeze ? 1: 0);

		end();
		rkCheat_CodeItem cItem = make_shared<rkCheat_Code>(desc, item, freeze, new WidgetField(desc_input, address_input, value_input, type_choice,freeze_check));
		data.push_back(cItem);

		desc_input->user_data(cItem.get());
		if (!item->isPointer())
			address_input->user_data(cItem.get());
		type_choice->user_data(cItem.get());
		freeze_check->user_data(cItem.get());
		value_input->user_data(cItem.get());

		cItem->setSign(Helpers::isSigned(item->type, item->value));
	
		if (m_operator != 0)
		{
			m_operator->setReadMemoryOperation(item, true);
		}
		rebuildTable();


	}

}

void CodeTable::deleteEntry(unsigned int row)
{
	if (m_operator != 0)
	{
		rkCheat_CodeList::iterator it = data.begin()+row;
		m_operator->removeMemoryOperation(MEMORY_COMMAND_READ, (*it)->m_address);
		m_operator->removeMemoryOperation(MEMORY_COMMAND_WRITE, (*it)->m_address);
	}

	if (data.size() == 1) //we are getting rid of the last entry
	{
		clearTable();
	}
	else
	{
		if (!data.at(row)->m_address->isPointer())
			this->remove(*data.at(row)->widget->address_input);
		this->remove(*data.at(row)->widget->description);
		this->remove(*data.at(row)->widget->freeze);
		this->remove(*data.at(row)->widget->type);
		this->remove(*data.at(row)->widget->value_input);
		data.erase(data.begin()+row, data.begin()+row+1);
		rebuildTable();
		rows(data.size());
	}
}

void CodeTable::deleteEntry(vector<int> &indexRows)
{
	rkCheat_CodeList cl;
	if (indexRows.size() >= data.size())
	{
		if (m_operator != 0)
		{
			for (rkCheat_CodeList::iterator it = data.begin(); it != data.end(); ++it)
			{
				m_operator->removeMemoryOperation(MEMORY_COMMAND_READ, (*it)->m_address);
				m_operator->removeMemoryOperation(MEMORY_COMMAND_WRITE, (*it)->m_address);
			}
		}
		clearTable();
	}
	else
	{
		for (vector<int>::iterator it = indexRows.begin(); it!= indexRows.end(); ++it) //iterate through index		
		{
			data[*it]->_delete = 1;
		}
		for (rkCheat_CodeList::iterator it = data.begin(); it != data.end();)
		{
			if ((*it)->_delete == 1)
			{
				m_operator->removeMemoryOperation(MEMORY_COMMAND_READ, (*it)->m_address);
				m_operator->removeMemoryOperation(MEMORY_COMMAND_WRITE, (*it)->m_address);

				if (!(*it)->m_address->isPointer())
					remove(*(*it)->widget->address_input);
				remove(*(*it)->widget->description);
				remove(*(*it)->widget->freeze);
				remove(*(*it)->widget->type);
				remove(*(*it)->widget->value_input);
				it = data.erase(it);
			}
			else
				it++;
		}
		rebuildTable();
		rows(data.size());
	}
}

void CodeTable::reInsert()
{
	int j=0;
	for (unsigned int i=0; i<data.size(); i++)
	{
		insert( *((Fl_Widget*)data[i]->widget->description), j++);
		if (!data[i]->m_address->isPointer())
			insert( *((Fl_Widget*)data[i]->widget->address_input), j++);
		insert( *((Fl_Widget*)data[i]->widget->value_input), j++);
		insert( *((Fl_Widget*)data[i]->widget->type), j++);
		insert( *((Fl_Widget*)data[i]->widget->freeze), j++);
	}
}

void CodeTable::rebuildTable()
{
	int x,y,w,h;
	for (unsigned int i=0; i<data.size(); i++)
	{
		find_cell(CONTEXT_TABLE, i, DESC_COL, x,y,w,h);
		((Fl_Widget*)data[i]->widget->description)->position(x, y);
		if (!data[i]->m_address->isPointer())
		{
			find_cell(CONTEXT_TABLE, i, ADDRESS_COL, x,y,w,h);
			((Fl_Widget*)data[i]->widget->address_input)->position(x, y);
		}
		find_cell(CONTEXT_TABLE, i, VALUE_COL, x,y,w,h);
		((Fl_Widget*)data[i]->widget->value_input)->position(x, y);
		find_cell(CONTEXT_TABLE, i, TYPE_COL, x,y,w,h);
		data[i]->widget->type->position(x, y);
		find_cell(CONTEXT_TABLE, i, FREEZE_COL, x,y,w,h);
		data[i]->widget->freeze->position(x+(w-20)/2,y);
	}
}

bool CodeTable::hasSelection()
{
	int t,b,l,r;
	get_selection(t,l,b,r);
	return (t > -1 && b > -1);
}

void CodeTable::draw_cell(TableContext context, int ROW, int COL, int X, int Y, int W, int H){
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
				if (c == FREEZE_COL)
					child(index)->resize(X+(W-20)/2,Y, W, H);
				else
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
		case DESC_COL:		DrawHeader("Description",X,Y,W,H); break;
		case ADDRESS_COL:	DrawHeader("Address",X,Y,W,H); break;
		case VALUE_COL:		DrawHeader("Value",X,Y,W,H); break;
		case TYPE_COL:		DrawHeader("Type",X,Y,W,H); break;
		case FREEZE_COL:	DrawHeader("Frozen",X,Y,W,H); break;
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
		if (COL == FREEZE_COL)
			return;
		if (COL == DESC_COL && data[ROW]->widget->description->visible())
			return;
		if (COL == ADDRESS_COL && !data[ROW]->m_address->isPointer() && data[ROW]->widget->address_input->visible())
			return;
		else if (COL == VALUE_COL && data[ROW]->widget->value_input->visible())
			return;
		else if (COL == TYPE_COL && data[ROW]->widget->type->visible())
			return;
	    fl_push_clip(X, Y, W, H);
	    {
	        // BG COLOR
		fl_color( row_selected(ROW) ? selection_color() : FL_WHITE);
		fl_rectf(X, Y, W, H);

		// TEXT
		fl_color(FL_BLACK);
		if ((unsigned int)ROW < data.size())
		{
			Variant variant((long long)data[ROW]->m_address->value);
			switch(COL)
			{
			case DESC_COL: 
				if (data[ROW]->description.empty())
					sprintf(s,"No Description");
				else
					sprintf(s,"%s",data[ROW]->description.c_str()); 
				break;
			case ADDRESS_COL: 
				if (data[ROW]->m_address->isPointer())
					sprintf(s,"0x%08lX -> 0x%08lX",data[ROW]->m_address->address, data[ROW]->m_address->pointer->resolved);
				else
					sprintf(s,"0x%08lX",data[ROW]->m_address->address);
				break;
			case VALUE_COL: 
				switch (data[ROW]->m_address->type)
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
				} break;
			case TYPE_COL: 
				switch (data[ROW]->m_address->type)
				{
				case SEARCH_VALUE_TYPE_1BYTE: sprintf(s,"1 Byte"); break;
				case SEARCH_VALUE_TYPE_2BYTE: sprintf(s,"2 Bytes"); break;
				case SEARCH_VALUE_TYPE_FLOAT: sprintf(s,"Float"); break;
				default: sprintf(s,"4 Bytes"); break;
				} break;
			//case 3: DrawCheckbox(X,Y,W,H); break;//sprintf(s,"%s",get<3>(data[ROW]) ? "Frozen" : ""); break;
			default: break;
			}
			if (COL != FREEZE_COL)
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

int CodeTable::handle(int evt)
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
void CodeTable::DrawData(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    // Draw cell bg
    //fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
    // Draw cell data
    fl_color(FL_GRAY0); fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
    // Draw box border
    fl_color(color()); fl_rect(X,Y,W,H);
fl_pop_clip();
}

void CodeTable::saveData(string filename)
{
	fstream f(filename, ios_base::out | ios_base::trunc | ios_base::binary);
	f << CCCHEAT_VERSION << "\n";
	for (rkCheat_CodeList::iterator it = data.begin(); it != data.end(); ++it)
	{
		f << (*it)->description << "\n";
		if ((*it)->m_address->isPointer())
		{
			f << (*it)->m_address->pointer->pointers.size() << " ";
			for (auto pt = (*it)->m_address->pointer->pointers.begin(); pt != (*it)->m_address->pointer->pointers.end(); ++pt)
			{
				f << hex << pt->offset << " ";
			}
		}
		else
			f << "0 ";

		f << hex << (*it)->m_address->address << " " << dec << (*it)->m_address->value << " " << (*it)->m_address->type << " " << (*it)->freeze << "\n";
	}
	f.close();
}

bool CodeTable::loadData(string filename)
{
	int offsetSize = 0;
	fstream f(filename, ios_base::in | ios_base::binary);
	if (!f)
		return false;
	clearTable();
	char version[255];
	char desc[255];
	f.getline(version, 255);
	double v = stod(string(version));
	int major = (int) v;
	int minor = (int)((v*100)) % 100;
	v = stod(string(CCCHEAT_VERSION));
	int this_minor = (int)((v*100)) % 100;
	if (major != 1 || minor < 10 || minor > this_minor)
		return false;
	while (true) 
	{
		unsigned long addr, value;
		char type;
		bool froze;
		PointerOffsets po;
		f.getline(desc, 255);
		f >> offsetSize;
		for (int i=0; i<offsetSize; i++)
		{
			unsigned int val;
			f >> hex >> val;
			po.push_back(val);
		}
		f >> hex >> addr >> dec >> value >> type >> froze;
		froze = false; //for now, we don't save the freeze status.
		f.ignore();
		if( f.eof() ) break;
		if (po.size() > 0)
			addEntry(desc, make_shared<AddressObj>(addr, po, type, 0), froze);
		else
			addEntry(desc, make_shared<AddressObj>(addr, value, type, 0), froze);
	}
	f.close();
	//rows(data.size());
	return true;
}

void CodeTable::clearTable()
{
	int nChildren = children();
	for (int i=0; i<nChildren; ++i)
		remove(*child(0));
	init_sizes ();
	rows(0);
	data.clear();
}

void CodeTable::DrawHeader(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, row_header_color());
    fl_color(FL_BLACK);
    fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
fl_pop_clip();
}

void CodeTable::DrawCheckbox(int X, int Y, int W, int H)
{
	fl_push_clip(X,Y,W,H);

	fl_pop_clip();
}

void CodeTable::onDescriptionChanged(rkCheat_Code *item, string value)
{
	item->description = value;
}

void CodeTable::onAddressChanged(rkCheat_Code *item, unsigned long value)
{
	if (m_operator != 0)
	{
		m_operator->removeMemoryOperation(MEMORY_COMMAND_READ, item->m_address);
		m_operator->removeMemoryOperation(MEMORY_COMMAND_WRITE, item->m_address);
	}
	item->m_address->address = value;
	item->freeze = false;
	item->widget->freeze->value(0);
	if (m_operator != 0)
	{
		m_operator->setReadMemoryOperation(item->m_address, true);
	}
}

void CodeTable::onValueChanged(rkCheat_Code *item, long long value)
{
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
}

void CodeTable::onCodeTypeChanged(rkCheat_Code *item, int type)
{
	long long int_val;
	float float_val;
	int oldtype = item->type;
	item->type = type;
	int_val = Helpers::convertValueType(item->m_address->value, type, oldtype, item->m_address->sign ? true : false);
	Variant variant(int_val);
	if (type != SEARCH_VALUE_TYPE_FLOAT)
	{
		item->widget->value_input->value(to_string(int_val).c_str());
	}
	else
	{
		float_val = variant.asFloat();
		item->widget->value_input->value(to_string(float_val).c_str());
	}
	item->widget->value_input->setValueType(type);
	item->m_address->value = item->widget->value_input->getLLValue();
	item->setSign(Helpers::isSigned(item->type, item->m_address->value));
	item->m_address->type = item->type;
	if (m_operator != 0)
	{
		m_operator->setReadMemoryOperation(item->m_address, true);
	}
}
void CodeTable::onCodeFreezeChanged(rkCheat_Code *item, int value)
{
	item->freeze = value > 0 ? true : false;
	if (m_operator != 0)
	{
		if (item->freeze)
			m_operator->setWriteMemoryOperation(item->m_address, item->m_address->value, true);
		else
			m_operator->removeMemoryOperation(MEMORY_COMMAND_WRITE, item->m_address);
	}
}

void CodeTable::onCellDoubleClicked(int row, int col)
{
	long long int_val; 
	if (data[row]->m_address->sign)
		int_val = (signed long)data[row]->m_address->value;
	else
		int_val = (unsigned long) data[row]->m_address->value;
	unsigned long address = data[row]->m_address->address;
	Variant variant((long long)data[row]->m_address->value);
	char hex[50];
	float float_val = variant.asFloat();
	switch (col)
	{
	case DESC_COL:
		current_cell_widget = data[row]->widget->description;
		if (current_cell_widget != last_cell_widget) //click on a new widget
		{
			if (last_cell_widget) last_cell_widget->hide();
			data[row]->widget->description->show();
			data[row]->widget->description->value(data[row]->description.c_str());
			data[row]->widget->description->take_focus();
		}
		last_cell_widget = current_cell_widget;
		break;
	case ADDRESS_COL: //address
		if (data[row]->m_address->isPointer()) //bring up the pointer editor!
		{
			current_cell_widget = 0;
			if (m_operator != 0)
			{
				AddressItem p = uiInstance->m_peWindow->popup(data[row]->m_address);
				if (p != nullptr)
				{
					m_operator->removeMemoryOperation(MEMORY_COMMAND_WRITE, data[row]->m_address);
					m_operator->removeMemoryOperation(MEMORY_COMMAND_READ, data[row]->m_address);
					data[row]->m_address = p;
					data[row]->widget->freeze->value(0);
					data[row]->freeze = false;
					m_operator->setReadMemoryOperation(data[row]->m_address, true);
				}
			}
		}
		else
		{
			current_cell_widget = data[row]->widget->address_input;
			if (current_cell_widget != last_cell_widget) //click on a new widget
			{
				if (last_cell_widget) last_cell_widget->hide();
				data[row]->widget->address_input->show();
				sprintf(hex, "%.8lX", address);
				data[row]->widget->address_input->value(hex);
				data[row]->widget->address_input->take_focus();
			}
			last_cell_widget = current_cell_widget;
		}
		break;
	case VALUE_COL: //input
		current_cell_widget = data[row]->widget->value_input;
		if (current_cell_widget != last_cell_widget) //click on a new widget
		{
			if (last_cell_widget) last_cell_widget->hide();
			data[row]->widget->value_input->show();
			data[row]->widget->value_input->take_focus();
			if (data[row]->type == SEARCH_VALUE_TYPE_FLOAT)
				data[row]->widget->value_input->value(to_string(float_val).c_str());
			else
			{
				switch (data[row]->type)
				{
				case SEARCH_VALUE_TYPE_1BYTE:
					data[row]->widget->value_input->value(data[row]->m_address->sign ? to_string((char)int_val).c_str() : to_string((unsigned char)int_val).c_str());
					break;
				case SEARCH_VALUE_TYPE_2BYTE:
					data[row]->widget->value_input->value(data[row]->m_address->sign ? to_string((short)int_val).c_str() : to_string((unsigned short)int_val).c_str());
					break;
				default:
					data[row]->widget->value_input->value(data[row]->m_address->sign ? to_string((long)int_val).c_str() : to_string((unsigned long)int_val).c_str());
					break;
				}
			}
		}
		last_cell_widget = current_cell_widget;
		break;
	case TYPE_COL: //type
		current_cell_widget = data[row]->widget->type;
		if (current_cell_widget != last_cell_widget) //click on a new widget
		{
			if (last_cell_widget) last_cell_widget->hide();
			data[row]->widget->type->show();
			data[row]->widget->type->take_focus();
		}
		last_cell_widget = current_cell_widget;
		break;
	case FREEZE_COL: //freeze
		current_cell_widget = data[row]->widget->freeze;
		if (current_cell_widget != last_cell_widget) //click on a new widget
		{
			if (last_cell_widget) last_cell_widget->hide();
			data[row]->widget->freeze->take_focus();
		}
		last_cell_widget = 0;
		break;
	}
}

void CodeTable::onCellRightClicked(int row, int col)
{
	const Fl_Menu_Item *m = 0;
	if (col == ADDRESS_COL)
	{
		Fl_Menu_Item rclick_menu[] = {
			{ "View Nearby...",   0, viewer,  (void*)this },
            { 0 }
            };
            m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, 0);
			//if ( m ) m->do_callback(0, m->user_data());
            if ( m ) m->do_callback(0, data[row].get());
	}
}


void CodeTable::onTableClicked()
{
	if (last_cell_widget) last_cell_widget->hide();
	last_cell_widget = 0;
}

void CodeTable::newCode()
{
	int choice = fl_choice("Would you like to add a new address or a new pointer?", "Cancel", "Address", "Pointer");
	if (choice == 1)
		addEntry("", make_shared<AddressObj>());
	else if (choice == 2)
	{
		PointerOffsets po;
		po.push_back(0);
		AddressItem p = uiInstance->m_peWindow->popup(0x0000, po);
		if (p != nullptr)
			addEntry("", p);
	}
}

vector<int> CodeTable::getSelectedRows() 
{
	vector<int> v;
	for (int i=0; i<rows(); i++)
	{
		if (row_selected(i))
			v.push_back(i);
	}
	return v;
}

void CodeTable::deleteCodes()
{
	vector<int> indexRows = getSelectedRows();
	if (indexRows.size() > 1)
		deleteEntry(indexRows);
	else if (indexRows.size() == 1)
		deleteEntry(indexRows.at(0));
	select_all_rows(0);
}

void CodeTable::addResults(vector<AddressItem> &items, char type)
{
	for (vector<AddressItem>::iterator it = items.begin(); it != items.end(); ++it)
	{
		(*it)->type = type;
		addEntry("", *it);
	}
}

void CodeTable::doDeactivate()
{
	 if (last_cell_widget) last_cell_widget->hide(); 
	 last_cell_widget = 0; 
	 if (!Fl::event_inside(this->parent()))
		select_all_rows(0);
}

void CodeTable::setMemoryOperator(MemoryOperator *op)
{ 
	m_operator = op;
	if (op != 0)
	{
		for (rkCheat_CodeList::iterator it = data.begin(); it != data.end(); ++it)
		{
			m_operator->setReadMemoryOperation((*it)->m_address, true);
		}
	}
}
