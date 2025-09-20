#include <arpa/inet.h>
#include <assert.h>
#include <evhttp.h>
#include <getopt.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "stellar/monitor.h"
#include "linenoise/linenoise.h"
#include "monitor_cmd_assistant.h"
#include "monitor_private.h"
#include "monitor/monitor_utils.h"

static char g_stellar_cli_prompt[128]; /* prompt pattern: cli@ip:port> */
static const char *g_stellar_monitor_history_file = ".stellar_cli_history.txt";
static const char *g_stellar_monitor_version = "stellar-cli v1.0";
static int g_stm_cli_noninteractive = 0;
static const char *g_stm_cli_noninteractive_cmd_line = NULL;
static const char *g_stm_cli_ipaddr_str = STM_SERVER_LISTEN_IP;
static unsigned short g_stm_cli_port_host = STM_SERVER_LISTEN_PORT;
static struct libevent_http_client *g_evh_client;
static struct stm_cmd_assistant *g_stm_cli_aide;
static int g_stm_cli_connect_timeout = STM_REQUEST_TIMEOUT;

static struct monitor_cli_args g_cli_args[4] = {
    {"-i", "--ip", 1, 0, NULL},
    {"-p", "--port", 1, 0, NULL},
    {"-t", "--timeout", 1, 0, NULL},
    {"-e", "--exec", 1, 1, NULL},
};

struct stm_cmd_parser
{
    sds raw_cmd_line; // need be free
    int argc;
    sds *argv;
};

struct stm_builtin_cmd_compose
{
    const char *cmd_name;
    monitor_cmd_cb *cmd_cb;
    const char *description;
};

struct libevent_http_client
{
    struct event_base *base;
    struct evhttp_connection *conn;
    struct evhttp_request *req;
    enum stm_http_response_code response_code;
    char *response_cstr;
};

static int (*g_response_handler)(struct libevent_http_client *evh_client);

static void cli_evhttp_free(void)
{
    evhttp_connection_free(g_evh_client->conn);
    event_base_free(g_evh_client->base);
    FREE(g_evh_client);
}

static void stm_cli_args_free(struct stm_cmd_parser *cmd_args)
{
    if (NULL != cmd_args)
    {
        sdsfree(cmd_args->raw_cmd_line);
        sdsfreesplitres(cmd_args->argv, cmd_args->argc);
        FREE(cmd_args);
    }
}

static struct monitor_reply *stm_cli_builtin_exit(UNUSED struct stellar_monitor *monitor, UNUSED int argc, UNUSED char *argv[], void *arg)
{
    for (size_t i = 0; i < sizeof(g_cli_args) / sizeof(struct monitor_cli_args); i++)
    {
        sdsfree(g_cli_args[i].value);
    }
    stm_cli_args_free((struct stm_cmd_parser *)arg);
    stm_cmd_assistant_free(g_stm_cli_aide);
    cli_evhttp_free();
    exit(0);
    return NULL;
}

static void signal_handler(int signo)
{
    if (signo == SIGINT)
    {
        stm_cli_builtin_exit(NULL, 0, NULL, NULL);
    }
}

static struct monitor_reply *stm_cli_builtin_clear(UNUSED struct stellar_monitor *monitor, UNUSED int argc, UNUSED char *argv[], UNUSED void *arg)
{
    linenoiseClearScreen();
    return NULL;
}

static struct stm_builtin_cmd_compose g_stm_cli_builtin_commands[] = {
    {"q", stm_cli_builtin_exit, "cause the shell to exit"},
    {"quit", stm_cli_builtin_exit, "cause the shell to exit"},
    {"exit", stm_cli_builtin_exit, "cause the shell to exit"},
    {"clear", stm_cli_builtin_clear, "clear the terminal screen"},
    {NULL, NULL, NULL}};

static void evhttp_conn_close_cb(UNUSED struct evhttp_connection *conn, UNUSED void *arg)
{
    snprintf(g_stellar_cli_prompt, sizeof(g_stellar_cli_prompt), "not connected>");
}

