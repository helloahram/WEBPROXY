/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void)
{
  // buf - QUERY_STRING 환경 변수에서 읽은 값을 저장할 포인터
  // p - 문자열에서 & 문자의 위치를 찾기 위해 사용될 포인터
  char *buf, *p;
  // arg1, arg2 - 첫번째와 두번째 숫자를 저장할 문자열
  // content - HTTP 응답 본문을 저장할 문자열
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
  int n1 = 0, n2 = 0; // 첫번째와 두번째 숫자를 정수형으로 저장할 변수

  /* Extract the two arguments */
  // getevn 함수는 환경 변수 QUERY_STRING 의 값을 가져온다
  // 이 문자열은 URL 의 쿼리 파라미터를 포함한다
  // buf 가 NULL 이 아니면 쿼리 문자열이 정상적으로 가져온 것
  if ((buf = getenv("QUERY_STRING")) != NULL)
  {
    p = strchr(buf, '&');
    *p = '\0';

    strcpy(arg1, buf);
    strcpy(arg2, p + 1);
    n1 = atoi(arg1);
    n2 = atoi(arg2);
  }

  /* Make the Response Body */
  sprintf(content, "QUERY_STRING=%s", buf);
  sprintf(content, "Welcome to add.com: ");
  sprintf(content, "%sTHE Internet addition portal \r\n<p>", content);
  sprintf(content, "%sThe answer is: %d + %d = %d \r\n<p>", content, n1, n2, n1 + n2);
  sprintf(content, "%sThanks for visiting! \r\n");

  /* Generate the HTTP response */
  printf("Connection: close \r\n");
  printf("Content-length: %d \r\n", (int)strlen(content));
  printf("Content-type: text/html \r\n\r\n");
  printf("%s", content);
  fflush(stdout);

  exit(0);
}
/* $end adder */
