//
//  CCHierarchiesSprite.m
//  CCHierarchiesSprite
//
//  Created by Xu Xiaocheng <xcxu@bread-media.com.cn>.
//  Copyright (c) 2012 Break-media. All rights reserved.
//

#include "CCHierarchiesSprite.h"
#include "CCHierarchiesSpriteConfig.h"
#include "CCHierarchiesSpriteSheetCache.h"
#include "CCHierarchiesSpriteAnimationCache.h"
#include "CCHierarchiesSpriteRuntimeAnimationCache.h"
#include <limits.h>
#include "shaders/CCShaderCache.h"
#include "kazmath/kazmath.h"
#include "kazmath/GL/mat4stack.h"
#include "kazmath/GL/matrix.h"


USING_NS_CC;
USING_NS_CC_EXT;

// CCHierarchiesSprite implement

inline static void hierarchiesCalcMatrix (CCHierarchiesSpriteAnimation::Item* item, CCHierarchiesSpriteAnimation::Element* element, CCAffineTransform* matrix) {
    FMatrix2D m0;
    FMatrix2D m1;
    
    m0.setAnchorX(element->anchorX - item->left);
    m0.setAnchorY(element->anchorY - item->bottom);
    
    m1.setScaleX(element->scaleX);
    m1.setScaleY(element->scaleY);
    m1.setSkewX(element->skewX);
    m1.setSkewY(element->skewY);
    m1.setTransformX(element->x);
    m1.setTransformY(element->y);
    
    FMatrix2D m = m0.concat(m1);
    
    *matrix = CCAffineTransformMake(m.a, -m.b, -m.c, m.d, m.tx, -m.ty);
}

inline static void hierarchiesUpdateQuadVertices (CCSize* size, CCAffineTransform* matrix, ccV3F_C4B_T2F_Quad* quad, float z) {
    float x1 = 0;
    float y1 = 0;
    
    float x2 = x1 + size->width;
    float y2 = y1 + size->height;
    float x = matrix->tx;
    float y = matrix->ty;
    
    float cr = matrix->a;
    float sr = matrix->b;
    float cr2 = matrix->d;
    float sr2 = -matrix->c;
    float ax = x1 * cr - y1 * sr2 + x;
    float ay = x1 * sr + y1 * cr2 + y;
    
    float bx = x2 * cr - y1 * sr2 + x;
    float by = x2 * sr + y1 * cr2 + y;
    
    float cx = x2 * cr - y2 * sr2 + x;
    float cy = x2 * sr + y2 * cr2 + y;
    
    float dx = x1 * cr - y2 * sr2 + x;
    float dy = x1 * sr + y2 * cr2 + y;
    
	//    if (flipX) {
	//        ax = -ax;
	//        bx = -bx;
	//        dx = -dx;
	//        cx = -cx;
	//    }
	//    if (flipY) {
	//        ay = -ay;
	//        by = -by;
	//        dy = -dy;
	//        cy = -cy;
	//    }
    
	quad->bl.vertices = (ccVertex3F) { ax, ay, z };
    quad->br.vertices = (ccVertex3F) { bx, by, z };
    quad->tl.vertices = (ccVertex3F) { dx, dy, z };
    quad->tr.vertices = (ccVertex3F) { cx, cy, z };
}

inline static void hierarchiesUpdateQuadTextureCoords (CCRect* rect, float texWidth, float texHeight, ccV3F_C4B_T2F_Quad* quad) {
    float left, right, top, bottom;
    
#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
    left	= (2*rect->origin.x+1)/(2*texWidth);
    right	= left + (rect.size.width*2-2)/(2*texWidth);
    top		= (2*rect->origin.y+1)/(2*texHeight);
    bottom	= top + (rect->size.height*2-2)/(2*texHeight);
#else
    left	= rect->origin.x/texWidth;
    right	= left + rect->size.width/texWidth;
    top		= rect->origin.y/texHeight;
    bottom	= top + rect->size.height/texHeight;
#endif // ! CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
    
	//    if(flipX)
	//        CC_SWAP(left,right);
	//    if(flipY)
	//        CC_SWAP(top,bottom);
    
    quad->bl.texCoords.u = left;
    quad->bl.texCoords.v = bottom;
    quad->br.texCoords.u = right;
    quad->br.texCoords.v = bottom;
    quad->tl.texCoords.u = left;
    quad->tl.texCoords.v = top;
    quad->tr.texCoords.u = right;
    quad->tr.texCoords.v = top;
}

//NEEDFIX: the effect of color amount is not right 
inline static void hierarchiesUpdateQuadTextureColorFromAnimation (float alpha_percent, int alpha_amount, float red_percent, int red_amount, float green_percent, int green_amount, float blue_percent, int blue_amount, ccV3F_C4B_T2F_Quad* quad) {
    int value;
    
    value = (255 * alpha_percent + alpha_amount);
    if (value < 0)
        value = 0;
    else if (value > 255)
        value = 255;
    quad->bl.colors.a = value;
    quad->br.colors.a = value;
    quad->tl.colors.a = value;
    quad->tr.colors.a = value;
    
    value = (255 * red_percent + red_amount);
    if (value < 0)
        value = 0;
    else if (value > 255)
        value = 255;
    quad->bl.colors.r = value;
    quad->br.colors.r = value;
    quad->tl.colors.r = value;
    quad->tr.colors.r = value;
    
    value = (255 * green_percent + green_amount);
    if (value < 0)
        value = 0;
    else if (value > 255)
        value = 255;
    quad->bl.colors.g = value;
    quad->br.colors.g = value;
    quad->tl.colors.g = value;
    quad->tr.colors.g = value;
    
    value = (255 * blue_percent + blue_amount);
    if (value < 0)
        value = 0;
    else if (value > 255)
        value = 255;
    quad->bl.colors.b = value;
    quad->br.colors.b = value;
    quad->tl.colors.b = value;
    quad->tr.colors.b = value;
}

