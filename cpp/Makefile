CXX = g++-4.9 
LINK = g++-4.9
CXX_FLAGS =  -g  -std=c++11 -DTEST_MODE -DTEST_MODE_C#-ansi -fexceptions -fPIC -fno-omit-frame-pointer -pthread

LIBS += -lpthread

LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -fopenmp
LIBS += -L/home/brian/intel_index/EmbAttSpotter -lembattspotter -larmadillo
LIBS += -L/home/brian/intel_index/EmbAttSpotter/vlfeat-0.9.20/bin/glnxa64/ -l:libvl.so
INCLUDE +=-I/home/brian/intel_index/EmbAttSpotter
PROGRAM_NAME = test

OBJECTS = obj/TranscribeBatchQueue.o \
          obj/NewExemplarsBatchQueue.o \
          obj/Global.o \
          obj/MasterQueue.o \
          obj/SpottingResults.o \
          obj/Knowledge.o \
          obj/Lexicon.o \
          obj/maxflow.o \
          obj/graph.o \
          obj/SpottingQueue.o \
          obj/batches.o \
          obj/AlmazanSpotter.o \
          obj/AlmazanDataset.o
          #obj/FacadeSpotter.o \
          #obj/test.o \
          #obj/TestQueue.o 

bin: $(PROGRAM_NAME)
all: $(PROGRAM_NAME)
tr: train
data: makeData


clean:
	- rm  $(PROGRAM_NAME)
	- rm  train
	- rm  makeData
	- rm  obj/*.o


$(PROGRAM_NAME):  $(OBJECTS)  
	$(LINK) $(CXX_FLAGS) -o $(PROGRAM_NAME) $(OBJECTS) $(LIBS)

train: $(OBJECTS)
	$(LINK) $(CXX_FLAGS) -o train train.cpp $(INCLUDE) $(OBJECTS) $(LIBS)
	
makeData: $(OBJECTS)
	$(LINK) $(CXX_FLAGS) -o makeData makeData.cpp $(INCLUDE) $(OBJECTS) $(LIBS)
	
obj/test.o: test.cpp MasterQueue.h TestQueue.h MasterQueue.h Knowledge.h Spotter.h FacadeSpotter.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/test.o test.cpp $(LIBS)

obj/TranscribeBatchQueue.o: TranscribeBatchQueue.h TranscribeBatchQueue.cpp Knowledge.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/TranscribeBatchQueue.o TranscribeBatchQueue.cpp $(LIBS)

obj/NewExemplarsBatchQueue.o: NewExemplarsBatchQueue.h NewExemplarsBatchQueue.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/NewExemplarsBatchQueue.o NewExemplarsBatchQueue.cpp $(LIBS)

obj/AlmazanSpotter.o: AlmazanSpotter.h AlmazanSpotter.cpp AlmazanDataset.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/AlmazanSpotter.o AlmazanSpotter.cpp $(LIBS)

obj/AlmazanDataset.o: AlmazanDataset.h AlmazanDataset.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/AlmazanDataset.o AlmazanDataset.cpp $(LIBS)

obj/SpottingQueue.o: SpottingQueue.h SpottingQueue.cpp Knowledge.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/SpottingQueue.o SpottingQueue.cpp $(LIBS)

obj/batches.o: batches.h batches.cpp WordBackPointer.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/batches.o batches.cpp $(LIBS)

#obj/SpottingQuery.o: SpottingQuery.h SpottingQuery.cpp
#	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/SpottingQuery.o SpottingQuery.cpp $(LIBS)

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

obj/Spotter.o: Spotter.h Spotter.cpp MasterQueue.h Knowledge.h 
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/Spotter.o Spotter.cpp $(LIBS)

obj/FacadeSpotter.o: Spotter.h FacadeSpotter.cpp MasterQueue.h Knowledge.h FacadeSpotter.h
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/FacadeSpotter.o FacadeSpotter.cpp $(LIBS)


obj/maxflow.o: maxflow/maxflow.cpp maxflow/graph.h maxflow/block.h  maxflow/instances.inc
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/maxflow.o maxflow/maxflow.cpp

obj/graph.o: maxflow/graph.cpp maxflow/graph.h \
                maxflow/block.h \
                maxflow/instances.inc
	$(CXX) -c $(CXX_FLAGS) $(INCLUDE) -o obj/graph.o maxflow/graph.cpp
