#include <string>
#include <tmxparser.h>
#include <sdl.h>
#include <cmath>
#include <malloc.h>
#include "global.h"
#include "entity.h"

using namespace std;
using namespace tmxparser;

namespace xx {

Sprite::Sprite() {
    SDL_Rect r = {0, 0, 0, 0};
    mImage = NULL;
    mArea = r;
}

Sprite::Sprite(Texture * image, SDL_Rect area) {
    mImage = image;
    mArea = area;
}

Sprite::~Sprite() {}

void Sprite::render(SDL_Renderer *renderer, int x, int y, double angle, SDL_Point* center, SDL_RendererFlip flip) {
    if (mImage == NULL) {
        return;
    }

    mImage->render(renderer, x, y, &mArea, angle, NULL, flip);
}

Entity::Entity(Sprite sprite, cpBody* body) {
    mSprite = sprite;
    mBody = body;
}

Entity::~Entity() {
    cpBodyFree(mBody);
}

cpVect toScreenCoord(TmxMap *m, cpVect v) {
    int mw = m->width*m->tileWidth;
    int mh = m->height*m->tileHeight;
    cpVect rv;
    rv.x = int((double)v.x / mw * SCREEN_WIDTH);
    rv.y = int((double)v.y / mh * SCREEN_HEIGHT);
    return rv;
}

EntityCollection Entity::fromTmxGetAll(string ogName, string tilesetName, TmxMap *m, int tileid, Texture *image, bool forEachObject, cpSpace *space) {
    int tw,th;
    Sprite sprite;
    TmxTilesetCollection_t tilesets = m->tilesetCollection;
    if (!tilesetName.empty()) {
        for (size_t i=0; i<tilesets.size(); i++) {
            if (tilesets[i].name == tilesetName) {
                TmxTileset ts = tilesets[i];
                tw = ts.tileWidth;
                th = ts.tileHeight;
                int ncols = ts.image.width/tw;
                SDL_Rect area = {(tileid%ncols)*tw, (int(ceil((double)(tileid+1)/ncols))-1)*th, tw, th};

                sprite = Sprite(image, area);
                break;
            }
        }
        if (sprite.image() == NULL) {
            printf("Cannot find tileset %s in map.\n", tilesetName.c_str());
        }
    }

    EntityCollection entities;

    TmxObjectGroupCollection_t objGroups = m->objectGroupCollection;
    for (size_t i=0; i<objGroups.size(); i++) {
        if (objGroups[i].name == ogName) {
            TmxObjectCollection_t objs = objGroups[i].objects;
            cpBody *body = NULL;
            if (!forEachObject) {
                body = cpBodyNew(10.0f, INFINITY);
            }
            for (size_t j=0; j<objs.size(); j++) {
                TmxObject obj = objs[j];
                cpBody *targetBody = body;
                if (forEachObject) {
                    targetBody = cpBodyNew(10.0f, INFINITY);
                }

                cpVect *verts = new cpVect[obj.shapePoints.size()];
                for (size_t k=0; k<obj.shapePoints.size(); k++) {
                    verts[k].x = obj.shapePoints[k].first/2;
                    verts[k].y = obj.shapePoints[k].second/2;
                }
//                for (size_t k=0; k<obj.shapePoints.size(); k++) {
//                    verts[k].x = obj.shapePoints[k].first + obj.x;
//                    verts[k].y = obj.shapePoints[k].second + obj.y;
//                }
                CP_CONVEX_HULL(obj.shapePoints.size(), verts, ncount, cverts)
                cpVect _offset = {cpFloat(obj.x), cpFloat(obj.y)};
                cpVect offset = toScreenCoord(m, _offset);
                cpBodySetPosition(targetBody, offset);
                if (obj.shapeType == TmxShapeType::kPolygon) {
                    cpShape *shape;

                    shape = cpBoxShapeNew(targetBody,tw,th,tw/2);
                    if(ogName == "planes")
                        {
                        cpShapeSetCollisionType(shape, PLANE_TYPE);
                        cpSpaceAddShape(space, shape);
                        cpShapeCacheBB(shape);
                        printf("%d\n",1);
                        };
                    if(ogName == "clouds")
                    {
                        cpShapeSetCollisionType(shape, CLOUD_TYPE);
                        cpSpaceAddShape(space, shape);
                        cpShapeCacheBB(shape);
                        printf("%d\n",2);
                    }

                    delete[] verts;
                }

                //forEachObject we create an entity, or at the end of the list we create an entity
                if (forEachObject || j==objs.size()-1) {
                    Entity * e = new Entity(sprite, targetBody);
                    e->setName(obj.name);
                    e->setType(obj.type);
                    e->setXY(obj.x, obj.y);

                    entities.push_back(e);
                }
            }
            break;
        }
    }

    return entities;
}

void Entity::render(SDL_Renderer * r) {
    cpVect pos = cpBodyGetPosition(mBody);
    SDL_Point sdlCent = {(int)pos.x, (int)pos.y};
    mSprite.render(r, (int)pos.x, (int)pos.y, (double)cpBodyGetAngle(mBody), &sdlCent);
}

void Entity::renderAll(EntityCollection ec, SDL_Renderer * r) {
    for (size_t i=0; i<ec.size(); i++) {
        ec[i]->render(r);
    }
}

void Entity::addAll(EntityCollection ec, cpSpace * space) {
    for (size_t i=0; i<ec.size(); i++) {
        cpSpaceAddBody(space, ec[i]->body());
    }
}

//End namspace
}
