#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <errno.h>

unsigned long long int charToInt(int bytes, ...);
struct ifreq ifr;
 
int main(int argc, char** argv)
{
    int sd;
    struct sockaddr_in addr;
    struct timespec ts;
    struct timespec back_ts;
    struct timespec recv_ts;
 
    unsigned char buf[2048];
    
    // IPv4 TCP のソケットを作成する
    if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    // 送信先アドレスとポート番号を設定する
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2964);
    addr.sin_addr.s_addr = inet_addr("");
    
    // サーバ接続（TCP の場合は、接続を確立する必要がある）
    connect(sd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
 
    unsigned char *temp;
    temp = (unsigned char *)malloc(8 * sizeof(char));
    // パケットを TCP で送信
    clock_gettime(CLOCK_REALTIME, &ts);
    for (int i = 3; i >= 0; i--) temp[(3-i)] = (unsigned char)(ts.tv_sec >> (i * 8)) % 256;
    for (int i = 3; i >= 0; i--) temp[4+(3-i)] = (unsigned char)(ts.tv_nsec >> (i * 8)) % 256;

    if(send(sd, temp, 8, 0) < 0) {
        perror("send");
        return -1;
    }
  
    // パケット受信。パケットが到着するまでブロック
    if(recv(sd, buf, sizeof(buf), 0) < 0) {
        perror("recv");
        return -1;
    }
    clock_gettime(CLOCK_REALTIME, &recv_ts);
    
    back_ts.tv_sec = charToInt(4, buf[0], buf[1], buf[2], buf[3]);
    back_ts.tv_nsec = charToInt(4, buf[4], buf[5], buf[6], buf[7]);
    // 出力
    // 送信時刻
    printf("%ld.%09ld,",ts.tv_sec,ts.tv_nsec);
    // サーバー時刻
    printf("%ld.%09ld,",back_ts.tv_sec,back_ts.tv_nsec);
    // 現在
    printf("%ld.%09ld\n",recv_ts.tv_sec,recv_ts.tv_nsec);
    close(sd);
 
    return 0;
}

unsigned long long int charToInt(int bytes, ...) {
	va_list list;
	unsigned long long int temp = 0; 

	va_start(list, bytes);
	for(int i = 0; i < bytes; i++) {
		temp += va_arg(list, int);
		if (bytes - i > 1) temp = temp << 8;
	}
	va_end(list);

	return temp;
}
