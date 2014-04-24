#include "player.h"

#include <SDL.h>
#include <stdio.h>
#include <cmath>
#include <string>
#include "text.h"
#include "texture.h"
#include "bullet.h"

namespace xx {

Player::Player(Entity * e) {
    mEntity=e;
    //Initialize
    mVelX=0;
    mVelY=0;
    Rpressed=false;
    Lpressed=false;
}

Player::~Player() {
}

void Player::render(SDL_Renderer *r) {

    mEntity->render(r);
    if( ammo[0].checkExist() ){
        ammo[0].render(r);
    }
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

void Player::handleEvent(SDL_Event e, SDL_Renderer *r, cpSpace *space) {
    //Rotation
    if( e.type == SDL_MOUSEMOTION) {
        int x,y;
        SDL_GetRelativeMouseState(&x,&y);
        if( x<-1 )
            rotLeft();
        if( x>1 )
            rotRight();
    }
    //If the right button was pressed
    if ( e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT ) {
        Rpressed = true;
    } else if ( e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT ) {
            Rpressed = false;
            mVelX = 0;
            mVelY = 0;
    }
    //if holding the button
    if (Rpressed) {
        mVelX=sin(cpBodyGetAngle(mEntity->body())*PI/180)*PLAYER_VEL;
        mVelY=-cos(cpBodyGetAngle(mEntity->body())*PI/180)*PLAYER_VEL;
    }
    //If the left button was pressed
    if ( e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT ) {
        Lpressed = true;
        ammo[0].getPlayerPos( mEntity->body()->p.x + mEntity->sprite().width()/2, mEntity->body()->p.y + mEntity->sprite().height()/2, mAngle );
        ammo[0].createBullet(r, space, 100);
        int tVelX,tVelY;
        tVelX=sin(cpBodyGetAngle(mEntity->body())*PI/180)*PLAYER_VEL;
        tVelY=-cos(cpBodyGetAngle(mEntity->body())*PI/180)*PLAYER_VEL;
        ammo[0].getPlayerVel(tVelX, tVelY);
    }
}

void Player::rotLeft() {
    cpFloat angle = cpBodyGetAngle(mEntity->body());
    angle = angleAdd(angle, -PLAYER_RAD);
    mAngle = angle;
    cpBodySetAngle(mEntity->body(), angle);
}

void Player::rotRight() {
    cpFloat angle = cpBodyGetAngle(mEntity->body());
    angle = angleAdd(angle, PLAYER_RAD);
    mAngle = angle;
    cpBodySetAngle(mEntity->body(), angle);
}

void Player::fly() {
    //Apply impulse
    cpBodyApplyImpulse(mEntity->body(), cpv(mVelX, mVelY), cpv(0, 0));
    if(ammo[0].checkExist())
        ammo[0].moveBullet();
    cpBody *body = mEntity->body();
    //Move around screen
    if( body->p.x > SCREEN_WIDTH )
        body->p.x = -mEntity->width();
    if( body->p.x < -mEntity->width() )
        body->p.x = SCREEN_WIDTH;
    if( body->p.y > SCREEN_HEIGHT )
        body->p.y = -mEntity->height();
    if( body->p.y < -mEntity->height() )
        body->p.y = SCREEN_HEIGHT;
}

void Player::drawHp(SDL_Renderer* mRenderer,int x,int y){
    char num[100]="HP: ",temp[100]="";
    SDL_Rect fillRect = { x, y, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 20 };
    SDL_SetRenderDrawColor( mRenderer, 0x99,0x33,0x66, 0x00 );
    SDL_RenderFillRect( mRenderer, &fillRect );

    fillRect = { x+4, y+4, SCREEN_WIDTH / 3 - 8, SCREEN_HEIGHT / 20 -8};
    SDL_SetRenderDrawColor( mRenderer, 0xFF,0xDD,0xFF, 0x00 );
    SDL_RenderFillRect( mRenderer, &fillRect );

    fillRect = { x+4, y+4, (SCREEN_WIDTH/3 - 8)*(double)hp/maxhp, SCREEN_HEIGHT / 20 -8};
    SDL_SetRenderDrawColor( mRenderer, 0xFF, 0x99, 0xCC, 0x00 );
    SDL_RenderFillRect( mRenderer, &fillRect );
    sprintf(temp,"%d",hp);
    strcat(num,temp);
    strcat(num,"/");
    sprintf(temp,"%d",maxhp);
    strcat(num,temp);
    Text hptxt(num,TTF_OpenFont( "BKANT.ttf", 20 ), {94,19,83});
    hptxt.render(mRenderer,x+10,y+35,200);
}
}
