#include <sys/uio.h>
 
#include <netinet/in.h>
#ifndef IPV6_ADDR_LEN
#define IPV6_ADDR_LEN   (sizeof(struct in6_addr))
#endif


#define SOCKS_MESSAGE_TOPIC "TOPIC_SOCKS"


enum socks_addr_type
{
    SOCKS_ADDR_IPV4,
    SOCKS_ADDR_IPV6,
    SOCKS_ADDR_FQDN
};
 
struct socks_addr
{
    enum socks_addr_type type;
    union
    {
        uint32_t ipv4;  /* network order */
        uint8_t ipv6[IPV6_ADDR_LEN] ;
        struct iovec fqdn;
    };
    uint16_t port;  /* network order */
};
 
enum socks_version
{
    SOCKS_VERSION_4,
    SOCKS_VERSION_5
};
 
struct socks_info
{
    enum socks_version version;
    struct socks_addr dst_addr;
    struct iovec user_name;
    struct iovec password;
};//message data
 