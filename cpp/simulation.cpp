#include <atomic>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Simulator.h"
#include "CrowdCAT.h"

using namespace std;
using namespace cv;

void controlLoop(CrowdCAT* crowdcat, atomic_bool* cont)
{
    while(1)
    {
        cout<<"CONTROL:"<<endl<<": quit"<<endl<<": show"<<endl<<": manual"<<endl<<": save"<<endl;;
        string line;
        getline(cin, line);
        if (line.compare("quit")==0)
        {
            crowdcat->misc("stopSpotting");
            cont->store(false);
            break;
        }
        else if (line.compare("show")==0)
        {
            crowdcat->misc("showCorpus");
        }
        else if (line.compare("manual")==0)
        {
            crowdcat->misc("manualFinish");
        }
        else if (line.compare("save")==0)
        {
            crowdcat->misc("save");
        }
    }
}

void threadLoop(CrowdCAT* crowdcat, Simulator* sim, atomic_bool* cont)
{
    string prevNgram="";
    int slept=0;
    thread::id thread_id = this_thread::get_id();
    stringstream ss;
    ss<<thread_id;
    string thread = ss.str();
    while (cont->load())
    {
        BatchWraper* batch = crowdcat->getBatch(5,500,0,prevNgram);
        /*if (batch->getType()==SPOTTINGS)
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

            crowdcat->updateSpottings(id,ids,labels,0);
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
            crowdcat-> updateNewExemplars(id,labels,0);
            prevNgram=ngrams.back();
        }
        else*/ if (batch->getType()==TRANSCRIPTION)
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
            crowdcat->updateTranscription(batchId,trans,manual);
        }
        else if (batch->getType()==RAN_OUT)
        {
            //this_thread::sleep_for(chrono::minutes(15));
            //slept+=15;
            prevNgram="_";
            cout<<"ran out, so manual finish."<<endl;
            crowdcat->misc("manualFinish");
        }
        else
        {
            cout<<"Blank batch given to sim"<<endl;
            this_thread::sleep_for(chrono::minutes(5));
            slept+=5;
            if (prevNgram.compare("-")==0)
            {
                crowdcat->misc("stopSpotting");
                cont->store(false);
            }

            prevNgram="-";
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
    string segmentationFile = "/home/brian/intel_index/data/bentham/ben_crowdcat_c_corpus.gtp";
    string charSegFile = "/home/brian/intel_index/data/bentham/manual_segmentations.csv";
    string spottingModelPrefix = "model/CrowdCAT_BENTHAM";
    string savePrefix = "save/sim_BENTHAM";
    if (argc>1)
        savePrefix=argv[1];
    if (argc>2)
        GlobalK::knowledge()->setSimSave(argv[2]);
    else
        GlobalK::knowledge()->setSimSave("save/simulationTracking.csv");
    if (argc>3)
        numSimThreads=atoi(argv[3]);

//#ifndef DEBUG_AUTO
    Simulator sim(dataname,charSegFile);
//#else
//    Simulator sim("test",charSegFile);
//#endif
    int avgCharWidth=-1;
    if (dataname.compare("BENTHAN")==0)
        avgCharWidth=37;
    else if (dataname.compare("NAMES")==0)
        avgCharWidth=20;
    else if (dataname.compare("GW")==0)
        avgCharWidth=38;

    int numSpottingThreads = 5;
    int numTaskThreads = 3;
    int height = 1000;
    int width = 2500;
    int milli = 7000;
    CrowdCAT* crowdcat = new CrowdCAT(lexiconFile,
                        pageImageDir,
                        //segmentationFile,
                        transcriberPrefix,
                        savePrefix,
                        //avgCharWidth,
                        //numSpottingThreads,
                        numTaskThreads,
                        height,
                        width,
                        milli,
                        0//pad
                        );
    atomic_bool cont(true);
    vector<thread*> taskThreads(numSimThreads);
    //string line;
    //cout<<"WAITING FOR ENTRY BEFORE BEGINNING SIM"<<endl;
    //getline(cin, line);
    cout<<"CrowdCAT SIMULATION STARTED"<<endl;
    for (int i=0; i<numSimThreads; i++)
    {
        taskThreads[i] = new thread(threadLoop,crowdcat,&sim,&cont);
        taskThreads[i]->detach();
        
    }
    controlLoop(crowdcat,&cont);

    cout<<"---DONE---"<<endl;
    //delete crowdcat;
    for (int i=0; i<numSimThreads; i++)
        delete taskThreads[i];
    this_thread::sleep_for(chrono::seconds(40));
    delete crowdcat;
}
