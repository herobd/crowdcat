
#include "FacadeSpotter.h"
void FacadeSpotter::addTestSpottings(string file)
{
    string pageLocation = "/home/brian/intel_index/data/gw_20p_wannot/";
    
    
    
    ifstream in(file);
    assert(in.is_open());
    string line;
    
    map<string,vector<Spotting> > toAdd;
    while(std::getline(in,line))
    {
        vector<string> strV;
        //split(line,',',strV);
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, ',')) {
            strV.push_back(item);
        }
        
        
        
        string ngram = strV[0];
        string spottingId = strV[0]+strV[1]+":"+to_string(testIter);
        
        string page = strV[2];
        size_t startpos = page.find_first_not_of(" \t");
        if( string::npos != startpos )
        {
            page = page.substr( startpos );
        }
        if (pages.find(page)==pages.end())
        {
            pages[page] = cv::imread(pageLocation+page);
            assert(pages[page].cols!=0);
        }
        
        int tlx=stoi(strV[3]);
        int tly=stoi(strV[4]);
        int brx=stoi(strV[5]);
        int bry=stoi(strV[6]);
        
        float score=-1*stof(strV[7]);
        bool truth = strV[8].find("true")!= string::npos?true:false;
        
        Spotting spotting(tlx, tly, brx, bry, stoi(page), &pages[page], ngram, score);
        toAdd[ngram].push_back(add(spotting));
    }
    
    for (auto p : toAdd)
    {
        loaded[p.first].push_back(p.second);
        //cout << "added "<<p.first<<endl;
    }
    
    
}

vector<Spotting> FacadeSpotter::runQuery(SpottingQuery* query)
{
    vector<Spotting> ret;
    if (loaded[query->ngram].size()>0)
    {
        cout <<"Spotting ["<<query->ngram<<"]"<<endl;
        ret = loaded[query->ngram].back();
        loaded[query->ngram].pop_back();
    }
    else
    {
        cout <<"Facade Spotter ran out of exemplars for ["<<query->ngram<<"]"<<", returning none."<<endl;
    }
    delete query;

    //TODO sleep
}

FacadeSpotter::FacadeSpotter(MasterQueue* masterQueue, const Corpus* corpus, string modelDir, int numThreads)
{
    vector<string> files = {"./data/GW_agSpottings_fold1_0.100000.csv"};
    for (string file : files)
        addTestSpottings(file);
}

//TODO Need to test removing query from queue and from live
