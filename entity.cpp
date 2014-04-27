#include <string>
#include <tmxparser.h>
#include <sdl.h>
#include <cmath>
#include <malloc.h>
#include <cassert>
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

Sprite::Sprite(Texture * image, SDL_Rect area, int width, int height) {
    mImage = image;
    mArea = area;
    tw = width;
    th = height;
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

cpVect tilePos(TmxMap *m, int k, int tileHeight) {
    int row = k/m->width;
    int col = k%m->width;
    //Tmx saves the tile as the bottom left one, so we have to shift it up
    return cpVect{col*m->tileWidth, row*m->tileHeight - tileHeight + m->tileHeight};
}

EntityCollection Entity::fromTmxGetAll(string ogName, string tilesetName, TmxMap *m, int tileid, Texture *image, cpSpace *space) {
    EntityCollection entities;
    vector<cpBody*> bodies;
    vector<cpVect> tileXYs;

    TmxObjectCollection_t oc;

    //Time to get the image from the tile
    Sprite sprite;
    TmxTilesetCollection_t tilesets = m->tilesetCollection;
    bool foundTile = false;
    int gid = 0;
    TmxTileset tileset;

    for (size_t i=0; i<tilesets.size(); i++) {
        if (tilesets[i].name == tilesetName) {
            TmxTileset ts = tilesets[i];
            gid = ts.firstgid+tileid;
            int tw = ts.tileWidth;
            int th = ts.tileHeight;
            int ncols = ts.image.width/tw;
            SDL_Rect area = {(tileid%ncols)*tw, (int(ceil((double)(tileid+1)/ncols))-1)*th, tw, th};

            sprite = Sprite(image, area, tw, th);
            foundTile = true;
            tileset = ts;
            break;
        }
    }

    assert(foundTile);

    //find the object group
     //Find obj, create shapes and add to bodies
    bool foundOg = false;
    TmxObjectGroupCollection_t objGroups = m->objectGroupCollection;
    for (size_t i=0; i<objGroups.size(); i++) {
        if (objGroups[i].name == ogName) {
            oc = objGroups[i].objects;
            foundOg = true;
            break;
        }
    }
    assert(foundOg);

    //find the layer
    TmxLayer layer;
    bool foundLayer = false;
    for (int i=0; i < m->layerCollection.size(); i++) {
        for (int j=0; j< m->layerCollection.size(); j++) {
            TmxLayer la = m->layerCollection[j];
            if (la.name == ogName) {
                layer = la;
                foundLayer = true;
                break;
            }
        }
    }
    assert(foundLayer);

    //find the bounding leftop point of the group of shapes
    cpVect topleft = {9999999, 9999999};
    for (size_t i=0; i<oc.size(); i++) {
        if (oc[i].x < topleft.x) {
            topleft.x = oc[i].x;
        }
        if (oc[i].y < topleft.y) {
            topleft.y = oc[i].y;
        }
    }
    cpVect objPos = topleft;

    //find the tile that goes with the object
    int xT=0, yT=0;
    for (int k=0; k<layer.tiles.size(); k++) {
        TmxLayerTile tile = layer.tiles[k];
        if (tile.gid == gid) {
            cpVect p = tilePos(m, k, tileset.tileHeight);
            tileXYs.push_back(p);
            if (p.x<=objPos.x && p.y<=objPos.y && p.x>=xT && p.y>=yT) {
                xT=p.x;
                yT=p.y;
            }
            bodies.push_back(cpBodyNew(5.0f, 2));
        }
    }

    //Now create the actual entities
    cpVect tileObjOffset = cpvsub(objPos, cpv(xT, yT));

    //Create shapes and add the right number of bodies
    for (size_t j=0; j<oc.size(); j++) {
        TmxObject obj = oc[j];

        cpVect *verts = new cpVect[obj.shapePoints.size()];
        for (size_t k=0; k<obj.shapePoints.size(); k++) {
            verts[k].x = obj.shapePoints[k].first;
            verts[k].y = obj.shapePoints[k].second;
        }

        cpVect objp = cpvsub(cpv(obj.x, obj.y), cpv(xT, yT));

        for (int i=0; i<bodies.size(); ++i) {
            cpVect tp = tileXYs[i];
            cpTransform trans = cpTransformTranslate(objp);
            cpShape *shape = NULL;
            if (obj.shapeType == TmxShapeType::kPolygon || obj.shapeType == TmxShapeType::kPolyline) {
                 shape = cpPolyShapeNew(bodies[i], obj.shapePoints.size(), verts, trans, (obj.width+obj.height)/2);
            }
            assert(shape != NULL);
            shape->body = bodies[i];
            cpSpaceAddShape(space, shape);
        }
        delete[] verts;
    }

    //All info gathered, now create the real things
    for (int i=0; i<tileXYs.size(); ++i) {
        cpBody *body = bodies[i];
        cpVect pos = tileXYs[i];
        cpBodySetPosition(body, cpvsub(pos, cpBodyGetCenterOfGravity(body)));
        Entity * e = new Entity(sprite, body);
        e->setType(ogName);
        e->setXY(cpBodyGetPosition(body).x, cpBodyGetPosition(body).y);
        entities.push_back(e);
    }

    for (int i=0; i<bodies.size(); ++i) {
        cpBody* body = bodies[i];
        printf("poss: %f, %f | %f, %f\n", cpBodyGetPosition(body).x, cpBodyGetPosition(body).y, cpBodyGetCenterOfGravity(body).x, cpBodyGetCenterOfGravity(body).y);
    }


    return entities;
}

void Entity::render(SDL_Renderer * r) {
    cpVect pos = cpBodyGetPosition(mBody);
    SDL_Point sdlCent = {(int)pos.x, (int)pos.y};
    mSprite.render(r, (int)pos.x , (int)pos.y, (double)cpBodyGetAngle(mBody), &sdlCent);
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
