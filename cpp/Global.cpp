#include "Global.h"
GlobalK* GlobalK::_self=NULL;
GlobalK::GlobalK()
{
    string filePaths[2] = {"./data/top500_bigrams_in_freq_order.txt" , "./data/top500_trigrams_in_freq_order.txt"};
    for (int i=0; i<1+(MAX_N-MIN_N); i++)
    {
        ifstream in(filePaths[i]);
        string ngram;
        while(getline(in,ngram))
        {   
            //transform(ngram.begin(), ngram.end(), ngram.begin(), ::tolower);
            ngram=lowercaseAndStrip(ngram);
            ngramRanks[i+MIN_N].push_back(ngram);
        }
        in.close();
    }
#ifdef NO_NAN
    xLock.lock();
#endif
}

GlobalK::~GlobalK()
{
#ifdef NO_NAN
    if (trackFile.good())
        trackFile.close();
    for (auto p : accumRes)
        delete p.second;
#endif
}
string GlobalK::lowercaseAndStrip(string s)
{
    string ret="";
    for (int i=0; i<s.length(); i++)
    {
        if (s[i]!=' ' && s[i]!='\n' &&  s[i]!='\t' &&  s[i]!='\r')
            ret+=tolower(s[i]);
    }
    return ret;
}

GlobalK* GlobalK::knowledge()
{
    if (_self==NULL)
        _self=new GlobalK();
    return _self;
}

#ifdef NO_NAN
void GlobalK::setSimSave(string file)
{
    spottingFile = file+".spots";
    transBadBatch=transBadNgram=transSent=spotSent=spotAccept=spotReject=spotAutoAccept=spotAutoReject=newExemplarSpotted=0;
    struct stat buffer;
    bool appending = (stat (file.c_str(), &buffer) == 0);
    trackFile.open(file,ofstream::app|ofstream::out);
    if (!appending)
    {
        trackFile<<"time,accuracyTrans,pWordsTrans,"/*pWords80_100,pWords60_80,pWords40_60,pWords20_40,pWords0_20,pWords0,*/"transSent,badTransBatchs,"/*badTransNgram,spotSent,spotAccept,spotReject,spotAutoAccept,spotAutoReject,newExemplarsSpotted,badPrunes,"*/;
        trackFile<<"accuracyTrans_IV,pWordsTrans_IV,"/*pWords80_100_IV,pWords60_80_IV,pWords40_60_IV,pWords20_40_IV,pWords0_20_IV,pWords0_IV,*/"misTrans"<<endl;
    }

    ifstream in (spottingFile);
    if (!in.good())
        in.open(spottingFile+".bck");
    if (in.good())
    {
        spotMut.lock();
        string line;
        getline(in,line);
        assert(line.compare("[accums]")==0);
        getline(in,line);
        int num = stoi(line);
        for (int i=0; i<num; i++)
        {
            getline(in,line);
            int ind = line.find(':');
            string ngram = line.substr(0,ind-1);
            string scores = line.substr(ind+2);
            stringstream ss(scores);
            while (getline(ss,line,','))
            {
                spottingAccums[ngram].push_back(stof(line));
            }
        }

        getline(in,line);
        assert(line.compare("[exemplars]")==0);
        getline(in,line);
        num = stoi(line);
        for (int i=0; i<num; i++)
        {
            getline(in,line);
            int ind = line.find(':');
            string ngram = line.substr(0,ind-1);
            string scores = line.substr(ind+2);
            stringstream ss(scores);
            while (getline(ss,line,','))
            {
                spottingExemplars[ngram].push_back(stof(line));
            }
        }

        getline(in,line);
        assert(line.compare("[normals]")==0);
        getline(in,line);
        num = stoi(line);
        for (int i=0; i<num; i++)
        {
            getline(in,line);
            int ind = line.find(':');
            string ngram = line.substr(0,ind-1);
            string scores = line.substr(ind+2);
            stringstream ss(scores);
            while (getline(ss,line,','))
            {
                spottingNormals[ngram].push_back(stof(line));
            }
        }

        getline(in,line);
        assert(line.compare("[others]")==0);
        getline(in,line);
        num = stoi(line);
        for (int i=0; i<num; i++)
        {
            getline(in,line);
            int ind = line.find(':');
            string ngram = line.substr(0,ind-1);
            string scores = line.substr(ind+2);
            stringstream ss(scores);
            while (getline(ss,line,','))
            {
                spottingOthers[ngram].push_back(stof(line));
            }
        }
        spotMut.unlock();
        in.close();
    }
}
#endif

int GlobalK::getNgramRank(string ngram)
{
    transform(ngram.begin(), ngram.end(), ngram.begin(), ::tolower);
    const vector<string>& ranks = ngramRanks[ngram.length()];
    for (int i=0; i<ranks.size(); i++)
    {
        if (ranks[i].compare(ngram)==0)
            return i+1;
    }
    return ranks.size()+1;
}


