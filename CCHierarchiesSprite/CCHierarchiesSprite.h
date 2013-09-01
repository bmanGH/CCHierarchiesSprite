//
//  CCHierarchiesSprite.h
//  CCHierarchiesSprite
//
//  version 0.1   12/22/2011
//  version 0.2   12/29/2011
//  version 0.3   12/31/2011
//  version 0.4   1/13/2012   support color transform
//  version 0.5   1/16/2012   support alpha transform and cook guide layer animation to key frames
//  version 0.6   2/3/2012    use item index to identification element instead of name; add library items' left and bottom properties
//  version 0.7   2/10/2012   add avatar, dyeing, event functions
//  version 0.8   2/27/2012   add single texture batch mode function
//  version 0.9   2/28/2012   add CCHierarchiesSpriteSheetCache, CCHierarchiesAnimationCache
//  version 0.9.2 3/22/2012   fix copy string without '\0' bug
//  version 0.10  3/23/2012   add multi-thread animation in batch mode
//  version 0.10.1   3/26/2012   export .sprites file and items image automatic
//  version 0.10.2   3/26/2012   chinese localization
//  version 0.10.3   3/26/2012   support set .sprites, .anims file, set batch after init
//  version 0.11     3/28/2012   support multi texture packer in batch mode
//  version 0.11.1   4/17/2012   support retina sprites file export
//  version 0.11.2   5/21/2012   hack export .sprites precision bug
//  version 0.12     6/5/2012    replace libxml2 with rapidxml
//  version 0.12.1   6/5/2012    add setAvatarTag and setAvatarTags to CCHierarchiesSprite class
//  version 0.12.2   6/6/2012    fix no-display bug in release build configure
//  version 0.13     6/11/2012   separate version between .hanims file format and runtime code
//  version 0.14   6/11/2012   add an config option that only support one element in keyframe for simplify and performance, so the depth property in element is also removed
//  version 0.14.1 6/21/2012   add batch z order scale and clear depth buffer between draw
//  version 0.14.5 6/25/2012   add set all texture params used by CCHierarchiesSpriteBatchNode
//  version 0.15   6/25/2012   draw sprites twice in CCHierarchiesBatchNode (1:draw opaque sprites by enable alpha test and enable depth write 2:draw transparent sprite by enable blend and disable depth write)
//  version 0.16   6/26/2012   simplify batch mode convert just by add child to batch node
//  version 0.16.1 6/27/2012   add z order include sprite in batch mode
//  version 0.16.5 6/27/2012   add hierarchies sprite's PositionTextureColorAlphaTest fragment shader
//  version 0.17   6/28/2012   add global runtime animation cache instead of runtime animation cache only in batch
//  version 0.17.1 7/9/2012    lerp frames only there are one element in both frames
//  version 0.18   3/28/2013   add configable feature lazy fresh
//  version 0.19   4/14/2013   upgrade to cocos2d-iphone v2.1
//  version 0.20   4/21/2013   support rotation sprites
//  version 0.20.1 7/6/2013    fix boundingBox with flip bug
//  version 0.20.2 7/19/2013   fix wrong anchor point with flip bug
//  version 0.20.3 8/1/2013    add 'eventContentCount' function
//
//  Created by bman <zx123xz321hm3@hotmail.com>.
//  Copyright (c) 2013. All rights reserved.
//

#ifndef _CCHierarchiesSprite_H_
#define _CCHierarchiesSprite_H_

#include "CCHierarchiesSpriteConfig.h"
#import "CCNode.h"
#import "CCTextureAtlas.h"
#import "CCActionInterval.h"
#ifdef HIERARCHIES_USE_CPP_11
#   include <unordered_map>
#else
#   include <map>
#endif
#include <string>
#include "CCHierarchiesSpriteSheet.h"
#include "CCHierarchiesSpriteAnimation.h"
#import "CCHierarchiesSpriteEventDelegate.h"

#define HIERARCHIES_SPRITE_SUPPORT_HANIMS_FILE_FORMAT_VERSION @"0.14"
#define HIERARCHIES_SPRITE_RUNTIME_VERSION @"0.20.3"


