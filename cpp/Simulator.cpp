#include "Simulator.h"
#include <csignal>

Simulator::Simulator()
{
    spotNet = net_load (spotNet_filename);

    ifstream in(prob_filename);
    string line;
    getline(in,line);
    isInSkipProb= stof(line);
    getline(in,line);
    falseNegativeProb = stof(line);
    getline(in,line);
    falsePositiveProb = stof(line);
    getline(in,line);
    notInSkipProb = stof(line);
    getline(in,line);
    notInFalsePositiveProb = stof(line);
}
//I need, for each word in the corpus"
//*GT
//*boundaries between letters


vector<int> Simulator::spottings(string ngram, vector<Location> locs, vector<string> gt, string prevNgram)
{
    assert(ngram.length()==2);
    int numSkip=0;
    int numT=0;
    int numF=0;
    int numObv=0;
    float error;
    vector<int> labels = getSpottingLabels(ngram,locs,&error); //(locs.size());
    for (int i=0; i<locs.size(); i++)
    {
        //labels[i] = getSpottingLabel(ngram,locs[i]);
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
    //nIn[2*N_LETTERS+4]=(numObv/5.0)*2.0-1.0;
    nIn[2*N_LETTERS+4]=2*error-1.0;

    float nOut;
    net_compute (spotNet, nIn, &nOut);
    //int milli = MINSPOT_MILLI+MAX_SPOT_MILLI*(nOut+1)/2.0;
    int milli = nOut*2*MILLI_STD + MILLI_MEAN;
    this_thread::sleep_for(chrono::milliseconds(milli));

    return labels;
}

vector<int> Simulator::getSpottingLabels(string ngram, vector<Location> locs, float* error)
{
    vector<int> ret;
    for (Location loc : locs)
    {
        int maxOverlap=0;
        int w=-1;
        for (int i=0; i<corpusWord.size(); i++)
        {
            if (    loc.pageId == corpusPage[i] &&
                    loc.y1<corpusYBounds[i].second &&
                    loc.y2>corpusYBounds[i].first &&
                    loc.x1<corpusXLetterBounds[i].back() &&
                    loc.x2>corpusXLetterBounds[i].front()
               )
            {
                int overlap = ( min(loc.y2,corpusYBounds[i].second)-max(loc.y1,corpusYBounds[i].first) ) *
                              ( min(loc.x2,corpusXLetterBounds[i].back())-max(loc.x1,corpusXLetterBounds[i].front()) );
                if (overlap>maxOverlap)
                {
                    maxOverlap=overlap;
                    w=i;
                }
            }
        }
        if (w>=0)
        {
            int l1 = corpusWord[w].indexOf(ngram[0]);
            int l2 = corpusWord[w].indexOf(ngram[1]);
            if (l1==l2-1)
            {
                if (RAND_PROB<isInSkipProb)
                    ret.push_back( -1 );
                else
                {
                    if (
                            (loc.x1>=corpusXLetterBounds[w][l1] && loc.x2<=corpusXLetterBounds[w][l2+1] && (loc.x2-loc.x1)/(corpusXLetterBounds[w][l2+1]-corpusXLetterBounds[w][l1]+0.0) > OVERLAP_INSIDE_THRESH) ||
                            (loc.x1<=corpusXLetterBounds[w][l1] && loc.x2>=corpusXLetterBounds[w][l2+1] && (loc.x2-loc.x1)/(corpusXLetterBounds[w][l2+1]-corpusXLetterBounds[w][l1]+0.0) < OVERLAP_CONSUME_THRESH) ||
                            ((min(loc.x2,corpusXLetterBounds[w][l2+1])-max(loc.x1,corpusXLetterBounds[w][l1]))/(corpusXLetterBounds[w][l2+1]+corpusXLetterBounds[w][l1]+0.0) > OVERLAP_SIDE_THRESH && max(max(loc.x1,corpusXLetterBounds[w][l1])-min(loc.x1,corpusXLetterBounds[w][l1]),max(loc.x2,corpusXLetterBounds[w][l2+1])-min(loc.x2,corpusXLetterBounds[w][l2+1]))/(min(loc.x2,corpusXLetterBounds[w][l2+1])-max(loc.x1,corpusXLetterBounds[w][l1])+0.0) < SIDE_NOT_INCLUDED_THRESH)
                       )
                    {
                        if  (RAND_PROB < falseNegativeProb)
                        {
                            ret.push_back(0);
                            *error+=1.0/locs.size();
                        }
                        else
                            ret.push_back(1);
                    }
                    else
                    {
                        if (RAND_PROB < falsePositiveProb)
                        {
                            ret.push_back(1);
                            *error+=1.0/locs.size();
                        }
                        else
                            ret.push_back(0);
                    }

                }
                
            }
            else
            {
                if (RAND_PROB<notInSkipProb)
                    ret.push_back( -1 );
                else
                {
                    if (RAND_PROB < notInFalsePositiveProb)
                    {
                        ret.push_back(1);
                        *error+=1.0/locs.size();
                    }
                    else
                        ret.push_back(0);
                }
            }
        }
        else
        {
            cout<<"Error, spotting did not match word"<<endl;
            raise(SIGINT);
            ret.push_back( 0 );
        }
    }
    return ret;
}
