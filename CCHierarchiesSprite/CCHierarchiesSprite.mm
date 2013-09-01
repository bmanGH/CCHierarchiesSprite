//
//  CCHierarchiesSprite.m
//  CCHierarchiesSprite
//
//  Created by bman <zx123xz321hm3@hotmail.com>.
//  Copyright (c) 2013 Break-media. All rights reserved.
//

#import "CCHierarchiesSprite.h"
#import "ccConfig.h"
#include <limits.h>
#import "cocos2d.h"
#include "kazmath/kazmath.h"
#include "kazmath/GL/mat4stack.h"
#include "kazmath/GL/matrix.h"
#import "CCHierarchiesSpriteSheetCache.h"
#import "CCHierarchiesSpriteAnimationCache.h"
#import "CCHierarchiesSpriteRuntimeAnimationCache.h"


#pragma mark - CCHierarchiesSprite

inline static void hierarchiesCalcMatrix (CCHierarchiesSpriteAnimation::Item* item, CCHierarchiesSpriteAnimation::Element* element, CGAffineTransform* matrix) {
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
    
    *matrix = CGAffineTransformMake(m.a, -m.b, -m.c, m.d, m.tx, -m.ty);
}

inline static void hierarchiesUpdateQuadVertices (CGSize size, CGAffineTransform* matrix, ccV3F_C4B_T2F_Quad* quad, float z, bool isRotation) {
    if (isRotation) {
        float tmp = size.width;
        size.width = size.height;
        size.height = tmp;
    }
    
    float x1 = 0;
    float y1 = 0;
    
    float x2 = x1 + size.width;
    float y2 = y1 + size.height;
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

inline static void hierarchiesUpdateQuadTextureCoords (CGRect rect, float texWidth, float texHeight, ccV3F_C4B_T2F_Quad* quad, bool isRotation) {
    float left, right, top, bottom;
    
#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL    
    left	= (2 * rect.origin.x + 1) / (2 * texWidth);
    right	= left + (rect.size.width * 2 - 2) / (2 * texWidth);
    top		= (2 * rect.origin.y + 1) / (2 * texHeight);
    bottom	= top + (rect.size.height * 2 - 2) / (2 * texHeight);
#else
    left	= rect.origin.x / texWidth;
    right	= left + rect.size.width / texWidth;
    top		= rect.origin.y / texHeight;
    bottom	= top + rect.size.height / texHeight;
#endif
    
	//    if(flipX)
	//        CC_SWAP(left,right);
	//    if(flipY)
	//        CC_SWAP(top,bottom);
    
    if (!isRotation) {
        quad->bl.texCoords.u = left;
        quad->bl.texCoords.v = bottom;
        quad->br.texCoords.u = right;
        quad->br.texCoords.v = bottom;
        quad->tl.texCoords.u = left;
        quad->tl.texCoords.v = top;
        quad->tr.texCoords.u = right;
        quad->tr.texCoords.v = top;
    }
    else {
        quad->bl.texCoords.u = left; // tl
        quad->bl.texCoords.v = top; // tl
        quad->br.texCoords.u = left; // bl
        quad->br.texCoords.v = bottom; // bl
        quad->tl.texCoords.u = right; // tr
        quad->tl.texCoords.v = top; // tr
        quad->tr.texCoords.u = right; // br
        quad->tr.texCoords.v = bottom; // br
    }
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


@interface CCHierarchiesSprite ()

- (void) setup; // setup runtime resource
- (void) shutdown; // release runtime resource

- (void) updateOpacityModifyRGB;
- (void) updateBlendFunc;
- (void) updateUnCachedDataInFrame;

@end


@implementation CCHierarchiesSprite

@synthesize sheet = _sheet;
@synthesize animation = _animation;
@synthesize textureAtlas = _texAtlas;
@synthesize flipX = _flipX;
@synthesize flipY = _flipY;
@synthesize sheetName = _sheetName;
@synthesize animationName = _animationName;
@synthesize delegate = _delegate;
@synthesize enableAvatar = _enabledAvatar;

+ (id) hierarchiesSpriteWithSpriteSheetFile:(NSString*)sheetFileName spriteAnimationFile:(NSString*)animationFileName delegate:(id<CCHierarchiesSpriteEventDelegate>)delegate {
    return [[[self alloc] initWithSpriteSheetFile:sheetFileName spriteAnimationFile:animationFileName delegate:delegate] autorelease];
}

+ (id) hierarchiesSpriteWithSpriteSheetFile:(NSString*)sheetFileName spriteAnimationFile:(NSString*)animationFileName {
    return [[[self alloc] initWithSpriteSheetFile:sheetFileName spriteAnimationFile:animationFileName] autorelease];
}

- (id) initWithSpriteSheetFile:(NSString*)sheetFileName spriteAnimationFile:(NSString*)animationFileName delegate:(id<CCHierarchiesSpriteEventDelegate>)delegate {
    if ( (self = [super init]) ) {
        _sheetName = [sheetFileName copy];
        _animationName = [animationFileName copy];
		
        _blendFunc.src = GL_SRC_ALPHA;
		_blendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
		
        _flipX = NO;
        _flipY = NO;
		
        _delegate = delegate;
		_curFrameIndex = INT_MAX;
        
        _sheet = [[CCHierarchiesSpriteSheetCache sharedHierarchiesSpriteSheetCache] addSpriteSheet:sheetFileName];
        _animation = [[CCHierarchiesSpriteAnimationCache sharedHierarchiesSpriteAnimationCache] addAnimation:animationFileName];
        [[CCHierarchiesSpriteRuntimeAnimationCache sharedHierarchiesSpriteRuntimeAnimationCache] insertHierarchiesSprite:self];
        
        for (int i = 0; i < _animation->getSymbolCount(); i++) {
            CCHierarchiesSpriteAnimation::Symbol symbol;
            _animation->getSymbolByIndex(i, symbol);
            CCHierarchiesSpriteAnimation::Item item;
            _animation->getItemByIndex(symbol.defaultItemIndex, item);
            _avatarMap[symbol.name].itemName = item.name;
            _avatarMap[symbol.name].itemIndex = symbol.defaultItemIndex;
        }
        
        [self displayFrameAtIndex:0];
        
        return self;
    }
    return nil;
}

- (id) initWithSpriteSheetFile:(NSString*)sheetFileName spriteAnimationFile:(NSString*)animationFileName {
    return [self initWithSpriteSheetFile:sheetFileName spriteAnimationFile:animationFileName delegate:nil];
}

- (void) dealloc {
    [[CCHierarchiesSpriteRuntimeAnimationCache sharedHierarchiesSpriteRuntimeAnimationCache] removeHierarchiesSprite:self];
    [[CCHierarchiesSpriteSheetCache sharedHierarchiesSpriteSheetCache] removeSpriteSheet:_sheetName];
    [[CCHierarchiesSpriteAnimationCache sharedHierarchiesSpriteAnimationCache] removeAnimation:_animationName];
    
    [_sheetName release];
    _sheetName = nil;
    [_animationName release];
    _animationName = nil;
    
    [super dealloc];
}

- (void) setSpriteSheetFile:(NSString*)sheetFileName spriteAnimationFile:(NSString*)animationFileName {
    // cleanup
    if (self.isRunning) {
        [self shutdown];
    }
    
	[[CCHierarchiesSpriteRuntimeAnimationCache sharedHierarchiesSpriteRuntimeAnimationCache] removeHierarchiesSprite:self];
	[[CCHierarchiesSpriteSheetCache sharedHierarchiesSpriteSheetCache] removeSpriteSheet:_sheetName];
	_sheet = nil;
    [[CCHierarchiesSpriteAnimationCache sharedHierarchiesSpriteAnimationCache] removeAnimation:_animationName];
	_animation = nil;
    
    [_sheetName release];
	_sheetName = nil;
    [_animationName release];
	_animationName = nil;
    
	_avatarMap.clear();
    
    // setup
    _sheetName = [sheetFileName copy];
	_animationName = [animationFileName copy];
    
	_sheet = [[CCHierarchiesSpriteSheetCache sharedHierarchiesSpriteSheetCache] addSpriteSheet:sheetFileName];
	_animation = [[CCHierarchiesSpriteAnimationCache sharedHierarchiesSpriteAnimationCache] addAnimation:animationFileName];
    [[CCHierarchiesSpriteRuntimeAnimationCache sharedHierarchiesSpriteRuntimeAnimationCache] insertHierarchiesSprite:self];
	
	for (int i = 0; i < _animation->getSymbolCount(); i++) {
		CCHierarchiesSpriteAnimation::Symbol symbol;
		_animation->getSymbolByIndex(i, symbol);
		CCHierarchiesSpriteAnimation::Item item;
		_animation->getItemByIndex(symbol.defaultItemIndex, item);
		_avatarMap[symbol.name].itemName = item.name;
		_avatarMap[symbol.name].itemIndex = symbol.defaultItemIndex;
	}
	
	_curFrameIndex = INT_MAX;
    [self displayFrameAtIndex:0];
	
    if (self.isRunning) {
        [self setup];
    }
}

- (unsigned int) eventContentCount:(NSString*)eventContent {
    const std::vector<CCHierarchiesSpriteAnimation::Event>& events = _animation->getEvents();
    unsigned int ret = 0;
    std::vector<CCHierarchiesSpriteAnimation::Event>::const_iterator iter;
    for (iter = events.begin(); iter != events.end(); iter++) {
        if (iter->content.compare([eventContent UTF8String]) == 0)
            ret++;
    }
    return ret;
}

- (void) setFlipX:(BOOL)value {
    _flipX = value;
	_isTransformDirty = _isInverseDirty = YES;
}

- (void) setFlipY:(BOOL)value {
    _flipY = value;
	_isTransformDirty = _isInverseDirty = YES;
}


#pragma mark - Setup and shutdown

- (void) onEnter {
	[super onEnter];
	[self setup];
	[self freshCurrentFrame];
}

- (void) onExit {
	[super onExit];
	[self shutdown];
}

- (void) setup {
    // image should load from the same directory with spritesheet file
    std::string imageName(_sheet->getImageName());
    std::string sheetName([_sheetName UTF8String]);
    size_t found = sheetName.find_last_of(HIERARCHIES_SPRITE_PATH_SEPARATOR);
    if (found != std::string::npos)
        imageName = sheetName.substr(0, found) + HIERARCHIES_SPRITE_PATH_SEPARATOR + imageName;
    
	NSString* imgName = [[NSString alloc] initWithUTF8String:imageName.c_str()];
	_texAtlas = [[CCTextureAtlas alloc] initWithFile:imgName capacity:HIERARCHIES_SPRITE_DEFAULT_TEXTURE_ATLAS_CAPACITY];
	[imgName release];
    
    [self setShaderProgram:[[CCShaderCache sharedShaderCache] programForKey:kCCShader_PositionTextureColor]];
	
	[self updateBlendFunc];
	[self updateOpacityModifyRGB];
}

- (void) shutdown {
    //	_quads.clear();
	_quadsToDraw = 0;
	
	[_texAtlas release];
	_texAtlas = nil;
}


#pragma mark - Animation

- (BOOL) displayFrameAtIndex:(unsigned int) frameIndex {
	if (_curFrameIndex == frameIndex)
        return true;
	
	if (_curFrameIndex != frameIndex) {
		// event dispatch
		if (_delegate) {
			CCHierarchiesSpriteAnimation::Event event;
			if (_animation->getEventByFrameId(frameIndex, event)) {
                [_delegate onSprite:self withEventContent:[NSString stringWithUTF8String:event.content.c_str()]];
			}
		}
	}
    
    _needFresh = YES;
	
    // if animation data is cached than read cached data
	AnimationCacheHashItem* cacheItem = NULL;
	HASH_FIND_STR([CCHierarchiesSpriteRuntimeAnimationCache sharedHierarchiesSpriteRuntimeAnimationCache]->_animationCache, [_animationName UTF8String], cacheItem);
	if (cacheItem) {
		if (frameIndex < cacheItem->cache.size() && cacheItem->cache[frameIndex].cached) {
			_curFrameIndex = frameIndex;
			
//			_quads = cacheItem->cache[frameIndex].frame;
            
            // update bounding box, content size and anchor point
			_bbox = cacheItem->cache[frameIndex].bbox;
			
			CGPoint anchorPoint = ccp((0 - _bbox.origin.x) / _bbox.size.width,
                                      (0 - _bbox.origin.y) / _bbox.size.height);
			[self setAnchorPoint:anchorPoint];
			[self setContentSize:_bbox.size];
			
			return true;
		}
	}
    else {
        CCLOG(@"<CCHierarchiesSprite> no animation cache with [%@]", _animationName);
        return false;
    }
    
    // calc frame quads and insert to cache
    CCHierarchiesSpriteAnimation::FrameElements frameElements;
    int eNum = _animation->getFrameElementsAtIndex(frameIndex, frameElements);
    
    if (eNum == -1) {
		CCLOG(@"<CCHierarchiesSprite> [%@] no elements at frame %d", _animationName, frameIndex);
        return false;
	}
    
    _curFrameIndex = frameIndex;
    
//	_quads.clear();
//	if ((eNum) > _quads.capacity())
//		_quads.reserve(eNum);
    
    std::vector<ccV3F_C4B_T2F_Quad> _quads(eNum);
    std::vector<int> avatarSymbolIndexList(eNum);
    
    float min_X = 10000, max_X = -10000, min_Y = 10000, max_Y = -10000;
    
    CGAffineTransform matrix;
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
			
			AvatarMapItem avatarMapItem;
			avatarMapItem = _avatarMap[symbol.name];
			
			CCHierarchiesSpriteAnimation::Item item;
			result = _animation->getItemByIndex(avatarMapItem.itemIndex, item);
			assert(result);
			
			CCHierarchiesSpriteSheet::Spr spr;
			result = _sheet->getSpr(avatarMapItem.itemName, spr);
			assert(result);
			
			// calc matrix
			hierarchiesCalcMatrix(&item, &(*elementIter), &matrix);
			
			// update vertices
			hierarchiesUpdateQuadVertices(CGSizeMake(spr.w, spr.h),
                                          &matrix,
                                          &quad,
                                          layerZOrder * HIERARCHIES_SPRITE_LAYER_Z_ORDER_SCALE,
                                          spr.isRotation);
			
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

            hierarchiesUpdateQuadTextureCoords(CGRectMake(spr.x, spr.y, spr.w, spr.h),
                                               _sheet->getImageWidth(),
                                               _sheet->getImageHeight(),
                                               &quad,
                                               spr.isRotation);
			
			_quads.push_back(quad);
            avatarSymbolIndexList.push_back(elementIter->symbolIndex);
        }
		layerZOrder++;
	}
    
    // update bounding box, content size and anchor point
	_bbox = CGRectMake(min_X, min_Y, max_X - min_X, max_Y - min_Y);
	CGPoint anchorPoint = ccp((0 - _bbox.origin.x) / _bbox.size.width,
                              (0 - _bbox.origin.y) / _bbox.size.height);
	[self setAnchorPoint:anchorPoint];
    [self setContentSize:_bbox.size];
	
	// insert animation frame data to cache
	cacheItem->cache[frameIndex].frame = _quads;
    cacheItem->cache[frameIndex].avatarSymbolIndexList = avatarSymbolIndexList;
	cacheItem->cache[frameIndex].bbox = _bbox;
	cacheItem->cache[frameIndex].cached = true;
	
	return true;
}

