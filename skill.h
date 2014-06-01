#ifndef SKILL_H_INCLUDED
#define SKILL_H_INCLUDED
#include <enet/enet.h>

namespace xx {
class Skill
{
    public:
        Skill();

        ~Skill();

        //Checking cooldown
        float coolDownCheck();

        void setTime(enet_uint32 time) { timer = time; }

        enet_uint32 cd;

        enet_uint32 timer;
    protected:
    private:
};
class Skillmanager
{
    public:
        Skillmanager();

        ~Skillmanager();

        float cdCheck(int i) { return skill[i].coolDownCheck(); }

        void resetCd(int i) { skill[i].timer = enet_time_get(); }
    protected:
    private:
        Skill skill[4];

};
}


#endif // SKILL_H
