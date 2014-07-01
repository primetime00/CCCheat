include ../../Makefile.common
#INCLUDES = -I. -I../../CCAPI -I../../../../fltk/fltk
#LIBS = -L../../FLTK -L../../CCAPI/CCAPI/lib -lccapi -lfltk -lGL -lGLU -lXrandr -lXinerama -lXext -lX11 -lXft -lfontconfig -ldl
#LFLAGS = -Wall $(DEBUG) $(LIBS)

objects = $(src:.cpp=.o)
src := $(wildcard *.cpp *.cxx)
#CCCheat : $(OBJS)
#	 $(CC) $(INCLUDES) $(OBJS) -o $(TARGET) $(LFLAGS)
#	\rm *.o

CCCheat : $(objects) ui_main.o
	 $(CC) $(INCLUDES) $(CFLAGS) $(objects) -o $(TARGET) $(LFLAGS)

%.o: %.cpp
	$(CC) $(INCLUDES) $(CFLAGS)  -c $< -o $@

ui_main.o: ui_main.cxx
	$(CC) $(INCLUDES) $(CFLAGS)  -c $< -o $@

clean:
	\rm -f *.o $(TARGET)

