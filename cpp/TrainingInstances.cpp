#include "TrainingInstaces.h"

TrainingInstaces::TrainingInstaces()
{
    line= cv::imread("data/trainingImages/line.png");
    spotting_0 = Spotting(398,7,435,57,  0,*line,"he",0);

}


BatchWraper* TrainingInstaces::getBatch(int num, int width, int color, int trainingNum)
{
#ifndef TEST_MODE
    try
    {
#endif
        bool hard=true;
        if (num==-1) {
            num=5;
            hard=false;

        }
        BatchWraper* ret= makeInstance(trainingNum,width,color);
        if (ret!=NULL)
            return ret;
        else
            return new BatchWraperBlank();
#ifndef TEST_MODE
    }
    catch (exception& e)
    {
        cout <<"Exception in TrainingInstaces::getBatch(), "<<e.what()<<endl;
    }
    catch (...)
    {
        cout <<"Exception in TrainingInstaces::getBatch(), UNKNOWN"<<endl;
    }
#endif
    return new BatchWraperBlank();
}

BatchWraper* makeInstance(int trainingNum, int width,int color)
{
    switch(trainingNum)
    {
        case 0://spotting right he
            SpottingsBatch batch("he",0);
            SpottingImage spottingImage(spotting_0,width,color);
            batch.push_back(spottingImage);            
            return new TrainingBatchWraperSpotting(&batch,"true",
                    "<p>The system begins by spotting subwords. You will help approve them.</p>"
                    "<p>If the highlighted region bottom image matches the text below it, swipe right.</p>");
            

        case 1://spotting wrong (not he)
        case 2://spotting bad BB (wrong) he
        case 3://close but good BB he
        case 4://p spotting right or
        case 5://p spotting right or
        case 6://p spotting wrong in
        case 7://p spotting right in
        case 8://transcribe (tap) the
        case 9://transcribe (spotting error) there (err, 'ir')
        case 10://transcribe (all error) Theadore
        case 11://p transcribe (tap) morning
        case 12://p transcribe (tap) join
        case 13://manual (no help)
        case 14://manual (auto compelete)
        case 15://?p manual (no help) 
        case 16://?p manual (auto compelete)
    }
    return NULL;
}
