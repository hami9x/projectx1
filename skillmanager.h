#ifndef SKILLMANAGER_H_INCLUDED
#define SKILLMANAGER_H_INCLUDED
#include <enet/enet.h>
#include "skill.h"
namespace xx {
class Skillmanager
{
    public:
        Skillmanager();

        ~Skillmanager();

        float cdCheck(int i) { return skill[i].coolDownCheck();  }

        void setTime(enet_uint32 time, int i) { skill[i].setTime(time); }
    protected:
    private:
        Skill skill[4];

};
}
#endif
