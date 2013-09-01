//
//  CCHierarchiesSpriteAnimationCache.m
//  CCHierarchiesSprite
//
//  Created by Xu Xiaocheng <xcxu@bread-media.com.cn>.
//  Copyright (c) 2012 Break-media. All rights reserved.
//

#include "CCHierarchiesSpriteAnimationCache.h"
#include "CCHierarchiesSpriteConfig.h"
#include "cocoa/CCString.h"
#include <sstream>

NS_CC_EXT_BEGIN

static CCHierarchiesSpriteAnimationCache* g_sharedHierarchiesSpriteAnimationCache = NULL;

CCHierarchiesSpriteAnimationCache* CCHierarchiesSpriteAnimationCache::sharedHierarchiesSpriteAnimationCache () {
	if (!g_sharedHierarchiesSpriteAnimationCache) {
        g_sharedHierarchiesSpriteAnimationCache = new CCHierarchiesSpriteAnimationCache();
        if (!g_sharedHierarchiesSpriteAnimationCache->init())
        {
            CC_SAFE_DELETE(g_sharedHierarchiesSpriteAnimationCache);
        }
    }
    return g_sharedHierarchiesSpriteAnimationCache;
}

void CCHierarchiesSpriteAnimationCache::purgeHierarchiesSpriteAnimationCache () {
	CC_SAFE_RELEASE_NULL(g_sharedHierarchiesSpriteAnimationCache);
}

CCHierarchiesSpriteAnimationCache::~CCHierarchiesSpriteAnimationCache () {
	SpriteAnimationCacheHashItem* hashItem, *tmp;
    HASH_ITER(hh, _animationCache, hashItem, tmp) {
        HASH_DEL(_animationCache, hashItem);
        delete hashItem;
    }
}

bool CCHierarchiesSpriteAnimationCache::init () {
	_animationCache = NULL;
	return true;
}

CCHierarchiesSpriteAnimation* CCHierarchiesSpriteAnimationCache::addAnimation (const char* name) {
	SpriteAnimationCacheHashItem* hashItem = NULL;
    HASH_FIND_STR(_animationCache, name, hashItem);
    if (!hashItem) {
        CCHierarchiesSpriteAnimation* animation = new CCHierarchiesSpriteAnimation(name);
        SpriteAnimationCacheHashItem* newCache = new SpriteAnimationCacheHashItem();
        int keyLen = strlen(name);
        newCache->name = (char*)calloc(keyLen + 1, sizeof(char)); // add 1 char for '\0'
        strcpy(newCache->name, name);
        newCache->retainCount = 1;
        newCache->animation = animation;
        HASH_ADD_KEYPTR(hh, _animationCache, newCache->name, keyLen, newCache);
        return animation;
    }
    else {
        hashItem->retainCount++;
        return hashItem->animation;
    }
}

CCHierarchiesSpriteAnimation* CCHierarchiesSpriteAnimationCache::getAnimation (const char* name) {
	SpriteAnimationCacheHashItem* hashItem = NULL;
    HASH_FIND_STR(_animationCache, name, hashItem);
    if (hashItem) {
        return hashItem->animation;
    }
    return NULL;
}

void CCHierarchiesSpriteAnimationCache::removeAnimation (const char* name) {
	SpriteAnimationCacheHashItem* hashItem = NULL;
    HASH_FIND_STR(_animationCache, name, hashItem);
    if (hashItem) {
        hashItem->retainCount--;
#if HIERARCHIES_SPRITE_ALWAY_CACHE_ANIMATION == 0
        if (hashItem->retainCount <= 0) {
            HASH_DEL(_animationCache, hashItem);
            delete hashItem;
        }
#endif
    }
}

const char* CCHierarchiesSpriteAnimationCache::description() {
	std::stringstream info;
    info << "CCHierarchiesAnimationCache:\n";
    info << "{\n";
    SpriteAnimationCacheHashItem* hashItem, *tmp;
    HASH_ITER(hh, _animationCache, hashItem, tmp) {
		info << "\t[" << hashItem->name << " : " << hashItem->retainCount << "]\n";
    }
	info << "}\n";
	return CCString::create(info.str())->getCString();
}

void CCHierarchiesSpriteAnimationCache::dumpInfo () {
	CCLOG("%s", description());
}

NS_CC_EXT_END