- (void) updateUnCachedDataInFrame {
    // get cached quads
    AnimationCacheHashItem* cacheItem = NULL;
	HASH_FIND_STR([CCHierarchiesSpriteRuntimeAnimationCache sharedHierarchiesSpriteRuntimeAnimationCache]->_animationCache, [_animationName UTF8String], cacheItem);
    std::vector<ccV3F_C4B_T2F_Quad> _quads = cacheItem->cache[_curFrameIndex].frame;
    
    // update uncached data
    bool ignoreColor = (_displayedOpacity == 255 &&
                        _displayedColor.r == 255 &&
                        _displayedColor.g == 255 &&
                        _displayedColor.b == 255);
    
    if (!_enabledAvatar) {
        if (!ignoreColor) {
            std::vector<ccV3F_C4B_T2F_Quad>::iterator quadIter;
            for (quadIter = _quads.begin(); quadIter != _quads.end(); quadIter++) {
                //		hierarchiesFlipQuadVertices(_flipX, _flipX, &(*quadIter));
                hierarchiesUpdateQuadTextureColor(_opacityModifyRGB,
                                                  _displayedOpacity,
                                                  _displayedColor.r,
                                                  _displayedColor.g,
                                                  _displayedColor.b,
                                                  &(*quadIter));
            }
        }
    }
    else {
        FrameCacheItem& frameCacheItemIter = cacheItem->cache.at(_curFrameIndex);
        std::vector<ccV3F_C4B_T2F_Quad>::iterator quadIter;
        std::vector<int>::iterator avatarSymbolIndexIter;
        for (quadIter = _quads.begin(), avatarSymbolIndexIter = frameCacheItemIter.avatarSymbolIndexList.begin();
             quadIter != _quads.end();
             quadIter++, avatarSymbolIndexIter++) {
            bool result = false;
            
            //		hierarchiesFlipQuadVertices(_flipX, _flipY, &(*quadIter));
            
            CCHierarchiesSpriteAnimation::Symbol symbol;
            result = _animation->getSymbolByIndex(*avatarSymbolIndexIter, symbol);
            assert(result);
            
            AvatarMapItem avatarMapItem;
            avatarMapItem = _avatarMap[symbol.name];
            
            CCHierarchiesSpriteAnimation::Item item;
            result = _animation->getItemByIndex(avatarMapItem.itemIndex, item);
            assert(result);
            
            // update avater tex coords, opacity and color vertices
            // IMPORTANT: because of performance, there is no re-calc matrix and vertices here
            CCHierarchiesSpriteSheet::Spr spr;
            result = _sheet->getSpr(avatarMapItem.itemName, spr);
            assert(result);
            
            hierarchiesUpdateQuadTextureCoords(CGRectMake(spr.x, spr.y, spr.w, spr.h),
                                               _sheet->getImageWidth(),
                                               _sheet->getImageHeight(),
                                               &(*quadIter),
                                               spr.isRotation);
            
            if (!ignoreColor)
                hierarchiesUpdateQuadTextureColor(_opacityModifyRGB,
                                                  _displayedOpacity,
                                                  _displayedColor.r,
                                                  _displayedColor.g,
                                                  _displayedColor.b,
                                                  &(*quadIter));
        }
    }
    
    // update quads to textureAtlas
	_quadsToDraw = _quads.size();
	if ((_quadsToDraw) > _texAtlas.capacity)
		[_texAtlas resizeCapacity:_quadsToDraw];
	[_texAtlas removeAllQuads];
	if (_quadsToDraw > 0)
		[_texAtlas insertQuads:&_quads[0] atIndex:0 amount:_quadsToDraw];
}

