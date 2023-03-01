/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdio.h>


#include "chat_cli.h"
#include "model_handler.h"

/* Logging and RTT */
#include "log.h"
#include "rtt_input.h"

//for the local unicast address stuff
#include "device_state_manager.h"

//LOG_MODULE_DECLARE(chat);
//for now just send logs to the regular log
//mess around with log src later

static const struct shell *chat_shell;

/******************************************************************************/
/***************************** Chat model setup *******************************/
/******************************************************************************/
static bt_mesh_chat_cli * mp_server_contexts[ACCESS_ELEMENT_COUNT];//added from health server. im guessing this is used for loading config

struct presence_cache {
	uint16_t addr;
	enum bt_mesh_chat_cli_presence presence;
};

/* Cache of Presence values of other chat clients. */
static struct presence_cache presence_cache[
			    CONFIG_BT_MESH_CHAT_SAMPLE_PRESENCE_CACHE_SIZE];

static const uint8_t *presence_string[] = {
	[BT_MESH_CHAT_CLI_PRESENCE_AVAILABLE] = "available",
	[BT_MESH_CHAT_CLI_PRESENCE_AWAY] = "away",
	[BT_MESH_CHAT_CLI_PRESENCE_DO_NOT_DISTURB] = "dnd",
	[BT_MESH_CHAT_CLI_PRESENCE_INACTIVE] = "inactive",
};

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

/**
 * Returns true if the provided address is unicast address.
 */
static bool address_is_unicast(uint16_t addr)
{
	return (addr > 0) && (addr <= 0x7FFF);
}

/**
 * Returns true if the node is new or the presence status is different from
 * the one stored in the cache.
 */
static bool presence_cache_entry_check_and_update(uint16_t addr,
				       enum bt_mesh_chat_cli_presence presence)
{
	static size_t presence_cache_head;
	size_t i;

	/* Find address in cache. */
	for (i = 0; i < ARRAY_SIZE(presence_cache); i++) {
		if (presence_cache[i].addr == addr) {
			if (presence_cache[i].presence == presence) {
				return false;
			}

			/* Break since the node in the cache. */
			break;
		}
	}

	/* Not in cache. */
	if (i == ARRAY_SIZE(presence_cache)) {
		for (i = 0; i < ARRAY_SIZE(presence_cache); i++) {
			if (!presence_cache[i].addr) {
				break;
			}
		}

		/* Cache is full. */
		if (i == ARRAY_SIZE(presence_cache)) {
			i = presence_cache_head;
			presence_cache_head = (presence_cache_head + 1)
			% CONFIG_BT_MESH_CHAT_SAMPLE_PRESENCE_CACHE_SIZE;
		}
	}

	/* Update cache. */
	presence_cache[i].addr = addr;
	presence_cache[i].presence = presence;

	return true;
}

static void print_client_status(void);

static void handle_chat_start(struct bt_mesh_chat_cli *chat)
{
	__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "print client status\n");
        //print_client_status();
}

static void handle_chat_presence(struct bt_mesh_chat_cli *chat,
				 struct bt_mesh_msg_ctx *ctx,
				 enum bt_mesh_chat_cli_presence presence)
{
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "handle chat presence\n");

        /*
	if (address_is_local(/*chat->model, ctx->addr)) {
		if (address_is_unicast(ctx->recv_dst)) {
			shell_print(chat_shell, "<you> are %s",
				    presence_string[presence]);
		}
	} else {
		if (address_is_unicast(ctx->recv_dst)) {
			shell_print(chat_shell, "<0x%04X> is %s", ctx->addr,
				    presence_string[presence]);
		} else if (presence_cache_entry_check_and_update(ctx->addr,
								 presence)) {
			shell_print(chat_shell, "<0x%04X> is now %s",
				    ctx->addr,
				    presence_string[presence]);
		}
	}*/
}

static void handle_chat_message(struct bt_mesh_chat_cli *chat,
				struct bt_mesh_msg_ctx *ctx,
				const uint8_t *msg)
{
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "handle chat msg\n");

	/* Don't print own messages. *
	if (address_is_local(chat->model, ctx->addr)) {
		return;
	}

	shell_print(chat_shell, "<0x%04X>: %s", ctx->addr, msg);*/
}

static void handle_chat_private_message(struct bt_mesh_chat_cli *chat,
					struct bt_mesh_msg_ctx *ctx,
					const uint8_t *msg)
{
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "handle chat private msg\n");
	/* Don't print own messages. *
	if (address_is_local(chat->model, ctx->addr)) {
		return;
	}

	shell_print(chat_shell, "<0x%04X>: *you* %s", ctx->addr, msg);*/
}

