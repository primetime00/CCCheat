#ifndef _VALUETABLE_
#define _VALUETABLE_

#include <FL/Fl.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <CCAPI/MemoryOperator.h>
#include "Common.h"
#include "IDeactivate.h"
#include "CodeTable.h"
#include <string>
#include <functional>
#include <memory>

using namespace std;

#define DEFAULT_ROWS 200

class ValueViewerTable : public Fl_Table_Row, public IDeactivate {
public:
  ValueViewerTable(int X, int Y, int W, int H, const char *l=0);
  ~ValueViewerTable() { Fl::remove_timeout(redraw); }

  bool hasSelection();
  void clearSelection() { select_all_rows(0); set_selection(-1,-1,-1,-1); }
  void addSelectedAddresses();
  void setMemoryOperator(MemoryOperator *op);
  void setCodeTable(CodeTable *t) { m_codeTable = t; }
  void setType(char type) { m_selectedType = type; }
  void setAddress(unsigned long address) { m_focusAddress = address; }
  void setSigned(bool s) { m_signed = s; }

  static void tableClickedCB(Fl_Widget *w, void *data) {
	  if (((Fl_Table*)w)->callback_context() == CONTEXT_CELL)
	  {
		  if (Fl::event_clicks())
			((ValueViewerTable*)w)->onCellDoubleClicked(((Fl_Table*)w)->callback_row(), ((Fl_Table*)w)->callback_col());
		  else if (Fl::event_button() == FL_RIGHT_MOUSE)
		  {
			  //((ValueViewerTable*)w)->onCellRightClicked(((Fl_Table*)w)->callback_row(), ((Fl_Table*)w)->callback_col());
		  }
	  }
	  else
	  {
		((ValueViewerTable*)w)->onTableClicked();
	  }
  }
  void startMemoryRead();
  void stopMemoryRead();
  static void redraw(void*);
  
protected:
  void draw_cell(TableContext context, int ROW=0, int COL=0, int X=0, int Y=0, int W=0, int H=0);
  int handle(int evt);

private:
  void DrawHeader(const char *s, int X, int Y, int W, int H);
  void DrawData(const char *s, int X, int Y, int W, int H);
  
  void onCellDoubleClicked(int row, int col);
  void onCellRightClicked(int row, int col);
  void onTableClicked();

  void doDeactivate();
  Fl_Widget *getWidget() { return this; }

  vector<int> getSelectedRows();
  MemoryOperator *m_operator;
  Fl_Callback *viewer;
  CodeTable *m_codeTable;

  char m_selectedType;
  bool m_signed;
  char m_memory[DEFAULT_ROWS*2];

  

private:
		unsigned long m_focusAddress, m_startAddress;

};

#endif