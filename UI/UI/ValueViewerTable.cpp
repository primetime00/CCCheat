#include "ValueViewerTable.h"
#include "CCAPI/Common.h"
#include "CCAPI/Helpers.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include "ui_main.h"

using namespace std;

#define ADDRESS_COL		0
#define VALUE_COL		1
#define HEX_COL			2	

#define VALUE_COLS		3



ValueViewerTable::ValueViewerTable(int X, int Y, int W, int H, const char *l) : Fl_Table_Row(X,Y,W,H,l) {

	m_operator = 0;
	m_selectedType = SEARCH_VALUE_TYPE_1BYTE;
	rkWindow::addObject(this);

	rows(DEFAULT_ROWS);
	row_header(0);
	row_height_all(25);
	row_resize(0);

	cols(VALUE_COLS);
	col_header(1);
	col_width_all(110);
	col_width(VALUE_COL, 250);
	col_resize(1);

	viewer = 0;
	m_codeTable = 0;
	m_startAddress = m_focusAddress = 0;
	m_signed = false;

	type(SELECT_MULTI);
	end();
	callback(tableClickedCB);

	Fl::add_timeout(0.5, redraw, this);
}

void ValueViewerTable::redraw(void *v)
{
	ValueViewerTable *t = (ValueViewerTable *)v;
	t->damage(FL_DAMAGE_ALL);
	Fl::repeat_timeout(1.0, ValueViewerTable::redraw, v);
}

void ValueViewerTable::addSelectedAddresses()
{
	if (m_codeTable == 0 || !hasSelection())
		return;
	for (int i=0; i<rows(); ++i)
	{
		if (i > DEFAULT_ROWS)
			break;
		if (row_selected(i))
		{
			m_codeTable->addEntry("Value Viewer Entry", make_shared<AddressObj>(m_startAddress+i, 0, m_selectedType, m_signed ? 1 : 0));
		}
	}
}

bool ValueViewerTable::hasSelection()
{
	int t,b,l,r;
	get_selection(t,l,b,r);
	return (t > -1 && b > -1);
}

