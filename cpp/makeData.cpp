
//#include "MasterQueue.h"
#include "Knowledge.h"
//#include "SpottingQueue.h"
//#include "spotting.h"
//#include "Lexicon.h"
//#include "BatchWraper.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iomanip>
#include <fstream>
#include <ostream>
#include <sstream>

//#include "AlmazanSpotter.h"

using namespace std;

int main(int argc, char** argv)
{
    EmbAttSpotter* spotter;
    string pageImageDir;
    if (argc>2)
    {
        pageImageDir = argv[1];
        string gtpFile= argv[2];
        string outTrainFile= argv[3];
        string outTestFile= argv[4];
        string outAllDir= argv[5];
        if (outAllDir[outAllDir.length()-1]!='/')
            outAllDir+='/';
        string outAllFile= argv[6];


        Knowledge::Corpus* corpus = new Knowledge::Corpus();
        corpus->addWordSegmentaionAndGT(pageImageDir, gtpFile);
        Dataset* dataset = new AlmazanDataset(corpus);

        ofstream outAll(outAllFile);
        ofstream outTest(outTestFile);
        ofstream outTrain(outTrainFile);
        outAll<<"[";
        outTrain<<"[";
        outTest<<"[";

        for (int i=0; i<dataset->size(); i++)
        {
            if (i!=0)
            {
                outAll<<", ";
            }
            ostringstream stringStream;
            //stringStream <<outAllDir;
            stringStream <<setfill('0') << setw(4)<< i <<".png";
            
            cv::imwrite(outAllDir+stringStream.str(),dataset->image(i));
            
            outAll<<"{\"dataset_id\":\"GW\", \"image_path\":\""<<stringStream.str()<<"\", \"gt\":\""<<dataset->labels()[i]<<"\", \"w\":"<<dataset->image(i).cols<<", \"h\":"<<dataset->image(i).rows<<"}";
            if (i%5==0)
            {
                if (i!=0)
                {
                    outTest<<", ";
                }
                outTest<<"{\"dataset_id\":\"GW\", \"image_path\":\""<<stringStream.str()<<"\", \"gt\":\""<<dataset->labels()[i]<<"\", \"w\":"<<dataset->image(i).cols<<", \"h\":"<<dataset->image(i).rows<<"}";
            }
            else
            {
                if (i!=1)
                {
                    outTrain<<", ";
                }
                outTrain<<"{\"dataset_id\":\"GW\", \"image_path\":\""<<stringStream.str()<<"\", \"gt\":\""<<dataset->labels()[i]<<"\", \"w\":"<<dataset->image(i).cols<<", \"h\":"<<dataset->image(i).rows<<"}";
            }
        }
        outAll<<"]";
        outTrain<<"]";
        outTest<<"]";

    }
    else
        cout<<"usage: "<<argv[0]<<" imageDir gtpFile trainFile testFile outDir allFile"<<endl;

    return 0;
}