// CCHierarchiesSprite
@interface CCHierarchiesSprite : CCNodeRGBA<CCBlendProtocol, CCTextureProtocol> {
    CCHierarchiesSpriteSheet* _sheet;
    CCHierarchiesSpriteAnimation* _animation;
	
    NSString* _sheetName;
    NSString* _animationName;
	
    struct AvatarMapItem {
        std::string itemName;
        int itemIndex;
    };
#ifdef HIERARCHIES_USE_CPP_11
    std::unordered_map<std::string, AvatarMapItem> _avatarMap;
#else
    std::map<std::string, AvatarMapItem> _avatarMap;
#endif
    
    unsigned int _curFrameIndex;
	BOOL _needFresh;
    
    CCTextureAtlas* _texAtlas;
    unsigned int _quadsToDraw;
    
    ccBlendFunc _blendFunc;
	BOOL _opacityModifyRGB;
    
    BOOL _flipX;
    BOOL _flipY;
    CGRect _bbox;
    BOOL _enabledAvatar;
    
    id<CCHierarchiesSpriteEventDelegate> _delegate;
	
//    std::vector<ccV3F_C4B_T2F_Quad> _quads;
}

@property (nonatomic, readonly, assign) CCHierarchiesSpriteSheet* sheet; // c++ pointer
@property (nonatomic, readonly, assign) CCHierarchiesSpriteAnimation* animation; // c++ pointer
@property (nonatomic, readwrite, retain) CCTextureAtlas* textureAtlas;
@property (nonatomic, readwrite, assign) BOOL flipX;
@property (nonatomic, readwrite, assign) BOOL flipY;
@property (nonatomic, readonly, retain) NSString* sheetName;
@property (nonatomic, readonly, retain) NSString* animationName;
@property (nonatomic, readwrite, assign) id<CCHierarchiesSpriteEventDelegate> delegate; // weak ref
@property (nonatomic, readwrite, assign, getter=isEnabledAvator) BOOL enableAvatar;

+ (id) hierarchiesSpriteWithSpriteSheetFile:(NSString*)sheetFileName spriteAnimationFile:(NSString*)animationFileName delegate:(id<CCHierarchiesSpriteEventDelegate>)delegate;
+ (id) hierarchiesSpriteWithSpriteSheetFile:(NSString*)sheetFileName spriteAnimationFile:(NSString*)animationFileName;
- (id) initWithSpriteSheetFile:(NSString*)sheetFileName spriteAnimationFile:(NSString*)animationFileName delegate:(id<CCHierarchiesSpriteEventDelegate>)delegate;
- (id) initWithSpriteSheetFile:(NSString*)sheetFileName spriteAnimationFile:(NSString*)animationFileName;

- (void) setSpriteSheetFile:(NSString*)sheetFileName spriteAnimationFile:(NSString*)animationFileName;

- (unsigned int) eventContentCount:(NSString*)eventContent;

- (BOOL) displayFrameAtIndex:(NSUInteger)frameIndex;
- (BOOL) freshCurrentFrame;
- (unsigned int) currentFrameIndex;

- (BOOL) setAvatarMapWithSymbol:(NSString*)symbol toItem:(NSString*)item;
- (void) setAvatarTag:(NSString*)tagName;

@end


// CCHierarchiesAnimate
@interface CCHierarchiesAnimate : CCActionInterval {
    NSString* _animationName;
    NSString* _spriteAnimationName;
    CCHierarchiesSpriteAnimation* _spriteAnimation;
    CCHierarchiesSpriteAnimation::Animation _currentAnimation;
    unsigned int _curFrameIndex;
}

@property (nonatomic, readonly, copy) NSString* animationName;
@property (nonatomic, readonly, copy) NSString* spriteAnimationName;
@property (nonatomic, readonly, assign) CCHierarchiesSpriteAnimation* spriteAnimation; // c++ pointer
@property (nonatomic, readonly, assign) CCHierarchiesSpriteAnimation::Animation currentAnimation;

+ (id) actionWithHierarchiesAnimationName:(NSString*)animationName spriteAnimationName:(NSString*)spriteAnimationName;
- (id) initWithHierarchiesAnimationName:(NSString*)animationName spriteAnimationName:(NSString*)spriteAnimationName;

@end

#endif

