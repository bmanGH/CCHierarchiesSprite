//
//  RectBinPacker
//
//

#ifndef _RectBinPacker_H_
#define _RectBinPacker_H_

#include <vector>
#include <algorithm>

class RectBinPacker {
    
public:
    struct Rect {
        float x;
        float y;
        float w;
        float h;
        
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
        
        Rect (float x, float y, float w, float h)
        : x(x), y(y), w(w), h(h) {
        }
        
        float getArea () const {
            return w * h;
        }

		void rotate () {
            std::swap(w, h);
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
        bool rotated;
        bool packed;
        
        Node ()
		: smallNodeIndex(-1), bigNodeIndex(-1), packed(false), rotated(false) {}
        
        Node (float x, float y, float w, float h, int userdataID)
        : rect(x, y, w, h), userdataID(userdataID), smallNodeIndex(-1), bigNodeIndex(-1), rotated(false), packed(false) {
        }
        
        Node (Rect& rect, int userdataID)
        : rect(rect), userdataID(userdataID), smallNodeIndex(-1), bigNodeIndex(-1), rotated(false), packed(false) {
        }

		Node (const Node& copy) {
            this->rect = copy.rect;
            this->userdataID = copy.userdataID;
            this->smallNodeIndex = copy.smallNodeIndex;
            this->bigNodeIndex = copy.bigNodeIndex;
			this->rotated = copy.rotated;
			this->packed = copy.packed;
        }
        
        Node& operator= (const Node& rhs) {
            this->rect = rhs.rect;
            this->userdataID = rhs.userdataID;
            this->smallNodeIndex = rhs.smallNodeIndex;
            this->bigNodeIndex = rhs.bigNodeIndex;
			this->rotated = rhs.rotated;
			this->packed = rhs.packed;
            return *this;
        }
    };

protected:
    float _maxW;
    float _maxH;
    std::vector<Node> _nodes;
    
	int tryFill (int nodeIndex, Rect insertRect);
	int tryFill (int nodeIndex, Rect insertRect, bool rotated);
    int fill (int nodeIndex, Rect insertRect, int insertUserdataID);
    int fill (int nodeIndex, Rect insertRect, int insertUserdataID, bool rotated);
    void split (int nodeIndex, Rect insertRect, int insertUserdataID, bool rotated);
    
public:
    RectBinPacker (float maxW, float maxH);
	RectBinPacker (const RectBinPacker& copy);
	RectBinPacker& operator= (const RectBinPacker& rhs);
    bool getNodeByUserdataID (int userdataID, Node& out);
    bool insert (float insertW, float insertH, int userdataID, Node& out);
    bool insert (float insertW, float insertH, int userdataID, bool allowRotate, Node& out);
	bool tryInsert (float insertW, float insertH);
	bool tryInsert (float insertW, float insertH, bool allowRotate);
    void clear();
    size_t getNodeCount();
    
};

#endif
