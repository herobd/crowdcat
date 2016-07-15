#ifndef CATTSS_H
#define CATTSS_H

#include "MasterQueue.h"
#include "Spotter.h"
#include "Knowledge.h"

class CATTSS
{
    private:
    MasterQueue* masterQueue;
    Spotter* spotter;
    Knowledge::Corpus* corpus;

    public:
    CATTS(string lexiconFile, string pageImageDir, string segmentationFile);

#endif
