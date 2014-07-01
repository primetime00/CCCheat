include ../../Makefile.common
objects = $(src:.cpp=.o)
src := $(wildcard *.cpp)

libccapi.a : $(objects)
	\mkdir -p ./lib
	ar rcs ./lib/libccapi.a $(objects)
	\rm *.o

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	\rm -f *.o
	\rm -rf lib

