#include "TrainingInstances.h"

#include "TrainingBatchWraperSpottings.h"
#include "TrainingBatchWraperTranscription.h"

TrainingInstances::TrainingInstances()
{
    line= cv::imread("data/trainingImages/line.png", CV_LOAD_IMAGE_GRAYSCALE);
    letsgo= cv::imread("data/trainingImages/letsgo.png", CV_LOAD_IMAGE_GRAYSCALE);
    spotting_0 = Spotting(398,7,435,57,  0,&line,"he",0);
    spotting_1 = Spotting(192,7,225,55,  0,&line,"he",0);
    spotting_2 = Spotting(547,9,582,53,  0,&line,"he",0);
    spotting_3 = Spotting(120,4,161,52,  0,&line,"he",0);
    spotting_4 = Spotting(587,15,632,63,  0,&line,"or",0);
    spotting_5 = Spotting(207,5,235,54,  0,&line,"or",0);
    spotting_6 = Spotting(628,21,654,62,  0,&line,"in",0);
    spotting_7 = Spotting(301,11,325,67,  0,&line,"in",0);
    word = new Knowledge::Word(0,0,0,0,&line,NULL,NULL,NULL,0);
    spotting_8a = Spotting(507,9,543,52,  0,&line,"th",0);
    spotting_9b = Spotting(429,5,459,55,  0,&line,"ir",0);
    spotting_10b = Spotting(150,4,194,56,  0,&line,"ea",0);
    spotting_11a = Spotting(275,12,315,66,  0,&line,"oi",0);
    spotting_12a = Spotting(572,12,615,66,  0,&line,"mo",0);
    spotting_12b = Spotting(655,14,689,66,  0,&line,"ng",0);
    spotting_GO = Spotting(11,6,211,63,  0,&letsgo,"[start]",0);
}


BatchWraper* TrainingInstances::getBatch(int width, int color, string prevNgram, int trainingNum)
{
#ifndef TEST_MODE
    try
    {
#endif
        BatchWraper* ret= makeInstance(trainingNum,width,color);
        if (ret!=NULL)
            return ret;
        else
            return new BatchWraperBlank();
#ifndef TEST_MODE
    }
    catch (exception& e)
    {
        cout <<"Exception in TrainingInstances::getBatch(), "<<e.what()<<endl;
    }
    catch (...)
    {
        cout <<"Exception in TrainingInstances::getBatch(), UNKNOWN"<<endl;
    }
#endif
    return new BatchWraperBlank();
}