double GlobalK::otsuThresh(vector<int> histogram)
{
    double sum =0;
    int total=0;
    for (int i = 1; i < histogram.size(); ++i)
    {
        total+=histogram[i];
        sum += i * histogram[i];
    }
    double sumB = 0;
    double wB = 0;
    double wF = 0;
    double mB;
    double mF;
    double max = 0.0;
    double between = 0.0;
    double threshold1 = 0.0;
    double threshold2 = 0.0;
    for (int i = 0; i < histogram.size(); ++i)
    {
        wB += histogram[i];
        if (wB == 0)
            continue;
        wF = total - wB;
        if (wF == 0)
            break;
        sumB += i * histogram[i];
        mB = sumB / (wB*1.0);
        mF = (sum - sumB) / (wF*1.0);
        between = wB * wF * pow(mB - mF, 2);
        if ( between >= max )
        {
            threshold1 = i;
            if ( between > max )
            {
                threshold2 = i;
            }
            max = between;
        }
    }

    return ( threshold1 + threshold2 ) / 2.0;
}
void GlobalK::saveImage(const cv::Mat& im, ofstream& out)
{
    if (im.cols==0)
    {
        out<<"X"<<endl;
    }
    else
    {
        bool color=im.channels()==3;
        vector<unsigned char> encoded;
        cv::imencode(".png",im,encoded);
        out<<color<<"\n";
        out<<encoded.size()<<"\n";
        for (unsigned char c : encoded)
            out<<c;
        out<<"\n";
    }
}
void GlobalK::loadImage(cv::Mat& im, ifstream& in)
{
    string line;
    getline(in,line);
    if (line[0]!='X')
    {
        bool color=stoi(line);
        getline(in,line);
        int size = stoi(line);
        vector<unsigned char> encoded(size);
        for (int i=0; i<size; i++)
        {
            encoded[i]=in.get();
        }
        assert('\n'==in.get());
        im=cv::imdecode(encoded,color?CV_LOAD_IMAGE_COLOR:CV_LOAD_IMAGE_GRAYSCALE);
    }
}

string GlobalK::currentDateTime() //from http://stackoverflow.com/a/10467633/1018830
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}
#ifdef NO_NAN

void GlobalK::sentSpottings()
{
    spotSent++;
}
void GlobalK::sentTrans()
{
    transSent++;
}
void GlobalK::badTransBatch()
{
    transBadBatch++;
}
void GlobalK::badTransNgram()
{
    transBadNgram++;
}
void GlobalK::accepted()
{
    spotAccept++;
}
void GlobalK::rejected()
{
    spotReject++;
}
void GlobalK::autoAccepted()
{
    spotAutoAccept++;
}
void GlobalK::autoRejected()
{
    spotAutoReject++;
}
void GlobalK::newExemplar()
{
    newExemplarSpotted++;
}
void GlobalK::saveTrack(float accTrans, float pWordsTrans, /*float pWords80_100, float pWords60_80, float pWords40_60, float pWords20_40, float pWords0_20, float pWords0,*/ string misTrans,
                       float accTrans_IV, float pWordsTrans_IV, /*float pWords80_100_IV, float pWords60_80_IV, float pWords40_60_IV, float pWords20_40_IV, float pWords0_20_IV, float pWords0_IV,*/ string misTrans_IV)
{
    track<<currentDateTime()<<","<<accTrans<<","<<pWordsTrans<<","<</*pWords80_100<<","<<pWords60_80<<","<<pWords40_60<<","<<pWords20_40<<","<<pWords0_20<<","<<pWords0<<","<<*/transSent<<","<<transBadBatch/*<<","<<transBadNgram<<","<<spotSent<<","<<spotAccept<<","<<spotReject<<","<<spotAutoAccept<<","<<spotAutoReject<<","<<newExemplarSpotted<<","<<badPrunes<<","*/;
    track << accTrans_IV<<","<<pWordsTrans_IV<<","<</*pWords80_100_IV<<","<<pWords60_80_IV<<","<<pWords40_60_IV<<","<<pWords20_40_IV<<","<<pWords0_20_IV<<","<<pWords0_IV<<","<<*/misTrans<<endl;//","<<misTrans_IV<<endl;
    //track+=currentDateTime()+","+accTrans+","+pWordsTrans+","+pWords80_100+","+pWords60_80+","+pWords40_60+","+pWords20_40+","+pWords0_20+","+pWords0+","+transSent+","+spotSent+","+spotAccept+","+spotReject+","+spotAutoAccept+","+spotAutoReject+","+newExemplarSpotted+"\n";
    transBadBatch=transBadNgram=transSent=spotSent=spotAccept=spotReject=spotAutoAccept=spotAutoReject=newExemplarSpotted=0;

}

