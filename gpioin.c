#include <pigpio.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#define LOW 0
#define HIGH 1
#define IN_PORT 20
#define OUT_PORT 21

struct timespec in_ts;
struct timespec supachan_time;
extern struct timespec fixed_offset;
struct timespec timesadd(const struct timespec *A, const struct timespec *B);

void gettime_in() {
	clock_gettime(CLOCK_REALTIME, &in_ts);  //時刻の取得
	supachan_time = timesadd(&in_ts, &fixed_offset);
	printf("%ld.%09ld,%ld.%09ld\n", in_ts.tv_sec, in_ts.tv_nsec, supachan_time.tv_sec, supachan_time.tv_nsec);
}

int gpioin() {
	int ret;
	if (gpioInitialise() < 0) {
		perror("Failed to Initialize");
		return 0;
	}
	if (gpioSetMode(IN_PORT, PI_INPUT) != 0) {
		perror("Failed to Set INPUT mode");
		return 1;
	}
	ret = gpioSetISRFunc(IN_PORT, FALLING_EDGE, 0, gettime_in);
	if(ret < 0) printf("error");
	return 0;
}

void terminate() {
	gpioTerminate();
}
	
