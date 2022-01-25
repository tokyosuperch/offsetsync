/*
 *  UDP client
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <sys/select.h>
#include <sys/wait.h>

/*
 *  クライアントの接続先サーバ情報
 */
struct client_info {
    unsigned short sv_port;     /* サーバのポート番号 */
    char *sv_ipaddr;            /* サーバのIPアドレス */
    char *msg;                  /* 送信メッセージ */

    int sd;                     /* ソケットディスクリプタ */
    struct sockaddr_in sv_addr; /* サーバのアドレス構造体 */
};
typedef struct client_info cl_info_t;

struct timespec ts;
struct timespec receipt_ts;
struct timespec back_ts;
struct timespec recv_ts;

extern int timestamp_stamp(unsigned char *temp, struct timespec *ts_p, int ts_len);
extern int clock_adjust(struct timespec *ts_p, struct timespec *back_ts_p, struct timespec *recv_ts_p);
extern int gpioin();

/*!
 * @brief      応答メッセージを受信する
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
udp_receive_msg(cl_info_t *info, char *errmsg)
{
    struct sockaddr_in recv_addr = {0}; /* 受信アドレス */
    int recv_addrlen = 0;
    int recv_msglen = 0;
    unsigned char buff[BUFSIZ];
    int ts_len = sizeof(struct timespec);
    fd_set readfds;
    struct timeval timeout;
    int ret_select;
    int rc = 0;
    
    //タイムアウト時間設定
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    /* 読み込みFD集合を空にする */
    FD_ZERO(&readfds);

    /* 読み込みFD集合にinfo->sdを追加 */
    FD_SET(info->sd, &readfds);
    
    /* selectでreadfdsのどれかが読み込み可能になるまでorタイムアウトまで待ち */
    ret_select = select(info->sd + 1, &readfds, NULL, NULL, &timeout);

    /* 戻り値をチェック */
    if (ret_select == -1) {
        /* select関数がエラー */
        fprintf(stderr, "Fatal: select error\n");
        return -1;
    }

    if (ret_select == 0) {
        /* 読み込み可能になったFDの数が0なのでタイムアウトと判断 */
        fprintf(stderr, "Receive Timeout\n");
        return -1;
    }
    /* 応答メッセージを受信する */
    recv_addrlen = sizeof(recv_addr);
    recv_msglen = recvfrom(info->sd, buff, BUFSIZ, 0, 
                           (struct sockaddr *)&recv_addr, &recv_addrlen);
    clock_gettime(CLOCK_REALTIME, &recv_ts);
    if(recv_msglen < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    /* 受信アドレスを確認する */
    if(info->sv_addr.sin_addr.s_addr != recv_addr.sin_addr.s_addr){
        sprintf(errmsg, "Error: received a packet from unknown source.");
        return(-1);
    }

    /* 応答メッセージを出力する */
    // buff[recv_msglen] = '\0';   /* null-terminate */
    memcpy(&receipt_ts, buff, ts_len);
    memcpy(&back_ts, buff+ts_len, ts_len);
    
    // 出力
    if (ts.tv_sec == receipt_ts.tv_sec && ts.tv_nsec == receipt_ts.tv_nsec) {
        rc = clock_adjust(&ts, &back_ts, &recv_ts);
    }

    return(0);
}

/*!
 * @brief      UDP接続してメッセージを送る
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
udp_send_msg(cl_info_t *info, char *errmsg)
{
    int rc = 0;
    unsigned char *temp;
    int ts_len = sizeof(struct timespec);
    int msg_len = ts_len;
    temp = (unsigned char *)malloc(ts_len);
    timestamp_stamp(temp, &ts, ts_len);
    /* メッセージの送信 */
    rc = sendto(info->sd,
                temp,
                msg_len,
                0,
                (struct sockaddr *)&(info->sv_addr),
                sizeof(info->sv_addr)
               );
    if(rc != msg_len){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    return(0);
}

/*!
 * @brief      エコークライアントを実行する
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
udp_echo_client(cl_info_t *info, char *errmsg)
{
    int rc = 0;

    /* メッセージを送信する */
    rc = udp_send_msg(info, errmsg);
    if(rc != 0) return(-1);

    /* 応答メッセージを受信する */
    rc = udp_receive_msg(info, errmsg);
    if(rc != 0) return(-1);

    return(0);
}

/*!
 * @brief      ソケットの初期化
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
socket_initialize(cl_info_t *info, char *errmsg)
{
    /* ソケットの生成 : UDPを指定する */
    info->sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(info->sd < 0){
        sprintf(errmsg, "(line:%d) %s", __LINE__, strerror(errno));
        return(-1);
    }

    /* サーバのアドレス構造体を作成する */
    info->sv_addr.sin_family = AF_INET;
    info->sv_addr.sin_addr.s_addr = inet_addr(info->sv_ipaddr);
    info->sv_addr.sin_port = htons(info->sv_port);

    return(0);
}

/*!
 * @brief      ソケットの終期化
 * @param[in]  info   クライアント接続情報
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static void
socket_finalize(cl_info_t *info)
{
    /* ソケット破棄 */
    if(info->sd != 0) close(info->sd);

    return;
}

/*!
 * @brief      UDPクライアント実行
 * @param[in]  info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
udp_client(cl_info_t *info, char *errmsg)
{
    int rc = 0;

    /* ソケットの初期化 */
    rc = socket_initialize(info, errmsg);
    if(rc != 0) return(-1);
    while(1) {
        pid_t pid = fork();
	    if (pid < 0) {
		    perror("fork");
	    } else if (pid == 0) {
		    // 子プロセスで別プログラムを実行
		    execlp("ntpdate", "ntpdate", "-p", "1", "-b", "supachan.sfc.wide.ad.jp", NULL);
		    perror("ntpdate");
	    }

	    // 親プロセス
	    int status;
	    pid_t r = waitpid(pid, &status, 0); //子プロセスの終了待ち
	    if (r < 0) {
		    perror("waitpid");
	    }
        /* メッセージの送受信を行う */
        rc = udp_echo_client(info, errmsg);
        if (rc == 0) {
	        sleep(1);
        };
    }

    /* ソケットの終期化 */
    socket_finalize(info);

    return(rc);
}

/*!
 * @brief      初期化処理。IPアドレスとポート番号を設定する。
 * @param[in]  argc   コマンドライン引数の数
 * @param[in]  argv   コマンドライン引数
 * @param[out] info   クライアント接続情報
 * @param[out] errmsg エラーメッセージ格納先
 * @return     成功ならば0、失敗ならば-1を返す。
 */
static int
initialize(int argc, char *argv[], cl_info_t *info, char *errmsg)
{
    /* if(argc != 4){
        sprintf(errmsg, "Usage: %s <ip-addr> <port> <msg>", argv[0]);
        return(-1);
    } */

    memset(info, 0, sizeof(cl_info_t));
    info->sv_ipaddr = "203.178.143.7";
    info->sv_port   = 2964;

    return(0);
}

/*!
 * @brief   main routine
 * @return  成功ならば0、失敗ならば-1を返す。
 */
int
main(int argc, char *argv[])
{
    gpioin();
    int rc = 0;
    cl_info_t info = {0};
    char errmsg[BUFSIZ];

    rc = initialize(argc, argv, &info, errmsg);
    if(rc != 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return(-1);
    }

    rc = udp_client(&info, errmsg);
    if(rc != 0){
        fprintf(stderr, "Error: %s\n", errmsg);
        return(-1);
    }

    return(0);
}
