#pragma once

#include <stdlib.h>
#ifdef __cplusplus
extern "C"
{
#endif

#include "stellar/log.h"

/*******************************************
 * module API *
 *******************************************/

struct module;
struct module *module_new(const char *name, void *ctx);
void module_free(struct module *mod);

void * module_get_ctx(struct module *mod);
void module_set_ctx(struct module *mod, void *ctx);

const char *module_get_name(struct module* mod);
void module_set_name(struct module* mod, const char *name);

/*******************************************
 * module manager API *
 *******************************************/

struct module_manager;

typedef struct module *module_on_instance_init_func(struct module_manager *mod_mgr);
typedef void module_on_instance_exit_func(struct module_manager *mod_mgr, struct module *mod);

typedef struct module *module_on_thread_init_func(struct module_manager *mod_mgr, int thread_id, struct module *mod);
typedef void module_on_thread_exit_func(struct module_manager *mod_mgr, int thread_id, struct module *mod);

struct module_hooks
{
	module_on_instance_init_func *on_instance_init_cb;
	module_on_instance_exit_func *on_instance_exit_cb;
	module_on_thread_init_func *on_thread_init_cb;
	module_on_thread_exit_func *on_thread_exit_cb;
};

struct module_manager *module_manager_new(struct module_hooks mod_specs[], size_t n_mod, int max_thread_num, const char *toml_path, struct logger *logger);
struct module_manager *module_manager_new_with_toml(const char *toml_path, int max_thread_num, struct logger *logger);

void module_manager_free(struct module_manager *mod_mgr);

void module_manager_register_thread(struct module_manager *mod_mgr, int thread_id);
void module_manager_unregister_thread(struct module_manager *mod_mgr, int thread_id);

// return -1 on error
int module_manager_get_thread_id(struct module_manager *mod_mgr);

struct module *module_manager_get_module(struct module_manager *mod_mgr, const char *module_name); 

int module_manager_get_max_thread_num(struct module_manager *mod_mgr);
const char *module_manager_get_toml_path(struct module_manager *mod_mgr);
struct logger *module_manager_get_logger(struct module_manager *mod_mgr);

/*******************************************
 * polling API *
 *******************************************/

typedef void on_polling_callback(struct module_manager *mod_mgr, void *polling_arg);
int module_manager_register_polling_node(struct module_manager *mod_mgr, on_polling_callback *on_polling, void *polling_arg);
void module_manager_polling_dispatch(struct module_manager *mod_mgr);

#ifdef __cplusplus
}
#endif