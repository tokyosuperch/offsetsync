#include <pigpio.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#define LOW 0
#define HIGH 1
#define IN_PORT 20
#define OUT_PORT 21

struct timespec out_ts;
struct tm out_tm;
struct timespec in_ts;
struct tm in_tm;
struct timespec offset;

struct timespec *timessub(struct timespec *A, struct timespec *B, struct timespec *C)
{
    C->tv_sec  = A->tv_sec  - B->tv_sec;
    C->tv_nsec = A->tv_nsec - B->tv_nsec;
    if(C->tv_nsec<0){
        C->tv_sec--;
        C->tv_nsec += 1000000000;
    }
    
    return C;
}

void gettime_in() {
	clock_gettime(CLOCK_REALTIME, &in_ts);  //時刻の取得
	localtime_r( &out_ts.tv_sec, &out_tm);
	localtime_r( &in_ts.tv_sec, &in_tm);
	printf("out:%d/%02d/%02d %02d:%02d:%02d.%09ld\n",
		out_tm.tm_year+1900,
		out_tm.tm_mon+1,
		out_tm.tm_mday,
		out_tm.tm_hour,
		out_tm.tm_min,
		out_tm.tm_sec,
		out_ts.tv_nsec);
	printf("in :%d/%02d/%02d %02d:%02d:%02d.%09ld\n",
		in_tm.tm_year+1900,
		in_tm.tm_mon+1,
		in_tm.tm_mday,
		in_tm.tm_hour,
		in_tm.tm_min,
		in_tm.tm_sec,
		in_ts.tv_nsec);
	timessub(&in_ts,&out_ts,&offset);
	printf("%ld.%09ld\n\n", offset.tv_sec, offset.tv_nsec);
}

void gettime_out() {
	clock_gettime(CLOCK_REALTIME, &out_ts);  //時刻の取得
}

int main() {
	int ret;
	if (gpioInitialise() < 0) {
		perror("Failed to Initialize");
		return 0;
	}
	if (gpioSetMode(IN_PORT, PI_INPUT) != 0) {
		perror("Failed to Set INPUT mode");
		return 1;
	}
	ret = gpioSetISRFunc(IN_PORT, FALLING_EDGE, 5000, gettime_in);
	if(ret < 0) printf("error");
	
	if (gpioSetMode(21, PI_OUTPUT) != 0) {
		perror("Failed to Set OUTPUT mode");
		return 1;
	}
	while(1) {
		gpioWrite(OUT_PORT, HIGH);
		usleep(1000000 - 1000);
		gpioWrite(OUT_PORT, LOW);
		gettime_out();
		usleep(1000);
	}
	gpioTerminate();
}
