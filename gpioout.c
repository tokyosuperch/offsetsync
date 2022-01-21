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
struct timespec offset;

void gettime_out() {
	clock_gettime(CLOCK_REALTIME, &out_ts);  //時刻の取得
	localtime_r( &out_ts.tv_sec, &out_tm);
	printf("%ld.%09ld\n",ts.tv_sec, out_ts.tv_nsec);
}

int main() {
	int ret;
	if (gpioInitialise() < 0) {
		perror("Failed to Initialize");
		return 0;
	}
	if (gpioSetMode(OUT_PORT, PI_OUTPUT) != 0) {
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
