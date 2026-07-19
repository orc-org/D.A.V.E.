#ifndef LIST_OF_WAYPOINTS_CPP
#define LIST_OF_WAYPOINTS_CPP

#include "Waypoint.cpp"
#include <cstdio>
#include <iostream>

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

        ListNode* getPoint(int position){
            if (position < 0) return nullptr;

            ListNode* current = head;
            int index = 0;

            while (current != nullptr && index < position) {
                current = current->next;
                index++;
            }

            return current;
        }

        ListNode* getPoint(Waypoint* point){
            ListNode* current = head;

            while (current != nullptr) {
                if (areEqual(current->point, point)) {
                    return current;
                }
                current = current->next;
            }
            return nullptr;
        }

        ListNode* getPoint(double x, double y){
            ListNode* current = head;

            while (current != nullptr) {
                if (current->point->getX() == x && current->point->getY() == y) {
                    return current;
                }
                current = current->next;
            }
            return nullptr;
        }

        ListNode* getGeodeticPoint(double longitude, double latitude){
            ListNode* current = head;

            while (current != nullptr) {
                if (current->point->getLongitude() == longitude && current->point->getLatitude() == latitude) {
                    return current;
                }
                current = current->next;
            }
            return nullptr;
        }

        ListNode* getEarthCentredPoint(double x, double y, double z){
            ListNode* current = head;
    
            while (current != nullptr) {
                if (current->point->getECEF_x() == x && current->point->getECEF_y() == y && current->point->getECEF_z() == z) {
                    return current;
                }
                current = current->next;
            }
            return nullptr;
        }

        void add(Waypoint* newPoint){
            ListNode* newNode = new ListNode(nullptr, tail, newPoint);
            if (tail != nullptr) {
                tail->next = newNode;
            }
            tail = newNode;
            if (head == nullptr) {
                head = newNode;
            }
        }

        void addFirst(Waypoint* newPoint){
            ListNode* newNode = new ListNode(head, nullptr, newPoint);
            if (head != nullptr) {
                head->previous = newNode;
            }
            head = newNode;
            if (tail == nullptr) {
                tail = newNode;
            }
        }

        void add(Waypoint* newPoint, int position){
            if (position <= 0) {
                addFirst(newPoint);
                return;
            }

            ListNode* current = head;
            int index = 0;

            while (current != nullptr && index < position) {
                current = current->next;
                index++;
            }

            if (current == nullptr) {
                add(newPoint);
            } else {
                ListNode* newNode = new ListNode(current, current->previous, newPoint);
                if (current->previous != nullptr) {
                    current->previous->next = newNode;
                }
                current->previous = newNode;
                if (current == head) {
                    head = newNode;
                }
            }
        }

        void addBefore(ListNode* newNode, ListNode* existingNode){
            
            if (existingNode == head) {
                head = newNode;
                newNode->previous = nullptr;
            }
            else{
                existingNode->previous->next = newNode;
                newNode->previous = existingNode->previous;
            }
            existingNode->previous = newNode;
            newNode->next = existingNode;
        }

        void printList() {
            ListNode* current = head;
            while (current != nullptr) {
                cout << "(" << current->point->getX() << ", " << current->point->getY() << ") -> ";
                current = current->next;
            }
            cout << "End of Route" << endl;
        }

        void clear() {
            ListNode* current = head;
            while (current != nullptr) {
                ListNode* next = current->next;
                delete current;
                current = next;
            }
            head = nullptr;
            tail = nullptr;
        }

        void removeFirst() {
            if (head == nullptr) return;

            ListNode* temp = head;
            head = head->next;
            if (head != nullptr) {
                head->previous = nullptr;
            } else {
                tail = nullptr;
            }
            delete temp;
        }

        void removeLast() {
            if (tail == nullptr) return;

            ListNode* temp = tail;
            tail = tail->previous;
            if (tail != nullptr) {
                tail->next = nullptr;
            } else {
                head = nullptr;
            }
            delete temp;
        }

        void removeAt(int position) {
            if (position < 0) return;

            ListNode* current = head;
            int index = 0;

            while (current != nullptr && index < position) {
                current = current->next;
                index++;
            }

            if (current == nullptr) return;

            if (current->previous != nullptr) {
                current->previous->next = current->next;
            } else {
                head = current->next;
            }

            if (current->next != nullptr) {
                current->next->previous = current->previous;
            } else {
                tail = current->previous;
            }

            delete current;
        }

        void remove(Waypoint* point) {
            ListNode* current = head;

            while (current != nullptr) {
                if (areEqual(current->point, point)) {
                    if (current->previous != nullptr) {
                        current->previous->next = current->next;
                    } else {
                        head = current->next;
                    }

                    if (current->next != nullptr) {
                        current->next->previous = current->previous;
                    } else {
                        tail = current->previous;
                    }

                    delete current;
                    return;
                }
                current = current->next;
            }
        }

        void remove(ListNode* node) {
            if (node == nullptr) return;

            if (node->previous != nullptr) {
                node->previous->next = node->next;
            } else {
                head = node->next;
            }

            if (node->next != nullptr) {
                node->next->previous = node->previous;
            } else {
                tail = node->previous;
            }

            delete node;
        }

        void removePoint(double x, double y) {
            ListNode* current = head;

            while (current != nullptr) {
                if (current->point->getX() == x && current->point->getY() == y) {
                    if (current->previous != nullptr) {
                        current->previous->next = current->next;
                    } else {
                        head = current->next;
                    }

                    if (current->next != nullptr) {
                        current->next->previous = current->previous;
                    } else {
                        tail = current->previous;
                    }

                    delete current;
                    return;
                }
                current = current->next;
            }
        }

        void removeGeodeticPoint(double longitude, double latitude) {
            ListNode* current = head;

            while (current != nullptr) {
                if (current->point->getLongitude() == longitude && current->point->getLatitude() == latitude) {
                    if (current->previous != nullptr) {
                        current->previous->next = current->next;
                    } else {
                        head = current->next;
                    }

                    if (current->next != nullptr) {
                        current->next->previous = current->previous;
                    } else {
                        tail = current->previous;
                    }

                    delete current;
                    return;
                }
                current = current->next;
            }
        }

        void removeEarthCentredPoint(double x, double y, double z) {
            ListNode* current = head;

            while (current != nullptr) {
                if (current->point->getECEF_x() == x && current->point->getECEF_y() == y && current->point->getECEF_z() == z) {
                    if (current->previous != nullptr) {
                        current->previous->next = current->next;
                    } else {
                        head = current->next;
                    }

                    if (current->next != nullptr) {
                        current->next->previous = current->previous;
                    } else {
                        tail = current->previous;
                    }

                    delete current;
                    return;
                }
                current = current->next;
            }
        }

        bool areEqual(Waypoint* point1, Waypoint* point2) {
            return (point1->getX() == point2->getX()) && (point1->getY() == point2->getY());
        }
};

// dummy main function for standalone testing
// int main(int argc, char** argv) {
//     (void)argc;
//     (void)argv;
// 
//     return 0;
// }
#endif