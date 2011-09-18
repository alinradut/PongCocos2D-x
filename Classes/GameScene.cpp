//
//  GameScene.cpp
//  Pong
//
//  Created by Clawoo on 9/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "GameScene.h"

USING_NS_CC;

#define CC_SPRITE_ACTUAL_WIDTH(sprite) sprite->getContentSize().width * sprite->getScaleX()
#define CC_SPRITE_ACTUAL_HEIGHT(sprite) sprite->getContentSize().width * sprite->getScaleX()
#define CC_SPRITE_ACTUAL_FRAME(sprite) CCRectMake(sprite->getPosition().x - CC_SPRITE_ACTUAL_WIDTH(sprite)/2, \
                                                    sprite->getPosition().y - CC_SPRITE_ACTUAL_HEIGHT(sprite)/2, \
                                                    CC_SPRITE_ACTUAL_WIDTH(sprite), \
                                                    CC_SPRITE_ACTUAL_HEIGHT(sprite))

CCScene* GameScene::scene()
{
	CCScene *scene = CCScene::node();
	GameScene *layer = GameScene::node();
	scene->addChild(layer);
	return scene;
}

bool GameScene::init()
{
    if (!CCLayer::init())
    {
        return false;
    }
    this->setIsTouchEnabled(true);
    
    gameArea_ = CCRectMake(20, 50, 273, 382);
    touchArea_ = CCRectMake(20, 0, 280, 50);
    
    CCSize paddleSize = CCSizeMake(40, 10);
    
    // create the user paddle from the generic white sprite
    userPaddle_ = CCSprite::spriteWithFile("square.png");

    // scale the sprite to match our desired size
    userPaddle_->setScaleX(paddleSize.width / userPaddle_->getContentSize().width);
    userPaddle_->setScaleY(paddleSize.height / userPaddle_->getContentSize().height);
    userPaddle_->setPosition(ccp(gameArea_.origin.x + gameArea_.size.width/2, gameArea_.origin.y + 20));
    this->addChild(userPaddle_);
    
    // create the CPU paddle from the generic white sprite
    cpuPaddle_ = CCSprite::spriteWithFile("square.png");
    
    // scale the sprite to match our desired size
    cpuPaddle_->setScaleX(paddleSize.width / cpuPaddle_->getContentSize().width);
    cpuPaddle_->setScaleY(paddleSize.height / cpuPaddle_->getContentSize().height);
    cpuPaddle_->setPosition(ccp(gameArea_.origin.x + gameArea_.size.width/2, gameArea_.origin.y + gameArea_.size.height - 20));
    this->addChild(cpuPaddle_);
    
    ball_ = CCSprite::spriteWithFile("square.png");
    ball_->setPosition(ccp(gameArea_.origin.x + gameArea_.size.width/2, gameArea_.origin.y + 30));
    this->addChild(ball_);
    
    this->scheduleUpdate();
    return true;
}

void GameScene::draw()
{
    // draw the game area outline
    glColor4f(1.0, 1.0, 1.0, 1.0);
	glLineWidth(1);
	CCPoint vertices[] = { 
        CCPointMake(gameArea_.origin.x, gameArea_.origin.y), 
        CCPointMake(gameArea_.origin.x + gameArea_.size.width, gameArea_.origin.y), 
        CCPointMake(gameArea_.origin.x + gameArea_.size.width, gameArea_.origin.y + gameArea_.size.height),
        CCPointMake(gameArea_.origin.x, gameArea_.origin.y + gameArea_.size.height)
    };
	ccDrawPoly( vertices, 4, true);
    
    // draw the marker at the middle of the field
	ccDrawLine(CCPointMake(gameArea_.origin.x, gameArea_.origin.y + gameArea_.size.height/2),
               CCPointMake(gameArea_.origin.x + gameArea_.size.width, gameArea_.origin.y + gameArea_.size.height/2));
}

