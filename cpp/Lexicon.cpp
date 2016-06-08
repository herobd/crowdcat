#include "Lexicon.h"
#include <ctime>

static Lexicon* self=NULL;

static vector<string> Lexicon::search(string query, Meta meta)
{
    if (self==NULL) {
        self=new Lexicon();
    }
    
    vector<string> ret1, ret2;
    
    clock_t start = clock();
    regex q(query);
    for (string word : self->words)
    {
    
        if (regex_match (word, q ))
            ret1.push_back(word);
    }
    clock_t time1 = clock()-start;
    
    start = clock();
    string s = self->words_lineSeperated;
    regex e("^"+query+"$");
    smatch m;
    while (std::regex_search (s,m,e)) {
        for (auto x:m) 
            ret2.push_back(x.str());
        
        s = m.suffix().str();
    }
    clock_t time2 = clock()-start;
    
    cout <<"search times, vector: "<<time1<<"  string: "<<time2<<"  for query "<<query<<endl;
    assert(ret1.size() == ret2.size());
    for (int i=0; i<ret1.size(); i++)
    {
        assert(ret1[i].compare(ret2[i])==0);
    }
    
    return ret1;
}

static bool readIn(string fileName)
{
    if (self==NULL) {
        self=new Lexicon();
    }
    
    ifstream in(fileName);
    string word;
    regex notWordChar ("[^\\w]");
    while(readLine(in,word))
    {
        word = regex_replace (word,notWordChar,"");
        
        self->words_lineSeperated+=word+"\n";
        self->words.push_back(word);
    }
    in.close();
}
