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
                    't',
                    't',
                    't',
                    'n',
                    's',
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
     };
    //testNumType={'t','t','t','t','t','t'};
    //dummyWord = new Knowledge::Word(0,0,0,0,NULL,NULL,NULL,NULL,0);
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
    if (label)
    {
        for (const Spotting* s : spottingsT[ngram])
        {
            if (pageId == s->pageId && 
                abs(tlx-s->tlx)<5 && abs(tly-s->tly)<5 &&
                abs(brx-s->brx)<5 && abs(bry-s->bry)<5)
                return;
        }
        spottingsT[ngram].push_back(new Spotting(tlx,tly,brx,bry,pageId,corpus->imgForPageId(pageId),ngram,0,label?1:0));
    }
    else
    {
        for (const Spotting* s : spottingsF[ngram])
        {
            if (pageId == s->pageId &&
                abs(tlx-s->tlx)<5 && abs(tly-s->tly)<5 &&
                abs(brx-s->brx)<5 && abs(bry-s->bry)<5)
                return;
        }
        spottingsF[ngram].push_back(new Spotting(tlx,tly,brx,bry,pageId,corpus->imgForPageId(pageId),ngram,0,label?1:0));
    }
    //if (ngramList.find(ngram) == ngramList.end())
    //{
    //    ngramList.push_back(ngram);

}
void TestingInstances::addTrans(string label, vector<string> poss, vector<Spotting> spots, int wordIdx, bool manual)
{
    if (wordIdx>=corpus->size())
        return;
    Knowledge::Word* w = corpus->getWord(wordIdx);
    bool toss;
    int tlx, tly, brx, bry;
    int pageId=w->getPageId();
    w->getBoundsAndDone(&tlx,&tly,&brx,&bry,&toss);
    multimap<int,Spotting> spottings;
    for (auto s : spots)
    {
        //spottings.insert( make_pair(s.second.x1,Spotting(s.second.x1,s.second.y1,s.second.x2,s.second.y2,pageId,corpus->imgForPageId(pageId),s.first,0)) );
        s.pageId=pageId;
        s.pagePnt=corpus->imgForPageId(pageId);
        spottings.insert( make_pair(s.tlx,s) );
    }
    if (manual)
    {
        for (TranscribeBatch* b : manTrans)
        {
            if (w == b->getBackPointer())
                return;
        }
        manTrans.push_back(new TranscribeBatch(w,poss,corpus->imgForPageId(pageId),&spottings,tlx,tly,brx,bry,label));
    }
    else
    {
        for (TranscribeBatch* b : manTrans)
        {
            if (w == b->getBackPointer() && 
                spottings.size()==b->getSpottingPoints().size() && 
                label.compare(b->getGT())==0)
                return;
        }
        multimap<float,string> scored;
        for (int i=0; i< poss.size(); i++)
            scored.insert( make_pair(i,poss[i]) );
        trans.push_back(new TranscribeBatch(w,scored,corpus->imgForPageId(pageId),&spottings,tlx,tly,brx,bry,label));
    }

}

