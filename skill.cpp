#include "skill.h"
#include "texture.h"
#include "player.h"
#include <enet/enet.h>

namespace xx {
Skill::Skill(): mCd(0) {
    mTimer = 0;
}

float Skill::coolDownCheck() {
    enet_uint32 now = enet_time_get();
    if (((float)(now - mTimer) / mCd > 1.00) || mTimer == 0 ) {
        return 1.00;
    } else {
        return (float)(now - mTimer) / mCd;
    }
}
Skill::~Skill() {
}

Skillmanager::Skillmanager() {
    mSkill[1].setCd(5000);
}

Skillmanager::~Skillmanager() {
}

}
