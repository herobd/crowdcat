#include "Simulator.h"
#include <csignal>


//BENHTANM 
//spottingAverageMilli=16116
//spottingErrorProbConst=0.035
//spottingSkipProbConst=0.078


//NAMES 
//spottingAverageMilli=16783
//spottingErrorProbConst=0.058
//spottingSkipProbConst=0.029

Simulator::Simulator(string dataname, string segCSV)
{
    //TODO read in seg GT
    ifstream in (segCSV);
    string line;
    //getline(in,line);//header
    while (getline(in,line))
    {
        string s;
        std::stringstream ss(line);
        getline(ss,s,',');
        corpusWord.push_back(s);
        getline(ss,s,',');
        corpusPage.push_back(stoi(s)+1); //+1 is correction from my creation of segmentation file
        getline(ss,s,',');//x1
        int x1=stoi(s);
        getline(ss,s,',');
        int y1=stoi(s);
        getline(ss,s,',');//x2
        getline(ss,s,',');
        int y2=stoi(s);
        corpusYBounds.push_back(make_pair(y1,y2));
        vector<int> lettersStart, lettersEnd;
        vector<int> lettersStartRel, lettersEndRel;

        while (getline(ss,s,','))
        {
            lettersStart.push_back(stoi(s));
            lettersStartRel.push_back(stoi(s)-x1);
            getline(ss,s,',');
            lettersEnd.push_back(stoi(s));
            lettersEndRel.push_back(stoi(s)-x1);
            //getline(ss,s,',');//conf
        }
        assert(lettersStart.size() == corpusWord.back().length());
        corpusXLetterStartBounds.push_back(lettersStart);
        corpusXLetterEndBounds.push_back(lettersEnd);
        corpusXLetterStartBoundsRel.push_back(lettersStartRel);
        corpusXLetterEndBoundsRel.push_back(lettersEndRel);
    }
    in.close();
    //sending corpus word is purely for error checking
    GlobalK::knowledge()->setCorpusXLetterBounds(&corpusXLetterStartBoundsRel,&corpusXLetterEndBoundsRel,&corpusWord);

    if (dataname.compare("test")==0)
    {
        spottingAverageMilli=1;
        spottingAverageMilli_prev=1;
        //spottingErrorProbConst=0.035;
        //spottingSkipProbConst=0.078;
        spottingErrorProb_m=0; //https://plot.ly/create/
        spottingErrorProb_b=0;
        spottingSkipProb_m=0;
        spottingSkipProb_b=0;


        transMilli_b=1;//position 0.07 (no -1)
        transMilli_m=1;
        transMilli_notAvail=1;
        transErrorProbAvail=0;
        transErrorProbNotAvail=0;
        //transSkipProb=0;

        manMilli_b=1;
        manMilli_m=1;
        manErrorProb=0;
    }
    else if (dataname.compare("BENTHAM")==0)
    {
        spottingAverageMilli=14235;
        spottingAverageMilli_prev=-4338;
        //spottingErrorProbConst=0.035;
        //spottingSkipProbConst=0.078;
        spottingErrorProb_m=-0.0041; //https://plot.ly/create/
        spottingErrorProb_b=0.061;
        spottingSkipProb_m=0.02652;
        spottingSkipProb_b=0.084055;


        transMilli_b=4392.10;//position 0.07 (no -1)
        transMilli_m=611.95;
        transMilli_notAvail=11539;
        transErrorProbAvail=1- 0.95;
        transErrorProbNotAvail=1- 0.5484;
        //transSkipProb=0.012;

        manMilli_b=2608.0068;
        manMilli_m=726.1412;
        manErrorProb=1-0.8977;
    }
    else
    {
        spottingAverageMilli=19951;
        spottingAverageMilli_prev=-6825;
        spottingErrorProb_m=0.00059863;
        spottingErrorProb_b=0.08758;
        spottingSkipProb_m=-0.000805;
        spottingSkipProb_b=0.010303;

        transMilli_b=4772.45;//position 0.12 (no -1)
        transMilli_m=1260.48;
        transMilli_notAvail=5749;
        transErrorProbAvail=1- 0.8812;
        transErrorProbNotAvail=1- 0.5208;
        //transSkipProb=0.012;

        manMilli_b=4210.1689;
        manMilli_m=670.0784;
        manErrorProb=1-0.6933;
        //transMilli_b=4957.61;//num poss 0.0093
        //transMilli_m=500.34;

        //transMilli_b=5319.33;//prev 0.017
        //2011.88;


    }
}
//I need, for each word in the corpus"
//*GT
//*boundaries between letters

void Simulator::skipAndError(vector<int>& labels)
{
    float T=0;
    for (int c : labels)
        T+=c;
    float e = T*spottingErrorProb_m + spottingErrorProb_b; //
    float s = T*spottingSkipProb_m + spottingSkipProb_b; //
    if (RAND_PROB < e)
    {
        int i = rand()%labels.size();
        labels[i]=!labels[i];
    }
    if (RAND_PROB < s)
    {
        int i = rand()%labels.size();
        labels[i]=-1;
    }
}

