
#include "BatchWraperTranscription.h"
BatchWraperTranscription::BatchWraperTranscription(Word* word, int width, int contextPad, bool allowManual)
{
    manual = allowManual;

    bool burn;
    int tlx, tly, brx, bry;
    word->getBoundsAndDoneAndGT(&tlx, &tly, &brx, &bry, &burn, &gt);


    int wordH = bry-tly+1;
    int wordW = brx-tlx+1;

    int topPad = min(contextPad, tly);
    const cv::Mat* origImg = word->getPage();
    int bottomPad = min(contextPad, origImg->rows-(bry+1));
    int wordHPad = wordH+topPad+bottomPad;
    //int textH= textImg.rows;
    //newTextImg = cv::Mat::zeros(textH,width,CV_8UC3);
    int padLeft = max((((int)width)-wordW)/2,0);
    scale=1.0;
    cv::Mat newWordImg;
    if (width>=wordW)
    {
        newWordImg = cv::Mat::zeros(wordHPad,width,origImg->type());
        int pasteX=0;
        if (width>wordW) {
            int cropX = (tlx-padLeft>=0)?tlx-padLeft:0;
            pasteX = (tlx-padLeft>=0)?0:padLeft-tlx;
            int cropWidth = (tlx-padLeft>=0)?width:width+(tlx-padLeft);
            if (cropWidth+cropX>=(*origImg).cols) {
                cropWidth=(*origImg).cols-cropX;
            }
            (*origImg)(cv::Rect(cropX,tly-topPad,cropWidth,wordHPad)).copyTo(newWordImg(cv::Rect(pasteX, 0, cropWidth, wordHPad)));

        }
        //highlight
        if (newWordImg.channels()!=3)
            cv::cvtColor(newWordImg,newWordImg,CV_GRAY2BGR);
        //newWordImg(cv::Rect(padLeft,0,wordW,wordH)) -= cv::Vec3b(50,0,0);
        for (int r=0; r<wordH; r++)
            for (int c=0; c<wordW; c++)
                newWordImg.at<cv::Vec3b>(r,c+padLeft+pasteX) -= cv::Vec3b(50,0,0);
    }
    else
    {
        
        newWordImg = cv::Mat::zeros(wordHPad,wordW,origImg->type());
        (*origImg)(cv::Rect(tlx,tly-topPad,wordW,wordHPad)).copyTo(newWordImg(cv::Rect(0, 0, wordW, wordHPad)));
        scale = width/(0.0+wordW);//we save the scale to allow a proper display of ngram locations
        cv::resize(newWordImg, newWordImg, cv::Size(), scale,scale, cv::INTER_CUBIC );
        //highlight
        if (newWordImg.channels()!=3)
            cv::cvtColor(newWordImg,newWordImg,CV_GRAY2BGR);
        //newWordImg -= cv::Vec3b(50,0,0);
        for (int r=0; r<newWordImg.rows; r++)
            for (int c=0; c<newWordImg.cols; c++)
                newWordImg.at<cv::Vec3b>(r,c) -= cv::Vec3b(50,0,0);
    }

    base64::encoder E;
    vector<int> compression_params={CV_IMWRITE_PNG_COMPRESSION,9};
    
    manual = allowManual;
    batchId=to_string(word->getId());
    vector<uchar> outBuf;
    cv::imencode(".png",newWordImg,outBuf,compression_params);
    stringstream ss;
    ss.write((char*)outBuf.data(),outBuf.size());
    stringstream encoded;
    E.encode(ss, encoded);
    string dataBase64 = encoded.str();
    wordImgStr=dataBase64;
    retPoss = word->popTopXPossibilities(NUM_TRANS_POSS);
//#ifdef NO_NAN
//    images.resize(1);
//    images[0]=batch->getImage();
//#endif
}
#ifndef NO_NAN
void BatchWraperTranscription::doCallback(Callback *callback)
{
    Nan:: HandleScope scope;
    v8::Local<v8::Array> possibilities = Nan::New<v8::Array>(retPoss.size());
    for (unsigned int index=0; index<retPoss.size(); index++) {
	Nan::Set(possibilities, index, Nan::New(retPoss[index]).ToLocalChecked());
    }
    string batchType;
    if (manual)
       batchType = "manual";
    else
       batchType = "transcription";
    Local<Value> argv[] = {
	Nan::Null(),
	Nan::New(batchId).ToLocalChecked(),
        Nan::New(batchType).ToLocalChecked(),
	Nan::New(wordImgStr).ToLocalChecked(),
	Nan::New(possibilities),
        Nan::New(gt).ToLocalChecked()
    };
    

    callback->Call(6, argv);
}
#else
void BatchWraperTranscription::getTranscription(int* batchId, vector<string>* poss, bool* manual, string* gt)
{
    *batchId=stoi(this->batchId);
    *poss=retPoss;
    *manual=this->manual;
    *gt=this->gt;
}
#endif
