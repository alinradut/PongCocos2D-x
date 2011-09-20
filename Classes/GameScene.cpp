//
//  GameScene.cpp
//  Pong
//
//  Created by Clawoo on 9/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "GameScene.h"

USING_NS_CC;

#define CC_SPRITE_RECT(sprite) CCRectMake(CC_SPRITE_LEFT(sprite), \
            CC_SPRITE_BOTTOM(sprite), \
            CC_SPRITE_ACTUAL_WIDTH(sprite), \
            CC_SPRITE_ACTUAL_HEIGHT(sprite))

#define CC_SPRITE_ACTUAL_WIDTH(sprite) sprite->getContentSize().width * sprite->getScaleX()
#define CC_SPRITE_ACTUAL_HEIGHT(sprite) sprite->getContentSize().height * sprite->getScaleY()

#define CC_SPRITE_LEFT(sprite) sprite->getPosition().x - CC_SPRITE_ACTUAL_WIDTH(sprite)/2
#define CC_SPRITE_RIGHT(sprite) sprite->getPosition().x + CC_SPRITE_ACTUAL_WIDTH(sprite)/2

#define CC_SPRITE_TOP(sprite) sprite->getPosition().y + CC_SPRITE_ACTUAL_HEIGHT(sprite)/2
#define CC_SPRITE_BOTTOM(sprite) sprite->getPosition().y - CC_SPRITE_ACTUAL_HEIGHT(sprite)/2

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
    ball_->setPosition(ccp(userPaddle_->getPosition().x, CC_SPRITE_TOP(userPaddle_) + CC_SPRITE_ACTUAL_HEIGHT(ball_)/2));
    this->addChild(ball_);
    
    velocity_ = ccp(0,0);
    
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
    // bounce off the walls, account for the 1px line around the game area
    if (CC_SPRITE_LEFT(ball_) + velocity_.x <= gameArea_.origin.x + 1
        || CC_SPRITE_RIGHT(ball_) + velocity_.x >= gameArea_.origin.x + gameArea_.size.width - 1)
    {
        velocity_.x = -velocity_.x;
    }
    
    // temporary
    if (CC_SPRITE_BOTTOM(ball_) <= gameArea_.origin.y + 1
        || CC_SPRITE_TOP(ball_) + velocity_.y >= gameArea_.origin.y + gameArea_.size.height - 1)
    {
        velocity_.y = -velocity_.y;
    }
    
    // the rect of the future position of the ball
    CCRect futureRect = CCRectMake(CC_SPRITE_LEFT(ball_) + velocity_.x, 
                                   CC_SPRITE_BOTTOM(ball_) + velocity_.y, 
                                   CC_SPRITE_ACTUAL_WIDTH(ball_), 
                                   CC_SPRITE_ACTUAL_HEIGHT(ball_));
    
    // check if the future position of the ball will intersect the user's paddle
    if (!this->handleCollision(userPaddle_, futureRect))
    {
        if (!this->handleCollision(cpuPaddle_, futureRect))
        {
            ballIsEscapingPaddle_ = false;
        }
    }
    
    ball_->setPosition(ccp(ball_->getPosition().x + velocity_.x, ball_->getPosition().y + velocity_.y));
    
    if (CC_SPRITE_BOTTOM(ball_) <= gameArea_.origin.y + 1)
    {
        CCLog("Player Lost");
    }
    
    if (CC_SPRITE_TOP(ball_) >= gameArea_.origin.y + gameArea_.size.height - 1)
    {
        CCLog("CPU Lost");
    }
}

