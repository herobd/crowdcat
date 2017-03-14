#include "batches.h"

vector< cv::Vec3f > TranscribeBatch::colors = {cv::Vec3f(1.15,1.15,0.85),cv::Vec3f(1.15,0.85,1.15),cv::Vec3f(0.86,0.96,1.2),cv::Vec3f(0.87,1.2,0.87),cv::Vec3f(1.2,0.87,0.87),cv::Vec3f(0.87,0.87,1.2)};
cv::Vec3f TranscribeBatch::wordHighlight(0.9,1.2,1.2);
atomic_ulong TranscribeBatch::_id(0);//I'm assuming 0 is the default value
atomic_ulong SpottingsBatch::_batchId;
atomic_ulong NewExemplarsBatch::_batchId;
atomic_ulong Spotting::_id;

void TranscribeBatch::highlightPix(cv::Vec3b &p, cv::Vec3f color)
{

    p[0] = min(255.f,p[0]*color[0]);
    p[1] = min(255.f,p[1]*color[1]);
    p[2] = min(255.f,p[2]*color[2]);
}

#if TRANS_DONT_WAIT     
TranscribeBatch::TranscribeBatch(WordBackPointer* origin, multimap<float,string> scored, const cv::Mat* origImg, const multimap<int,Spotting>* spottings, int tlx, int tly, int brx, int bry, string gt, unsigned long id, bool lowPriority) : manual(false), lowPriority(lowPriority)
#else
TranscribeBatch::TranscribeBatch(WordBackPointer* origin, multimap<float,string> scored, const cv::Mat* origImg, const multimap<int,Spotting>* spottings, int tlx, int tly, int brx, int bry, string gt, unsigned long id) : manual(false)
#endif
{
    for (auto p : scored)
    {
        possibilities.push_back(p.second);
    }
    init(origin, origImg, spottings, tlx, tly, brx, bry, gt, id);
}

bool icompare(const string& a, const string& b)
{
    int len = min(a.length(),b.length());
    for (unsigned int i = 0; i < len; ++i)
        if (tolower(a[i]) < tolower(b[i]))
            return true;
        else if (tolower(a[i]) > tolower(b[i]))
            return false;
    //They are the same up to this point
    return a.length() < b.length();
}
TranscribeBatch::TranscribeBatch(WordBackPointer* origin, vector<string> prunedDictionary, const cv::Mat* origImg, const multimap<int,Spotting>* spottings, int tlx, int tly, int brx, int bry, string gt, unsigned long id) : manual(true)
{
#if TRANS_DONT_WAIT     
    lowPriority=false;
#endif
    possibilities=prunedDictionary;
    sort(possibilities.begin(), possibilities.end(), icompare);
    init(origin, origImg, spottings, tlx, tly, brx, bry, gt, id);
}
void TranscribeBatch::init(WordBackPointer* origin, const cv::Mat* origImg, const multimap<int,Spotting>* spottings, int tlx, int tly, int brx, int bry, string gt, unsigned long id)
{
    this->origin=origin;
    if (id!=0)
        this->id=id;
    else
        this->id = --_id;
    this->origImg=origImg;
    this->spottings=spottings;
    this->tlx=tlx;
    this->tly=tly;
    this->brx=brx;
    this->bry=bry;
    this->gt=gt;


    int wordH = bry-tly+1;
    int wordW = brx-tlx+1;

    wordImg = cv::Mat::zeros(wordH,wordW,origImg->type());
    (*origImg)(cv::Rect(tlx,tly,wordW,wordH)).copyTo(wordImg(cv::Rect(0, 0, wordW, wordH)));
    if (wordImg.type()!=CV_8UC3)
        cv::cvtColor(wordImg,wordImg,CV_GRAY2RGB);

    //textImg = cv::Mat::zeros(50,wordW,CV_8UC3);

    int colorIndex=0;
    for (auto iter : *spottings)
    {
        const Spotting& s = iter.second;
        cv::Point org((int)((min(wordW,(s.brx-tlx))-max(0,(s.tlx-tlx)))*0.2 + max(0,(s.tlx-tlx))) , 33);
        //cv::putText(textImg, s.ngram, org, cv::FONT_HERSHEY_DUPLEX, 1.0, cv::Scalar(colors[colorIndex][0]*255,colors[colorIndex][1]*255,colors[colorIndex][2]*255),1);
        spottingPoints.push_back(SpottingPoint(s.id,
                    org.x,
                    s.ngram,
                    colors[colorIndex][0]*255,
                    colors[colorIndex][1]*255,
                    colors[colorIndex][2]*255,
                    s.pageId,s.tlx,s.tly,s.brx,s.bry));    
        //cout <<"drawing spotting: "<<s.tlx<<", "<<s.tly<<", "<<s.brx<<", "<<s.bry<<endl;
        for (int r= max(0,s.tly-tly); r<min(wordH,(s.bry-tly)); r++)
            for (int c= max(0,s.tlx-tlx); c<min(wordW,(s.brx-tlx)); c++)
            {
                highlightPix(wordImg.at<cv::Vec3b>(r,c),colors[colorIndex]);
                
            }
        colorIndex = (colorIndex+1)%colors.size();
    }
    for (int r= 0; r<5; r++)
        for (int c= 0; c<wordW; c++)
        {
            highlightPix(wordImg.at<cv::Vec3b>(r,c),wordHighlight);
            
        }
    for (int r= wordH-6; r<wordH; r++)
        for (int c= 0; c<wordW; c++)
        {
            highlightPix(wordImg.at<cv::Vec3b>(r,c),wordHighlight);
        }
    for (int r= 5; r<wordH-6; r++)
        for (int c= 0; c<5; c++)
        {
            highlightPix(wordImg.at<cv::Vec3b>(r,c),wordHighlight);
        }
    for (int r= 5; r<wordH-6; r++)
        for (int c= wordW-6; c<wordW; c++)
        {
            highlightPix(wordImg.at<cv::Vec3b>(r,c),wordHighlight);
        }

}

