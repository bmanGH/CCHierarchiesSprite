/*
 * Hierarchies Sprite Animation Exporter
 *
 * author: xu xiao cheng <xcxu@break-media.com.cn>
 *
 * version 0.1   12/22/2011
 * version 0.2   12/29/2011
 * version 0.3   12/31/2011
 * version 0.4   1/13/2012   support color transform
 * version 0.5   1/16/2012   support alpha transform and cook guide layer animation to key frames
 * version 0.6   2/3/2012    use item index to identification element instead of name; add library items' left and bottom properties
 * version 0.7   2/10/2012   add avatar, dyeing, event functions
 * version 0.8   2/27/2012   add single texture batch mode function
 * version 0.9   2/28/2012   add CCHierarchiesSpriteSheetCache, CCHierarchiesAnimationCache
 * version 0.9.1 3/15/2012   simplify work if no avater and dyeing
 * version 0.9.2 3/22/2012   fix copy string without '\0' bug
 * version 0.10  3/23/2012   add multi-thread animation in batch mode
 * version 0.10.1   3/26/2012   export .sprites file and items image automatic
 * version 0.10.2   3/26/2012   chinese localization
 * version 0.10.3   3/26/2012   support set .sprites, .anims file, set batch after init
 * version 0.11     3/28/2012   support multi texture packer in batch mode
 * version 0.11.1   4/17/2012   support retina sprites file export
 * version 0.11.2   5/21/2012   hack export .sprites precision bug
 * version 0.12     6/5/2012    replace libxml2 with rapidxml
 * version 0.12.1   6/5/2012    add setAvatarTag and setAvatarTags to CCHierarchiesSprite class
 * version 0.12.2   6/6/2012    fix no-display bug in release build configure
 * version 0.13     11/6/2012   separate version between .hanims file format and runtime code
 * version 0.13.1   31/8/2012   remove retina .sprites export
 * version 0.13.2   4/9/2012    export POT texture
 * version 0.14     16/4/2013   remove animation loop and color dyeing
 * version 0.14.1   6/7/2013    add trace log
 */

// constant

var EXPORTER_VERSION = "0.14";
var ANIM_TAGS_LAYER_NAME = "anim_tags";
var SPACE_SHIFT_STEP = 4;
var EVENT_TAGS_LAYER_NAME = "event_tags";
var ITEMS_ATLAS_MAX_WIDTH = 2048;
var ITEMS_SPACING = 5;
// var RETINA_FILE_SUFFIX = "@2x";

//////

// helper functions

stringReplace = function (source, find, replace) {
	if (!source || !source.length)
		return '';
	//return source.replace(find, replace);
	return source.split(find).join(replace);
}

saveXMLToFile = function (contents, fileURL) {
	if (!contents)
		return false;
		
	//var fileURL = fl.browseForFileURL("save", "save data to ...");
	//if (!fileURL || !fileURL.length)
	//	return false;
		
	var ending = fileURL.slice(-7);
	if (ending != '.hanims')
		fileURL += '.hanims';
	
	var contentsLinebreaks = stringReplace(contents, "\n", "\r\n");
	
	if (!FLfile.write(fileURL, contentsLinebreaks))
	{
		alert("save XML to file error");
		return false;
	}
	
	return true;
}

shiftSpaces = function (shift) {
	var spaces = "";
	for (var s = 0; s < shift; s++) {
		spaces += " ";
	}
	return spaces;
}

getGuidedTweeningKeyframeIndices = function(layer, startFrameIndex, endFrameIndex)
{
	if (!layer)	return [];
	// find the index of the parent layer
	var parentLayerIndex = -1;
	var timeline = fl.getDocumentDOM().getTimeline();
	if (!timeline)	return [];
	for (var li=0; li<timeline.layers.length; li++)
	{
		var theLayer = timeline.layers[li];
		if (theLayer == layer.parentLayer && theLayer.layerType == 'guide')
		{
			parentLayerIndex = li;
			//fl.trace('found parentLayerIndex: ' + parentLayerIndex);
			break;
		}
	}
	var parentLayer = timeline.layers[parentLayerIndex];
	if (!parentLayer) return [];

	var list = [];
	// added the -1 to the for condition because there is no point in remembering whether the last frame is a keyframe,
	// and it contributes to a bug
	for (var frameIndex = startFrameIndex; frameIndex < endFrameIndex - 1; frameIndex++)
	{
		var frame = layer.frames[frameIndex];
		if (!frame)
			continue;

		// first check that the frame is a keyframe
		var isFirstFrame = (frameIndex == startFrameIndex);
		var isKeyframe = (isFirstFrame || frame.startFrame == frameIndex);
		if (!isKeyframe)
			continue;

		// now check that the keyframe has a motion tween
		var isTweening = frame.tweenType == 'motion';
		if (!isTweening)
			continue;

		// now check that the motion guide contains graphics at the same frame
		var parentFrame = parentLayer.frames[frameIndex];
		if (!parentFrame)
			continue;

		if (!parentFrame.elements.length)
			continue;

		// now check that the tween has an existing ending keyframe
		var lastFrameIndex = frameIndex + frame.duration;
		var lastFrame = layer.frames[lastFrameIndex];
		if (!lastFrame)
			continue;


		list[frameIndex] = frame.duration;
	}

	return list;
}

