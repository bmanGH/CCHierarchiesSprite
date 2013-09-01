//
//  CCHierarchiesSpriteSheetCache.m
//  CCHierarchiesSprite
//
//  Created by bman <zx123xz321hm3@hotmail.com>.
//  Copyright (c) 2013. All rights reserved.
//

#import "CCHierarchiesSpriteSheetCache.h"
#import "CCHierarchiesSpriteConfig.h"
#include <sstream>


@implementation CCHierarchiesSpriteSheetCache

static CCHierarchiesSpriteSheetCache* g_sharedHierarchiesSpriteSheetCache = nil;

+ (instancetype) sharedHierarchiesSpriteSheetCache {
	@synchronized(self)
	{
		if (!g_sharedHierarchiesSpriteSheetCache) {
			g_sharedHierarchiesSpriteSheetCache = [[CCHierarchiesSpriteSheetCache alloc] init];
		}
	}
    return g_sharedHierarchiesSpriteSheetCache;
}

+ (void) purgeHierarchiesSpriteSheetCache {
	@synchronized(self)
	{
		[g_sharedHierarchiesSpriteSheetCache release];
		g_sharedHierarchiesSpriteSheetCache = nil;
	}
}

+ (id) allocWithZone:(NSZone *)zone {
	@synchronized(self)
	{
		NSAssert(g_sharedHierarchiesSpriteSheetCache == nil, @"Attempted to allocate a second instance of a singleton.");
		g_sharedHierarchiesSpriteSheetCache = [super allocWithZone:zone];
		return g_sharedHierarchiesSpriteSheetCache;
	}
	return nil;
}

- (id) init {
    if ( (self = [super init]) ) {
        _sheetCache = NULL;
        return self;
    }
    return nil;
}

- (void) dealloc {
    SheetCacheHashItem* hashItem, *tmp;
    HASH_ITER(hh, _sheetCache, hashItem, tmp) {
        HASH_DEL(_sheetCache, hashItem);
        delete hashItem;
    }
    
    [super dealloc];
}

- (CCHierarchiesSpriteSheet*) addSpriteSheet:(NSString*)name {
    SheetCacheHashItem* hashItem = NULL;
    HASH_FIND_STR(_sheetCache, [name UTF8String], hashItem);
    if (!hashItem) {
        CCHierarchiesSpriteSheet* sheet = new CCHierarchiesSpriteSheet([name UTF8String]);
        SheetCacheHashItem* newCache = new SheetCacheHashItem();
        int keyLen = strlen([name UTF8String]);
        newCache->name = (char*)calloc(keyLen + 1, sizeof(char)); // add 1 char for '\0'
        strcpy(newCache->name, [name UTF8String]);
        newCache->retainCount = 2; // + 1 for cache
        newCache->spriteSheet = sheet;
        HASH_ADD_KEYPTR(hh, _sheetCache, newCache->name, keyLen, newCache);
        return sheet;
    }
    else {
        hashItem->retainCount++;
        return hashItem->spriteSheet;
    }
}

- (CCHierarchiesSpriteSheet*) getSpriteSheet:(NSString*)name {
    SheetCacheHashItem* hashItem = NULL;
    HASH_FIND_STR(_sheetCache, [name UTF8String], hashItem);
    if (hashItem) {
        return hashItem->spriteSheet;
    }
    return NULL;
}

- (void) removeSpriteSheet:(NSString*)name {
    SheetCacheHashItem* hashItem = NULL;
    HASH_FIND_STR(_sheetCache, [name UTF8String], hashItem);
    if (hashItem) {
        hashItem->retainCount--;
        if (hashItem->retainCount <= 0) {
            HASH_DEL(_sheetCache, hashItem);
            delete hashItem;
        }
    }
}

- (void) removeUnusedSpriteSheet {
    SheetCacheHashItem* hashItem, *tmp;
    HASH_ITER(hh, _sheetCache, hashItem, tmp) {
        if (hashItem->retainCount == 1) {
            HASH_DEL(_sheetCache, hashItem);
            delete hashItem;
        }
    }
}

- (NSString*) description
{
    NSMutableString* info = [NSMutableString string];
    [info appendString:@"CCHierarchiesSpriteSheetCache:\n"];
    [info appendString:@"{\n"];
    SheetCacheHashItem* hashItem, *tmp;
    HASH_ITER(hh, _sheetCache, hashItem, tmp) {
        [info appendFormat:@"\t[%s : %d]\n", hashItem->name, hashItem->retainCount];
    }
    [info appendString:@"}"];
	return info;
}

@end
