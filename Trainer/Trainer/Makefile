include ../../Makefile.common

TRAIN_TARGET="Trainer"
objects = $(src:.cpp=.o)
src := $(wildcard *.cpp *.cxx)
INCLUDES += -I$(UI_DIR)
objects += $(UI_DIR)/UI/IPInput.o $(UI_DIR)/UI/NumberInput.o

Trainer : $(objects) ui_trainer.o
	 $(CC) $(DEBUG) $(INCLUDES) $(CFLAGS) $(objects) -o $(TRAIN_TARGET) $(LFLAGS)
	 \mkdir -p dump
	 \xxd -i Trainer > dump/TrainerSrc.cpp
	 \sed -i 's/Trainer/trainSrc/g' dump/TrainerSrc.cpp

%.o: %.cpp
	$(CC) $(DEBUG) $(INCLUDES) $(CFLAGS)  -c $< -o $@

ui_trainer.o: ui_trainer.cxx
	$(CC) $(DEBUG) $(INCLUDES) $(CFLAGS)  -c $< -o $@

clean:
	\rm -f *.o $(TRAIN_TARGET)
	\rm -rf dump

