include ../../Makefile.common
OBJS = CodeTable.o IPInput.o InterfaceCCAPI.o NumberInput.o RangeTable.o ResultTable.o SearchOperationChoice.o ValueInput.o ValueViewerTable.o ValueViewerWindow.o main.o rkWindow.o ui_main.o 
#INCLUDES = -I. -I../../CCAPI -I../../../../fltk/fltk
#LIBS = -L../../FLTK -L../../CCAPI/CCAPI/lib -lccapi -lfltk -lGL -lGLU -lXrandr -lXinerama -lXext -lX11 -lXft -lfontconfig -ldl
#LFLAGS = -Wall $(DEBUG) $(LIBS)

CCCheat : $(OBJS)
	 $(CC) $(INCLUDES) $(OBJS) -o $(TARGET) $(LFLAGS)
#	\rm *.o

CodeTable.o :
	$(CC) $(CFLAGS)  CodeTable.cpp 
IPInput.o  :
	$(CC) $(CFLAGS)  IPInput.cpp
InterfaceCCAPI.o :
	$(CC) $(CFLAGS)  InterfaceCCAPI.cpp
NumberInput.o :
	$(CC) $(CFLAGS)  NumberInput.cpp
RangeTable.o :
	$(CC) $(CFLAGS)  RangeTable.cpp
ResultTable.o :
	$(CC) $(CFLAGS)  ResultTable.cpp
SearchOperationChoice.o :
	$(CC) $(CFLAGS)  SearchOperationChoice.cpp
ValueInput.o :
	$(CC) $(CFLAGS)  ValueInput.cpp
ValueViewerWindow.o :
	$(CC) $(CFLAGS)  ValueViewerWindow.cpp
ValueViewerTable.o :
	$(CC) $(CFLAGS)  ValueViewerTable.cpp
main.o :
	$(CC) $(CFLAGS)  main.cpp
rkWindow.o :
	$(CC) $(CFLAGS)  rkWindow.cpp
ui_main.o :
	$(CC) $(CFLAGS)  ui_main.cxx

clean:
	\rm -f *.o $(TARGET)

