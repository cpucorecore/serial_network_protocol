#include "ui.h"
#include "global.h"
#include <string.h>
#include <stdio.h>

static ui_model g_mod;

void ui_init(int file_size)
{
   memset((void*)&g_mod, 0, sizeof(ui_model));

   g_mod.file_size_bytes = file_size;
   g_mod.file_size_M = file_size * 1.0f / 1024 / 1024;
   g_mod.total_pkt = (file_size + MAX_DATA_LEN) / MAX_DATA_LEN;
   g_mod.start_seconds = time(NULL);
}

int ui_get_pkt_num()
{
   return g_mod.total_pkt;
}

int ui_get_retry_pkt_cnt()
{
   return g_mod.err_pck_cnt;
}

void ui_update_model_sended_size(int sended_size)
{
   time_t cur_secs = 0;
   cur_secs = time(NULL);
   g_mod.sended_size += sended_size;
   g_mod.sended_pkt += 1;
   g_mod.used_seconds = cur_secs - g_mod.start_seconds;
   g_mod.sended_percent = 100.0f * g_mod.sended_pkt / g_mod.total_pkt;
   g_mod.speed_of_transfer = (float)g_mod.sended_size / 1024 / g_mod.used_seconds;
   g_mod.err_ratio = (float)g_mod.err_pck_cnt / (g_mod.sended_pkt + g_mod.err_pck_cnt);
   g_mod.expected_remain_seconds = g_mod.used_seconds * (g_mod.total_pkt - g_mod.sended_pkt) / g_mod.sended_pkt;
}

void ui_update_model_err_pkt_add_one()
{
   g_mod.err_pck_cnt += 1;
   g_mod.err_ratio = (float)g_mod.err_pck_cnt / (g_mod.sended_pkt + g_mod.err_pck_cnt);
}

void ui_show()
{
   printf("___________________________________\n");
   printf("%.2fM  progress:%.2f%%  %d/%dpkt\n", g_mod.file_size_M, g_mod.sended_percent, g_mod.sended_pkt, g_mod.total_pkt);
   printf("used time:\t%dsecs(%dmins %dsecs)\n", g_mod.used_seconds, g_mod.used_seconds / 60, g_mod.used_seconds % 60);
   printf("remain time:\t%dsecs(%dmins %dsecs)\n", g_mod.expected_remain_seconds, g_mod.expected_remain_seconds / 60, g_mod.expected_remain_seconds % 60);
   printf("speed: %.2fKb/s\n", g_mod.speed_of_transfer);
   printf("err ratio: %.2f%%, err_pkt_cnt: %d\n", g_mod.err_ratio, g_mod.err_pck_cnt);
}
