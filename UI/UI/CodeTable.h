#ifndef _CODETABLE_
#define _CODETABLE_

#include <FL/Fl.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <CCAPI/MemoryOperator.h>
#include "Common.h"
#include "IDeactivate.h"
#include <string>
#include <functional>
#include <memory>

#define CODE_TABLE_NEW		0x00
#define CODE_TABLE_DELETE	0x01
#define CODE_TABLE_SAVE		0x02
#define CODE_TABLE_LOAD		0x03

using namespace std;


class CodeTable : public Fl_Table_Row, public IDeactivate {
public:
  CodeTable(int X, int Y, int W, int H, const char *l=0);
  ~CodeTable() { Fl::remove_timeout(redraw); }

  void addEntry(string desc, unsigned long address, long long value, char type, bool freeze=false);
  void addPointer(string desc, PointerItem p, bool freeze=false);
  void deleteEntry(unsigned int row);
  void deleteEntry(vector<int> &indexRows);

  void saveData(string filename);
  bool loadData(string filename);
  void clearTable();
  bool hasSelection();
  void clearSelection() { select_all_rows(0); set_selection(-1,-1,-1,-1); }
  void newCode();
  void deleteCodes();
  void addResults(vector<AddressItem> &items, char type);
  void setMemoryOperator(MemoryOperator *op);
  void setViewer(Fl_Callback *cb) { viewer = cb; }

  static void tableClickedCB(Fl_Widget *w, void *data) {
	  if (((Fl_Table*)w)->callback_context() == CONTEXT_CELL)
	  {
		  if (Fl::event_clicks())
			((CodeTable*)w)->onCellDoubleClicked(((Fl_Table*)w)->callback_row(), ((Fl_Table*)w)->callback_col());
		  else if (Fl::event_button() == FL_RIGHT_MOUSE)
			  ((CodeTable*)w)->onCellRightClicked(((Fl_Table*)w)->callback_row(), ((Fl_Table*)w)->callback_col());
	  }
	  else
	  {
		((CodeTable*)w)->onTableClicked();
	  }
  }
  static void codeDescriptionChangedCB(Fl_Widget *w, void *item) { ((CodeTable *)w->parent()->parent())->onDescriptionChanged((rkCheat_Code*)item, ((Fl_Input*)w)->value()); }
  static void codeAddressChangedCB(Fl_Widget *w, void *item) { ((CodeTable *)w->parent()->parent())->onAddressChanged((rkCheat_Code*)item, ((ValueInput*)w)->getULValue()); }
  static void codeValueChangedCB(Fl_Widget *w, void *item) { ((CodeTable *)w->parent()->parent())->onValueChanged((rkCheat_Code*)item, ((ValueInput*)w)->getLLValue()); }
  static void codeTypeChangedCB(Fl_Widget *w, void *item) { ((CodeTable *)w->parent()->parent())->onCodeTypeChanged((rkCheat_Code*)item, ((Fl_Choice*)w)->value()); }
  static void codeFreezeChangedCB(Fl_Widget *w, void *item) { 
	  ((CodeTable *)w->parent()->parent())->onCodeFreezeChanged((rkCheat_Code*)item, ((Fl_Check_Button*)w)->value()); 
  }

  static void redraw(void*);
  
protected:
  void draw_cell(TableContext context, int ROW=0, int COL=0, int X=0, int Y=0, int W=0, int H=0);
  void table_scrolled() { cout << "YAY" << endl;}
  int handle(int evt);

private:
  void DrawHeader(const char *s, int X, int Y, int W, int H);
  void DrawData(const char *s, int X, int Y, int W, int H);
  void DrawCheckbox(int X, int Y, int W, int H);

  void rebuildTable();
  void reInsert();

  void onDescriptionChanged(rkCheat_Code *data, string desc);
  void onAddressChanged(rkCheat_Code *data, unsigned long value);
  void onValueChanged(rkCheat_Code *data, long long value);
  void onCodeTypeChanged(rkCheat_Code *data, int value);
  void onCodeFreezeChanged(rkCheat_Code *data, int value);
  void onCellDoubleClicked(int row, int col);
  void onCellRightClicked(int row, int col);
  void onTableClicked();

  void doDeactivate();
  Fl_Widget *getWidget() { return this; }

  vector<int> getSelectedRows();
  MemoryOperator *m_operator;
  Fl_Callback *viewer;

  

private:
		rkCheat_CodeList data;
		Fl_Widget *last_cell_widget;
		Fl_Widget *current_cell_widget;

};

#endif