#include <atomic>

#include "Simulator.h"
#include "CATTSS.h"

void controlLoop(CATTSS* cattss, atomic_bool* cont)
{
    while(1)
    {
        cout<<"CONTROL:"<<endl<<": quit"<<endl<<": show"<<endl<<": manual"<<endl;
        string line;
        getline(cin, line);
        if (line.compare("quit")==0)
        {
            cattss->misc("stopSpotting");
            cont.store(false);
            break;
        }
        else if (line.compare("show")==0)
        {
            cattss->misc("showCorpus");
        }
        else if (line.compare("manual")==0)
        {
            cattss->misc("manualFinish");
        }
    }
}

void threadLoop(CATTSS* cattss, Simulator* sim, atomic_bool* cont)
{
    string prevNgram="";
    int slept=0;
    while (cont->load())
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
                prevNgram="!";
            }
            cattss->updateTranscription(batchId,trans,manual);
        }
        else if (batch->getType()==RAN_OUT)
        {
            this_thread::sleep_for(chrono::minutes(15));
            slept+=15;
            prevNgram="_";
        }
        else
        {
            cout<<"Blank batch given to sim"<<endl;
            prevNgram="_";
        }

        delete batch;
        
    }
    cout<<"Thread slept: "<<slept<<endl;
}




int main(int argc, char** agrv)
{
    string dataname="BENTHAM";
    string lexiconFile = "/home/brian/intel_index/data/wordsEnWithNames.txt";
    string pageImageDir = "/home/brian/intel_index/data/bentham/BenthamDatasetR0-Images/Images/Pages";
    string segmentationFile = "/home/brian/intel_index/data/bentham/ben_cattss_c_corpus.gtp";
    string spottingModelPrefix = "model/CATTSS_BENTHAM";
    string savePrefix = "save/sim_BENTHAM";
    int numSpottingThreads = 5;
    int numTaskThreads = 3;
    int height = 1000;
    int width = 2500;
    int milli = 4000;
    cattss = new CATTSS(lexiconFile,
                        pageImageDir,
                        segmentationFile,
                        spottingModelPrefix,
                        savePrefix,
                        numSpottingThreads,
                        numTaskThreads,
                        height,
                        width,
                        milli,
                        0//pad
                        );

    Simulator sim(dataname);
    atomic_bool cont(true);
    taskThreads.resize(numSimThreads);
    for (int i=0; i<numTSimhreads; i++)
    {
        taskThreads[i] = new thread(threadLoop,cattss,&sim,&cont);
        taskThreads[i]->detach();
        
    }
    controlLoop(cattss,&cont);

    cout<<"---DONE---"<<endl;
    //delete cattss;
}
