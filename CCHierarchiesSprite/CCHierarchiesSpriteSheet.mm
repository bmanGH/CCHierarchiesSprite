//
//  CCHierarchiesSpriteSheet.cpp
//  CCHierarchiesSprite
//
//  Created by bman <zx123xz321hm3@hotmail.com>.
//  Copyright (c) 2013. All rights reserved.
//

#include "CCHierarchiesSpriteSheet.h"
#import "CCFileUtils.h"

using namespace rapidxml;


CCHierarchiesSpriteSheet::CCHierarchiesSpriteSheet (std::string xmlFile) {
    // load file
    NSString* filePath = [[CCFileUtils sharedFileUtils] fullPathFromRelativePath:[NSString stringWithUTF8String:xmlFile.c_str()]];
    NSData* data = [[NSData alloc] initWithContentsOfFile:filePath];
    assert(data);
	char* xml = (char*)malloc(data.length + 1);
	memcpy(xml, data.bytes, data.length);
	xml[data.length] = '\0'; // add string end char
	[data release];
    
    // parse xml
    xml_document<> doc;
	doc.parse<0>(xml);
    
    // <img name=S w=N h=N>
    xml_node<>* imgNode = doc.first_node("img");
    if (NULL == imgNode) {
        CCLOG(@"parse <img> Node error");
        free(xml);
        return;
    }
    
    xml_attribute<>* img_name = imgNode->first_attribute("name");
    if (NULL == img_name) {
        CCLOG(@"parse <img> Node <name> Attr error");
        free(xml);
        return;
    }
    _imageName = img_name->value();
    
    xml_attribute<>* img_w = imgNode->first_attribute("w");
    if (NULL == img_w) {
        CCLOG(@"parse <img> Node <w> Attr error");
        free(xml);
        return;
    }
    _imageWidth = atof(img_w->value());
    
    xml_attribute<>* img_h = imgNode->first_attribute("h");
    if (NULL == img_h) {
        CCLOG(@"parse <img> Node <h> Attr error");
        free(xml);
        return;
    }
    _imageHeight = atof(img_h->value());
    
    // <definitions>
    xml_node<>* definitionsNode = imgNode->first_node("definitions");
    if (NULL == definitionsNode) {
        CCLOG(@"parse <definitions> Node error");
        free(xml);
        return;
    }
    
    // parse dir and spr
    std::string dirPath("");
    xml_node<>* dirOrSprNode = definitionsNode->first_node();
    parseDirOrSprNode(dirPath, dirOrSprNode);
    
    // parse xml end
    free(xml);
}

CCHierarchiesSpriteSheet::~CCHierarchiesSpriteSheet () {
}

void CCHierarchiesSpriteSheet::parseDirOrSprNode (std::string dirPath, rapidxml::xml_node<>* dirOrSprNode) {
    if (NULL == dirOrSprNode)
        return;
    
	// <spr name=S x=N y=N w=N h=N ?isRotation=B>
    if (strcmp(dirOrSprNode->name(), "spr") == 0) {
        xml_attribute<>* attribute = NULL;
        
        attribute = dirOrSprNode->first_attribute("name");
        if (NULL == attribute) {
            CCLOG(@"parse <spr> Node <name> Attr error at path:%s", dirPath.c_str());
            return; 
        }
        std::string spr_name(attribute->value());
        
        attribute = dirOrSprNode->first_attribute("x");
        if (NULL == attribute) {
            CCLOG(@"parse <spr> Node <x> Attr error at path:%s", dirPath.c_str());
            return;
        }
        float spr_x = atof(attribute->value());
        
        attribute = dirOrSprNode->first_attribute("y");
        if (NULL == attribute) {
            CCLOG(@"parse <spr> Node <y> Attr error at path:%s", dirPath.c_str());
            return;
        }
        float spr_y = atof(attribute->value());
        
        attribute = dirOrSprNode->first_attribute("w");
        if (NULL == attribute) {
            CCLOG(@"parse <spr> Node <w> Attr error at path:%s", dirPath.c_str());
            return;
        }
        float spr_w = atof(attribute->value());
        
        attribute = dirOrSprNode->first_attribute("h");
        if (NULL == attribute) {
            CCLOG(@"parse <spr> Node <h> Attr error at path:%s", dirPath.c_str());
            return;
        }
        float spr_h = atof(attribute->value());
        
        attribute = dirOrSprNode->first_attribute("isRotation");
        bool spr_isRotation = false;
		if (NULL != attribute) {
			if (strcmp(attribute->value(), "true") == 0) {
                spr_isRotation = true;
            }
            else {
                spr_isRotation = false;
            }
		}
        
        std::pair<std::string, Spr> item(dirPath + spr_name, Spr(dirPath + spr_name, spr_x, spr_y, spr_w, spr_h, spr_isRotation));
        _sprList.insert(item);
    }
	// <dir name=S>
    else if (strcmp(dirOrSprNode->name(), "dir") == 0) {
        xml_attribute<>* attribute = dirOrSprNode->first_attribute("name");
        if (NULL == attribute) {
            CCLOG(@"parse <dir> Node <name> Attr error at path:%s", dirPath.c_str());
            return;
        }
        std::string dir_name(attribute->value());
        
        parseDirOrSprNode(dirPath + dir_name, dirOrSprNode->first_node());
    }
    
    parseDirOrSprNode(dirPath, dirOrSprNode->next_sibling());
}

std::string CCHierarchiesSpriteSheet::getImageName () {
    return _imageName;
}

float CCHierarchiesSpriteSheet::getImageWidth() {
    return _imageWidth;
}

float CCHierarchiesSpriteSheet::getImageHeight() {
    return _imageHeight;
}

bool CCHierarchiesSpriteSheet::getSpr (std::string name, Spr& out) {
#ifdef HIERARCHIES_USE_CPP_11
    std::unordered_map<std::string, Spr>::iterator iter = _sprList.find(name);
#else
    std::map<std::string, Spr>::iterator iter = _sprList.find(name);
#endif
    if (iter != _sprList.end()) {
        out = iter->second;
        return true;
    }
    return false;
}
