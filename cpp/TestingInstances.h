#ifndef TESTING_INST
#define TESTING_INST

#include <mutex>
#include <map>
#include <list>
#include "BatchWraperSpottings.h"
#include "BatchWraperTranscription.h"
#include "spotting.h"
#include "batches.h"
#include "SpecialInstances.h"
#include "Knowledge.h"

#define UNDEF_N -1

class TestingInstances: public SpecialInstances
{
    public:
        TestingInstances(const Knowledge::Corpus* corpus, int contextPad);
        ~TestingInstances()
        {
            for (auto p: spottingsT)
            {
                for (Spotting* s : p.second)
                    delete s;
            }
            for (auto p: spottingsF)
            {
                for (Spotting* s : p.second)
                    delete s;
            }
            for (TranscribeBatch* t : trans)
                delete t;
            for (TranscribeBatch* t : manTrans)
                delete t;
        }
        BatchWraper* getBatch(int width, int color, string prevNgram, int testingNum);
        void addSpotting(string ngram, bool label, int pageId, int tlx, int tly, int brx, int bry);
        void addTrans(string label, vector<string> poss, vector<Spotting> spots, int wordIdx, bool manual);
        void allLoaded();

        
    private:
        //Knowledge::Word* dummyWord;
        vector<char> testNumType;

        vector<string> ngramList;

        map<int, vector<bool> > ngramsUsed;
        map<int, mutex> ngramsMut;


        map<string,vector<Spotting*> > spottingsT;
        map<string,vector<bool> > spottingsTUsed;
        map<string,mutex> spottingsTMut;
        map<string,vector<Spotting*> > spottingsF;
        map<string,vector<bool> > spottingsFUsed;
        map<string,mutex> spottingsFMut;

        vector<TranscribeBatch*> trans;
        vector<bool> transUsed;
        mutex transMut;
        vector<TranscribeBatch*> manTrans;
        vector<bool> manTransUsed;
        mutex manTransMut;

        const Knowledge::Corpus* corpus;
        int contextPad;

        int getNextIndex(vector<bool>& setUsed, mutex& mutLock);
        BatchWraper* getSpottingsBatch(string ngram, int width, int color, string prevNgram, int testingNum);
        BatchWraper* getManTransBatch(int width);
        BatchWraper* getTransBatch(int width);
        BatchWraper* makeInstance(int testingNum, int width,int color, string prevNgram);
};
#endif
