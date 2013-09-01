//
//  HelloWorldLayer.m
//  CCHierarchiesSpriteDemo
//
//  Created by bman on 4/14/13.
//  Copyright bman 2013. All rights reserved.
//


// Import the interfaces
#import "HelloWorldLayer.h"

// Needed to obtain the Navigation Controller
#import "AppDelegate.h"

#import "CCHierarchiesSprite.h"

#pragma mark - HelloWorldLayer

// HelloWorldLayer implementation
@implementation HelloWorldLayer

// Helper class method that creates a Scene with the HelloWorldLayer as the only child.
+(CCScene *) scene
{
	// 'scene' is an autorelease object.
	CCScene *scene = [CCScene node];
	
	// 'layer' is an autorelease object.
	HelloWorldLayer *layer = [HelloWorldLayer node];
	
	// add layer as a child to scene
	[scene addChild: layer];
	
	// return the scene
	return scene;
}

// on "init" you need to initialize your instance
-(id) init
{
	// always call "super" init
	// Apple recommends to re-assign "self" with the "super's" return value
	if( (self=[super init]) ) {
        [self setTouchEnabled:YES];
        
        CGSize winSize = [CCDirector sharedDirector].winSize;
        
        CCHierarchiesSprite* spr = nil;
        int i = 1;
        
        spr = [CCHierarchiesSprite hierarchiesSpriteWithSpriteSheetFile:@"Anim/abc.sprites" spriteAnimationFile:@"Anim/abc.hanims"];
        spr.position = ccp(i * 150, winSize.height / 2);
        spr.enableAvatar = YES;
        [spr runAction:[CCRepeatForever actionWithAction: [CCHierarchiesAnimate actionWithHierarchiesAnimationName:@"idle" spriteAnimationName:@"Anim/abc.hanims"]]];
        [self addChild:spr];
        spr.color = ccRED;
        spr.flipX = YES;
        i++;
        
        spr = [CCHierarchiesSprite hierarchiesSpriteWithSpriteSheetFile:@"Anim/m03_Chainsaw.sprites" spriteAnimationFile:@"Anim/m03_Chainsaw.hanims"];
        spr.position = ccp(i * 150, winSize.height / 2);
        
        [spr runAction:[CCSequence actions:[CCHierarchiesAnimate actionWithHierarchiesAnimationName:@"atk" spriteAnimationName:spr.animationName],
                        [CCHierarchiesAnimate actionWithHierarchiesAnimationName:@"walk" spriteAnimationName:spr.animationName],
                        [CCCallBlock actionWithBlock:^{
                            [spr setSpriteSheetFile:@"Anim/m12_Banshee.sprites" spriteAnimationFile:@"Anim/m12_Banshee.hanims"];
//                            [spr stopAllActions];
                            [spr runAction:[CCRepeatForever actionWithAction:[CCHierarchiesAnimate actionWithHierarchiesAnimationName:@"atk" spriteAnimationName:@"Anim/m12_Banshee.hanims"]]];
                        }],
                        nil]];
        
        [self addChild:spr];
        spr.scale = 2.0;
        i++;
        
        spr = [CCHierarchiesSprite hierarchiesSpriteWithSpriteSheetFile:@"Anim/m07_Blob.sprites" spriteAnimationFile:@"Anim/m07_Blob.hanims"];
        spr.position = ccp(i * 150, winSize.height / 2);
        [spr runAction:[CCRepeatForever actionWithAction: [CCHierarchiesAnimate actionWithHierarchiesAnimationName:@"atk" spriteAnimationName:spr.animationName]]];
        [self addChild:spr];
        spr.color = ccGREEN;
        spr.opacity = 128;
        i++;
        
        spr = [CCHierarchiesSprite hierarchiesSpriteWithSpriteSheetFile:@"Anim/m09_Guardian.sprites" spriteAnimationFile:@"Anim/m09_Guardian.hanims"];
        spr.position = ccp(i * 150, winSize.height / 2);
        [spr runAction:[CCRepeatForever actionWithAction: [CCHierarchiesAnimate actionWithHierarchiesAnimationName:@"walk" spriteAnimationName:spr.animationName]]];
        [self addChild:spr];
        spr.scale = 0.5;
        spr.opacity = 128;
        spr.flipY = YES;
        i++;
        
        spr = [CCHierarchiesSprite hierarchiesSpriteWithSpriteSheetFile:@"Anim/m12_Banshee.sprites" spriteAnimationFile:@"Anim/m12_Banshee.hanims"];
        spr.position = ccp(i * 150, winSize.height / 2);
        
        spr.enableAvatar = YES;
        [spr runAction:[CCSequence actions:[CCHierarchiesAnimate actionWithHierarchiesAnimationName:@"atk" spriteAnimationName:spr.animationName],
                        [CCHierarchiesAnimate actionWithHierarchiesAnimationName:@"idle" spriteAnimationName:spr.animationName],
                        [CCCallBlock actionWithBlock:^{
                            [spr setAvatarMapWithSymbol:@"body" toItem:@"head 2"];
                        }],
                        nil]];
        
        [self addChild:spr];
        spr.skewX = -30;
        spr.skewY = 30;
        i++;
        
        spr = [CCHierarchiesSprite hierarchiesSpriteWithSpriteSheetFile:@"Anim/m11_Zombero.sprites" spriteAnimationFile:@"Anim/m11_Zombero.hanims"];
        spr.position = ccp(i * 150, winSize.height / 2);
        
        [self runAction:[CCRepeatForever actionWithAction:[CCSequence actions:[CCCallBlock actionWithBlock:^{
                            [spr removeFromParentAndCleanup:YES];
                            [spr setSpriteSheetFile:@"Anim/m11_Zombero.sprites" spriteAnimationFile:@"Anim/m11_Zombero.hanims"];
                            [self addChild:spr];
                            [spr runAction:[CCHierarchiesAnimate actionWithHierarchiesAnimationName:@"atk" spriteAnimationName:@"Anim/m11_Zombero.hanims"]];
                        }],
                         [CCDelayTime actionWithDuration:2],
                         [CCCallBlock actionWithBlock:^{
                            [spr removeFromParentAndCleanup:YES];
                            [spr setSpriteSheetFile:@"Anim/m03_Chainsaw.sprites" spriteAnimationFile:@"Anim/m03_Chainsaw.hanims"];
                        }],
                         [CCDelayTime actionWithDuration:2],
                         //                        [CCHierarchiesAnimate actionWithHierarchiesAnimationName:@"idle" spriteAnimationName:@"m03_Chainsaw.hanims"],
                         [CCCallBlock actionWithBlock:^{
                            [self addChild:spr];
                            [spr runAction:[CCHierarchiesAnimate actionWithHierarchiesAnimationName:@"atk" spriteAnimationName:@"Anim/m03_Chainsaw.hanims"]];
                        }],
                        [CCDelayTime actionWithDuration:2],
                         nil]]];
        i++;
    }
	return self;
}

