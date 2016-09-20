#include "TestingInstances.h"


TestingInstances::TestingInstances(const Knowledge::Corpus* corpus) : corpus(corpus)
{
    //n = new ngram spotting
    //s = same ngram spotting (as previous)
    //t = transcription
    //m = manual transcription
     testNumType={  'n',
                    's',
                    's',
                    'n',
                    's',
                    's',
                    's',
                    't',
                    't',
                    't',
                    'n',
                    's',
                    's',
                    'n',
                    't',
                    't',
                    't',
                    't',
                    'm',
                    'm',
                    'm',
                    'm',
                    'm',
                    'm',
     };
    dummyWord = new Knowledge::Word(0,0,0,0,NULL,NULL,NULL,NULL,0);
}


BatchWraper* TestingInstances::getBatch(int width, int color, string prevNgram, int testingNum)
{
#ifndef TEST_MODE
    try
    {
#endif
        BatchWraper* ret= makeInstance(testingNum,width,color,prevNgram);
        if (ret!=NULL)
            return ret;
        else
            return new BatchWraperBlank();
#ifndef TEST_MODE
    }
    catch (exception& e)
    {
        cout <<"Exception in TestingInstances::getBatch(), "<<e.what()<<endl;
    }
    catch (...)
    {
        cout <<"Exception in TestingInstances::getBatch(), UNKNOWN"<<endl;
    }
#endif
    return new BatchWraperBlank();
}

void TestingInstances::addSpotting(string ngram, bool label, int pageId, int tlx, int tly, int brx, int bry)
{
    spottings[ngram].push_back(new Spotting(tlx,tly,brx,bry,pageId,corpus->imgForPageId(pageId),ngram,0,label?1:0));
    //if (ngramList.find(ngram) == ngramList.end())
    //{
    //    ngramList.push_back(ngram);

}
void TestingInstances::addTrans(string label, vector<string> poss, multimap<string,Location> spots, int wordIdx, bool manual)
{
    Knowledge::Word* w = corpus->getWord(wordIdx);
    bool toss;
    int tlx, tly, brx, bry;
    int pageId=w->getPageId();
    w->getBoundsAndDone(&tlx,&tly,&brx,&bry,&toss);
    multimap<int,Spotting> spottings;
    for (auto s : spots)
    {
        spottings.insert( make_pair(s.second.x1,Spotting(s.second.x1,s.second.y1,s.second.x2,s.second.y2,pageId,corpus->imgForPageId(pageId),s.first,0)) );
    }
    if (manual)
    {
        manTrans.push_back(new TranscribeBatch(dummyWord,poss,corpus->imgForPageId(pageId),&spottings,tlx,tly,brx,bry));
    }
    else
    {
        multimap<float,string> scored;
        for (int i=0; i< poss.size(); i++)
            scored.insert( make_pair(i,poss[i]) );
        trans.push_back(new TranscribeBatch(dummyWord,scored,corpus->imgForPageId(pageId),&spottings,tlx,tly,brx,bry));
    }

}

void TestingInstances::allLoaded()
{
    for (auto p : spottings)
    {
        ngramList.push_back(p.first);
        spottingsUsed[p.first].resize(p.second.size());
        spottingsMut[p.first];
    }
    for (int n=0; n<testNumType.size(); n++)
    {
        char t = testNumType[n];
        if (t=='n')
        {
            ngramsUsed[n].resize(ngramList.size());
            ngramsMut[n];
        }
    }
    transUsed.resize(trans.size());
    manTransUsed.resize(manTrans.size());
}


int TestingInstances::getNextIndex(vector<bool>& setUsed, mutex& mutLock)
{
    int setIdx = rand()%setUsed.size();
    int start=setIdx;
    mutLock.lock();
    while (setUsed.at(setIdx))
    {
        setIdx = (setIdx+1)%setUsed.size();
        if (setIdx==start)
        {
            setUsed.assign(setUsed.size(),false);
            break;
        }
    }
    setUsed.at(setIdx)=true;
    mutLock.unlock();
    return setIdx;
}


BatchWraper* TestingInstances::getSpottingsBatch(string ngram, int width, int color, string prevNgram)
{
    SpottingsBatch* batch = new SpottingsBatch(ngram,0);
    for (int i=0; i<5; i++)
    {
        int idx = getNextIndex(spottingsUsed[ngram], spottingsMut[ngram]);
        Spotting* spotting = spottings[ngram][ idx ];
        batch->push_back(SpottingImage(*spotting,width,color,prevNgram));            
    }
    BatchWraper* ret = new BatchWraperSpottings(batch);
    return ret;
}

BatchWraper* TestingInstances::getTransBatch(int width)
{
    TranscribeBatch* batch = trans[ getNextIndex(transUsed, transMut) ];
    batch->setWidth(width);
    return (BatchWraper*) (new BatchWraperTranscription(batch));
}
BatchWraper* TestingInstances::getManTransBatch(int width)
{
    TranscribeBatch* batch = manTrans[ getNextIndex(manTransUsed, manTransMut) ];
    batch->setWidth(width);
    return (BatchWraper*) (new BatchWraperTranscription(batch));
}
    

BatchWraper* TestingInstances::makeInstance(int testingNum, int width,int color, string prevNgram)
{
    if (testNumType[testingNum] == 'n') {
        string ngram = ngramList[ getNextIndex(ngramsUsed[testingNum],ngramsMut[testingNum]) ];
        return getSpottingsBatch(ngram,width,color,prevNgram);
    } else if (testNumType[testingNum] == 's') {
        return getSpottingsBatch(prevNgram,width,color,prevNgram);
    } else if (testNumType[testingNum] == 't') {
        return getTransBatch(width);
    } else if (testNumType[testingNum] == 'm') {
        return getManTransBatch(width);
    }
    return NULL;
}