nextPOT = function (x)
{
	x = x - 1;
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);
	return x + 1;
}

//////

// data structure

Element = function () {
	this.symbolIndex = 0;
//	this.name = "";
	this.x = 0;
	this.y = 0;
	this.anchorX = 0;
	this.anchorY = 0;
	this.scaleX = 1;
	this.scaleY = 1;
	this.rotation = 0;
	this.skewX = 0;
	this.skewY = 0;
	this.depth = 0;
//	this.top = 0;
//	this.left = 0;
//	this.width = 0;
//	this.height = 0;
//	this.mat_a = 0;
//	this.mat_b = 0;
//	this.mat_c = 0;
//	this.mat_d = 0;
//	this.mat_tx = 0;
//	this.mat_ty = 0;
	this.color_alpha_percent = 1;
	this.color_alpha_amount = 0;
	this.color_red_percent = 1;
	this.color_red_amount = 0;
	this.color_green_percent = 1;
	this.color_green_amount = 0;
	this.color_blue_percent = 1;
	this.color_blue_amount = 0;
	
	this.toXML = function (shift) {
		var spaces = shiftSpaces(shift);
		var xml = spaces + '<Element' 
				+ ' symbolIndex="' + this.symbolIndex + '"'
//				+ ' name="' + this.name + '"'
				+ ' x="' + this.x + '"'
				+ ' y="' + this.y + '"'
				+ ' anchorX="' + this.anchorX + '"'
				+ ' anchorY="' + this.anchorY + '"'
				+ ' scaleX="' + this.scaleX + '"'
				+ ' scaleY="' + this.scaleY + '"'
				+ ' rotation="' + this.rotation + '"'
				+ ' skewX="' + this.skewX + '"'
				+ ' skewY="' + this.skewY + '"'
				+ ' depth="' + this.depth + '"'
//				+ ' top="' + this.top + '"'
//				+ ' left="' + this.left + '"'
//				+ ' width="' + this.width + '"'
//				+ ' height="' + this.height + '"'
//				+ ' mat_a="' + this.mat_a + '"'
//				+ ' mat_b="' + this.mat_b + '"'
//				+ ' mat_c="' + this.mat_c + '"'
//				+ ' mat_d="' + this.mat_d + '"'
//				+ ' mat_tx="' + this.mat_tx + '"'
//				+ ' mat_ty="' + this.mat_ty + '"'
				+ ' color_alpha_percent="' + this.color_alpha_percent + '"'
				+ ' color_alpha_amount="' + this.color_alpha_amount + '"'
				+ ' color_red_percent="' + this.color_red_percent + '"'
				+ ' color_red_amount="' + this.color_red_amount + '"'
				+ ' color_green_percent="' + this.color_green_percent + '"'
				+ ' color_green_amount="' + this.color_green_amount + '"'
				+ ' color_blue_percent="' + this.color_blue_percent + '"'
				+ ' color_blue_amount="' + this.color_blue_amount + '"'
				+ ' >\n';
		xml += spaces + '</Element>\n';
		return xml;
	}
}

KeyFrame = function () {
	this.id = 0;
	this.duration = 0;
//	this.begin = [];
//	this.end = [];
	this.elements = [];
	this.isMotion = false;
	
	this.toXML = function (shift) {
		var spaces = shiftSpaces(shift);
		var xml = spaces + '<KeyFrame' 
				+ ' id="' + this.id + '"'
				+ ' duration="' + this.duration + '"'
				+ ' isMotion="' + this.isMotion + '"'
				+ ' >\n';
		
//		var spaceStep = shiftSpaces(SPACE_SHIFT_STEP);
//		
//		xml += spaces + spaceStep + '<begin>\n';
//		for (var i = 0; i < this.begin.length; i++) {
//			xml += this.begin[i].toXML(shift + SPACE_SHIFT_STEP * 2);
//		}
//		xml += spaces + spaceStep + '</begin>\n';
//		
//		xml += spaces + spaceStep + '<end>\n';
//		for (var j = 0; j < this.end.length; j++) {
//			xml += this.end[j].toXML(shift + SPACE_SHIFT_STEP * 2);
//		}
//		xml += spaces + spaceStep + '</end>\n';

		for (var i = 0; i < this.elements.length; i++) {
			xml += this.elements[i].toXML(shift + SPACE_SHIFT_STEP);
		}
		
		xml += spaces + '</KeyFrame>\n';
		
		return xml;
	}
}

