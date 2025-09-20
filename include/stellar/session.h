#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "stellar/packet.h"
#include "stellar/module.h"

enum session_state
{
    SESSION_STATE_INIT = 0,
    SESSION_STATE_OPENING = 1,
    SESSION_STATE_ACTIVE = 2,
    SESSION_STATE_CLOSING = 3,
    SESSION_STATE_DISCARD = 4,
    SESSION_STATE_CLOSED = 5,
    MAX_STATE = 6,
};

enum session_type
{
    SESSION_TYPE_TCP = 0x1,
    SESSION_TYPE_UDP = 0x2,
};

enum session_direction
{
    SESSION_DIRECTION_INBOUND = 0,
    SESSION_DIRECTION_OUTBOUND = 1,
};

enum flow_type
{
    FLOW_TYPE_NONE = -1,
    FLOW_TYPE_C2S = 0,
    FLOW_TYPE_S2C = 1,
    MAX_FLOW_TYPE = 2,
};

enum closing_reason
{
    CLOSING_BY_TIMEOUT = 0x1,
    CLOSING_BY_LRU_EVICTED = 0x2,
    CLOSING_BY_PORT_REUSE_EVICTED = 0x3,
    CLOSING_BY_CLIENT_FIN = 0x4,
    CLOSING_BY_CLIENT_RST = 0x5,
    CLOSING_BY_SERVER_FIN = 0x6,
    CLOSING_BY_SERVER_RST = 0x7,
};

enum session_stat
{
    // raw packet
    STAT_RAW_PACKETS_RECEIVED,
    STAT_RAW_BYTES_RECEIVED,

    STAT_RAW_PACKETS_TRANSMITTED,
    STAT_RAW_BYTES_TRANSMITTED,

    STAT_RAW_PACKETS_DROPPED,
    STAT_RAW_BYTES_DROPPED,

    // duplicate packets
    STAT_DUPLICATE_PACKETS_BYPASS,
    STAT_DUPLICATE_BYTES_BYPASS,

    // injected packet
    STAT_INJECTED_PACKETS_FAILED,
    STAT_INJECTED_PACKETS_SUCCESS,
    STAT_INJECTED_BYTES_SUCCESS,

    // pseudo packet
    STAT_PSEUDO_PACKETS_RECEIVED,
    STAT_PSEUDO_BYTES_RECEIVED,

    STAT_PSEUDO_PACKETS_TRANSMITTED,
    STAT_PSEUDO_BYTES_TRANSMITTED,

    STAT_PSEUDO_PACKETS_DROPPED,
    STAT_PSEUDO_BYTES_DROPPED,

    // TCP segment
    STAT_TCP_SEGMENTS_RECEIVED,
    STAT_TCP_PAYLOADS_RECEIVED,

    STAT_TCP_SEGMENTS_EXPIRED,
    STAT_TCP_PAYLOADS_EXPIRED,

    STAT_TCP_SEGMENTS_RETRANSMIT,
    STAT_TCP_PAYLOADS_RETRANSMIT,

    STAT_TCP_SEGMENTS_OVERLAP,
    STAT_TCP_PAYLOADS_OVERLAP,

    STAT_TCP_SEGMENTS_NOSPACE,
    STAT_TCP_PAYLOADS_NOSPACE,

    STAT_TCP_SEGMENTS_INORDER,
    STAT_TCP_PAYLOADS_INORDER,

    STAT_TCP_SEGMENTS_REORDERED,
    STAT_TCP_PAYLOADS_REORDERED,

    STAT_TCP_SEGMENTS_BUFFERED,
    STAT_TCP_PAYLOADS_BUFFERED,

    STAT_TCP_SEGMENTS_RELEASED,
    STAT_TCP_PAYLOADS_RELEASED,

    MAX_STAT,
};

// realtime in milliseconds
enum session_timestamp
{
    SESSION_TIMESTAMP_START,
    SESSION_TIMESTAMP_LAST,
    MAX_TIMESTAMP,
};

struct session;
#define SESSION_SEEN_C2S_FLOW (1 << 0)
#define SESSION_SEEN_S2C_FLOW (1 << 1)
int session_is_symmetric(const struct session *sess, unsigned char *flag);
int session_has_duplicate_traffic(const struct session *sess);
enum session_type session_get_type(const struct session *sess);
enum session_state session_get_current_state(const struct session *sess);
enum closing_reason session_get_closing_reason(const struct session *sess);
enum session_direction session_get_direction(const struct session *sess);
enum flow_type session_get_flow_type(const struct session *sess);
const struct packet *session_get_first_packet(const struct session *sess, enum flow_type type);
uint64_t session_get_id(const struct session *sess);
uint64_t session_get_timestamp(const struct session *sess, enum session_timestamp type);
uint64_t session_get_stat(const struct session *sess, enum flow_type type, enum session_stat stat);
const char *session_get_readable_addr(const struct session *sess);
void session_set_exdata(struct session *sess, int idx, void *ex_ptr);
void *session_get_exdata(const struct session *sess, int idx);

#define SESSION_MANAGER_MODULE_NAME "session_manager_module"
struct session_manager;
struct session_manager *module_to_session_manager(struct module *mod);
int session_manager_new_session_exdata_index(struct session_manager *sess_mgr, const char *name, exdata_free *func, void *arg);
struct session *session_manager_lookup_session_by_packet(struct session_manager *sess_mgr, uint16_t thread_id, const struct packet *pkt);
struct session *session_manager_lookup_session_by_id(struct session_manager *sess_mgr, uint16_t thread_id, uint64_t sess_id);
void session_manager_discard_session(struct session_manager *sess_mgr, uint16_t thread_id, struct session *sess);

struct module *session_manager_on_init(struct module_manager *mod_mgr);
void session_manager_on_exit(struct module_manager *mod_mgr, struct module *mod);
struct module *session_manager_on_thread_init(struct module_manager *mod_mgr, int thread_id, struct module *mod);
void session_manager_on_thread_exit(struct module_manager *mod_mgr, int thread_id, struct module *mod);

void session_manager_on_packet_forward(struct packet *pkt, struct module *mod);
void session_manager_on_packet_output(struct packet *pkt, struct module *mod);

#define SESSION_DEBUGGER_MODULE_NAME "session_debugger_module"
struct module *session_debugger_on_init(struct module_manager *mod_mgr);
void session_debugger_on_exit(struct module_manager *mod_mgr, struct module *mod);
void session_debugger_on_packet_forward(struct packet *pkt, struct module *mod);

struct module *session_monitor_on_init(struct module_manager *mod_mgr);
void session_monitor_on_exit(struct module_manager *mod_mgr, struct module *mod);

#define TCP_SEGMENT_FROM_RAW_PACKET 1
#define TCP_SEGMENT_FROM_REASSEMBLY 0
struct tcp_segment
{
    uint8_t from; // TCP_SEGMENT_FROM_RAW_PACKET or TCP_SEGMENT_FROM_REASSEMBLY
    uint32_t len;
    const void *data;
    struct tcp_segment *next;
};

struct session *packet_exdata_to_session(const struct session_manager *sess_mgr, const struct packet *pkt);
struct tcp_segment *packet_exdata_to_tcp_segment(const struct session_manager *sess_mgr, const struct packet *pkt);
/*
 * example how to use tcp_segment
 *
 * struct tcp_segment *seg = packet_exdata_to_tcp_segment(sess_mgr, pkt);
 * while (seg)
 * {
 *    do_something(seg->data, seg->len);
 *    seg = seg->next;
 * }
 */

#ifdef __cplusplus
}
#endif
