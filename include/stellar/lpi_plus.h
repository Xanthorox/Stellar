#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include "stellar/module.h"
#include "stellar/packet.h"

#define LPI_PLUS_MODULE_NAME "LPI_PLUS"

struct module *lpi_plus_init(struct module_manager *mod_mgr);
void lpi_plus_exit(struct module_manager *mod_mgr, struct module *mod);

struct lpi_plus;
struct lpi_plus *module_to_lpi_plus(struct module *mod);

const char *lpi_plus_appid2name(struct lpi_plus *lpip, int appid);

void lpi_plus_on_packet(struct packet *pkt, struct module *mod);
int32_t *packet_exdata_to_lpip_appid(struct lpi_plus *lpip, struct packet *pkt, size_t *appid_num);


#ifdef __cplusplus
}
#endif