inline static void hierarchiesUpdateQuadTextureColor (bool opacityModifyRGB, int opacity, int color_r, int color_g, int color_b, ccV3F_C4B_T2F_Quad* quad) {
	// If opacityModifyRGB is NO then opacity will be applied as: glColor(R,G,B,opacity);
	// If opacityModifyRGB is YES then oapcity will be applied as: glColor(opacity, opacity, opacity, opacity );
	if (opacityModifyRGB) {
		quad->bl.colors.a = quad->bl.colors.a * (opacity / 255.0f);
		quad->br.colors.a = quad->br.colors.a * (opacity / 255.0f);
		quad->tl.colors.a = quad->tl.colors.a * (opacity / 255.0f);
		quad->tr.colors.a = quad->tr.colors.a * (opacity / 255.0f);
		
		quad->bl.colors.r = quad->bl.colors.r * (color_r / 255.0f) * (opacity / 255.0f);
		quad->br.colors.r = quad->br.colors.r * (color_r / 255.0f) * (opacity / 255.0f);
		quad->tl.colors.r = quad->tl.colors.r * (color_r / 255.0f) * (opacity / 255.0f);
		quad->tr.colors.r = quad->tr.colors.r * (color_r / 255.0f) * (opacity / 255.0f);
		
		quad->bl.colors.g = quad->bl.colors.g * (color_g / 255.0f) * (opacity / 255.0f);
		quad->br.colors.g = quad->br.colors.g * (color_g / 255.0f) * (opacity / 255.0f);
		quad->tl.colors.g = quad->tl.colors.g * (color_g / 255.0f) * (opacity / 255.0f);
		quad->tr.colors.g = quad->tr.colors.g * (color_g / 255.0f) * (opacity / 255.0f);
		
		quad->bl.colors.b = quad->bl.colors.b * (color_b / 255.0f) * (opacity / 255.0f);
		quad->br.colors.b = quad->br.colors.b * (color_b / 255.0f) * (opacity / 255.0f);
		quad->tl.colors.b = quad->tl.colors.b * (color_b / 255.0f) * (opacity / 255.0f);
		quad->tr.colors.b = quad->tr.colors.b * (color_b / 255.0f) * (opacity / 255.0f);
	}
	else {
		quad->bl.colors.a = quad->bl.colors.a * (opacity / 255.0f);
		quad->br.colors.a = quad->br.colors.a * (opacity / 255.0f);
		quad->tl.colors.a = quad->tl.colors.a * (opacity / 255.0f);
		quad->tr.colors.a = quad->tr.colors.a * (opacity / 255.0f);
		
		quad->bl.colors.r = quad->bl.colors.r * (color_r / 255.0f);
		quad->br.colors.r = quad->br.colors.r * (color_r / 255.0f);
		quad->tl.colors.r = quad->tl.colors.r * (color_r / 255.0f);
		quad->tr.colors.r = quad->tr.colors.r * (color_r / 255.0f);
		
		quad->bl.colors.g = quad->bl.colors.g * (color_g / 255.0f);
		quad->br.colors.g = quad->br.colors.g * (color_g / 255.0f);
		quad->tl.colors.g = quad->tl.colors.g * (color_g / 255.0f);
		quad->tr.colors.g = quad->tr.colors.g * (color_g / 255.0f);
		
		quad->bl.colors.b = quad->bl.colors.b * (color_b / 255.0f);
		quad->br.colors.b = quad->br.colors.b * (color_b / 255.0f);
		quad->tl.colors.b = quad->tl.colors.b * (color_b / 255.0f);
		quad->tr.colors.b = quad->tr.colors.b * (color_b / 255.0f);
	}
}

inline static void hierarchiesExpandRectByPoint (float* minX, float* maxX, float* minY, float* maxY, float* pX, float* pY) {
    if (*pX < *minX) {
        *minX = *pX;
    }
    if (*pX > *maxX) {
        *maxX = *pX;
    }
    
    if (*pY < *minY) {
        *minY = *pY;
    }
    if (*pY > *maxY) {
        *maxY = *pY;
    }
}

//inline static void hierarchiesFlipQuadVertices (bool flipX, bool flipY, ccV3F_C4B_T2F_Quad* quad) {
//    if (flipX) {
//        quad->bl.vertices.x = -quad->bl.vertices.x;
//        quad->br.vertices.x = -quad->br.vertices.x;
//        quad->tl.vertices.x = -quad->tl.vertices.x;
//        quad->tr.vertices.x = -quad->tr.vertices.x;
//    }
//    if (flipY) {
//        quad->bl.vertices.y = -quad->bl.vertices.y;
//        quad->br.vertices.y = -quad->br.vertices.y;
//        quad->tl.vertices.y = -quad->tl.vertices.y;
//        quad->tr.vertices.y = -quad->tr.vertices.y;
//    }
//}