bool GameScene::handleCollision(cocos2d::CCSprite *paddle, cocos2d::CCRect ballRect)
{
    if (CCRect::CCRectIntersectsRect(ballRect, CC_SPRITE_RECT(paddle)))
    {
        if (!ballIsEscapingPaddle_)
        {
            ballIsEscapingPaddle_ = true;
            // check if the collision happens when the bottom of the ball is 
            if (CC_SPRITE_TOP(paddle) > CC_SPRITE_BOTTOM(ball_))
            {
                if (paddle->getPosition().x <= ballRect.origin.x)
                {
                    CCLog("bounced off the right");
                    velocity_.x *= -1;
                    velocity_.y *= -1;
                }
                else if (paddle->getPosition().y >= ballRect.origin.x + ballRect.size.width)
                {
                    CCLog("bounced off the left side");
                    velocity_.x *= -1;
                    velocity_.y *= -1;
                }
            }
            else
            {
                CCLog("normal bounce");
                velocity_.y *= -1;
            }
            
            // split the paddle in 3 equal parts
            // [____|____|____]
            // if the ball hits one the left or the right side, increase the ball speed
            /*        float leftSide = CC_SPRITE_LEFT(paddle) + CC_SPRITE_ACTUAL_WIDTH(paddle)/3;
             float rightSide = CC_SPRITE_LEFT(paddle) + CC_SPRITE_ACTUAL_WIDTH(paddle)/3 * 2;
             if (futureRect.origin.x + futureRect.size.width/2 < leftSide)
             {
             CCLog("increase speed by 10 percent, left side");
             velocity_.x *= 1.1;
             }
             if (futureRect.origin.x + futureRect.size.width/2 > rightSide)
             {
             CCLog("increase speed by 10 percent, right side");
             velocity_.x *= 1.1;
             }*/
        }
        return true;
    }
    return false;
}

void GameScene::handleUserTouch(CCTouch *touch)
{
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

        // if the touch was within the area below the paddle, "grab" the paddle and set its position to the touch location
        if (diffX < userPaddle_->getContentSize().width/2 * userPaddle_->getScaleX() 
            || userHasGrabbedThePaddle_
            || (velocity_.x == 0 && velocity_.y == 0))
        {
            userHasGrabbedThePaddle_ = true;
            userPaddle_->setPosition(destPosition);
            
            // if the game hasn't started yet, move the ball with the paddle
            if (velocity_.x == 0 && velocity_.y == 0)
            {
                ball_->setPosition(ccp(userPaddle_->getPosition().x, CC_SPRITE_TOP(userPaddle_) + CC_SPRITE_ACTUAL_HEIGHT(ball_)/2));
            }
        }
        // otherwise animate the paddle to that position
        else
        {
            userPaddle_->runAction(CCMoveTo::actionWithDuration(0.3 * diffX / gameArea_.size.width, destPosition));
        }
    }
}

void GameScene::ccTouchesBegan(cocos2d::CCSet* touches, cocos2d::CCEvent* event)
{
    // get the touch object
    CCTouch* touch = (CCTouch *)touches->anyObject();
    this->handleUserTouch(touch);
}

void GameScene::ccTouchesMoved(CCSet* touches, CCEvent* event)
{
    // get the touch object
    CCTouch* touch = (CCTouch *)touches->anyObject();
    this->handleUserTouch(touch);
}

void GameScene::ccTouchesEnded(CCSet* touches, CCEvent* event)
{
    // let go of the paddle
    userHasGrabbedThePaddle_ = false;
    
    if (velocity_.x == 0 && velocity_.y == 0)
    {
        
        // get the touch object
        CCTouch* touch = (CCTouch *)touches->anyObject();
        // obtain the location of the touch
        CCPoint location = touch->locationInView(touch->view());
        // convert the location of the touch to GL coordinates
        location = CCDirector::sharedDirector()->convertToGL(location);
        
        if (CCRect::CCRectContainsPoint(gameArea_, location))
        {
            // check that the touch was made above the paddle
            if (location.y > ball_->getPosition().y)
            {
                // determine the slope of the line between the touch and the ball position
                float m = (location.x - ball_->getPosition().x) / (location.y - ball_->getPosition().y);
                
                // determine the angle between the touch and a horizontal line
                float angle = atanf(m);
                
                // do not allow touches to be at a smaller angle than 70 degrees with respect to a vertical line
                if (CC_RADIANS_TO_DEGREES(angle) > 70)
                {
                    angle = CC_DEGREES_TO_RADIANS(70);
                }
                if (CC_RADIANS_TO_DEGREES(angle) < -70)
                {
                    angle = CC_DEGREES_TO_RADIANS(-70);
                }
                
                CCPoint initialVelocity = ccp(0,0);
                
                // compute the horizontal velocity
                initialVelocity.x = 3 * sin(angle);
                
                // compute the vertical velocity
                initialVelocity.y = 3 * cos(angle);
                
                velocity_ = initialVelocity;
            }
        }
    }
}