Layer = function () {
	this.name = "";
	this.frames = [];
	
	this.toXML = function (shift) {
		var spaces = shiftSpaces(shift);
		var xml = spaces + '<Layer' + ' name="' + this.name + '"' + ' >\n';
		for (var i = 0; i < this.frames.length; i++) {
			xml += this.frames[i].toXML(shift + SPACE_SHIFT_STEP);
		}
		xml += spaces + '</Layer>\n';
		return xml;
	}
}

Animation = function () {
	this.name = "";
	this.startFrameIndex = 0;
	this.endFrameIndex = 0;
	
	this.toXML = function (shift) {
		var spaces = shiftSpaces(shift);
		var xml = spaces + '<Animation' + ' name="' + this.name + '"'
				+ ' startFrameIndex="' + this.startFrameIndex + '"'
				+ ' endFrameIndex="' + this.endFrameIndex + '"'
				+ ' >\n';
		xml += spaces + '</Animation>\n';
		return xml;
	}
}

Item = function () {
	this.name = "";
	this.left = 0;
	this.bottom = 0; // element's top - element's height
	
	this.toXML = function (shift) {
		var spaces = shiftSpaces(shift);
		var xml = spaces + '<Item'
				+ ' name="' + this.name + '"'
				+ ' left="' + this.left + '"'
				+ ' bottom="' + this.bottom + '"'
				+ ' >\n';
		xml += spaces + '</Item>\n';
		return xml;
	}
}

Symbol = function () {
	this.name = ""
	this.defaultItemIndex = 0
	
	this.toXML = function (shift) {
		var spaces = shiftSpaces(shift);
		var xml = spaces + '<Symbol'
				+ ' name="' + this.name + '"'
				+ ' defaultItemIndex="' + this.defaultItemIndex + '"'
				+ ' >\n';
		xml += spaces + '</Symbol>\n';
		return xml;
	}
}

Event = function () {
	this.frameId = 0;
	this.content = ""
	
	this.toXML = function (shift) {
		var spaces = shiftSpaces(shift);
		var xml = spaces + '<Event'
				+ ' frameId="' + this.frameId + '"'
				+ ' content="' + this.content + '"'
				+ ' >\n';
		xml += spaces + '</Event>\n';
		return xml;
	}
}

Hierarchies = function () {
	this.version = "";
	this.frameRate = 0;
	this.anims = [];
	this.layers = [];
	this.items = [];
	this.symbols = [];
	this.events = [];
	
	this.getItemIndexByName = function (itemName) {
		for (var index = 0; index < this.items.length; index++) {
			if (this.items[index].name == itemName) {
				return index;
			}
		}
		return -1;
	}
	
	this.getSymbolIndexByName = function (symbolName) {
		for (var index = 0; index < this.symbols.length; index++) {
			if (this.symbols[index].name == symbolName) {
				return index;
			}
		}
		return -1;
	}
	
	this.toXML = function (shift) {
		var spaces = shiftSpaces(shift);
		var xml = spaces + '<Hierarchies' 
				+ ' version="' + this.version + '"'
				+ ' frameRate="' + this.frameRate  + '"'
				+ ' >\n';
				
		var spaceStep = shiftSpaces(SPACE_SHIFT_STEP);
		
		xml += spaces + spaceStep + '<items>\n';
		for (var k = 0; k < this.items.length; k++) {
			xml += this.items[k].toXML(shift + SPACE_SHIFT_STEP * 2);
		}
		xml += spaces + spaceStep + '</items>\n';
		
		xml += spaces + spaceStep + '<symbols>\n';
		for (var l = 0; l < this.symbols.length; l++) {
			xml += this.symbols[l].toXML(shift + SPACE_SHIFT_STEP * 2);
		}
		xml += spaces + spaceStep + '</symbols>\n';
		
		xml += spaces + spaceStep + '<anims>\n';
		for (var i = 0; i < this.anims.length; i++) {
			xml += this.anims[i].toXML(shift + SPACE_SHIFT_STEP * 2);
		}
		xml += spaces + spaceStep + '</anims>\n';
		
		xml += spaces + spaceStep + '<events>\n';
		for (var m = 0; m < this.events.length; m++) {
			xml += this.events[m].toXML(shift + SPACE_SHIFT_STEP * 2);
		}
		xml += spaces + spaceStep + '</events>\n';
		
		xml += spaces + spaceStep + '<layers>\n';
		for (var j = 0; j < this.layers.length; j++) {
			xml += this.layers[j].toXML(shift + SPACE_SHIFT_STEP * 2);
		}
		xml += spaces + spaceStep + '</layers>\n';
		
		xml += spaces + '</Hierarchies>\n';
		return xml;
	}
}

//////

// build Hierarchies data
var data = new Hierarchies();
data.version = EXPORTER_VERSION;
data.frameRate = fl.getDocumentDOM().frameRate;