void TranscribeBatch::setWidth(unsigned int width, int contextPad) 
{

    int wordH = wordImg.rows;
    int wordW = wordImg.cols;
    int topPad = min(contextPad, tly);
    int bottomPad = min(contextPad, origImg->rows-(bry+1));
    int wordHPad = wordH+topPad+bottomPad;
    //int textH= textImg.rows;
    //newTextImg = cv::Mat::zeros(textH,width,CV_8UC3);
    int padLeft = max((((int)width)-wordW)/2,0);
    for (SpottingPoint& sp : spottingPoints)
        sp.setPad(padLeft);
    scale=1.0;
    if (width>=wordW)
    {
        newWordImg = cv::Mat::zeros(wordHPad,width,origImg->type());
        if (width>wordW) {
            int cropX = (tlx-padLeft>=0)?tlx-padLeft:0;
            int pasteX = (tlx-padLeft>=0)?0:padLeft-tlx;
            int cropWidth = (tlx-padLeft>=0)?width:width+(tlx-padLeft);
            if (cropWidth+cropX>=(*origImg).cols) {
                cropWidth=(*origImg).cols-cropX;
            }
            (*origImg)(cv::Rect(cropX,tly-topPad,cropWidth,wordHPad)).copyTo(newWordImg(cv::Rect(pasteX, 0, cropWidth, wordHPad)));

        }
        if (newWordImg.type() != CV_8UC3)
            cv::cvtColor(newWordImg,newWordImg,CV_GRAY2RGB);
        wordImg(cv::Rect(0,0,wordW,wordH)).copyTo(newWordImg(cv::Rect(padLeft, topPad, wordW, wordH)));
        //textImg(cv::Rect(0,0,wordW,textH)).copyTo(newTextImg(cv::Rect(padLeft, 0, wordW, textH)));
    }
    else
    {
        
        newWordImg = cv::Mat::zeros(wordHPad,wordW,origImg->type());
        (*origImg)(cv::Rect(tlx,tly-topPad,wordW,wordHPad)).copyTo(newWordImg(cv::Rect(0, 0, wordW, wordHPad)));
        if (newWordImg.type() != CV_8UC3)
            cv::cvtColor(newWordImg,newWordImg,CV_GRAY2RGB);
        wordImg(cv::Rect(0,0,wordW,wordH)).copyTo(newWordImg(cv::Rect(0, topPad, wordW, wordH)));
        scale = width/(0.0+wordW);//we save the scale to allow a proper display of ngram locations
        cv::resize(newWordImg, newWordImg, cv::Size(), scale,scale, cv::INTER_CUBIC );
        //cv::resize(textImg(cv::Rect(0,0,wordW,textH)), newTextImg, cv::Size(), scale,1, cv::INTER_CUBIC );
    }

#ifdef TEST_MODE_LONG
    //cout <<"transcription word"<<endl;
    //cv::imshow("trans",newWordImg);
    //cv::waitKey(1);
#endif
}

void TranscribeBatch::save(ofstream& out)
{
    out<<"TRANSCRIBEBATCH"<<endl;
    out<<lowPriority<<"\n";
    out<<origin->getSpottingIndex()<<"\n";
    out<<possibilities.size()<<"\n";
    for (string p : possibilities)
    {
        out<<p<<"\n";
    }
    GlobalK::saveImage(wordImg,out);
    //Global::writeImage(newWordImg,out);

    //out<<imgWidth<<"\n";
    out<<id<<"\n";
    out<<tlx<<"\n"<<tly<<"\n"<<brx<<"\n"<<bry<<"\n";
    
    out<<spottingPoints.size()<<"\n";
    for (SpottingPoint& s : spottingPoints)
    {
        s.save(out);
    }
    //out<<scale<<"\n";
    out<<gt<<"\n";
    out<<manual<<"\n";
    out<<_id.load()<<"\n";
}

TranscribeBatch::TranscribeBatch(ifstream& in, CorpusRef* corpusRef)
{
    string line;
    getline(in,line);
    assert(line.compare("TRANSCRIBEBATCH")==0);
#if TRANS_DONT_WAIT     
    getline(in,line);
    lowPriority=stoi(line);
#endif
    getline(in,line);
    int wid = stoi(line);
    origin = corpusRef->getBackPointer(wid);
    origImg = corpusRef->getWordImg(wid);
    spottings = corpusRef->getSpottingsPointer(wid);
    getline(in,line);
    int size = stoi(line);
    possibilities.resize(size);
    for (int i=0; i<size; i++)
    {
        getline(in,possibilities[i]);
    }
    GlobalK::loadImage(wordImg,in);
    //Global::readImage(newWordImg,in);

    getline(in,line);
    id = stoul(line);
    getline(in,line);
    tlx = stoi(line);
    getline(in,line);
    tly = stoi(line);
    getline(in,line);
    brx = stoi(line);
    getline(in,line);
    bry = stoi(line);

    getline(in,line);
    size = stoi(line);
    spottingPoints.resize(size);
    for (int i=0; i<size; i++)
    {
        spottingPoints[i] = SpottingPoint(in);
        assert(corpusRef->verify(spottingPoints[i].page,spottingPoints[i].x1,spottingPoints[i].y1,spottingPoints[i].x2,spottingPoints[i].y2));
    }
    getline(in,gt);
    getline(in,line);
    manual=stoi(line);
    getline(in,line);
    _id.store( stoul(line));
}
