#include "utils.h"

namespace xx {
    namespace utils {
        uint32 now() {
            return enet_time_get();
        }

        Timer::Timer(long interval) {
            mInterval = interval;
            reset();
        }

        uint32 Timer::elapsed() {
            return now()-mStart;
        }

        void Timer::reset() {
            mStart = now();
        }

        bool Timer::exceededReset() {
            if (elapsed() >= mInterval) {
                reset();
                return true;
            }
            return false;
        }
    }
}
