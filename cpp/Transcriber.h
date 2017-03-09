#ifndef TRANSCRIBER_H
#define TRANSCRIBER_H

class Transcriber
{
    public:
    vector< multimap<float,string> > transcribe(Dataset* words);
    multimap<float,string> transcribe(const Mat& image);

};

#endif
