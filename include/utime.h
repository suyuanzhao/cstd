
#ifndef _UTIME_H_
#define _UTIME_H_

#include <time.h>


////////////////////////////////////////////////////////////////////////
// ��ʱ�� (��λΪ��, ��ȷ������)
////////////////////////////////////////////////////////////////////////


#ifdef _WIN32
#include <windows.h>
// �߾��ȼ�ʱ�� (��λΪ��, ��ȷ��΢��)
static __int64 utime_frequency()
{
  __int64  frequency;
  QueryPerformanceFrequency((LARGE_INTEGER*)(&frequency));
  return frequency;
}
static __int64 utime_counter()
{
  __int64  counter;
  QueryPerformanceCounter((LARGE_INTEGER*)(&counter));
  return counter;
}
#define utime_restart(_start_time)  _start_time = utime_counter()
#define utime_start(_start_time)    int64 utime_restart(_start_time)
#define utime_elapsed(_start_time)  ((double)(utime_counter() - _start_time) / utime_frequency())
#endif

#if defined(__linux__) && !defined(__ARM_NEON)
#include <sys/time.h>
// ��ü�������ʱ��Ƶ��
// ��ó�ʼֵ
static int64_t utime_counter()
{
  int64_t  counter;
  timeval t;
  gettimeofday(&t, NULL);
  counter = ((int64_t)t.tv_sec) * 1000 + (int64_t)t.tv_usec;
  //printf("%lu %d\n", counter, sizeof(t.tv_sec));
  return counter;
}
#define utime_restart(_start_time)  _start_time = utime_counter()
#define utime_start(_start_time)    int64_t utime_restart(_start_time)
#define utime_elapsed(_start_time)  ((double)(utime_counter() - _start_time) / 1000000)
#endif



//��ʼ��ʱ
#ifndef utime_restart
#define utime_restart(_start_time)  _start_time = clock()
#define utime_start(_start_time)    clock_t utime_restart(_start_time)
#define utime_elapsed(_start_time)  (double)(clock() - _start_time) / CLOCKS_PER_SEC
#endif




#endif // _UTIME_H_