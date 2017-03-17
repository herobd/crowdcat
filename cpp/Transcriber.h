#ifndef TRANSCRIBER_H
#define TRANSCRIBER_H

class Transcriber
{
    public:
    virtual vector< multimap<float,string> > transcribe(Dataset* words)=0;
    virtual multimap<float,string> transcribe(const Mat& image)=0;
    virtual void addLexicon(const vector<string>& lexicon)=0;

};

#endif
