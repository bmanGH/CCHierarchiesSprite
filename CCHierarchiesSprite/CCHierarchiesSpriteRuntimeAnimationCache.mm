//
//  CCHierarchiesSpriteRuntimeAnimationCache.m
//  HierarchiesSpriteViewer
//
//  Created by bman <zx123xz321hm3@hotmail.com>.
//  Copyright (c) 2013. All rights reserved.
//

#import "CCHierarchiesSpriteRuntimeAnimationCache.h"
#import "CCHierarchiesSpriteConfig.h"
#import "CCHierarchiesSprite.h"
#include <sstream>


@implementation CCHierarchiesSpriteRuntimeAnimationCache

static CCHierarchiesSpriteRuntimeAnimationCache* g_sharedHierarchiesSpriteRuntimeAnimationCache = nil;

+ (CCHierarchiesSpriteRuntimeAnimationCache*) sharedHierarchiesSpriteRuntimeAnimationCache {
	@synchronized(self)
	{
		if (!g_sharedHierarchiesSpriteRuntimeAnimationCache) {
			g_sharedHierarchiesSpriteRuntimeAnimationCache = [[CCHierarchiesSpriteRuntimeAnimationCache alloc] init];
		}
	}
    return g_sharedHierarchiesSpriteRuntimeAnimationCache;
}

+ (void) purgeHierarchiesSpriteRuntimeAnimationCache {
	@synchronized(self)
	{
		[g_sharedHierarchiesSpriteRuntimeAnimationCache release];
		g_sharedHierarchiesSpriteRuntimeAnimationCache = nil;
	}
}

+ (id) allocWithZone:(NSZone *)zone {
	@synchronized(self)
	{
		NSAssert(g_sharedHierarchiesSpriteRuntimeAnimationCache == nil, @"Attempted to allocate a second instance of a singleton.");
		g_sharedHierarchiesSpriteRuntimeAnimationCache = [super allocWithZone:zone];
		return g_sharedHierarchiesSpriteRuntimeAnimationCache;
	}
	return nil;
}

- (id) init {
    if ( (self = [super init]) ) {
        _animationCache = NULL;
        return self;
    }
    return nil;
}

- (void) dealloc {
    AnimationCacheHashItem* hashItem, *tmp;
    HASH_ITER(hh, _animationCache, hashItem, tmp) {
        HASH_DEL(_animationCache, hashItem);
        delete hashItem;
    }
	
	[super dealloc];
}

- (void) insertHierarchiesSprite:(CCHierarchiesSprite*)sprite {
	AnimationCacheHashItem* hashItem = NULL;
	HASH_FIND_STR(_animationCache, [sprite.animationName UTF8String], hashItem);
	if (!hashItem) {
		AnimationCacheHashItem* newCache = new AnimationCacheHashItem(sprite.animation->getFrameCount());
		int keyLen = strlen([sprite.animationName UTF8String]);
		newCache->name = (char*)calloc(keyLen + 1, sizeof(char)); // add 1 for char '\0'
		strcpy(newCache->name, [sprite.animationName UTF8String]);
		newCache->retainCount = 2; // + 1 for cache
		HASH_ADD_KEYPTR(hh, _animationCache, newCache->name, keyLen, newCache);
	}
	else {
		hashItem->retainCount++;
	}
}

- (void) removeHierarchiesSprite:(CCHierarchiesSprite*)sprite {
    AnimationCacheHashItem* hashItem = NULL;
    HASH_FIND_STR(_animationCache, [sprite.animationName UTF8String], hashItem);
    if (hashItem) {
        hashItem->retainCount--;
        if (hashItem->retainCount <= 0) {
            HASH_DEL(_animationCache, hashItem);
            delete hashItem;
        }
    }
}

- (void) removeUnusedHierarchiesSprite {
    AnimationCacheHashItem* hashItem, *tmp;
    HASH_ITER(hh, _animationCache, hashItem, tmp) {
        if (hashItem->retainCount == 1) {
            HASH_DEL(_animationCache, hashItem);
            delete hashItem;
        }
    }
}

- (NSString*) description
{
    NSMutableString* info = [NSMutableString string];
    [info appendString:@"CCHierarchiesSpriteRuntimeAnimationCache:\n"];
    [info appendString:@"{\n"];
	AnimationCacheHashItem* hashItem, *tmp;
    HASH_ITER(hh, _animationCache, hashItem, tmp) {
        [info appendFormat:@"\t[%s : %ld : %d]\n", hashItem->name, hashItem->cache.size(), hashItem->retainCount];
    }
    [info appendString:@"}"];
	return info;
}

@end
