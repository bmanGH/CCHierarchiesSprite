//
//  CCHierarchiesSpriteAnimation.h
//  CCHierarchiesSprite
//
//  Created by Xu Xiaocheng <xcxu@bread-media.com.cn>.
//  Copyright (c) 2012 Break-media. All rights reserved.
//

#ifndef _CCHierarchiesSpriteAnimation_H_
#define _CCHierarchiesSpriteAnimation_H_

#include <string>
#include <vector>
#include <map>
#include <math.h>
#include "rapidxml/rapidxml.hpp"
#include "CCHierarchiesSpriteConfig.h"
#include "ccMacros.h"

NS_CC_EXT_BEGIN

// Quaternion 2D
struct Spinor {
    float real;
    float complex;
    
    Spinor () {}
    
    Spinor (float real, float complex)
    : real(real), complex(complex) {
    }
    
    Spinor (const Spinor& copy) {
        this->real = copy.real;
        this->complex = copy.complex;
    }
    
    Spinor& operator= (const Spinor& rhs) {
        this->real = rhs.real;
        this->complex = rhs.complex;
        return *this;
    }
    
    Spinor (float radian) {
        real = cosf(radian);
        complex = sinf(radian);
    }
    
    Spinor operator* (const float rhs) {
        return Spinor(real * rhs, complex * rhs);
    }
    
    Spinor operator* (const Spinor& rhs) {
        return Spinor(real * rhs.real - complex * rhs.complex, real * rhs.complex + complex * rhs.real);
    }
    
    Spinor operator+ (const Spinor& rhs) {
        return Spinor(real + rhs.real, complex + rhs.complex);
    }
    
    float length () {
        return sqrtf(real * real + complex * complex);
    }
    
    float lengthSquared () {
        return (real * real + complex * complex);
    }
    
    Spinor invert () {
        Spinor s(real, -complex);
        return Spinor(s * s.lengthSquared());
    }
    
    Spinor normalized () {
        float l = length();
        return Spinor(real / l, complex / l);
    }
    
    float getRadian () {
        return atan2f(complex, real) * 2;
    }
    
    Spinor lerp (Spinor& target, float t) {
        Spinor s = (((*this) * (1 - t)) + (target * t)).normalized();
        return s;
    }
    
    Spinor slerp (Spinor& target, float t) {
        float tr;
        float tc;
        float omega, cosom, sinom, scale0, scale1;
        
        // calc cosine
        cosom = this->real * target.real + this->complex * target.complex;
        
        // adjust signs
        if (cosom < 0) {
            cosom = -cosom;
            tc = -target.complex;
            tr = -target.real;
        }
        else {
            tc = target.complex;
            tr = target.real;
        }
        
        // coefficients
        if ((1 - cosom) > 0.001f) { // threshold, use linear interp if too close
            omega = acosf(cosom);
            sinom = sinf(omega);
            scale0 = sinf((1 - t) * omega) / sinom;
            scale1 = sinf(t * omega) / sinom;
        }
        else {
            scale0 = 1 - t;
            scale1 = t;
        }
        
        // calc final
        Spinor s;
        s.complex = scale0 * this->complex + scale1 * tc;
        s.real = scale0 * this->real + scale1 * tr;
        
        return s;
    }
    
    static float radianSlerp (float from, float to, float t) {
        Spinor fromS(cosf(from / 2), sinf(from / 2));
        Spinor toS(cosf(to / 2), sinf(to / 2));
        return fromS.slerp(toS, t).getRadian();
    }
    
};

// Flash 2D matrix manipulations
struct FMatrix2D {
    float a;
    float b;
    float c;
    float d;
    float tx;
    float ty;
    
    void setIdentity () {
        a = 1;
        b = 0;
        c = 0;
        d = 1;
        tx = 0;
        ty = 0;
    }
    
    FMatrix2D () {
        setIdentity();
    }
    
    float getScaleX () {
        return sqrtf(a*a + b*b);
    }
    
    float getScaleY () {
        return sqrtf(c*c + d*d);
    }
    
    float getSkewXRadians ()
    {
        return atan2f(-c, d);
    }
    
    float getSkewYRadians ()
    {
        return atan2f(b, a);
    }
    
    float getSkewX () {
        return atan2f(-c, d) * (180.0f/M_PI);
    }
    
    float getSkewY () {
        return atan2f(b, a) * (180.0f/M_PI);
    }
	
