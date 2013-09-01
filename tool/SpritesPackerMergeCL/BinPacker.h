//
//  BinPacker.h
//  CCHierarchiesSprite
//
//  Created by Xu Xiaocheng <xcxu@bread-media.com.cn>.
//  Copyright (c) 2012 Break-media. All rights reserved.
//

#ifndef _BinPacker_H_
#define _BinPacker_H_

#include <vector>
#include <algorithm>

class BinPacker {
    
public:
    struct Rect {
        int x;
        int y;
        int w;
        int h;
        
        Rect () {}
        
        Rect (const Rect& copy) {
            this->x = copy.x;
            this->y = copy.y;
            this->w = copy.w;
            this->h = copy.h;
        }
        
        Rect& operator= (const Rect& rhs) {
            this->x = rhs.x;
            this->y = rhs.y;
            this->w = rhs.w;
            this->h = rhs.h;
            return *this;
        }
        
        Rect (int x, int y, int w, int h)
        : x(x), y(y), w(w), h(h) {
        }
        
        int getArea () const {
            return w * h;
        }
        
        // std::sort(rects.rbegin(), rects.rend()); // Sort from greatest to least area
        bool operator< (const Rect& rhs) {
            return this->getArea() < rhs.getArea();
        }
    };
    
    struct Node {
        Rect rect;
        int userdataID;
        int smallNodeIndex;
        int bigNodeIndex;
//        bool rotated;
        bool packed;
        
        Node ()
		: smallNodeIndex(-1), bigNodeIndex(-1), packed(false) {}
        
//        Node (int x, int y, int w, int h, int userdataID)
//        : rect(x, y, w, h), userdataID(userdataID), smallNodeIndex(-1), bigNodeIndex(-1), rotated(false), packed(false) {
//        }
        Node (int x, int y, int w, int h, int userdataID)
        : rect(x, y, w, h), userdataID(userdataID), smallNodeIndex(-1), bigNodeIndex(-1), packed(false) {
        }
        
//        Node (Rect& rect, int userdataID)
//        : rect(rect), userdataID(userdataID), smallNodeIndex(-1), bigNodeIndex(-1), rotated(false), packed(false) {
//        }
        Node (Rect& rect, int userdataID)
        : rect(rect), userdataID(userdataID), smallNodeIndex(-1), bigNodeIndex(-1), packed(false) {
        }
        
//        void rotate () {
//            std::swap(rect.w, rect.h);
//            rotated = true;
//        }
    };

protected:
    int _maxW;
    int _maxH;
    std::vector<Node> _nodes;
    
	int tryFill (int nodeIndex, Rect insertRect);
    int fill (int nodeIndex, Rect insertRect, int insertUserdataID);
    void split (int nodeIndex, Rect insertRect, int insertUserdataID);
    
public:
    BinPacker (int maxW, int maxH);
    bool getNodeByUserdataID (int userdataID, Node& out);
    bool insert (int insertW, int insertH, int insertUserdataID, Node& out);
//    bool insert (int insertW, int insertH, int insertUserdataID, bool allowRotation, Node& out);
	bool tryInsert (int insertW, int insertH);
    void clear();
    size_t getNodeCount();
    
};

#endif
