#include "provided.h"
#include "support.h"
#include <list>
#include <algorithm>
using namespace std;

struct Node
{
	GeoCoord startCoord;
	GeoCoord endCoord;  // GeoCoord object REMEMBER THIS
	Node* parent; // parent Node will use to deal with "neighbors or childs"
	double g; // g gost
	double h; // h coset
	double f() const { return g + h; }
	std::string streetName;
	std::vector<Node*> m_nodes;// to store sub nodes
	Node* getNode()
	{
		Node* node = new Node;
		this->m_nodes.push_back(node);
		g = 0;
		h = 0;
		return node;
	}
	// MEMORY LEAK BUT UNCOMEMENTING GIVES ERROR WITH SKELETON CODE
	//~Node() {
	//	delete parent; // get rid of this
	//	for (int i = 0; i < m_nodes.size(); i++)
	//	{
	//		delete m_nodes[i];
	//	}
	//}
	void clearAll()
	{
		for (int i = 0; i < m_nodes.size(); i++)
		{
			delete m_nodes[i];
		}
	}
};



class PointToPointRouterImpl
{
public:
	PointToPointRouterImpl(const StreetMap* sm);
	~PointToPointRouterImpl();
	DeliveryResult generatePointToPointRoute(
		const GeoCoord& start,
		const GeoCoord& end,
		list<StreetSegment>& route,
		double& totalDistanceTravelled) const;

private:
	const StreetMap* smap;
};


bool foundTheEnd(GeoCoord coord, std::vector<StreetSegment>& segments, const StreetMap* sm, StreetSegment& endseg)
{
	vector<StreetSegment> segofsegs;
	for (int i = 0; i < segments.size(); i++) {
		sm->getSegmentsThatStartWith(segments[i].end, segofsegs);//generating each succesor`s successors in segofsegs
		for (int j = 0; j < segofsegs.size(); j++) {
			//check if this segment of segment is the goal
			if (segofsegs[j].end == coord) {
				endseg = segofsegs[j];
				return true;
			}
		}
	}
	return false;
}


bool isLowerCost(const std::vector<Node*>& open_list, const std::vector<Node*>& closed_list, Node* q, Node* next)
{

	//"********************Open List for F value************************************"
	for (int j = 0; j < open_list.size(); j++) {

		if (open_list[j]->startCoord == next->startCoord && open_list[j]->f() <= next->f()) //checking if succor has large f than q ,skip this by returning true
		{
			return true;//if returns true then the element is not pushed in open list
		}
	}
	//"********************Closed  List for F value************************************"

	for (int j = 0; j < closed_list.size(); j++) {
		if (closed_list[j]->startCoord == next->startCoord && closed_list[j]->f() <= next->f()) //checking if succor has large f then q ,skip this by returning true
		{
			return true;//if returns true then the element is not pushed in open list
		}
	}
	return false;
}



PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
	smap = sm;
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
	//delete smap;
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
	const GeoCoord& start,
	const GeoCoord& end,
	list<StreetSegment>& route,
	double& totalDistanceTravelled) const
{
	/*--------------BAD_COORD or START POSITION EQUALS END POSITION----------*/
	// The geo source or end desitination coordinate was not found in mapdata
	vector<StreetSegment> segList;
	if (!(smap->getSegmentsThatStartWith(start, segList)) || !(smap->getSegmentsThatStartWith(end, segList)) || start == end)
	{
		segList.clear();
		route.clear();
		totalDistanceTravelled = 0;
		return BAD_COORD;
	}
	/*-----------------------------SUCCESS---------------------------------*/
	// path found from start to end

	segList.clear(); //clear memory
	// A* ALGORITHM
	// 1. intialize open list
	// 2. initialize closed list
	// 3. vector list for children
	// push starting node onto the open list leaving f value at 0
	vector<Node*> open_list; // 1
	vector<Node*> closed_list; // 2
	vector<StreetSegment> coordSegments; // 3
	StreetSegment endSeg; // to get end of the end node
	Node startSave;
	Node* startNode = startSave.getNode();
	startNode->parent = nullptr;
	startNode->streetName = " ";
	startNode->startCoord = start;
	startNode->endCoord = start;
	startNode->g = 0; // leave f at zero for f = g + h
	startNode->h = distanceEarthMiles(start, end); // estimated distance from start to end
	open_list.push_back(startNode); // push starting node on the open
	//4. while the open list is not empty
	while (!open_list.empty())
	{
		//a) find the node with the last f value on the open list, naming it openMin
		// Find the segment with the least f value
		double minIndex = 0; // finding the minimum of the open list
		for (int i = 0; i < open_list.size(); i++) // finding the node with the min f in the open list
		{
			if (open_list[i]->f() <= open_list[minIndex]->f()) // calling function in our Node Struct to get f
				minIndex = i;
			/*else if (open_list[i]->f() == open_list[minIndex]->f())
			{
				minIndex = i;
			}*/
		}
		Node* openMin = open_list[minIndex]; // this the min of the open list "openMin"
		// b) pop openMin off the open list
		open_list.erase(open_list.begin() + minIndex); // erase the open list until the min node
		// this removes the visited segment to avoid repetition
		// c) generate openMin's successors and set their parents to openMin
		smap->getSegmentsThatStartWith(openMin->endCoord, coordSegments); // O(1) calls StreetMap
		// function
		// d) For the successors of openMin, if the successor is the end goal, stop searching

		/*---------------------if openMin has the end goal, then its child reached the end*-----------*/
		if (foundTheEnd(end, coordSegments, smap, endSeg)) // checks if openMin leads to end destination
		{
			Node* parent = openMin;
			route.push_back(endSeg); // pushes last segment from the foundThehead in the route list paremeter
			totalDistanceTravelled = openMin->g + distanceEarthMiles(openMin->startCoord, endSeg.end); // updating totalDistanceTravelled
			StreetSegment s;
			while (parent->parent != nullptr) // push all segments from openMin until the start in the route list paremeter
			{
				s.name = parent->streetName;
				s.start = parent->startCoord;
				s.end = parent->endCoord;
				route.push_back(s); // update route
				parent = parent->parent;
			}
			reverse(route.begin(), route.end()); // correct order reversing the route list
			return DELIVERY_SUCCESS;
		}
		// for each child of openMin
		for (int i = 0; i < coordSegments.size(); i++)
		{
			if (openMin->endCoord == coordSegments[i].start)
			{
				// create successor from the end, making Parent the openMin
				Node endNode;
				Node* endNext = endNode.getNode(); // generates struct object
				endNext->parent = openMin;
				endNext->startCoord = coordSegments[i].start;
				endNext->endCoord = coordSegments[i].end;
				endNext->g = openMin->g + distanceEarthMiles(openMin->startCoord, endNext->startCoord); // setting g the actual distance until it
				endNext->h = distanceEarthMiles(endNext->endCoord, end);// setting h; approximated value from this node to th end
				endNext->streetName = coordSegments[i].name;

				// Now, if a node with the same position as the child is in the OPEN list which
				// has a lower f, skip it
				// For closed list, skip it and add it to the open list
				if (!isLowerCost(open_list, closed_list, openMin, endNext))
				{
					open_list.push_back(endNext);
				}

				/*endNode.clearAll();
				endNext->clearAll();*/
				
			}
		}
		closed_list.push_back(openMin);// push into closed list
	}
	/*---------------------------NO ROUTE------------------------------*/

	route.clear();
	totalDistanceTravelled = 0;
	return NO_ROUTE;  // Delete this line and implement this function correctly
}



//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
	m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
	delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
	const GeoCoord& start,
	const GeoCoord& end,
	list<StreetSegment>& route,
	double& totalDistanceTravelled) const
{
	return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
