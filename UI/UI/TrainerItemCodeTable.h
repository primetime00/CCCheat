#ifndef _TRAINCODETABLE_
#define _TRAINCODETABLE_

#include <FL/Fl.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Menu_Item.H>
#include <CCAPI/Types.h>
#include "Common.h"
#include "IDeactivate.h"
#include <string>
#include <functional>
#include <memory>

using namespace std;
#define MAX_CODE_MENU 1024

class TrainerItemWindow;
struct ItemField
{
	AddressItem data;
	Fl_Choice *choice;
	ValueInput *value_input;
	Fl_Menu_Item *menu;
	char _delete;
	ItemField(Fl_Choice *c, ValueInput *v, Fl_Menu_Item *m) { choice = c; value_input = v; menu = m; _delete = 0;}
	ItemField() { value_input = 0; choice = 0; menu = 0; _delete = 0;}
	~ItemField() {
		choice->menu(0);
		delete []menu;
		delete value_input; 
		delete choice;
		value_input = 0;
		choice = 0;
	}

};

typedef shared_ptr<ItemField> CodeItem;
typedef vector<CodeItem> CodeItems;

class TrainerItemCodeTable : public Fl_Table_Row, public IDeactivate {
public:
  TrainerItemCodeTable(int X, int Y, int W, int H, const char *l=0);
  ~TrainerItemCodeTable() { if (m_menu) { delete [] m_menu; } }

  void init();
  void createEntry();
  void deleteEntry(unsigned int row);
  void deleteEntry(vector<int> &indexRows);

  void addCode() { createEntry(); }
  void addCodes(AddressList codes);
  void deleteCode();

  bool hasSelection();
  void clearSelection() { select_all_rows(0); set_selection(-1,-1,-1,-1); }

  CodeItems getCodes() { return trainList; }

  void setCodeList(const rkCheat_CodeList &list);

  void clearTable();

  static void tableClickedCB(Fl_Widget *w, void *data);
  
  static void codeDescriptionChangedCB(Fl_Widget *w, void *item) { 
	  ((TrainerItemCodeTable *)w->parent()->parent())->onDescriptionChanged((ItemField*)((Fl_Choice*)w)->user_data(), (AddressObj*)((Fl_Choice*)w)->mvalue()->user_data(), ((Fl_Choice*)w)->value()); 
  }  
  //static void codeValueChangedCB(Fl_Widget *w, void *item) { ((TrainerItemCodeTable *)w->parent())->onValueChanged((CodeItem*)item, ((ValueInput*)w)->getLLValue()); }  
  static void codeValueChangedCB(Fl_Widget *w, void *item) { 
	  ((TrainerItemCodeTable *)w->parent()->parent())->onValueChanged((ItemField*)((Fl_Input*)w)->user_data(), 
		  (AddressObj*)((ItemField*)((Fl_Input*)w)->user_data())->choice->mvalue()->user_data(),
		  ((ValueInput*)w)->getLLValue()); 
  }  
  bool isEmpty();
  bool canAdd();
  void setParent(TrainerItemWindow *w) { parentWindow = w; }

  static void selectionTimeout(void *);
  
protected:
  void draw_cell(TableContext context, int ROW=0, int COL=0, int X=0, int Y=0, int W=0, int H=0);
  int handle(int evt);

private:
  void DrawHeader(const char *s, int X, int Y, int W, int H);
  void DrawData(const char *s, int X, int Y, int W, int H);
  void DrawCheckbox(int X, int Y, int W, int H);

  void rebuildTable();
  void reInsert();

  void onDescriptionChanged(ItemField *item, AddressObj *addr, int sel);
  void onValueChanged(ItemField *item, AddressObj *addr, long long value);
  void onCellDoubleClicked(int row, int col);
  void onCellRightClicked(int row, int col);
  void onTableClicked();
  Fl_Menu_Item *copyMenu(Fl_Menu_Item *m);
  void selectRow(int row);
  int getSelectedRow() { return selectedRow; }


  void doDeactivate();
  Fl_Widget *getWidget() { return this; }

  vector<int> getSelectedRows();
  rkCheat_CodeList codeList;
  CodeItems trainList;
  Fl_Menu_Item *m_menu;
  TrainerItemWindow *parentWindow;

  int selectedRow;

private:
		Fl_Widget *last_cell_widget;
		Fl_Widget *current_cell_widget;

};

#endif