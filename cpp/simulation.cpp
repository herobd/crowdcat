#include <atomic>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Simulator.h"
#include "CATTSS.h"

using namespace std;
using namespace cv;

void controlLoop(CATTSS* cattss, atomic_bool* cont)
{
    while(1)
    {
        cout<<"CONTROL:"<<endl<<": quit"<<endl<<": show"<<endl<<": manual"<<endl<<": save"<<endl;;
        string line;
        getline(cin, line);
        if (line.compare("quit")==0)
        {
            cattss->misc("stopSpotting");
            cont->store(false);
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
        else if (line.compare("save")==0)
        {
            cattss->misc("save");
        }
    }
}

void threadLoop(CATTSS* cattss, Simulator* sim, atomic_bool* cont)
{
    string prevNgram="";
    int slept=0;
    thread::id thread_id = this_thread::get_id();
    stringstream ss;
    ss<<thread_id;
    string thread = ss.str();
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

#ifdef DEBUG_AUTO
            vector<Mat> images = batch->getImages();
            for (int i=0; i<labels.size(); i++)
            {
                if (gt[i].length()>2)
                {
                    string name = thread;
                    if (labels[i])
                        name+="_TRUE";
                    else 
                        name+="_FALSE";
                    imshow(name,images.at(i));
                    do
                    {
                        cout<<name<<"> ["<<ngram<<"] Loc page:"<<locs[i].pageId<<", bb: "<<locs[i].x1<<" "<<locs[i].y1<<" "<<locs[i].x2<<" "<<locs[i].y2<<endl;
                    }
                    while(waitKey() != 10);//enter
                }
            }
#endif

            cattss->updateSpottings(id,ids,labels,0);
            prevNgram = ngram;
        }
        else if (batch->getType()==NEW_EXEMPLARS)
        {
            string id;
            vector<string> ngrams;
            vector<Location> locs;
            batch->getNewExemplars(&id,&ngrams,&locs);
            vector<int> labels = sim->newExemplars(ngrams,locs,prevNgram);
#ifdef DEBUG_AUTO
            vector<Mat> images = batch->getImages();
            for (int i=0; i<labels.size(); i++)
            {
                string name = thread;
                if (labels[i])
                    name+="_TRUE";
                else 
                    name+="_FALSE";
                imshow(name,images[i]);
                do
                {
                    cout<<name<<"> ["<<ngrams[i]<<"] Loc page:"<<locs[i].pageId<<", bb: "<<locs[i].x1<<" "<<locs[i].y1<<" "<<locs[i].x2<<" "<<locs[i].y2<<endl;
                }
                while(waitKey() != 10);//enter
            }
#endif
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
#ifdef DEBUG_AUTO
#endif
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




int main(int argc, char** argv)
{
    int numSimThreads=1;

    string dataname="BENTHAM";
    string lexiconFile = "/home/brian/intel_index/data/wordsEnWithNames.txt";
    string pageImageDir = "/home/brian/intel_index/data/bentham/BenthamDatasetR0-Images/Images/Pages";
    string segmentationFile = "/home/brian/intel_index/data/bentham/ben_cattss_c_corpus.gtp";
    string charSegFile = "/home/brian/intel_index/data/bentham/manual_segmentations.csv";
    string spottingModelPrefix = "model/CATTSS_BENTHAM";
    string savePrefix = "save/sim_BENTHAM";
    if (argc>1)
        savePrefix=argv[1];
    if (argc>2)
        GlobalK::knowledge()->setSimSave(argv[2]);
    else
        GlobalK::knowledge()->setSimSave("save/simulationTracking.csv");

    int numSpottingThreads = 5;
    int numTaskThreads = 3;
    int height = 1000;
    int width = 2500;
    int milli = 7000;
    CATTSS* cattss = new CATTSS(lexiconFile,
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
//#ifndef DEBUG_AUTO
//    Simulator sim(dataname,charSegFile);
//#else
    Simulator sim("test",charSegFile);
//#endif
    atomic_bool cont(true);
    vector<thread*> taskThreads(numSimThreads);
    for (int i=0; i<numSimThreads; i++)
    {
        taskThreads[i] = new thread(threadLoop,cattss,&sim,&cont);
        taskThreads[i]->detach();
        
    }
    controlLoop(cattss,&cont);

    cout<<"---DONE---"<<endl;
    //delete cattss;
    for (int i=0; i<numSimThreads; i++)
        delete taskThreads[i];
    this_thread::sleep_for(chrono::seconds(40));
    delete cattss;
}