	float getTransformX () {
		return tx;
	}
	
	float getTransformY () {
		return ty;
	}
    
    void setScaleX (float scaleX) {
        float oldValue = getScaleX();
        // avoid division by zero
        if (oldValue)
        {
            float ratio = scaleX / oldValue;
            a *= ratio;
            b *= ratio;
        }
        else
        {
            float skewYRad = getSkewYRadians();
            a = cosf(skewYRad) * scaleX;
            b = sinf(skewYRad) * scaleX;
        }
    }
    
    void setScaleY (float scaleY) {
        float oldValue = getScaleY();
        // avoid division by zero
        if (oldValue)
        {
            float ratio = scaleY / oldValue;
            c *= ratio;
            d *= ratio;
        }
        else
        {
            float skewXRad = getSkewXRadians();
            c = -sinf(skewXRad) * scaleY;
            d =  cosf(skewXRad) * scaleY;
        }
    }
    
    void setSkewXRadians (float skewX) {
        float scaleY = getScaleY();
        c = -scaleY * sinf(skewX);
        d =  scaleY * cosf(skewX);
    }
    
    void setSkewYRadians (float skewY) {
        float scaleX = getScaleX();
        a = scaleX * cosf(skewY);
        b = scaleX * sinf(skewY);
    }
    
    void setSkewX (float skewX) {
        setSkewXRadians(skewX*(M_PI/180.0f));
    }
    
    void setSkewY (float skewY) {
        setSkewYRadians(skewY*(M_PI/180.0f));
    }
    
    void setAnchorX (float anchorX) {
        tx = -anchorX;
    }
    
    void setAnchorY (float anchorY) {
        ty = -anchorY;
    }
    
    void setTransformX (float transformX) {
        tx = transformX;
    }
    
    void setTransformY (float transformY) {
        ty = transformY;
    }
    
	//    FMatrix2D invert () {
	//        FMatrix2D ret;
	//        ret.a = d/(a*d-b*c);
	//        ret.b = -b/(a*d-b*c);
	//        ret.c = -c/(a*d-b*c);
	//        ret.d = a/(a*d-b*c);
	//        ret.tx = (c*ty-d*tx)/(a*d-b*c);
	//        ret.ty = -(a*ty-b*tx)/(a*d-b*c);
	//        return ret;
	//    }
    
	//    void translate (float translateX, float translateY) {
	//        tx = tx + a * translateX + c * translateY;
	//        ty = ty + b * translateX + d * translateY;
	//    }
    
    FMatrix2D concat (FMatrix2D& m) {
        FMatrix2D ret;
        ret.a = a * m.a + b * m.c;
        ret.b = a * m.b + b * m.d;
        ret.c = c * m.a + d * m.c;
        ret.d = c * m.b + d * m.d;
        ret.tx = tx * m.a + ty * m.c + m.tx;
        ret.ty = tx * m.b + ty * m.d + m.ty;
        return ret;
    }
    
};

class CC_DLL CCHierarchiesSpriteAnimation {
    
	//    friend class HierarchiesSprite;
    
public:
	
    struct Animation {
        std::string name;
        unsigned int startFrameIndex;
        unsigned int endFrameIndex;
        bool loop;
        
        Animation () {
        }
        
        Animation (std::string name, unsigned int startFrameIndex, unsigned int endFrameIndex, bool loop)
        : name(name), startFrameIndex(startFrameIndex), endFrameIndex(endFrameIndex), loop(loop) {
        }
        
        Animation (const Animation& copy) {
            this->name = copy.name;
            this->startFrameIndex = copy.startFrameIndex;
            this->endFrameIndex = copy.endFrameIndex;
            this->loop = copy.loop;
        }
        
        Animation& operator= (const Animation& rhs) {
            this->name = rhs.name;
            this->startFrameIndex = rhs.startFrameIndex;
            this->endFrameIndex = rhs.endFrameIndex;
            this->loop = rhs.loop;
            return *this;
        }
    };
    
