#ifndef ANIMATION_H_INCLUDED
#define ANIMATION_H_INCLUDED
#include "texture.h"
#include <SDL.h>
#include <string>

namespace xx {

class Animation {
    public:

        Animation();

        ~Animation();

        void setupAnimation(SDL_Renderer *r, std::string spritelink, int frames, int width, int height);
        void renderAnimation(SDL_Renderer *r);
        void setAnimation(int x, int y, int frameRate);
    private:
        Texture mSprite;
        SDL_Rect gSpriteClips[];
        int mX,mY,currentFrame,frameRate,totalFrame;
        bool stop;
};

}

#endif // ANIMATION_H_INCLUDED
