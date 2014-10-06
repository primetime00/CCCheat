#include "TrainerTable.h"
#include "CCAPI/Common.h"
#include "CCAPI/Helpers.h"
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <FL/fl_draw.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Text_Display.H>
#include "ui_trainer.h"

using namespace std;
const char value[] = "Here\nis is So what are you Going To DO?? Especially if this wraps too much or the text goes into overflow!";


#define TRAIN_ROW_HEIGHT		50
#define TRAIN_ROW_DESC_WIDTH	420
#define TRAIN_ROW_BUTTON_WIDTH	200
#define TRAIN_BUTTON_WIDTH 100
#define TRAIN_BUTTON_HEIGHT TRAIN_ROW_HEIGHT-10


TrainerTable::TrainerTable(int X, int Y, int W, int H, const char *l) : Fl_Table_Row(X,Y,W,H,l) {
	const int codes = 5;

	uchar r,g,b;
	Fl::get_color(FL_GRAY, r, g, b);
	Fl::set_color(FL_GRAY0, r-20,g-20,b-20);

	rows(codes);
	row_header(0);
	row_height_all(TRAIN_ROW_HEIGHT);
	row_resize(0);

	cols(2);
	col_header(0);
	col_width_all(110);
	col_width(0, TRAIN_ROW_DESC_WIDTH);
	col_width(1, TRAIN_ROW_BUTTON_WIDTH);
	col_resize(1);
	unsigned int width=0;
	for (int i=0; i<cols(); ++i)
		width += col_width(i);
	resize(X,Y,width+25, H);
	end();


	Fl::add_timeout(1.0, redraw, this);
}

void TrainerTable::redraw(void *v)
{
	TrainerTable *t = (TrainerTable *)v;
	t->damage(FL_DAMAGE_ALL);
	Fl::repeat_timeout(1.0, TrainerTable::redraw, v);
}

void TrainerTable::draw_cell(TableContext context, int ROW, int COL, int X, int Y, int W, int H){
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
				if (c == 1)
				{
					if (index % 2 == 1)
					{
						auto box = (Fl_Group*)child(index);
						box->resize(X,Y,W,H);
						auto button = box->child(0);
						button->resize(box->x() + (box->w() - TRAIN_BUTTON_WIDTH)/2, box->y() - (box->h() - TRAIN_BUTTON_HEIGHT)/2, TRAIN_BUTTON_WIDTH, TRAIN_BUTTON_HEIGHT);
					}
				}
				else
				{
					if (index % 2 == 0)
						child(index)->resize(X,Y,W,H);
				}
				index++;
			}
	    }
	    init_sizes();			// tell group children resized
	    return;
	}

      case CONTEXT_COL_HEADER:                  // Draw column headers
		switch (COL)
		{
		case 0:		DrawHeader("Description",X,Y,W,H); break;
		case 1:	DrawHeader("Enable",X,Y,W,H); break;
		default: break;
		}
        return; 
      case CONTEXT_ROW_HEADER:                  // Draw row headers
		fl_font(FL_HELVETICA, 14);              // set the font for our drawing operations
        sprintf(s,"%03d:",COL);                 // "001:", "002:", etc
        DrawHeader(s,X,Y,W,H);
        return; 
      case CONTEXT_CELL:    // Draw data in cells
		return;
      default:
        return;
    }
}

int TrainerTable::handle(int evt)
{
	int res = Fl_Table_Row::handle(evt);
	return res;
#if 0
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
#endif
}
void TrainerTable::DrawData(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    // Draw cell bg
    //fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
    // Draw cell data
    fl_color(FL_GRAY0); fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
    // Draw box border
    fl_color(color()); fl_rect(X,Y,W,H);
fl_pop_clip();
}

void TrainerTable::DrawHeader(const char *s, int X, int Y, int W, int H) {
fl_push_clip(X,Y,W,H);
    fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, row_header_color());
    fl_color(FL_BLACK);
    fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
fl_pop_clip();
}

void TrainerTable::DrawCheckbox(int X, int Y, int W, int H)
{
	fl_push_clip(X,Y,W,H);

	fl_pop_clip();
}