BatchWraper* TrainingInstances::makeInstance(int trainingNum, int width,int color)
{
    if (trainingNum==0){//spotting right he*/
            
        SpottingsBatch* batch = new SpottingsBatch("he",0);
        SpottingImage spottingImage(spotting_0,width,color);
        batch->push_back(spottingImage);            
        string correct="1";
        string instructions =
                "<p>The system begins by spotting subwords. You will help approve them.</p>"
                "<p>If the highlighted region of the bottom image matches the text below it, swipe right.</p>"
                "<p><i>(tap the screen to continue)</i></p>";
        return (BatchWraper*) (new TrainingBatchWraperSpottings(batch,correct,instructions,false));
    } else if (trainingNum==1) {//spotting wrong (not he)
        SpottingsBatch* batch = new SpottingsBatch("he",0);
        SpottingImage spottingImage(spotting_1,width,color);
        batch->push_back(spottingImage);            
        string correct="0";
        string instructions =
                "<p>If the highlighted region does not match the text, swipe left.</p>";
        return (BatchWraper*) (new TrainingBatchWraperSpottings(batch,correct,instructions,false));
    } else if (trainingNum==2) {//spotting bad BB (wrong) he
        SpottingsBatch* batch = new SpottingsBatch("he",0);
        SpottingImage spottingImage(spotting_2,width,color);
        batch->push_back(spottingImage);            
        string correct="0";
        string instructions =
                "<p>The system uses the location of the subword to estimate how many letters are left in the word around it.</p>"
                "<p>If the highlighted region is significantly off the desired letters, it's wrong, so swipe left.</p>";
        return (BatchWraper*) (new TrainingBatchWraperSpottings(batch,correct,instructions,false));
    } else if (trainingNum==3) {//close but good BB he
        SpottingsBatch* batch = new SpottingsBatch("he",0);
        SpottingImage spottingImage(spotting_3,width,color);
        batch->push_back(spottingImage);            
        string correct="1";
        string instructions =
                "<p>If the highlighted region isn't perfectly aligned, but reasonibly close, the system can handle it, so swipe right.</p>";
        return (BatchWraper*) (new TrainingBatchWraperSpottings(batch,correct,instructions,false));
    } else if (trainingNum==4) {//p spotting right or
        SpottingsBatch* batch = new SpottingsBatch("or",0);
        SpottingImage spottingImage(spotting_4,width,color);
        batch->push_back(spottingImage);            
        string correct="1";
        string instructions =
                "<p>Note that you will see a tab representing a subword target change coming down before any of its respective images. Additionally, when the target text changes, you will notice a color change.</p>"
                "<p>Do these next few subwords.</p>";
        return (BatchWraper*) (new TrainingBatchWraperSpottings(batch,correct,instructions,false));
    } else if (trainingNum==5) {//p spotting right or
        SpottingsBatch* batch = new SpottingsBatch("or",0);
        SpottingImage spottingImage(spotting_5,width,color);
        batch->push_back(spottingImage);            
        string correct="1";
        string instructions ="";
        return (BatchWraper*) (new TrainingBatchWraperSpottings(batch,correct,instructions,false));
    } else if (trainingNum==6) {//p spotting wrong in
        SpottingsBatch* batch = new SpottingsBatch("in",0);
        SpottingImage spottingImage(spotting_6,width,color);
        batch->push_back(spottingImage);            
        string correct="0";
        string instructions ="";
        return (BatchWraper*) (new TrainingBatchWraperSpottings(batch,correct,instructions,false));
    } else if (trainingNum==7) {//p spotting right in
        SpottingsBatch* batch = new SpottingsBatch("in",0);
        SpottingImage spottingImage(spotting_7,width,color);
        batch->push_back(spottingImage);            
        string correct="1";
        string instructions ="";
        return (BatchWraper*) (new TrainingBatchWraperSpottings(batch,correct,instructions,false));
    } else if (trainingNum==8) {//transcribe (tap) the
        multimap<float,string> scored;
        scored.insert( make_pair(-1,"the") );
        scored.insert( make_pair(0,"tho") );
        scored.insert( make_pair(1,"thy") );
        scored.insert( make_pair(2,"thee") );
        multimap<int,Spotting> spottings;
        spottings.insert( make_pair(507,spotting_8a) );
        TranscribeBatch* batch = new TranscribeBatch(word,scored,&line,&spottings,511,12,563,54,"the",0);
        batch->setWidth(width);
        string correct="the";
        string instructions =
                "<p>When the system has gotten enough subwords for a given word, it will show you this.</p>"
                "<p>You just need to tap on the correct transcription.</p>";
        return (BatchWraper*) (new TrainingBatchWraperTranscription(batch,correct,instructions,false));
    } else if (trainingNum==9) {//transcribe (spotting error) there (err, 'ir')
        multimap<float,string> scored;
        scored.insert( make_pair(-1,"their") );
        scored.insert( make_pair(0,"theirs") );
        scored.insert( make_pair(1,"heir") );
        multimap<int,Spotting> spottings;
        spottings.insert( make_pair(398,spotting_0) );
        spottings.insert( make_pair(429,spotting_9b) );//id:10
        TranscribeBatch* batch = new TranscribeBatch(word,scored,&line,&spottings,383,5,465,59, "there",0);
        batch->setWidth(width);
        string correct="$REMOVE:"+to_string(spotting_9b.id)+"$";
        string instructions =
                "<p>Sometimes a subword will have been classified incorrectly, meaning the correct transcription won't be shown.</p>"
                "<p>Tap the <b>X</b> below the '<b>ir</b>' to indicate this is an incorrect spotting.</p>";
        return (BatchWraper*) (new TrainingBatchWraperTranscription(batch,correct,instructions,false));
    } else if (trainingNum==10) {//transcribe (all error) Theadore
        multimap<float,string> scored;
        scored.insert( make_pair(-1,"theater") );
        scored.insert( make_pair(0,"heater") );
        scored.insert( make_pair(1,"wheat") );
        multimap<int,Spotting> spottings;
        spottings.insert( make_pair(120,spotting_3) );
        spottings.insert( make_pair(150,spotting_10b) );
        TranscribeBatch* batch = new TranscribeBatch(word,scored,&line,&spottings,85,2,252,56, "Theadore",0);
        batch->setWidth(width);
        string correct="$ERROR$";
        string instructions =
                "<p>If the subwords are all correct, but the correct transcription is not available, just tap the <b>[None / Error]</b> button at the bottom of the transcription list.</p>";
        return (BatchWraper*) (new TrainingBatchWraperTranscription(batch,correct,instructions,false));
    } else if (trainingNum==11) {//p transcribe (tap) join
        multimap<float,string> scored;
        scored.insert( make_pair(-1,"coined") );
        scored.insert( make_pair(-0.5,"joiner") );
        scored.insert( make_pair(0,"joined") );
        scored.insert( make_pair(1,"doings") );
        scored.insert( make_pair(2,"goings") );
        multimap<int,Spotting> spottings;
        spottings.insert( make_pair(275,spotting_11a) );//oi
        spottings.insert( make_pair(301,spotting_7) );//in
        TranscribeBatch* batch = new TranscribeBatch(word,scored,&line,&spottings,261,13,368,65, "joined",0);
        batch->setWidth(width);
        string correct="joined";
        string instructions =
            "<p>Here's a couple more to practice on.</p>";
        return (BatchWraper*) (new TrainingBatchWraperTranscription(batch,correct,instructions,false));
    } else if (trainingNum==12) {//p transcribe (tap) morning (scroll down)
        multimap<float,string> scored;
        scored.insert( make_pair(-1,"moaning") );
        scored.insert( make_pair(0,"mooring") );
        scored.insert( make_pair(1,"mousing") );
        //scored.insert( make_pair(2,"mobbing") );
        //scored.insert( make_pair(3,"moating") );
        scored.insert( make_pair(4,"mocking") );
        scored.insert( make_pair(5,"mopping") );
        scored.insert( make_pair(6,"morning") );
        multimap<int,Spotting> spottings;
        spottings.insert( make_pair(572,spotting_12a) );//mo
        spottings.insert( make_pair(655,spotting_12b) );//ng
        TranscribeBatch* batch = new TranscribeBatch(word,scored,&line,&spottings,570,19,691,65, "morning",0);
        batch->setWidth(width);
        string correct="morning";
        string instructions ="";
        return (BatchWraper*) (new TrainingBatchWraperTranscription(batch,correct,instructions,false));
    } else if (trainingNum==13) {//manual (no help)
        vector<string> prunedDictionary;
        multimap<int,Spotting> spottings;
        spottings.insert( make_pair(120,spotting_3) );//he
        spottings.insert( make_pair(150,spotting_10b) );//ea
        TranscribeBatch* batch = new TranscribeBatch(word,prunedDictionary,&line,&spottings,85,2,252,56, "Theadore",0);
        batch->setWidth(width);
        string correct="Theadore";
        string instructions =
            "<p>When the system recognizes it won't be able to do any more for a word, you will have to manually type in the transcription.</p>"
            "<p><i>*sigh*</i></p>";
        return (BatchWraper*) (new TrainingBatchWraperTranscription(batch,correct,instructions,false));
    } else if (trainingNum==14) {//manual (auto compelete)
        vector<string> prunedDictionary = {
                "theft",
                "their",
                "theme",
                "thens",
                "there",
                "therm",
                "these",
                "theta",
                "thews",
                "thewy",
                "theres",
                "therms",
                "theron",
                "thereof",
                "thereon"
        };
        multimap<int,Spotting> spottings;
        spottings.insert( make_pair(398,spotting_0) );
        TranscribeBatch* batch = new TranscribeBatch(word,prunedDictionary,&line,&spottings,383,5,465,59, "there",0);
        batch->setWidth(width);
        string correct="there";
        string instructions =
            "<p>It will try and help you by providing an intelligent autocomplete.<p>"
            "<p>Complete these next couple transcriptions and your training will be complete!</p>";
        return (BatchWraper*) (new TrainingBatchWraperTranscription(batch,correct,instructions,false));
    } else if (trainingNum==15) {//?p manual (auto complete, but no help) 
        vector<string> prunedDictionary;
        multimap<int,Spotting> spottings;
        TranscribeBatch* batch = new TranscribeBatch(word,prunedDictionary,&line,&spottings,7,8,73,59, "he",0);
        batch->setWidth(width);
        string correct="he";
        string instructions ="";
        return (BatchWraper*) (new TrainingBatchWraperTranscription(batch,correct,instructions,false));
    } else if (trainingNum==16) {//p spotting right or
        /*SpottingsBatch* batch = new SpottingsBatch("[start]",0);
        SpottingImage spottingImage(spotting_GO,width,color);
        batch->push_back(spottingImage);            
        string correct="1";
        string instructions ="";
        return (BatchWraper*) (new TrainingBatchWraperSpottings(batch,correct,instructions,true));*/
        multimap<float,string> scored;
        scored.insert( make_pair(-1,"[start]") );
        multimap<int,Spotting> spottings;
        TranscribeBatch* batch = new TranscribeBatch(word,scored,&letsgo,&spottings,11,6,211,63, "[start]",0);
        batch->setWidth(width);
        string correct="[start]";
        string instructions ="";
        return (BatchWraper*) (new TrainingBatchWraperTranscription(batch,correct,instructions,true));
    }
    return NULL;
}
