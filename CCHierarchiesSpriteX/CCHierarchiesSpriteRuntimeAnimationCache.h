//
//  CCHierarchiesSpriteRuntimeAnimationCache.h
//  HierarchiesSpriteViewer
//
//  Created by Xc Xu on 6/28/12.
//  Copyright (c) 2012 Break-medai. All rights reserved.
//

#ifndef _CCHierarchiesSpriteRuntimeAnimationCache_H_
#define _CCHierarchiesSpriteRuntimeAnimationCache_H_

#include <vector>
#include "cocoa/CCObject.h"
#include "ccMacros.h"
#include "ccTypes.h"
#include "support/data_support/uthash.h"
#include "cocos-ext.h"

NS_CC_EXT_BEGIN

class CCHierarchiesSprite;

class CCHierarchiesSpriteRuntimeAnimationCache : CCObject {
	
	friend class CCHierarchiesSprite;
	
public:
	struct FrameCacheItem {
		std::vector<ccV3F_C4B_T2F_Quad> frame;
		CCRect bbox;
		bool cached;
		
		FrameCacheItem () 
		: cached(false) {}
	};
	
	struct AnimationCacheHashItem {
		char* name;
		std::vector<FrameCacheItem> cache;
		int retainCount;
		UT_hash_handle hh;
		
		AnimationCacheHashItem () : name(NULL), retainCount(0) {}
		AnimationCacheHashItem (size_t n)
		: cache(n), name(NULL), retainCount(0) {
		}
		~AnimationCacheHashItem () {
			if (name != NULL)
				free(name);
		}
	};
	
protected:
	AnimationCacheHashItem* _animationCache;
	
public:
	static CCHierarchiesSpriteRuntimeAnimationCache* sharedHierarchiesSpriteRuntimeAnimationCache ();
	static void purgeHierarchiesSpriteRuntimeAnimationCache ();
	
	virtual ~CCHierarchiesSpriteRuntimeAnimationCache ();
	bool init ();
	
	void insertHierarchiesSprite (CCHierarchiesSprite* sprite);
	void removeHierarchiesSprite (CCHierarchiesSprite* sprite);
	
	const char* description ();
	void dumpInfo ();
	
};

NS_CC_EXT_END

#endif