- (BOOL) freshCurrentFrame {    
    unsigned int frameIndex = _curFrameIndex;
    _curFrameIndex = INT_MAX;
	return [self displayFrameAtIndex:frameIndex];
}

- (unsigned int) currentFrameIndex {
    return _curFrameIndex;
}


#pragma mark - Draw

- (void) draw {
	CC_PROFILER_START_CATEGORY(HIERARCHIES_SPRITE_PROFILER, @"CCHierarchiesSprite - animation");
    
    if (_needFresh) {
        [self updateUnCachedDataInFrame];
        _needFresh = NO;
    }
    
    CC_PROFILER_STOP_CATEGORY(HIERARCHIES_SPRITE_PROFILER, @"CCHierarchiesSprite - animation");
    
    // draw
	CC_PROFILER_START_CATEGORY(HIERARCHIES_SPRITE_PROFILER, @"CCHierarchiesSprite - draw");
	
//	// update quads to textureAtlas
//	int quadsToDraw = _quads.size();
//	if ((quadsToDraw) > _texAtlas.capacity)
//		[_texAtlas resizeCapacity:quadsToDraw];
//	[_texAtlas removeAllQuads];
//	if (quadsToDraw > 0)
//		[_texAtlas insertQuads:&_quads[0] atIndex:0 amount:quadsToDraw];
	
	// ignore the anchor point while drawing
    CGPoint ap = self.anchorPointInPoints;
    kmGLTranslatef(ap.x, ap.y, 0);
	
	CC_NODE_DRAW_SETUP();
	
	ccGLBlendFunc( _blendFunc.src, _blendFunc.dst );
	
    [_texAtlas drawNumberOfQuads:_quadsToDraw fromIndex:0];
	
	CHECK_GL_ERROR_DEBUG();
    
#if HIERARCHIES_DEBUG_DRAW == 1
    // draw bounding box
    CGPoint vertices[4] = {
        ccp(_bbox.origin.x, _bbox.origin.y),
        ccp(_bbox.origin.x + _bbox.size.width, _bbox.origin.y),
        ccp(_bbox.origin.x + _bbox.size.width, _bbox.origin.y + _bbox.size.height),
        ccp(_bbox.origin.x, _bbox.origin.y + _bbox.size.height),
    };
    ccDrawPoly(vertices, 4, true);
#endif
	
	CC_PROFILER_STOP_CATEGORY(HIERARCHIES_SPRITE_PROFILER, @"CCHierarchiesSprite - draw");
}