    struct Element {
        int symbolIndex;
		//        std::string name;
        float x;
        float y;
        float anchorX;
        float anchorY;
        float scaleX;
        float scaleY;
        float rotation;
        float skewX;
        float skewY;
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
        int depth;
#endif
		//        float left;
		//        float top;
		//        float width;
		//        float height;
		//        float mat_a;
		//        float mat_b;
		//        float mat_c;
		//        float mat_d;
		//        float mat_tx;
		//        float mat_ty;
        float color_alpha_percent;
        int color_alpha_amount;
        float color_red_percent;
        int color_red_amount;
        float color_green_percent;
        int color_green_amount;
        float color_blue_percent;
        int color_blue_amount;
        
        Element ()
        : color_alpha_percent(1), color_red_percent(1), color_green_percent(1), color_blue_percent(1) {
        }
        
		//        Element (std::string name, float x, float y, float anchorX, float anchorY, float scaleX, float scaleY, float rotation, float skewX, float skewY, int depth, float top, float left, float width, float height, float mat_a, float mat_b, float mat_c, float mat_d, float mat_tx, float mat_ty)
		//        : name(name), x(x), y(y), anchorX(anchorX), anchorY(anchorY), scaleX(scaleX), scaleY(scaleY), rotation(rotation), skewX(skewX), skewY(skewY), depth(depth), top(top), left(left), width(width), height(height),  mat_a(mat_a), mat_b(mat_b), mat_c(mat_c), mat_d(mat_d), mat_tx(mat_tx), mat_ty(mat_ty) {
		//        }
		//        Element (std::string name, float x, float y, float anchorX, float anchorY, float scaleX, float scaleY, float rotation, float skewX, float skewY, int depth, float color_alpha_percent, int color_alpha_amount, float color_red_percent, int color_red_amount, float color_green_percent, int color_green_amount, float color_blue_percent, int color_blue_amount)
		//        : name(name), x(x), y(y), anchorX(anchorX), anchorY(anchorY), scaleX(scaleX), scaleY(scaleY), rotation(rotation), skewX(skewX), skewY(skewY), depth(depth), color_alpha_percent(color_alpha_percent), color_alpha_amount(color_alpha_amount), color_red_percent(color_red_percent), color_red_amount(color_red_amount), color_green_percent(color_green_percent), color_green_amount(color_green_amount), color_blue_percent(color_blue_percent), color_blue_amount(color_blue_amount) {
		//        }
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
        Element (int symbolIndex, float x, float y, float anchorX, float anchorY, float scaleX, float scaleY, float rotation, float skewX, float skewY, int depth, float color_alpha_percent, int color_alpha_amount, float color_red_percent, int color_red_amount, float color_green_percent, int color_green_amount, float color_blue_percent, int color_blue_amount)
        : symbolIndex(symbolIndex), x(x), y(y), anchorX(anchorX), anchorY(anchorY), scaleX(scaleX), scaleY(scaleY), rotation(rotation), skewX(skewX), skewY(skewY), depth(depth), color_alpha_percent(color_alpha_percent), color_alpha_amount(color_alpha_amount), color_red_percent(color_red_percent), color_red_amount(color_red_amount), color_green_percent(color_green_percent), color_green_amount(color_green_amount), color_blue_percent(color_blue_percent), color_blue_amount(color_blue_amount) {
        }
#else
		Element (int symbolIndex, float x, float y, float anchorX, float anchorY, float scaleX, float scaleY, float rotation, float skewX, float skewY, float color_alpha_percent, int color_alpha_amount, float color_red_percent, int color_red_amount, float color_green_percent, int color_green_amount, float color_blue_percent, int color_blue_amount)
        : symbolIndex(symbolIndex), x(x), y(y), anchorX(anchorX), anchorY(anchorY), scaleX(scaleX), scaleY(scaleY), rotation(rotation), skewX(skewX), skewY(skewY), color_alpha_percent(color_alpha_percent), color_alpha_amount(color_alpha_amount), color_red_percent(color_red_percent), color_red_amount(color_red_amount), color_green_percent(color_green_percent), color_green_amount(color_green_amount), color_blue_percent(color_blue_percent), color_blue_amount(color_blue_amount) {
        }
#endif
        
