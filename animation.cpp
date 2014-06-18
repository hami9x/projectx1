#include "animation.h"
#include <string>
#include <SDL.h>
#include <iostream>

namespace xx {

    Animation::Animation(): currentFrame(-1), frameRate(0), totalFrame(-1), mX(0), mY(0) {
    }

    Animation::~Animation() {}

    void Animation::setupAnimation(SDL_Renderer *r,std::string link, int frame, int width, int height) {
        std::cout<<&mSprite<<std::endl;
        if(mSprite.loadFromFile(r, link))
        {
            gSpriteClips = new SDL_Rect[frame];
            for(int i=0;i< frame;i++)
            {
                //Set sprite clips
                std::cout<< "Load frame "<<i+1<<std::endl;
                gSpriteClips[ i ].x = width*i;
                gSpriteClips[ i ].y = 0;
                gSpriteClips[ i ].w = width;
                gSpriteClips[ i ].h = height;
            }
            totalFrame = frame;
        }

    }

    void Animation::setAnimation(int x, int y, int f) {
        mX = x;
        mY = y;
        frameRate = f;
        currentFrame = 0;
        active = true;
    }

    void Animation::renderAnimation(SDL_Renderer *r){
        if(active)
        {
            SDL_Rect *currentClip=&gSpriteClips[ currentFrame/frameRate ];
            mSprite.render(r, mX, mY, currentClip, 0,NULL, SDL_FLIP_NONE);
            currentFrame++;
            if( currentFrame> (totalFrame*frameRate) )
                active = false;
        }

    }

// Explosion Animation
const int n=10;
Animation explosion[n];
void ExplosionCreate(SDL_Renderer* r) {
    for(int i=0; i<n; i++)
    {
        explosion[i].setupAnimation(r, "explosion.png", 5, 118, 118);
    }
}

void Explosion(double x, double y) {
    for(int i=0; i<n; i++)
    {
        if(!explosion[i].checkActive())
        {
            std::cout<<"Start explosion at "<<x<<" "<<y<<std::endl;
            explosion[i].setAnimation(x, y, 20);
            break;
        }
    }
}

void ExplosionRender(SDL_Renderer* r) {
    for(int i=0; i<n; i++)
        explosion[i].renderAnimation(r);
}

}
