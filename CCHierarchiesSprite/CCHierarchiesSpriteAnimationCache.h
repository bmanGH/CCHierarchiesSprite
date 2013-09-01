//
//  CCHierarchiesSpriteAnimationCache.h
//  CCHierarchiesSprite
//
//  Created by bman <zx123xz321hm3@hotmail.com>.
//  Copyright (c) 2013. All rights reserved.
//

#ifndef _CCHierarchiesSpriteAnimationCache_H_
#define _CCHierarchiesSpriteAnimationCache_H_

#import <Foundation/Foundation.h>
#include "uthash.h"
#include "CCHierarchiesSpriteAnimation.h"


@interface CCHierarchiesSpriteAnimationCache : NSObject {
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
    SpriteAnimationCacheHashItem* _animationCache;
}

+ (instancetype) sharedHierarchiesSpriteAnimationCache;
+ (void) purgeHierarchiesSpriteAnimationCache;

- (CCHierarchiesSpriteAnimation*) addAnimation:(NSString*)name;
- (CCHierarchiesSpriteAnimation*) getAnimation:(NSString*)name;
- (void) removeAnimation:(NSString*)name;
- (void) removeUnusedAnimation;

@end

#endif
