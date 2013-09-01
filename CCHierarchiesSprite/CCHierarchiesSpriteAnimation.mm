//
//  CCHierarchiesSpriteAnimation.cpp
//  CCHierarchiesSprite
//
//  Created by bman <zx123xz321hm3@hotmail.com>.
//  Copyright (c) 2013. All rights reserved.
//

#include "CCHierarchiesSpriteAnimation.h"
#import "CCFileUtils.h"

using namespace rapidxml;


CCHierarchiesSpriteAnimation::CCHierarchiesSpriteAnimation (std::string xmlFile) {
	// load file
    NSString* filePath = [[CCFileUtils sharedFileUtils] fullPathFromRelativePath:[NSString stringWithUTF8String:xmlFile.c_str()]];
    NSData* data = [[NSData alloc] initWithContentsOfFile:filePath];
    assert(data);
	char* xml = (char*)malloc(data.length + 1);
	memcpy(xml, data.bytes, data.length);
	xml[data.length] = '\0'; // add string end char
	[data release];
	
    // parse xml
    xml_document<> doc;
	doc.parse<0>(xml);
    
    // <Hierarchies version=S frameRate=N>
	xml_node<>* hierarchiesNode = doc.first_node("Hierarchies");
	if (NULL == hierarchiesNode) {
		CCLOG(@"parse <Hierarchies> Node error");
		free(xml);
		return;
	}
    
	xml_attribute<>* hierarchies_version = hierarchiesNode->first_attribute("version");
	if (NULL == hierarchies_version) {
		CCLOG(@"parse <Hierarchies> Node <version> Attr error");
		free(xml);
		return;
	}
    _version = hierarchies_version->value();
    
	xml_attribute<>* hierarchies_frameRate = hierarchiesNode->first_attribute("frameRate");
	if (NULL == hierarchies_frameRate) {
		CCLOG(@"parse <Hierarchies> Node <frameRate> Attr error");
		free(xml);
		return;
	}
    _frameRate = atoi(hierarchies_frameRate->value());
    
    // <items>
	xml_node<>* itemsNode = hierarchiesNode->first_node("items");
	if (NULL == itemsNode) {
		CCLOG(@"parse <items> Node error");
		free(xml);
		return;
	}
	
    // parse Item
	//    parseItem(getNextXmlElement(itemsNode->children), doc);
	parseItems(itemsNode);
    
    // <symbols>
	xml_node<>* symbolsNode = hierarchiesNode->first_node("symbols");
	if (NULL == symbolsNode) {
		CCLOG(@"parse <symbols> Node error");
		free(xml);
		return;
	}
    
    // parse Symbol
	parseSymbols(symbolsNode);
    
    // <anims>
	xml_node<>* animsNode = hierarchiesNode->first_node("anims");
	if (NULL == animsNode) {
		CCLOG(@"parse <anims> Node error");
		free(xml);
		return;
	}
    
    // parse Animation
	//    parseAnimation(getNextXmlElement(animsNode->children), doc);
	parseAnimations(animsNode);
    
    // <events>
	xml_node<>* eventsNode = hierarchiesNode->first_node("events");
	if (NULL == eventsNode) {
		CCLOG(@"parse <events> Node error");
		free(xml);
		return;
	}
    
    // parse Event
	parseEvents(eventsNode);
    
    // <layers>
	xml_node<>* layersNode = hierarchiesNode->first_node("layers");
	if (NULL == layersNode) {
		CCLOG(@"parse <layers> Node error");
		free(xml);
		return;
	}
    
    // parse Layer, KeyFrame and Element
	parseLayers(layersNode);
    
    // parse xml end
	free(xml);
    
    // cache frame sum
    _frameSum = 0;
#ifdef HIERARCHIES_USE_CPP_11
    std::unordered_map<std::string, Animation>::iterator iter;
#else
    std::map<std::string, Animation>::iterator iter;
#endif
    for (iter = _anims.begin(); iter != _anims.end(); iter++) {
        if (iter->second.endFrameIndex > _frameSum)
            _frameSum = iter->second.endFrameIndex;
    }
}

