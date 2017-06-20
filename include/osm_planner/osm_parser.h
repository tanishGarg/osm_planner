//
// Created by michal on 31.12.2016.
//

#ifndef PROJECT_OSM_PARSER_H
#define PROJECT_OSM_PARSER_H


#include <tinyxml.h>
#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <nav_msgs/Path.h>
#include <tf/transform_datatypes.h>
#include <sensor_msgs/NavSatFix.h>

namespace osm_planner {

    class Parser {
    public:

        template <class T1>
        struct MyStruct{
           double latitude;
            double longitude;

        };

        typedef struct kkt {
            double latitude;
            double longitude;
            double angle;
        } KKT;

        typedef struct osm_node {
            double latitude;
            double longitude;
            double angle;
        } OSM_NODE;

        typedef struct osm_node_with_id {
            int id;
            OSM_NODE node;
        } OSM_NODE_WITH_ID;

        typedef struct osm_way {
            int id;
            std::vector<int> nodesId;
        } OSM_WAY;

        typedef struct translate_table {
            int oldID;
            int newID;
        } TRANSLATE_TABLE;

        const static int CURRENT_POSITION_MARKER = 0;
        const static int TARGET_POSITION_MARKER = 1;

        Parser(std::string xml);

        Parser();

        //start parsing
        void parse();

        //publishing functions
        void publishPoint(int pointID, int marker_type, double radius);

        void publishPoint(geometry_msgs::Point point, int marker_type, double radius);

        void publishPoint(double latitude, double longitude, int marker_type, double radius);

        void publishRouteNetwork();

        void publishRefusedPath(std::vector<int> nodesInPath);

        //deleting edge on the graph
        void deleteEdgeOnGraph(int nodeID_1, int nodeID_2);

        //GETTERS
        std::vector<std::vector<float> > *getGraphOfVertex(); //for dijkstra algorithm
        int getNearestPoint(double lat, double lon); //return OSM node ID
        int getNearestPointXY(double point_x, double point_y); //return OSM node ID
        OSM_NODE getNodeByID(int id);                //OSM NODE contains geogpraphics coordinates
        nav_msgs::Path getPath(std::vector<int> nodesInPath); //get the XY coordinates from vector of IDs
        OSM_NODE getStartPoint();

        //SETTERS
        void setStartPoint(double latitude, double longitude); //set the zero point in cartezian coordinates
        void setStartPoint();
        void setNewMap(std::string xml);

        void setInterpolationMaxDistance(double param);

        //Embedded class for calculating distance and bearing
        //Functions was inspired by: http://www.movable-type.co.uk/scripts/latlong.html
        class Haversine {

        public:

            template<class N1, class N2>static double getDistance(N1 node1, N2 node2){

                /*  Haversine formula:
                *  a = sin²(Δφ/2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ/2)
                *  c = 2 ⋅ atan2( √a, √(1−a) )
                *  d = R ⋅ c
                *
                *  φ - latitude;
                *  λ - longitude;
                */

                double dLat = node2.latitude * DEG2RAD - node1.latitude * DEG2RAD;
                double dLon = node2.longitude * DEG2RAD - node1.longitude * DEG2RAD;
                double a = sin(dLat / 2) * sin(dLat / 2) +
                           cos(node1.latitude * DEG2RAD) * cos(node2.latitude * DEG2RAD) *
                           sin(dLon / 2) * sin(dLon / 2);
                double c = 2 * atan2(sqrt(a), sqrt(1 - a));
                return R * c;
            };

            static double getCoordinateX(double lon1, double lon2, double lat1, double lat2){

                double dLon = lon2 * DEG2RAD - lon1 * DEG2RAD;
                double latAverage = (lat1 + lat2) / 2;
                double a = cos(latAverage * DEG2RAD) * cos(latAverage * DEG2RAD) *
                           sin(dLon / 2) * sin(dLon / 2);
                double dist = R * 2 * atan2(sqrt(a), sqrt(1 - a));

                return lon1 < lon2 ? dist : -dist;
            }

