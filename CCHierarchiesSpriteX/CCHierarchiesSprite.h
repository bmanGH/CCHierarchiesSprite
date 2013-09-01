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
//  version 0.13     11/6/2012   separate version between .hanims file format and runtime code
//  version 0.14   11/6/2012   add an config option that only support one element in keyframe for simplify and performance, so the depth property in element is also removed
//  version 0.14.1 21/6/2012   add batch z order scale and clear depth buffer between draw
//  version 0.14.5 25/6/2012   add set all texture params used by CCHierarchiesSpriteBatchNode
//  version 0.15   25/6/2012   draw sprites twice in CCHierarchiesBatchNode (1:draw opaque sprites by enable alpha test and enable depth write 2:draw transparent sprite by enable blend and disable depth write)
//  version 0.16   26/6/2012   simplify batch mode convert just by add child to batch node
//  version 0.16.1 27/6/2012   add z order include sprite in batch mode
//  version 0.16.5 27/6/2012   add hierarchies sprite's PositionTextureColorAlphaTest fragment shader
//  version 0.17   28/6/2012   add global runtime animation cache instead of runtime animation cache only in batch
//  version 0.17.1 9/7/2012    lerp frames only there are one element in both frames
//  ======================================================================================
//  version 0.18x   30/8/2012   port from cocos2d-iphone to cocos2d-x without batch mode
//
//  Created by Xu Xiaocheng <xcxu@bread-media.com.cn>.
//  Copyright (c) 2012 Break-media. All rights reserved.
//

#ifndef _CCHierarchiesSprite_H_
#define _CCHierarchiesSprite_H_

#include "base_nodes/CCNode.h"
#include "CCProtocols.h"
#include "textures/CCTextureAtlas.h"
#include "ccTypes.h"
#include "actions/CCActionInterval.h"
#include "cocoa/CCZone.h"
#include <map>
#include <string>
#include "CCHierarchiesSpriteSheet.h"
#include "CCHierarchiesSpriteAnimation.h"
#include "CCHierarchiesSpriteEventDelegate.h"

#define HIERARCHIES_SPRITE_SUPPORT_HANIMS_FILE_FORMAT_VERSION "0.13"
#define HIERARCHIES_SPRITE_RUNTIME_VERSION "0.18x"


NS_CC_EXT_BEGIN

class CC_DLL CCHierarchiesSprite : public CCNode, public CCTextureProtocol, public CCRGBAProtocol {
	
	CC_SYNTHESIZE_READONLY(CCHierarchiesSpriteSheet*, _sheet, Sheet)
	CC_SYNTHESIZE_READONLY(CCHierarchiesSpriteAnimation*, _animation, Animation)
	
	CC_SYNTHESIZE_READONLY(CCTextureAtlas*, _texAtlas, TexAtlas)
	
	CC_SYNTHESIZE(bool, _flipX, FlipX)
	CC_SYNTHESIZE(bool, _flipY, FlipY)
	CC_SYNTHESIZE(CCHierarchiesSpriteEventDelegate*, _delegate, Delegate) // weak ref
	
protected:
	std::string _sheetName;
	std::string _animationName;
	
    struct DyeingAvatarData {
        std::string itemName;
        int itemIndex;
        ccColor3B dyeingColor;
    };
    std::map<std::string, DyeingAvatarData> _dyeingAvatarMap;
    
    unsigned int _curFrameIndex;
	unsigned int _nextFrameIndex;
    
	std::vector<ccV3F_C4B_T2F_Quad> _quads;
	
	ccBlendFunc _blendFunc;
	GLubyte _opacity;
	ccColor3B _color;
	ccColor3B _colorUnmodified;
	bool _opacityModifyRGB;
	
protected:
	void updateOpacityModifyRGB ();
	void updateBlendFunc ();
	void updateUnCachedDataInFrame ();
	
public:
    virtual ~CCHierarchiesSprite ();
	
	static CCHierarchiesSprite* create (const char* sheetFileName, const char* animationFileName, CCHierarchiesSpriteEventDelegate* delegate = NULL, const char* subPath = NULL);
	
	bool initWithFile (const char* sheetFileName, const char* animationFileName, CCHierarchiesSpriteEventDelegate* delegate = NULL, const char* subPath = NULL);
	
	bool setSpriteFile (const char* sheetFileName, const char* animationFileName, const char* subPath = NULL);
	
	const char* getSheetName ();
	const char* getAnimationName ();
	
	bool displayFrameAtIndex (unsigned int frameIndex);
	bool freshCurrentFrame ();
	
	bool setAvatarMap (const char* symbol, const char* item);
	void setAvatarTag (const char* tagName);
	void setAvatarTags (const char* firstTagName, ...);
	
	bool setDyeingWithColor(const char* symbol, ccColor3B dyeingColor);
	void setDyeingAllWithColor(ccColor3B dyeingColor);
	
	// implement CCBlendProtocol
	virtual void setBlendFunc (ccBlendFunc blendFunc);
	virtual ccBlendFunc getBlendFunc (void);
	
	// implement CCTextureProtocol
	virtual CCTexture2D* getTexture (void);
    virtual void setTexture (CCTexture2D *texture);
	
	// implement CCRGBAProtocol
    virtual const ccColor3B& getColor (void);
	virtual void setColor (const ccColor3B& color);
    virtual void setOpacity (GLubyte opacity);
    virtual GLubyte getOpacity (void);
    virtual bool isOpacityModifyRGB (void);
    virtual void setOpacityModifyRGB (bool bValue);
	
	virtual void draw ();
};

class CCHierarchiesAnimate : public CCActionInterval {
	
protected:
	CCHierarchiesSpriteAnimation* _animation; // weak ref
    std::string _animationName;
    CCHierarchiesSpriteAnimation::Animation _curAnimation;
	
public:
    static CCHierarchiesAnimate* create (const char* animationName, CCHierarchiesSpriteAnimation* animation);
    bool initWithHierarchiesAnimation (const char* animationName, CCHierarchiesSpriteAnimation* animation);
    
    const char* getAnimationName ();
    
    virtual bool isDone ();
    virtual CCObject* copyWithZone( CCZone* pZone);
    virtual void update (float time);
	
};

NS_CC_EXT_END

#endif