#pragma mark - Avatar

- (void) setEnableAvatar:(BOOL)value {
    _enabledAvatar = value;
    [self freshCurrentFrame];
}

- (BOOL) setAvatarMapWithSymbol:(NSString*)symbol toItem:(NSString*)item {
	std::string symbolStr([symbol UTF8String]);
    std::string itemStr([item UTF8String]);
    
#ifdef HIERARCHIES_USE_CPP_11
    std::unordered_map<std::string, AvatarMapItem>::iterator symbolIter;
#else
    std::map<std::string, AvatarMapItem>::iterator symbolIter;
#endif
    symbolIter = _avatarMap.find(symbolStr);
    if (symbolIter == _avatarMap.end()) {
        return false;
    }
    
    CCHierarchiesSpriteAnimation::Item itemObj;
    int itemObjIndex = _animation->getItemByNameReturnIndex(itemStr, itemObj);
    if (itemObjIndex == -1) {
        return false;
    }
    
    symbolIter->second.itemName = itemObj.name;
    symbolIter->second.itemIndex = itemObjIndex;
    
    [self freshCurrentFrame];
    return true;
}

- (void) setAvatarTag:(NSString*)tagName {
	std::string tagNameStr([tagName UTF8String]);
	
#ifdef HIERARCHIES_USE_CPP_11
	std::unordered_map<std::string, AvatarMapItem>::iterator symbolIter;
#else
    std::map<std::string, AvatarMapItem>::iterator symbolIter;
#endif
	for (symbolIter = _avatarMap.begin(); symbolIter != _avatarMap.end(); symbolIter++) {
		std::string itemName = tagNameStr + HIERARCHIES_SPRITE_PATH_SEPARATOR + symbolIter->first;
		CCHierarchiesSpriteAnimation::Item itemObj;
		int itemObjIndex = _animation->getItemByNameReturnIndex(itemName,  itemObj);
		if (itemObjIndex != -1) {
			symbolIter->second.itemName = itemObj.name;
			symbolIter->second.itemIndex = itemObjIndex;
		}
	}
	
	[self freshCurrentFrame];
}