vector<int> Simulator::spottings(string ngram, vector<Location> locs, vector<string> gt, string prevNgram)
{
    assert(ngram.length()==2);
    //int numSkip=0;
    //int numT=0;
    //int numF=0;
    //int numObv=0;
    vector<int> labels(locs.size());
    for (int i=0; i<locs.size(); i++)
    {
        if (gt[i].length()>2)
            labels[i] = getSpottingLabel(ngram,locs[i]);
        else
            labels[i] = stoi(gt[i]);
    }
    skipAndError(labels);
    /*for (int i=0; i<locs.size(); i++)
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
    }*/
    
    /*float nIn[nInSIze];
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
    int milli = nOut*2*MILLI_STD + MILLI_MEAN;*/
    //int same = ngram.compare(prevNgram)==0?1:0;
    //int milli = c + numT*t  + same*p + (1.0-error)*a;
    bool prev =ngram.compare(prevNgram);
    int milli = (labels.size()/5.0)*spottingAverageMilli + prev?spottingAverageMilli_prev:0;
    this_thread::sleep_for(chrono::milliseconds(milli));

    return labels;
}

int Simulator::getSpottingLabel(string ngram, Location loc, bool strict)
{
    float overlap_insides_thresh = OVERLAP_INSIDE_THRESH;
    float overlap_consume_thresh = OVERLAP_CONSUME_THRESH;
    float overlap_size_thresh = OVERLAP_SIDE_THRESH;
    float size_note_included_thresh = SIDE_NOT_INCLUDED_THRESH;
    if (strict)
    {
        overlap_insides_thresh = OVERLAP_INSIDE_THRESH_STRICT;
        overlap_consume_thresh = OVERLAP_CONSUME_THRESH_STRICT;
        overlap_size_thresh = OVERLAP_SIDE_THRESH_STRICT;
        size_note_included_thresh = SIDE_NOT_INCLUDED_THRESH_STRICT;
    }
    int maxOverlap=0;
    int w=-1;
    for (int i=0; i<corpusWord.size(); i++)
    {
        if (    loc.pageId == corpusPage[i] &&
                loc.y1<corpusYBounds[i].second &&
                loc.y2>corpusYBounds[i].first &&
                loc.x1<corpusXLetterEndBounds[i].back() &&
                loc.x2>corpusXLetterStartBounds[i].front()
           )
        {
            int overlap = ( min(loc.y2,corpusYBounds[i].second)-max(loc.y1,corpusYBounds[i].first) ) *
                          ( min(loc.x2,corpusXLetterEndBounds[i].back())-max(loc.x1,corpusXLetterStartBounds[i].front()) );
            if (overlap>maxOverlap)
            {
                maxOverlap=overlap;
                w=i;
            }
        }
    }
    if (w>=0)
    {
        //cout<<corpusWord[w]<<" : "<<ngram<<endl;
        //int l1 = corpusWord[w].find(ngram[0]);
        //int l2 = corpusWord[w].find(ngram[1]);
        for (int l2=1; l2<corpusWord[w].length(); l2++)
        {
            int l1=l2-1;
            if (ngram[0]!=corpusWord[w][l1] || ngram[1]!=corpusWord[w][l2])
                continue;
            //if (RAND_PROB<isInSkipProb)
            //    ret.push_back( -1 );
            //else
            {
                /*cout<<"page: "<<loc.pageId<<"  y1:"<<loc.y1<<endl;
                cout<<loc.x1<<"\t"<<loc.x2<<"\t"<<endl;
                cout<<corpusXLetterStartBounds[w][l1]<<"\t"<<corpusXLetterEndBounds[w][l2]<<endl;
                cout<<"inside["<<((loc.x1>=corpusXLetterStartBounds[w][l1] && loc.x2<=corpusXLetterEndBounds[w][l2])?1:0)<<"]: "<<(loc.x2-loc.x1)/(corpusXLetterEndBounds[w][l2]-corpusXLetterStartBounds[w][l1]+0.0)<<" ?> "<<overlap_insides_thresh<<endl;
                cout<<"consume["<<((loc.x1<=corpusXLetterStartBounds[w][l1] && loc.x2>=corpusXLetterEndBounds[w][l2])?1:0)<<"]: "<<(loc.x2-loc.x1)/(corpusXLetterEndBounds[w][l2]-corpusXLetterStartBounds[w][l1]+0.0)<<" ?< "<<overlap_consume_thresh<<endl;
                cout<<"("<<endl<<"side: "<<(min(loc.x2,corpusXLetterEndBounds[w][l2])-max(loc.x1,corpusXLetterStartBounds[w][l1]))/(corpusXLetterEndBounds[w][l2]+corpusXLetterStartBounds[w][l1]+0.0)<<" ?> "<<overlap_size_thresh<<endl;
                cout<<"not inc: "<<max(max(loc.x1,corpusXLetterStartBounds[w][l1])-min(loc.x1,corpusXLetterStartBounds[w][l1]),max(loc.x2,corpusXLetterEndBounds[w][l2])-min(loc.x2,corpusXLetterEndBounds[w][l2]))/(min(loc.x2,corpusXLetterEndBounds[w][l2])-max(loc.x1,corpusXLetterStartBounds[w][l1])+0.0)<<" ?< "<<size_note_included_thresh<<endl<<")"<<endl;*/
                if (
                        (loc.x1>=corpusXLetterStartBounds[w][l1] && loc.x2<=corpusXLetterEndBounds[w][l2] && (loc.x2-loc.x1)/(corpusXLetterEndBounds[w][l2]-corpusXLetterStartBounds[w][l1]+0.0) > overlap_insides_thresh) ||
                        (loc.x1<=corpusXLetterStartBounds[w][l1] && loc.x2>=corpusXLetterEndBounds[w][l2] && (loc.x2-loc.x1)/(corpusXLetterEndBounds[w][l2]-corpusXLetterStartBounds[w][l1]+0.0) < overlap_consume_thresh) ||
                        ((min(loc.x2,corpusXLetterEndBounds[w][l2])-max(loc.x1,corpusXLetterStartBounds[w][l1]))/(corpusXLetterEndBounds[w][l2]-corpusXLetterStartBounds[w][l1]+0.0) > overlap_size_thresh && max(max(loc.x1,corpusXLetterStartBounds[w][l1])-min(loc.x1,corpusXLetterStartBounds[w][l1]),max(loc.x2,corpusXLetterEndBounds[w][l2])-min(loc.x2,corpusXLetterEndBounds[w][l2]))/(min(loc.x2,corpusXLetterEndBounds[w][l2])-max(loc.x1,corpusXLetterStartBounds[w][l1])+0.0) < size_note_included_thresh)
                   )
                {
                    //if  (RAND_PROB < falseNegativeProb)
                    //{
                    //    return 0;
                    //    *error+=1.0/locs.size();
                    //}
                    //else

                    //cout<<"alignment good"<<endl;
                        return 1;
                }
                else
                {
                    //if (RAND_PROB < falsePositiveProb)
                    //{
                    //    return 1;
                    //    *error+=1.0/locs.size();
                    //}
                    //else

                    //cout<<"alignment bad"<<endl;
                        return 0;
                }

            }
            
        }
        //else if we didn't find the ngram
        {
            //if (RAND_PROB<notInSkipProb)
            //    ret.push_back( -1 );
            //else
            {
                //if (RAND_PROB < notInFalsePositiveProb)
                //{
                //    return 1;
                //    *error+=1.0/locs.size();
                //}
                //else

                //cout<<"letters bad"<<endl;
                    return 0;
            }
        }
    }
    else
    {
        cout<<"Error, spotting did not match word"<<endl;
        cout<<"   Page:"<<loc.pageId<<" bb: "<<loc.x1<<" "<<loc.y1<<" "<<loc.x2<<" "<<loc.y2<<endl;
        //raise(SIGINT);
        return 0;
    }

}

