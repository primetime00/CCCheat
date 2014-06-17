include ../../Makefile.common
OBJS = CCAPI.o ChunkFuzzySearch.o ChunkValueSearch.o MemoryOperator.o RangeMemory.o RangeTask.o SearchMemory.o SearchTask.o DumpMemory.o DumpTask.o Task.o

libccapi.a : $(OBJS)
	\mkdir -p ./lib
	ar rcs ./lib/libccapi.a $(OBJS)
	\rm *.o

CCAPI.o :
	$(CC) $(CFLAGS) CCAPI.cpp
ChunkFuzzySearch.o :
	$(CC) $(CFLAGS) ChunkFuzzySearch.cpp
ChunkValueSearch.o :
	$(CC) $(CFLAGS) ChunkValueSearch.cpp
MemoryOperator.o :
	$(CC) $(CFLAGS) MemoryOperator.cpp
RangeMemory.o :
	$(CC) $(CFLAGS) RangeMemory.cpp
RangeTask.o :
	$(CC) $(CFLAGS) RangeTask.cpp
SearchMemory.o :
	$(CC) $(CFLAGS) SearchMemory.cpp
SearchTask.o :
	$(CC) $(CFLAGS) SearchTask.cpp
DumpMemory.o :
	$(CC) $(CFLAGS) DumpMemory.cpp
DumpTask.o :
	$(CC) $(CFLAGS) DumpTask.cpp
Task.o :
	$(CC) $(CFLAGS) Task.cpp

clean:
	\rm -f *.o
	\rm -rf lib

