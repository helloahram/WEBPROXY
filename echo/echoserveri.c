#include "csapp.h"
#include "echo.c"

int main(int argc, char **argv)
{
    int listenfd; // 서버 소켓의 파일 디스크립터, 클라이언트 연결 요청 시 사용 -> 듣기 소켓
    int connfd;   // connfd 개별 클라이언트와의 통신을 위한 소켓 디스크립터 -> 연결 소켓
    // 듣기 소켓과 연결 소켓은 분리되어야 한다
    socklen_t clientlen; // 클라이언트 주소 구조체의 크기를 저장할 변수
    struct sockaddr_storage clientaddr;
    /* Enough space for any address 왜냐하면 IPv4, IPv6 모두 처리할 수 있도록 */
    char client_hostname[MAXLINE], client_port[MAXLINE];
    // Python 으로 치면 char test[MAXLINE] = string test
    // C언어에는 string 이 없고 C++ 에서 있음

    // 명령행 인자 체크 - 포트 번호를 인자로 받아야 한다
    // ./echoserveri 12345
    // ./echoserveri 인자 1, 12345 인자 2
    if (argc != 2) // 포트 번호가 제대로 입력되었는지 확인
    {
        fprintf(stderr, "usage: %s <port> \n", argv[0]);
        exit(0);
    }

    // Open_listenfd 함수는 지정된 포트에서 수신 대기할 서버 소켓을 열어 반환한다
    // 쉽게 말하면 서버 소켓 생성 및 특정 포트에서 연결 요청 대기
    // argv[1] 은 포트 번호, 주어진 포트 번호로 리스닝 소켓 생성
    listenfd = Open_listenfd(argv[1]);
    // 무한 루프로 클라이언트의 연결 요청을 처리
    while (1)
    {
        clientlen = sizeof(struct sockaddr_storage); // 구조체의 크기 저장
        /* Accept 함수는 새로운 소켓 디스크립터 connfd 를 반환하며
        connfd 는 해당 클라이언트와 통신할 때 사용한다
        쉽게 말해서 Accept 함수로 클라이언트의 연결 요청을 수락 */
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        // 연결된 클라이언트의 정보 (호스트명과 포트)를 가져옴
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        // 연결된 클라이언트의 정보를 출력
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        echo(connfd);  // 데이터를 송수신을 담당하는 echo 함수 호출, 클라이언트와 통신
        Close(connfd); // 클라이언트와의 통신이 끝나면 연결 소켓 닫기
    }
    exit(0);
}