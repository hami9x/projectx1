#include <cassert>
#include "text.h"

namespace xx {

Text::Text(string text, TTF_Font *font, SDL_Color color) {
    assert(font != NULL);

    mRenderer = NULL;
    mText = text;
    mFont = font;
    mColor = color;
    mWrapWidth = -1;
}

void Text::render(SDL_Renderer *r, int x, int y, int wrapWidth)
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

Text::~Text() {
    mTexture.free();
}

int Text::width()
{
	return mTexture.width();
}

int Text::height()
{
	return mTexture.width();
}

}
