#include "explosion.h"
#include "texture.h"
//TODO: Move it into a class
//No more global and unnecessary logic

namespace xx {

//Explosion animation
const int EXPLOSION_ANIMATION_FRAME = 5;
const int EXPLOSION_MAXIMUM_SPRITES = 3;
SDL_Rect gSpriteClips[ EXPLOSION_ANIMATION_FRAME ];
Texture explosionSprite[EXPLOSION_MAXIMUM_SPRITES];
bool explosionCheck[EXPLOSION_MAXIMUM_SPRITES]; // check if sprite not in use
double explosionPosition[EXPLOSION_MAXIMUM_SPRITES][2];
int explosionFrame[EXPLOSION_MAXIMUM_SPRITES]; //explosion current frame

//Explosion render
void explosionRenderStep(SDL_Renderer *r, int spriteNumber) {
    //Render current frame
    int i = spriteNumber;
    SDL_Rect* currentClip = &gSpriteClips[ explosionFrame[i]/20 ];
    explosionSprite[i].render(r, (int)explosionPosition[i][0], (int)explosionPosition[i][1], currentClip, 0, NULL, SDL_FLIP_NONE);
    explosionFrame[i]++;
    if( explosionFrame[i] > 80 ){
        explosionCheck[i] = false;
        explosionFrame[i] = 0;
    }
}

void explosionCheckRender(SDL_Renderer *r) {
    for(int i=0; i < EXPLOSION_MAXIMUM_SPRITES; i++)
        if( explosionCheck[i] ) {
            explosionRenderStep(r, i);
        }
}

void explosionPrepare(SDL_Renderer *r) {
    //Load explosion animation texture
    bool check = true;
    for(int i=0; i < EXPLOSION_MAXIMUM_SPRITES; i++){
        if ( !explosionSprite[i].loadFromFile(r, "explosion.png") )
            check = false;
        explosionCheck[i] = false;
    }
    if ( check )
    {
        //Set sprite clips
        gSpriteClips[ 0 ].x =   0;
        gSpriteClips[ 0 ].y =   0;
        gSpriteClips[ 0 ].w =  118;
        gSpriteClips[ 0 ].h = 118;

        gSpriteClips[ 1 ].x =   118;
        gSpriteClips[ 1 ].y =   0;
        gSpriteClips[ 1 ].w =  118;
        gSpriteClips[ 1 ].h = 118;

        gSpriteClips[ 2 ].x =   236;
        gSpriteClips[ 2 ].y =   0;
        gSpriteClips[ 2 ].w =  118;
        gSpriteClips[ 2 ].h = 118;

        gSpriteClips[ 3 ].x =   354;
        gSpriteClips[ 3 ].y =   0;
        gSpriteClips[ 3 ].w =  118;
        gSpriteClips[ 3 ].h = 118;

        gSpriteClips[ 4 ].x =   472;
        gSpriteClips[ 4 ].y =   0;
        gSpriteClips[ 4 ].w =  118;
        gSpriteClips[ 4 ].h = 118;
    }
}

//Setting explosion
void explosionSet(int spriteNumber, double x, double y )
{
    int i = spriteNumber;
    explosionCheck[i] = true;
    explosionPosition[i][0] = x;
    explosionPosition[i][1] = y;
}

//Ns end
}