static void handle_chat_message_reply(struct bt_mesh_chat_cli *chat,
				      struct bt_mesh_msg_ctx *ctx)
{
	
        __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "msg recieved\n");
        //shell_print(chat_shell, "<0x%04X> received the message", ctx->addr);
}

static const struct bt_mesh_chat_cli_handlers chat_handlers = {
	.start = handle_chat_start,
	.presence = handle_chat_presence,
	.message = handle_chat_message,
	.private_message = handle_chat_private_message,
	.message_reply = handle_chat_message_reply,
};

/* .. include_startingpoint_model_handler_rst_1 */
//no struct keyword needed - its part of the typedef
static bt_mesh_chat_cli chat = {
	.handlers = &chat_handlers,
};



/*
static void print_client_status(void)
{
	if (!bt_mesh_is_provisioned()) {
		shell_print(chat_shell,
			    "The mesh node is not provisioned. Please provision the mesh node before using the chat.");
	} else {
		shell_print(chat_shell,
			    "The mesh node is provisioned. The client address is 0x%04x.",
			    bt_mesh_model_elem(chat.model)->addr);
	}

	shell_print(chat_shell, "Current presence: %s",
		    presence_string[chat.presence]);
}
*/
/*

/******************************************************************************/
/******************************** Chat shell **********************************/
/******************************************************************************/
static int cmd_status(const struct shell *shell, size_t argc, char *argv[])
{
	print_client_status();

	return 0;
}

void readBuf(){
    SEGGER_RTT_Read(0, chat.buf, 100);
}

int cmd_message2(){
        char msg[100] = "hamborger\0";
        //uint8_t buf[100];//={5,6,7,8,9};
        //SEGGER_RTT_Read(0, buf, 100);//technically works, the read window is extremely short and we need an extra null termination char

        //memcpy(buf,msg,strlen(msg));
        //health_msg_fault_status_t * p_pdu = (health_msg_fault_status_t *) message_buffer;

        //uint8_t buf = (uint8_t)atoi(msg);//TODO make this cleaner
        return bt_mesh_chat_cli_message_send(&chat, chat.buf);
}
static int cmd_message(const struct shell *shell, size_t argc, char *argv[])
{
	int err;

	if (argc < 2) {
		return 22;//-EINVAL;
	}

	err = bt_mesh_chat_cli_message_send(&chat, argv[1]);
	if (err) {
                __LOG(LOG_SRC_APP, LOG_LEVEL_WARN, "Failed to send message: %d", err); //translated
	}

	/* Print own messages in the chat. */
	shell_print(shell, "<you>: %s", argv[1]);

	return 0;
}

static int cmd_private_message(const struct shell *shell, size_t argc,
			       char *argv[])
{
	uint16_t addr;
	int err;

	if (argc < 3) {
		return 22;//-EINVAL;
	}

	addr = strtol(argv[1], NULL, 0);

	/* Print own message to the chat. */
	shell_print(shell, "<you>: *0x%04X* %s", addr, argv[2]);

	err = bt_mesh_chat_cli_private_message_send(&chat, addr, argv[2]);
	if (err) {
                __LOG(LOG_SRC_APP, LOG_LEVEL_WARN, "Failed to publish message: %d", err); //translated
	}

	return 0;
}

static int cmd_presence_set(const struct shell *shell, size_t argc,
			    char *argv[])
{
	size_t i;

	if (argc < 2) {
		return 22;//-EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(presence_string); i++) {
		if (!strcmp(argv[1], presence_string[i])) {
			enum bt_mesh_chat_cli_presence presence;
			int err;

			presence = i;

			err = bt_mesh_chat_cli_presence_set(&chat, presence);
			if (err) {
                                __LOG(LOG_SRC_APP, LOG_LEVEL_WARN, "Failed to update presence: %d", err); //translated
			}

			/* Print own presence in the chat. */
			shell_print(shell, "You are now %s",
				    presence_string[presence]);

			return 0;
		}
	}

	shell_print(shell,
		    "Unknown presence status: %s. Possible presence statuses:",
		    argv[1]);
	for (i = 0; i < ARRAY_SIZE(presence_string); i++) {
		shell_print(shell, "%s", presence_string[i]);
	}

	return 0;
}

