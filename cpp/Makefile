CXX = g++-4.9 
LINK = g++-4.9
CXX_FLAGS =  -g  -std=c++11 #-ansi -fexceptions -fPIC -fno-omit-frame-pointer -pthread

LIBS += -lpthread

LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -fopenmp


PROGRAM_NAME = test

OBJECTS = obj/test.o obj/MasterQueue.o obj/SpottingResults.o obj/TestQueue.o

bin: $(PROGRAM_NAME)
all: $(PROGRAM_NAME)


clean:
	- rm  $(PROGRAM_NAME)
	- rm  obj/*.o


$(PROGRAM_NAME):  $(OBJECTS)  
	$(LINK) $(CXX_FLAGS) -o $(PROGRAM_NAME) $(OBJECTS) $(LIBS)

	
obj/test.o: test.cpp MasterQueue.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/test.o test.cpp $(LIBS)

obj/MasterQueue.o: MasterQueue.h MasterQueue.cpp SpottingResults.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/MasterQueue.o MasterQueue.cpp $(LIBS)

obj/SpottingResults.o: SpottingResults.h SpottingResults.cpp 
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/SpottingResults.o SpottingResults.cpp $(LIBS)
	
obj/TestQueue.o: TestQueue.h TestQueue.cpp SpottingResults.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/TestQueue.o TestQueue.cpp $(LIBS)
