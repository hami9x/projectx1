#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED
#include <enet/enet.h>

namespace xx {
    typedef enet_uint32 uint32;

    namespace utils {

        uint32 now();

        class Timer {
            //interval in ms
            uint32 mInterval;
            uint32 mStart;

        public:
            Timer(long interval = 0);
            void reset();
            uint32 elapsed();
            bool exceededReset();
        };
    }

}

#endif
