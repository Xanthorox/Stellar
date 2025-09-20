#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
#include "stellar/module.h"
#define MONITOR_MODULE_NAME "monitor"
    struct stellar_monitor;
    /*  use stellar_module_manager_get_module() to get monitor_module */
    struct stellar_monitor *monitor_module_to_monitor(struct module *monitor_module);

#define error_format_server_close "ERR Server closed the connection"
#define error_format_connection_error "ERR could not connect to '%s:%u' : Connection refused"
#define error_format_unknown_command "ERR unknown command '%s'"
#define error_format_unknown_arg "ERR unrecognized args '%s'"
#define error_format_wrong_number_of_args "ERR wrong number of arguments for '%s' command"
#define error_format_arg_not_valid_integer "ERR arg '%s' is not an integer or out of range"

    struct monitor_reply;
    struct monitor_reply *monitor_reply_nil(void);
    struct monitor_reply *monitor_reply_new_string(const char *format, ...);
    struct monitor_reply *monitor_reply_new_integer(long long integer);
    struct monitor_reply *monitor_reply_new_double(double dval);
    struct monitor_reply *monitor_reply_new_error(const char *format, ...); /* for common errors, "error_format_xxx" should be used */
    struct monitor_reply *monitor_reply_new_status(const char *format, ...);

    /* Like Linux bash, argv[0] is always the command name itself */
    typedef struct monitor_reply *(monitor_cmd_cb)(struct stellar_monitor *monitor, int argc, char *argv[], void *arg);

    /*
     * All command characters are case-insensitive.
     * The set of flags 'flags' specify the behavior of the command, and should be
     * passed as a C string composed of space separated words, The set of flags are:
     *
     *    "write"   :  The command may modify the data set (it may also read from it).
     *    "readonly":  The command returns data from keys but never writes.
     *
     * return value:
     *    0: success
     *   -1: failed
     *    1: already exist, has been registered
     */
    int monitor_register_cmd(struct stellar_monitor *monitor, const char *cmd, monitor_cmd_cb *cb, const char *flags,
                             const char *hint, const char *description, void *arg);

/*
    All modules should support "-h/--help/help" args to output usage information.
      "show modulea help"
      "show moduleb -h"
      "show modulec --help"
*/

    struct module *monitor_on_init(struct module_manager *mod_mgr);
    void monitor_on_exit(struct module_manager *mod_mgr __attribute__((unused)), struct module *mod);

#ifdef __cplusplus
}
#endif
