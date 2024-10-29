/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);                                         // HTTP 트랜잭션을 처리하는 함수
void read_requesthdrs(rio_t *rp);                          // 요청 헤더를 읽고 무시하는 함수
int parse_uri(char *uri, char *filename, char *cgiargs);   // URI 를 분석하는 함수
void serve_static(int fd, char *filename, int filesize);   // 정적 컨텐츠를 제공하는 함수
void get_filetype(char *filename, char *filetype);         // 파일 유형을 추측하는 함수
void serve_dynamic(int fd, char *filename, char *cgiargs); // 동적 컨텐츠를 제공하는 함수
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg); // 에러 메시지를 클라이언트에 전송하는 함수

int main(int argc, char **argv)
{
  int listenfd, connfd;                  // 듣기 소켓과 연결 소켓을 저장할 변수
  char hostname[MAXLINE], port[MAXLINE]; // 클라이언트의 호스트 네임과 포트를 저장할 변수 선언
  socklen_t clientlen;                   // 클라이언트 주소의 길이를 저장할 변수 선언
  struct sockaddr_storage clientaddr;    // 클라이언트 주소 구조체 선언

  /* Check command line args */
  if (argc != 2)
  { // 인수가 2개가 아닐 경우, 사용법 출력
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]); // 지정된 포트에서 듣기 소켓 열기
  while (1)
  {
    clientlen = sizeof(clientaddr); // 클라이언트의 주소 길이 초기화
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen); // line:netp:tiny:accept 연결 수락
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0); // 클아이언트 정보 가져오기
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);  // line:netp:tiny:doit
    Close(connfd); // line:netp:tiny:close
  }
}

/*
 * doit() 한 개의 HTTP 트랜잭션을 처리한다
 */
void doit(int fd)
{
  int is_static;    // 정적 컨텐츠 여부를 나타내는 변수
  struct stat sbuf; // 파일 상태 정보를 저장할 구조체
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  /* Read Request line and headers */
  Rio_readinitb(&rio, fd);           // rio 구조체 초기화
  Rio_readlineb(&rio, buf, MAXLINE); // 요청 라인 열기
  printf("Request headers:\n");      // 요청 헤더 출력 시작
  printf("%s", buf);                 // 요청 라인 출력
  sscanf(buf, "%s %s %s", method, uri, version);

  // strcasecmp 대소문자 무시하고 비교하는 함수, 같은 값이 나오면 0
  // 첫번째가 두번째보다 작으면 음수, 크면 양수 반환
  if (strcasecmp(method, "GET"))
  { // 요청이 GET 이 아닌 경우, 클라이언트에 에러 전송
    clienterror(fd, method, "501", "Not implemented",
                "Tiny does not implement this method");
    return; // 함수 종료
  }
  read_requesthdrs(&rio); // 요청 헤더 읽기

  /* Parase URI from GET request */
  // URI 를 파싱하여 파일 이름과 CGI 인수를 얻음
  is_static = parse_uri(uri, filename, cgiargs);
  if (stat(filename, &sbuf) < 0)
  { // 파일을 가져오는데 실패할 경우 클라이언트에 404 에러 전송
    clienterror(fd, filename, "404", "Not found",
                "Tiny couldn't find this file");
    return; // 함수 종료
  }

  if (is_static) /* Serve Static Content */
  {
    // 파일이 일반 파일인지와 읽기 권한을 가지고 있는지 여부 검사
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
    { // 클라이언트에 403 에러 전송
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny couldn't read the file");
      return; // 함수 종료
    }
    serve_static(fd, filename, sbuf.st_size);
  }
  else /* Serve Dynamic Content */
  {
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden",
                  "Tiny couldn't run the CGI Program");
      return; // 함수 종료
    }
    serve_dynamic(fd, filename, cgiargs);
  }
}

/*
 * clienterror() 에러 메시지를 클라이언트에게 보낸다
 */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF]; // HTTP 응답을 저장할 버퍼 선언

  /* Build the HTTP Response Body */
  sprintf(body, "<html><title>Tiny Error</title>"); // HTML 문서 제목
  sprintf(body, "%s<body bgcolor="
                "ffffff"
                ">\r\n",
          body);                                                 // 배경색 설정
  sprintf(body, "%s%s %s\r\n", body, errnum, shortmsg);          // 에러 번호와 간단한 메시지 추가
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);        // 상세 에러 메시지 추가
  sprintf(body, "%s<hr><em>The Tiny Web Server</em>\r\n", body); // 서버 정보 추가

  /* Print the HTTP Response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);          // HTTP 상태 라인 구성
  Rio_writen(fd, buf, strlen(buf));                              // 클라이언트에 상태 라인 전송
  sprintf(buf, "Content-type: text/html\r\n");                   // 콘텐츠 타입 설정
  Rio_writen(fd, buf, strlen(buf));                              // 클라이언트에 콘텐츠 타입 전송
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body)); // 본문의 길이 설정
  Rio_writen(fd, buf, strlen(buf));                              // 클아이언트에 콘텐츠 길이 전송
  Rio_writen(fd, body, strlen(body));                            // 클라이언트에 본문 전송
}

/*
 * read_requestthdrs() 요청 함수를 읽고 무시한다
 */
