#include "csapp.h"

int main(int argc, char **argv)
{                 // argv argument vector, c 는 count
    int clientfd; // 서버와의 연결을 나타내는 소켓 파일 디스크립터
    /* argv 배열은 프로그램 실행 시 제공되는 명령행 인자들을 저장하며
    host 와 port 는 포인터를 이용하여 별도 메모리 할당 없이 사용할 수 있다 */
    char *host, *port, buf[MAXLINE];
    rio_t rio; // 읽기/ 입력을 위한 RIO (Buffered I/O) 구조체

    if (argc != 3)
    { // argc != 3 인 경우, 사용법 메시지 출력하고 종료
        /*
        * argc 는 프로그램이 실행될 때 전달된 인수의 수
        argv[0] 실행할 프로그램의 이름 (자동으로 포함됨)
        argv[1] 연결할 서버의 호스트 이름 (첫번째 사용자 인수)
        argv[2] 연결할 서버의 포트 번호 (두번째 사용자 인수)
        */
        fprintf(stderr, "usage: %s <host> <port> \n", argv[0]);
        exit(0);
    }

    host = argv[1];
    port = argv[2];

    /* Open_clientfd(host, port) 로 지정된 호스트와 포트에 대해
     서버와 연결을 설정하고, 연결된 소켓 FD clientfd 를 반환한다 */
    clientfd = Open_clientfd(host, port);
    /* Rio 버퍼에 소켓 FD 를 초기화하여 읽기 작업 준비 */
    Rio_readinitb(&rio, clientfd);

    // 표준 입력 stdin 으로 입력을 받아 buf 에 저장
    while (Fgets(buf, MAXLINE, stdin) != NULL)
    {
        // 입력이 있을 때마다 서버에 데이터를 보낸다
        Rio_writen(clientfd, buf, strlen(buf));
        // 서버의 응답을 읽는다
        Rio_readlineb(&rio, buf, MAXLINE);
        // 표준 출력 stdout 에 출력한다
        Fputs(buf, stdout);
    }

    // 서버와의 연결을 닫고 프로그램 종료
    Close(clientfd);
    exit(0);
}