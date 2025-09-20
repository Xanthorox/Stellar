#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#define __FAVOR_BSD 1
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netinet/ip_icmp.h>
#include <linux/if_ether.h>
#include <linux/mpls.h>

#include "stellar/packet_tag.h"
#include "stellar/exdata.h"
#include "stellar/module.h"

struct packet;
/******************************************************************************
 * layer
 ******************************************************************************/

enum layer_proto
{
    LAYER_PROTO_NONE = 0,

    // L2 -- data link layer
    LAYER_PROTO_ETHER = 1,
    LAYER_PROTO_PWETH = 2,
    LAYER_PROTO_PPP = 3,
    LAYER_PROTO_L2TP = 4,

    // L2 -- tunnel
    LAYER_PROTO_VLAN = 21,
    LAYER_PROTO_PPPOE = 22,
    LAYER_PROTO_MPLS = 23,

    // L3 -- network layer
    LAYER_PROTO_IPV4 = 31,
    LAYER_PROTO_IPV6 = 32,
    LAYER_PROTO_IPAH = 33,

    // L3 -- tunnel
    LAYER_PROTO_GRE = 41,

    // L4 -- transport layer
    LAYER_PROTO_UDP = 51,
    LAYER_PROTO_TCP = 52,
    LAYER_PROTO_ICMP = 53,
    LAYER_PROTO_ICMP6 = 54,

    // L4 -- tunnel
    LAYER_PROTO_VXLAN = 61,
    LAYER_PROTO_GTP_U = 62,
    LAYER_PROTO_GTP_C = 63,
};

struct layer
{
    enum layer_proto proto;
    uint16_t hdr_len;
    union
    {
        // all hdr ptr refer to raw packet, read-only
        const struct ethhdr *eth;
        const struct ip *ip4;
        const struct ip6_hdr *ip6;
        const struct tcphdr *tcp;
        const struct udphdr *udp;
        const struct icmphdr *icmp4;
        const struct icmp6_hdr *icmp6;
        const struct mpls_label *mpls;
        const char *raw; // e.g. pppoe, l2tp, gre, gtp, etc.
    } hdr;
};

int packet_get_layer_count(const struct packet *pkt);
const struct layer *packet_get_layer_by_idx(const struct packet *pkt, int idx);

//  // example: foreach layer in packet (inorder)
//  int count = packet_get_layer_count(pkt);
//  for (int i = 0; i < count; i++)
//  {
//      const struct layer *layer = packet_get_layer_by_idx(pkt, i);
//      // do something with layer
//  }
//
//
//  // example: foreach layer in packet (reverse)
//  int count = packet_get_layer_count(pkt);
//  for (int i = count - 1; i >= 0; i--)
//  {
//      const struct layer *layer = packet_get_layer_by_idx(pkt, i);
//      // do something with layer
//  }

/******************************************************************************
 * tunnel
 ******************************************************************************/

enum tunnel_type
{
    TUNNEL_IPV4 = 1, // contain layers: IPv4, (next inner layer must be IPv4 / IPv6)
    TUNNEL_IPV6 = 2, // contain layers: IPv6, (next inner layer must be IPv4 / IPv6)

    TUNNEL_GRE = 3, // contain layers: IPv4 + GRE
                    // contain layers: IPv6 + GRE

    TUNNEL_GTP = 4, // contain layers: IPv4 + UDP + GTP
                    // contain layers: IPv6 + UDP + GTP

    TUNNEL_VXLAN = 5, // contain layers: IPv4 + UDP + VXLAN
                      // contain layers: IPv6 + UDP + VXLAN

    TUNNEL_L2TP = 6, // contain layers: IPv4 + UDP + L2TP
                     // contain layers: IPv6 + UDP + L2TP

    TUNNEL_TEREDO = 7, // contain layers: IPv4 + UDP, (next inner layer must be IPv6)
};

#define MAX_LAYERS_PER_TUNNEL 3
struct tunnel
{
    enum tunnel_type type;

    int layer_count;
    const struct layer *layers[MAX_LAYERS_PER_TUNNEL];
};

int packet_get_tunnel_count(const struct packet *pkt);
// return  0: success 
// return -1: failed
int packet_get_tunnel_by_idx(const struct packet *pkt, int idx, struct tunnel *out);

/******************************************************************************
 * utils
 ******************************************************************************/

#define MAX_SIDS 8
struct sids
{
    uint16_t sid[MAX_SIDS];
    int used;
};
void packet_prepend_sids(struct packet *pkt, const struct sids *sids);

enum packet_direction
{
    PACKET_DIRECTION_OUTGOING = 0, // Internal -> External: 0
    PACKET_DIRECTION_INCOMING = 1, // External -> Internal: 1
};
enum packet_direction packet_get_direction(const struct packet *pkt);

