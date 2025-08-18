#include <openssl/ssl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "yt.h"
#include "yt_config.h"

#define MAX_KEY_LEN	31
#define MAX_VAL_LEN	63

int find_any_key_value(char *str, char *key, char *value)
{
    // slightly modified Jonathan Leffler's code from:
    // https://stackoverflow.com/questions/24490410/extracting-key-value-with-scanf-in-c

    char junk[256];
    const char *search = str;

    while (*search != '\0') {
        int offset;
        if (sscanf
            (search, " \"%31[a-zA-Z]\": \"%63[0-9]\"%n", key, value,
             &offset) == 2)
            return (search + offset - str);
        int rc;
        if ((rc = sscanf(search, "%255s%n", junk, &offset)) != 1)
            return EOF;
        search += offset;
    }

    return EOF;
}

int find_key_value(char *str, char *key, char *value)
{
    char found[MAX_KEY_LEN + 1];
    int offset;
    char *search = str;

    while ((offset = find_any_key_value(search, found, value)) > 0) {
        if (strcmp(found, key) == 0)
            return (search + offset - str);
        search += offset;
    }

    return offset;
}

int get_channel_statistics(char **views, char **subs, char **videos)
{
    // minimalist HTTPS client in C on Linux: https://gist.github.com/nir9

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return EXIT_FAILURE;

    const struct sockaddr_in addr = {
        AF_INET,
        htons(443),
        { htonl(0xACD90C6A)	}   // 172.217.12.106 = www.googleapis.com
    };
    if (connect(sockfd, (void *)&addr, sizeof(addr)) < 0)
        return EXIT_FAILURE;

    const struct timeval tv = {
        5,		// 5 seconds
        0		// 0 useconds
    };
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&tv, sizeof(tv));

    SSL_CTX *ctx = SSL_CTX_new(TLS_method());
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    if (SSL_connect(ssl) <= 0) return EXIT_FAILURE;

    // odszyfruj API key i channel ID
    char api_key[128], chan_id[128];
    size_t klen = get_yt_api_key(api_key, sizeof api_key);
    size_t ilen = get_yt_channel_id(chan_id, sizeof chan_id);
    if (!klen || !ilen) return EXIT_FAILURE;

    char request[1024];
    int n = snprintf(request, sizeof(request),
        "GET https://youtube.googleapis.com/youtube/v3/channels?part=statistics&id=%s&key=%s HTTP/1.1\r\n\r\nAccept: application/json\r\n",
        chan_id, api_key);
    if (n <=0 || n >= (int)sizeof(request))
        return EXIT_FAILURE;

    SSL_write(ssl, request, (int)strlen(request));

    // po użyciu - wymaż sekrety z pamięci
    secure_wipe(api_key, klen);
    secure_wipe(chan_id, ilen);

    char response[8192] = { 0 };
    // blocking method will return -1 if nothing is received after 5 seconds
    int rc = SSL_read(ssl, response, (int)sizeof(response) - 1);
    // connection error or timeout
    if (rc <= 0)
        return EXIT_FAILURE;

    // check http status code
    if (response[9] != '2' && response[10] != '0' && response[11] != '0')
        return EXIT_FAILURE;

    static char view_count[MAX_VAL_LEN + 1];
    static char subscriber_count[MAX_VAL_LEN + 1];
    static char video_count[MAX_VAL_LEN + 1];

    find_key_value(response, "viewCount", view_count);
    *views = view_count;

    find_key_value(response, "subscriberCount", subscriber_count);
    *subs = subscriber_count;

    find_key_value(response, "videoCount", video_count);
    *videos = video_count;

    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);

    return EXIT_SUCCESS;
}
