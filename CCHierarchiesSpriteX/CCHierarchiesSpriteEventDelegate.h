//
//  CCHierarchiesSpriteEventDelegate.h
//  CCHierarchiesSprite
//
//  Created by Xu Xiaocheng <xcxu@bread-media.com.cn>.
//  Copyright (c) 2012 Break-media. All rights reserved.
//

#ifndef _CCHierarchiesSpriteEventDelegatee_H_
#define _CCHierarchiesSpriteEventDelegatee_H_

#include "platform/CCPlatformMacros.h"
#include "cocos-ext.h"


NS_CC_EXT_BEGIN

class CCHierarchiesSprite;

class CC_DLL CCHierarchiesSpriteEventDelegate {
	
public:
	virtual void onEventContent (CCHierarchiesSprite* sprite, const char* eventContent) = 0;
	
};

NS_CC_EXT_END

#endif
