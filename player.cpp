#include "player.h"

#include <SDL.h>
#include <stdio.h>
#include <cmath>
#include <string>
#include "text.h"
#include "texture.h"

namespace xx {

Player::Player(Entity * e) {
    mEntity=e;
    //Initialize
    mVelX=0;
    mVelY=0;
    pressed=false;
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
    printf("%f\n",angle);
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
        mVelX=sin(cpBodyGetAngle(mEntity->body())*PI/180)*PLAYER_VEL;
        mVelY=-cos(cpBodyGetAngle(mEntity->body())*PI/180)*PLAYER_VEL;
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

void Player::drawHp1(SDL_Renderer* mRenderer){
    char num[100]="HP: ",temp[100]="";
    SDL_Rect fillRect = { 0, 0, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 20 };
    SDL_SetRenderDrawColor( mRenderer, 0x99,0x33,0x66, 0x00 );
    SDL_RenderFillRect( mRenderer, &fillRect );

    fillRect = { 4, 4, SCREEN_WIDTH / 3 - 8, SCREEN_HEIGHT / 20 -8};
    SDL_SetRenderDrawColor( mRenderer, 0xFF,0xDD,0xFF, 0x00 );
    SDL_RenderFillRect( mRenderer, &fillRect );

    fillRect = { 4, 4, (SCREEN_WIDTH/3 - 8)*(double)hp/maxhp, SCREEN_HEIGHT / 20 -8};
    SDL_SetRenderDrawColor( mRenderer, 0xFF, 0x99, 0xCC, 0x00 );
    SDL_RenderFillRect( mRenderer, &fillRect );
    sprintf(temp,"%d",hp);
    strcat(num,temp);
    strcat(num,"/");
    sprintf(temp,"%d",maxhp);
    strcat(num,temp);
    Text hptxt(num,TTF_OpenFont( "BKANT.ttf", 20 ), {94,19,83});
    hptxt.render(mRenderer,10,35,200);
    //SDL_RenderPresent(mRenderer);
}
void Player::drawHp2(SDL_Renderer* mRenderer){
    char num[100]="HP: ",temp[100]="";
    SDL_Rect fillRect = { SCREEN_WIDTH / 1.5, 0, SCREEN_WIDTH / 3, SCREEN_HEIGHT / 20 };
    SDL_SetRenderDrawColor( mRenderer, 0x99,0x33,0x66, 0x00 );
    SDL_RenderFillRect( mRenderer, &fillRect );

    fillRect = { SCREEN_WIDTH / 1.5 + 4, 4, SCREEN_WIDTH / 3 - 8, SCREEN_HEIGHT / 20 -8};
    SDL_SetRenderDrawColor( mRenderer, 0xFF,0xDD,0xFF, 0x00 );
    SDL_RenderFillRect( mRenderer, &fillRect );

    fillRect = { SCREEN_WIDTH / 1.5 + 4, 4, (SCREEN_WIDTH/3 - 8)*(double)hp/maxhp, SCREEN_HEIGHT / 20 -8};
    SDL_SetRenderDrawColor( mRenderer, 0xFF, 0x99, 0xCC, 0x00 );
    SDL_RenderFillRect( mRenderer, &fillRect );
    sprintf(temp,"%d",hp);
    strcat(num,temp);
    strcat(num,"/");
    sprintf(temp,"%d",maxhp);
    strcat(num,temp);
    Text hptxt(num,TTF_OpenFont( "BKANT.ttf", 20 ), {94,19,83});
    hptxt.render(mRenderer,676,35,200);
    //SDL_RenderPresent(mRenderer);
}

}