// build anims data
if (fl.getDocumentDOM().getTimeline().layers[0].name == ANIM_TAGS_LAYER_NAME) {
	var lastAnim = null;
	var ind = 0
	for (ind = 0; ind < fl.getDocumentDOM().getTimeline().layers[0].frames.length; ind++) {
		if (fl.getDocumentDOM().getTimeline().layers[0].frames[ind].startFrame == ind) {
			if (lastAnim == null) {
				lastAnim = new Animation();
				lastAnim.startFrameIndex = ind;
				
				lastAnim.loop = false;
				lastAnim.name = fl.getDocumentDOM().getTimeline().layers[0].frames[ind].name;
			}
			else {
				lastAnim.endFrameIndex = ind - 1;
				data.anims.push(lastAnim);
				lastAnim = new Animation();
				lastAnim.startFrameIndex = ind;
				
				lastAnim.loop = false;
				lastAnim.name = fl.getDocumentDOM().getTimeline().layers[0].frames[ind].name;
			}
		}
	}
	if (lastAnim != null) {
		lastAnim.endFrameIndex = ind - 1;
		data.anims.push(lastAnim);
	}
}
else {
	alert('no "' + ANIM_TAGS_LAYER_NAME + '" layer');
}

// build event data
if (fl.getDocumentDOM().getTimeline().layers[1].name == EVENT_TAGS_LAYER_NAME) {
	for (var m = 0; m < fl.getDocumentDOM().getTimeline().layers[1].frames.length; m++) {
		if (fl.getDocumentDOM().getTimeline().layers[1].frames[m].startFrame == m &&
			fl.getDocumentDOM().getTimeline().layers[1].frames[m].name.length > 0) {
			var event = new Event();
			event.frameId = m;
			event.content = fl.getDocumentDOM().getTimeline().layers[1].frames[m].name;
			data.events.push(event);
		}
	}
}

