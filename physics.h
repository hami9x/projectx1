#ifndef PHYSICS_H_INCLUDED
#define PHYSICS_H_INCLUDED
#include <chipmunk_private.h>
#include "utils.h"

using namespace std;

namespace xx {

class Physics {
private:
    cpSpace *mSpace;
    uint32 mTimeStep;
    uint32 mAccumulator;

public:
    Physics(uint32);
    void free();

    void addCollisionHandler(cpCollisionType typeA, cpCollisionType typeB, cpCollisionBeginFunc beginFunc, cpCollisionSeparateFunc separateFunc);
    void setupCollisions();
    void step(uint32 frameTime);
    void spaceStep(uint32 rawStep);
    cpSpace* space() { return mSpace; };
};

//ns end
}

#endif // PHYSICS_H_INCLUDED