static int cmd_presence_get(const struct shell *shell, size_t argc,
			    char *argv[])
{
	uint16_t addr;
	int err;

	if (argc < 2) {
		return 22;//-EINVAL;
	}

	addr = strtol(argv[1], NULL, 0);

	err = bt_mesh_chat_cli_presence_get(&chat, addr);
	if (err) {
                __LOG(LOG_SRC_APP, LOG_LEVEL_WARN, "Failed to publish message: %d", err); //translated
	}

	return 0;
}

/*
SHELL_STATIC_SUBCMD_SET_CREATE(presence_cmds,
	SHELL_CMD_ARG(set, NULL,
		      "Set presence of the current client <presence: available, away, dnd or inactive>",
		      cmd_presence_set, 2, 0),
	SHELL_CMD_ARG(get, NULL,
		      "Get presence status of the remote node <node>",
		      cmd_presence_get, 2, 0),
	SHELL_SUBCMD_SET_END
);*/

static int cmd_presence(const struct shell *shell, size_t argc, char *argv[])
{
	if (argc == 1) {
		shell_help(shell);
		/* shell returns 1 when help is printed */
		return 1;
	}

	if (argc != 3) {
		return 22;//-EINVAL;
	}

	return 0;
}

/*
SHELL_STATIC_SUBCMD_SET_CREATE(chat_cmds,
	SHELL_CMD_ARG(status, NULL, "Print client status", cmd_status, 1, 0),
	SHELL_CMD(presence, &presence_cmds, "Presence commands", cmd_presence),
	SHELL_CMD_ARG(private, NULL,
		      "Send a private text message to a client <node> <message>",
		      cmd_private_message, 3, 0),
	SHELL_CMD_ARG(msg, NULL, "Send a text message to the chat <message>",
		      cmd_message, 2, 0),
	SHELL_SUBCMD_SET_END
);*/

static int cmd_chat(const struct shell *shell, size_t argc, char **argv)
{
	if (argc == 1) {
		shell_help(shell);
		/* shell returns 1 when help is printed */
		return 1;
	}

	shell_error(shell, "%s unknown parameter: %s", argv[0], argv[1]);

	return 22;//-EINVAL;
}

/*
SHELL_CMD_ARG_REGISTER(chat, &chat_cmds, "Bluetooth Mesh Chat Client commands",
		       cmd_chat, 1, 1);*/

/******************************************************************************/
/******************************** Public API **********************************/
/******************************************************************************/


uint32_t frankenchat_init(/*bt_mesh_chat_cli * p_server, uint16_t element_index, uint16_t company_id*/)//,
        /*health_server_attention_cb_t attention_cb,
        const health_server_selftest_t * p_selftests, uint8_t num_selftests)*/
{
    /*p_server->attention_handler = attention_cb;
    p_server->p_selftests = p_selftests;
    p_server->num_selftests = num_selftests;
    p_server->previous_test_id = 0;
    p_server->company_id = company_id;
    p_server->attention_timer = 0;
    p_server->fast_period_divisor = 0;
    p_server->p_next = NULL;*/
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "frankenchat init running\n");

    /* Clear fault arrays: */
    //bitfield_clear_all(p_server->registered_faults, HEALTH_SERVER_FAULT_ARRAY_SIZE);
    //bitfield_clear_all(p_server->current_faults, HEALTH_SERVER_FAULT_ARRAY_SIZE);
    bt_mesh_chat_cli * p_server = &chat;//TODO this is a private element, gotta make it clear
    uint16_t element_index = 0;//guess element 0. TODO make this an input

    access_model_add_params_t add_params =
    {
        .element_index = element_index,
        .model_id = ACCESS_MODEL_VENDOR( BT_MESH_CHAT_CLI_VENDOR_MODEL_ID,BT_MESH_CHAT_CLI_VENDOR_COMPANY_ID), 
        .p_opcode_handlers = m_opcode_handlers,   //works once extern const stuff is set up in chat cli.h
        .opcode_count = 5,//sizeof(m_opcode_handlers) / sizeof(m_opcode_handlers[0]),//TODO fix magic number
        //.publish_timeout_cb = health_publish_timeout_handler,
        .p_args = p_server
    };

    //this line actually makes the model available through mesh
    uint32_t status = access_model_add(&add_params, &(p_server->model_handle));

    if (status == NRF_SUCCESS)
    {
        //allows for subscription/publication
        mp_server_contexts[element_index] = p_server;
        status = access_model_subscription_list_alloc(p_server->model_handle);
    }

    return status;
}