        Element (const Element& copy) {
            this->symbolIndex = copy.symbolIndex;
			//            this->name = copy.name;
            this->x = copy.x;
            this->y = copy.y;
            this->anchorX = copy.anchorX;
            this->anchorY = copy.anchorY;
            this->scaleX = copy.scaleX;
            this->scaleY = copy.scaleY;
            this->rotation = copy.rotation;
            this->skewX = copy.skewX;
            this->skewY = copy.skewY;
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
            this->depth = copy.depth;
#endif
			//            this->top = copy.top;
			//            this->left = copy.left;
			//            this->width = copy.width;
			//            this->height = copy.height;
			//            this->mat_a = copy.mat_a;
			//            this->mat_b = copy.mat_b;
			//            this->mat_c = copy.mat_c;
			//            this->mat_d = copy.mat_d;
			//            this->mat_tx = copy.mat_tx;
			//            this->mat_ty = copy.mat_ty;
            this->color_alpha_percent = copy.color_alpha_percent;
            this->color_alpha_amount = copy.color_alpha_amount;
            this->color_red_percent = copy.color_red_percent;
            this->color_red_amount = copy.color_red_amount;
            this->color_green_percent = copy.color_green_percent;
            this->color_green_amount = copy.color_green_amount;
            this->color_blue_percent = copy.color_blue_percent;
            this->color_blue_amount = copy.color_blue_amount;
        }
        
        Element& operator= (const Element& rhs) {
            this->symbolIndex = rhs.symbolIndex;
			//            this->name = rhs.name;
            this->x = rhs.x;
            this->y = rhs.y;
            this->anchorX = rhs.anchorX;
            this->anchorY = rhs.anchorY;
            this->scaleX = rhs.scaleX;
            this->scaleY = rhs.scaleY;
            this->rotation = rhs.rotation;
            this->skewX = rhs.skewX;
            this->skewY = rhs.skewY;
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
            this->depth = rhs.depth;
#endif
			//            this->top = rhs.top;
			//            this->left = rhs.left;
			//            this->width = rhs.width;
			//            this->height = rhs.height;
			//            this->mat_a = rhs.mat_a;
			//            this->mat_b = rhs.mat_b;
			//            this->mat_c = rhs.mat_c;
			//            this->mat_d = rhs.mat_d;
			//            this->mat_tx = rhs.mat_tx;
			//            this->mat_ty = rhs.mat_ty;
            this->color_alpha_percent = rhs.color_alpha_percent;
            this->color_alpha_amount = rhs.color_alpha_amount;
            this->color_red_percent = rhs.color_red_percent;
            this->color_red_amount = rhs.color_red_amount;
            this->color_green_percent = rhs.color_green_percent;
            this->color_green_amount = rhs.color_green_amount;
            this->color_blue_percent = rhs.color_blue_percent;
            this->color_blue_amount = rhs.color_blue_amount;
            return *this;
        }
        
        float lerpf (float src, float target, float t) {
            return (src + t * (target - src));
        }
        
        int lerpi (int src, int target, float t) {
            return (src + t * (target - src));
        }
        
        Element lerp (const Element& target, float t) {
            Element ret;
            ret.symbolIndex = this->symbolIndex;
			//            ret.name = this->name;
            ret.x = lerpf(this->x, target.x, t);
            ret.y = lerpf(this->y, target.y, t);
            ret.anchorX = this->anchorX;
            ret.anchorY = this->anchorY;
            ret.scaleX = lerpf(this->scaleX, target.scaleX, t);
            ret.scaleY = lerpf(this->scaleY, target.scaleY, t);
            ret.rotation = Spinor::radianSlerp(this->rotation * (M_PI / 180.0f), target.rotation * (M_PI / 180.0f), t) * (180.0f / M_PI);
            // //HACK: since the rotation direction is different between rotation and skew
            // if ( ret.rotation != ret.rotation ) {
                ret.skewX = Spinor::radianSlerp(this->skewX * (M_PI / 180.0f), target.skewX * (M_PI / 180.0f), t) * (180.0f / M_PI);
                ret.skewY = Spinor::radianSlerp(this->skewY * (M_PI / 180.0f), target.skewY * (M_PI / 180.0f), t) * (180.0f / M_PI);
            // }
            // else {
            //     ret.skewX = lerpf(this->skewX, target.skewX, t);
            //     ret.skewY = lerpf(this->skewY, target.skewY, t);
            // }
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
            ret.depth = this->depth;
#endif
			//            ret.top = this->top;
			//            ret.left = this->left;
			//            ret.width = this->width;
			//            ret.height = this->height;
			//            ret.mat_a = lerpf(this->mat_a, target.mat_a, t);
			//            ret.mat_b = lerpf(this->mat_b, target.mat_b, t);
			//            ret.mat_c = lerpf(this->mat_c, target.mat_c, t);
			//            ret.mat_d = lerpf(this->mat_d, target.mat_d, t);
			//            ret.mat_tx = lerpf(this->mat_tx, target.mat_tx, t);
			//            ret.mat_ty = lerpf(this->mat_ty, target.mat_ty, t);
            ret.color_alpha_percent = lerpf(this->color_alpha_percent, target.color_alpha_percent, t);
            ret.color_alpha_amount = lerpi(this->color_alpha_amount, target.color_alpha_amount, t);
            ret.color_red_percent = lerpf(this->color_red_percent, target.color_red_percent, t);
            ret.color_red_amount = lerpi(this->color_red_amount, target.color_red_amount, t);
            ret.color_green_percent = lerpf(this->color_green_percent, target.color_green_percent, t);
            ret.color_green_amount = lerpi(this->color_green_amount, target.color_green_amount, t);
            ret.color_blue_percent = lerpf(this->color_blue_percent, target.color_blue_percent, t);
            ret.color_blue_amount = lerpi(this->color_blue_amount, target.color_blue_amount, t);
            return ret;
        }
        
