#include "physics.h"
#include "player.h"

using namespace std;

namespace xx {

void planeCloud(cpShape * planeShape, int num) {
    Player *p1 = (Player*)cpBodyGetUserData(cpShapeGetBody(planeShape));
    p1->setInCloud(num);
}

static cpBool planeCloudBeginFunc(cpArbiter *arb, cpSpace *space, void*) {
    cpShape *a,*b;
    cpArbiterGetShapes(arb, &a,&b);
    planeCloud(a, 1);
}

static void planeCloudSepFunc(cpArbiter *arb, cpSpace *space, void*) {
    cpShape *a,*b;
    cpArbiterGetShapes(arb, &a,&b);
    planeCloud(a, -3);
}

static cpBool bulletCloudBeginFunc(cpArbiter *arb, cpSpace *space, void*) {
    cpShape *a,*b;
    cpArbiterGetShapes(arb, &a,&b);
}

//Post-step: Ammo free
static void ammoFree( cpSpace *space, cpShape *shape, void *unused) {
    cpBody *body = cpShapeGetBody(shape);
    Bullet *ammo = (Bullet*)cpBodyGetUserData(body);
    //cpVect pos = cpBodyGetPosition(Body);
    //printf("body->p: %f %f \n", Body->p.x, Body->p.y);
    //printf("Position: %f %f \n",pos.x,pos.y);
    //ammo->explosion(Body->p.x - 59, Body->p.y - 59);
//    for(int i=0; i < EXPLOSION_MAXIMUM_SPRITES; i++)
//        if( !explosionCheck[i] )
//            explosionSet(i, Body->p.x - 59, Body->p.y - 59);
    ammo->free();
}

static cpBool bulletPlaneBeginFunc(cpArbiter *arb, cpSpace *space, void*) {
    cpShape *a,*b;
    cpArbiterGetShapes(arb, &a,&b);

    cpBool check = cpShapeGetSensor(a);
    if(!check){
        printf("Hit plane\n");

        cpSpaceAddPostStepCallback( space, (cpPostStepFunc)ammoFree, a, NULL);
    }
}

Physics::Physics(uint32 timeStep) {
    // Create an empty space.
    mSpace = cpSpaceNew();
    cpSpaceSetGravity(mSpace, cpv(0, 0));
    cpSpaceSetDamping(mSpace, 0.5);
    mTimeStep = timeStep;
    mAccumulator = 0;
}

void Physics::free() {
    cpSpaceFree(mSpace);
}

void Physics::step(uint32 frameTime) {
    mAccumulator += frameTime;
    while ( mAccumulator >= mTimeStep )
    {
        spaceStep(mTimeStep);
        mAccumulator -= mTimeStep;
    }
}

void Physics::spaceStep(uint32 rawStep) {
    cpSpaceStep(mSpace, (double)rawStep/1000.);
}

void Physics::addCollisionHandler(cpCollisionType typeA, cpCollisionType typeB, cpCollisionBeginFunc beginFunc, cpCollisionSeparateFunc separateFunc) {
    cpCollisionHandler * handler = cpSpaceAddCollisionHandler(mSpace, typeA, typeB);
    if (beginFunc != NULL) {
        handler->beginFunc = beginFunc;
    }
    if (separateFunc != NULL) {
        handler->separateFunc = separateFunc;
    }
}

void Physics::setupCollisions() {
    addCollisionHandler(PLANE_TYPE, CLOUD_TYPE, planeCloudBeginFunc, planeCloudSepFunc);
    addCollisionHandler(BULLET_TYPE, CLOUD_TYPE, bulletCloudBeginFunc, NULL);
    addCollisionHandler(BULLET_TYPE, PLANE_TYPE, bulletPlaneBeginFunc, NULL);
}

//ns end
}
