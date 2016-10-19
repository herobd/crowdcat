#ifndef SPECIAL_INSTANCES_H
#define SPECIAL_INSTANCES_H

#include "BatchWraper.h"

class SpecialInstances
{
    public:
        virtual BatchWraper* getBatch(int width, int color, string prevNgram, int batchNum) =0;

};

#endif 
