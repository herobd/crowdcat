
void* threadTask(void* cattss)
{
    //signal(SIGPIPE, SIG_IGN);
    ((CATTSS*)cattss)->threadLoop();
    //pthread_exit(NULL);
}

void run(int numThreads)
{

}
void stop()
{
    cont.store(0);
    for (int i=0; i<taskThreads.size(); i++)
        sem_post(&semLock);
}


void threadLoop(CATTSS* cattss, Simulator* sim)
{
    string prevNgram="";
    while (cont.load())
    {
        BatchWraper* batch = cattss->getBatch(5,500,0,prevNgram);
        if (batch->getType()==SPOTTINGS)
        {
            string id;
            vector<string> ids;
            string ngram;
            vector<Location> locs;
            vector<string> gt;
            batch->getSpottings(&id,&ngram,&ids,&locs,&gt);
            vector<int> labels = sim->spottings(ngram,locs,gt,prevNgram);
            cattss->updateSpottings(id,ids,labels,0);
            prevNgram = ngram
        }
        else if (batch->getType()==NEW_EXEMPLARS)
        {
            string id;
            vector<string> ngrams;
            vector<Location> locs;
            batch->getNewExemplars(&id,&ngrams,&locs);
            vector<int> labels = sim->newExemplars(ngrams,locs,prevNgram);
            cattss-> updateNewExemplars(id,labels,0);
            prevNgram=ngrams.back();
        }
        else if (batch->getType()==TRANSCRIPTION)
        {
            string batchId;
            int wordIndex;
            vector<SpottingPoint> spottings;
            vector<string> poss;
            bool manual;
            string gt;
            batch->getTranscription(&batchId,&wordIndex,&spottings,&poss,&manual,&gt);
            string trans;
            if (manual)
            {
                trans=sim->manual(wordIndex,poss,gt,prevNgram.compare("#")==0);
                prevNgram="#";
            }
            else
            {
                trans=sim->transcription(wordIndex,spottings,poss,gt,prevNgram.compare("!")==0);
                prevNgram="!"
            }
            cattss->updateTranscription(batchId,trans,manual);
        }
        
    }
}




int main(int argc, char** agrv)
{
    string lexiconFile = string(*lexiconFileNAN);
    string pageImageDir = string(*pageImageDirNAN);
    string segmentationFile = string(*segmentationFileNAN);
    string spottingModelPrefix = string(*spottingModelPrefixNAN);
    string savePrefix = string(*savePrefixNAN);
    int numSpottingThreads = To<int>(info[5]).FromJust();
    int numTaskThreads = To<int>(info[6]).FromJust();
    int height = To<int>(info[7]).FromJust();
    int width = To<int>(info[8]).FromJust();
    int milli = To<int>(info[9]).FromJust();
    cattss = new CATTSS(lexiconFile,
                        pageImageDir,
                        segmentationFile,
                        spottingModelPrefix,
                        savePrefix,
                        numSpottingThreads,
                        numTaskThreads,
                        height,
                        width,
                        milli);

    Simulator sim;
    taskThreads.resize(numSimThreads);
    for (int i=0; i<numTSimhreads; i++)
    {
        taskThreads[i] = new thread(threadLoop,cattss,&sim);
        taskThreads[i]->detach();
        
    }
}
