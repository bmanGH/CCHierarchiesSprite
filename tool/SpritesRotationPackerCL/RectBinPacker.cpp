//
//  RectBinPacker
//
//

#include "RectBinPacker.h"

RectBinPacker::RectBinPacker (float maxW, float maxH)
: _maxW(maxW), _maxH(maxH) {
    _nodes.push_back(Node(0, 0, _maxW, _maxH, -1));
}

RectBinPacker::RectBinPacker (const RectBinPacker& copy) {
	this->_maxW = copy._maxW;
	this->_maxH = copy._maxH;
	this->_nodes = copy._nodes;
}

RectBinPacker& RectBinPacker::operator= (const RectBinPacker& rhs) {
	this->_maxW = rhs._maxW;
	this->_maxH = rhs._maxH;
	this->_nodes = rhs._nodes;
	return *this;
}

bool RectBinPacker::getNodeByUserdataID (int userdataID, Node& out) {
    std::vector<Node>::iterator iter;
    for (iter = _nodes.begin(); iter != _nodes.end(); iter++) {
        if (iter->userdataID == userdataID) {
            out = *iter;
            return true;
        }
    }
    return false;
}

bool RectBinPacker::insert (float insertW, float insertH, int userdataID, Node& out) {
    return insert(insertW, insertH, userdataID, false, out);
}

bool RectBinPacker::insert (float insertW, float insertH, int userdataID, bool allowRotate, Node& out) {
	Rect insertRect(0, 0, insertW, insertH);

	int ret = -1;
	if (allowRotate)
		ret = fill(0, insertRect, userdataID, false);
	else
		ret = fill(0, insertRect, userdataID);

    if (ret != -1) {
        out = _nodes[ret];
        return true;
    }
    else {
        return false;
    }
}

bool RectBinPacker::tryInsert (float insertW, float insertH) {
	return tryInsert(insertW, insertH, false);
}

bool RectBinPacker::tryInsert (float insertW, float insertH, bool allowRotate) {
	Rect insertRect(0, 0, insertW, insertH);

	int ret = -1;
	if (allowRotate)
		ret = tryFill(0, insertRect, false);
	else
		ret = tryFill(0, insertRect);

	if (ret != -1)
		return true;
	else
		return false;
}

void RectBinPacker::clear () {
    _nodes.clear();
    _nodes.push_back(Node(0, 0, _maxW, _maxH, -1));
}

size_t RectBinPacker::getNodeCount() {
    return _nodes.size();
}

int RectBinPacker::tryFill (int nodeIndex, Rect insertRect) {
	Node node = _nodes[nodeIndex];

	if (insertRect.w <= node.rect.w && insertRect.h <= node.rect.h) {
        if (node.packed == false) {
            return nodeIndex;
        }
        else {
            int ret = -1;
            if (node.smallNodeIndex != -1) {
                ret = tryFill(node.smallNodeIndex, insertRect);
            }
            if (ret == -1 && node.bigNodeIndex != -1) {
                ret = tryFill(node.bigNodeIndex, insertRect);
            }
            return ret;
        }
    }
    else {
        return -1;
    }
}

int RectBinPacker::tryFill (int nodeIndex, Rect insertRect, bool rotated) {
	Node node = _nodes[nodeIndex];

	int nodeWH = node.rect.w > node.rect.h ? 1 : -1;
	int insertWH = insertRect.w > insertRect.h ? 1 : -1;
	if (nodeWH * insertWH < 0) {
		insertRect.rotate();
		rotated = !rotated;
	}

    if (insertRect.w <= node.rect.w && insertRect.h <= node.rect.h) {
        if (node.packed == false) {
            return nodeIndex;
        }
        else {
            int ret = -1;
            if (node.smallNodeIndex != -1) {
                ret = tryFill(node.smallNodeIndex, insertRect, rotated);
            }
            if (ret == -1 && node.bigNodeIndex != -1) {
                ret = tryFill(node.bigNodeIndex, insertRect, rotated);
            }
            return ret;
        }
    }
    else {
        return -1;
    }
}

int RectBinPacker::fill (int nodeIndex, Rect insertRect, int insertUserdataID) {
	Node node = _nodes[nodeIndex];
    if (insertRect.w <= node.rect.w && insertRect.h <= node.rect.h) {
        if (node.packed == false) {
            split(nodeIndex, insertRect, insertUserdataID, false);
            return nodeIndex;
        }
        else {
            int ret = -1;
            if (node.smallNodeIndex != -1) {
                ret = fill(node.smallNodeIndex, insertRect, insertUserdataID);
            }
            if (ret == -1 && node.bigNodeIndex != -1) {
                ret = fill(node.bigNodeIndex, insertRect, insertUserdataID);
            }
            return ret;
        }
    }
    else {
        return -1;
    }
}

int RectBinPacker::fill (int nodeIndex, Rect insertRect, int insertUserdataID, bool rotated) {
    Node node = _nodes[nodeIndex];

	int nodeWH = node.rect.w > node.rect.h ? 1 : -1;
	int insertWH = insertRect.w > insertRect.h ? 1 : -1;
	if (nodeWH * insertWH < 0) {
		insertRect.rotate();
		rotated = !rotated;
	}

    if (insertRect.w <= node.rect.w && insertRect.h <= node.rect.h) {
        if (node.packed == false) {
            split(nodeIndex, insertRect, insertUserdataID, rotated);
            return nodeIndex;
        }
        else {
            int ret = -1;
            if (node.smallNodeIndex != -1) {
                ret = fill(node.smallNodeIndex, insertRect, insertUserdataID, rotated);
            }
            if (ret == -1 && node.bigNodeIndex != -1) {
                ret = fill(node.bigNodeIndex, insertRect, insertUserdataID, rotated);
            }
            return ret;
        }
    }
    else {
        return -1;
    }
}

void RectBinPacker::split (int nodeIndex, Rect insertRect, int insertUserdataID, bool rotated) {
    Node node = _nodes[nodeIndex];
    
    Rect left = node.rect;
    Rect right = node.rect;
    Rect top = node.rect;
    Rect bottom = node.rect;
    
    left.y += insertRect.h;
    left.w = insertRect.w;
    left.h -= insertRect.h;
    right.x += insertRect.w;
    right.w -= insertRect.w;
    
    bottom.x += insertRect.w;
    bottom.h = insertRect.h;
    bottom.w -= insertRect.w;
    top.y += insertRect.h;
    top.h -= insertRect.h;
    
    float maxLeftRightArea = left.getArea();
    if (right.getArea() > maxLeftRightArea) {
        maxLeftRightArea = right.getArea();
    }
    
    float maxBottomTopArea = bottom.getArea();
    if (top.getArea() > maxBottomTopArea) {
        maxBottomTopArea = top.getArea();
    }
    
    if (maxLeftRightArea > maxBottomTopArea) {
        if (left.getArea() > right.getArea()) {
            _nodes.push_back(Node(left, -1));
            _nodes.push_back(Node(right, -1));
        } else {
            _nodes.push_back(Node(right, -1));
            _nodes.push_back(Node(left, -1));
        }
    } else {
        if (bottom.getArea() > top.getArea()) {
            _nodes.push_back(Node(bottom, -1));
            _nodes.push_back(Node(top, -1));
        } else {
            _nodes.push_back(Node(top, -1));
            _nodes.push_back(Node(bottom, -1));
        }
    }
    
    node.userdataID = insertUserdataID;
    node.smallNodeIndex = _nodes.size() - 1;
    node.bigNodeIndex = _nodes.size() - 2;
    node.packed = true;
	node.rotated = rotated;
    _nodes[nodeIndex] = node;
}
