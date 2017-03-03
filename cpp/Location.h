#ifndef LOCATION_H
#define LOCATION_H
struct Location
{
    int pageId, x1,y1,x2,y2;
    Location() {}
    Location(int pageId, int x1, int y1, int x2, int y2) : pageId(pageId), 
                                                            x1(x1), 
                                                            y1(y1), 
                                                            x2(x2), 
                                                            y2(y2) {}
};
#endif
