#pragma once

#include <stddef.h>
#include <stdint.h>

#define MESSAGE_MAGIC 0x12345678

#define SESSION_FLAGS_MESSAGE_TOPIC "TOPIC_SESSION_FLAGS"

enum
{
	session_flags_bulky_mask = 1,
	session_flags_cbr_mask,
	session_flags_local_client_mask,
	session_flags_local_server_mask,
	session_flags_download_mask,
	session_flags_interactive_mask,
	session_flags_inbound_mask,
	session_flags_outbound_mask,
	session_flags_pseudo_unidirectional_mask,
	session_flags_streaming_mask,
	session_flags_unidirectional_mask,
	session_flags_random_looking_mask,
	session_flags_c2s_mask,
	session_flags_s2c_mask,
	session_flags_bidirectional_mask,
	session_flags_tunneling_mask,
	session_flags_all_mask
};

#define SESSION_FLAGS_START 		(0x0000000000000001)
#define SESSION_FLAGS_BULKY 		(SESSION_FLAGS_START << session_flags_bulky_mask)
#define SESSION_FLAGS_CBR 			(SESSION_FLAGS_START << session_flags_cbr_mask)
#define SESSION_FLAGS_LOCAL_CLIENT 		(SESSION_FLAGS_START << session_flags_local_client_mask)
#define SESSION_FLAGS_LOCAL_SERVER 		(SESSION_FLAGS_START << session_flags_local_server_mask)
#define SESSION_FLAGS_DOWNLOAD 		(SESSION_FLAGS_START << session_flags_download_mask)
#define SESSION_FLAGS_INTERACTIVE 		(SESSION_FLAGS_START << session_flags_interactive_mask)
#define SESSION_FLAGS_INBOUND 			(SESSION_FLAGS_START << session_flags_inbound_mask)
#define SESSION_FLAGS_OUTBOUND 		(SESSION_FLAGS_START << session_flags_outbound_mask)
#define SESSION_FLAGS_PSEUDO_UNIDIRECTIONAL 	(SESSION_FLAGS_START << session_flags_pseudo_unidirectional_mask)
#define SESSION_FLAGS_STREAMING 		(SESSION_FLAGS_START << session_flags_streaming_mask)
#define SESSION_FLAGS_UNIDIRECTIONAL 		(SESSION_FLAGS_START << session_flags_unidirectional_mask)
#define SESSION_FLAGS_RANDOM_LOOKING 		(SESSION_FLAGS_START << session_flags_random_looking_mask)
#define SESSION_FLAGS_C2S 			(SESSION_FLAGS_START << session_flags_c2s_mask)
#define SESSION_FLAGS_S2C 			(SESSION_FLAGS_START << session_flags_s2c_mask)
#define SESSION_FLAGS_BIDIRECTIONAL 		(SESSION_FLAGS_START << session_flags_bidirectional_mask)
#define SESSION_FLAGS_TUNNELING 		(SESSION_FLAGS_START << session_flags_tunneling_mask)

struct session_flags_message
{
        int magic;
        uint64_t flags;
        uint32_t array_num;
        uint32_t *packet_sequence_array;
};