inline static float float_round (float r) {
    return (r > 0.0f) ? floorf(r + 0.5f) : ceilf(r - 0.5f);
}


NS_CC_EXT_BEGIN

CCHierarchiesSprite::~CCHierarchiesSprite () {
	CC_SAFE_RELEASE(_texAtlas);
	
#if HIERARCHIES_SPRITE_CACHE_RUNTIME_ANIMATION == 1
	CCHierarchiesSpriteRuntimeAnimationCache::sharedHierarchiesSpriteRuntimeAnimationCache()->removeHierarchiesSprite(this);
#endif
    
#if HIERARCHIES_SPRITE_CACHE_ANIMATION == 1
	CCHierarchiesSpriteAnimationCache::sharedHierarchiesSpriteAnimationCache()->removeAnimation(_animationName.c_str());
#else
	delete _animation;
#endif
	
#if HIERARCHIES_SPRITE_CACHE_SPRITE_SHEET == 1
    CCHierarchiesSpriteSheetCache::sharedHierarchiesSpriteSheetCache()->removeSpriteSheet(_sheetName.c_str());
#else
	delete _sheet;
#endif
}

CCHierarchiesSprite* CCHierarchiesSprite::create (const char* sheetFileName, const char* animationFileName, CCHierarchiesSpriteEventDelegate* delegate, const char* subPath) {
	CCHierarchiesSprite* ret = new CCHierarchiesSprite();
	if (ret->initWithFile(sheetFileName, animationFileName, delegate, subPath))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return NULL;
}

bool CCHierarchiesSprite::initWithFile (const char* sheetFileName, const char* animationFileName, CCHierarchiesSpriteEventDelegate* delegate, const char* subPath) {
#if HIERARCHIES_SPRITE_CACHE_SPRITE_SHEET == 1
    _sheet = CCHierarchiesSpriteSheetCache::sharedHierarchiesSpriteSheetCache()->addSpriteSheet(sheetFileName);
#else
	_sheet = new CCHierarchiesSpriteSheet(sheetFileName);
#endif
	
#if HIERARCHIES_SPRITE_CACHE_ANIMATION == 1
	_animation = CCHierarchiesSpriteAnimationCache::sharedHierarchiesSpriteAnimationCache()->addAnimation(animationFileName);
#else
	_animation = new CCHierarchiesSpriteAnimation(animationFileName);
#endif
	
	setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor));
	
	_texAtlas = new CCTextureAtlas();
	std::string imagePath(subPath);
	imagePath.append(_sheet->getImageName().c_str());
	
	if (!_texAtlas->initWithFile(imagePath.c_str(), HIERARCHIES_SPRITE_DEFAULT_TEXTURE_ATLAS_CAPACITY)) {
		CC_SAFE_RELEASE(this);
		return false;
	}
	
	_opacity = 255;
	_color = _colorUnmodified = ccWHITE;
	_opacityModifyRGB = false;
	
	_blendFunc.src = GL_SRC_ALPHA;
	_blendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
	
	updateBlendFunc();
	updateOpacityModifyRGB();
	
	_flipX = false;
	_flipY = false;
	
	for (int i = 0; i < _animation->getSymbolCount(); i++) {
		CCHierarchiesSpriteAnimation::Symbol symbol;
		_animation->getSymbolByIndex(i, symbol);
		CCHierarchiesSpriteAnimation::Item item;
		_animation->getItemByIndex(symbol.defaultItemIndex, item);
		_dyeingAvatarMap[symbol.name].itemName = item.name;
		_dyeingAvatarMap[symbol.name].itemIndex = symbol.defaultItemIndex;
		_dyeingAvatarMap[symbol.name].dyeingColor = ccWHITE;
	}
	
	_delegate = delegate;
	
	_curFrameIndex = INT_MAX;
	
	_sheetName = sheetFileName;
	_animationName = animationFileName;
	
#if HIERARCHIES_SPRITE_CACHE_RUNTIME_ANIMATION == 1
	CCHierarchiesSpriteRuntimeAnimationCache::sharedHierarchiesSpriteRuntimeAnimationCache()->insertHierarchiesSprite(this);
#endif
	
	displayFrameAtIndex(0);
	
	return true;
}

