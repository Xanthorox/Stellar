#include <sys/uio.h>
 
#define STRATUM_MESSAGE_TOPIC "TOPIC_STRATUM"
 
enum cryptocurrency_type
{
    ETH=1,
    OTHER=2
};
 
struct stratum_field
{
    enum cryptocurrency_type type;
    struct iovec mining_pools;
    struct iovec mining_program;
    struct iovec mining_subscribe;
     
};//message data