static void evhttp_request_error_cb(enum evhttp_request_error errnum, void *arg)
{
    (void)arg;
    switch (errnum)
    {
    case EVREQ_HTTP_TIMEOUT:
        g_evh_client->response_code = STM_HTTP_408_REQUEST_TIMEOUT;
        break;
    case EVREQ_HTTP_INVALID_HEADER:
        g_evh_client->response_code = STM_HTTP_403_FORBIDDEN;
        break;
    case EVREQ_HTTP_DATA_TOO_LONG:
        g_evh_client->response_code = STM_HTTP_413_PAYLOAD_TOO_LARGE;
        break;
    case EVREQ_HTTP_EOF:
        break;
    default:
        break;
    }
}

static void evhttp_response_cb(struct evhttp_request *req, void *arg)
{
    (void)arg;
    if (req == NULL)
    {
        return;
    }
    struct evbuffer *input_buffer = evhttp_request_get_input_buffer(req);
    size_t evbuf_len = evbuffer_get_length(input_buffer);
    if (NULL == input_buffer || 0 == evbuf_len)
    {
        return;
    }
    g_evh_client->response_cstr = (char *)calloc(1, evbuf_len + 1);
    evbuffer_remove(input_buffer, g_evh_client->response_cstr, evbuf_len);
    g_evh_client->response_code = STM_HTTP_200_OK;
    // terminate event_base_dispatch()
    event_base_loopbreak(g_evh_client->base);
}

static struct libevent_http_client *evhttp_client_new(const char *server_ip, unsigned short server_port)
{
    struct libevent_http_client *evh_client =
        (struct libevent_http_client *)calloc(1, sizeof(struct libevent_http_client));

    evh_client->base = event_base_new();
    evh_client->conn = evhttp_connection_base_new(evh_client->base, NULL,
                                                  server_ip, server_port);
    evhttp_connection_set_timeout(evh_client->conn, g_stm_cli_connect_timeout);
    return evh_client;
}

static int evhttp_client_request_new(struct libevent_http_client *evh_client)
{
    evh_client->req = evhttp_request_new(evhttp_response_cb, evh_client);
    evhttp_request_set_error_cb(evh_client->req, evhttp_request_error_cb);
    evhttp_connection_set_closecb(evh_client->conn, evhttp_conn_close_cb, evh_client->req);
    evh_client->response_cstr = NULL;
    evh_client->response_code = STM_HTTP_204_NO_CONTENT;
    return 0;
}

static void evhttp_client_add_header(struct libevent_http_client *evh_client, const char *key, const char *value)
{
    struct evkeyvalq *output_headers = evhttp_request_get_output_headers(evh_client->req);
    evhttp_add_header(output_headers, key, value);
}

static void evhttp_client_add_uri(struct libevent_http_client *evh_client,
                                  enum evhttp_cmd_type type, const char *uri)
{
    evhttp_make_request(evh_client->conn, evh_client->req, type, uri);
}

static int default_response_handler(struct libevent_http_client *evh_client)
{
    if (evh_client->response_code != STM_HTTP_200_OK || evh_client->response_cstr == NULL)
    {
        snprintf(g_stellar_cli_prompt, sizeof(g_stellar_cli_prompt), "not connected>");
        fprintf(stderr, "ERR failed to connect to %s:%u\n",
                g_stm_cli_ipaddr_str, g_stm_cli_port_host);
        return -1;
    }

    printf("%s", evh_client->response_cstr);
    fflush(stdout);
    FREE(evh_client->response_cstr);
    snprintf(g_stellar_cli_prompt, sizeof(g_stellar_cli_prompt),
             "cli@%s:%u>", g_stm_cli_ipaddr_str, g_stm_cli_port_host);
    return 0;
}