#pragma mark - CCNode

- (CGAffineTransform) nodeToParentTransform {
	if ( _isTransformDirty ) {
        // flip anchor point in points
        CGPoint anchorPointInPoints = _anchorPointInPoints;
        if (_flipX && _flipY) {
            anchorPointInPoints = ccp(-_anchorPointInPoints.x, -_anchorPointInPoints.y);
        }
        else if (_flipX) {
            anchorPointInPoints = ccp(-_anchorPointInPoints.x, _anchorPointInPoints.y);
        }
        else if (_flipY) {
            anchorPointInPoints = ccp(_anchorPointInPoints.x, -_anchorPointInPoints.y);
        }
        
		// Translate values
		float x = _position.x;
		float y = _position.y;
        
		if ( _ignoreAnchorPointForPosition ) {
			x += anchorPointInPoints.x;
			y += anchorPointInPoints.y;
		}
        
		// Rotation values
		// Change rotation code to handle X and Y
		// If we skew with the exact same value for both x and y then we're simply just rotating
		float cx = 1, sx = 0, cy = 1, sy = 0;
		if( _rotationX || _rotationY ) {
			float radiansX = -CC_DEGREES_TO_RADIANS(_rotationX);
			float radiansY = -CC_DEGREES_TO_RADIANS(_rotationY);
			cx = cosf(radiansX);
			sx = sinf(radiansX);
			cy = cosf(radiansY);
			sy = sinf(radiansY);
		}
        
		BOOL needsSkewMatrix = ( _skewX || _skewY );
        
		// optimization:
		// inline anchor point calculation if skew is not needed
		// Adjusted transform calculation for rotational skew
		if( !needsSkewMatrix && !CGPointEqualToPoint(_anchorPointInPoints, CGPointZero) ) {
			x += cy * -anchorPointInPoints.x * _scaleX + -sx * -anchorPointInPoints.y * _scaleY;
			y += sy * -anchorPointInPoints.x * _scaleX +  cx * -anchorPointInPoints.y * _scaleY;
		}
        
        
		// Build Transform Matrix
		// Adjusted transfor m calculation for rotational skew
		_transform = CGAffineTransformMake( cy * _scaleX, sy * _scaleX,
										   -sx * _scaleY, cx * _scaleY,
										   x, y );
        
		// XXX: Try to inline skew
		// If skew is needed, apply skew and then anchor point
		if( needsSkewMatrix ) {
			CGAffineTransform skewMatrix = CGAffineTransformMake(1.0f, tanf(CC_DEGREES_TO_RADIANS(_skewY)),
																 tanf(CC_DEGREES_TO_RADIANS(_skewX)), 1.0f,
																 0.0f, 0.0f );
			_transform = CGAffineTransformConcat(skewMatrix, _transform);
            
			// adjust anchor point
			if( ! CGPointEqualToPoint(anchorPointInPoints, CGPointZero) )
				_transform = CGAffineTransformTranslate(_transform, -anchorPointInPoints.x, -anchorPointInPoints.y);
		}
        
        // flip
        if (_flipX && _flipY) {
            _transform = CGAffineTransformScale(_transform, -1, -1);
        }
        else if (_flipX) {
            _transform = CGAffineTransformScale(_transform, -1, 1);
        }
        else if (_flipY) {
            _transform = CGAffineTransformScale(_transform, 1, -1);
        }
        
		_isTransformDirty = NO;
	}
    
	return _transform;
}