bool CCHierarchiesSprite::setSpriteFile (const char* sheetFileName, const char* animationFileName, const char* subPath) {
#if HIERARCHIES_SPRITE_CACHE_RUNTIME_ANIMATION == 1
	CCHierarchiesSpriteRuntimeAnimationCache::sharedHierarchiesSpriteRuntimeAnimationCache()->removeHierarchiesSprite(this);
#endif
	
#if HIERARCHIES_SPRITE_CACHE_SPRITE_SHEET == 1
    CCHierarchiesSpriteSheetCache::sharedHierarchiesSpriteSheetCache()->removeSpriteSheet(_sheetName.c_str());
	_sheet = NULL;
    _sheet = CCHierarchiesSpriteSheetCache::sharedHierarchiesSpriteSheetCache()->addSpriteSheet(sheetFileName);
#else
	CC_SAFE_DELETE(_sheet);
	_sheet = new CCHierarchiesSpriteSheet(sheetFileName);
#endif
	
#if HIERARCHIES_SPRITE_CACHE_ANIMATION == 1
	CCHierarchiesSpriteAnimationCache::sharedHierarchiesSpriteAnimationCache()->removeAnimation(_animationName.c_str());
	_animation = NULL;
	_animation = CCHierarchiesSpriteAnimationCache::sharedHierarchiesSpriteAnimationCache()->addAnimation(animationFileName);
#else
	CC_SAFE_DELETE(_animation);
	_animation = new CCHierarchiesSpriteAnimation(animationFileName);
#endif
	
	_sheetName = sheetFileName;
	_animationName = animationFileName;
	
	_quads.clear();
	CC_SAFE_RELEASE_NULL(_texAtlas);
	_texAtlas = new CCTextureAtlas();
	if (!_texAtlas->initWithFile(_sheet->getImageName().c_str(), HIERARCHIES_SPRITE_DEFAULT_TEXTURE_ATLAS_CAPACITY))
		return false;
	
#if HIERARCHIES_SPRITE_CACHE_RUNTIME_ANIMATION == 1
	CCHierarchiesSpriteRuntimeAnimationCache::sharedHierarchiesSpriteRuntimeAnimationCache()->insertHierarchiesSprite(this);
#endif
	
	_dyeingAvatarMap.clear();
	for (int i = 0; i < _animation->getSymbolCount(); i++) {
		CCHierarchiesSpriteAnimation::Symbol symbol;
		_animation->getSymbolByIndex(i, symbol);
		CCHierarchiesSpriteAnimation::Item item;
		_animation->getItemByIndex(symbol.defaultItemIndex, item);
		_dyeingAvatarMap[symbol.name].itemName = item.name;
		_dyeingAvatarMap[symbol.name].itemIndex = symbol.defaultItemIndex;
		_dyeingAvatarMap[symbol.name].dyeingColor = ccWHITE;
	}
	
	_curFrameIndex = INT_MAX;
	
	displayFrameAtIndex(0);
	
	return true;
}

const char* CCHierarchiesSprite::getSheetName () {
	return _sheetName.c_str();
}

const char* CCHierarchiesSprite::getAnimationName () {
	return _animationName.c_str();
}

