/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "chat_cli.h"
#include <string.h>

/* Logging and RTT */
#include "log.h"
#include "rtt_input.h"

//LOG_MODULE_DECLARE(chat);

/*
BUILD_ASSERT(BT_MESH_MODEL_BUF_LEN(BT_MESH_CHAT_CLI_OP_MESSAGE,
				   BT_MESH_CHAT_CLI_MSG_MAXLEN_MESSAGE) <=
		    BT_MESH_RX_SDU_MAX,
	     "The message must fit inside an application SDU.");
BUILD_ASSERT(BT_MESH_MODEL_BUF_LEN(BT_MESH_CHAT_CLI_OP_MESSAGE,
				   BT_MESH_CHAT_CLI_MSG_MAXLEN_MESSAGE) <=
		    BT_MESH_TX_SDU_MAX,
	     "The message must fit inside an application SDU.");*/
/*
static void encode_presence(struct net_buf_simple *buf,
			    enum bt_mesh_chat_cli_presence presence)
{
	bt_mesh_model_msg_init(buf, BT_MESH_CHAT_CLI_OP_PRESENCE);
	net_buf_simple_add_u8(buf, presence);
}

static const uint8_t *extract_msg(struct net_buf_simple *buf)
{
	buf->data[buf->len - 1] = '\0';
	return net_buf_simple_pull_mem(buf, buf->len);
}
*/
/**
 * Returns true if the specified address is an address of the local element.
 */
static bool address_is_local(/*struct bt_mesh_model *mod,*/ uint16_t addr)
{
        //this method doesn't actually use mod. there is a local method for getting addresses
        dsm_local_unicast_address_t local_addresses;
        dsm_local_unicast_addresses_get(&local_addresses);

            //there was a third check in original code from the inline function that checked own address
    return  (addr >= local_addresses.address_start &&
            addr < local_addresses.address_start + local_addresses.count);

}

static int handle_message(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "handle_message\n");
    char buf[100];//TODO figure out a max message length, also move away from strings
    memcpy(buf,p_message->p_data, p_message->length +1);//extra space for null termination \0 for string
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "%d,%d, %s\n",strlen(buf),p_message->length, buf);
/*
	struct bt_mesh_chat_cli *chat = model->user_data;
	const uint8_t *msg;

	msg = extract_msg(buf);

	if (chat->handlers->message) {
		chat->handlers->message(chat, ctx, msg);
	}
*/
	return 0;
}

/* .. include_startingpoint_chat_cli_rst_1 */
/*
static void send_message_reply(struct bt_mesh_chat_cli *chat,
			     struct bt_mesh_msg_ctx *ctx)
{
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_CHAT_CLI_OP_MESSAGE_REPLY,
				 BT_MESH_CHAT_CLI_MSG_LEN_MESSAGE_REPLY);
	bt_mesh_model_msg_init(&msg, BT_MESH_CHAT_CLI_OP_MESSAGE_REPLY);

	(void)bt_mesh_model_send(chat->model, ctx, &msg, NULL, NULL);
}

static int handle_private_message(struct bt_mesh_model *model,
				  struct bt_mesh_msg_ctx *ctx,
				  struct net_buf_simple *buf)
{
	struct bt_mesh_chat_cli *chat = model->user_data;
	const uint8_t *msg;

	msg = extract_msg(buf);

	if (chat->handlers->private_message) {
		chat->handlers->private_message(chat, ctx, msg);
	}

	send_message_reply(chat, ctx);
	return 0;
}
*/
/* .. include_endpoint_chat_cli_rst_1 */

/*
static int handle_message_reply(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
				struct net_buf_simple *buf)
{
	struct bt_mesh_chat_cli *chat = model->user_data;

	if (chat->handlers->message_reply) {
		chat->handlers->message_reply(chat, ctx);
	}

	return 0;
}

static int handle_presence(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
			   struct net_buf_simple *buf)
{
	struct bt_mesh_chat_cli *chat = model->user_data;
	enum bt_mesh_chat_cli_presence presence;

	presence = net_buf_simple_pull_u8(buf);

	if (chat->handlers->presence) {
		chat->handlers->presence(chat, ctx, presence);
	}

	return 0;
}

static int handle_presence_get(struct bt_mesh_model *model, struct bt_mesh_msg_ctx *ctx,
			       struct net_buf_simple *buf)
{
	struct bt_mesh_chat_cli *chat = model->user_data;

	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_CHAT_CLI_OP_PRESENCE,
				 BT_MESH_CHAT_CLI_MSG_LEN_PRESENCE);

	encode_presence(&msg, chat->presence);

	(void) bt_mesh_model_send(chat->model, ctx, &msg, NULL, NULL);

	return 0;
}
*/

