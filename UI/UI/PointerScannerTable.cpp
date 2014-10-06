#include "PointerScannerTable.h"
#include "CCAPI/Common.h"
#include "CCAPI/Helpers.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include <algorithm>
#include "ui_main.h"

using namespace std;

#define ADDRESS_COL			0
#define OFFSET_COL			1
#define DEPTH_COL			2	
#define RESULT_COL			3	

#define VALUE_COLS			4



PointerScannerTable::PointerScannerTable(int X, int Y, int W, int H, const char *l) : Fl_Table_Row(X,Y,W,H,l) {

	rkWindow::addObject(this);
	unsigned int width = 0;
	//rows(DEFAULT_PT_ROWS);
	rows(1);
	row_header(0);
	row_height_all(25);
	row_resize(0);

	cols(VALUE_COLS);
	col_header(1);
	col_width_all(110);
	col_width(OFFSET_COL, 100);
	col_width(DEPTH_COL, 80);
	col_resize(1);

	m_resultAddress = 0;

	type(SELECT_SINGLE);
	for (int i=0; i<cols(); ++i)
		width += col_width(i);
	resize(x(), y(), width+4, h());
	end();
	callback(tableClickedCB);

}

bool PointerScannerTable::hasSelection()
{
	int t,b,l,r;
	get_selection(t,l,b,r);
	return (t > -1 && b > -1);
}

void PointerScannerTable::draw_cell(TableContext context, int ROW, int COL, int X, int Y, int W, int H){
    static char s[40];
	stringstream str;
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
		case OFFSET_COL:	DrawHeader("Offset",X,Y,W,H); break;
		case DEPTH_COL:		DrawHeader("Depth",X,Y,W,H); break;
		case RESULT_COL:	DrawHeader("Result",X,Y,W,H); break;
		default: break;
		}
        return; 
      case CONTEXT_ROW_HEADER:                  // Draw row headers
		fl_font(FL_HELVETICA, 14);              // set the font for our drawing operations
        sprintf(s,"%03d:",COL);                 // "001:", "002:", etc
        DrawHeader(s,X,Y,W,H);
        return; 
      case CONTEXT_CELL:    // Draw data in cells
		if (ROW > DEFAULT_PT_ROWS)
			break;
		if (m_pointerList.size() == 0 || (unsigned)ROW >= m_pointerList.size())
			break;
		fl_font(FL_HELVETICA, 14);              // set the font for our drawing operations
	    fl_push_clip(X, Y, W, H);
	    {
        // BG COLOR
		fl_color( row_selected(ROW) ? selection_color() : FL_WHITE);
		fl_rectf(X, Y, W, H);

		// TEXT
		fl_color(FL_BLACK);
		switch(COL)
		{
		case ADDRESS_COL: sprintf(s,"0x%08lX",m_pointerList[ROW]->address); break;
		case OFFSET_COL:
			str.clear();
			for (auto it=m_pointerList[ROW]->pointer->pointers.begin(); it != m_pointerList[ROW]->pointer->pointers.end(); ++it)
			{
				str << hex << it->offset << " ";
			}
			sprintf(s, "%s", str.str().c_str());
			break;
		case DEPTH_COL: sprintf(s,"%lu",m_pointerList[ROW]->pointer->pointers.size()); break;
		case RESULT_COL: sprintf(s,"0x%08lX",m_pointerList[ROW]->pointer->resolved); break;
		default: break;
		}
		DrawData(s,X,Y,W,H);
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

int PointerScannerTable::handle(int evt)
{
	int res = 0;
	when(FL_WHEN_NEVER);
	res = Fl_Table_Row::handle(evt);
	if (hasSelection())
		uiInstance->m_psAddPointerButton->activate();
	else
		uiInstance->m_psAddPointerButton->deactivate();
	return res;
}
void PointerScannerTable::DrawData(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    // Draw cell bg
    //fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
    // Draw cell data
    fl_color(FL_GRAY0); fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
    // Draw box border
    fl_color(color()); fl_rect(X,Y,W,H);
fl_pop_clip();
}

void PointerScannerTable::DrawHeader(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, row_header_color());
    fl_color(FL_BLACK);
    fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
fl_pop_clip();
}

void PointerScannerTable::onCellDoubleClicked(int row, int col)
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

void PointerScannerTable::onCellRightClicked(int row, int col)
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


void PointerScannerTable::onTableClicked()
{
}

void PointerScannerTable::doDeactivate()
{
	 //if (!Fl::event_inside(this->parent()))
//		select_all_rows(0);
}

void PointerScannerTable::addRow(unsigned long address, PointerOffsets offsets)
{
	bool found = false;
	for (auto it = m_pointerList.begin(); it != m_pointerList.end(); ++it)
	{
		if ((*it)->address == address)
		{
			PointerItem p = make_shared<PointerObj>(address, offsets);
			if ((*it)->pointer->equal(*p))
			{
				found = true;
				break;
			}
		}
	}
	if (!found)
	{
		AddressItem ai = make_shared<AddressObj>(address, offsets, SEARCH_VALUE_TYPE_4BYTE, 0);
		ai->pointer->resolved = m_resultAddress;
		m_pointerList.push_back(ai);
		rows(m_pointerList.size());
		damage(FL_DAMAGE_ALL);
	}
}

AddressItem PointerScannerTable::getSelectedPointer()
{
	if (hasSelection())
	{
		for (int i=0; i<rows(); ++i)
			if (row_selected(i))
				return m_pointerList[i];
	}
	return nullptr;
}