bool CCHierarchiesSprite::displayFrameAtIndex (unsigned int frameIndex) {
	if (_curFrameIndex == frameIndex)
        return true;
	
#if HIERARCHIES_SPRITE_ENABLE_EVENT_IN_ANIMATION == 1
	if (_curFrameIndex != frameIndex) {
		// event dispatch
		if (_delegate) {
			CCHierarchiesSpriteAnimation::Event event;
			if (_animation->getEventByFrameId(frameIndex, event)) {
				_delegate->onEventContent(this, event.content.c_str());
			}
		}
	}
#endif
	
#if HIERARCHIES_SPRITE_CACHE_RUNTIME_ANIMATION == 1
    // if animation data is cached than read cached data
	CCHierarchiesSpriteRuntimeAnimationCache::AnimationCacheHashItem* cacheItem = NULL;
	HASH_FIND_STR(CCHierarchiesSpriteRuntimeAnimationCache::sharedHierarchiesSpriteRuntimeAnimationCache()->_animationCache, _animationName.c_str(), cacheItem);
	if (cacheItem) {
		if (frameIndex < cacheItem->cache.size() && cacheItem->cache[frameIndex].cached) {
			_curFrameIndex = frameIndex;
			
			_quads = cacheItem->cache[frameIndex].frame;
			CCRect bbox = cacheItem->cache[frameIndex].bbox;
			
			CCPoint anchorPoint = CCPointMake( (0 - bbox.origin.x) / bbox.size.width, 
											  (0 - bbox.origin.y) / bbox.size.height );
			setAnchorPoint(anchorPoint);
			setContentSize(bbox.size);
			
			updateUnCachedDataInFrame();
			
			return true;
		}
		// else -> calc frame quads and insert to cache
	}
	else {
		CCLOG("<CCHierarchiesSprite> [%s] no cache item", _animationName.c_str());
		return false;
	}
#endif
    
    CCHierarchiesSpriteAnimation::FrameElements frameElements;
    int eNum = _animation->getFrameElementsAtIndex(frameIndex, frameElements);
    
    if (eNum == -1) {
		CCLOG("<CCHierarchiesSprite> [%s] no elements at frame %d", _animationName.c_str(), frameIndex);
        return false;
	}
    
    _curFrameIndex = frameIndex;
    
	_quads.clear();
#if HIERARCHIES_SPRITE_ENABLE_DYEING == 1
	if ((eNum * 2) > _quads.capacity())
		_quads.reserve(eNum * 2);
#else
	if ((eNum) > _quads.capacity())
		_quads.reserve(eNum);
#endif
    
    float min_X = 10000, max_X = -10000, min_Y = 10000, max_Y = -10000;
    
    CCAffineTransform matrix;
    ccV3F_C4B_T2F_Quad quad;
	bool result = false;
    CCHierarchiesSpriteAnimation::FrameElements::reverse_iterator layerIter;
	int layerZOrder = 0;
    for (layerIter = frameElements.rbegin(); layerIter != frameElements.rend(); layerIter++) {
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
        std::vector<CCHierarchiesSpriteAnimation::Element>::iterator elementIter;
        for (elementIter = layerIter->begin(); elementIter != layerIter->end(); elementIter++)
#else
		HierarchiesSpriteAnimation::Element* elementIter = &(*layerIter);
#endif
		{
			CCHierarchiesSpriteAnimation::Symbol symbol;
			result = _animation->getSymbolByIndex(elementIter->symbolIndex, symbol);
			assert(result);
			
			DyeingAvatarData dyeingAvatarData;
			dyeingAvatarData = _dyeingAvatarMap[symbol.name];
			
			CCHierarchiesSpriteAnimation::Item item;
			result = _animation->getItemByIndex(dyeingAvatarData.itemIndex, item);
			assert(result);
			
			CCHierarchiesSpriteSheet::Spr spr;
			result = _sheet->getSpr(dyeingAvatarData.itemName, spr);
			assert(result);
			
			// calc matrix
			hierarchiesCalcMatrix(&item, &(*elementIter), &matrix);
			
			// update vertices
			CCSize size = CCSizeMake(spr.w, spr.h);
			
			hierarchiesUpdateQuadVertices(&size, &matrix, &quad, layerZOrder * HIERARCHIES_SPRITE_LAYER_Z_ORDER_SCALE);
			
			// update color from animation
			hierarchiesUpdateQuadTextureColorFromAnimation (elementIter->color_alpha_percent, elementIter->color_alpha_amount,
															elementIter->color_red_percent, elementIter->color_red_amount,
															elementIter->color_green_percent, elementIter->color_green_amount,
															elementIter->color_blue_percent, elementIter->color_blue_amount,
															&quad);
			
			hierarchiesExpandRectByPoint(&min_X, &max_X, &min_Y, &max_Y, &quad.bl.vertices.x, &quad.bl.vertices.y);
			hierarchiesExpandRectByPoint(&min_X, &max_X, &min_Y, &max_Y, &quad.br.vertices.x, &quad.br.vertices.y);
			hierarchiesExpandRectByPoint(&min_X, &max_X, &min_Y, &max_Y, &quad.tl.vertices.x, &quad.tl.vertices.y);
			hierarchiesExpandRectByPoint(&min_X, &max_X, &min_Y, &max_Y, &quad.tr.vertices.x, &quad.tr.vertices.y);
			
#if HIERARCHIES_SPRITE_ENABLE_AVATER == 1
			quad.bl.texCoords.u = (GLfloat)1; // use not 1 means this quad is not a dyeing part
			quad.bl.texCoords.v = 0;
			quad.br.texCoords.u = (GLfloat)elementIter->symbolIndex; // cache symbol index
			quad.br.texCoords.v = 0;
			quad.tl.texCoords.u = 0;
			quad.tl.texCoords.v = 0;
			quad.tr.texCoords.u = 0;
			quad.tr.texCoords.v = 0;
#else
			CCRect rect = CCRectMake(spr.x, spr.y, spr.w, spr.h);
			hierarchiesUpdateQuadTextureCoords(&rect, _sheet->getImageWidth(), _sheet->getImageHeight(), &quad);
#endif
			
			_quads.push_back(quad);
			
#if HIERARCHIES_SPRITE_ENABLE_DYEING == 1
			// dyeing part
			if (item.dyeingPartItemIndex != -1) {
				_animation->getItemByIndex(item.dyeingPartItemIndex, item);
				result = _sheet->getSpr(item.name, spr);
				assert(result);
				
				// calc matrix
				hierarchiesCalcMatrix(&item, &(*elementIter), &matrix);
				
				// update vertices
				CCSize size = CCSizeMake(spr.w, spr.h);
				
				// z order should correct include sprite in batch mode
				hierarchiesUpdateQuadVertices(&size, &matrix, &quad, layerZOrder * HIERARCHIES_SPRITE_LAYER_Z_ORDER_SCALE + HIERARCHIES_SPRITE_LAYER_Z_ORDER_SCALE * 0.5f);
				
				// update color from animation
				hierarchiesUpdateQuadTextureColorFromAnimation (elementIter->color_alpha_percent, elementIter->color_alpha_amount,
																elementIter->color_red_percent, elementIter->color_red_amount,
																elementIter->color_green_percent, elementIter->color_green_amount,
																elementIter->color_blue_percent, elementIter->color_blue_amount,
																&quad);
				
				hierarchiesExpandRectByPoint(&min_X, &max_X, &min_Y, &max_Y, &quad.bl.vertices.x, &quad.bl.vertices.y);
				hierarchiesExpandRectByPoint(&min_X, &max_X, &min_Y, &max_Y, &quad.br.vertices.x, &quad.br.vertices.y);
				hierarchiesExpandRectByPoint(&min_X, &max_X, &min_Y, &max_Y, &quad.tl.vertices.x, &quad.tl.vertices.y);
				hierarchiesExpandRectByPoint(&min_X, &max_X, &min_Y, &max_Y, &quad.tr.vertices.x, &quad.tr.vertices.y);
				
				quad.bl.texCoords.u = (GLfloat)0; // use 0 means this quad is a dyeing part
				quad.bl.texCoords.v = 0;
				quad.br.texCoords.u = (GLfloat)elementIter->symbolIndex; // cache symbol index
				quad.br.texCoords.v = 0;
				quad.tl.texCoords.u = 0;
				quad.tl.texCoords.v = 0;
				quad.tr.texCoords.u = 0;
				quad.tr.texCoords.v = 0;
				
				_quads.push_back(quad);
			}
#endif
		}
		layerZOrder++;
	}
		
	CCRect bbox = CCRectMake(min_X, min_Y, max_X - min_X, max_Y - min_Y);
	CCPoint anchorPoint = CCPointMake( (0 - bbox.origin.x) / bbox.size.width, 
									  (0 - bbox.origin.y) / bbox.size.height );
	setAnchorPoint(anchorPoint);
	setContentSize(bbox.size);
	
#if HIERARCHIES_SPRITE_CACHE_RUNTIME_ANIMATION == 1
	// insert animation frame data to cache
	cacheItem->cache[frameIndex].frame = _quads;
	cacheItem->cache[frameIndex].bbox = bbox;
	cacheItem->cache[frameIndex].cached = true;
#endif
	
	updateUnCachedDataInFrame();
	
	return true;
}

