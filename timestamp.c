#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define NANO_TO_MICRO 100
#define NANO_TO_MILLI 1000000
#define NANO_TO_SEC 1000000000

struct timespec timessub(const struct timespec *A, const struct timespec *B)
{
    struct timespec C;
    C.tv_sec  = A->tv_sec  - B->tv_sec;
    C.tv_nsec = A->tv_nsec - B->tv_nsec;
    if(C.tv_nsec<0){
        C.tv_sec--;
        C.tv_nsec += NANO_TO_SEC;
    }
    
    return C;
}

struct timespec timesadd(const struct timespec *A, const struct timespec *B)
{
    struct timespec C;
    C.tv_sec  = A->tv_sec  + B->tv_sec;
    C.tv_nsec = A->tv_nsec + B->tv_nsec;
    if(C.tv_nsec>=NANO_TO_SEC){
        C.tv_sec++;
        C.tv_nsec -= NANO_TO_SEC;
    }
    
    return C;
}

struct timespec timediv(const struct timespec *A, long int B)
{
    struct timespec C;
    long int div;
    long int mod;
    C.tv_sec  = A->tv_sec;
    C.tv_nsec = A->tv_nsec;
    div = C.tv_sec / B;
    mod = C.tv_sec - (div * B);
    C.tv_sec = div;
    C.tv_nsec += mod * NANO_TO_SEC;
    C.tv_nsec /= B;
    if (C.tv_nsec < 0) {
        C.tv_nsec += NANO_TO_SEC;
        C.tv_sec -= 1;
    }
    return C;
}

int timestamp_stamp(unsigned char *temp, struct timespec *ts_p, int ts_len) {
    
    clock_gettime(CLOCK_REALTIME, ts_p);
    memcpy(temp, ts_p, ts_len);
    
    return 0;
}

int clock_adjust(struct timespec *ts_p, struct timespec *back_ts_p, struct timespec *recv_ts_p) {
    struct timespec alpha = timessub(back_ts_p, ts_p);
    struct timespec beta = timessub(back_ts_p, recv_ts_p);
    struct timespec doubleoffset = timesadd(&alpha, &beta);
    struct timespec offset = timediv(&doubleoffset, 2);
    if (offset.tv_sec != 0 || offset.tv_nsec > 500 * NANO_TO_MILLI) {
        struct timespec adj_realtime;
        struct timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        adj_realtime = timesadd(&now, &offset);
        return clock_settime(CLOCK_REALTIME, &adj_realtime);
    } else {
        struct timeval offset_val = {offset.tv_sec, offset.tv_nsec / NANO_TO_MICRO};
        return adjtime(&offset_val, NULL);
    }
}