if (fl.getDocumentDOM().getTimeline().layers[0].name == ANIM_TAGS_LAYER_NAME) {
	var ii = 0; // the Hierarchies layers array index
	for (var i = 0; i < fl.getDocumentDOM().getTimeline().layers.length; i++) {
		if (fl.getDocumentDOM().getTimeline().layers[i].name == ANIM_TAGS_LAYER_NAME) {
			continue;
		}
		else if (fl.getDocumentDOM().getTimeline().layers[i].name == EVENT_TAGS_LAYER_NAME) {
			continue;
		}
		else if (fl.getDocumentDOM().getTimeline().layers[i].layerType == "guide") {
			continue;
		}
		else if (fl.getDocumentDOM().getTimeline().layers[i].layerType == "mask") {
			continue;
		}
		else if (fl.getDocumentDOM().getTimeline().layers[i].layerType == "masked") {
			continue;
		}
		else if (fl.getDocumentDOM().getTimeline().layers[i].layerType == "folder") {
			continue;
		}
		else if (fl.getDocumentDOM().getTimeline().layers[i].layerType == "guided") {
			// if there is a motion guide on this layer, convert it all to keyframes
			// so we can grab the values from them.
			var guidedTweenKeyframes = getGuidedTweeningKeyframeIndices(fl.getDocumentDOM().getTimeline().layers[i], 
																		0, 
																		fl.getDocumentDOM().getTimeline().layers[i].frameCount);
			
			fl.getDocumentDOM().getTimeline().currentLayer = i;
			// This loop has redundancies sometimes, but is necessary because
			// we could have spans of hold (non-tweening) frames in the middle 
			for (var tfi=0; tfi < guidedTweenKeyframes.length; tfi++)
			{
				if (!guidedTweenKeyframes[tfi])
					continue;

				var tweeningFrameIndex = tfi;
				var tweeningKeyframe = fl.getDocumentDOM().getTimeline().layers[i].frames[tweeningFrameIndex];

				// Workaround for Flash convertToKeyframes bug with connected tweens--we have to keyframe the connected
				// chain of tweens all at once, otherwise the results are incorrect.
				var lastFrameIndex = tweeningFrameIndex; //+tweeningKeyframe.duration;
				var lastFrame = fl.getDocumentDOM().getTimeline().layers[i].frames[lastFrameIndex];

				while (lastFrame
					   && lastFrame.tweenType == "motion"
					   && guidedTweenKeyframes[lastFrameIndex])
				{
					var nextFrameIndex = lastFrameIndex + lastFrame.duration;
					// avoid the case where the last frame of the layer has a motion tween on it,
					// in which case it has a duration of 1, but there is no keyframe after it
					if (fl.getDocumentDOM().getTimeline().layers[i].frames[nextFrameIndex])
					{
						lastFrameIndex = nextFrameIndex;
						lastFrame = fl.getDocumentDOM().getTimeline().layers[i].frames[lastFrameIndex];
					}
					else
					{
						break;
					}
				}

				if (lastFrameIndex > tweeningFrameIndex + 1)
				{
					fl.getDocumentDOM().getTimeline().convertToKeyframes(tweeningFrameIndex + 1, lastFrameIndex);
				}
			}
		}
		
		// the type of this layer is normal or guided(cooked)
		{
			// build layers data
			fl.trace("build layer:" + fl.getDocumentDOM().getTimeline().layers[i].name);
			var layer = new Layer();
			layer.name = fl.getDocumentDOM().getTimeline().layers[i].name;
			data.layers.push(layer);
			
//			var m = 0;
			
			var lastKeyFrame = null;
			var j = 0;
			for (j = 0; j < fl.getDocumentDOM().getTimeline().layers[i].frames.length; j++) {
				// build KeyFrame data
				if (fl.getDocumentDOM().getTimeline().layers[i].frames[j].startFrame == j) {
					if (lastKeyFrame == null) {
						fl.trace("  build key frame:" + j);
						lastKeyFrame = new KeyFrame();
						lastKeyFrame.id = j;
						if (fl.getDocumentDOM().getTimeline().layers[i].frames[j].tweenType == "motion") {
							lastKeyFrame.isMotion = true;
						}
						else {
							lastKeyFrame.isMotion = false;
						}
						for (var k = 0; k < fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements.length; k++) {
							// build Element data
							if (fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].elementType == "instance" &&
								fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].instanceType == "symbol") {
								var element = new Element();
								
								// build symbol data
								var symbolIndex = data.getSymbolIndexByName(fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.name);
								if (symbolIndex == -1) {
									fl.trace("    build symbol:" + fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.name);
									var symbol = new Symbol();
									symbol.name = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.name;
									
									// build item data
									if (fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers.length > 0) {
										for (var p = 0; p < fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames.length; p++) {
											fl.trace("      build item:" + fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[p].elements[0].libraryItem.name);
											var itemIndex = data.getItemIndexByName(fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[p].elements[0].libraryItem.name);
											if (itemIndex == -1) {
												var item = new Item();
												item.name = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[p].elements[0].libraryItem.name;
												item.left = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[p].elements[0].left;
												item.bottom = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[p].elements[0].top +
																fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[p].elements[0].height;
												
												itemIndex = data.items.push(item) - 1;
											}
											
											if (p == 0) {
												symbol.defaultItemIndex = itemIndex;
											}
										}
									}
									else {
										alert(symbol.name + " error");
									}
									
									element.symbolIndex = data.symbols.push(symbol) - 1;
								}
								else {
									element.symbolIndex = symbolIndex;
								}
								
//								var itemIndex = data.getItemIndexByName(fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.name);
//								if (itemIndex == -1) {
//									var item = new Item();
//									item.name = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.name;
//									item.left = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[0].elements[0].left;
//									item.bottom = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[0].elements[0].top +
//												fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[0].elements[0].height;
//									
//									element.itemIndex = data.items.push(item) - 1;
//								}
//								else {
//									element.itemIndex = itemIndex;
//								}
								
//								element.name = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.name;
//								element.x = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].x;
//								element.y = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].y;
//								element.anchorX = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].transformX;
//								element.anchorY = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].transformY;
								element.x = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].transformX;
								element.y = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].transformY;
								element.anchorX = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].getTransformationPoint().x;
								element.anchorY = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].getTransformationPoint().y;
//								element.top = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].top;
//								element.left = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].left;
//								element.width = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].width;
//								element.height = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].height;
								element.scaleX = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].scaleX;
								element.scaleY = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].scaleY;
								element.rotation = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].rotation;
								element.skewX = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].skewX;
								element.skewY = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].skewY;
								element.depth = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].depth;
//								element.mat_a = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.a;
//								element.mat_b = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.b;
//								element.mat_c = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.c;
//								element.mat_d = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.d;
//								element.mat_tx = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.tx;
//								element.mat_ty = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.ty;
								if (fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorMode == "advanced") {
									element.color_alpha_percent = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorAlphaPercent / 100.0;
									element.color_alpha_amount = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorAlphaAmount;
									element.color_red_percent = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorRedPercent / 100.0;
									element.color_red_amount = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorRedAmount;
									element.color_green_percent = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorGreenPercent / 100.0;
									element.color_green_amount = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorGreenAmount;
									element.color_blue_percent = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorBluePercent / 100.0;
									element.color_blue_amount = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorBlueAmount;
								}
								else if (fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorMode == "alpha") {
									element.color_alpha_percent = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorAlphaPercent / 100.0;
								}
