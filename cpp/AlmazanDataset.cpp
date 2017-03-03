#include "AlmazanDataset.h"

AlmazanDataset::AlmazanDataset(const Dataset* other)
{
    for (int i=0; i<other->size(); i++)
    {
        cv::Mat patch=other->image(i);
        assert(patch.rows*patch.cols>1);
        if (patch.channels()>1)
            cv::cvtColor(patch,patch,CV_RGB2GRAY);
        
        //patch = preprocess(patch,i);        
        wordImages.push_back(patch);
        _labels.push_back(other->labels()[i]);
        ids.push_back(other->wordId(i));
    }

}

cv::Mat AlmazanDataset::preprocess(cv::Mat im, int i)
{
    int maxH=80;
    int minH=80;
    cv::Mat patch;
    im.convertTo(patch,CV_32F);
    patch/=255;
    double m;
    minMaxIdx(patch,NULL,&m);
    if (m<0.2)
        patch*=0.2/m;
    
    if (patch.rows>maxH)
    {
        double ratio = (maxH+0.0)/patch.rows;
        cv::resize(patch,patch,cv::Size(),ratio,ratio,cv::INTER_CUBIC);
        if (i>=0)
            ratios[i]=ratio;
    }
    else if (patch.rows<minH)
    {
        double ratio = (maxH+0.0)/patch.rows;
        cv::resize(patch,patch,cv::Size(),ratio,ratio,cv::INTER_CUBIC);
        if (i>=0)
            ratios[i]=ratio;
    }
    
    
    patch*=255;
    patch.convertTo(patch,CV_8U);
    return patch;
}

int AlmazanDataset::backwards(int i, int x) const
{
    auto iter = ratios.find(i);
    if (iter != ratios.end())
        return x/(iter->second);
    else 
        return x;
}

const vector<string>& AlmazanDataset::labels() const
{
    return _labels;
}
int AlmazanDataset::size() const
{
    return _labels.size();
}
const cv::Mat AlmazanDataset::image(unsigned int i) const
{
    return wordImages.at(i);
}
/*
int xa=min(stoi(sm[2]),stoi(sm[3]));
        int xb=max(stoi(sm[2]),stoi(sm[3]));
        int ya=min(stoi(sm[4]),stoi(sm[5]));
        int yb=max(stoi(sm[4]),stoi(sm[5]));
        
        int x1=max(0,xa);
        int x2=min(curIm.cols-1,xb);
        int y1=max(0,ya);
        int y2=min(curIm.rows-1,yb);
*/
