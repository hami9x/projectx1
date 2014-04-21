#include "player.h"

#include <SDL.h>
#include <stdio.h>
#include <cmath>

#include "texture.h"

namespace xx {

Player::Player(Entity * e) {
    mEntity=e;
    //Initialize
    mVelX=0;
    mVelY=0;
    pressed=false;
    rdegrees=0;
    mdegrees=90;
}

Player::~Player() {
}

void Player::render(SDL_Renderer *r) {

    mEntity->render(r);
}

cpFloat angleAdd(cpFloat angle, cpFloat delta) {
    angle += delta;
    if (angle >= 360) {
        angle -= 360;
    }
    if (angle < 0) {
        angle += 360;
    }
    return angle;
}

void Player::handleEvent(SDL_Event e) {
    //Rotation
    if( e.type == SDL_MOUSEMOTION) {
        int x,y;
        SDL_GetRelativeMouseState(&x,&y);
        if( x<-1 )
            rotLeft();
        if( x>1 )
            rotRight();
    }
    //If the button was pressed
    if ( e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT ) {
        pressed=true;
    } else if ( e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT ) {
            pressed=false;
            mVelX=0;
            mVelY=0;
    }
    //if holding the button
    if (pressed) {
        printf("%f\n", cpBodyGetAngle(mEntity->body()));
        mVelX=-sin(cpBodyGetAngle(mEntity->body()))*PLAYER_VEL;
        mVelY=-sin(cpBodyGetAngle(mEntity->body()))*PLAYER_VEL;
    }
}

void Player::rotLeft() {
    cpFloat angle = cpBodyGetAngle(mEntity->body());
    angle = angleAdd(angle, -PLAYER_RAD);
    cpBodySetAngle(mEntity->body(), angle);
}

void Player::rotRight() {
    cpFloat angle = cpBodyGetAngle(mEntity->body());
    angle = angleAdd(angle, PLAYER_RAD);
    cpBodySetAngle(mEntity->body(), angle);
}

void Player::fly() {
    //Move the aircraft left or right
    cpBodyApplyImpulse(mEntity->body(), cpv(mVelX, mVelY), cpv(0, 0));
}

}