#pragma mark - CCBlendProtocol

- (void) setBlendFunc:(ccBlendFunc)value {
    _blendFunc = value;
}

- (ccBlendFunc) blendFunc {
    return _blendFunc;
}


#pragma mark - CCTextureProtocol

- (CCTexture2D*) texture {
    return _texAtlas.texture;
}

- (void) setTexture:(CCTexture2D*)texture {
    _texAtlas.texture = texture;
    [self updateBlendFunc];
    [self updateOpacityModifyRGB];
    
    [self freshCurrentFrame];
}


#pragma mark - CCRGBAProtocol

-(void) setColor:(ccColor3B)color3
{
    [super setColor:color3];
	[self freshCurrentFrame];
}

-(void) setOpacity:(GLubyte)opacity
{
    [super setOpacity:opacity];
	[self freshCurrentFrame];
}

-(void) updateDisplayedColor:(ccColor3B)parentColor
{
    [super updateDisplayedColor:parentColor];
    [self freshCurrentFrame];
}

-(void) updateDisplayedOpacity:(GLubyte)parentOpacity
{
    [super updateDisplayedOpacity:parentOpacity];
    [self freshCurrentFrame];
}

-(void) setOpacityModifyRGB:(BOOL)modify
{
	if( _opacityModifyRGB != modify ) {
		_opacityModifyRGB = modify;
		[self freshCurrentFrame];
	}
}

