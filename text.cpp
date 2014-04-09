#include "text.h"

#include <cassert>

XxText::XxText(string text, TTF_Font *font, SDL_Color color) {
    assert(font != NULL);

    mRenderer = NULL;
    mText = text;
    mFont = font;
    mColor = color;
    mWrapWidth = -1;
}

bool XxText::render(SDL_Renderer *r, int x, int y, int wrapWidth)
{
    assert(wrapWidth > 0);
    if (mRenderer != r || mWrapWidth != wrapWidth)
    {
        SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(mFont, mText.c_str(), mColor, wrapWidth);
        mTexture.updateSurface(r, surface);
    }

    mRenderer = r;
    mWrapWidth = wrapWidth;

    mTexture.render(mRenderer, x, y);
}

XxText::~XxText() {
    mTexture.free();
}

int XxText::width()
{
	return mTexture.width();
}

int XxText::height()
{
	return mTexture.width();
}
