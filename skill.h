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

    protected:
    private:
        enet_uint32 timer;

};
}


#endif // SKILL_H
