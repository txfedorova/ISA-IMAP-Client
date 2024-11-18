#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <netdb.h>
#include <openssl/bio.h>
#include <getopt.h>

// error types
#define ARGUMENT_ERROR 10
#define ALLOCATION_ERROR 11
#define CONNECTION_ERROR 12
/// @brief program arguments structure
typedef struct Arguments {
    char *feedfile; // path to the feedfile 
    char *certfile; // name of the file with certificates 
    char *certaddr; // path to the directory with certificates
    char *auth_file;
    char *mailbox;
    char *out_dir;
    int t_flag;     // ssl/tls flag
    int a_flag;     // redcor author/email flag 
    int u_flag;     // record URL flag
    int n_flag;
    int h_flag;
    int p_flag;
    int port;       // 
    FILE *file;     
}Arguments;

typedef struct Communication {
    BIO *bio;              // BIO object pointer
    SSL_CTX *ctx;          // SSL information structure pointer
    SSL *ssl;              // SSL connection structure pointer
    char *request;         // request text
    char *response;        // response text
    int response_length;   // length of the response
}Communication;

/// @brief program structure
typedef struct IMAPCL {
    Arguments arguments;                   // arguments structure
    Communication communication;           // communication structure

}IMAPCL;