		//        bool isRotationTransform () {
		//            return !(rotation != rotation); // if rotation is NaN means this element use skew transform
		//        }
    };
    
    struct KeyFrame {
        unsigned int id;
        unsigned int duration;
		//        std::vector<Element> begin;
		//        std::vector<Element> end;
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
        std::vector<Element> elements;
#else
		Element element;
		bool isEmpty; // means no element
#endif
        bool isMotion;
        
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
		KeyFrame () {
        }
        
        KeyFrame (unsigned int id, unsigned int duration, bool isMotion)
        : id(id), duration(duration), isMotion(isMotion) {
        }
#else
        KeyFrame () 
		: isEmpty(true) {
        }
        
        KeyFrame (unsigned int id, unsigned int duration, bool isMotion)
        : id(id), duration(duration), isMotion(isMotion), isEmpty(true) {
        }
#endif
        
        KeyFrame (const KeyFrame& copy) {
            this->id = copy.id;
            this->duration = copy.duration;
			//            this->begin = copy.begin;
			//            this->end = copy.end;
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
            this->elements = copy.elements;
#else
			this->element = copy.element;
			this->isEmpty = copy.isEmpty;
#endif
            this->isMotion = copy.isMotion;
        }
        
        KeyFrame& operator= (const KeyFrame& rhs) {
            this->id = rhs.id;
            this->duration = rhs.duration;
			//            this->begin = rhs.begin;
			//            this->end = rhs.end;
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
            this->elements = rhs.elements;
#else
			this->element = rhs.element;
			this->isEmpty = rhs.isEmpty;
#endif
            this->isMotion = rhs.isMotion;
            return *this;
        }
        
		//        bool findElementByName (std::string name, Element& out) {
		//            std::vector<Element>::iterator iter;
		//            for (iter = elements.begin(); iter != elements.end(); iter++) {
		//                if (iter->name.compare(name) == 0) {
		//                    out = *iter;
		//                    return true;
		//                }
		//            }
		//            return false;
		//        }
        
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
        bool findElementBySymbolIndex (int symbolIndex, Element& out) {
            std::vector<Element>::iterator iter;
            for (iter = elements.begin(); iter != elements.end(); iter++) {
                if (iter->symbolIndex == symbolIndex) {
                    out = *iter;
                    return true;
                }
            }
            return false;
        }
#endif
    };
    
    struct Layer {
        std::string name;
        std::vector<KeyFrame> frames;
        
        Layer (std::string name)
        : name(name) {
        }
        
        Layer (const Layer& copy) {
            this->name = copy.name;
            this->frames = copy.frames;
        }
        
        Layer& operator= (const Layer& rhs) {
            this->name = rhs.name;
            this->frames = rhs.frames;
            return *this;
        }
    };
    
    struct Item {
        std::string name;
        float left;
        float bottom;
        int dyeingPartItemIndex; // -1 means no dyeing part
        
        Item () 
        : dyeingPartItemIndex(-1) {
        }
        
        Item (std::string name, float left, float bottom, int dyeingPartItemIndex)
        : name(name), left(left), bottom(bottom), dyeingPartItemIndex(dyeingPartItemIndex) {
        }
        