TrainerTable::TrainObj::TrainObj(rkTrainerCode &code)
{
	if (code == nullptr)
		return;
	tBuffer = new Fl_Text_Buffer();
	tBuffer->append(code->name.c_str());
	tDisplay = new Fl_Text_Display(0,0,100,100);
	tDisplay->buffer(tBuffer);
	tDisplay->textfont(FL_HELVETICA);
	tDisplay->textsize(14);
	tDisplay->color(FL_GRAY);
	tDisplay->box(Fl_Boxtype::FL_NO_BOX);
	tDisplay->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
	cBox = new Fl_Group(0,0,TRAIN_BUTTON_WIDTH, TRAIN_ROW_HEIGHT);
	cBox->box(FL_FLAT_BOX);
	cBox->begin();
	cButton = new Fl_Light_Button(0,0,TRAIN_BUTTON_WIDTH,TRAIN_BUTTON_HEIGHT,"Enable");
	cButton->callback(TrainerClass::TrainButtonCB);
	cButton->user_data(code.get());
	cBox->end();
	cCode = code;
}

void TrainerTable::TrainObj::place(int row, TrainerTable *t)
{
	int x1,y1,w1,h1;
	t->find_cell(Fl_Table::CONTEXT_TABLE, row, 0, x1,y1,w1,h1);
	tDisplay->position(x1,y1);
	tDisplay->size(w1,h1);
	t->find_cell(Fl_Table::CONTEXT_TABLE, row, 1, x1,y1,w1,h1);

	cBox->position(x1+w1-cBox->w()-30,y1+((h1-cBox->h())/2));
	cBox->color(row % 2 == 0 ? FL_GRAY0 : FL_GRAY);

	//cButton->position(x1+w1-cButton->w()-30,y1+((h1-cButton->h())/2));
	if (row % 2 == 0)
		tDisplay->color(FL_GRAY0);
}


void TrainerTable::setCodes(char *codes)
{
	begin();
#if 0
	for (int i=0; i<1; ++i)
	{
		rkTrainerCode codeData = make_shared<rkTrainerCodeObj>();
		stringstream st;
		st << "Code " << i;
		codeData->name = st.str();
		codeData->condition = true;
		codeData->conditionType = SEARCH_VALUE_GREATER;
		codeData->conditionValue = 5380;
		PointerOffsets pt, pt2;
		pt.push_back(6);
		pt.push_back(4);
		pt2.push_back(3);
		pt2.push_back(5);
		codeData->m_conditionAddress = make_shared<AddressObj>(0x20+i, 1000, SEARCH_VALUE_TYPE_4BYTE, 0);
		codeData->m_conditionAddress->value = 5000;
		auto ai = make_shared<AddressObj>(0x3000+i, pt, SEARCH_VALUE_TYPE_4BYTE, 0);
		ai->store = 7000;
		auto ai2 = make_shared<AddressObj>(0x2000+i, pt2, SEARCH_VALUE_TYPE_4BYTE, 0);
		ai2->store = 1000;
		codeData->m_addresses.push_back(ai);
		codeData->m_addresses.push_back(ai2);
		TrainItem t = make_shared<TrainObj>(codeData);
		t->place(i, this);
		m_list.push_back(t);
	}
	rows(m_list.size());
	end();
#endif
#if 1
#if 0
		rkTrainerCode codeData = make_shared<rkTrainerCodeObj>();
		stringstream st;
		codes[0] = 1;
		st << "Code 1";
		codeData->name = st.str();
		codeData->condition = true;
		codeData->conditionType = SEARCH_VALUE_GREATER;
		codeData->conditionValue = 5380;
		PointerOffsets pt, pt2;
		pt.push_back(6);
		pt.push_back(4);
		pt2.push_back(3);
		pt2.push_back(5);
		codeData->m_conditionAddress = make_shared<AddressObj>(0x20, 1000, SEARCH_VALUE_TYPE_4BYTE, 0);
		codeData->m_conditionAddress->value = 5000;
		auto ai = make_shared<AddressObj>(0x3000, pt, SEARCH_VALUE_TYPE_4BYTE, 0);
		ai->store = 7000;
		auto ai2 = make_shared<AddressObj>(0x2000, pt2, SEARCH_VALUE_TYPE_4BYTE, 0);
		ai2->store = 1000;
		codeData->m_addresses.push_back(ai);
		codeData->m_addresses.push_back(ai2);
		codeData->write(&codes[1]);
#endif
	begin();
#if 0
	ifstream outDebugFile("C:/Users/keg45397/Documents/Visual Studio 2012/Projects/CCCheat/UI/UI/dbg.dat", ofstream::binary);
	outDebugFile.read((char*)codes, 999);
	outDebugFile.close();
#endif
	char numberOfCodes = codes[0];
	int pos = 1;
	for (int i=0; i<numberOfCodes; ++i)
	{
		rkTrainerCode codeData = make_shared<rkTrainerCodeObj>();
		pos += codeData->read(&codes[pos]);
		TrainItem t = make_shared<TrainObj>(codeData);
		t->place(i, this);
		m_list.push_back(t);
	}
	rows(m_list.size());
	end();
#endif
}