//
//  CCHierarchiesSpriteSheetCache.h
//  CCHierarchiesSprite
//
//  Created by bman <zx123xz321hm3@hotmail.com>.
//  Copyright (c) 2013. All rights reserved.
//

#ifndef _CCHierarchiesSpriteSheetCache_H_
#define _CCHierarchiesSpriteSheetCache_H_

#import <Foundation/Foundation.h>
#include "CCHierarchiesSpriteSheet.h"
#include "uthash.h"


@interface CCHierarchiesSpriteSheetCache : NSObject {
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
    SheetCacheHashItem* _sheetCache;
}

+ (instancetype) sharedHierarchiesSpriteSheetCache;
+ (void) purgeHierarchiesSpriteSheetCache;

- (CCHierarchiesSpriteSheet*) addSpriteSheet:(NSString*)name;
- (CCHierarchiesSpriteSheet*) getSpriteSheet:(NSString*)name;
- (void) removeSpriteSheet:(NSString*)name;
- (void) removeUnusedSpriteSheet;

@end

#endif
