//
//  BinPacker.cpp
//  CCHierarchiesSprite
//
//  Created by Xu Xiaocheng <xcxu@bread-media.com.cn>.
//  Copyright (c) 2012 Break-media. All rights reserved.
//

#include "BinPacker.h"

BinPacker::BinPacker (int maxW, int maxH)
: _maxW(maxW), _maxH(maxH) {
    _nodes.push_back(Node(0, 0, _maxW, _maxH, -1));
}

bool BinPacker::getNodeByUserdataID (int userdataID, Node& out) {
    std::vector<Node>::iterator iter;
    for (iter = _nodes.begin(); iter != _nodes.end(); iter++) {
        if (iter->userdataID == userdataID) {
            out = *iter;
            return true;
        }
    }
    return false;
}

bool BinPacker::insert (int insertW, int insertH, int userdataID, Node& out) {
    Rect insertRect(0, 0, insertW, insertH);
    int ret = fill(0, insertRect, userdataID);
    if (ret != -1) {
        out = _nodes[ret];
        return true;
    }
    else {
        return false;
    }
}

bool BinPacker::tryInsert (int insertW, int insertH) {
	Rect insertRect(0, 0, insertW, insertH);
	int ret = tryFill(0, insertRect);
	if (ret != -1)
		return true;
	else
		return false;
}

void BinPacker::clear () {
    _nodes.clear();
    _nodes.push_back(Node(0, 0, _maxW, _maxH, -1));
}

size_t BinPacker::getNodeCount() {
    return _nodes.size();
}

int BinPacker::tryFill (int nodeIndex, Rect insertRect) {
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

int BinPacker::fill (int nodeIndex, Rect insertRect, int insertUserdataID) {
    Node node = _nodes[nodeIndex];
    if (insertRect.w <= node.rect.w && insertRect.h <= node.rect.h) {
        if (node.packed == false) {
            split(nodeIndex, insertRect, insertUserdataID);
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

void BinPacker::split (int nodeIndex, Rect insertRect, int insertUserdataID) {
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
    
    int maxLeftRightArea = left.getArea();
    if (right.getArea() > maxLeftRightArea) {
        maxLeftRightArea = right.getArea();
    }
    
    int maxBottomTopArea = bottom.getArea();
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
    _nodes[nodeIndex] = node;
}
