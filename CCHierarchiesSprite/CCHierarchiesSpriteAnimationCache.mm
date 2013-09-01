//
//  CCHierarchiesSpriteAnimationCache.m
//  CCHierarchiesSprite
//
//  Created by bman <zx123xz321hm3@hotmail.com>.
//  Copyright (c) 2013. All rights reserved.
//

#import "CCHierarchiesSpriteAnimationCache.h"
#import "CCHierarchiesSpriteConfig.h"
#include <sstream>


@implementation CCHierarchiesSpriteAnimationCache

static CCHierarchiesSpriteAnimationCache* g_sharedHierarchiesSpriteAnimationCache = nil;

+ (instancetype) sharedHierarchiesSpriteAnimationCache {
    @synchronized(self)
	{
		if (!g_sharedHierarchiesSpriteAnimationCache) {
			g_sharedHierarchiesSpriteAnimationCache = [[CCHierarchiesSpriteAnimationCache alloc] init];
		}
	}
    return g_sharedHierarchiesSpriteAnimationCache;
}

+ (void) purgeHierarchiesSpriteAnimationCache {
	@synchronized(self)
	{
		[g_sharedHierarchiesSpriteAnimationCache release];
		g_sharedHierarchiesSpriteAnimationCache = nil;
	}
}

+ (id) allocWithZone:(NSZone *)zone {
	@synchronized(self)
	{
		NSAssert(g_sharedHierarchiesSpriteAnimationCache == nil, @"Attempted to allocate a second instance of a singleton.");
		g_sharedHierarchiesSpriteAnimationCache = [super allocWithZone:zone];
		return g_sharedHierarchiesSpriteAnimationCache;
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
    SpriteAnimationCacheHashItem* hashItem, *tmp;
    HASH_ITER(hh, _animationCache, hashItem, tmp) {
        HASH_DEL(_animationCache, hashItem);
        delete hashItem;
    }
    
    [super dealloc];
}

- (CCHierarchiesSpriteAnimation*) addAnimation:(NSString*)name {
    SpriteAnimationCacheHashItem* hashItem = NULL;
    HASH_FIND_STR(_animationCache, [name UTF8String], hashItem);
    if (!hashItem) {
        CCHierarchiesSpriteAnimation* animation = new CCHierarchiesSpriteAnimation([name UTF8String]);
        SpriteAnimationCacheHashItem* newCache = new SpriteAnimationCacheHashItem();
        int keyLen = strlen([name UTF8String]);
        newCache->name = (char*)calloc(keyLen + 1, sizeof(char)); // add 1 char for '\0'
        strcpy(newCache->name, [name UTF8String]);
        newCache->retainCount = 2; // + 1 for cache
        newCache->animation = animation;
        HASH_ADD_KEYPTR(hh, _animationCache, newCache->name, keyLen, newCache);
        return animation;
    }
    else {
        hashItem->retainCount++;
        return hashItem->animation;
    }
}

- (CCHierarchiesSpriteAnimation*) getAnimation:(NSString*)name {
    SpriteAnimationCacheHashItem* hashItem = NULL;
    HASH_FIND_STR(_animationCache, [name UTF8String], hashItem);
    if (hashItem) {
        return hashItem->animation;
    }
    return NULL;
}

- (void) removeAnimation:(NSString*)name {
    SpriteAnimationCacheHashItem* hashItem = NULL;
    HASH_FIND_STR(_animationCache, [name UTF8String], hashItem);
    if (hashItem) {
        hashItem->retainCount--;
        if (hashItem->retainCount <= 0) {
            HASH_DEL(_animationCache, hashItem);
            delete hashItem;
        }
    }
}

- (void) removeUnusedAnimation {
    SpriteAnimationCacheHashItem* hashItem, *tmp;
    HASH_ITER(hh, _animationCache, hashItem, tmp) {
        if (hashItem->retainCount == 1) {
            HASH_DEL(_animationCache, hashItem);
            delete hashItem;
        }
    }
}

- (NSString*) description {
    NSMutableString* info = [NSMutableString string];
    [info appendString:@"CCHierarchiesAnimationCache:\n"];
    [info appendString:@"{\n"];
    SpriteAnimationCacheHashItem* hashItem, *tmp;
    HASH_ITER(hh, _animationCache, hashItem, tmp) {
        [info appendFormat:@"\t[%s : %d]\n", hashItem->name, hashItem->retainCount];
    }
    [info appendString:@"}\n"];
	return info;
}

@end