        Item (const Item& copy) {
            this->name = copy.name;
            this->left = copy.left;
            this->bottom = copy.bottom;
            this->dyeingPartItemIndex = copy.dyeingPartItemIndex;
        }
        
        Item& operator= (const Item& rhs) {
            this->name = rhs.name;
            this->left = rhs.left;
            this->bottom = rhs.bottom;
            this->dyeingPartItemIndex = rhs.dyeingPartItemIndex;
            return *this;
        }
    };
    
    struct Symbol {
        std::string name;
        int defaultItemIndex;
        
        Symbol () {}
        
        Symbol (std::string name, int defaultItemIndex)
        : name(name), defaultItemIndex(defaultItemIndex) {
        }
        
        Symbol (const Symbol& copy) {
            this->name = copy.name;
            this->defaultItemIndex = copy.defaultItemIndex;
        }
        
        Symbol& operator= (const Symbol& rhs) {
            this->name = rhs.name;
            this->defaultItemIndex = rhs.defaultItemIndex;
            return *this;
        }
    };
    
    struct Event {
        unsigned int frameId;
        std::string content;
        
        Event () {
        }
        
        Event (unsigned int frameId, std::string content)
        : frameId(frameId), content(content) {
        }
        
        Event (const Event& copy) {
            this->frameId = copy.frameId;
            this->content = copy.content;
        }
        
        Event& operator= (const Event& rhs) {
            this->frameId = rhs.frameId;
            this->content = rhs.content;
            return *this;
        }
    };
    
	//    struct ElementInfo {
	//        int symbolIndex;
	//        
	//        ElementInfo () {}
	//        
	//        ElementInfo (int symbolIndex)
	//        : symbolIndex(symbolIndex) {
	//        }
	//        
	//        ElementInfo (const ElementInfo& copy) {
	//            this->symbolIndex = copy.symbolIndex;
	//        }
	//        
	//        ElementInfo& operator= (const ElementInfo& rhs) {
	//            this->symbolIndex = rhs.symbolIndex;
	//            return *this;
	//        }
	//    };
    
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
    typedef std::vector< std::vector<Element> > FrameElements;
#else
	typedef std::vector< Element > FrameElements;
#endif
	//    typedef std::vector< std::vector<ElementInfo> > FrameElementsInfo;
    
private:
    std::string _version;
    unsigned int _frameRate;
    std::vector<Item> _items;
    std::map<std::string, Animation> _anims;
    std::vector<Layer> _layers;
    std::vector<Symbol> _symbols;
    std::vector<Event> _events;
    unsigned int _frameSum; // base on 0
    
private:
	void parseItems (rapidxml::xml_node<>* itemsNode);
	void parseSymbols (rapidxml::xml_node<>* symbolsNode);
	void parseAnimations (rapidxml::xml_node<>* animationsNode);
	void parseEvents (rapidxml::xml_node<>* eventsNode);
	void parseLayers (rapidxml::xml_node<>* layersNode);
	void parseKeyFrames (Layer& layer, rapidxml::xml_node<>* layerNode);
	void parseElements (KeyFrame& frame, rapidxml::xml_node<>* keyFrameNode);
    
public:
    CCHierarchiesSpriteAnimation (std::string xmlFile);
    //    CCHierarchiesSpriteAnimation (const CCHierarchiesSpriteAnimation& copy);
    virtual ~CCHierarchiesSpriteAnimation();
    
    std::string getVersion ();
    unsigned int getFrameRate ();
    size_t getAnimationCount ();
	size_t getFrameCount ();
    bool getAnimationByName (std::string name, Animation& out);
	std::vector<Animation> getAnimationList ();
    size_t getLayerCount ();
    bool getLayerByIndex (unsigned int index, Layer& out);
    bool getKeyFrameIncludeIndexAtLayer (unsigned int layerIndex, unsigned int index, KeyFrame& out);
    int getItemByNameReturnIndex (std::string name, Item& out);
    bool getItemByIndex (int itemIndex, Item& out);
    size_t getItemCount ();
    bool getSymbolByIndex (int symbolIndex, Symbol& out);
    size_t getSymbolCount ();
    bool getEventByFrameId (unsigned int frameId, Event& out);
    int getFrameElementsAtIndex (unsigned int index, FrameElements& out);
	//    int getFrameElementsInfoAtIndex (unsigned int index, FrameElementsInfo& out);
    
};

NS_CC_EXT_END

#endif
