#include "Simulator.h"

Simulator::Simulator()
{
    spotNet = net_load (spotNet_filename);
}


vector<int> Simulator::spottings(string ngram, vector<Location> locs, vector<string> gt, string prevNgram)
{
    assert(ngram.length()==2);
    int numSkip=0;
    int numT=0;
    int numF=0;
    int numObv=0;
    vector<int> labels(locs.size());
    for (int i=0; i<locs.size(); i++)
    {
        labels[i] = getSpottingLabel(ngram,locs[i]);
        if (labels[i]==0)
            numF++;
        else if (labels[i]==1)
            numT++;
        else
            numSkip++;
        if (gt[i].compare("0")==0)
            numObv++;
    }
    
    float nIn[nInSIze];
    for (int i=0; i<2*N_LETTERS; i++)
        nIn[i]=0;
    nIn[ngram[0]-'a']=1;
    nIn[N_LETTERS+ngram[1]-'a']=1;
    nIn[2*N_LETTERS+0]=(numSkip/5.0)*2.0-1.0;
    nIn[2*N_LETTERS+1]=(numT/5.0)*2.0-1.0;
    nIn[2*N_LETTERS+2]=(numF/5.0)*2.0-1.0;
    nIn[2*N_LETTERS+3]=(prevNgram.compare(ngram)==0)?1:0;
    nIn[2*N_LETTERS+4]=(numObv/5.0)*2.0-1.0;

    float nOut;
    net_compute (spotNet, nIn, &nOut);
    int milli = MINSPOT_MILLI+MAX_SPOT_MILLI*(nOut+1)/2.0;
    this_thread::sleep_for(chrono::milliseconds(milli));

    return labels;
}

vector<int> Simulator::getSpottingLabel(string ngram, vector<Location> locs)
{

}