void CCHierarchiesSprite::updateUnCachedDataInFrame () {
    bool ignoreColor = (_opacity == 255 && _color.r == 255 && _color.g == 255 && _color.b == 255);
    
#if (HIERARCHIES_SPRITE_ENABLE_AVATER == 0) && (HIERARCHIES_SPRITE_ENABLE_DYEING == 0)
    if (ignoreColor)
        return;
    
    std::vector<ccV3F_C4B_T2F_Quad>::iterator quadIter;
	for (quadIter = _quads.begin(); quadIter != _quads.end(); quadIter++) {
        //		hierarchiesFlipQuadVertices(_flipX, _flipX, &(*quadIter));
		hierarchiesUpdateQuadTextureColor(_opacityModifyRGB, _opacity, _color.r, _color.g, _color.b,
										  &(*quadIter));
    }
#else
    std::vector<ccV3F_C4B_T2F_Quad>::iterator quadIter;
	for (quadIter = _quads.begin(); quadIter != _quads.end(); quadIter++) {
		bool result = false;
		
        //		hierarchiesFlipQuadVertices(_flipX, _flipY, &(*quadIter));
		
		CCHierarchiesSpriteAnimation::Symbol symbol;
		result = _animation->getSymbolByIndex((int)quadIter->br.texCoords.u, symbol);
		assert(result);
		
		DyeingAvatarData dyeingAvatarData;
		dyeingAvatarData = _dyeingAvatarMap[symbol.name];
		
		CCHierarchiesSpriteAnimation::Item item;
		result = _animation->getItemByIndex(dyeingAvatarData.itemIndex, item);
		assert(result);
		
		// update avater tex coords, opacity and color vertices
		if (quadIter->bl.texCoords.u != 0) {
#	if HIERARCHIES_SPRITE_ENABLE_AVATER == 1
			CCHierarchiesSpriteSheet::Spr spr;
			result = _sheet->getSpr(dyeingAvatarData.itemName, spr);
			assert(result);
			
			CCRect rect = CCRectMake(spr.x, spr.y, spr.w, spr.h);
			hierarchiesUpdateQuadTextureCoords(&rect, _sheet->getImageWidth(), _sheet->getImageHeight(), &(*quadIter));
#	endif
			
            if (!ignoreColor)
                hierarchiesUpdateQuadTextureColor(_opacityModifyRGB, _opacity, _color.r, _color.g, _color.b,
											  &(*quadIter));
		}
		else {
#	if HIERARCHIES_SPRITE_ENABLE_DYEING == 1
			result = _animation->getItemByIndex(item.dyeingPartItemIndex, item);
			assert(result);
			
			CCHierarchiesSpriteSheet::Spr spr;
			result = _sheet->getSpr(item.name, spr);
			assert(result);
			
			CCRect rect = CCRectMake(spr.x, spr.y, spr.w, spr.h);
			hierarchiesUpdateQuadTextureCoords(&rect, _sheet->getImageWidth(), _sheet->getImageHeight(), &(*quadIter));
			
			hierarchiesUpdateQuadTextureColor(_opacityModifyRGB,
											  _opacity,
											  dyeingAvatarData.dyeingColor.r,
											  dyeingAvatarData.dyeingColor.g,
											  dyeingAvatarData.dyeingColor.b,
											  &(*quadIter));
#	endif
		}
    }
#endif
}

bool CCHierarchiesSprite::freshCurrentFrame () {
	unsigned int frameIndex = _curFrameIndex;
    _curFrameIndex = INT_MAX;
	return displayFrameAtIndex(frameIndex);
}