//								lastKeyFrame.begin.push(element);
								lastKeyFrame.elements.push(element);
							}
						}
					}
					else {
						lastKeyFrame.duration = j - lastKeyFrame.id;
//						for (var k = 0; k < fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements.length; k++) {
//							// build Element data
//							if (fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].elementType == "instance" &&
//								fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].instanceType == "symbol") {
//								var element = new Element();
//								element.name = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].libraryItem.name;
//								element.x = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].x;
//								element.y = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].y;
//								element.anchorX = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].transformX;
//								element.anchorY = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].transformY;
//								element.scaleX = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].scaleX;
//								element.scaleY = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].scaleY;
//								element.rotation = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].rotation;
//								element.skewX = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].skewX;
//								element.skewY = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].skewY;
//								element.depth = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].depth;
//								element.mat_a = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].matrix.a;
//								element.mat_b = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].matrix.b;
//								element.mat_c = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].matrix.c;
//								element.mat_d = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].matrix.d;
//								element.mat_tx = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].matrix.tx;
//								element.mat_ty = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].matrix.ty;
//								lastKeyFrame.end.push(element);
//							}
//						}
						data.layers[ii].frames.push(lastKeyFrame);

						fl.trace("  build key frame:" + j);
						lastKeyFrame = new KeyFrame();
						lastKeyFrame.id = j;
						if (fl.getDocumentDOM().getTimeline().layers[i].frames[j].tweenType == "motion") {
							lastKeyFrame.isMotion = true;
						}
						else {
							lastKeyFrame.isMotion = false;
						}
						for (var k = 0; k < fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements.length; k++) {
							// build Element data
							if (fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].elementType == "instance" &&
								fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].instanceType == "symbol") {
								var element = new Element();
								
								// build symbol data
								var symbolIndex = data.getSymbolIndexByName(fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.name);
								if (symbolIndex == -1) {
									fl.trace("    build symbol:" + fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.name);
									var symbol = new Symbol();
									symbol.name = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.name;
									
									// build item data
									if (fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers.length > 0) {
										for (var p = 0; p < fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames.length; p++) {
											fl.trace("      build item:" + fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[p].elements[0].libraryItem.name);
											var itemIndex = data.getItemIndexByName(fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[p].elements[0].libraryItem.name);
											if (itemIndex == -1) {
												var item = new Item();
												item.name = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[p].elements[0].libraryItem.name;
												item.left = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[p].elements[0].left;
												item.bottom = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[p].elements[0].top +
																fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[p].elements[0].height;
												
												itemIndex = data.items.push(item) - 1;
											}
											
											if (p == 0) {
												symbol.defaultItemIndex = itemIndex;
											}
										}
									}
									else {
										alert(symbol.name + " error");
									}
									
									element.symbolIndex = data.symbols.push(symbol) - 1;
								}
								else {
									element.symbolIndex = symbolIndex;
								}
								
//								var itemIndex = data.getItemIndexByName(fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.name);
//								if (itemIndex == -1) {
//									var item = new Item();
//									item.name = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.name;
//									item.left = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[0].elements[0].left;
//									item.bottom = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[0].elements[0].top +
//													fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.timeline.layers[0].frames[0].elements[0].height;
//									
//									element.itemIndex = data.items.push(item) - 1;
//								}
//								else {
//									element.itemIndex = itemIndex;
//								}
								
//								element.name = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.name;
//								element.x = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].x;
//								element.y = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].y;
//								element.anchorX = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].transformX;
//								element.anchorY = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].transformY;
								element.x = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].transformX;
								element.y = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].transformY;
								element.anchorX = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].getTransformationPoint().x;
								element.anchorY = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].getTransformationPoint().y;
//								element.top = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].top;
//								element.left = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].left;
//								element.width = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].width;
//								element.height = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].height;
								element.scaleX = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].scaleX;
								element.scaleY = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].scaleY;
								element.rotation = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].rotation;
								element.skewX = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].skewX;
								element.skewY = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].skewY;
								element.depth = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].depth;
//								element.mat_a = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.a;
//								element.mat_b = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.b;
//								element.mat_c = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.c;
//								element.mat_d = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.d;
//								element.mat_tx = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.tx;
//								element.mat_ty = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.ty;
								if (fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorMode == "advanced") {
									element.color_alpha_percent = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorAlphaPercent / 100.0;
									element.color_alpha_amount = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorAlphaAmount;
									element.color_red_percent = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorRedPercent / 100.0;
									element.color_red_amount = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorRedAmount;
									element.color_green_percent = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorGreenPercent / 100.0;
									element.color_green_amount = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorGreenAmount;
									element.color_blue_percent = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorBluePercent / 100.0;
									element.color_blue_amount = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorBlueAmount;
								}
								else if (fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorMode == "alpha") {
									element.color_alpha_percent = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].colorAlphaPercent / 100.0;
								}
//								lastKeyFrame.begin.push(element);
								lastKeyFrame.elements.push(element);
							}
						}
					}
				} // key frame end
				
