#ifndef _TRAINERTABLE_
#define _TRAINERTABLE_

#include <FL/Fl.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Text_Display.H>
#include <CCAPI/MemoryOperator.h>
#include "UI/Common.h"
#include <string>
#include <functional>
#include <memory>
#include "Trainer.h"

using namespace std;



class TrainerTable : public Fl_Table_Row {
public:
  TrainerTable(int X, int Y, int W, int H, const char *l=0);
  ~TrainerTable() { Fl::remove_timeout(redraw); }

  void setCodes (char *codes);

  struct TrainObj
  {
	  Fl_Text_Display *tDisplay;
	  Fl_Text_Buffer *tBuffer;
	  Fl_Light_Button *cButton;
	  Fl_Group *cBox;
	  rkTrainerCode cCode;
	  friend class TrainerTable;
	  TrainObj(rkTrainerCode &code);
	  ~TrainObj() { 
		  if (tDisplay) delete tDisplay; if (tBuffer) delete tBuffer; if (cButton) delete cButton; if (cBox) delete cBox;
		  tDisplay = 0; tBuffer = 0; cButton = 0; cCode = nullptr; cBox = 0;
	  }
	  void place(int row, TrainerTable *t);
  };
  typedef shared_ptr<TrainObj> TrainItem;
  typedef list<TrainItem> TrainList;

//  static void codeFreezeChangedCB(Fl_Widget *w, void *item) { 
//	  ((TrainerTable *)w->parent()->parent())->onCodeFreezeChanged((rkCheat_Code*)item, ((Fl_Check_Button*)w)->value()); 
//  }

  static void redraw(void*);
  
protected:
  void draw_cell(TableContext context, int ROW=0, int COL=0, int X=0, int Y=0, int W=0, int H=0);
  int handle(int evt);

private:
  void DrawHeader(const char *s, int X, int Y, int W, int H);
  void DrawData(const char *s, int X, int Y, int W, int H);
  void DrawCheckbox(int X, int Y, int W, int H);

private:
	TrainList m_list;
};

#endif