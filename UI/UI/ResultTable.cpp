#include "ResultTable.h"
#include "CCAPI/Common.h"
#include <iostream>
#include <fstream>
#include <FL/fl_draw.H>

using namespace std;

#define RESULT_COLS 3
#define MAX_RESULTS 100

ResultTable::ResultTable(int X, int Y, int W, int H, const char *l) : Fl_Table_Row(X,Y,W,H,l) {

	//rows(RANGE_ROWS);
	m_operator = 0;
	resList = NULL;
	row_header(0);
	row_height_all(20);
	row_resize(0);

	cols(RESULT_COLS);
	col_header(1);
	col_width_all(110);
	col_width(1, 200);
	col_resize(1);

	type(SELECT_MULTI);
	resultType = SEARCH_VALUE_TYPE_4BYTE;
	end();
	Fl::add_timeout(1.0, redraw, this);
}

void ResultTable::addEntry(unsigned long address, unsigned long value, char sign)
{
#if 0
	bool found = false;
	for (int i=0; i<data.size(); i++)
	{
		if (get<0>(data.at(i)) == address) //we have this
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		data.push_back(make_tuple(address,value, sign));
		rows(data.size());
	}
#endif
}
void ResultTable::deleteEntry(unsigned int row)
{
	if (data.size() == 1) //we are getting rid of the last entry
	{
		clear();
	}
	else
	{
		data.erase(data.begin()+row, data.begin()+row+1);
		rows(data.size());
	}
}

bool ResultTable::hasSelection()
{
	int t,b,l,r;
	get_selection(t,l,b,r);
	return (t > -1 && b > -1);
}

void ResultTable::setResults(rkCheat_Results &r)
{
	int rounds = 0;
	resList = &r;
	data.clear();
	for (rkCheat_Results::iterator mit = resList->begin(); mit != resList->end(); ++mit)
	{
		unsigned long id = mit->first;
		vector<ResultList> *res = mit->second;
		for (unsigned int thread=0; thread<res->size(); ++thread)
		{
			for (ResultList::iterator it = res->at(thread).begin(); it != res->at(thread).end(); ++it) //each result list (bunch of addresslists)
			{
				for (AddressList::iterator ait = it->second.begin(); ait != it->second.end(); ++ait) //each address list
				{
					data.push_back(ResultRow(&(*ait), it->first, thread, id));
					rounds++;
					if (rounds > MAX_RESULTS)
					{
						rows(data.size());
						return;
					}
				}
			}
		}
	}
	rows(data.size());
}

