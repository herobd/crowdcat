#include "Word.h"

atomic_uint Word::_id(0);
const cv::Mat Word::getWordImg() const
{
    return (*pagePnt)(cv::Rect(tlx,tly,brx-tlx+1,bry-tly+1));
}

const cv::Mat Word::getImg()// const
{
#ifdef TEST_MODE
        //cout<<"[read] "<<gt<<" ("<<tlx<<","<<tly<<") getImg"<<endl;
#endif
    pthread_rwlock_rdlock(&lock);
    const cv::Mat ret = getWordImg();
    pthread_rwlock_unlock(&lock);
#ifdef TEST_MODE
        //cout<<"[unlock] "<<gt<<" ("<<tlx<<","<<tly<<") getImg"<<endl;
#endif
    return ret;
}

void Word::getWordImgAndBin(cv::Mat& wordImg, cv::Mat& b)
{
    //cv::Mat b;
    int blockSize = (1+bry-tly)/2;
    if (blockSize%2==0)
        blockSize++;
    wordImg = getWordImg();
    //wordImg = (*pagePnt)(cv::Rect(tlx,tly,brx-tlx+1,bry-tly+1));
    if (wordImg.type()==CV_8UC3)
        cv::cvtColor(wordImg,wordImg,CV_RGB2GRAY);
    cv::adaptiveThreshold(wordImg, b, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, blockSize, 10);
}

void Word::findBaselines(const cv::Mat& gray, const cv::Mat& bin)
{
    assert(gray.cols==bin.cols && gray.rows==bin.rows); 
    //top and botBaseline should have page cords.
    int avgWhite=0;
    int countWhite=0;
    cv::Mat hist = cv::Mat::zeros(gray.rows,1,CV_32F);
    map<int,int> topPixCounts, botPixCounts;
    for (int c=0; c<gray.cols; c++)
    {
        int topPix=-1;
        int lastSeen=-1;
        for (int r=0; r<gray.rows; r++)
        {
            if (bin.at<unsigned char>(r,c))
            {
                if (topPix==-1)
                {
                    topPix=r;
            }
                lastSeen=r;
            }
            else
            {
                avgWhite+=gray.at<unsigned char>(r,c);
                countWhite++;
            }
            hist.at<float>(r,0)+=gray.at<unsigned char>(r,c);
        }
        if (topPix!=-1)
            topPixCounts[topPix]++;
        if (lastSeen!=-1)
            botPixCounts[lastSeen]++;
    }
    avgWhite /= countWhite;

    int maxTop=-1;
    int maxTopCount=-1;
    for (auto c : topPixCounts)
    {
        if (c.second > maxTopCount)
        {
            maxTopCount=c.second;
            maxTop=c.first;
        }
    }
    int maxBot=-1;
    int maxBotCount=-1;
    for (auto c : botPixCounts)
    {
        if (c.second > maxBotCount)
        {
            maxBotCount=c.second;
            maxBot=c.first;
        }
    }

    //cv::Mat kernel = cv::Mat::ones( 5, 1, CV_32F )/ (float)(5);
    //cv::filter2D(hist, hist, -1 , kernel );
    cv::Mat edges;
    int pad=5;
    cv::Mat paddedHist = cv::Mat::ones(hist.rows+2*pad,1,hist.type());
    double avg=0;
    double maxHist=-99999;
    double minHist=99999;
    for (int r=0; r<hist.rows; r++)
    {
        avg+=hist.at<float>(r,0);
        if (hist.at<float>(r,0)>maxHist)
            maxHist=hist.at<float>(r,0);
        if (hist.at<float>(r,0)<minHist)
            minHist=hist.at<float>(r,0);
    }
    avg/=hist.rows;
    paddedHist *= avg;
    hist.copyTo(paddedHist(cv::Rect(0,pad,1,hist.rows)));
    float kernelData[11] = {1,1,1,1,.5,0,-.5,-1,-1,-1,-1};
    cv::Mat kernel = cv::Mat(11,1,CV_32F,kernelData);
    cv::filter2D(paddedHist, edges, -1 , kernel );//, Point(-1,-1), 0 ,BORDER_AVERAGE);
    float kernelData2[11] = {.1,.1,.1,.1,.1,.1,.1,.1,.1,.1,.1};
    cv::Mat kernel2 = cv::Mat(11,1,CV_32F,kernelData2);
    cv::Mat blurred;
    cv::filter2D(hist, blurred, -1 , kernel2 );//, Point(-1,-1), 0 ,BORDER_AVERAGE);
    topBaseline=-1;
    float maxEdge=-9999999;
    botBaseline=-1;
    float minEdge=9999999;
    float minPeak=9999999;
    float center=-1;
    for (int r=pad; r<gray.rows+pad; r++)
    {
        assert(r<edges.rows);
        float v = edges.at<float>(r,0);
        if (v>maxEdge)
        {
            maxEdge=v;
            topBaseline=r-pad;
        }
        if (v<minEdge)
        {
            minEdge=v;
            botBaseline=r-pad;
        }

        assert(r-pad<blurred.rows);
        if (blurred.at<float>(r-pad,0) < minPeak) {
            center=r-pad;
            minPeak=blurred.at<float>(r-pad,0);
        }
    }
    if (topBaseline>center)
    {
        if (maxTop < center)
            topBaseline=maxTop;
        else
        {
            maxEdge=-999999;
            for (int r=pad; r<center+pad; r++)
            {
                assert(edges.rows>r);
                float v = edges.at<float>(r,0);
                if (v>maxEdge)
                {
                    maxEdge=v;
                    topBaseline=r-pad;
                }
            }
        }
    }
    if (botBaseline<center)
    {
        if (maxBot > center)
            botBaseline=maxBot;
        else
        {
            minEdge=999999;
            for (int r=center+1; r<gray.rows+pad; r++)
            {
                assert(edges.rows>r);
                float v = edges.at<float>(r,0);
                if (v<minEdge)
                {
                    minEdge=v;
                    botBaseline=r-pad;
                }
            }
        }
    }
    if (botBaseline < topBaseline)//If they fail this drastically, the others won't be much better.
    {
        topBaseline=maxTop;
        botBaseline=maxBot;
    }
    topBaseline += tly;
    botBaseline += tly;
}