void CCHierarchiesSpriteAnimation::parseItems (rapidxml::xml_node<>* itemsNode) {
	// <Item name=S left=N bottom=N>
	xml_node<>* itemNode = itemsNode->first_node("Item");
	while (itemNode) {
		xml_attribute<>* attribute = NULL;
		
		attribute = itemNode->first_attribute("name");
		if (NULL == attribute) {
			CCLOG(@"parse <Item> Node <name> Attr error");
			return;
		}
		std::string Item_name(attribute->value());
		
		attribute = itemNode->first_attribute("left");
		if (NULL == attribute) {
			CCLOG(@"parse <Item> Node <left> Attr error");
			return;
		}
		float Item_left = atof(attribute->value());
		
		attribute = itemNode->first_attribute("bottom");
		if (NULL == attribute) {
			CCLOG(@"parse <Item> Node <bottom> Attr error");
			return;
		}
		float Item_bottom = atof(attribute->value());
		
		_items.push_back(Item(Item_name, Item_left, Item_bottom));
		
		itemNode = itemNode->next_sibling("Item");
	}
}

void CCHierarchiesSpriteAnimation::parseAnimations (rapidxml::xml_node<>* animationsNode) {
	// <Animation name=S startFrameIndex=N endFrameIndex=N>
	xml_node<>* animationNode = animationsNode->first_node("Animation");
	while (animationNode) {
		xml_attribute<>* attribute = NULL;
		
		attribute = animationNode->first_attribute("name");
		if (NULL == attribute) {
			CCLOG(@"parse <Animation> Node <name> Attr error");
			return;
		}
		std::string Animation_name(attribute->value());
		
		attribute = animationNode->first_attribute("startFrameIndex");
		if (NULL == attribute) {
			CCLOG(@"parse <Animation> Node <startFrameIndex> Attr error");
			return;
		}
		unsigned int Animation_startFrameIndex = atoi(attribute->value());
		
		attribute = animationNode->first_attribute("endFrameIndex");
		if (NULL == attribute) {
			CCLOG(@"parse <Animation> Node <endFrameIndex> Attr error");
			return;
		}
		unsigned int Animation_endFrameIndex = atoi(attribute->value());
		
		std::pair<std::string, Animation> item(Animation_name, Animation(Animation_name, Animation_startFrameIndex, Animation_endFrameIndex));
        _anims.insert(item);
		
		animationNode = animationNode->next_sibling("Animation");
	}
}

void CCHierarchiesSpriteAnimation::parseLayers (rapidxml::xml_node<>* layersNode) {
	// <Layer name=S>
	xml_node<>* layerNode = layersNode->first_node("Layer");
	while (layerNode) {
		xml_attribute<>* attribute = NULL;
		
		attribute = layerNode->first_attribute("name");
		if (NULL == attribute) {
			CCLOG(@"parse <Layer> Node <name> Attr error");
			return;
		}
		std::string Layer_name(attribute->value());
		
		Layer layer(Layer_name);
		parseKeyFrames(layer, layerNode);
		_layers.push_back(layer);
		
		layerNode = layerNode->next_sibling("Layer");
	}
}

void CCHierarchiesSpriteAnimation::parseSymbols (rapidxml::xml_node<>* symbolsNode) {
	// <Symbol name=S defaultItemIndex=I>
	xml_node<>* symbolNode = symbolsNode->first_node("Symbol");
	while (symbolNode) {
		xml_attribute<>* attribute = NULL;
		
		attribute = symbolNode->first_attribute("name");
		if (NULL == attribute) {
			CCLOG(@"parse <Symbol> Node <name> Attr error");
			return;
		}
		std::string Symbol_name(attribute->value());
		
		attribute = symbolNode->first_attribute("defaultItemIndex");
		if (NULL == attribute) {
			CCLOG(@"parse <Symbol> Node <defaultItemIndex> Attr error");
			return;
		}
		int Symbol_defaultItemIndex = atoi(attribute->value());
		
		_symbols.push_back(Symbol(Symbol_name, Symbol_defaultItemIndex));
		
		symbolNode = symbolNode->next_sibling("Symbol");
	}
}