void GlobalK::writeTrack()
{
    trackFile<<track.rdbuf()<<flush;
    track.str(string());
    track.clear();

    rename( spottingFile.c_str() , (spottingFile+".bck").c_str() );
    ofstream out(spottingFile);
    spotMut.lock();
    out<<"[accums]\n"<<spottingAccums.size()<<endl;
    for (auto aps : spottingAccums)
    {
        out<<aps.first<<" : ";
        for (float ap : aps.second)
            out<<ap<<",";
        out<<endl;
    }
    out<<"[exemplars]\n"<<spottingExemplars.size()<<endl;
    for (auto aps : spottingExemplars)
    {
        out<<aps.first<<" : ";
        for (float ap : aps.second)
            out<<ap<<",";
        out<<endl;
    }
    out<<"[normals]\n"<<spottingNormals.size()<<endl;
    for (auto aps : spottingNormals)
    {
        out<<aps.first<<" : ";
        for (float ap : aps.second)
            out<<ap<<",";
        out<<endl;
    }
    out<<"[others]\n"<<spottingOthers.size()<<endl;
    for (auto aps : spottingOthers)
    {
        out<<aps.first<<" : ";
        for (float ap : aps.second)
            out<<ap<<",";
        out<<endl;
    }
    spotMut.unlock();
    out.close();
}

void GlobalK::storeSpottingAccum(string ngram, float ap)
{
    spotMut.lock();
    spottingAccums[ngram].push_back(ap);
    spotMut.unlock();
}
void GlobalK::storeSpottingExemplar(string ngram, float ap)
{
    spotMut.lock();
    spottingExemplars[ngram].push_back(ap);
    spotMut.unlock();
}
void GlobalK::storeSpottingNormal(string ngram, float ap)
{
    spotMut.lock();
    spottingNormals[ngram].push_back(ap);
    spotMut.unlock();
}
void GlobalK::storeSpottingOther(string ngram, float ap)
{
    spotMut.lock();
    spottingOthers[ngram].push_back(ap);
    spotMut.unlock();
}

vector<SubwordSpottingResult>* GlobalK::accumResFor(string ngram)
{
    accumResMut.lock();
    if (accumRes.find(ngram) == accumRes.end())
        accumRes[ngram] = new vector<SubwordSpottingResult>();
    vector<SubwordSpottingResult>* toRet= accumRes.at(ngram);
    accumResMut.unlock();
    return toRet;
}
#endif

#ifdef TEST_MODE
bool GlobalK::ngramAt(string ngram, int pageId, int tlx, int tly, int brx, int bry)
{
    float overlap_insides_thresh = OVERLAP_INSIDE_THRESH;
    float overlap_consume_thresh = OVERLAP_CONSUME_THRESH;
    float overlap_size_thresh = OVERLAP_SIDE_THRESH;
    float size_note_included_thresh = SIDE_NOT_INCLUDED_THRESH;


    WordBound searchL(tlx-5,tly-5,brx,bry);
    WordBound searchU(brx,bry,brx+5,bry+5);
    auto iterL = wordBounds[pageId].lower_bound(searchL);
    auto iterU = wordBounds[pageId].upper_bound(searchU);

    int bestOverlap =-1;
    const WordBound* best;
    while (iterL != iterU)
    {
        int overlap = ( min(iterL->brx, brx)-max(iterL->tlx,tlx) ) *
            ( min(iterL->bry, bry)-max(iterL->tly,tly) );
        assert(overlap != bestOverlap);
        if (overlap > bestOverlap)
        {
            bestOverlap = overlap;
            best = &(*iterL);
        }
        iterL++;
    }
    int loc = best->text.find(ngram);
    if (loc == string::npos)
        return false;

    //int ngramOverlap = min(best->startBounds[loc],tlx) - max(best->endBounds, brx);
    if (
            (tlx>=best->startBounds[loc] && brx<=best->endBounds[loc+ngram.size()-1] && (brx-tlx)/(best->endBounds[loc+ngram.size()-1]-best->startBounds[loc]+0.0) > overlap_insides_thresh) ||
            (tlx<=best->startBounds[loc] && brx>=best->endBounds[loc+ngram.size()-1] && (brx-tlx)/(best->endBounds[loc+ngram.size()-1]-best->startBounds[loc]+0.0) < overlap_consume_thresh) ||
            ((min(brx,best->endBounds[loc+ngram.size()-1])-max(tlx,best->startBounds[loc]))/(best->endBounds[loc+ngram.size()-1]-best->startBounds[loc]+0.0) > overlap_size_thresh && max(max(tlx,best->startBounds[loc])-min(tlx,best->startBounds[loc]),max(brx,best->endBounds[loc+ngram.size()-1])-min(brx,best->endBounds[loc+ngram.size()-1]))/(min(brx,best->endBounds[loc+ngram.size()-1])-max(tlx,best->startBounds[loc])+0.0) < size_note_included_thresh)
       )
    {
        return true;
    }
    
    return false;
}

void GlobalK::addWordBound(string word, int pageId, int tlx, int tly, int brx, int bry, vector<int> startBounds, vector<int> endBounds)
{
    wordBounds[pageId].emplace(word,tlx,tly,brx,bry,startBounds,endBounds);
}
#endif
