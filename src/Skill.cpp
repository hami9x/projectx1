#include "Skill.h"
#include "texture.h"
#include "player.h"

Skill::Skill()
{
    //ctor
}
Skill::push(Player *p){
    lastUsedPush = enet_time_get();
    cpBodyApplyImpulseAtWorldPoint(p->body(),cpvmult(p->vectorForward(),60), cpv(0, 0));
}
Skill::coolDownCheck(int skillNum){
    enet_uint32 now = enet_time_get();
    enet_uint32 timer;
    switch(skillNum){
        case 1:
            timer = lastUsedPush;
            if ((now-timer)/5000 >  1) return 1; else return (now-timer)/5000;
        }
}


Skill::~Skill()
{
    //dtor
}
