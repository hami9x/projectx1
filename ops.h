#ifndef OPS_H_INCLUDED
#define OPS_H_INCLUDED

#include "chipmunk_private.h"
#include "player.h"

namespace xx {

//static cpBool beginFunc(cpArbiter *arb, cpSpace *space, void* unused);
//
//static void separateFunc (cpArbiter *arb, cpSpace *space, void *unused);
//
//void collision(int type_a, int type_b, cpSpace *space, Player *pl);

void setupCollisions(cpSpace *space, Player *pl);
void explosionCheckRender(SDL_Renderer *);
void explosionPrepare(SDL_Renderer *r);

}

#endif // OPS_H_INCLUDED