void CCHierarchiesSpriteAnimation::parseEvents (rapidxml::xml_node<>* eventsNode) {
	// <Event frameId=N content=S>
	xml_node<>* eventNode = eventsNode->first_node("Event");
	while (eventNode) {
		xml_attribute<>* attribute = NULL;
		
		attribute = eventNode->first_attribute("frameId");
		if (NULL == attribute) {
			CCLOG(@"parse <Event> Node <frameId> Attr error");
			return;
		}
		unsigned int Event_frameId = atoi(attribute->value());
		
		attribute = eventNode->first_attribute("content");
		if (NULL == attribute) {
			CCLOG(@"parse <Event> Node <content> Attr error");
			return;
		}
		std::string Event_content(attribute->value());
		
		_events.push_back(Event(Event_frameId, Event_content));
		
		eventNode = eventNode->next_sibling("Event");
	}
}

void CCHierarchiesSpriteAnimation::parseKeyFrames (Layer& layer, rapidxml::xml_node<>* layerNode) {
	// <KeyFrame id=N duration=N isMotion=B>
	xml_node<>* keyFrameNode = layerNode->first_node("KeyFrame");
	while (keyFrameNode) {
		xml_attribute<>* attribute = NULL;
		
		attribute = keyFrameNode->first_attribute("id");
		if (NULL == attribute) {
			CCLOG(@"parse <KeyFrame> Node <id> Attr error");
			return;
		}
		unsigned int KeyFrame_id = atoi(attribute->value());
		
		attribute = keyFrameNode->first_attribute("duration");
		if (NULL == attribute) {
			CCLOG(@"parse <KeyFrame> Node <duration> Attr error");
			return;
		}
		unsigned int KeyFrame_duration = atoi(attribute->value());
		
		attribute = keyFrameNode->first_attribute("isMotion");
		if (NULL == attribute) {
			CCLOG(@"parse <KeyFrame> Node <isMotion> Attr error");
			return;
		}
		bool KeyFrame_isMotion = false;
        if (strcmp(attribute->value(), "true") == 0) {
            KeyFrame_isMotion = true;
        }
        else {
            KeyFrame_isMotion = false;
        }
		
		KeyFrame frame(KeyFrame_id, KeyFrame_duration, KeyFrame_isMotion);
		parseElements(frame, keyFrameNode);
		layer.frames.push_back(frame);
		
		keyFrameNode = keyFrameNode->next_sibling("KeyFrame");
	}
}

void CCHierarchiesSpriteAnimation::parseElements (KeyFrame& frame, rapidxml::xml_node<>* keyFrameNode) {
	// <Element symbolIndex=N x=F y=F anchorX=F anchorY=F scaleX=F scaleY=F rotation=F(sometimes NaN) skewX=F skewY=F depth=N >
	xml_node<>* elementNode = keyFrameNode->first_node("Element");
	while (elementNode) {
		xml_attribute<>* attribute = NULL;
		
		attribute = elementNode->first_attribute("symbolIndex");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <symbolIndex> Attr error");
			return;
		}
		int Element_symbolIndex = atoi(attribute->value());
		
		attribute = elementNode->first_attribute("x");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <x> Attr error");
			return;
		}
		float Element_x = atof(attribute->value());
		
		attribute = elementNode->first_attribute("y");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <y> Attr error");
			return;
		}
		float Element_y = atof(attribute->value());
		
		attribute = elementNode->first_attribute("anchorX");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <anchorX> Attr error");
			return;
		}
		float Element_anchorX = atof(attribute->value());
		
		attribute = elementNode->first_attribute("anchorY");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <anchorY> Attr error");
			return;
		}
		float Element_anchorY = atof(attribute->value());
		
		attribute = elementNode->first_attribute("scaleX");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <scaleX> Attr error");
			return;
		}
		float Element_scaleX = atof(attribute->value());
		
		attribute = elementNode->first_attribute("scaleY");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <scaleY> Attr error");
			return;
		}
		float Element_scaleY = atof(attribute->value());
		
		attribute = elementNode->first_attribute("rotation");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <rotation> Attr error");
			return;
		}
		float Element_rotation = atof(attribute->value());
		
		attribute = elementNode->first_attribute("skewX");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <skewX> Attr error");
			return;
		}
		float Element_skewX = atof(attribute->value());
		
		attribute = elementNode->first_attribute("skewY");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <skewY> Attr error");
			return;
		}
		float Element_skewY = atof(attribute->value());
		
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
		attribute = elementNode->first_attribute("depth");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <depth> Attr error");
			return;
		}
		int Element_depth = atoi(attribute->value());