            template<class N1, class N2> static double getCoordinateX(N1 node1, N2 node2){

                double dLon = node2.longitude * DEG2RAD - node1.longitude * DEG2RAD;
                double latAverage = (node1.latitude + node2.latitude) / 2;
                double a = cos(latAverage * DEG2RAD) * cos(latAverage * DEG2RAD) *
                           sin(dLon / 2) * sin(dLon / 2);
                double dist = R * 2 * atan2(sqrt(a), sqrt(1 - a));

                return node1.longitude < node2.longitude ? dist : -dist;
            };

            static double getCoordinateY(double lat1, double lat2){

                static double R = 6371e3;
                double dLat = lat2 * DEG2RAD - lat1 * DEG2RAD;
                double a = sin(dLat / 2) * sin(dLat / 2);
                double dist = R * 2 * atan2(sqrt(a), sqrt(1 - a));

                return lat1 < lat2 ? dist : -dist;
            }

            template<class N1, class N2> static double getCoordinateY(N1 node1, N2 node2){

                static double R = 6371e3;
                double dLat = node2.latitude * DEG2RAD - node1.latitude * DEG2RAD;
                double a = sin(dLat / 2) * sin(dLat / 2);
                double dist = R * 2 * atan2(sqrt(a), sqrt(1 - a));

                return node1.latitude < node2.latitude ? dist : -dist;
            };

            template<class N1, class N2> static double getBearing(N1 node1, N2 node2){

              /*   Haversine formula:
              *   a = sin²(Δφ/2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ/2)
              *   c = 2 ⋅ atan2( √a, √(1−a) )
              *   d = R ⋅ c
              *
              *  φ - latitude;
              *  λ - longitude;
              */

                double dLon = node2.longitude * DEG2RAD - node1.longitude * DEG2RAD;

                double y = sin(dLon) * cos(node2.latitude * DEG2RAD);
                double x = cos(node1.latitude * DEG2RAD) * sin(node2.latitude * DEG2RAD) -
                           sin(node1.latitude * DEG2RAD) * cos(node2.latitude * DEG2RAD) * cos(dLon);
                return atan2(y, x);
            };

        private:
            constexpr static double R = 6371e3;
            constexpr static double DEG2RAD = M_PI / 180;
            constexpr static double RAD2DEG = 180 / M_PI;
        };

    private:

        //map source
        std::string xml;

        //publishers
        ros::Publisher position_marker_pub;
        ros::Publisher target_marker_pub;
        ros::Publisher path_pub;
        ros::Publisher refused_path_pub;

        //visualization msgs
        visualization_msgs::Marker position_marker, target_marker;

        std::string map_frame; //name of frame for msgs
        bool visualization; //enable or disable publishing markers and paths for rviz visualization

        int size_of_nodes;  //usage in function getNodesInWay(), counter of currently read nodes

        //vector arrays of OSM nodes and ways
        std::vector<OSM_WAY> ways;
        std::vector<OSM_NODE> nodes;
        std::vector<OSM_NODE_WITH_ID> interpolated_nodes;
        std::vector<TRANSLATE_TABLE> table;
        std::vector<std::vector<float> > networkArray;

        //start point - must be set and than you can publishing paths
        OSM_NODE startPoint;

        void createMarkers();

        void createWays(TiXmlHandle *hRootWay, TiXmlHandle *hRootNode, std::vector<std::string> osm_value);
        bool isSelectedWay(TiXmlElement *tag, std::vector<std::string> values);

        void createNodes(TiXmlHandle *hRootNode);

        void createNetwork();

        void getNodesInWay(TiXmlElement *wayElement, OSM_WAY *way, std::vector<OSM_NODE_WITH_ID> nodes);

        bool translateID(int id, int *ret_value);

        //ADDED for interpolation
        //------------------------------------------
        //interpolation - creating more nodes between parameters node1 and node2
        std::vector<OSM_NODE> getInterpolatedNodes(OSM_NODE node1, OSM_NODE node2);

        double interpolation_max_distance;

        //finding node by osm id in std::vector<OSM_NODE_WITH_ID> nodes buffer
        OSM_NODE_WITH_ID getNodeByOsmId(std::vector<OSM_NODE_WITH_ID> nodes, int id);
        //------------------------------------------

    };
}

#endif //PROJECT_OSM_PARSER_H