-(BOOL) doesOpacityModifyRGB
{
	return _opacityModifyRGB;
}


#pragma mark - Private

- (void) updateOpacityModifyRGB {
    _opacityModifyRGB = [_texAtlas.texture hasPremultipliedAlpha];
}

- (void) updateBlendFunc {
    if( ! [_texAtlas.texture hasPremultipliedAlpha] ) {
		_blendFunc.src = GL_SRC_ALPHA;
		_blendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
	}
}

@end


#pragma mark - CCHierarchiesAnimate

@implementation CCHierarchiesAnimate

@synthesize animationName = _animationName;
@synthesize spriteAnimationName = _spriteAnimationName;
@synthesize spriteAnimation = _spriteAnimation;
@synthesize currentAnimation = _currentAnimation;

+ (id) actionWithHierarchiesAnimationName:(NSString*)animationName spriteAnimationName:(NSString*)spriteAnimationName {
    return [[[self alloc] initWithHierarchiesAnimationName:animationName spriteAnimationName:spriteAnimationName] autorelease];
}

- (id) initWithHierarchiesAnimationName:(NSString*)animationName spriteAnimationName:(NSString*)spriteAnimationName {
    NSAssert(animationName != nil && spriteAnimationName != nil, @"CCHierarchiesAnimate: arguments must be non-nil");
    
    _spriteAnimation = [[CCHierarchiesSpriteAnimationCache sharedHierarchiesSpriteAnimationCache] addAnimation:spriteAnimationName];
    
    if (!_spriteAnimation->getAnimationByName([animationName UTF8String], _currentAnimation)) {
		CCLOG(@"no animation (%@) in (%@) while create CCHierarchiesAnimate", animationName, spriteAnimationName);
        [[CCHierarchiesSpriteAnimationCache sharedHierarchiesSpriteAnimationCache] removeAnimation:spriteAnimationName];
		[self release];
		return nil;
	}
	
	float duration = (float)(_currentAnimation.endFrameIndex - _currentAnimation.startFrameIndex + 1) / _spriteAnimation->getFrameRate();
	
	if( (self=[super initWithDuration:duration]) ) {
		_animationName = [animationName copy];
        _spriteAnimationName = [spriteAnimationName copy];
	}
	return self;
}

