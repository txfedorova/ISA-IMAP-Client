#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <getopt.h>


#define BUFFER_SIZE 1024
#define DEFAULT_IMAP_PORT 143
#define DEFAULT_IMAPS_PORT 993

typedef struct {
    char *server;
    int port;
    int use_ssl;
    char *auth_file;
    char *mailbox;
    char *out_dir;
    int new_only;
    int headers_only;
} imap_config;

SSL_CTX *ssl_ctx = NULL;
SSL *ssl = NULL;

void parse_arguments(int argc, char *argv[], imap_config *config);

int main(int argc, char *argv[]) {
    imap_config config;
    parse_arguments(argc, argv, &config);
    return 0;
}

void parse_arguments(int argc, char *argv[], imap_config *config) {
    config->port = 0;
    config->use_ssl = 0;
    config->mailbox = "INBOX";
    config->new_only = 0;
    config->headers_only = 0;

    int opt;
    while ((opt = getopt(argc, argv, "p:Tc:C:nh:a:b:o:")) != -1) {
        switch (opt) {
            case 'p':
                config->port = atoi(optarg);
                break;
            case 'T':
                config->use_ssl = 1;
                break;
            case 'a':
                config->auth_file = optarg;
                break;
            case 'b':
                config->mailbox = optarg;
                break;
            case 'o':
                config->out_dir = optarg;
                break;
            case 'n':
                config->new_only = 1;
                break;
            case 'h':
                config->headers_only = 1;
                break;
            default:
                fprintf(stderr, "Usage: imapcl server [-p port] [-T] [-a auth_file] [-b mailbox] -o out_dir\n");
                exit(EXIT_FAILURE);
        }
    }

    // if (optind < argc) {
    //     config->server = argv[optind];
    // } else {
    //     fprintf(stderr, "Server name is required.\n");
    //     exit(EXIT_FAILURE);
    // }
    // if (!config->auth_file || !config->out_dir) {
    //     fprintf(stderr, "Auth file and output directory are required.\n");
    //     exit(EXIT_FAILURE);
    // }
    if (!config->port) {
        config->port = config->use_ssl ? DEFAULT_IMAPS_PORT : DEFAULT_IMAP_PORT;
    }
}
