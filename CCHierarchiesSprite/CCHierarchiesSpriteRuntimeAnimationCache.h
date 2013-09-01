//
//  CCHierarchiesSpriteRuntimeAnimationCache.h
//  HierarchiesSpriteViewer
//
//  Created by bman <zx123xz321hm3@hotmail.com>.
//  Copyright (c) 2013. All rights reserved.
//

#ifndef _CCHierarchiesSpriteRuntimeAnimationCache_H_
#define _CCHierarchiesSpriteRuntimeAnimationCache_H_

#include <vector>
#import "ccTypes.h"
#include "uthash.h"


@class CCHierarchiesSprite;

struct FrameCacheItem {
    std::vector<ccV3F_C4B_T2F_Quad> frame;
    std::vector<int> avatarSymbolIndexList;
    CGRect bbox;
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
	: name(NULL), cache(n), retainCount(0) {
	}
    ~AnimationCacheHashItem () {
        if (name != NULL)
            free(name);
    }
};

@interface CCHierarchiesSpriteRuntimeAnimationCache : NSObject {
@public
    AnimationCacheHashItem* _animationCache; // public to CCHierarchiesSprite
}

+ (instancetype) sharedHierarchiesSpriteRuntimeAnimationCache;
+ (void) purgeHierarchiesSpriteRuntimeAnimationCache;

- (void) insertHierarchiesSprite:(CCHierarchiesSprite*)sprite;
- (void) removeHierarchiesSprite:(CCHierarchiesSprite*)sprite;
- (void) removeUnusedHierarchiesSprite;

@end

#endif