- (void) startWithTarget:(id)aTarget {
    NSAssert([aTarget isKindOfClass:[CCHierarchiesSprite class]], @"CCHierarchiesAnimate can only effect on CCHierarchiesSprite");
    
    CCHierarchiesSprite* target = (CCHierarchiesSprite*)aTarget;
    NSAssert([target.animationName isEqualToString:_spriteAnimationName], @"CCHierarchiesAnimate effect on a invalid CCHierarchiesSprite instance");

	[super startWithTarget:aTarget];
    
    _curFrameIndex = _currentAnimation.startFrameIndex;
    [target displayFrameAtIndex:_curFrameIndex];
}

- (void) update:(ccTime)time {
	CCHierarchiesSprite* target = (CCHierarchiesSprite*)_target;
	unsigned int curFrameIndex = _currentAnimation.startFrameIndex + time * (_currentAnimation.endFrameIndex - _currentAnimation.startFrameIndex);
    while (_curFrameIndex != curFrameIndex) {
        _curFrameIndex++;
        [target displayFrameAtIndex:curFrameIndex];
    }
}

- (id) copyWithZone: (NSZone*) zone
{
	CCAction *copy = [[[self class] allocWithZone: zone] initWithHierarchiesAnimationName:_animationName spriteAnimationName:_spriteAnimationName];
	return copy;
}

- (void) dealloc {
    [[CCHierarchiesSpriteAnimationCache sharedHierarchiesSpriteAnimationCache] removeAnimation:_spriteAnimationName];
	[_animationName release];
    [_spriteAnimationName release];
	
	[super dealloc];
}

@end