static int command_json_parse_handler(struct libevent_http_client *evh_client)
{
    if (evh_client->response_code != STM_HTTP_200_OK || evh_client->response_cstr == NULL)
    {
        snprintf(g_stellar_cli_prompt, sizeof(g_stellar_cli_prompt), "not connected>");
        fprintf(stderr, "ERR failed to connect to %s:%u\n",
                g_stm_cli_ipaddr_str, g_stm_cli_port_host);
        return -1;
    }
    if (stm_cmd_assistant_json_load(g_stm_cli_aide, evh_client->response_cstr) < 0)
    {
        fprintf(stderr, "ERR failed to synchronize command info with the monitor server\n");
        return -1;
    }
    FREE(evh_client->response_cstr);
    return 0;
}

static void stm_cli_usage(void)
{
    printf("%s\r\n", g_stellar_monitor_version);
    printf("Usage:\r\n");
    printf("      %s [OPTIONS] [ -e command [arg [arg ...]]]\r\n", "stellar-cli");
    printf("\t%s %-6s %s\r\n", "-i", "--ip", "stellar monitor server ip address");
    printf("\t%s %-6s %s\r\n", "-p", "--port", "stellar monitor server port");
    printf("\t%s %-6s %s\r\n", "-e", "--exec", "non-interactive mode, exit after executing command");
    printf("\t%s %-6s %s\r\n", "-t", "--timeout", "maximum time(sec) allowed for connecting to server");
    exit(0);
}

static int stm_cli_exec_builtin_cmd(struct stm_cmd_parser *cmd_args)
{
    const char *cli_cmd_name = cmd_args->argv[0];
    size_t raw_cmd_len = strlen(cli_cmd_name);
    for (int i = 0; g_stm_cli_builtin_commands[i].cmd_name != NULL; i++)
    {
        if (stm_strncasecmp_exactly(g_stm_cli_builtin_commands[i].cmd_name,
                                    cli_cmd_name, raw_cmd_len) == 0)
        {
            g_stm_cli_builtin_commands[i].cmd_cb(NULL, cmd_args->argc, cmd_args->argv, (void *)cmd_args);
            return 1;
        }
    }
    return 0;
}

static sds stm_cli_build_RESTful_url(struct stm_cmd_parser *cmd_args)
{
    sds url;
    char restful_path[256] = {0};
    snprintf(restful_path, sizeof(restful_path), "/%s/%s", STM_RESTFUL_VERSION, STM_RESTFUL_RESOURCE);
    url = sdsempty();
    url = sdscatprintf(url, "%s?%s=", restful_path, STM_RESTFUL_URI_CMD_KEY);
    for (int i = 0; i < cmd_args->argc; i++)
    {
        url = sdscat(url, cmd_args->argv[i]);
        if (i < cmd_args->argc - 1)
        {
            url = sdscat(url, " "); // add blank space
        }
    }
    char *encoded_url_str = stm_http_url_encode(url);
    sds encoded_url = sdsnew(encoded_url_str);
    sdsfree(url);
    free(encoded_url_str);
    return encoded_url;
}

static int stm_cli_evhttp_run_cmd(struct stm_cmd_parser *cmd_args)
{
    evhttp_client_request_new(g_evh_client);
    evhttp_client_add_header(g_evh_client, "Connection", "keep-alive");
    evhttp_client_add_header(g_evh_client, "Content-Length", "0");
    sds url = stm_cli_build_RESTful_url(cmd_args);
    evhttp_client_add_uri(g_evh_client, EVHTTP_REQ_GET, url);
    int ret = event_base_dispatch(g_evh_client->base);
    sdsfree(url);
    return ret;
}

/* call remote command use RESTful */
static int stm_cli_exec_rpc_cmd(UNUSED const char *raw_cmd_line, struct stm_cmd_parser *cmd_args)
{
    int ret = stm_cli_evhttp_run_cmd(cmd_args);
    g_response_handler(g_evh_client);
    return ret;
}

static struct stm_cmd_parser *stm_cli_parse_cmd_line(const char *line)
{
    struct stm_cmd_parser *cmd_args =
        (struct stm_cmd_parser *)calloc(1, sizeof(struct stm_cmd_parser));
    cmd_args->raw_cmd_line = sdsnew(line);
    cmd_args->argv = sdssplitargs(line, &cmd_args->argc);
    return cmd_args;
}