void CCHierarchiesSprite::draw () {
	CC_PROFILER_START_CATEGORY(HIERARCHIES_SPRITE_PROFILER, @"CCHierarchiesSprite - draw");
	
	// update quads to textureAtlas
	int quadsToDraw = _quads.size();
#if HIERARCHIES_SPRITE_ENABLE_DYEING == 1
	if ((quadsToDraw * 2) > _texAtlas->getCapacity())
		_texAtlas->resizeCapacity(quadsToDraw * 2);
#else
	if ((quadsToDraw) > _texAtlas->getCapacity())
		_texAtlas->resizeCapacity(quadsToDraw);
#endif
	_texAtlas->removeAllQuads();
	if (quadsToDraw > 0)
		_texAtlas->insertQuads(&_quads[0], 0, quadsToDraw);
	
#if HIERARCHIES_DEBUG_DRAW == 1
    // draw bounding box
	CCRect bbox = boundingBox();
	bbox = CCRectApplyAffineTransform(bbox, parentToNodeTransform());
    CCPoint vertices[4]={
        ccp(bbox.origin.x, bbox.origin.y),
        ccp(bbox.origin.x, bbox.origin.y + bbox.size.height),
        ccp(bbox.origin.x + bbox.size.width, bbox.origin.y + bbox.size.height),
        ccp(bbox.origin.x + bbox.size.width, bbox.origin.y),
    };
    ccDrawPoly(vertices, 4, true);
#endif
	
	// ignore the anchor point while drawing
	CCPoint ap = getAnchorPointInPoints();
	kmGLTranslatef(ap.x, ap.y, 0);
	
	// flip
	if (_flipX && _flipY) {
		kmGLScalef(-1, -1, 1);
	}
	else if (_flipX) {
		kmGLScalef(-1, 1, 1);
	}
	else if (_flipY) {
		kmGLScalef(1, -1, 1);
	}
	
	CC_NODE_DRAW_SETUP();
	
	ccGLBlendFunc( _blendFunc.src, _blendFunc.dst );
	
	_texAtlas->drawNumberOfQuads(quadsToDraw, 0);
	
	CHECK_GL_ERROR_DEBUG();
	CC_INCREMENT_GL_DRAWS(1);
	
	CC_PROFILER_STOP_CATEGORY(HIERARCHIES_SPRITE_PROFILER, @"CCHierarchiesSprite - draw");
}

bool CCHierarchiesSprite::setAvatarMap (const char* symbol, const char* item) {
	std::string symbolStr(symbol);
    std::string itemStr(item);
    
    std::map<std::string, DyeingAvatarData>::iterator symbolIter;
    symbolIter = _dyeingAvatarMap.find(symbolStr);
    if (symbolIter == _dyeingAvatarMap.end()) {
        return false;
    }
    
    CCHierarchiesSpriteAnimation::Item itemObj;
    int itemObjIndex = _animation->getItemByNameReturnIndex(itemStr, itemObj);
    if (itemObjIndex == -1) {
        return false;
    }
    
    symbolIter->second.itemName = itemObj.name;
    symbolIter->second.itemIndex = itemObjIndex;
    
    freshCurrentFrame();
    return true;
}

void CCHierarchiesSprite::setAvatarTag (const char* tagName) {
	std::string tagNameStr(tagName);
	
	std::map<std::string, DyeingAvatarData>::iterator symbolIter;
	for (symbolIter = _dyeingAvatarMap.begin(); symbolIter != _dyeingAvatarMap.end(); symbolIter++) {
		std::string itemName = tagNameStr + HIERARCHIES_SPRITE_AVATAR_TAG_SEPARATOR + symbolIter->first;
		CCHierarchiesSpriteAnimation::Item itemObj;
		int itemObjIndex = _animation->getItemByNameReturnIndex(itemName,  itemObj);
		if (itemObjIndex != -1) {
			symbolIter->second.itemName = itemObj.name;
			symbolIter->second.itemIndex = itemObjIndex;
		}
	}
	
	freshCurrentFrame();
}

void CCHierarchiesSprite::setAvatarTags (const char* firstTagName, ...) {
	std::string tagNameStr;
	va_list args;
	va_start(args, firstTagName);
	
	for (const char* tagName = firstTagName; tagName != NULL; tagName = va_arg(args, const char*)) {
		tagNameStr.append(tagName);
		tagNameStr.append(HIERARCHIES_SPRITE_AVATAR_TAG_SEPARATOR_STRING);
	}
	
	va_end(args);
	
	std::map<std::string, DyeingAvatarData>::iterator symbolIter;
	for (symbolIter = _dyeingAvatarMap.begin(); symbolIter != _dyeingAvatarMap.end(); symbolIter++) {
		std::string itemName = tagNameStr + symbolIter->first;
		CCHierarchiesSpriteAnimation::Item itemObj;
		int itemObjIndex = _animation->getItemByNameReturnIndex(itemName,  itemObj);
		if (itemObjIndex != -1) {
			symbolIter->second.itemName = itemObj.name;
			symbolIter->second.itemIndex = itemObjIndex;
		}
	}
	
	freshCurrentFrame();
}

bool CCHierarchiesSprite::setDyeingWithColor(const char* symbol, ccColor3B dyeingColor) {
	std::string symbolStr(symbol);
    
    std::map<std::string, DyeingAvatarData>::iterator symbolIter;
    symbolIter = _dyeingAvatarMap.find(symbolStr);
    if (symbolIter == _dyeingAvatarMap.end()) {
        return false;
    }
    
    symbolIter->second.dyeingColor = dyeingColor;
    
    freshCurrentFrame();
    return true;
}

