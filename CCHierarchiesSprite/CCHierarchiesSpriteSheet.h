//
//  CCHierarchiesSpriteSheet.h
//  CCHierarchiesSprite
//
//  Created by bman <zx123xz321hm3@hotmail.com>.
//  Copyright (c) 2013. All rights reserved.
//

#ifndef _CCHierarchiesSpriteSheet_H_
#define _CCHierarchiesSpriteSheet_H_

#include "CCHierarchiesSpriteConfig.h"
#include <string>
#   ifdef HIERARCHIES_USE_CPP_11
#include <unordered_map>
#   else
#include <map>
#   endif
#include "rapidxml.hpp"


class CCHierarchiesSpriteSheet {
    
public:
    struct Spr {
        std::string name;
        float x;
        float y;
        float w;
        float h;
        bool isRotation;
        
        Spr () {}
        
        Spr (std::string name, float x, float y, float w, float h, bool isRotation)
        : name(name), x(x), y(y), w(w), h(h), isRotation(isRotation) {
        }
        
        Spr (const Spr& copy) {
            this->name = copy.name;
            this->x = copy.x;
            this->y = copy.y;
            this->w = copy.w;
            this->h = copy.h;
            this->isRotation = copy.isRotation;
        }
        
        Spr& operator= (const Spr& rhs) {
            this->name = rhs.name;
            this->x = rhs.x;
            this->y = rhs.y;
            this->w = rhs.w;
            this->h = rhs.h;
            this->isRotation = rhs.isRotation;
            return *this;
        }
    };
    
protected:
    std::string _imageName;
    float _imageWidth;
    float _imageHeight;
#ifdef HIERARCHIES_USE_CPP_11
    std::unordered_map<std::string, Spr> _sprList;
#else
    std::map<std::string, Spr> _sprList;
#endif
    
protected:
	void parseDirOrSprNode (std::string dirPath, rapidxml::xml_node<>* dirOrSprNode);
    
public:
    CCHierarchiesSpriteSheet (std::string xmlFile);
//    CCHierarchiesSpriteSheet (const CCHierarchiesSpriteSheet& copy);
    virtual ~CCHierarchiesSpriteSheet ();
    
    std::string getImageName ();
    float getImageWidth ();
    float getImageHeight ();
    bool getSpr (std::string name, Spr& out);
    
};

#endif
