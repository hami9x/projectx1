#include "skill.h"
#include "texture.h"
#include "player.h"
#include <enet/enet.h>

namespace xx {
Skill::Skill(){
    timer = 0;
}
float Skill::coolDownCheck(){
    enet_uint32 now = enet_time_get();
    if (((float)(now-timer)/cd > 1.00) || timer == 0 ) {
        return 1.00;
    } else {
        return (float)(now-timer)/cd;
    }
}
Skill::~Skill()
{
}
Skillmanager::Skillmanager(){
    skill[1].cd = 5000;
}
Skillmanager::~Skillmanager(){
}
}
