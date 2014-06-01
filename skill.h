#ifndef SKILL_H_INCLUDED
#define SKILL_H_INCLUDED
#include <enet/enet.h>

namespace xx {
class Skill
{
        enet_uint32 mCd;

        enet_uint32 mTimer;

    public:
        Skill();

        ~Skill();

        //Checking cooldown
        float coolDownCheck();

        void setTime(enet_uint32 time) { mTimer = time; }

        void setCd(enet_uint32 cooldown) { mCd = cooldown; }
};
class Skillmanager
{
         Skill mSkill[4];

    public:
        Skillmanager();

        ~Skillmanager();

        float cdCheck(int i) { return mSkill[i].coolDownCheck(); }

        void resetCd(int i) { mSkill[i].setTime(enet_time_get()); }

};
}


#endif // SKILL_H
