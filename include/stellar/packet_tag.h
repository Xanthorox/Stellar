#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * Packet Tag Key
 ******************************************************************************/

#define PKT_TAG_KEY_SESS 1 << 0
#define PKT_TAG_KEY_IPPROTO 1 << 1
#define PKT_TAG_KEY_L7PROTO 1 << 2

/******************************************************************************
 * Packet Tag Val
 ******************************************************************************/

#define PKT_TAG_VAL_SESS_NEW 1 << 0
#define PKT_TAG_VAL_SESS_FREE 1 << 1
#define PKT_TAG_VAL_SESS_RAWPKT 1 << 2
#define PKT_TAG_VAL_SESS_PSEUDOPKT 1 << 3
#define PKT_TAG_VAL_SESS_FLAG 1 << 4
#define PKT_TAG_VAL_SESS_TCP_STREAM 1 << 5
#define PKT_TAG_VAL_SESS_ALL (PKT_TAG_VAL_SESS_NEW | PKT_TAG_VAL_SESS_FREE | PKT_TAG_VAL_SESS_RAWPKT | PKT_TAG_VAL_SESS_PSEUDOPKT | PKT_TAG_VAL_SESS_FLAG | PKT_TAG_VAL_SESS_TCP_STREAM)

#define PKT_TAG_VAL_IPPROTO_TCP 1 << 10
#define PKT_TAG_VAL_IPPROTO_UDP 1 << 11
#define PKT_TAG_VAL_IPPROTO_ICMP 1 << 12
#define PKT_TAG_VAL_IPPROTO_ALL (PKT_TAG_VAL_IPPROTO_TCP | PKT_TAG_VAL_IPPROTO_UDP | PKT_TAG_VAL_IPPROTO_ICMP)

#define PKT_TAG_VAL_L7PROTO_SSL 1 << 21
#define PKT_TAG_VAL_L7PROTO_DNS 1 << 22
#define PKT_TAG_VAL_L7PROTO_HTTP 1 << 23
#define PKT_TAG_VAL_L7PROTO_QUIC 1 << 24
#define PKT_TAG_VAL_L7PROTO_ALL (PKT_TAG_VAL_L7PROTO_SSL | PKT_TAG_VAL_L7PROTO_DNS | PKT_TAG_VAL_L7PROTO_HTTP | PKT_TAG_VAL_L7PROTO_QUIC)

#ifdef __cplusplus
}
#endif