enum packet_action
{
    PACKET_ACTION_FORWARD = 0,
    PACKET_ACTION_DROP = 1,
};

enum packet_type
{
    PACKET_TYPE_RAW = 0,
    PACKET_TYPE_PSEUDO = 1,
};

enum packet_stage
{
    PACKET_STAGE_PREROUTING,
    PACKET_STAGE_INPUT,
    PACKET_STAGE_FORWARD,
    PACKET_STAGE_OUTPUT,
    PACKET_STAGE_POSTROUTING,
    PACKET_STAGE_MAX,
};

void packet_set_type(struct packet *pkt, enum packet_type type);
enum packet_type packet_get_type(const struct packet *pkt);

void packet_set_action(struct packet *pkt, enum packet_action action);
enum packet_action packet_get_action(const struct packet *pkt);

void packet_set_timeval(struct packet *pkt, const struct timeval *tv);
const struct timeval *packet_get_timeval(const struct packet *pkt);

const char *packet_get_raw_data(const struct packet *pkt);
uint16_t packet_get_raw_len(const struct packet *pkt);

const char *packet_get_payload_data(const struct packet *pkt);
uint16_t packet_get_payload_len(const struct packet *pkt);

void packet_set_exdata(struct packet *pkt, int idx, void *ex_ptr);
void *packet_get_exdata(const struct packet *pkt, int idx);

void packet_tag_set(struct packet *pkt, uint64_t key_bits, uint64_t val_bits);
void packet_tag_get(const struct packet *pkt, uint64_t *key_bits, uint64_t *val_bits);

int packet_get_ip_proto(const struct packet *pkt);
enum packet_stage packet_get_stage(const struct packet *pkt);

/******************************************************************************
 * packet manager
 ******************************************************************************/

#define PACKET_MANAGER_MODULE_NAME "packet_manager_module"
struct packet_manager;
struct packet_manager *module_to_packet_manager(struct module *mod);
int packet_manager_new_packet_exdata_index(struct packet_manager *pkt_mgr, const char *name, exdata_free *func, void *arg);

typedef void on_packet_callback(struct packet *pkt, struct module *mod);
int packet_manager_register_node(struct packet_manager *pkt_mgr, const char *name, enum packet_stage stage,
                                 uint64_t interested_tag_key_bits,
                                 uint64_t interested_tag_val_bits,
                                 on_packet_callback *cb, struct module *mod);
// if two modules claim the same packet at the same stage, the second 'claim' fails.
// return  0 on success
// return -1 on failure
int packet_manager_claim_packet(struct packet_manager *pkt_mgr, uint16_t thread_id, struct packet *pkt, on_packet_callback *cb, void *arg);
void packet_manager_schedule_packet(struct packet_manager *pkt_mgr, uint16_t thread_id, struct packet *pkt, enum packet_stage stage);

/*
 * tcp_seq: the sequence number of the new TCP packet (in host byte order)
 * tcp_ack: the acknowledgment number of the new TCP packet (in host byte order)
 * tcp_options_len: the length of the options (must be a multiple of 4)
 */
struct packet *packet_manager_build_tcp_packet(struct packet_manager *pkt_mgr, uint16_t thread_id, const struct packet *origin_pkt,
                                               uint32_t tcp_seq, uint32_t tcp_ack, uint8_t tcp_flags,
                                               const char *tcp_options, uint16_t tcp_options_len,
                                               const char *tcp_payload, uint16_t tcp_payload_len);
struct packet *packet_manager_build_udp_packet(struct packet_manager *pkt_mgr, uint16_t thread_id, const struct packet *origin_pkt,
                                               const char *udp_payload, uint16_t udp_payload_len);
struct packet *packet_manager_build_l3_packet(struct packet_manager *pkt_mgr, uint16_t thread_id, const struct packet *origin_pkt,
                                              uint8_t ip_proto, const char *l3_payload, uint16_t l3_payload_len);
struct packet *packet_manager_dup_packet(struct packet_manager *pkt_mgr, uint16_t thread_id, const struct packet *origin_pkt);
void packet_manager_free_packet(struct packet_manager *pkt_mgr, uint16_t thread_id, struct packet *pkt);


struct module *packet_manager_on_init(struct module_manager *mod_mgr);
void packet_manager_on_exit(struct module_manager *mod_mgr __attribute__((unused)), struct module *mod);
struct module *packet_manager_on_thread_init(struct module_manager *mod_mgr __attribute__((unused)), int thread_id, struct module *mod);
void packet_manager_on_thread_exit(struct module_manager *mod_mgr __attribute__((unused)), int thread_id, struct module *mod);

#ifdef __cplusplus
}
#endif
