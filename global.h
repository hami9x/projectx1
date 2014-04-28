#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#include <cmath>

//Screen dimension constants
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 704;

//Collision Type
const int PLANE_TYPE = 1;
const int CLOUD_TYPE = 2;
const int BULLET_TYPE =3;

inline double rad2deg(double radians) {
    return radians*(180.0/M_PI);
}

inline double deg2rad(double degs) {
    return degs*M_PI/180;
}

#endif // DEFS_H_INCLUDED
