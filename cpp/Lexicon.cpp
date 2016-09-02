#include "Lexicon.h"
//#include <ctime>

Lexicon* Lexicon::self=NULL;

vector<string> Lexicon::search(string query, Meta meta, int max)
{
    
    
    vector<string> ret1;//, ret2;
    
    //clock_t start = clock();
    regex q(query);
    for (const string& word : words)
    {
    
        if (regex_match (word, q ))
        {
            ret1.push_back(word);
            if (max>0 && ret1.size()>max)
                break;
        }
    }
    /*clock_t time1 = clock()-start;
    
    start = clock();
    string s = words_lineSeperated;
    //regex e("^"+query+"$");
    regex e("(?:\\n)("+query+")(?:\\n)");
    smatch m;
    while (std::regex_search (s,m,e)) {
        //for (auto x:m) 
        //    ret2.push_back(x.str());
        ret2.push_back(m[1].str());
        
        s = m.suffix().str();
    }
    clock_t time2 = clock()-start;
    
    cout <<"search times, vector: "<<time1<<"  string: "<<time2<<"  for query "<<query<<endl;
    assert(ret1.size() == ret2.size());
    for (int i=0; i<ret1.size(); i++)
    {
        assert(ret1[i].compare(ret2[i])==0);
    }*/
#ifdef TEST_MODE
    cout << query <<": ";
    if (max>0 && ret1.size()>max)
        cout<<" MAXED";
    else
        for (string s : ret1)
            cout<<s<<", ";
    cout<<endl;
#endif
    return ret1;
}

bool Lexicon::readIn(string fileName)
{
    
    ifstream in(fileName);
    string word;
    regex notWordChar ("[^\\w]");
    while(getline(in,word))
    {
        word = regex_replace (word,notWordChar,"");
        
        words_lineSeperated+=word+"\n";
        words.push_back(word);
    }
    in.close();
    return true;
}
