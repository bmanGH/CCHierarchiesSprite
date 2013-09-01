//
//  CCHierarchiesSpriteEventDelegate.h
//  CCHierarchiesSprite
//
//  Created by bman <zx123xz321hm3@hotmail.com>.
//  Copyright (c) 2013. All rights reserved.
//

#ifndef _CCHierarchiesSpriteEventDelegatee_H_
#define _CCHierarchiesSpriteEventDelegatee_H_

#import <Foundation/Foundation.h>


@class CCHierarchiesSprite;

@protocol CCHierarchiesSpriteEventDelegate
    
- (void) onSprite:(CCHierarchiesSprite*)sprite withEventContent:(NSString*)eventContent;

@end

#endif
