#ifndef _RESULTTABLE_
#define _RESULTTABLE_

#include <FL/Fl.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Table.H>
#include <CCAPI/Common.h>
#include <CCAPI/MemoryOperator.h>
#include "Common.h"
#include <string>

#define RESULT_TABLE_ADD		0x00
#define RESULT_TABLE_DELETE		0x01
#define RESULT_TABLE_REFRESH	0x02


using namespace std;


class ResultTable : public Fl_Table_Row {
public:
  ResultTable(int X, int Y, int W, int H, const char *l=0);

  void setResults(rkCheat_Results &r);
  void addEntry(unsigned long address, unsigned long value, char sign);
  void deleteEntry(unsigned int row);

  void saveData(string filename);
  bool loadData(string filename);
  void clear();
  bool hasSelection();
  void clearSelection() { select_all_rows(0); set_selection(-1,-1,-1,-1); }
  void setResultType(char type) { resultType = type; }
  void setMemoryOperator(MemoryOperator *op) { m_operator = op; }
  vector<AddressItem> getSelectedResults();
  vector<int> getSelectedRows();
  void deleteResults();
  void refreshResults();

  //rkCheat_ResultList getSelectedRanges();
  static void redraw(void*);

protected:
  void draw_cell(TableContext context, int ROW=0, int COL=0, int X=0, int Y=0, int W=0, int H=0);

private:
  void DrawHeader(const char *s, int X, int Y, int W, int H);
  void DrawData(const char *s, int X, int Y, int W, int H);


private:
		//rkCheat_ResultList data;
		//AddressList data;
		MemoryOperator *m_operator;
		rkCheat_Results *resList;
		Results data;
		char resultType;
};

#endif