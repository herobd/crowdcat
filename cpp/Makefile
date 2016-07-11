CXX = g++-4.9 
LINK = g++-4.9
CXX_FLAGS =  -g  -std=c++11 -DTEST_MODE#-ansi -fexceptions -fPIC -fno-omit-frame-pointer -pthread

LIBS += -lpthread

LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -fopenmp


PROGRAM_NAME = test

OBJECTS = obj/TranscribeBatchQueue.o obj/NewExemplarsBatchQueue.o obj/Global.o obj/test.o obj/MasterQueue.o obj/SpottingResults.o obj/TestQueue.o obj/Knowledge.o obj/Lexicon.o obj/maxflow.o obj/graph.o

bin: $(PROGRAM_NAME)
all: $(PROGRAM_NAME)


clean:
	- rm  $(PROGRAM_NAME)
	- rm  obj/*.o


$(PROGRAM_NAME):  $(OBJECTS)  
	$(LINK) $(CXX_FLAGS) -o $(PROGRAM_NAME) $(OBJECTS) $(LIBS)

	
obj/test.o: test.cpp MasterQueue.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/test.o test.cpp $(LIBS)

obj/TranscribeBatchQueue.o: TranscribeBatchQueue.h TranscribeBatchQueue.cpp Knowledge.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/TranscribeBatchQueue.o TranscribeBatchQueue.cpp $(LIBS)

obj/NewExemplarsBatchQueue.o: NewExemplarsBatchQueue.h NewExemplarsBatchQueue.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/NewExemplarsBatchQueue.o NewExemplarsBatchQueue.cpp $(LIBS)

obj/Global.o: Global.h Global.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/Global.o Global.cpp $(LIBS)

obj/MasterQueue.o: MasterQueue.h MasterQueue.cpp SpottingResults.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/MasterQueue.o MasterQueue.cpp $(LIBS)

obj/SpottingResults.o: SpottingResults.h SpottingResults.cpp 
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/SpottingResults.o SpottingResults.cpp $(LIBS)
	
obj/TestQueue.o: TestQueue.h TestQueue.cpp SpottingResults.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/TestQueue.o TestQueue.cpp $(LIBS)
	
obj/Knowledge.o: Knowledge.h Knowledge.cpp Lexicon.h Global.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/Knowledge.o Knowledge.cpp $(LIBS)
	
obj/Lexicon.o: Lexicon.h Lexicon.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/Lexicon.o Lexicon.cpp $(LIBS)

obj/maxflow.o: maxflow/maxflow.cpp maxflow/graph.h maxflow/block.h  maxflow/instances.inc
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/maxflow.o maxflow/maxflow.cpp

obj/graph.o: maxflow/graph.cpp maxflow/graph.h \
                maxflow/block.h \
                maxflow/instances.inc
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/graph.o maxflow/graph.cpp
