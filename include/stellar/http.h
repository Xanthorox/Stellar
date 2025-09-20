#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C"
{
#endif

#define HTTP_TOPIC "HTTP_MESSAGE"
    struct http_message;

    enum http_message_type
    {
        HTTP_TRANSACTION_START,

        HTTP_MESSAGE_REQ_LINE,
        HTTP_MESSAGE_REQ_HEADER,
        HTTP_MESSAGE_REQ_HEADER_END, // todo, delete END, push all fileds at once
        HTTP_MESSAGE_REQ_BODY_START,
        HTTP_MESSAGE_REQ_BODY,
        HTTP_MESSAGE_REQ_BODY_END,

        HTTP_MESSAGE_RES_LINE,
        HTTP_MESSAGE_RES_HEADER,
        HTTP_MESSAGE_RES_HEADER_END, // todo, delete END, push all fileds at once
        HTTP_MESSAGE_RES_BODY_START,
        HTTP_MESSAGE_RES_BODY,
        HTTP_MESSAGE_RES_BODY_END,

        HTTP_TRANSACTION_END,

        HTTP_MESSAGE_MAX
    };

    struct http_header_field
    {
        char *name;
        size_t name_len;
        char *value;
        size_t value_len;
    };

    struct http_request_line
    {
        char *method;
        size_t method_len;
        char *uri;
        size_t uri_len;
        char *version;
        size_t version_len;

        int major_version;
        int minor_version;
    };

    struct http_response_line
    {
        char *version;
        size_t version_len;
        char *status;
        size_t status_len;
        int major_version;
        int minor_version;
        int status_code;
    };

    enum http_message_type http_message_get_type(const struct http_message *msg);

    void http_message_get0_request_line(const struct http_message *msg, struct http_request_line *line);

    void http_message_get0_response_line(const struct http_message *msg, struct http_response_line *line);

    /*
     *  Pay attention: key->iov_base is case-insensitive.
     */
    void http_message_get0_header(const struct http_message *msg, const char *name, size_t name_len, struct http_header_field *field_result);

    /**
     * @brief loop reading all headers.
     *
     * @retval succeed( >= 0) failed(-1)
     */
    int http_message_get0_next_header(const struct http_message *msg, struct http_header_field *header);

    /**
     * @retval succeed( >= 0) failed(-1)
     */
    int http_message_reset_header_iter(struct http_message *msg); // to do , obsoleted

    void http_message_get0_uncompressed_body(const struct http_message *msg, const char **body, size_t *body_len);

    /**
     * @brief If the body hasn't been compressed, same as http_message_get0_uncompressed_body().
     *
     */
    void http_message_get0_decompressed_body(const struct http_message *msg, const char **body, size_t *body_len);

    void http_message_get0_raw_url(const struct http_message *msg, const char **url, size_t *url_len);

    /*
        return value:
        0: failed
        >1: success, length of decoded_url_buffer, not C string( no EOF with '\0' )
    */
    size_t http_url_decode(const char *raw_url, size_t raw_url_len, char *decoded_url_buffer, size_t decoded_url_buffer_len);

    /**
     * @retval succeed( >= 0) failed(-1)
     */
    int http_message_get_transaction_seq(const struct http_message *msg);

#ifdef __cplusplus
}
#endif
