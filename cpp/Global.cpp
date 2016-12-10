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
            transform(ngram.begin(), ngram.end(), ngram.begin(), ::tolower);
            ngramRanks[i+MIN_N].push_back(ngram);
        }
        in.close();
    }

    transSent=spotSent=spotAccept=spotReject=spotAutoAccept=spotAutoReject=newExemplarSpotted=0;
    trackFile.open("save/simulationTracking.csv");
    trackFile<<"time,pWordsTrans,pWords80_100,pWords60_80,pWords40_60,pWords20_40,pWords0_20,pWords0,transSent,spotSent,spotAccept,spotReject,spotAutoAccept,spotAutoReject,newExemplarsSpotted"<<endl;
}

GlobalK::~GlobalK()
{
    trackFile.close();
}

GlobalK* GlobalK::knowledge()
{
    if (_self==NULL)
        _self=new GlobalK();
    return _self;
}

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

void GlobalK::sentSpottings()
{
    spotSent++;
}
void GlobalK::sentTrans()
{
    transSent++;
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

void GlobalK::saveTrack(float pWordsTrans, float pWords80_100, float pWords60_80, float pWords40_60, float pWords20_40, float pWords0_20, float pWords0)
{
    time_t timeSec;
    time(&timeSec);
    trackFile<<timeSec<<","<<pWordsTrans<<","<<pWords80_100<<","<<pWords60_80<<","<<pWords40_60<<","<<pWords20_40<<","<<pWords0_20<<","<<pWords0<<","<<transSent<<","<<spotSent<<","<<spotAccept<<","<<spotReject<<","<<spotAutoAccept<<","<<spotAutoReject<<","<<newExemplarSpotted<<endl;
    transSent=spotSent=spotAccept=spotReject=spotAutoAccept=spotAutoReject=newExemplarSpotted=0;
}
