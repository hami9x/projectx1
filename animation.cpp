#include "animation.h"
#include <string>
#include <SDL.h>

namespace xx {

    Animation::Animation() {
    }

    Animation::~Animation() {}

    void Animation::setupAnimation(SDL_Renderer *r,std::string link, int frame, int width, int height) {
        if(mSprite.loadFromFile(r, link))
        {
            for(int i=0;i< frame;i++)
            {
                //Set sprite clips
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
        stop = false;
    }

    void Animation::renderAnimation(SDL_Renderer *r){
        if(!stop)
        {
            SDL_Rect *currentClip=&gSpriteClips[ currentFrame/frameRate ];
            mSprite.render(r, mX, mY, currentClip, 0,NULL, SDL_FLIP_NONE);
            currentFrame++;
            if( currentFrame> (totalFrame*frameRate) )
                stop = true;
        }

    }

}