//				// do key frame split if the end of an animation inside in a key frame sequence
//				if (j == data.anims[m].endFrameIndex) {
//					lastKeyFrame.duration = j - lastKeyFrame.id + 1;
//					for (var k = 0; k < fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements.length; k++) {
//						// build Element data
//						if (fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].elementType == "instance" &&
//							fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].instanceType == "symbol") {
//							var element = new Element();
//							element.name = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].libraryItem.name;
//							element.x = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].x;
//							element.y = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].y;
//							element.anchorX = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].transformX;
//							element.anchorY = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].transformY;
//							element.scaleX = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].scaleX;
//							element.scaleY = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].scaleY;
//							element.rotation = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].rotation;
//							element.skewX = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].skewX;
//							element.skewY = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].skewY;
//							element.depth = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].depth;
//							element.mat_a = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.a;
//							element.mat_b = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.b;
//							element.mat_c = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.c;
//							element.mat_d = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.d;
//							element.mat_tx = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.tx;
//							element.mat_ty = fl.getDocumentDOM().getTimeline().layers[i].frames[j].elements[k].matrix.ty;
//							lastKeyFrame.end.push(element);
//						}
//					}
//					data.layers[ii].frames.push(lastKeyFrame);
//					
//					lastKeyFrame = null; // prevent double insert elements at end of layer
//					
//					if ( (j+1) < fl.getDocumentDOM().getTimeline().layers[i].frames.length &&
//						fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].startFrame != (j+1) ) {
						fl.trace("  build key frame:" + j);
