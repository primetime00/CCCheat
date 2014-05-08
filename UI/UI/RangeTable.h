#ifndef _RANGETABLE_
#define _RANGETABLE_

#include <FL/Fl.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Table.H>
#include "Common.h"
#include <string>

using namespace std;

#define RANGE_ROWS 50
#define RANGE_COLS 2
class RangeTable : public Fl_Table_Row {
public:
  RangeTable(int X, int Y, int W, int H, const char *l=0);

  void addEntry(unsigned long start, unsigned long end);
  void deleteEntry(unsigned int row);

  void saveData(string filename);
  bool loadData(string filename);
  void clear();
  bool hasSelection();
  void clearSelection() { select_all_rows(0); set_selection(-1,-1,-1,-1); }

  rkCheat_RangeList getSelectedRanges();

protected:
  void draw_cell(TableContext context, int ROW=0, int COL=0, int X=0, int Y=0, int W=0, int H=0);

private:
  void DrawHeader(const char *s, int X, int Y, int W, int H);
  void DrawData(const char *s, int X, int Y, int W, int H);


private:
		rkCheat_RangeList data;
};

#endif