static void stm_cli_exec_cmd(const char *raw_line)
{
    struct stm_cmd_parser *cmd_args = stm_cli_parse_cmd_line(raw_line);
    if (stm_cli_exec_builtin_cmd(cmd_args))
    {
        goto fun_exit;
    }
    stm_cli_exec_rpc_cmd(raw_line, cmd_args);

fun_exit:
    stm_cli_args_free(cmd_args);
    return;
}

static int stm_cli_builtin_help(const char *line)
{
    int argc = 0;
    int is_help_cmd = 0;
    sds *array = sdssplitargs(line, &argc);

    if (argc != 1)
    {
        sdsfreesplitres(array, argc);
        return 0;
    }
    if ((strcasecmp(array[argc - 1], "help") == 0) || (strcasecmp(array[argc - 1], "--help") == 0) || (strcasecmp(array[argc - 1], "-h") == 0) || (strcasecmp(array[argc - 1], "/?") == 0) || (strcasecmp(array[argc - 1], "?") == 0))
    {
        is_help_cmd = 1;
    }
    if (is_help_cmd == 1)
    {
        stm_cli_exec_cmd("show command brief");
    }
    sdsfreesplitres(array, argc);
    return is_help_cmd;
}

static void stm_cli_register_builtin_cmd(void)
{
    for (int i = 0; g_stm_cli_builtin_commands[i].cmd_name != NULL; i++)
    {
        stm_cmd_assistant_register_cmd(g_stm_cli_aide,
                                       g_stm_cli_builtin_commands[i].cmd_name,
                                       g_stm_cli_builtin_commands[i].cmd_cb,
                                       NULL, "readonly", "<cr>", 
                                       g_stm_cli_builtin_commands[i].description);
    }
}

static void stm_cli_run(void)
{
    char *line;
    /* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
    linenoiseHistoryLoad(g_stellar_monitor_history_file); /* Load the history at startup */
    /* Non-interactive mode */
    if (g_stm_cli_noninteractive)
    {
        stm_cli_exec_cmd(g_stm_cli_noninteractive_cmd_line);
        exit(0);
    }

    /* Synchronize with the monitor server on boot up */
    g_response_handler = command_json_parse_handler;
    stm_cli_exec_cmd(STM_CLIENT_SERVER_SYNC_CMD);
    g_response_handler = default_response_handler;

    /* register builtin command after synchronization */
    stm_cli_register_builtin_cmd();

    /* Interactive mode */
    while (1)
    {
        line = linenoise(g_stellar_cli_prompt);
        if (line && strlen(line) > 0)
        {
            if (stm_cli_builtin_help(line) == 0)
            {
                stm_cli_exec_cmd(line);
            }
            fflush(stdout);
            linenoiseHistoryAdd(line);
        }
        FREE(line);
    }
}

static const char *stm_cli_short_options = "he:i:p:t:";
static const struct option stm_cli_long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"ip", required_argument, NULL, 'i'},
    {"port", required_argument, NULL, 'p'},
    {"exec", required_argument, NULL, 'e'},
    {"timeout", required_argument, NULL, 't'},
};

static int stm_cli_check_args(int argc, char *_argv[])
{
    int c, ret = 0;
    char **argv_tmp = CALLOC(char *, argc + 1);
    for (int i = 0; i < argc; i++)
    {
        argv_tmp[i] = _argv[i];
    }
    while (1)
    {
        c = getopt_long(argc, argv_tmp, stm_cli_short_options, stm_cli_long_options, NULL);
        if (c == -1)
        {
            ret = 0;
            break;
        }
        switch (c)
        {
        case 'h':
            stm_cli_usage();
            break;
        case 'i':
        case 'p':
        case 'e':
        case 't':
            break;
        case '?': /* invalid or unknown option */
            ret = -1;
            break;
        default:
            ret = -1;
            break;
        }
    }
    FREE(argv_tmp);
    return ret;
}