//						lastKeyFrame = new KeyFrame();
//						lastKeyFrame.id = j + 1;
//						for (var k = 0; k < fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements.length; k++) {
//							// build Element data
//							if (fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].elementType == "instance" &&
//								fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].instanceType == "symbol") {
//								var element = new Element();
//								element.name = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].libraryItem.name;
//								element.x = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].x;
//								element.y = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].y;
//								element.anchorX = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].transformX;
//								element.anchorY = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].transformY;
//								element.scaleX = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].scaleX;
//								element.scaleY = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].scaleY;
//								element.rotation = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].rotation;
//								element.skewX = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].skewX;
//								element.skewY = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].skewY;
//								element.depth = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].depth;
//								element.mat_a = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].matrix.a;
//								element.mat_b = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].matrix.b;
//								element.mat_c = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].matrix.c;
//								element.mat_d = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].matrix.d;
//								element.mat_tx = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].matrix.tx;
//								element.mat_ty = fl.getDocumentDOM().getTimeline().layers[i].frames[j+1].elements[k].matrix.ty;
//								lastKeyFrame.begin.push(element);
//							}
//						}
//					}
//						
//					m++;
//					if (m >= data.anims.length)
//						break;
//				} // animation cut end
			} // frames end
			if (lastKeyFrame != null) {
				lastKeyFrame.duration = j - lastKeyFrame.id;
//				for (var k = 0; k < fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements.length; k++) {
//					// build Element data
//					if (fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].elementType == "instance" &&
//						fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].instanceType == "symbol") {
//						var element = new Element();
//						element.name = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].libraryItem.name;
//						element.x = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].x;
//						element.y = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].y;
//						element.anchorX = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].transformX;
//						element.anchorY = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].transformY;
//						element.scaleX = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].scaleX;
//						element.scaleY = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].scaleY;
//						element.rotation = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].rotation;
//						element.skewX = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].skewX;
//						element.skewY = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].skewY;
//						element.depth = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].depth;
//						element.mat_a = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].matrix.a;
//						element.mat_b = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].matrix.b;
//						element.mat_c = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].matrix.c;
//						element.mat_d = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].matrix.d;
//						element.mat_tx = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].matrix.tx;
//						element.mat_ty = fl.getDocumentDOM().getTimeline().layers[i].frames[j-1].elements[k].matrix.ty;
//						lastKeyFrame.end.push(element);
//					}
//				}
				data.layers[ii].frames.push(lastKeyFrame);
			}
			ii++;
		}
	} // layers end
	
	var xml = '<?xml version="1.0"?>\n'
			+ '<!-- Generated by Hierarchies Sprite Animation Exporter version ' + EXPORTER_VERSION + ' -->\n'
			+ data.toXML(0);
			
	var fileURL = fl.browseForFileURL("save", "save data to ...");
	if (fileURL && fileURL.length) {
		if (saveXMLToFile(xml, fileURL) == false) {
			alert("export Hierarchies Sprite Animation failed");
		}
		else {
			alert("export Hierarchies Sprite Animation successed");
			
			// export item images and .sprites file automatic
			var ending = fileURL.slice(-4);
			var pngFileURL = fileURL;
//			var retinaPngFileURL = fileURL;
			if (ending != '.png') {
				pngFileURL = fileURL + '.png';
//				retinaPngFileURL = fileURL + RETINA_FILE_SUFFIX + '.png';
			}
			
			ending = fileURL.slice(-8);
			var spritesFileURL = fileURL;
			var retinaSpritesFileURL = fileURL;
			if (ending != '.sprites') {
				spritesFileURL = fileURL + '.sprites';
//				retinaSpritesFileURL = fileURL + RETINA_FILE_SUFFIX + '.sprites';
			}
			
			// export item images automatic
			var origDoc = fl.getDocumentDOM();
			var targetDoc = fl.createDocument();
			var currentWidth = ITEMS_SPACING;
			var currentHeight = ITEMS_SPACING;
			var heightMax = 0;
			var xmlSprites = "";
//			var xmlSpritesRetina = "";
			for (var i = 0; i < data.items.length; i++) {
				fl.trace("export item[" + i + "] : " + data.items[i].name);
				var itemIndex = origDoc.library.findItemIndex(data.items[i].name);
				var targetItem = origDoc.library.items[itemIndex];
				targetDoc.addItem({x : 0, y : 0}, targetItem);
				var lastElement = targetDoc.getTimeline().layers[0].frames[0].elements[i];
				// if (currentWidth + ITEMS_SPACING * 3 + Math.round(lastElement.width) > ITEMS_ATLAS_MAX_WIDTH) {
				// 	currentWidth = ITEMS_SPACING;
				// 	currentHeight += heightMax + ITEMS_SPACING * 2;
				// 	heightMax = 0;
				// }
				if (currentWidth + ITEMS_SPACING * 3 + lastElement.width > ITEMS_ATLAS_MAX_WIDTH) {
					currentWidth = ITEMS_SPACING;
					currentHeight += heightMax + ITEMS_SPACING * 2;
					heightMax = 0;
				}
				//lastElement.width = Math.round(lastElement.width);
				//lastElement.height = Math.round(lastElement.height);
				lastElement.x = lastElement.x + currentWidth - lastElement.left;
				lastElement.y = lastElement.y + currentHeight - lastElement.top;
				
				// xmlSprites += '            <spr name="' + data.items[i].name
				// 			+ '" x="' + Math.floor(lastElement.left - 1) //HACK: -1 ?
				// 			+ '" y="' + Math.floor(lastElement.top - 1) //HACK: -1 ?
				// 			+ '" w="' + Math.round(lastElement.width + 2) //HACK: +2 ?
				// 			+ '" h="' + Math.round(lastElement.height + 2) //HACK: +2 ?
				// 			+ '"/>\n';
//				xmlSpritesRetina += '            <spr name="' + data.items[i].name
//							+ '" x="' + Math.floor(lastElement.left - 1) * 2 //HACK: -1 ?
//							+ '" y="' + Math.floor(lastElement.top - 1) * 2 //HACK: -1 ?
//							+ '" w="' + Math.round(lastElement.width + 2) * 2 //HACK: +2 ?
//							+ '" h="' + Math.round(lastElement.height + 2) * 2 //HACK: +2 ?
//							+ '"/>\n';
				xmlSprites += '            <spr name="' + data.items[i].name
							+ '" x="' + lastElement.left
							+ '" y="' + lastElement.top
							+ '" w="' + lastElement.width
							+ '" h="' + lastElement.height
							+ '"/>\n';
				
				// currentWidth += Math.round(lastElement.width + ITEMS_SPACING);
				// if (Math.round(lastElement.height) > heightMax)
				// 	heightMax = Math.round(lastElement.height);
				currentWidth += lastElement.width + ITEMS_SPACING;
				if (lastElement.height > heightMax)
					heightMax = lastElement.height;
			}
			
			var atlasWidth = Math.round(ITEMS_ATLAS_MAX_WIDTH);
			var atlasHeight = nextPOT(Math.round(currentHeight + heightMax + ITEMS_SPACING));
			
			var spritesXML = '<?xml version="1.0"?>\n'
						+ '<!-- Generated by Hierarchies Sprite Animation Exporter version ' + EXPORTER_VERSION + ' -->\n'
						+ '<img name="' + pngFileURL.match(/[^\/]+$/) 
						+ '" w="' + atlasWidth 
						+ '" h="' + atlasHeight + '">\n'
						+ '    <definitions>\n'
						+ '        <dir name="">\n'
						+ xmlSprites
						+ '        </dir>\n'
						+ '    </definitions>\n'
						+ '</img>\n';
//			var spritesRetinaXML = '<?xml version="1.0"?>\n'
//						+ '<!-- Generated by darkFunction Editor (www.darkfunction.com) -->\n'
//						+ '<img name="' + retinaPngFileURL.match(/[^\/]+$/) 
//						+ '" w="' + atlasWidth * 2
//						+ '" h="' + atlasHeight * 2 + '">\n'
//						+ '    <definitions>\n'
//						+ '        <dir name="">\n'
//						+ xmlSpritesRetina
//						+ '        </dir>\n'
//						+ '    </definitions>\n'
//						+ '</img>\n';
			
			targetDoc.width = atlasWidth;
			targetDoc.height = atlasHeight;
			if (targetDoc.exportPNG(pngFileURL, false, true) == false) {
				alert("export items image error");
			}
			
			var spritesXMLLinebreaks = stringReplace(spritesXML, "\n", "\r\n");
			if (!FLfile.write(spritesFileURL, spritesXMLLinebreaks))
			{
				alert("export .sprites file error");
			}
//			spritesXMLLinebreaks = stringReplace(spritesRetinaXML, "\n", "\r\n");
//			if (!FLfile.write(retinaSpritesFileURL, spritesXMLLinebreaks))
//			{
//				alert("export .sprites file error");
//			}
		}
	}
}
