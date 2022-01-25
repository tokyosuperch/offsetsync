#include <time.h>
#include <stdlib.h>
#include <string.h>

int timestamp_stamp(unsigned char *temp, struct timespec *tp, int ts_len) {
    
    clock_gettime(CLOCK_REALTIME, tp);
    memcpy(temp, tp, ts_len);
    
    return 0;
}
