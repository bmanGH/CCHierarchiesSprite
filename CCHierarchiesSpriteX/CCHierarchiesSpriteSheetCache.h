//
//  CCHierarchiesSpriteSheetCache.h
//  CCHierarchiesSprite
//
//  Created by Xu Xiaocheng <xcxu@bread-media.com.cn>.
//  Copyright (c) 2012 Break-media. All rights reserved.
//

#ifndef _CCHierarchiesSpriteSheetCache_H_
#define _CCHierarchiesSpriteSheetCache_H_

#include "CCHierarchiesSpriteSheet.h"
#include "ccMacros.h"
#include "cocoa/CCObject.h"
#include "support/data_support/uthash.h"

NS_CC_EXT_BEGIN

class CCHierarchiesSpriteSheetCache : CCObject {
    
public:
    struct SheetCacheHashItem {
        char* name;
        int retainCount;
        CCHierarchiesSpriteSheet* spriteSheet;
        UT_hash_handle hh;
        
        SheetCacheHashItem () {}
        ~SheetCacheHashItem () {
            if (name != NULL)
                free(name);
            if (spriteSheet != NULL)
                delete spriteSheet;
        }
    };
    
protected:
    SheetCacheHashItem* _sheetCache;
    
public:
    static CCHierarchiesSpriteSheetCache* sharedHierarchiesSpriteSheetCache ();
    static void purgeHierarchiesSpriteSheetCache ();
    
    virtual ~CCHierarchiesSpriteSheetCache ();
    bool init ();
    
    CCHierarchiesSpriteSheet* addSpriteSheet (const char* name);
    CCHierarchiesSpriteSheet* getSpriteSheet (const char* name);
    void removeSpriteSheet (const char* name);
    
    const char* description ();
    void dumpInfo ();
    
};

NS_CC_EXT_END

#endif