static void handle_ping(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args){

    uint16_t rx_payload[2];
    memcpy(rx_payload,p_message->p_data, p_message->length);//p_message->length should just be 4

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "ping to 0x%04x, from 0x%04x\n",rx_payload[0],rx_payload[1]);
   
    //get local addr
    dsm_local_unicast_address_t local_addresses;
    dsm_local_unicast_addresses_get(&local_addresses);
    uint16_t local_addr = local_addresses.address_start;

    if (rx_payload[1]==local_addr){
        return;//don't reply to own ping
    }
    
    //reply
    if (rx_payload[0]==0xFFFF||rx_payload[0]==local_addr){
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "reply to ping\n");

        uint16_t tx_payload[2] = {rx_payload[1],local_addr};//TODO consider making payload a named struct

        access_message_tx_t packet =
        {
            .opcode = BT_MESH_CHAT_CLI_OP_PING_REPLY,
            .p_buffer = tx_payload,
            .length = 4,
            .force_segmented = false,
            .transmic_size = NRF_MESH_TRANSMIC_SIZE_DEFAULT,
            .access_token = nrf_mesh_unique_token_get()
        };
        //send reply
        access_model_reply(handle, p_message, &packet);
    }
}

static void handle_ping_reply(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args){
    //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "ping reply received\n");
    uint16_t rx_payload[2];
    memcpy(rx_payload,p_message->p_data, p_message->length);//p_message->length should just be 4

    //get local addr
    dsm_local_unicast_address_t local_addresses;
    dsm_local_unicast_addresses_get(&local_addresses);
    uint16_t local_addr = local_addresses.address_start;

    if (rx_payload[0]==local_addr){
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "ping to 0x%04x, from 0x%04x\n",rx_payload[0],rx_payload[1]);
    }
}


//FLAG: NEW STRUCT
//dummy functions
static void handle_private_message2(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args)
{__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "handle_private_message2\n");}
static void handle_message_reply2(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args)
{__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "handle_message_reply2\n");}
static void handle_presence2(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args)
{__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "handle_presence2\n");}
static void handle_presence_get2(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args)
{__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "handle_presence_get2\n");}
//using dummy callbacks for now. The real callbacks have different inputs between SDKs

//try ACCESS_OPCODE_VENDOR macro if these don't expand right
const access_opcode_handler_t m_opcode_handlers[] =
{
    { BT_MESH_CHAT_CLI_OP_MESSAGE,         handle_message },
    { BT_MESH_CHAT_CLI_OP_PRIVATE_MESSAGE, handle_private_message2 },
    { BT_MESH_CHAT_CLI_OP_MESSAGE_REPLY,   handle_message_reply2 },
    { BT_MESH_CHAT_CLI_OP_PING,            handle_ping },
    { BT_MESH_CHAT_CLI_OP_PING_REPLY,      handle_ping_reply },
};

/*
static int bt_mesh_chat_cli_update_handler(struct bt_mesh_model *model)
{
	struct bt_mesh_chat_cli *chat = model->user_data;

	// Continue publishing current presence. 
        encode_presence(model->pub->msg, chat->presence);

	return 0;
}
*/
/* .. include_startingpoint_chat_cli_rst_3 */
#ifdef CONFIG_BT_SETTINGS
static int bt_mesh_chat_cli_settings_set(struct bt_mesh_model *model,
					 const char *name,
					 size_t len_rd,
					 settings_read_cb read_cb,
					 void *cb_arg)
{
	struct bt_mesh_chat_cli *chat = model->user_data;

	if (name) {
		return -ENOENT;
	}

	ssize_t bytes = read_cb(cb_arg, &chat->presence,
				sizeof(chat->presence));
	if (bytes < 0) {
		return bytes;
	}

	if (bytes != 0 && bytes != sizeof(chat->presence)) {
		return -EINVAL;
	}

	return 0;
}
#endif
/* .. include_endpoint_chat_cli_rst_3 */

/* .. include_startingpoint_chat_cli_rst_4 */

/*
static int bt_mesh_chat_cli_init(struct bt_mesh_model *model)
{
	struct bt_mesh_chat_cli *chat = model->user_data;

	chat->model = model;

	net_buf_simple_init_with_data(&chat->pub_msg, chat->buf,
				      sizeof(chat->buf));
	chat->pub.msg = &chat->pub_msg;
	chat->pub.update = bt_mesh_chat_cli_update_handler;

	return 0;
}
*/
/* .. include_endpoint_chat_cli_rst_4 */

