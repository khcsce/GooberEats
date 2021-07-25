#include "provided.h"
#include <vector>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private:
    const StreetMap* smap;
    string angleControl(double angle) const;
    void deliveryExtractor(GeoCoord start, GeoCoord destination, double& deliveryDiistances,
        vector<DeliveryCommand> & commands, list<StreetSegment> streetList,string food) const;
    bool turnDirection(double direction, string& turnDirection) const;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
{
    smap = sm;
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}


DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    list<StreetSegment> streetList;
    vector<list<StreetSegment>> streetVect;
   //vector<DeliveryRequest> optimizedDeliv = deliveries;
    //DeliveryOptimizer deliveryOpt(smap);
    //double oldCrow = 0, newCrow = 0;
    //deliveryOpt.optimizeDeliveryOrder(depot, optimizedDeliv, oldCrow, newCrow);
    // THE OPTIMIZER ACTUALLY MADE THE DISTANCE WORSE SO I DIDN'T IMPLEMENT IT IN THIS FUNCTION :)
    double subDistanceDelivery, totalDistance = 0; // temporary distance to finalize totalDistanceTravelled at the end
    GeoCoord gdepot = depot; // departure postion
    PointToPointRouter pointRouter(smap); // PointToPointRouter takes in fully loaded StreetMap as a parameter
    streetList.clear();
    DeliveryResult deliveryRoute;
    // LOOP THROUGH AND GO TEACH SUB ENDLOCATION TO DELIVER THE FOOD TO PEOPLE
    for (int i = 0; i < deliveries.size(); i++)
    {
        // generatePointToPoint distance parameter is by reference
        deliveryRoute = pointRouter.generatePointToPointRoute(gdepot, deliveries[i].location, streetList, subDistanceDelivery);
        // PointToPointRouter class returns the same constant, so take advantage
        if (deliveryRoute == NO_ROUTE)
            return NO_ROUTE;
        if (deliveryRoute == BAD_COORD)
            return BAD_COORD;
        deliveryExtractor(depot, deliveries[i].location, subDistanceDelivery, commands, streetList, deliveries[i].item);
        streetList.clear();
        totalDistance += subDistanceDelivery;
        gdepot = deliveries[i].location;
    }
    //TRACE BACK TO DEPARTURE LOCATION!!!! 
    pointRouter.generatePointToPointRoute(deliveries.back().location, depot, streetList, subDistanceDelivery);
    deliveryExtractor(depot, depot, subDistanceDelivery, commands, streetList,"");
    totalDistanceTravelled = totalDistance + subDistanceDelivery;
    return DELIVERY_SUCCESS;
}


string DeliveryPlannerImpl::angleControl(double angle) const
{
    if (angle >= 0 && angle < 22.5)
        return "east";
    else if (angle >= 22.5 && angle < 67.5)
        return"northeast";
    else if (angle >= 67.5 && angle < 112.5)
        return "north";
    else if (angle >= 112.5 && angle < 157.5)
        return "northwest";
    else if (angle >= 157.5 && angle < 202.5)
        return  "west";
    else if (angle >= 202.5 && angle < 247.5)
        return  "southwest";
    else if (angle >= 247.5 && angle < 292.5)
        return "south";
    else if (angle >= 292.5 && angle < 337.5)
        return "southeast";
    else
        return "east";
}

bool DeliveryPlannerImpl::turnDirection(double direction, string& turnDirection) const
{
    if (direction >= 1 && direction < 180)
    {
        turnDirection = "left";
        return true;
    }
    else
    {
        turnDirection = "right";
        return true;
    }
    if (direction < 1 || direction > 359)
        return false;
}


void DeliveryPlannerImpl::deliveryExtractor(GeoCoord start, GeoCoord destination, double& deliveryDiistances,
        vector<DeliveryCommand>& commands, list<StreetSegment> streetList, string food) const
{
        string currStreet = streetList.back().name;
        string direction;
        PointToPointRouter pointRouter(smap);
        DeliveryCommand command;
        StreetSegment old;
        double dist = 0;
        double directDisplacement;
        double angle = angleOfLine(streetList.back());
        for (auto it = streetList.begin(); it != streetList.end(); it++)
        {
            if (it->name == currStreet)
            {
                dist = dist + distanceEarthMiles(it->start, it->end);
                old = *it;
            }
            else
            {
                directDisplacement = angleBetween2Lines(old, *it);
                command.initAsProceedCommand(angleControl(angle), currStreet, dist);
                dist = 0;
                commands.push_back(command);
                currStreet = it->name;
                if (turnDirection(directDisplacement, direction/*proceed_Angle(angle))*/))
                {
                    dist = dist + distanceEarthMiles(it->start, it->end);
                    command.initAsTurnCommand(direction, currStreet);
                    commands.push_back(command);
                    angle = angleOfLine(*it);
                } // otherwise just proceed
            }
            if (it->end == destination)
            {
                angleControl(angle);
                command.initAsProceedCommand(direction, currStreet, dist);
                commands.push_back(command);
            }
            if (it->end == destination)
            {
                if (food != "")
                {
                    command.initAsDeliverCommand(food);
                    commands.push_back(command);
                }
            }
        }
    }




//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}