// on "dealloc" you need to release all your retained objects
- (void) dealloc
{
	// in case you have something to dealloc, do it in this method
	// in this particular example nothing needs to be released.
	// cocos2d will automatically release all the children (Label)
	
	// don't forget to call "super dealloc"
	[super dealloc];
}


#pragma mark - Touch

- (void) ccTouchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch* touch = [touches anyObject];
    CGPoint touchPoint = [touch locationInView:touch.view];
    touchPoint = [[CCDirector sharedDirector] convertToGL:touchPoint];
    touchPoint = [self convertToNodeSpace:touchPoint];
    NSLog(@"hit at %f, %f", touchPoint.x, touchPoint.y);
    
    for (CCNode* node in self.children) {
        if (CGRectContainsPoint(node.boundingBox, touchPoint)) {
            if ([node isKindOfClass:[CCHierarchiesSprite class]]) {
                CCHierarchiesSprite* sprite = (CCHierarchiesSprite*)node;
                NSLog(@"hit %@", sprite.animationName);
            }
            else if ([node isKindOfClass:[CCSprite class]]) {
                NSLog(@"hit CCSprite instance");
            }
        }
    }
}

- (void) ccTouchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
}

- (void) ccTouchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
}

- (void) ccTouchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
}


#pragma mark - GameKit delegate

-(void) achievementViewControllerDidFinish:(GKAchievementViewController *)viewController
{
	AppController *app = (AppController*) [[UIApplication sharedApplication] delegate];
	[[app navController] dismissModalViewControllerAnimated:YES];
}

-(void) leaderboardViewControllerDidFinish:(GKLeaderboardViewController *)viewController
{
	AppController *app = (AppController*) [[UIApplication sharedApplication] delegate];
	[[app navController] dismissModalViewControllerAnimated:YES];
}
@end