vector<int> Simulator::newExemplars(vector<string> ngrams, vector<Location> locs, string prevNgram)
{
    vector<int> labels(locs.size());
    for (int i=0; i<locs.size(); i++)
       labels[i] = getSpottingLabel(ngrams[i],locs[i],true);
    skipAndError(labels);
    
    int milli = (labels.size()/5.0)*spottingAverageMilli;
    this_thread::sleep_for(chrono::milliseconds(milli));

    return labels;
}

string Simulator::transcription(int wordIndex, vector<SpottingPoint> spottings, vector<string> poss, string gt, bool lastWasTrans)
{
    string ret ="";
    int milli;
    transform(gt.begin(), gt.end(), gt.begin(), ::tolower);
    for (int i=0; i< poss.size(); i++)
    {
        if (gt.compare(poss[i])==0)
        {
            ret=poss[i];
            milli=transMilli_b + transMilli_m*i;
        }
    }
    if (ret.length()==0)
    {
        milli=transMilli_notAvail;

        for (SpottingPoint sp : spottings)
        {
            if (0==getSpottingLabel(sp.getNgram(),Location(sp.page,sp.x1,sp.y1,sp.x2,sp.y2)))
            {
                ret="$REMOVE:"+sp.getId()+"$";
                break;
            }
        }

        if (ret.length()==0)
        {
            if (RAND_PROB < transErrorProbNotAvail)
                ret="$REMOVE:"+spottings.front().getId()+"$";//If an error is made, I'm just removing a spottings. Not sure what actually should happen
            else
                ret="$ERROR$";
        }
        else
        {
            if (RAND_PROB < transErrorProbNotAvail)
                ret="$ERROR$";//Assume that user misses that there ngram is wrong
        }
    }
    else
    {
        if (RAND_PROB < transErrorProbAvail)
            ret="$ERROR$";//Assume user didn't see correct trans
    }
            
    this_thread::sleep_for(chrono::milliseconds(milli));

    return ret;
}

string Simulator::manual(int wordIndex, vector<string> poss, string gt, bool lastWasMan)
{
    int milli = manMilli_b + manMilli_m*gt.length();
    string ret=gt;
    if (RAND_PROB < manErrorProb)
        ret[0]='!';
    this_thread::sleep_for(chrono::milliseconds(milli));
    return ret;
}
