#include "timer.h"
#include <signal.h>
#include <sys/time.h>

int timer_init(alarm_call_back_t handler)
{
   signal(SIGALRM, handler);
   return 0;
}

void timer_reset_and_start(int secs)
{
   struct itimerval interval;

   interval.it_interval.tv_sec = secs;
   interval.it_interval.tv_usec = 0;
   interval.it_value = interval.it_interval;
   setitimer(ITIMER_REAL, &interval, 0);
}
