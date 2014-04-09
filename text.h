#ifndef TEXT_H_INCLUDED
#define TEXT_H_INCLUDED

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

#include "texture.h"

using namespace std;

class XxText {
    XxTexture mTexture;
    SDL_Renderer *mRenderer;
    TTF_Font *mFont;
    SDL_Color mColor;
    int mWrapWidth;
    string mText;

    public:
    XxText(string, TTF_Font*, SDL_Color);
    ~XxText();

    int width();
    int height();

    bool render(SDL_Renderer *r, int x, int y, int wrapWidth);
};

#endif // TEXT_H_INCLUDED
