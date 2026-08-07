// #include "Waypoint.cpp"
#include <cstdio>
#include <iostream>

#include "ListOfWaypoints.hpp"
#include "Waypoint.hpp"
    
ListNode* ListOfWaypoints::getPoint(int position){
    if (position < 0) return nullptr;
#include "ListOfWaypoints.hpp"
#include "Waypoint.hpp"
    
ListNode* ListOfWaypoints::getPoint(int position){
    if (position < 0) return nullptr;

    ListNode* current = head;
    int index = 0;
    ListNode* current = head;
    int index = 0;

    while (current != nullptr && index < position) {
        current = current->next;
            index++;
    }
    while (current != nullptr && index < position) {
        current = current->next;
            index++;
    }

    return current;
}
    return current;
}

ListNode* ListOfWaypoints::getPoint(Waypoint* point){
    ListNode* current = head;
ListNode* ListOfWaypoints::getPoint(Waypoint* point){
    ListNode* current = head;

    while (current != nullptr) {
        if (areEqual(current->point, point)) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}
    while (current != nullptr) {
        if (areEqual(current->point, point)) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

ListNode* ListOfWaypoints::getPoint(double x, double y){
    ListNode* current = head;
ListNode* ListOfWaypoints::getPoint(double x, double y){
    ListNode* current = head;

    while (current != nullptr) {
        if (current->point->getX() == x && current->point->getY() == y) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}
    while (current != nullptr) {
        if (current->point->getX() == x && current->point->getY() == y) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

ListNode* ListOfWaypoints::getGeodeticPoint(double latitude, double longitude){
    ListNode* current = head;
ListNode* ListOfWaypoints::getGeodeticPoint(double latitude, double longitude){
    ListNode* current = head;

    while (current != nullptr) {
        if (current->point->getLongitude() == longitude && current->point->getLatitude() == latitude) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}
    while (current != nullptr) {
        if (current->point->getLongitude() == longitude && current->point->getLatitude() == latitude) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

ListNode* ListOfWaypoints::getEarthCentredPoint(double x, double y, double z){
    ListNode* current = head;
ListNode* ListOfWaypoints::getEarthCentredPoint(double x, double y, double z){
    ListNode* current = head;
    
    while (current != nullptr) {
        if (current->point->getECEF_x() == x && current->point->getECEF_y() == y && current->point->getECEF_z() == z) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}
    while (current != nullptr) {
        if (current->point->getECEF_x() == x && current->point->getECEF_y() == y && current->point->getECEF_z() == z) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

void ListOfWaypoints::add(Waypoint* newPoint){
    ListNode* newNode = new ListNode(nullptr, tail, newPoint);
    if (tail != nullptr) {
        tail->next = newNode;
    }
    tail = newNode;
    if (head == nullptr) {
        head = newNode;
    }
}
void ListOfWaypoints::add(Waypoint* newPoint){
    ListNode* newNode = new ListNode(nullptr, tail, newPoint);
    if (tail != nullptr) {
        tail->next = newNode;
    }
    tail = newNode;
    if (head == nullptr) {
        head = newNode;
    }
}

void ListOfWaypoints::addFirst(Waypoint* newPoint){
    ListNode* newNode = new ListNode(head, nullptr, newPoint);
    if (head != nullptr) {
        head->previous = newNode;
    }
    head = newNode;
    if (tail == nullptr) {
        tail = newNode;
    }
}
void ListOfWaypoints::addFirst(Waypoint* newPoint){
    ListNode* newNode = new ListNode(head, nullptr, newPoint);
    if (head != nullptr) {
        head->previous = newNode;
    }
    head = newNode;
    if (tail == nullptr) {
        tail = newNode;
    }
}

void ListOfWaypoints::add(Waypoint* newPoint, int position){
    if (position <= 0) {
        addFirst(newPoint);
        return;
    }
void ListOfWaypoints::add(Waypoint* newPoint, int position){
    if (position <= 0) {
        addFirst(newPoint);
        return;
    }

    ListNode* current = head;
    int index = 0;
    ListNode* current = head;
    int index = 0;

    while (current != nullptr && index < position) {
        current = current->next;
        index++;
    }
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

void ListOfWaypoints::addBefore(ListNode* newNode, ListNode* existingNode){
void ListOfWaypoints::addBefore(ListNode* newNode, ListNode* existingNode){
            
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

void ListOfWaypoints::printList() {
    ListNode* current = head;
    while (current != nullptr) {
        cout << "(" << current->point->getX() << ", " << current->point->getY() << ") -> ";
        current = current->next;
    }
    cout << "End of Route" << endl;
}
void ListOfWaypoints::printList() {
    ListNode* current = head;
    while (current != nullptr) {
        cout << "(" << current->point->getX() << ", " << current->point->getY() << ") -> ";
        current = current->next;
    }
    cout << "End of Route" << endl;
}

void ListOfWaypoints::clear() {
    ListNode* current = head;
    while (current != nullptr) {
        ListNode* next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
    tail = nullptr;
}
void ListOfWaypoints::clear() {
    ListNode* current = head;
    while (current != nullptr) {
        ListNode* next = current->next;
        delete current;
        current = next;
    }
    head = nullptr;
    tail = nullptr;
}

void ListOfWaypoints::removeFirst() {
    if (head == nullptr) return;
void ListOfWaypoints::removeFirst() {
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
    ListNode* temp = head;
    head = head->next;
    if (head != nullptr) {
        head->previous = nullptr;
    } else {
        tail = nullptr;
    }
    delete temp;
}

void ListOfWaypoints::removeLast() {
    if (tail == nullptr) return;
void ListOfWaypoints::removeLast() {
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
    ListNode* temp = tail;
    tail = tail->previous;
    if (tail != nullptr) {
        tail->next = nullptr;
    } else {
        head = nullptr;
    }
    delete temp;
}

void ListOfWaypoints::removeAt(int position) {
    if (position < 0) return;
void ListOfWaypoints::removeAt(int position) {
    if (position < 0) return;

    ListNode* current = head;
    int index = 0;
    ListNode* current = head;
    int index = 0;

    while (current != nullptr && index < position) {
        current = current->next;
        index++;
    }
    while (current != nullptr && index < position) {
        current = current->next;
        index++;
    }

    if (current == nullptr) return;
    if (current == nullptr) return;

    if (current->previous != nullptr) {
       current->previous->next = current->next;
    } else {
        head = current->next;
    }
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
    if (current->next != nullptr) {
        current->next->previous = current->previous;
    } else {
        tail = current->previous;
    }

    delete current;
}
    delete current;
}

void ListOfWaypoints::remove(Waypoint* point) {
    ListNode* current = head;
void ListOfWaypoints::remove(Waypoint* point) {
    ListNode* current = head;

    while (current != nullptr) {
        if (areEqual(current->point, point)) {
            if (current->previous != nullptr) {
                current->previous->next = current->next;
            } else {
                head = current->next;
            }
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
            delete current;
            return;
        }
        current = current->next;
    }
}

void ListOfWaypoints::remove(ListNode* node) {
    if (node == nullptr) return;
void ListOfWaypoints::remove(ListNode* node) {
    if (node == nullptr) return;

    if (node->previous != nullptr) {
        node->previous->next = node->next;
    } else {
        head = node->next;
    }
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
    if (node->next != nullptr) {
        node->next->previous = node->previous;
    } else {
        tail = node->previous;
    }

    delete node;
}
    delete node;
}

bool ListOfWaypoints::removePoint(double x, double y) {
    ListNode* current = head;
bool ListOfWaypoints::removePoint(double x, double y) {
    ListNode* current = head;

    while (current != nullptr) {
        if (current->point->getX() == x && current->point->getY() == y) {
            if (current->previous != nullptr) {
                current->previous->next = current->next;
            } else {
                head = current->next;
            }
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
            if (current->next != nullptr) {
                current->next->previous = current->previous;
            } else {
                tail = current->previous;
            }

            delete current;
            return true;
        }
        current = current->next;
        return false;
    }
}
            delete current;
            return true;
        }
        current = current->next;
        return false;
    }
}

bool ListOfWaypoints::removeGeodeticPoint(double latitude, double longitude) {
    ListNode* current = head;
bool ListOfWaypoints::removeGeodeticPoint(double latitude, double longitude) {
    ListNode* current = head;

    while (current != nullptr) {
        if (current->point->getLongitude() == longitude && current->point->getLatitude() == latitude) {
            if (current->previous != nullptr) {
                current->previous->next = current->next;
            } else {
                head = current->next;
            }
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
            if (current->next != nullptr) {
                current->next->previous = current->previous;
            } else {
                tail = current->previous;
            }

            delete current;
            return true;
        }
        current = current->next;
    }
    return false;
}
            delete current;
            return true;
        }
        current = current->next;
    }
    return false;
}

bool ListOfWaypoints::removeEarthCentredPoint(double x, double y, double z) {
    ListNode* current = head;
bool ListOfWaypoints::removeEarthCentredPoint(double x, double y, double z) {
    ListNode* current = head;

    while (current != nullptr) {
        if (current->point->getECEF_x() == x && current->point->getECEF_y() == y && current->point->getECEF_z() == z) {
            if (current->previous != nullptr) {
                current->previous->next = current->next;
            } else {
                head = current->next;
            }
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
             if (current->next != nullptr) {
                current->next->previous = current->previous;
            } else {
                tail = current->previous;
            }

            delete current;
            return true;
        }
        current = current->next;
    }
    return false;
}
            delete current;
            return true;
        }
        current = current->next;
    }
    return false;
}

bool ListOfWaypoints::areEqual(Waypoint* point1, Waypoint* point2) {
    return (point1->getX() == point2->getX()) && (point1->getY() == point2->getY());
}