void TestingInstances::allLoaded()
{
    for (auto p : spottingsT)
    {
        if (spottingsT.size()>=spottingsF.size())
            ngramList.push_back(p.first);
        spottingsTUsed[p.first].resize(p.second.size());
        spottingsTMut[p.first];
    }
    for (auto p : spottingsF)
    {
        if (spottingsT.size()<spottingsF.size())
            ngramList.push_back(p.first);
        ngramList.push_back(p.first);
        spottingsFUsed[p.first].resize(p.second.size());
        spottingsFMut[p.first];
    }
    
    //Remove ngrams for which we don't have enough instances
    for (auto iter=ngramList.begin(); iter!=ngramList.end(); )
    {
        string ngram=*iter;
        if (spottingsT[ngram].size() + spottingsF[ngram].size() < 20)
        {
            iter=ngramList.erase(iter);
            spottingsT.erase(ngram);
            spottingsF.erase(ngram);
        }
        else
            iter++;
    }

    int singleC=0;
    int maxSingle = manTrans.size()*0.2;
    auto iter = manTrans.begin();
    while (iter != manTrans.end())
    {
        if ((*iter)->getGT().length()==1)
        {
            if (singleC<maxSingle)
            {
                singleC++;
                iter++;
            }
            else
            {
                iter = manTrans.erase(iter);
            }
        }
        else
            iter++;
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
    ngramsUsed[UNDEF_N].resize(ngramList.size());
    ngramsMut[UNDEF_N];

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


BatchWraper* TestingInstances::getSpottingsBatch(string ngram, int width, int color, string prevNgram, int testingNum)
{
    SpottingsBatch* batch = new SpottingsBatch(ngram,testingNum);
    //This gives a batches with a uniform distribution as to their T/F ratio,
    //assuming we have enough True examples of the ngram
    int numTrue = rand()%min(6, 1+(int)(spottingsT[ngram].size()/2));
    list<Spotting*> batchInsts;
    for (int i=0; i<numTrue; i++)
    {
        int idx = getNextIndex(spottingsTUsed[ngram], spottingsTMut[ngram]);
        //cout<<"True spot: "<<idx<<" of "<<spottingsT[ngram].size()<<endl;
        Spotting* spotting = spottingsT[ngram][ idx ];
        batchInsts.push_back(spotting);
    }
    for (int i=numTrue; i<5; i++)
    {
        int idx = getNextIndex(spottingsFUsed[ngram], spottingsFMut[ngram]);
        //cout<<"False spot: "<<idx<<" of "<<spottingsF[ngram].size()<<endl;
        Spotting* spotting = spottingsF[ngram][ idx ];
        batchInsts.push_back(spotting);
    }
    for (int i=0; i<5; i++)
    {
        bool front = rand()%2;
        Spotting* spotting;
        if (front)
        {
            spotting = batchInsts.front();
            batchInsts.pop_front();
        }
        else
        {
            spotting = batchInsts.back();
            batchInsts.pop_back();
        }
        batch->push_back(SpottingImage(*spotting,width,color,prevNgram));        
    }
    BatchWraper* ret = new BatchWraperSpottings(batch);
    return ret;
}

BatchWraper* TestingInstances::getTransBatch(int width)
{
    TranscribeBatch* batch = trans[ getNextIndex(transUsed, transMut) ];
    //cout<<"trans batch: "<<batch->getId()<<" of "<<trans.size()<<endl;
    batch->setWidth(width);
    return (BatchWraper*) (new BatchWraperTranscription(batch));
}
BatchWraper* TestingInstances::getManTransBatch(int width)
{
    TranscribeBatch* batch = manTrans[ getNextIndex(manTransUsed, manTransMut) ];
    //cout<<"man batch: "<<batch->getId()<<" of "<<manTrans.size()<<endl;
    batch->setWidth(width);
    return (BatchWraper*) (new BatchWraperTranscription(batch));
}
    

BatchWraper* TestingInstances::makeInstance(int testingNum, int width,int color, string prevNgram)
{
    if (testingNum>=testNumType.size())
        return NULL;

    if (testNumType[testingNum] == 'n') {
        string ngram = ngramList[ getNextIndex(ngramsUsed[testingNum],ngramsMut[testingNum]) ];
        return getSpottingsBatch(ngram,width,color,prevNgram,testingNum);
    } else if (testNumType[testingNum] == 's') {
        if (spottingsTUsed.find(prevNgram) == spottingsTUsed.end())
            prevNgram = ngramList[ getNextIndex(ngramsUsed[UNDEF_N],ngramsMut[UNDEF_N]) ];
        return getSpottingsBatch(prevNgram,width,color,prevNgram,testingNum);
    } else if (testNumType[testingNum] == 't') {
        return getTransBatch(width);
    } else if (testNumType[testingNum] == 'm') {
        return getManTransBatch(width);
    }
    return NULL;
}
