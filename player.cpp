#include "player.h"
#include <SDL.h>
#include <stdio.h>
#include <cmath>
#include <string>
#include "texture.h"
#include "bullet.h"

namespace xx {

Player::Player(Entity * e) {
    mEntity=e;
    //Initialize
    mVel = cpvzero;
    Rpressed=false;
    Lpressed=false;
    maxAmmo=5;
}

Player::~Player() {
}

void Player::render(SDL_Renderer *r) {

    for(int i=0; i<=maxAmmo; i++)
    {
        if( ammo[i].checkExist() )
            ammo[i].render(r);
        if( ammo[i].checkExpl() )
            ammo[i].eRender(r);
    }
}

cpFloat angleAdd(cpFloat angle, cpFloat delta) {
    angle += delta;
    if (angle >= 2*M_PI) {
        angle -= 2*M_PI;
    }
    if (angle < 0) {
        angle += 2*M_PI;
    }
    return angle;
}

cpVect Player::vectorForward() {
    cpFloat vx = sin(cpBodyGetAngle(mEntity->body())*M_PI/180)*PLAYER_VEL;
    cpFloat vy = -cos(cpBodyGetAngle(mEntity->body())*M_PI/180)*PLAYER_VEL;
    return cpv(vx, vy);
}

 void Player::handleEvent(SDL_Event e, SDL_Renderer *r, cpSpace *space, PlayerChange *pc) {
    PlayerMove *m = pc->mutable_move();
    //Rotation
    if( e.type == SDL_MOUSEMOTION) {
        int x,y;
        SDL_GetRelativeMouseState(&x,&y);
        if( x<-1 )
            rotLeft();
        if( x>1 )
            rotRight();
    }
    m->set_angle(cpBodyGetAngle(body()));

    //If the right button was pressed
    if ( e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT ) {
        Rpressed = true;
    } else if ( e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT ) {
            Rpressed = false;
            mVel = cpvzero;
    }

    //if holding the right button
    if (Rpressed) {
        mVel = vectorForward();
        m->set_forwards(m->forwards()+1);
    }
    //If the left button was pressed
    if ( e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT ) {
        Lpressed = true;
    } else if ( e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT ) {
        Lpressed = false;
    }
}

void Player::handleFire(SDL_Renderer *r, cpSpace *space, cpFloat &time) {
    //If holding the left button
    if (Lpressed) {
        cpBody *body = mEntity->body();
        for( int i=0; i<=maxAmmo; i++) {
            if ( !ammo[i].checkExist() && time>2 ) {
                time = 0;
                ammo[i].getPlayerPos(  body->p.x , body->p.y, mAngle, mEntity->sprite().height()/2 );
                ammo[i].createBullet(r, space, &ammo[i], 500);
                int tVelX,tVelY;
                tVelX=sin( cpBodyGetAngle( body ) )*PLAYER_VEL;
                tVelY=-cos( cpBodyGetAngle( body ) )*PLAYER_VEL;
                ammo[i].getPlayerVel(tVelX, tVelY);
            }
        }
    }
}

void Player::rotLeft() {
    cpFloat angle = cpBodyGetAngle(mEntity->body());
    angle = angleAdd(angle, -PLAYER_RAD);
    mAngle = angle;
    cpBodySetAngle(mEntity->body(), angle);
    //printf("%f %f\n", mAngle, mEntity->body()->a);
}

void Player::rotRight() {
    cpFloat angle = cpBodyGetAngle(mEntity->body());
    angle = angleAdd(angle, PLAYER_RAD);
    mAngle = angle;
    cpBodySetAngle(mEntity->body(), angle);
     //printf("%f %f\n", mAngle, mEntity->body()->a);
}

void Player::fly() {
    //Apply impulse
    cpBodyApplyImpulseAtLocalPoint(mEntity->body(), mVel, cpv(0, 0));
    cpBody * body = mEntity->body();
    for(int i=0; i<=maxAmmo; i++)
        if( ammo[i].checkExist() )
            ammo[i].moveBullet();

    //Move around screen
    if( body->p.x > SCREEN_WIDTH + mEntity->width()/2 )
        body->p.x = -mEntity->width();
    if( body->p.x < -mEntity->width() )
        body->p.x = SCREEN_WIDTH + mEntity->width()/2;
    if( body->p.y > SCREEN_HEIGHT + mEntity->height()/2)
        body->p.y = -mEntity->height();
    if( body->p.y < -mEntity->height() )
        body->p.y = SCREEN_HEIGHT + mEntity->height()/2;
}

void freeBulle(Bullet a){
    a.free();
}

void Player::hurt(int dam){
    if (dam<hp) hp-=dam; else {
        hp=0;
    }
    }

}