/* .. include_startingpoint_chat_cli_rst_5 */
/*
static int bt_mesh_chat_cli_start(struct bt_mesh_model *model)
{
	struct bt_mesh_chat_cli *chat = model->user_data;

	if (chat->handlers->start) {
		chat->handlers->start(chat);
	}

	return 0;
}
/* .. include_endpoint_chat_cli_rst_5 */

/* .. include_startingpoint_chat_cli_rst_6 */
/*
static void bt_mesh_chat_cli_reset(struct bt_mesh_model *model)
{
	struct bt_mesh_chat_cli *chat = model->user_data;

	chat->presence = BT_MESH_CHAT_CLI_PRESENCE_AVAILABLE;

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		(void) bt_mesh_model_data_store(model, true, NULL, NULL, 0);
	}
}
/* .. include_endpoint_chat_cli_rst_6 */

/* .. include_startingpoint_chat_cli_rst_7 */
/*
const struct bt_mesh_model_cb _bt_mesh_chat_cli_cb = {
	.init = bt_mesh_chat_cli_init,
	.start = bt_mesh_chat_cli_start,
#ifdef CONFIG_BT_SETTINGS
	.settings_set = bt_mesh_chat_cli_settings_set,
#endif
	.reset = bt_mesh_chat_cli_reset,
};
/* .. include_endpoint_chat_cli_rst_7 */

/* .. include_startingpoint_chat_cli_rst_8 */
/*
int bt_mesh_chat_cli_presence_set(struct bt_mesh_chat_cli *chat,
			 enum bt_mesh_chat_cli_presence presence)
{
	if (presence != chat->presence) {
		chat->presence = presence;

		if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
			(void) bt_mesh_model_data_store(chat->model, true,
						NULL, &presence,
						sizeof(chat->presence));
		}
	}

	encode_presence(chat->model->pub->msg, chat->presence);

	return bt_mesh_model_publish(chat->model);
}
/* .. include_endpoint_chat_cli_rst_8 */
/*
int bt_mesh_chat_cli_presence_get(struct bt_mesh_chat_cli *chat,
				  uint16_t addr)
{
	struct bt_mesh_msg_ctx ctx = {
		.addr = addr,
		.app_idx = chat->model->keys[0],
		.send_ttl = BT_MESH_TTL_DEFAULT,
		.send_rel = true,
	};

	BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_CHAT_CLI_OP_PRESENCE_GET,
				 BT_MESH_CHAT_CLI_MSG_LEN_PRESENCE_GET);
	bt_mesh_model_msg_init(&buf, BT_MESH_CHAT_CLI_OP_PRESENCE_GET);

	return bt_mesh_model_send(chat->model, &ctx, &buf, NULL, NULL);
}
*/
uint32_t bt_mesh_chat_cli_message_send(bt_mesh_chat_cli *chat,
				  const uint8_t *msg)
{
    //create packet
    access_message_tx_t packet =
    {
        .opcode = BT_MESH_CHAT_CLI_OP_MESSAGE,
        .p_buffer = msg,
        .length = strlen(msg)+1,//TODO use generic serial data not strings
        .force_segmented = false,
        .transmic_size = NRF_MESH_TRANSMIC_SIZE_DEFAULT,
        .access_token = nrf_mesh_unique_token_get()
    };
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "%d, %s\n",strlen(msg), msg);

    //send/publish the packet
    return access_model_publish(chat->model_handle, &packet);
}

/* .. include_startingpoint_chat_cli_rst_9 */
/*
int bt_mesh_chat_cli_private_message_send(struct bt_mesh_chat_cli *chat,
					  uint16_t addr,
					  const uint8_t *msg)
{
	struct bt_mesh_msg_ctx ctx = {
		.addr = addr,
		.app_idx = chat->model->keys[0],
		.send_ttl = BT_MESH_TTL_DEFAULT,
		.send_rel = true,
	};

	BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_CHAT_CLI_OP_PRIVATE_MESSAGE,
				 BT_MESH_CHAT_CLI_MSG_MAXLEN_MESSAGE);
	bt_mesh_model_msg_init(&buf, BT_MESH_CHAT_CLI_OP_PRIVATE_MESSAGE);

	net_buf_simple_add_mem(&buf, msg,
			       strnlen(msg,
				       CONFIG_BT_MESH_CHAT_CLI_MESSAGE_LENGTH));
	net_buf_simple_add_u8(&buf, '\0');

	return bt_mesh_model_send(chat->model, &ctx, &buf, NULL, NULL);
}
/* .. include_endpoint_chat_cli_rst_9 */
