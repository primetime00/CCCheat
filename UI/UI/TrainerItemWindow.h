#ifndef __TRAINERITEMWINDOW__
#define __TRAINERITEMWINDOW__
#include "rkWindow.h"
#include "CodeTable.h"
#include "Trainer/Trainer.h"
#include "TrainerItemCodeTable.h"

class TrainerItemWindow : public rkWindow
{
public:
	TrainerItemWindow(int X, int Y, int W, int H, const char *l=0) : rkWindow(X,Y,W,H, l) {  }
	TrainerItemWindow(int W, int H, const char *l=0) : rkWindow(W,H, l) {  }

	void init();
	void show();

	void popup(CodeTable *t, rkTrainerCodeObj *code = 0);

	void onConditionChanged();
	void onConditionCodeChanged(AddressObj *addr = 0);
	void onNameChanged();
	void onAddCode();
	void onDeleteCode();
	void onCancel();
	void onOK();

	rkTrainerCode getCode() { return m_code; }

	static void OnCodeTableUpdate(TrainerItemCodeTable *ct, bool timeout=false);
	static void UpdateTimeout(void *);

	static string getDescriptionOrCode(const rkCheat_CodeItem &item);

	void capture();

private:
	void checkButtonStatus();
	void setupCodes(CodeTable *t, rkTrainerCodeObj *code);
	rkCheat_CodeList m_cl;
	rkTrainerCode m_code;
};

#endif