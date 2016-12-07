
//#include "MasterQueue.h"
#include "Knowledge.h"
//#include "SpottingQueue.h"
//#include "spotting.h"
//#include "Lexicon.h"
//#include "BatchWraper.h"
#include "opencv2/core/core.hpp"

#include "AlmazanSpotter.h"

int main(int argc, char** argv)
{
    EmbAttSpotter* spotter;
    string pageImageDir;
    if (argc>3)
    {
        pageImageDir = argv[1];
        string trainingFile= argv[2];
        string modelPrefix= argv[3];
        int contextPad= 0;

        spotter = new EmbAttSpotter(modelPrefix);

        Knowledge::Corpus* tr_corpus = new Knowledge::Corpus(contextPad);
        tr_corpus->addWordSegmentaionAndGT(pageImageDir, trainingFile);
        Dataset* tr_dataset = new AlmazanDataset(tr_corpus);
        spotter->setTraining_dataset(tr_dataset);
        
        string segmentationFile= argv[4];
        Knowledge::Corpus* corpus = new Knowledge::Corpus(contextPad);
        corpus->addWordSegmentaionAndGT(pageImageDir, segmentationFile);
        Dataset* dataset = new AlmazanDataset(corpus);
        spotter->setCorpus_dataset(dataset,true);

    }
    else
        cout<<"usage: "<<argv[0]<<" imageDir trainFile modelPrefix testFile"<<endl;

    return 0;
}
