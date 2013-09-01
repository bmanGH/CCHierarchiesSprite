//
//  CCHierarchiesSpriteSheet.cpp
//  CCHierarchiesSprite
//
//  Created by Xu Xiaocheng <xcxu@bread-media.com.cn>.
//  Copyright (c) 2012 Break-media. All rights reserved.
//

#include "CCHierarchiesSpriteSheet.h"
#include "CCHierarchiesSpriteConfig.h"
#include "platform/CCFileUtils.h"

using namespace rapidxml;
USING_NS_CC;


NS_CC_EXT_BEGIN

CCHierarchiesSpriteSheet::CCHierarchiesSpriteSheet (std::string xmlFile) {
	std::string filePath = CCFileUtils::sharedFileUtils()->fullPathFromRelativePath(xmlFile.c_str());
	
	unsigned long size = 0;
	unsigned char* data = CCFileUtils::sharedFileUtils()->getFileData(filePath.c_str(), "rb", &size);
	char* xml = (char*)malloc(size + 2);
	memcpy(xml, data, size);
	xml[size] = '\0';
	xml[size + 1] = '\0';
	CC_SAFE_DELETE_ARRAY(data);
    
    // parse xml
    xml_document<> doc;
	doc.parse<0>(xml);
    
    // <img name=S w=N h=N>
    xml_node<>* imgNode = doc.first_node("img");
    if (NULL == imgNode) {
        CCLOG("parse <img> Node error");
        free(xml);
        return;
    }
    
    xml_attribute<>* img_name = imgNode->first_attribute("name");
    if (NULL == img_name) {
        CCLOG("parse <img> Node <name> Attr error");
        free(xml);
        return;
    }
    _imageName = img_name->value();
    
    xml_attribute<>* img_w = imgNode->first_attribute("w");
    if (NULL == img_w) {
        CCLOG("parse <img> Node <w> Attr error");
        free(xml);
        return;
    }
    _imageWidth = atoi(img_w->value());
    
    xml_attribute<>* img_h = imgNode->first_attribute("h");
    if (NULL == img_h) {
        CCLOG("parse <img> Node <h> Attr error");
        free(xml);
        return;
    }
    _imageHeight = atoi(img_h->value());
    
    // <definitions>
    xml_node<>* definitionsNode = imgNode->first_node("definitions");
    if (NULL == definitionsNode) {
        CCLOG("parse <definitions> Node error");
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
    
	// <spr name=S x=N y=N w=N h=N>
    if (strcmp(dirOrSprNode->name(), "spr") == 0) {
        xml_attribute<>* attribute = NULL;
        
        attribute = dirOrSprNode->first_attribute("name");
        if (NULL == attribute) {
            CCLOG("parse <spr> Node <name> Attr error at path:%s", dirPath.c_str());
            return; 
        }
        std::string spr_name(attribute->value());
        
        attribute = dirOrSprNode->first_attribute("x");
        if (NULL == attribute) {
            CCLOG("parse <spr> Node <x> Attr error at path:%s", dirPath.c_str());
            return;
        }
        unsigned int spr_x = atoi(attribute->value());
        
        attribute = dirOrSprNode->first_attribute("y");
        if (NULL == attribute) {
            CCLOG("parse <spr> Node <y> Attr error at path:%s", dirPath.c_str());
            return;
        }
        unsigned int spr_y = atoi(attribute->value());
        
        attribute = dirOrSprNode->first_attribute("w");
        if (NULL == attribute) {
            CCLOG("parse <spr> Node <w> Attr error at path:%s", dirPath.c_str());
            return;
        }
        unsigned int spr_w = atoi(attribute->value());
        
        attribute = dirOrSprNode->first_attribute("h");
        if (NULL == attribute) {
            CCLOG("parse <spr> Node <h> Attr error at path:%s", dirPath.c_str());
            return;
        }
        unsigned int spr_h = atoi(attribute->value());
        
        std::pair<std::string, Spr> item(dirPath + spr_name, Spr(dirPath + spr_name, spr_x, spr_y, spr_w, spr_h));
        _sprList.insert(item);
    }
	// <dir name=S>
    else if (strcmp(dirOrSprNode->name(), "dir") == 0) {
        xml_attribute<>* attribute = dirOrSprNode->first_attribute("name");
        if (NULL == attribute) {
            CCLOG("parse <dir> Node <name> Attr error at path:%s", dirPath.c_str());
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

unsigned int CCHierarchiesSpriteSheet::getImageWidth() {
    return _imageWidth;
}

unsigned int CCHierarchiesSpriteSheet::getImageHeight() {
    return _imageHeight;
}

bool CCHierarchiesSpriteSheet::getSpr (std::string name, Spr& out) {
    std::map<std::string, Spr>::iterator iter = _sprList.find(name);
    if (iter != _sprList.end()) {
        out = iter->second;
        return true;
    }
    return false;
}

NS_CC_EXT_END
