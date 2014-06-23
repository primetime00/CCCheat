#ifndef _POINTERTABLE_
#define _POINTERTABLE_

#include <FL/Fl.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Input.H>
#include <CCAPI/MemoryOperator.h>
#include <CCAPI/Common.h>
#include "Common.h"
#include "IDeactivate.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>

using namespace std;

#define DEFAULT_PT_ROWS 200

class PointerScannerTable : public Fl_Table_Row, public IDeactivate {
public:
  PointerScannerTable(int X, int Y, int W, int H, const char *l=0);
  ~PointerScannerTable() { }

  bool hasSelection();
  void clearSelection() { select_all_rows(0); set_selection(-1,-1,-1,-1); }
  void addRow(unsigned long address, PointerOffsets offsets);
  void reset() { m_pointerList.clear(); }
  void setResultAddress(unsigned long a) { m_resultAddress = a; }
#if 0
  void addSelectedAddresses();
  void setMemoryOperator(MemoryOperator *op);
  void setCodeTable(CodeTable *t) { m_codeTable = t; }
  void setType(char type) { m_selectedType = type; }
  void setAddress(unsigned long address) { m_focusAddress = address; }
  unsigned long getAddress() { return m_focusAddress; }
#endif

  static void tableClickedCB(Fl_Widget *w, void *data) {
	  if (((Fl_Table*)w)->callback_context() == CONTEXT_CELL)
	  {
		  if (Fl::event_clicks())
			((PointerScannerTable*)w)->onCellDoubleClicked(((Fl_Table*)w)->callback_row(), ((Fl_Table*)w)->callback_col());
		  else if (Fl::event_button() == FL_RIGHT_MOUSE)
		  {
			  //((PointerScannerTable*)w)->onCellRightClicked(((Fl_Table*)w)->callback_row(), ((Fl_Table*)w)->callback_col());
		  }
	  }
	  else
	  {
		((PointerScannerTable*)w)->onTableClicked();
	  }
  }
#if 0
  void startMemoryRead();
  void stopMemoryRead();
  static void redraw(void*);
#endif

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

private:
	vector<PointerItem> m_pointerList;
		unsigned long m_focusAddress, m_startAddress;
		unsigned long m_resultAddress;

};

#endif