void Corpus::save(ofstream& out)
{
    //string saveName = savePrefix+"_Corpus.sav";
    //ofstream out(saveName);
    out<<"CORPUS"<<endl;
    out<<averageCharWidth<<"\n"<<countCharWidth<<"\n"<<threshScoring<<"\n";
    pthread_rwlock_rdlock(&pagesLock);
    out<<pages.size()<<"\n";
    for (auto p : pages)
    {
        out<<p.first<<"\n";
        p.second->save(out);
    }
    out<<pageIdMap.size()<<"\n";
    for (auto p : pageIdMap)
    {
        out<<p.first<<"\n"<<p.second<<"\n";
    }
    out<<numWordsReadIn<<"\n";
    pthread_rwlock_unlock(&pagesLock);

    pthread_rwlock_rdlock(&spottingsMapLock);
    out<<spottingsToWords.size()<<"\n";
    for (auto p : spottingsToWords)
    {
        out<<p.first<<"\n"<<p.second.size()<<"\n";
        for (Word* w : p.second)
            out<<w->getSpottingIndex()<<"\n";
    }
    pthread_rwlock_unlock(&spottingsMapLock);

    //spotter->save(out); use normal load
    manQueue.save(out);
    /*out<<_gt.size()<<"\n";
    for (string g : _gt)
        out<<g<<"\n";
    out<<_words.size()<<"\n";
    for (auto w : _words)
        out<<w->getSpottingIndex()<<"\n";
    out<<_wordImgs.size()<<"\n";
    for (const Mat& m : _wordImages)*/
    //just call recreateDatasetVectors
    //out.close();
}

Corpus::Corpus(ifstream& in)
{
    //string loadName = loadPrefix+"_Corpus.sav";
    //ifstream in(loadName);

    pthread_rwlock_init(&pagesLock,NULL);
    pthread_rwlock_init(&spottingsMapLock,NULL);

    string line;
    getline(in,line);
    assert(line.compare("CORPUS")==0);
    getline(in,line);
    averageCharWidth = stof(line);
    getline(in,line);
    countCharWidth = stoi(line);
    getline(in,line);
    threshScoring = stof(line);
    getline(in,line);
    int pagesSize = stoi(line);
    for (int i=0; i<pagesSize; i++)
    {
        getline(in,line);
        int pageId = stoi(line);
        Page* page = new Page(in,&spotter,&averageCharWidth,&countCharWidth);
        pages[pageId]=page;
    }
    getline(in,line);
    pagesSize = stoi(line);
    for (int i=0; i<pagesSize; i++)
    {
        string s;
        getline(in,s);
        getline(in,line);
        int pageId = stoi(line);
        pageIdMap[s]=pageId;
    }
    getline(in,line);
    numWordsReadIn=stoi(line);
    recreateDatasetVectors(false);

    getline(in,line);
    int spottingsToWordsSize=stoi(line);
    for (int i=0; i<spottingsToWordsSize; i++)
    {
        getline(in,line);
        unsigned long sid=stoul(line);
        getline(in,line);
        int wordLen=stoi(line);
        for (int j=0; j<wordLen; j++)
        {
            getline(in,line);
            int spottingIndex=stoi(line);
            spottingsToWords[sid].push_back(_words[spottingIndex]);
        }
    }
    CorpusRef* cr = getCorpusRef();
    manQueue.load(in,cr);
    delete cr;
    //in.close();
}
