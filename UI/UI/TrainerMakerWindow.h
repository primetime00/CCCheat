#ifndef __TRAINERWINDOW__
#define __TRAINERWINDOW__
#include "FL/Fl_Text_Buffer.H"
#include "rkWindow.h"
#include "Common.h"
#include "Trainer/Trainer.h"

class TrainerMakerWindow : public rkWindow
{
public:
	TrainerMakerWindow(int X, int Y, int W, int H, const char *l=0) : rkWindow(X,Y,W,H, l) { infoBuffer = new Fl_Text_Buffer(); }
	TrainerMakerWindow(int W, int H, const char *l=0) : rkWindow(W,H, l) { infoBuffer = new Fl_Text_Buffer(); }

	void init();
	void show();

	void createNewCode();
	void onSelectCode();
	void onEdit();
	void checkButtonStatus();
	void onCreate();

	void popup();
	void capture();

private:
	vector<rkTrainerCode> m_codes;
	Fl_Text_Buffer *infoBuffer;
};

#endif