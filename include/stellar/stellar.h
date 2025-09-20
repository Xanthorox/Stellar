#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "stellar/log.h"
#include "stellar/module.h"

struct stellar;
struct stellar *stellar_new(const char *toml_file);
void stellar_run(struct stellar *st);
void stellar_free(struct stellar *st);
void stellar_loopbreak(struct stellar *st);
void stellar_reload_log_level(struct stellar *st);
struct logger *stellar_get_logger(struct stellar *st);
struct module_manager *stellar_get_module_manager(struct stellar *st);

#ifdef __cplusplus
}
#endif