#endif
		
		attribute = elementNode->first_attribute("color_alpha_percent");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <color_alpha_percent> Attr error");
			return;
		}
		float Element_color_alpha_percent = atof(attribute->value());
		
		attribute = elementNode->first_attribute("color_alpha_amount");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <color_alpha_amount> Attr error");
			return;
		}
		int Element_color_alpha_amount = atoi(attribute->value());
		
		attribute = elementNode->first_attribute("color_red_percent");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <color_red_percent> Attr error");
			return;
		}
		float Element_color_red_percent = atof(attribute->value());
		
		attribute = elementNode->first_attribute("color_red_amount");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <color_red_amount> Attr error");
			return;
		}
		int Element_color_red_amount = atoi(attribute->value());
		
		attribute = elementNode->first_attribute("color_green_percent");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <color_green_percent> Attr error");
			return;
		}
		float Element_color_green_percent = atof(attribute->value());
		
		attribute = elementNode->first_attribute("color_green_amount");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <color_green_amount> Attr error");
			return;
		}
		int Element_color_green_amount = atoi(attribute->value());
		
		attribute = elementNode->first_attribute("color_blue_percent");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <color_blue_percent> Attr error");
			return;
		}
		float Element_color_blue_percent = atof(attribute->value());
		
		attribute = elementNode->first_attribute("color_blue_amount");
		if (NULL == attribute) {
			CCLOG(@"parse <Element> Node <color_blue_amount> Attr error");
			return;
		}
		int Element_color_blue_amount = atoi(attribute->value());
		
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
		Element element(Element_symbolIndex, Element_x, Element_y, Element_anchorX, Element_anchorY, Element_scaleX, Element_scaleY, Element_rotation, Element_skewX, Element_skewY, Element_depth, Element_color_alpha_percent, Element_color_alpha_amount, Element_color_red_percent, Element_color_red_amount, Element_color_green_percent, Element_color_green_amount, Element_color_blue_percent, Element_color_blue_amount);
		
		frame.elements.push_back(element);
		
		elementNode = elementNode->next_sibling("Element");
#else
		Element element(Element_symbolIndex, Element_x, Element_y, Element_anchorX, Element_anchorY, Element_scaleX, Element_scaleY, Element_rotation, Element_skewX, Element_skewY, Element_color_alpha_percent, Element_color_alpha_amount, Element_color_red_percent, Element_color_red_amount, Element_color_green_percent, Element_color_green_amount, Element_color_blue_percent, Element_color_blue_amount);
		
		frame.element = element;
		frame.isEmpty = false;
		
		break; // support only one element
#endif
	}
}

CCHierarchiesSpriteAnimation::~CCHierarchiesSpriteAnimation () {
}

std::string CCHierarchiesSpriteAnimation::getVersion () {
    return _version;
}

unsigned int CCHierarchiesSpriteAnimation::getFrameRate () {
    return _frameRate;
}

size_t CCHierarchiesSpriteAnimation::getAnimationCount () {
    return _anims.size();
}

size_t CCHierarchiesSpriteAnimation::getFrameCount() {
	return _frameSum + 1;
}

const std::vector<CCHierarchiesSpriteAnimation::Layer>& CCHierarchiesSpriteAnimation::getLayers () {
    return _layers;
}

const std::vector<CCHierarchiesSpriteAnimation::Symbol>& CCHierarchiesSpriteAnimation::getSymbols () {
    return _symbols;
}

const std::vector<CCHierarchiesSpriteAnimation::Item>& CCHierarchiesSpriteAnimation::getItems () {
    return _items;
}

const std::vector<CCHierarchiesSpriteAnimation::Event>& CCHierarchiesSpriteAnimation::getEvents () {
    return _events;
}

bool CCHierarchiesSpriteAnimation::getAnimationByName (std::string name, Animation& out) {
#ifdef HIERARCHIES_USE_CPP_11
    std::unordered_map<std::string, Animation>::iterator iter = _anims.find(name);
#else
    std::map<std::string, Animation>::iterator iter = _anims.find(name);
#endif
    if (iter != _anims.end()) {
        out = iter->second;
        return true;
    }
    return false;
}

