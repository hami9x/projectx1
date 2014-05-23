#include "ops.h"

namespace xx {

//Explosion animation
const int EXPLOSION_ANIMATION_FRAME = 5;
const int EXPLOSION_MAXIMUM_SPRITES = 3;
SDL_Rect gSpriteClips[ EXPLOSION_ANIMATION_FRAME ];
Texture explosionSprite[EXPLOSION_MAXIMUM_SPRITES];
bool explosionCheck[EXPLOSION_MAXIMUM_SPRITES]; // check if sprite not in use
cpFloat explosionPosition[EXPLOSION_MAXIMUM_SPRITES][2];
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
void explosionSet(int spriteNumber, cpFloat x, cpFloat y )
{
    int i = spriteNumber;
    explosionCheck[i] = true;
    explosionPosition[i][0] = x;
    explosionPosition[i][1] = y;
}

//in/out Cloud counting
void planeCloud(cpShape *shape,int num){
    cpBody *Body = cpShapeGetBody(shape);
    Player *p1 = (Player*)cpBodyGetUserData(Body);
    p1->setInCloud(num);
}

//Post-step: Ammo free
static void
ammoFree( cpSpace *space, cpShape *shape, void *unused)
{
    cpBody *Body = cpShapeGetBody(shape);
    Bullet *ammo = (Bullet*)cpBodyGetUserData(Body);
    cpVect pos = cpBodyGetPosition(Body);
    printf("body->p: %f %f \n", Body->p.x, Body->p.y);
    printf("Position: %f %f \n",pos.x,pos.y);
    //ammo->explosion(Body->p.x - 59, Body->p.y - 59);
    for(int i=0; i < EXPLOSION_MAXIMUM_SPRITES; i++)
        if( !explosionCheck[i] )
            explosionSet(i, Body->p.x - 59, Body->p.y - 59);
    ammo->free();
}

//COLLISION HANDLER
static cpBool beginFunc(cpArbiter *arb, cpSpace *space, void* unused)
{
    cpShape *a,*b;
    cpArbiterGetShapes(arb, &a,&b);
    if( cpShapeGetCollisionType(a) == PLANE_TYPE && cpShapeGetCollisionType(b) == CLOUD_TYPE )
        planeCloud(a, 3);
    if( cpShapeGetCollisionType(a) == BULLET_TYPE && cpShapeGetCollisionType(b) == CLOUD_TYPE )
        printf("Bullet hit Cloud\n");
    if( cpShapeGetCollisionType(a) == BULLET_TYPE && cpShapeGetCollisionType(b) == PLANE_TYPE )
    {
        cpBool check = cpShapeGetSensor(a);
        if(!check){
            printf("Hit plane\n");

            cpSpaceAddPostStepCallback( space, (cpPostStepFunc)ammoFree, a, NULL);
        }
    }

    return 0;
}

static void separateFunc (cpArbiter *arb, cpSpace *space, void *unused)
{
    cpShape *a,*b;
    cpArbiterGetShapes(arb, &a,&b);
    if( cpShapeGetCollisionType(a) == PLANE_TYPE && cpShapeGetCollisionType(b) == CLOUD_TYPE )
       planeCloud(a, -3);
    if( cpShapeGetCollisionType(a) == BULLET_TYPE && cpShapeGetCollisionType(b) == CLOUD_TYPE )
        printf("Bullet separate Cloud\n");
}

void collision(int type_a, int type_b, cpSpace *space, Player *pl)
{
    //collision
    cpCollisionHandler * handler = cpSpaceAddCollisionHandler(space, type_a, type_b);
    handler->beginFunc = beginFunc;
    handler->separateFunc = separateFunc;
}

void setupCollisions(cpSpace *space, Player *p1) {
    collision(PLANE_TYPE, CLOUD_TYPE, space, p1);
    collision(BULLET_TYPE, CLOUD_TYPE, space, p1);
    collision(BULLET_TYPE, PLANE_TYPE, space, p1);
}

}
