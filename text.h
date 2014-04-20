#ifndef TEXT_H_INCLUDED
#define TEXT_H_INCLUDED

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

#include "texture.h"

using namespace std;

namespace xx {

    class Text {
        Texture mTexture;
        SDL_Renderer *mRenderer;
        TTF_Font *mFont;
        SDL_Color mColor;
        int mWrapWidth;
        string mText;

        public:
        Text(string, TTF_Font*, SDL_Color);
        ~Text();

        int width();
        int height();

        void render(SDL_Renderer *r, int x, int y, int wrapWidth);
    };

}

#endif // TEXT_H_INCLUDED