std::vector<CCHierarchiesSpriteAnimation::Animation> CCHierarchiesSpriteAnimation::getAnimationList () {
	std::vector<Animation> ret;
#ifdef HIERARCHIES_USE_CPP_11
	std::unordered_map<std::string, Animation>::iterator iter;
#else
    std::map<std::string, Animation>::iterator iter;
#endif
	for (iter = _anims.begin(); iter != _anims.end(); iter++) {
		ret.push_back(iter->second);
	}
	return ret;
}

size_t CCHierarchiesSpriteAnimation::getLayerCount () {
    return _layers.size();
}

bool CCHierarchiesSpriteAnimation::getLayerByIndex (unsigned int index, Layer& out) {
    if (index >= _layers.size())
        return false;
    
    out = _layers[index];
    return true;
}

bool CCHierarchiesSpriteAnimation::getKeyFrameIncludeIndexAtLayer (unsigned int layerIndex, unsigned int index, KeyFrame& out) {
    if (layerIndex >= _layers.size())
        return false;
    
    std::vector<KeyFrame>::iterator iter;
    for (iter = _layers[layerIndex].frames.begin(); iter != _layers[layerIndex].frames.end(); iter++) {
        if (index >= iter->id && index < (iter->id + iter->duration)) {
            out = *iter;
            return true;
        }
    }
    
    return false;
}

int CCHierarchiesSpriteAnimation::getItemByNameReturnIndex(std::string name, Item& out) {
    for (int i = 0; i < _items.size(); i++) {
        if (_items[i].name.compare(name) == 0) {
            out = _items[i];
            return i;
        }
    }
    return -1;
}

bool CCHierarchiesSpriteAnimation::getItemByIndex (int itemIndex, Item& out) {
    if (itemIndex >= _items.size())
        return false;
    
    out = _items[itemIndex];
    return true;
}

size_t CCHierarchiesSpriteAnimation::getItemCount () {
    return _items.size();
}

bool CCHierarchiesSpriteAnimation::getSymbolByIndex(int symbolIndex, Symbol& out) {
    if (symbolIndex >= _symbols.size())
        return false;
    
    out = _symbols[symbolIndex];
    return true;
}

size_t CCHierarchiesSpriteAnimation::getSymbolCount() {
    return _symbols.size();
}

bool CCHierarchiesSpriteAnimation::getEventByFrameId (unsigned int frameId, Event& out) {
    std::vector<Event>::iterator evtIter;
    for (evtIter = _events.begin(); evtIter != _events.end(); evtIter++) {
        if (evtIter->frameId == frameId) {
            out = *evtIter;
            return true;
        }
    }
    return false;
}

