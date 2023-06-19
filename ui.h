#ifndef UI_H_
#define UI_H_

#include <time.h>

typedef struct _ui {
   size_t file_size_bytes;
   float file_size_M;
   size_t sended_size;
   time_t start_seconds;
   time_t used_seconds;
   time_t expected_remain_seconds;
   float sended_percent;
   float speed_of_transfer;//Kb/s
   size_t err_pck_cnt;
   size_t total_pkt;
   size_t sended_pkt;
   float err_ratio;
}ui_model;

void ui_init(int file_size);
void ui_update_model_sended_size(int sended_size);
void ui_update_model_err_pkt_add_one();
void ui_show();
int ui_get_pkt_num();
int ui_get_retry_pkt_cnt();

#endif//UI_H_
