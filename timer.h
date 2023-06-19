#ifndef TIMER_H_
#define TIMER_H_

typedef void (*alarm_call_back_t)(int sig);

int timer_init(alarm_call_back_t handler);
void timer_reset_and_start(int secs);

#endif//TIMER_H_
