#include "skill.h"
#include "texture.h"
#include "player.h"
#include <enet/enet.h>

namespace xx {
Skill::Skill(){
    lastUsedPush = 0;
}
float Skill::coolDownCheck(int skillNum){
    enet_uint32 now = enet_time_get();
    enet_uint32 timer;
    switch(skillNum){
        case 1:
            timer = lastUsedPush;
            if (((float)(now-timer)/5000 > 1.00) || timer == 0 ) return 1.00; else return (float)(now-timer)/5000;
        }
}


Skill::~Skill()
{
    //dtor
}
}
