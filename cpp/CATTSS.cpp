#include "CATTSS.h"

CATTSS::CATTSS(string lexiconFile, string pageImageDir, string segmentationFile)
{
    
    masterQueue = new MasterQueue();
    Lexicon::instance()->readIn(lexiconFile);
    corpus = new Knowledge::Corpus();
    corpus->addWordSegmentaionAndGT(pageImageDir, segmentationFile);
    spotter = new FacadeSpotter(masterQueue,corpus,"model");
    
#ifdef TEST_MODE
    int pageId=0;

    //Spotting er1 (1160,15,1190,36,pageId,corpus->imgForPageId(pageId),"er",0.0);//[1]
    //vector<Spotting > init = {er1};
    //masterQueue->updateSpottingResults(new vector<Spotting>(init));
    Spotting* er1 = new Spotting(811,18,842,40,pageId,corpus->imgForPageId(pageId),"er",0);//[1]
    vector<Spotting* > init = {er1};
    spotter->addQueries(init);
#endif
    //test
    /*
        Spotting s1(1000, 1458, 1154, 1497, 2720272, corpus->imgForPageId(2720272), "ma", 0.01);
        Spotting s2(1196, 1429, 1288, 1491, 2720272, corpus->imgForPageId(2720272), "ch", 0.01);
        Spotting s3(1114, 1465, 1182, 1496, 2720272, corpus->imgForPageId(2720272), "ar", 0.01);
        Spotting s4(345, 956, 415, 986, 2720272, corpus->imgForPageId(2720272), "or", 0.01);
        Spotting s5(472, 957, 530, 987, 2720272, corpus->imgForPageId(2720272), "er", 0.01);
        Spotting s6(535, 943, 634, 986, 2720272, corpus->imgForPageId(2720272), "ed", 0.01);
        Spotting s7(355, 1046, 455, 1071, 2720272, corpus->imgForPageId(2720272), "un", 0.01);
        Spotting s8(492, 1030, 553, 1069, 2720272, corpus->imgForPageId(2720272), "it", 0.01);
        Spotting s9(439, 1024, 507, 1096, 2720272, corpus->imgForPageId(2720272), "fi", 0.01);
        vector<Spotting> toAdd={s1,s2,s3,s4,s5,s6,s7,s8,s9};
        vector<TranscribeBatch*> newBatches = corpus->updateSpottings(&toAdd,NULL,NULL);
        assert(newBatches.size()>0);
        masterQueue->enqueueTranscriptionBatches(newBatches);
        if (toAdd.size()>9)
        {
            cout <<"harvested (init):"<<endl;
            for (int i=9; i<toAdd.size(); i++)
                imshow("har: "+toAdd[i].ngram,toAdd[i].img());
            waitKey();
        }

        vector<unsigned long> toRemoveSpottings={s7.id};
        vector<unsigned long> toRemoveBatches;        
        toAdd.clear();
        Spotting s10(356, 780, 423, 816, 2720272, corpus->imgForPageId(2720272), "on", 0.01);
        Spotting s11(429, 765, 494, 864, 2720272, corpus->imgForPageId(2720272), "ly", 0.01);
        toAdd.push_back(s10); toAdd.push_back(s11);
        newBatches = corpus->updateSpottings(&toAdd,&toRemoveSpottings,&toRemoveBatches);
        if (toAdd.size()>2)
        {
            cout <<"harvested (init):"<<endl;
            for (int i=9; i<toAdd.size(); i++)
                imshow("har: "+toAdd[i].ngram,toAdd[i].img());
            waitKey();
        }
        //vector<TranscribeBatch*> modBatches = corpus->removeSpottings(toRemoveSpottings,toRemoveBatches);
        masterQueue->enqueueTranscriptionBatches(newBatches,&toRemoveBatches);
        cout <<"Enqueued "<<newBatches.size()<<" new trans batches"<<endl;            
        if (toRemoveBatches.size()>0)
            cout <<"Removed "<<toRemoveBatches.size()<<" trans batches"<<endl;            
    /**/
    //test

}
BatchWraper* CATTSS::getBatch(int num, int width, int color, string prevNgram)
{
    bool hard=true;
    if (num==-1) {
        num=5;
        hard=false;

    }
    BatchWraper* ret= masterQueue->getBatch(num,hard,width,color,prevNgram);
#ifdef TEST_MODE_C
    return ret;
#endif
    if (ret!=NULL)
        return ret;
    else
        return new BatchWraperBlank();
}


void CATTSS::updateSpottings(string resultsId, vector<string> ids, vector<int> labels, int resent)
{
    //cout <<"Recieved batch for "<<resultsId<<endl;
    vector<pair<unsigned long,string> > toRemoveSpottings;        
    vector<unsigned long> toRemoveBatches;        
    vector<Spotting>* toAdd = masterQueue->feedback(stoul(resultsId),ids,labels,resent,&toRemoveSpottings);
    if (toAdd!=NULL)
    {
        vector<Spotting*> newExemplars;
        vector<pair<unsigned long,string> > toRemoveExemplars;        
        vector<TranscribeBatch*> newBatches = corpus->updateSpottings(toAdd,&toRemoveSpottings,&toRemoveBatches,&newExemplars,&toRemoveExemplars);
        masterQueue->enqueueTranscriptionBatches(newBatches,&toRemoveBatches);
        masterQueue->enqueueNewExemplars(newExemplars);
        //cout <<"Enqueued "<<newBatches.size()<<" new trans batches"<<endl;            
        //if (toRemoveBatches.size()>0)
        //    cout <<"Removed "<<toRemoveBatches.size()<<" trans batches"<<endl;     
        toRemoveExemplars.insert(toRemoveExemplars.end(),toRemoveSpottings.begin(),toRemoveSpottings.end());
        spotter->removeQueries(&toRemoveExemplars);
        spotter->addQueries(*toAdd);
        delete toAdd;
    }
    /*else //We presume that this set has been finished
    {
        for (int i=0; i<ids.size(); i++)
        {
            if (labels[i]==0)
    */
}

void CATTSS::updateTranscription(string id, string transcription)
{
    vector< pair<unsigned long, string> > toRemoveExemplars;
    masterQueue->transcriptionFeedback(stoul(id),transcription,&toRemoveExemplars);
    spotter->removeQueries(&toRemoveExemplars);
}


void CATTSS::updateNewExemplars(string resultsId,  vector<int> labels, int resent)
{
    vector<pair<unsigned long,string> > toRemoveExemplars;        
    vector<SpottingExemplar*> toSpot = masterQueue->newExemplarsFeedback(stoul(resultsId), labels, &toRemoveExemplars);

    spotter->removeQueries(&toRemoveExemplars);
    spotter->addQueries(toSpot);
}

void CATTSS::misc(string task)
{
    if (task.compare("showCorpus")==0)
    {
        corpus->show();
    }
    else if (task.compare("stopSpotting")==0)
    {
        spotter->stop();
    }
    else if (task.length()>14 && task.substr(0,14).compare("startSpotting:")==0)
    {
        int num = stoi(task.substr(14));
        if (num>0)
            spotter->run(num);
        else
            cout<<"ERROR: tried to start spotting with "<<num<<" threads"<<endl;
    }

}    

