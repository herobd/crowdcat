#include "CATTSS.h"

CATTSS::CATTSS(string lexiconFile, string pageImageDir, string segmentationFile)
{
    
    masterQueue = new MasterQueue();
    Lexicon::instance()->readIn(lexiconFile);
    corpus = new Knowledge::Corpus();
    corpus->addWordSegmentaionAndGT(pageImageDir, segmentationFile);
    spotter = new Spotter(masterQueue,corpus,"model");


}
BatchWraper* CATTSS::getBatch(int num, int width, int color, string prevNgram)
{
    bool hard=true;
    if (num==-1) {
        num=5;
        hard=false;

    }
    return masterQueue->getBatch(num,hard,width,color,prevNgram);
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
        vector<TranscribeBatch*> newBatches = corpus->updateSpottings(toAdd,&toRemoveSpottings,&toRemoveBatches,&newExemplars);
        masterQueue->enqueueTranscriptionBatches(newBatches,&toRemoveBatches);
        masterQueue->enqueueNewExemplars(&newExemplars);
        //cout <<"Enqueued "<<newBatches.size()<<" new trans batches"<<endl;            
        //if (toRemoveBatches.size()>0)
        //    cout <<"Removed "<<toRemoveBatches.size()<<" trans batches"<<endl;     
        spotter->removeQueries(&toRemoveSpottings);
        spotter->addQueries(toAdd);
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
