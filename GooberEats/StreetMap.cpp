#include "provided.h"
#include <string>
#include <vector>
#include <functional>
#include "ExpandableHashMap.h"
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<std::string>()(g.latitudeText + g.longitudeText);
}

unsigned int hasher(const std::string& s) {
    return std::hash<std::string>()(s);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment>> smap;
};
StreetMapImpl::StreetMapImpl() {}
StreetMapImpl::~StreetMapImpl() {}
bool StreetMapImpl::load(string mapFile)
{
    ifstream infile(mapFile);
    string seg;
    if (!infile)
        return false;
    while (getline(infile, seg)) 
    {
        /*
        for (int i = 0; i < seg.length(); i++)
        {
            if (isalpha(seg[i]))      
        }
        */
        string streetSeg = seg;
        int streetIndex;
        infile >> streetIndex;
        infile.ignore(100000000, '\n');
        for (int i = 0; i < streetIndex; i++) 
        {
            //infile.ignore(100000000, '\n');
            string currCoord;
            getline(infile, currCoord);
            vector<StreetSegment> regVect, reverseVect;
            istringstream iss(currCoord);
            string start_x, start_y, end_x, end_y;
            vector<StreetSegment>* findVector;
            if (!(iss >> start_x >> start_y >> end_x >> end_y))
                continue;
            GeoCoord startCoord(start_x, start_y),endCoord(end_x, end_y);
            StreetSegment segment(startCoord, endCoord, streetSeg),reverseSegment(endCoord, startCoord, streetSeg);
            findVector = smap.find(startCoord);
            if (findVector == nullptr) 
            {
                regVect.push_back(segment);
                smap.associate(startCoord, regVect);
            }
            else 
            {
                findVector->push_back(segment);
            }
            findVector = smap.find(endCoord);
            if (findVector == nullptr) {
                reverseVect.push_back(reverseSegment);
                smap.associate(endCoord, reverseVect);
            }
            else 
            {
                findVector->push_back(reverseSegment);
            }
        }
    }
    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    const vector<StreetSegment>* segment = smap.find(gc);
    if (segment == nullptr)
    {
        return false;
    }
    else
    {
        segs.clear();
        segs = *segment;
        return true;
    }
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    return m_impl->getSegmentsThatStartWith(gc, segs);
}