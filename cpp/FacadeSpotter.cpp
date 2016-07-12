
#include "FacadeSpotter.h"
void FacadeSpotter::addTestSpottings(string file)
{
    string pageLocation = "/home/brian/intel_index/data/gw_20p_wannot/";
    
    
    
    ifstream in(file);
    assert(in.is_open());
    string line;
    
    //map<string,vector<Spotting> > toAdd;
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
        string spottingId = strV[0]+strV[1];
        int spottingNum = stoi(strV[1]);

        string page = strV[2];
        size_t startpos = page.find_first_not_of(" \t");
        
        int tlx=stoi(strV[3]);
        int tly=stoi(strV[4]);
        int brx=stoi(strV[5]);
        int bry=stoi(strV[6]);
        
        float score=-1*stof(strV[7]);
        bool truth = strV[8].find("true")!= string::npos?true:false;
        
        Spotting spotting(tlx, tly, brx, bry, stoi(page), corpus->imgForPageId(stoi(page)), ngram, score);
        if (loaded[ngram].size() < spottingNum+1)
            loaded[ngram].resize(spottingNum+1);
        loaded[ngram][spottingNum].push_back(spotting);
    }
    
    /*for (auto p : toAdd)
    {
        loaded[p.first].push_back(p.second);
        //cout << "added "<<p.first<<endl;
    }*/
    
    
}

vector<Spotting>* FacadeSpotter::runQuery(SpottingQuery* query)
{
    vector<Spotting>* ret;
    if (loaded[query->getNgram()].size()>0)
    {
        cout <<"Spotting ["<<query->getNgram()<<"]"<<endl;
        ret = new vector<Spotting>(loaded[query->getNgram()].back());
        loaded[query->getNgram()].pop_back();
    }
    else
    {
        cout <<"Facade Spotter ran out of exemplars for ["<<query->getNgram()<<"]"<<", returning none."<<endl;
    }
    delete query;

    this_thread::sleep_for (chrono::milliseconds(500));

    return ret;
}

FacadeSpotter::FacadeSpotter(MasterQueue* masterQueue, const Knowledge::Corpus* corpus, string modelDir, int numThreads) : Spotter(masterQueue, corpus, modelDir, numThreads)
{
    string file= "./data/GW_spottings_fold1_0.100000.csv";
    addTestSpottings(file);
}