static int stm_cli_evhttp_init(void)
{
    g_evh_client = evhttp_client_new(g_stm_cli_ipaddr_str, g_stm_cli_port_host);
    assert(g_evh_client != NULL);
    return 0;
}

static void cli_linenoise_completion_cb(const char *line, linenoiseCompletions *lc)
{
    stm_cmd_assistant_input_line(g_stm_cli_aide, line, (void *)lc);
}

static char *cli_linenoise_hints_cb(const char *line, int *color, int *bold)
{
    char *hints = (char *)stm_cmd_assistant_input_line_for_hints(g_stm_cli_aide, line);
    if (NULL == hints)
    {
        return NULL;
    }
    sds tmp = sdsnew(" "); // add a blank space before hints, easy to input the next command
    tmp = sdscat(tmp, hints);
    *color = STM_CLI_CMD_HINTS_COLOR;
    *bold = STM_CLI_CMD_HINTS_BOLD;
    return tmp;
}

static void cli_linenoise_free_hints_cb(void *arg)
{
    sdsfree((sds)arg);
}

void cli_assistant_completion_cb(void *arg, const char *candidate_completion)
{
    linenoiseCompletions *lc = (linenoiseCompletions *)arg;
    linenoiseAddCompletion(lc, candidate_completion);
}

static int stm_assistant_init(void)
{
    g_stm_cli_aide = stm_cmd_assistant_new();
    if (NULL == g_stm_cli_aide)
    {
        return -1;
    }
    /* Set the completion callback. This will be called every time the
     * user uses the <tab> key. */
    linenoiseSetCompletionCallback(cli_linenoise_completion_cb);
    stm_cmd_assistant_set_completion_cb(g_stm_cli_aide, cli_assistant_completion_cb);
    linenoiseSetHintsCallback(cli_linenoise_hints_cb);
    linenoiseSetFreeHintsCallback(cli_linenoise_free_hints_cb);
    return 0;
}

static int stm_cli_parse_args(int argc, char *argv[])
{
    if (stm_cli_check_args(argc, argv) < 0)
    {
        return -1;
    }
    if (monitor_util_parse_cmd_args(argc, (const char **)argv, g_cli_args,
                                    sizeof(g_cli_args) / sizeof(struct monitor_cli_args)) < 0)
    {
        return -1;
    }
    if (g_cli_args[0].value != NULL)
    {
        g_stm_cli_ipaddr_str = g_cli_args[0].value;
    }
    int tmp_val;
    if (g_cli_args[1].value != NULL)
    {
        tmp_val = atoi(g_cli_args[1].value);
        if (tmp_val <= 0 || tmp_val > 65535)
        {
            fprintf(stderr, "invalid port: %s\n", g_cli_args[1].value);
            return -1;
        }
        g_stm_cli_port_host = (unsigned short)tmp_val;
    }
    if (g_cli_args[2].value != NULL)
    {
        tmp_val = atoi(g_cli_args[2].value);
        if (tmp_val <= 0)
        {
            fprintf(stderr, "invalid timeout: %s\n", g_cli_args[2].value);
            return -1;
        }
        g_stm_cli_connect_timeout = tmp_val;
    }
    if (g_cli_args[3].value != NULL)
    {
        g_stm_cli_noninteractive = 1;
        g_stm_cli_noninteractive_cmd_line = g_cli_args[3].value;
    }
    snprintf(g_stellar_cli_prompt, sizeof(g_stellar_cli_prompt),
             "cli@%s:%u>", g_stm_cli_ipaddr_str, g_stm_cli_port_host);
    return 0;
}

int main(int argc, char *argv[])
{
    if (stm_cli_parse_args(argc, argv) < 0)
    {
        return -1;
    }
    if (stm_assistant_init() < 0)
    {
        return -1;
    }
    if (stm_cli_evhttp_init() < 0)
    {
        return -1;
    }
    g_response_handler = default_response_handler;
    signal(SIGINT, signal_handler);
    stm_cli_run();
    return 0;
}
