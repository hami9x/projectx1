#include "player.h"
#include <SDL.h>
#include "texture.h"
#include <stdio.h>
#include <cmath>

XxPlayer::XxPlayer()
{
    //Initialize
    mPosX=0;
    mPosY=0;
    mVelX=0;
    mVelY=0;
    press=false;
    rdegrees=0;
    mdegrees=90;
}

XxPlayer::~XxPlayer()
{
}

void XxPlayer::getAvatar(SDL_Renderer *r,char s[])
{
    aircraft.loadFromFile(r,s);

}

void XxPlayer::render(SDL_Renderer *r)
{
    aircraft.render(r, mPosX, mPosY, NULL, rdegrees, NULL, SDL_FLIP_NONE);
}

void XxPlayer::handleEvent(SDL_Event e)
{
    //Rotation
    if( e.type == SDL_MOUSEMOTION)
    {
        int x,y;
        SDL_GetRelativeMouseState(&x,&y);
        if( x<-1 )
            left();
        if( x>1 )
            right();
    }
    //If right button was pressed
    if ( e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT )
        press=true;
    //if right button was released
    else if ( e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_RIGHT )
        {
            press=false;
            mVelX=0;
            mVelY=0;
        }
    //if holding right button
    if (press)
    {
        mVelY=-sin(mdegrees*PI/180)*PLAYER_VEL;
        mVelX=cos(mdegrees*PI/180)*PLAYER_VEL;
    }
}

void XxPlayer::left()
{
    rdegrees -= PLAYER_RAD;
    if(rdegrees<-360)
        rdegrees+=360;

    mdegrees += PLAYER_RAD;
    if ( mdegrees > 360 )
        mdegrees -= 360;
}

void XxPlayer::right()
{
    rdegrees += PLAYER_RAD;
    if (rdegrees>360)
        rdegrees -=360;

    mdegrees -= PLAYER_RAD;
    if ( mdegrees < 0 )
        mdegrees += 360;
}

void XxPlayer::fly()
{
    //Move the aircraft left or right
    mPosX += round(mVelX);

    //went too far to the left
    if ( mPosX + aircraft.width() < 0 )
        mPosX = sWidth;
    //went too far to the right
    if ( mPosX > sWidth )
        mPosX = -aircraft.width();

    //Move the aircraft up or down
    mPosY += round(mVelY);

    //went too far up
    if ( mPosY + aircraft.height() < 0 )
        mPosY = sHeight;
    //went too far down
    if ( mPosY > sHeight )
        mPosY = -aircraft.height();
}

void XxPlayer::getScreenSize(int a, int b)
{
    sHeight=a;
    sWidth=b;
}
