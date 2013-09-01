//
//  CCHierarchiesSpriteRuntimeAnimationCache.m
//  HierarchiesSpriteViewer
//
//  Created by Xc Xu on 6/28/12.
//  Copyright (c) 2012 Break-medai. All rights reserved.
//

#include "CCHierarchiesSpriteRuntimeAnimationCache.h"
#include "CCHierarchiesSpriteConfig.h"
#include "CCHierarchiesSprite.h"
#include <sstream>

NS_CC_EXT_BEGIN

static CCHierarchiesSpriteRuntimeAnimationCache* g_sharedHierarchiesSpriteRuntimeAnimationCache = NULL;

CCHierarchiesSpriteRuntimeAnimationCache* CCHierarchiesSpriteRuntimeAnimationCache::sharedHierarchiesSpriteRuntimeAnimationCache () {
	if (!g_sharedHierarchiesSpriteRuntimeAnimationCache) {
        g_sharedHierarchiesSpriteRuntimeAnimationCache = new CCHierarchiesSpriteRuntimeAnimationCache();
        if (!g_sharedHierarchiesSpriteRuntimeAnimationCache->init())
        {
            CC_SAFE_DELETE(g_sharedHierarchiesSpriteRuntimeAnimationCache);
        }
    }
    return g_sharedHierarchiesSpriteRuntimeAnimationCache;
}

void CCHierarchiesSpriteRuntimeAnimationCache::purgeHierarchiesSpriteRuntimeAnimationCache () {
	CC_SAFE_RELEASE_NULL(g_sharedHierarchiesSpriteRuntimeAnimationCache);
}

CCHierarchiesSpriteRuntimeAnimationCache::~CCHierarchiesSpriteRuntimeAnimationCache () {
	AnimationCacheHashItem* hashItem, *tmp;
    HASH_ITER(hh, _animationCache, hashItem, tmp) {
        HASH_DEL(_animationCache, hashItem);
        delete hashItem;
    }
}

bool CCHierarchiesSpriteRuntimeAnimationCache::init () {
	_animationCache = NULL;
	return true;
}

void CCHierarchiesSpriteRuntimeAnimationCache::insertHierarchiesSprite (CCHierarchiesSprite* sprite) {
	AnimationCacheHashItem* hashItem = NULL;
	HASH_FIND_STR(_animationCache, sprite->getAnimationName(), hashItem);
	if (!hashItem) {
		AnimationCacheHashItem* newCache = new AnimationCacheHashItem(sprite->getAnimation()->getFrameCount());
		int keyLen = strlen(sprite->getAnimationName());
		newCache->name = (char*)calloc(keyLen + 1, sizeof(char)); // add 1 for char '\0'
		strcpy(newCache->name, sprite->getAnimationName());
		newCache->retainCount = 1;
		HASH_ADD_KEYPTR(hh, _animationCache, newCache->name, keyLen, newCache);
	}
	else {
		hashItem->retainCount++;
	}
}

void CCHierarchiesSpriteRuntimeAnimationCache::removeHierarchiesSprite (CCHierarchiesSprite* sprite) {
	AnimationCacheHashItem* hashItem = NULL;
    HASH_FIND_STR(_animationCache, sprite->getAnimationName(), hashItem);
    if (hashItem) {
        hashItem->retainCount--;
        if (hashItem->retainCount <= 0) {
            HASH_DEL(_animationCache, hashItem);
            delete hashItem;
        }
    }
}

const char* CCHierarchiesSpriteRuntimeAnimationCache::description () {
	std::stringstream info;
    info << "CCHierarchiesSpriteRuntimeAnimationCache:\n";
    info << "{\n";
	AnimationCacheHashItem* hashItem, *tmp;
    HASH_ITER(hh, _animationCache, hashItem, tmp) {
		info << "\t[" << hashItem->name << " : " << hashItem->cache.size() << " : " << hashItem->retainCount << "]\n";
    }
    info << "}\n";
	
	return CCString::create(info.str().c_str())->getCString();
}

void CCHierarchiesSpriteRuntimeAnimationCache::dumpInfo () {
	CCLog("%s", description());
}

NS_CC_EXT_END
