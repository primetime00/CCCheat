#include "RangeTable.h"
#include <iostream>
#include <fstream>
#include <FL/fl_draw.H>

using namespace std;

RangeTable::RangeTable(int X, int Y, int W, int H, const char *l) : Fl_Table_Row(X,Y,W,H,l) {

	addEntry(0, 0xFFFFFFFF);
	//rows(RANGE_ROWS);
	row_header(0);
	row_height_all(20);
	row_resize(0);

	cols(RANGE_COLS);
	col_header(1);
	col_width_all(110);
	col_resize(1);

	type(SELECT_MULTI);
	end();
}

void RangeTable::addEntry(unsigned long start, unsigned long end)
{
	bool found = false;
	for (unsigned int i=0; i<data.size(); i++)
	{
		if (data.at(i).first == start && data.at(i).second == end) //we have this
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		data.push_back(make_pair(start,end));
		rows(data.size());
	}
}
void RangeTable::deleteEntry(unsigned int row)
{
	if (data.size() == 1) //we are getting rid of the last entry
	{
		clear();
		addEntry(0, 0xFFFFFFFF);
	}
	else
	{
		data.erase(data.begin()+row, data.begin()+row+1);
		rows(data.size());
	}
}

bool RangeTable::hasSelection()
{
	int t,b,l,r;
	get_selection(t,l,b,r);
	return (t > -1 && b > -1);
}

void RangeTable::draw_cell(TableContext context, int ROW, int COL, int X, int Y, int W, int H){
    static char s[40];
	fl_font(FL_HELVETICA, 14);              // set the font for our drawing operations
    switch ( context ) {
      case CONTEXT_STARTPAGE:                   // before page is drawn..
        return; 
      case CONTEXT_COL_HEADER:                  // Draw column headers
        if (COL == 0)
			DrawHeader("Begin",X,Y,W,H);
		else
			DrawHeader("End",X,Y,W,H);
        return; 
      case CONTEXT_ROW_HEADER:                  // Draw row headers
        sprintf(s,"%03d:",COL);                 // "001:", "002:", etc
        DrawHeader(s,X,Y,W,H);
        return; 
      case CONTEXT_CELL:    // Draw data in cells
	    fl_push_clip(X, Y, W, H);
	    {
	        // BG COLOR
		fl_color( row_selected(ROW) ? selection_color() : FL_WHITE);
		fl_rectf(X, Y, W, H);

		// TEXT
		fl_color(FL_BLACK);
		if ((unsigned int)ROW < data.size())
		{
			if (COL == 0)
				sprintf(s,"0x%08lX",data[ROW].first);
			else
				sprintf(s,"0x%08lX",data[ROW].second);
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

void RangeTable::DrawData(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    // Draw cell bg
    //fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
    // Draw cell data
    fl_color(FL_GRAY0); fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
    // Draw box border
    fl_color(color()); fl_rect(X,Y,W,H);
fl_pop_clip();
}

void RangeTable::saveData(string filename)
{
	fstream f(filename, ios_base::out | ios_base::trunc);
	for (rkCheat_RangeList::iterator it = data.begin(); it != data.end(); ++it)
	{
		f << it->first << " " << it->second << endl;
	}
	f.close();
}

bool RangeTable::loadData(string filename)
{
	rkCheat_RangeList rList;
	rkCheat_RangeItem rItem;
	fstream f(filename, ios_base::in);
	if (!f)
		return false;
	while (true) 
	{
		unsigned long val1, val2;
		f >> val1 >> val2;
		if( f.eof() ) break;
		rItem.first = val1;
		rItem.second = val2;
		rList.push_back(rItem);
	}
	f.close();
	rows(0);
	data = rList;
	rows(data.size());
	return true;
}

void RangeTable::clear()
{
	rows(0);
	data.clear();
}

void RangeTable::DrawHeader(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, row_header_color());
    fl_color(FL_BLACK);
    fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
fl_pop_clip();
}

rkCheat_RangeList RangeTable::getSelectedRanges()
{
	rkCheat_RangeList rList;
	for (int i = 0; i<rows(); i++)
	{
		if (row_selected(i))
			rList.push_back(rkCheat_RangeItem(data[i]));
	}
	return rList;
}
