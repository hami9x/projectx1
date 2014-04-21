#ifndef ENTITY_H_INCLUDED
#define ENTITY_H_INCLUDED
#include <chipmunk.h>
#include "texture.h"
#include "tmxparser.h"

using namespace std;
using namespace tmxparser;

namespace xx {

class Sprite {
    Texture *mImage;
    SDL_Rect mArea;

public:
    Sprite();
    Sprite(Texture *image, SDL_Rect area);
    ~Sprite();

    Texture * image() { return mImage; }

    void render(SDL_Renderer *renderer, int x, int y, double angle, SDL_Point* center, SDL_RendererFlip flip=SDL_FLIP_NONE);
};

class Entity;
typedef vector<Entity*> EntityCollection;

class Entity {
    Sprite mSprite;
    cpBody *mBody;
    string mName;
    string mType;
    int mX;
    int mY;

public:
    Entity(Sprite, cpBody*);
    ~Entity();
    inline void setName(string name) { mName = name; }
    inline void setType(string type) { mType = type; }
    inline string name() { return mName; }
    inline string type() { return mType; }
    inline void setX(int x) { mX = x; }
    inline void setY(int y) { mY = y; }
    inline int x() { return mX; }
    inline int y() { return mY; }
    inline void setXY(int x, int y) { mX = x; mY = y; }
    inline cpBody *body() { return mBody; }
    inline Sprite sprite() { return mSprite; }
    static EntityCollection fromTmxGetAll(string ogName, string tilesetName, TmxMap *m, int tileid, Texture *image, bool forEachObject);
    static void renderAll(EntityCollection,  SDL_Renderer *);
    static void addAll(EntityCollection, cpSpace *);

    void render(SDL_Renderer *);
};

}
#endif // ENTITY_H_INCLUDED