void CCHierarchiesSprite::setDyeingAllWithColor(ccColor3B dyeingColor) {
	std::map<std::string, DyeingAvatarData>::iterator iter;
    for (iter = _dyeingAvatarMap.begin(); iter != _dyeingAvatarMap.end(); iter++) {
        iter->second.dyeingColor = dyeingColor;
    }
    
    freshCurrentFrame();
}
	
void CCHierarchiesSprite::setBlendFunc (ccBlendFunc value) {
	_blendFunc = value;
}

ccBlendFunc CCHierarchiesSprite::getBlendFunc (void) {
	return _blendFunc;
}

void CCHierarchiesSprite::updateBlendFunc () {
	if( !_texAtlas->getTexture()->hasPremultipliedAlpha() ) {
		_blendFunc.src = GL_SRC_ALPHA;
		_blendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
	}
}

CCTexture2D* CCHierarchiesSprite::getTexture (void) {
	return _texAtlas->getTexture();
}

void CCHierarchiesSprite::setTexture(CCTexture2D *texture) {
	_texAtlas->setTexture(texture);
	updateOpacityModifyRGB();
	updateBlendFunc();
    
    freshCurrentFrame();
}

const ccColor3B& CCHierarchiesSprite::getColor (void) {
	if(_opacityModifyRGB)
		return _colorUnmodified;
	
	return _color;
}

void CCHierarchiesSprite::setColor (const ccColor3B& color3) {
	_color = _colorUnmodified = color3;
	
	if( _opacityModifyRGB ){
		_color.r = color3.r * _opacity/255;
		_color.g = color3.g * _opacity/255;
		_color.b = color3.b * _opacity/255;
	}
    
    freshCurrentFrame();
}

GLubyte CCHierarchiesSprite::getOpacity (void) {
	return _opacity;
}

void CCHierarchiesSprite::setOpacity (GLubyte opacity) {
	_opacity = opacity;
	
	// special opacity for premultiplied textures
	if( _opacityModifyRGB )
		setColor(_colorUnmodified);
    
    freshCurrentFrame();
}

bool CCHierarchiesSprite::isOpacityModifyRGB (void) {
	return _opacityModifyRGB;
}

void CCHierarchiesSprite::setOpacityModifyRGB (bool bValue) {
	_opacityModifyRGB = bValue;
}

void CCHierarchiesSprite::updateOpacityModifyRGB () {
	_opacityModifyRGB = _texAtlas->getTexture()->hasPremultipliedAlpha();
}


// CCHierarchiesAnimate implement

CCHierarchiesAnimate* CCHierarchiesAnimate::create (const char* animationName, CCHierarchiesSpriteAnimation* animation) {
    CCHierarchiesAnimate* ret = new CCHierarchiesAnimate();
	if (ret->initWithHierarchiesAnimation(animationName, animation))
	{
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return NULL;
}

bool CCHierarchiesAnimate::initWithHierarchiesAnimation (const char* animationName, CCHierarchiesSpriteAnimation* animation) {
    CCAssert( animationName != NULL && animation != NULL, "CCHierarchiesAnimate: arguments must be non-nil");
    
    if (!animation->getAnimationByName(animationName, _curAnimation)) {
		CCLOG("no animation (%s) when create CCHierarchiesAnimate", animationName);
        return false;
	}
    
    float duration = (float)(_curAnimation.endFrameIndex - _curAnimation.startFrameIndex + 1) / animation->getFrameRate();
    
    if (CCActionInterval::initWithDuration(duration)) {
        _animation = animation;
		_animationName = animationName;
        return true;
    }
    else {
        CC_SAFE_RELEASE(this);
        return false;
    }
}

const char* CCHierarchiesAnimate::getAnimationName () {
    return _animationName.c_str();
}

bool CCHierarchiesAnimate::isDone () {
    if (m_elapsed >= m_fDuration) {
        if (_curAnimation.loop) {
            m_elapsed -= m_fDuration;
        }
        else {
            return true;
        }
    }
    return false;
}

CCObject* CCHierarchiesAnimate::copyWithZone( CCZone* pZone) {
    CCZone* pNewZone = NULL;
    CCHierarchiesAnimate* pRet = NULL;
    if(pZone && pZone->m_pCopyObject) //in case of being called at sub class
    {
        pRet = static_cast<CCHierarchiesAnimate*>(pZone->m_pCopyObject);
    }
    else
    {
        pRet = new CCHierarchiesAnimate();
        pZone = pNewZone = new CCZone(pRet);
    }
    
//    CCActionInterval::copyWithZone(pZone); // call super class init once
//    
//    pRet->initWithHierarchiesAnimation(_animationName.c_str(), _animation); // call super class init twice ??
    
    if (pRet->initWithHierarchiesAnimation(_animationName.c_str(), _animation)) {
        CC_SAFE_DELETE(pNewZone);
        return pRet;
    }
    else {
        CC_SAFE_DELETE(pNewZone);
        return NULL;
    }
}

void CCHierarchiesAnimate::update (float time) {
    CCHierarchiesSprite* target = static_cast<CCHierarchiesSprite*>(m_pTarget);
    unsigned int curFrameIndex = _curAnimation.startFrameIndex + time * (_curAnimation.endFrameIndex - _curAnimation.startFrameIndex);
    target->displayFrameAtIndex(curFrameIndex);
}

NS_CC_EXT_END