int CCHierarchiesSpriteAnimation::getFrameElementsAtIndex (unsigned int index, FrameElements& out) {
    if (index > _frameSum)
        return -1;
    
	//    out.clear();
    
    int eNum = 0;
    std::vector<Layer>::iterator layerIter;
    for (layerIter = _layers.begin(); layerIter != _layers.end(); layerIter++) {
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
        std::vector<Element> elements;
#endif
        
        std::vector<KeyFrame>::iterator keyFrameIter;
        for (keyFrameIter = layerIter->frames.begin(); keyFrameIter != layerIter->frames.end(); keyFrameIter++) {
            if (index >= keyFrameIter->id && index < (keyFrameIter->id + keyFrameIter->duration)) {
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 1
				if (keyFrameIter->isEmpty == true)
					break;
#endif
				
                if (keyFrameIter->isMotion == false) {
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
                    for (int i = 0; i < keyFrameIter->elements.size(); i++) {
                        Element& e = keyFrameIter->elements.at(i);
                        elements.push_back(e);
                        eNum++;
                    }
#else
					out.push_back(keyFrameIter->element);
					eNum++;
#endif
                }
                else {
					//                if (index == keyFrameIter->id) {
					//                    elements = keyFrameIter->begin;
					//                }
					//                else if (index == (keyFrameIter->id + keyFrameIter->duration - 1)) {
					//                    elements = keyFrameIter->end;
					//                }
					//                else {
					//                    for (int i = 0; i < keyFrameIter->begin.size(); i++) {
					//                        Element& e_src = keyFrameIter->begin.at(i);
					//                        Element& e_target = keyFrameIter->end.at(i);
					//                        float t = (float)(index - keyFrameIter->id) / (float)(keyFrameIter->duration - 1);
					//                        elements.push_back(e_src.lerp(e_target, t));
					//                    }
					//                }
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
					//                    for (int i = 0; i < keyFrameIter->elements.size(); i++) {
					//                        if (keyFrameIter->duration == 1) {
					//                            Element& e = keyFrameIter->elements.at(i);
					//                            elements.push_back(e);
					//                        }
					//                        else {
					//                            Element& e_src = keyFrameIter->elements.at(i);
					//                            Element e_target;
					//							//                        if ( (keyFrameIter+1) != layerIter->frames.end() && (keyFrameIter+1)->findElementByName(e_src.name, e_target)) {
					//                            if ( (keyFrameIter+1) != layerIter->frames.end() && (keyFrameIter+1)->findElementBySymbolIndex(e_src.symbolIndex, e_target)) {
					//                                float t = (float)(index - keyFrameIter->id) / (float)(keyFrameIter->duration - 1);
					//                                elements.push_back(e_src.lerp(e_target, t));
					//                            }
					//                            else {
					//                                elements.push_back(e_src);
					//                            }
					//                        }
					//                        eNum++;
					//                    }
					
					// lerp frames only there are one element in both frames
					if (keyFrameIter->elements.size() == 1) {
						if (keyFrameIter->duration == 1) {
                            Element& e = keyFrameIter->elements.at(0);
                            elements.push_back(e);
                        }
						else {
							Element& e_src = keyFrameIter->elements.at(0);
							if ( (keyFrameIter+1) != layerIter->frames.end() && 
								(keyFrameIter+1)->elements.size() == 1 ) {
								Element e_target = (keyFrameIter+1)->elements.at(0);
								float t = (float)(index - keyFrameIter->id) / (float)(keyFrameIter->duration - 1);
								elements.push_back(e_src.lerp(e_target, t));
							}
							else {
								elements.push_back(e_src);
							}
						}
						eNum++;
					}
					else {
						for (int i = 0; i < keyFrameIter->elements.size(); i++) {
							Element& e = keyFrameIter->elements.at(i);
							elements.push_back(e);
							eNum++;
						}
					}
#else
					if (keyFrameIter->duration == 1) {
						out.push_back(keyFrameIter->element);
					}
					else {
						Element& e_src = keyFrameIter->element;
						if ( (keyFrameIter+1) != layerIter->frames.end() && (keyFrameIter+1)->isEmpty == false) {
							Element e_target = (keyFrameIter+1)->element;
							float t = (float)(index - keyFrameIter->id) / (float)(keyFrameIter->duration - 1);
							out.push_back(e_src.lerp(e_target, t));
						}
						else {
							out.push_back(e_src);
						}
					}
					eNum++;
#endif
                }
                break;
            }
        }
        
#if HIERARCHIES_SPRITE_ONE_ELEMENT_IN_ONE_FRAME == 0
        out.push_back(elements);
#endif
    }
    
    return eNum;
}

//int CCHierarchiesSpriteAnimation::getFrameElementsInfoAtIndex (unsigned int index, FrameElementsInfo& out) {
//    if (index > _frameSum)
//        return -1;
//    
//    //    out.clear();
//    
//    int eNum = 0;
//    std::vector<Layer>::iterator layerIter;
//    for (layerIter = _layers.begin(); layerIter != _layers.end(); layerIter++) {
//        std::vector<ElementInfo> elementsInfo;
//        
//        std::vector<KeyFrame>::iterator keyFrameIter;
//        for (keyFrameIter = layerIter->frames.begin(); keyFrameIter != layerIter->frames.end(); keyFrameIter++) {
//            if (index >= keyFrameIter->id && index < (keyFrameIter->id + keyFrameIter->duration)) {
//                for (int i = 0; i < keyFrameIter->elements.size(); i++) {
//                    Element& e = keyFrameIter->elements.at(i);
//                    elementsInfo.push_back(ElementInfo(e.symbolIndex));
//                    eNum++;
//                }
//                break;
//            }
//        }
//        
//        out.push_back(elementsInfo);
//    }
//    
//    return eNum;
//}