void GameScene::update(cocos2d::ccTime dt)
{
    CCRect ballFrame = CC_SPRITE_ACTUAL_FRAME(ball_);
    
    if (ballFrame.origin.y >= gameArea_.origin.y + gameArea_.size.height)
    {
        CCLog("CPU Lost");
    }
    
    if (ballFrame.origin.y + ballFrame.size.height <= gameArea_.origin.y)
    {
        CCLog("Player Lost");
    }
    
    if (CCRect::CCRectIntersectsRect(ballFrame, CC_SPRITE_ACTUAL_FRAME(userPaddle_)))
    {
        // bounce off the user's paddle
    }
    
    if (CCRect::CCRectIntersectsRect(ballFrame, CC_SPRITE_ACTUAL_FRAME(cpuPaddle_)))
    {
        // bounce off the CPU's paddle
    }
}

void GameScene::ccTouchesBegan(cocos2d::CCSet* touches, cocos2d::CCEvent* event)
{
    // get the touch object
    CCTouch* touch = (CCTouch *)touches->anyObject();
    // obtain the location of the touch
    CCPoint location = touch->locationInView(touch->view());
    // convert the location of the touch to GL coordinates
    location = CCDirector::sharedDirector()->convertToGL(location);
    
    // only process if the touch was inside the designated touch area
    if (CCRect::CCRectContainsPoint(touchArea_, location))
    {
        // calculate how far left can the paddle go
        float minX = gameArea_.origin.x + userPaddle_->getContentSize().width/2 * userPaddle_->getScaleX();
        // calculate how far right can the paddle go
        float maxX = gameArea_.origin.x + gameArea_.size.width - userPaddle_->getContentSize().width/2 * userPaddle_->getScaleX();
        
        // calculate the final position of the paddle
        CCPoint destPosition = ccp(MIN(MAX(minX, location.x), maxX), userPaddle_->getPosition().y);
        
        // calculate the distance from the paddle to the user's touch
        float diffX = fabs(destPosition.x - userPaddle_->getPosition().x);
        
        userPaddle_->stopAllActions();
        
        // if the touch was within the area of the paddle, "grab the paddle" and set its position to the touch location
        if (diffX < userPaddle_->getContentSize().width/2 * userPaddle_->getScaleX() 
            || userHasGrabbedThePaddle_)
        {
            userHasGrabbedThePaddle_ = true;
            userPaddle_->setPosition(destPosition);
        }
        // otherwise animate the paddle to that position
        else
        {
            userPaddle_->runAction(CCMoveTo::actionWithDuration(0.3 * diffX / gameArea_.size.width, destPosition));
        }
    }
}

void GameScene::ccTouchesMoved(CCSet* touches, CCEvent* event)
{
    // get the touch object
    CCTouch* touch = (CCTouch *)touches->anyObject();
    // obtain the location of the touch
    CCPoint location = touch->locationInView(touch->view());
    // convert the location of the touch to GL coordinates
    location = CCDirector::sharedDirector()->convertToGL(location);
    
    // only process if the touch was inside the designated touch area
    if (CCRect::CCRectContainsPoint(touchArea_, location))
    {
        // calculate how far left can the paddle go
        float minX = gameArea_.origin.x + userPaddle_->getContentSize().width/2 * userPaddle_->getScaleX();
        // calculate how far right can the paddle go
        float maxX = gameArea_.origin.x + gameArea_.size.width - userPaddle_->getContentSize().width/2 * userPaddle_->getScaleX();
        
        // calculate the final position of the paddle
        CCPoint destPosition = ccp(MIN(MAX(minX, location.x), maxX), userPaddle_->getPosition().y);
        
        // calculate the distance from the paddle to the user's touch
        float diffX = fabs(destPosition.x - userPaddle_->getPosition().x);
        
        userPaddle_->stopAllActions();
        
        // if the touch was within the area of the paddle, "grab the paddle" and set its position to the touch location
        if (diffX < userPaddle_->getContentSize().width/2 * userPaddle_->getScaleX() 
            || userHasGrabbedThePaddle_)
        {
            userHasGrabbedThePaddle_ = true;
            userPaddle_->setPosition(destPosition);
        }
        else
        {
            // otherwise animate the paddle to that position
            userPaddle_->runAction(CCMoveTo::actionWithDuration(0.3 * diffX / gameArea_.size.width, destPosition));
        }
    }
}

void GameScene::ccTouchesEnded(CCSet* touches, CCEvent* event)
{
    // let go of the paddle
    userHasGrabbedThePaddle_ = false;
}

