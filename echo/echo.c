#include "csapp.h"
/*
echo 함수는 서버 소켓 프로그래밍에서, 클라이언트로부터 메시지를 받아
다시 클라이언트에게 그대로 돌려주는 역할을 한다
connfd 라는 연결 파일 디스크립터를 매개변수로 받아 클라이언트와의 연결을 처리
*/
void echo(int connfd)
{
    size_t n; // 읽은 바이트 수를 저장할 변수
    char buf[MAXLINE];
    // Robust I/O 구조체 rio 를 선언하여
    // 버퍼링된 I/O 연산을 위한 정보를 저장할 공간을 만든다
    rio_t rio;

    // connfd 로부터 읽기 위한 rio 구조체 초기화
    Rio_readinitb(&rio, connfd);
    // 클라이언트로부터 메시지를 읽어서 echo 한다
    // Rio_readlineb 클라이언트로부터 한 줄을 읽음
    while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
    {
        // 읽은 바이트 수 출력
        printf("server received %d bytes\n", (int)n);
        // 읽은 메시지를 클라이언트에게 다시 전송
        Rio_writen(connfd, buf, n);
    }
}