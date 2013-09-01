//
//  CCHierarchiesSpriteSheet.h
//  CCHierarchiesSprite
//
//  Created by Xu Xiaocheng <xcxu@bread-media.com.cn>.
//  Copyright (c) 2012 Break-media. All rights reserved.
//

#ifndef _CCHierarchiesSpriteSheet_H_
#define _CCHierarchiesSpriteSheet_H_

#include <string>
#include <map>
#include "rapidxml/rapidxml.hpp"
#include "ccMacros.h"
#include "cocos-ext.h"


NS_CC_EXT_BEGIN

class CC_DLL CCHierarchiesSpriteSheet {
    
	//    friend class HierarchiesSprite;
    
public:
    struct Spr {
        std::string name;
        unsigned int x;
        unsigned int y;
        unsigned int w;
        unsigned int h;
        
        Spr () {}
        
        Spr (std::string name, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
        : name(name), x(x), y(y), w(w), h(h) {
        }
        
        Spr (const Spr& copy) {
            this->name = copy.name;
            this->x = copy.x;
            this->y = copy.y;
            this->w = copy.w;
            this->h = copy.h;
        }
        
        Spr& operator= (const Spr& rhs) {
            this->name = rhs.name;
            this->x = rhs.x;
            this->y = rhs.y;
            this->w = rhs.w;
            this->h = rhs.h;
            return *this;
        }
    };
    
private:
    std::string _imageName;
    unsigned int _imageWidth;
    unsigned int _imageHeight;
    std::map<std::string, Spr>_sprList;
    
private:
	void parseDirOrSprNode (std::string dirPath, rapidxml::xml_node<>* dirOrSprNode);
    
public:
    CCHierarchiesSpriteSheet (std::string xmlFile);
	//    CCHierarchiesSpriteSheet (const CCHierarchiesSpriteSheet& copy);
    virtual ~CCHierarchiesSpriteSheet ();
    
    std::string getImageName ();
    unsigned int getImageWidth ();
    unsigned int getImageHeight ();
    bool getSpr (std::string name, Spr& out);
    
};

NS_CC_EXT_END

#endif