void ResultTable::draw_cell(TableContext context, int ROW, int COL, int X, int Y, int W, int H){
    static char s[40];
	fl_font(FL_HELVETICA, 14);              // set the font for our drawing operations
    switch ( context ) {
      case CONTEXT_STARTPAGE:                   // before page is drawn..
        return; 
      case CONTEXT_COL_HEADER:                  // Draw column headers
		switch (COL)
		{
		case 0:	DrawHeader("Address",X,Y,W,H); break;
		case 1:	DrawHeader("Dec",X,Y,W,H); break;
		case 2:	DrawHeader("Hex",X,Y,W,H); break;
		default: break;
		}
        return; 
      case CONTEXT_ROW_HEADER:                  // Draw row headers
        sprintf(s,"%03d:",COL);                 // "001:", "002:", etc
        DrawHeader(s,X,Y,W,H);
        return; 
      case CONTEXT_CELL:    // Draw data in cells
	    fl_push_clip(X, Y, W, H);
	    {
	        // BG COLOR
		int rs = row_selected(ROW);
		fl_color( row_selected(ROW) ? selection_color() : FL_WHITE);
		fl_rectf(X, Y, W, H);

		// TEXT
		fl_color(FL_BLACK);
		unsigned long v = (data[ROW].item->value);
		sprintf(s, (data[ROW].item->sign) == TEST_SIGN_YES ? "%hd" : "%hu",  (unsigned char)(data[ROW].item->value));
		if ((unsigned int)ROW < data.size())
		{
			switch(COL)
			{
			case 0: sprintf(s,"0x%08X",(data[ROW].item->address)); break;
			case 1: 
				switch (resultType)
				{
				case SEARCH_VALUE_TYPE_FLOAT: sprintf(s,"%.4f", *((float*)&(data[ROW].item->value))); break;
				case SEARCH_VALUE_TYPE_1BYTE: 
					if ((data[ROW].item->sign) == TEST_SIGN_YES)
						sprintf(s, "%hd", (char)(data[ROW].item->value)); 
					else
						sprintf(s, "%hu", (unsigned char)(data[ROW].item->value)); 
					break;
				case SEARCH_VALUE_TYPE_2BYTE: 
					if ((data[ROW].item->sign) == TEST_SIGN_YES)
						sprintf(s, "%hd", (short)(data[ROW].item->value)); 
					else
						sprintf(s, "%hu", (unsigned short)(data[ROW].item->value)); 
					break;
				case SEARCH_VALUE_TYPE_4BYTE: 
					if ((data[ROW].item->sign) == TEST_SIGN_YES)
						sprintf(s, "%ld", (long)(data[ROW].item->value)); 
					else
						sprintf(s, "%lu", (unsigned long)(data[ROW].item->value)); 
					break;
				default: sprintf(s,"%ld", (data[ROW].item->value)); break;
				} break;
			case 2: 
				switch (resultType)
				{
				case SEARCH_VALUE_TYPE_4BYTE: sprintf(s,"%X", *((unsigned long*)&(data[ROW].item->value))); break;
				case SEARCH_VALUE_TYPE_FLOAT: sprintf(s,"%X", *((unsigned long*)&(data[ROW].item->value))); break;
				case SEARCH_VALUE_TYPE_2BYTE: sprintf(s,"%X", *((unsigned short*)&(data[ROW].item->value))); break;
				case SEARCH_VALUE_TYPE_1BYTE: sprintf(s,"%X", *((unsigned char*)&(data[ROW].item->value))); break;
				default: sprintf(s,"%ld", (data[ROW].item->value)); break;
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

void ResultTable::DrawData(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    // Draw cell bg
    //fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
    // Draw cell data
    fl_color(FL_GRAY0); fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
    // Draw box border
    fl_color(color()); fl_rect(X,Y,W,H);
fl_pop_clip();
}

void ResultTable::saveData(string filename)
{
#if 0
	fstream f = fstream(filename, ios_base::out | ios_base::trunc);
	for (AddressList::iterator it = data.begin(); it != data.end(); ++it)
	{
		f << get<0>(*it) << " " << get<1>(*it) << " " << get<2>(*it) << endl;
	}
	f.close();
#endif
}

bool ResultTable::loadData(string filename)
{
#if 0
	AddressList rList;
	AddressItem rItem;
	fstream f = fstream(filename, ios_base::in);
	if (!f)
		return false;
	while (true) 
	{
		unsigned long addr, value;
		char sign;
		f >> addr >> value >> sign;
		if( f.eof() ) break;
		rItem = make_tuple(addr, value, sign);
		rList.push_back(rItem);
	}
	f.close();
	rows(0);
	data = rList;
	rows(data.size());
#endif
	return true;
}

void ResultTable::clear()
{
	rows(0);
	data.clear();
}

void ResultTable::DrawHeader(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, row_header_color());
    fl_color(FL_BLACK);
    fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
fl_pop_clip();
}

vector<AddressItem> ResultTable::getSelectedResults()
{
	vector<AddressItem> res;
	for (int i=0; i<data.size(); ++i)
	{
		if (row_selected(i))
		{
			res.push_back(*data[i].item);
		}
	}
	return res;
}

vector<int> ResultTable::getSelectedRows()
{
	vector<int> res;
	for (int i=0; i<data.size(); ++i)
	{
		if (row_selected(i))
		{
			res.push_back(i);
		}
	}
	return res;
}

void ResultTable::deleteResults()
{
	int index = 0;
	if (resList == 0)
		return;
	for (rkCheat_Results::iterator resultIt = resList->begin(); resultIt != resList->end(); ++resultIt)
	{
		vector<ResultList> *rangeSearchResults = resultIt->second;
		for (vector<ResultList>::iterator rangeSearchResultsIT = rangeSearchResults->begin(); rangeSearchResultsIT != rangeSearchResults->end(); ++rangeSearchResultsIT)
		{
			for (ResultList::iterator sectionResultIT = rangeSearchResultsIT->begin(); sectionResultIT != rangeSearchResultsIT->end(); ++sectionResultIT)
			{
				for (AddressList::iterator addressListIT = sectionResultIT->second.begin(); addressListIT != sectionResultIT->second.end();)
				{
					AddressList *ar = &sectionResultIT->second;
					if (row_selected(index))
					{
						addressListIT = ar->erase(addressListIT, addressListIT+1);
					}
					else
						++addressListIT;
					index++;
					if (index > MAX_RESULTS)
					{
						select_all_rows(0);
						setResults(*resList);
						return;
					}
				}
			}
		}
	}
	select_all_rows(0);
	setResults(*resList);
}

void ResultTable::refreshResults()
{
	if (m_operator == 0)
		return;
	for (Results::iterator it = data.begin(); it != data.end(); ++it)
	{
		m_operator->setReadMemoryOperation(it->item->address, resultType, (char*)&it->item->value, false);
	}
}

void ResultTable::redraw(void *v)
{
	ResultTable *t = (ResultTable *)v;
	t->damage(FL_DAMAGE_ALL);
	Fl::repeat_timeout(1.0, ResultTable::redraw, v);
}


#if 0
rkCheat_RangeList ResultTable::getSelectedRanges()
{
	rkCheat_RangeList rList;
	for (int i = 0; i<rows(); i++)
	{
		if (row_selected(i))
			rList.push_back(rkCheat_RangeItem(data[i]));
	}
	return rList;
}
#endif