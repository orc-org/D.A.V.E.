#pragma once

#include <cstdio>
#include <iostream>
#include "Waypoint.hpp"

using namespace std;


/*
    ListNode
    --------
    the core part of my doubly linked list implementation
*/
class ListNode{
    public:
        Waypoint* point;
        ListNode* next;
        ListNode* previous;

        ListNode(ListNode* next, ListNode* previous,Waypoint* newPoint) : point(newPoint), next(next), previous(previous) {}
        
        ~ListNode() { delete point; }
};

/*
    ListOfWaypoints
    ---------------
    a simple pointer based implementation of a doubly linked list
*/
class ListOfWaypoints{
    private:
        ListNode* head;
        ListNode* tail;

    public:
        ListOfWaypoints() : head(nullptr), tail(nullptr) {}
        
        ~ListOfWaypoints() { clear(); }

        ListNode* getHead(){
            return head;
        }

        ListNode* getTail(){
            return tail;
        }

        ListNode* getPoint(int position);

        ListNode* getPoint(Waypoint* point);

        ListNode* getPoint(double x, double y);

        ListNode* getGeodeticPoint(double longitude, double latitude);

        ListNode* getEarthCentredPoint(double x, double y, double z);

        void add(Waypoint* newPoint);

        void addFirst(Waypoint* newPoint);

        void add(Waypoint* newPoint, int position);

        void addBefore(ListNode* newNode, ListNode* existingNode);

        void printList();

        void clear();

        void removeFirst();

        void removeLast();

        void removeAt(int position);

        void remove(Waypoint* point);

        void remove(ListNode* node);

        void removePoint(double x, double y);

        void removeGeodeticPoint(double longitude, double latitude);

        void removeEarthCentredPoint(double x, double y, double z);  

        bool areEqual(Waypoint* point1, Waypoint* point2);
};

