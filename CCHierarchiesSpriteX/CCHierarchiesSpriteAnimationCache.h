//
//  CCHierarchiesSpriteAnimationCache.h
//  CCHierarchiesSprite
//
//  Created by Xu Xiaocheng <xcxu@bread-media.com.cn>.
//  Copyright (c) 2012 Break-media. All rights reserved.
//

#ifndef _CCHierarchiesSpriteAnimationCache_H_
#define _CCHierarchiesSpriteAnimationCache_H_

#include "cocoa/CCObject.h"
#include "ccMacros.h"
#include "ccTypes.h"
#include "support/data_support/uthash.h"
#include "CCHierarchiesSpriteAnimation.h"


NS_CC_EXT_BEGIN

class CCHierarchiesSpriteAnimationCache : public CCObject {
	
public:
	struct SpriteAnimationCacheHashItem {
        char* name;
        int retainCount;
        CCHierarchiesSpriteAnimation* animation;
        UT_hash_handle hh;
        
        SpriteAnimationCacheHashItem () {}
        ~SpriteAnimationCacheHashItem () {
            if (name != NULL)
                free(name);
            if (animation != NULL)
                delete animation;
        }
    };
	
protected:
    SpriteAnimationCacheHashItem* _animationCache;
	
public:
	static CCHierarchiesSpriteAnimationCache* sharedHierarchiesSpriteAnimationCache ();
	static void purgeHierarchiesSpriteAnimationCache ();
	
	virtual ~CCHierarchiesSpriteAnimationCache ();
	bool init ();
	
	CCHierarchiesSpriteAnimation* addAnimation (const char* name);
	CCHierarchiesSpriteAnimation* getAnimation (const char* name);
	void removeAnimation (const char* name);
	
	const char* description ();
	void dumpInfo ();
	
};

NS_CC_EXT_END

#endif
