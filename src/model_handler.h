/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file
 * @brief Model handler
 */

#ifndef MODEL_HANDLER_H__
#define MODEL_HANDLER_H__

#include "chat_cli.h"
#include <stdlib.h>

uint32_t frankenchat_init(/*bt_mesh_chat_cli* p_server, uint16_t element_index, uint16_t company_id*/);
int cmd_message2();
void pingNeighbors(int ttl, uint16_t addr);//0xFFFF for broadcast
void readBuf();

#ifdef __cplusplus
extern "C" {
#endif

const struct bt_mesh_comp *model_handler_init(void);

#ifdef __cplusplus
}
#endif

#endif /* MODEL_HANDLER_H__ */
