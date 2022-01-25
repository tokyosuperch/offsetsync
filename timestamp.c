#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>

#define NANO_TO_MICRO 1000
#define NANO_TO_MILLI 1000000
#define NANO_TO_SEC 1000000000

const struct timespec fixdata[24] = {
    {0, 5330048}, // 0
    {0, 5147584}, // 1
    {0, 5852416}, // 2
    {0, 5122496}, // 3
    {0, 4972608}, // 4
    {0, 5150016}, // 5
    {0, 4840000}, // 6
    {0, 5289920}, // 7
    {0, 4782400}, // 8
    {0, 4622528}, // 9
    {0, 5080000}, // 10
    {0, 5022528}, // 11
    {0, 1687488}, // 12
    {0, -27072448}, // 13
    {0, 5035008}, // 14
    {0, 4457472}, // 15
    {0, 4704960}, // 16
    {0, 8427456}, // 17
    {0, 7784896}, // 18
    {0, 8484928}, // 19
    {0, 8817472}, // 20
    {0, 9092480}, // 21
    {0, 7934976}, // 22
    {0, 8610048} // 23
};

const struct timespec trim_min[24] = {
    {0, 31288704},
    {0, 27407616},
    {0, 11930048},
    {0, 26751200},
    {0, 27813792},
    {0, 38031104},
    {0, 27003744},
    {0, 32931840},
    {0, 25280960},
    {0, 26268672},
    {0, 29937696},
    {0, 35817792},
    {0, -936224},
    {0, 78681248},
    {0, 31956800},
    {0, 21830016},
    {0, 21244640},
    {0, 55244960},
    {0, 48949280},
    {0, 55461248},
    {0, 56233728},
    {0, 55068960},
    {0, 45200128},
    {0, 53687392}
};
    
const struct timespec trim_max[24] = {
    {0, 54038912},
    {0, 60527360},
    {0, 88670144},
    {0, 61241312},
    {0, 59883680},
    {0, 42621184},
    {0, 60933728},
    {0, 50080768},
    {0, 63411648},
    {0, 61798912},
    {0, 55337248},
    {0, 44077376},
    {0, 115873760},
    {0, 145271200},
    {0, 52685632},
    {0, 69909888},
    {0, 70705120},
    {0, 64965024},
    {0, 72958240},
    {0, 64571264},
    {0, 63864064},
    {0, 65518624},
    {0, 79899904},
    {0, 67447648}
};

struct timespec timessub(const struct timespec *A, const struct timespec *B)
{
    struct timespec C;
    C.tv_sec  = A->tv_sec  - B->tv_sec;
    C.tv_nsec = A->tv_nsec - B->tv_nsec;
    if (C.tv_sec <= 0 && C.tv_nsec < 0) {
        if(C.tv_nsec>0){
            C.tv_sec++;
            C.tv_nsec -= NANO_TO_SEC;
        }
        if(C.tv_nsec<=NANO_TO_SEC * -1){
            C.tv_sec--;
            C.tv_nsec += NANO_TO_SEC;
        }
    } else {
        if(C.tv_nsec<0){
            C.tv_sec--;
            C.tv_nsec += NANO_TO_SEC;
        }
        if(C.tv_nsec>=NANO_TO_SEC){
            C.tv_sec++;
            C.tv_nsec -= NANO_TO_SEC;
        }
    }
    return C;
}

struct timespec timesadd(const struct timespec *A, const struct timespec *B)
{
    struct timespec C;
    C.tv_sec  = A->tv_sec  + B->tv_sec;
    C.tv_nsec = A->tv_nsec + B->tv_nsec;
    if (C.tv_sec <= 0 && C.tv_nsec < 0) {
        if(C.tv_nsec>0){
            C.tv_sec++;
            C.tv_nsec -= NANO_TO_SEC;
        }
        if(C.tv_nsec<=NANO_TO_SEC * -1){
            C.tv_sec--;
            C.tv_nsec += NANO_TO_SEC;
        }
    } else {
        if(C.tv_nsec<0){
            C.tv_sec--;
            C.tv_nsec += NANO_TO_SEC;
        }
        if(C.tv_nsec>=NANO_TO_SEC){
            C.tv_sec++;
            C.tv_nsec -= NANO_TO_SEC;
        }
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
    struct timespec rtt = timessub(&alpha, &beta);
    struct timespec doubleoffset = timesadd(&alpha, &beta);
    struct timespec offset = timediv(&doubleoffset, 2);
    struct timespec now;
    struct timespec adj_realtime;
    clock_gettime(CLOCK_REALTIME, &now);
    adj_realtime = timesadd(&now, &offset);
    struct tm tm;
    localtime_r(&adj_realtime.tv_sec, &tm);
    int whathour;
    if (tm.tm_min < 30) {
        whathour = tm.tm_hour;
    } else {
        if (tm.tm_hour == 23) {
            whathour = 0;
        } else {
            whathour = tm.tm_hour + 1;
        }
    }
    // 外れ値の除外
    struct timespec rtt_min = timessub(&rtt, &trim_min[whathour]);
    if (rtt_min.tv_sec != 0 || rtt_min.tv_nsec < 0) {
        printf("min out:%ld.%09ld\n", rtt.tv_sec, rtt.tv_nsec);
        return -1;
    }
    struct timespec rtt_max = timessub(&rtt, &trim_max[whathour]);
    if (rtt_max.tv_sec != 0 || rtt_max.tv_nsec > 0) {
        printf("max out:%ld.%09ld\n", rtt.tv_sec, rtt.tv_nsec);
        return -1;
    }
    struct timespec fixed_offset = timessub(&offset, &fixdata[whathour]);
    // 先程より進んでいるので取り直し
    clock_gettime(CLOCK_REALTIME, &now);
    adj_realtime = timesadd(&now, &fixed_offset);
    return clock_settime(CLOCK_REALTIME, &adj_realtime);
    
}