void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE]; // 요청 헤더를 저장할 버퍼 선언

  Rio_readlineb(rp, buf, MAXLINE); // 요청 헤더 한 줄 읽기
  while (strcmp(buf, "\r\n"))      // 빈 줄 (헤더 끝)을 만날 때까지 반복
  {
    Rio_readlineb(rp, buf, MAXLINE); // 다음 요청 헤더 읽기
    printf("%s", buf);               // 요청 헤더 출력
  }
  return; // 함수 종료
}

/*
 * parse_uri() HTTP URI 분석
 */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
  char *ptr;

  if (!strstr(uri, "cgi-bin")) /* Static Content */
  {
    strcpy(cgiargs, "");             // CGI 인자 초기화
    strcpy(filename, ".");           // 파일 이름을 현재 디렉토리로 설정
    strcat(filename, uri);           // URI를 파일 이름에 추가
    if (uri[strlen(uri) - 1] == '/') // URI가 슬래시로 끝나는지 확인
      strcat(filename, "home.html"); // 슬래시로 끝나면 home.html 추가
    return 1;                        // 정적 콘텐츠로 처리됨
  }
  else /* Dynamic content */
  {
    ptr = index(uri, '?'); // URI 에서 '?' 문자의 위치를 찾음
    if (ptr)               // '?' 문자가 있으면
    {                      // CGI 인자에 '?' 이후의 문자열을 저장
      strcpy(cgiargs, ptr + 1);
      *ptr = '\0'; // URI 를 '?' 이전으로 잘라냄
    }
    else
      strcpy(cgiargs, ""); // CGI 인자가 없으면 빈 문자열 생성
    strcpy(filename, "."); // 파일 이름을 현재 디렉토리로 설정
    strcat(filename, uri); // URI 를 파일 이름에 추가
    return 0;              // 동적 콘텐츠로 처리됨
  }
}

/*
 * Serve_static() 정적 컨텐츠를 클라이언트에 제공한다
 */
void serve_static(int fd, char *filename, int filesize)
{
  int srcfd;                                  // 소스 파일 디스크립터
  char *srcp, filetype[MAXLINE], buf[MAXBUF]; // 포인터, 파일 타입 및 버퍼 선언

  /* Send response headers to client */
  get_filetype(filename, filetype);                          // 파일 이름으로부터 파일 타입을 결정
  sprintf(buf, "HTTP/1.0 200 OK\r\n");                       // HTTP 응답 상태 줄 설정
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);        // 서버 정보 추가
  sprintf(buf, "%sConnection: close\r\n", buf);              // 연결 종료 정보 추가
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);   // 콘텐츠 길이 설정
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype); // 콘텐츠 타입 추가
  Rio_writen(fd, buf, strlen(buf));                          // 응답 헤더를 클라이언트에 전송
  printf("Response headers:\n");
  printf("%s", buf); // 응답 헤더 출력

  /* Send response body to client */
  srcfd = Open(filename, O_RDONLY, 0);                        // 파일을 읽기 전용으로 열기
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // 파일 내용을 메모리에 저장
  Close(srcfd);                                               // 파일 디스크립터 닫기
  Rio_writen(fd, srcp, filesize);                             // 응답 본문을 클라이언트에 전송
  Munmap(srcp, filesize);                                     // 메모리 매핑 해제
}

/*
 * get_filetype() - Derive file type from filename
 */
void get_filetype(char *filename, char *filetype)
{
  if (strstr(filename, ".html"))     // 파일 이름에 ".html" 이 포함되어 있으면
    strcpy(filetype, "text/html");   // 파일 타입을 HTML 로 설정
  else if (strstr(filename, ".gif")) // ".gif" 가 포함되어 있으면
    strcpy(filetype, "image/gif");   // 파일 타입을 GIF 로 설정, 이하 생략
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else // 알려진 타입이 아니면 기본으로 텍스트로 설정
    strcpy(filetype, "text/plain");
}

/*
 * serve_dynamic() 동적 콘텐츠를 클라이언트에 제공한다
 */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
  // 응답 버퍼 및 빈 인자 리스트 삽입
  char buf[MAXLINE], *emptylist[] = {NULL};

  /* Return first part of HTTP Response */
  sprintf(buf, "HTTP/1.0 200 OK \r\n");         // HTTP 응답 상태 줄 설정
  Rio_writen(fd, buf, strlen(buf));             // 응답 헤더를 클라이언트에 전송
  sprintf(buf, "Server: Tiny Web Server \r\n"); // 서버 정보 추가
  Rio_writen(fd, buf, strlen(buf));             // 서버 정보를 클라이언트에 전송

  if (Fork() == 0) /* Child */
  {
    /* Real Server sould set all CGI vars here */
    setenv("QUERY_STRING", cgiargs, 1);   // CGI 인자를 환경 변수로 설정
    Dup2(fd, STDOUT_FILENO);              /* Redirect stdout to clinet */
    Execve(filename, emptylist, environ); /* Run CGI program */
  }
  Wait(NULL); /* Parent wait for and reaps child */
}