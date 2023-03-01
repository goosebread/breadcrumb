/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file
 * @brief Model handler
 */

#ifndef UWB_BREAD_H__
#define UWB_BREAD_H__

#include <stdint.h>

void breadcrumb_dwm_init(void);
int ss_init_run(void);
static void resp_msg_get_ts(uint8_t *ts_field, uint32_t *ts);
//void ss_initiator_task_function (void * pvParameter);
int target_node_select (int target_node);
int frame_incr(int target_node);
void print_distance(int target_node);

#endif /* UWB_BREAD_H__ */