void ValueViewerTable::draw_cell(TableContext context, int ROW, int COL, int X, int Y, int W, int H){
    static char s[400];
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
		case ADDRESS_COL:	DrawHeader("Address",X,Y,W,H); break;
		case VALUE_COL:		DrawHeader("Value",X,Y,W,H); break;
		case HEX_COL:		DrawHeader("Hex",X,Y,W,H); break;
		default: break;
		}
        return; 
      case CONTEXT_ROW_HEADER:                  // Draw row headers
		fl_font(FL_HELVETICA, 14);              // set the font for our drawing operations
        sprintf(s,"%03d:",COL);                 // "001:", "002:", etc
        DrawHeader(s,X,Y,W,H);
        return; 
      case CONTEXT_CELL:    // Draw data in cells
		if (ROW > DEFAULT_ROWS)
			break;
		fl_font(FL_HELVETICA, 14);              // set the font for our drawing operations
	    fl_push_clip(X, Y, W, H);
	    {
	        // BG COLOR
		if ((unsigned)ROW == (m_focusAddress - m_startAddress) )
			fl_color( row_selected(ROW) ? selection_color() : FL_GREEN);
		else
			fl_color( row_selected(ROW) ? selection_color() : FL_WHITE);
		fl_rectf(X, Y, W, H);

		// TEXT
		fl_color(FL_BLACK);
		if ((unsigned int)ROW < DEFAULT_ROWS)
		{
			Variant variant;
			unsigned long tmpLong = BSWAP32((unsigned long)variant.convertToLong((char*)(&m_memory[ROW])));
			unsigned short tmpShort = BSWAP16((unsigned short)variant.convertToShort((char*)(&m_memory[ROW])));
			Variant var2((long)tmpLong);
			switch(COL)
			{
			case ADDRESS_COL: sprintf(s,"0x%08lX",m_startAddress+ROW); break;
			case VALUE_COL: 
				switch (m_selectedType)
				{
				case SEARCH_VALUE_TYPE_FLOAT: 
					sprintf(s,"%f", var2.asFloat() ); break;
				case SEARCH_VALUE_TYPE_1BYTE: 
					if (m_signed)
						sprintf(s,"%hd", (char)m_memory[ROW]); 
					else
						sprintf(s,"%hu", (unsigned char)m_memory[ROW]); 
					break;					
				case SEARCH_VALUE_TYPE_2BYTE:
					if (m_signed) 
						 sprintf(s,"%hd", (short) tmpShort); 
					 else
						 sprintf(s,"%hu", (unsigned short) tmpShort); 
					 break;					
				default:
					if (m_signed) 
						sprintf(s,"%ld", (long) tmpLong); 
					else
						sprintf(s,"%lu", (unsigned long) tmpLong); 
					break;
				} break;
			case HEX_COL: 
				switch (m_selectedType)
				{
				case SEARCH_VALUE_TYPE_FLOAT: 
					sprintf(s,"%lX", tmpLong); break;
				case SEARCH_VALUE_TYPE_1BYTE: 
						 sprintf(s,"%X", (unsigned char)m_memory[ROW]); 
						 break;					
				case SEARCH_VALUE_TYPE_2BYTE: 
						 sprintf(s,"%X", tmpShort); 
						 break;					
				default:
						 sprintf(s,"%lX", tmpLong); 
						 break;
				} break;
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

int ValueViewerTable::handle(int evt)
{
	int res = 0;
	when(FL_WHEN_NEVER);
	res = Fl_Table_Row::handle(evt);
	return res;
}
void ValueViewerTable::DrawData(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    // Draw cell bg
    //fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
    // Draw cell data
    fl_color(FL_GRAY0); fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
    // Draw box border
    fl_color(color()); fl_rect(X,Y,W,H);
fl_pop_clip();
}

void ValueViewerTable::DrawHeader(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, row_header_color());
    fl_color(FL_BLACK);
    fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
fl_pop_clip();
}

void ValueViewerTable::onCellDoubleClicked(int row, int col)
{
#if 0
	long long int_val; 
	if (data[row].sign)
		int_val = (signed long)data[row]->value;
	else
		int_val = (unsigned long) data[row]->value;
	unsigned long address = data[row]->address;
	char hex[50];
	float float_val = *(float*)&data[row]->value;
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
		current_cell_widget = data[row]->widget->address_input;
		if (current_cell_widget != last_cell_widget) //click on a new widget
		{
			if (last_cell_widget) last_cell_widget->hide();
			data[row]->widget->address_input->show();
			sprintf(hex, "%.8X", address);
			data[row]->widget->address_input->value(hex);
			data[row]->widget->address_input->take_focus();
		}
		last_cell_widget = current_cell_widget;
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
					data[row]->widget->value_input->value(data[row]->sign ? to_string((char)int_val).c_str() : to_string((unsigned char)int_val).c_str());
					break;
				case SEARCH_VALUE_TYPE_2BYTE:
					data[row]->widget->value_input->value(data[row]->sign ? to_string((short)int_val).c_str() : to_string((unsigned short)int_val).c_str());
					break;
				default:
					data[row]->widget->value_input->value(data[row]->sign ? to_string((long)int_val).c_str() : to_string((unsigned long)int_val).c_str());
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
#endif
}

void ValueViewerTable::onCellRightClicked(int row, int col)
{
#if 0
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
#endif
}


void ValueViewerTable::onTableClicked()
{
}

void ValueViewerTable::doDeactivate()
{
	 //if (!Fl::event_inside(this->parent()))
//		select_all_rows(0);
}

void ValueViewerTable::setMemoryOperator(MemoryOperator *op)
{ 
	m_operator = op;
}

void ValueViewerTable::startMemoryRead()
{
	if (m_operator == 0)
		return;
	m_startAddress = 0;
	if ((long long)m_focusAddress - (DEFAULT_ROWS/2) < 0)
		m_startAddress = 0;
	else if ((long long)m_focusAddress + (DEFAULT_ROWS/2) > 0xFFFFFFFF)
		m_startAddress = 0xFFFFFFFF - (DEFAULT_ROWS/2);
	else
		m_startAddress = m_focusAddress - (DEFAULT_ROWS/2);
	m_operator->setChunkReadMemoryOperation(m_startAddress, DEFAULT_ROWS+3, m_memory, true);
	row_position(m_focusAddress - m_startAddress);
}

void ValueViewerTable::stopMemoryRead()
{
	if (m_operator == 0)
		return;
	m_operator->removeChunkReadOperation(m_startAddress);
}
