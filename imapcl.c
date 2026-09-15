/*
 * BUT FIT 2024
 * NETWORK APPLICATIONS AND NETWORK ADMINISTRATION
 * Author: TATIANA FEDOROVA (xfedor14)
 * IMAPCL
 */

#define _XOPEN_SOURCE 700
#include "imapcl.h"


/// @brief program usage
void help() {
    printf("\n\n---------------------------------------IMAPCL---------------------------------------\n\n");
    printf("Program stahne zpravy ulozene na serveru a ulozi je do zadaneho adresare.\n");
    printf("Po spusteni program stahne zpravy a na standardni vystup vypise jejich pocet.\n\n");
    printf("Pouziti:\n");
    printf("./imapcl server [-p port] [-T [-c certfile] [-C certaddr]] [-n] [-h] -a auth_file [-b MAILBOX] -o out_dir\n\n");
    printf("./imapcl eva.fit.vutbr.cz -o maildir -a auth_file.txt");
    printf("Poradi parametru je libovolne.\n\n");
    printf("server\t\tIP adresa nebo domenove jmeno pozadovaneho serveru.\n\n");
    printf("-p port\t\tvvolitelny parametr pro cislo portu na serveru.\n\n");
    printf("-T\t\tzapina sifrovani (imaps), pokud neni uvedeno, pouzije se nesifrovana varianta protokolu.\n\n");
    printf("-c certfile\tvolitelny soubor s certifikaty pro overeni certifikatu SSL/TLS.\n\n");
    printf("-C certaddr\tvolitelny adresar pro vyhledani certifikatu pro overeni certifikatu SSL/TLS.\n\n");
    printf("-n\t\tpracuje pouze s novymi zpravami.\n\n");
    printf("-h\t\tstahuje pouze hlavicky zprav.\n\n");
    printf("-a auth_file\todkaz na soubor s autentizacnimi udaji (prikaz LOGIN).\n\n");
    printf("-b MAILBOX\tvolitelny parametr pro nazev schranky (vychozi je INBOX).\n\n");
    printf("-o out_dir\turcuje vystupni adresar pro ulozeni stazenych zprav.\n\n");
}

/// @brief free the memory and return program to initialized state 
/// @param imapcl program structure
void destroy(IMAPCL *imapcl) {
    
    // clean program arguments
    if(imapcl->arguments.feedfile)
        free(imapcl->arguments.feedfile);
    if(imapcl->arguments.certfile)
        free(imapcl->arguments.certfile);
    if(imapcl->arguments.certaddr)
        free(imapcl->arguments.certaddr);
    if(imapcl->arguments.auth_file)
        free(imapcl->arguments.auth_file);
    if(imapcl->arguments.mailbox)
        free(imapcl->arguments.mailbox);
    if(imapcl->arguments.out_dir)
        free(imapcl->arguments.out_dir);

    imapcl->arguments.feedfile = NULL;
    imapcl->arguments.certfile = NULL;
    imapcl->arguments.certaddr = NULL;
    imapcl->arguments.auth_file = NULL;
    imapcl->arguments.mailbox = NULL;
    imapcl->arguments.out_dir = NULL;

    imapcl->arguments.t_flag = 0;
    imapcl->arguments.a_flag = 0;
    imapcl->arguments.n_flag = 0;
    imapcl->arguments.h_flag = 0;
    imapcl->arguments.p_flag = 0;
    imapcl->arguments.b_flag = 0;
    imapcl->arguments.o_flag = 0;

    if(imapcl->arguments.file) {
        fclose(imapcl->arguments.file);
    }
}

/// @brief process error, output error message to stderr and continue or stop the program
/// @param error_msg error message 
/// @param error_type error type 
void process_error(char *error_msg, int error_type, IMAPCL *imapcl) {
    switch (error_type)
    {
    case ARGUMENT_ERROR:
        fprintf(stderr, "%s", error_msg);
        destroy(imapcl);
        // help();
        exit(error_type);
        break;
    case ALLOCATION_ERROR:
    case CONNECTION_ERROR:
        fprintf(stderr, "%s", error_msg);
        destroy(imapcl);
        // help();
        exit(error_type);
        break;
    default:
        break;
    }
}

/// @brief initialize program variables and structures
/// @param imapcl program structure 
void init(IMAPCL *imapcl) {
    
    // initialize program arguments
    imapcl->arguments.feedfile = NULL;
    imapcl->arguments.certfile = NULL;
    imapcl->arguments.certaddr = NULL;
    imapcl->arguments.auth_file = NULL;
    imapcl->arguments.mailbox = "INBOX";
    imapcl->arguments.out_dir = NULL;

    imapcl->arguments.t_flag = 0;
    imapcl->arguments.a_flag = 0;

    imapcl->arguments.n_flag = 0;
    imapcl->arguments.h_flag = 0;
    imapcl->arguments.p_flag = 0;
    imapcl->arguments.b_flag = 0;
    imapcl->arguments.o_flag = 0;

    imapcl->arguments.file = NULL;
  
    imapcl->communication.bio = NULL;
    imapcl->communication.ctx = NULL;
    imapcl->communication.ssl = NULL;
    imapcl->communication.request = NULL;
    imapcl->communication.response = NULL;

}

/// @brief parse program arguments and fill program structure
/// @param argc amount of arguments 
/// @param argv array of program arguments strings
/// @param imapcl program structure
void parse_program_arguments(int argc, char **argv, IMAPCL *imapcl) {

    if(argc < 2) {
        process_error("ERROR: wrong format of arguments\n", ARGUMENT_ERROR, imapcl);
    }

    int argument;
    // disable getopt error output 
    opterr = 0;
    imapcl->arguments.port = 143;
    // process optional arguments
    while((argument = getopt_long(argc, argv, "p:Tc:C:nha:b:o:", NULL, NULL)) != -1) {
        // check option flag
        switch(argument) {
            case 'p':
                // check if it is the first usage of the argument
                if((imapcl->arguments.port != 993) || (imapcl->arguments.port != 143))
                    process_error("ERROR: -p flag argument must be set only once! \n", ARGUMENT_ERROR, imapcl);
                if (optarg == NULL) {
                    if (imapcl->arguments.t_flag == 0) {
                        imapcl->arguments.port = 143;
                    }
                    else {
                        imapcl->arguments.port = 993;
                    }
                } else {
                    char *endptr;
                    imapcl->arguments.port = strtol(optarg, &endptr, 10);
                    if (*endptr != '\0' || imapcl->arguments.port <= 0) {
                        process_error("ERROR: -p flag requires a valid positive integer value\n", ARGUMENT_ERROR, imapcl);
                    }
                }
                imapcl->arguments.p_flag = 1;
                break;
            case 'T':
                if(imapcl->arguments.t_flag)
                    process_error("ERROR: -T flag argument must be set only once!\n", ARGUMENT_ERROR, imapcl);
                    if (imapcl->arguments.port == 143)
                        imapcl->arguments.port = 993;
                imapcl->arguments.t_flag = 1;
                break;
            case 'c':
                if(imapcl->arguments.certfile)
                    process_error("ERROR: certfile must be set only once!\n", ARGUMENT_ERROR, imapcl);
                if (optarg == NULL) {
                    process_error("ERROR: -c flag requires a valid argument (authentication file)\n", ARGUMENT_ERROR, imapcl);
                }
                if (!isalpha((unsigned char)optarg[0])) {
                    process_error("ERROR: The argument for -c must start with a letter\n", ARGUMENT_ERROR, imapcl);
                } 
                imapcl->arguments.certfile = malloc(strlen(optarg) + 1);
                if(!imapcl->arguments.certfile)
                    process_error("ERROR: can't allocate the memory\n", ALLOCATION_ERROR, imapcl);
                strcpy(imapcl->arguments.certfile, optarg);
                imapcl->arguments.cf_flag = 1;
                break;
            case 'C':
                if(imapcl->arguments.certaddr)
                    process_error("ERROR: certaddr must be set only once!\n", ARGUMENT_ERROR, imapcl);
                imapcl->arguments.certaddr = malloc(strlen(optarg) + 1);
                if(!imapcl->arguments.certaddr)
                    process_error("ERROR: can't allocate the memory\n", ALLOCATION_ERROR, imapcl);
                strcpy(imapcl->arguments.certaddr, optarg);
                imapcl->arguments.ca_flag = 1;
                break;
            case 'n':
                if(imapcl->arguments.n_flag)
                    process_error("ERROR: -n flag argument must be set only once!\n", ARGUMENT_ERROR, imapcl);
                imapcl->arguments.n_flag = 1;
                break;
            case 'h':
                if(imapcl->arguments.n_flag)
                    process_error("ERROR: -h flag argument must be set only once!\n", ARGUMENT_ERROR, imapcl);
                imapcl->arguments.h_flag = 1;
                break;
            case 'a':
                if(imapcl->arguments.auth_file)
                    process_error("ERROR: -a flag argument must be set only once!\n", ARGUMENT_ERROR, imapcl);
                if (optarg == NULL) {
                    process_error("ERROR: -a flag requires a valid argument (authentication file)\n", ARGUMENT_ERROR, imapcl);
                }
                if (!isalpha((unsigned char)optarg[0])) {
                    process_error("ERROR: The argument for -a must start with a letter\n", ARGUMENT_ERROR, imapcl);
                }  
                imapcl->arguments.auth_file = malloc(strlen(optarg) + 1);
                if(!imapcl->arguments.auth_file)
                    process_error("ERROR: can't allocate the memory\n", ALLOCATION_ERROR, imapcl);
                strcpy(imapcl->arguments.auth_file, optarg);
                imapcl->arguments.a_flag = 1;
                break;
            case 'b':
                if(imapcl->arguments.mailbox && imapcl->arguments.mailbox!="INBOX")
                    process_error("ERROR: -b flag argument must be set only once!\n", ARGUMENT_ERROR, imapcl);
                if (optarg == NULL) {
                    process_error("ERROR: -b flag requires a valid argument (mailbox name)\n", ARGUMENT_ERROR, imapcl);
                }
                if (!isalpha((unsigned char)optarg[0])) {
                    process_error("ERROR: The argument for -b must start with a letter.\n", ARGUMENT_ERROR, imapcl);
                }
                imapcl->arguments.mailbox = malloc(strlen(optarg) + 1);
                if(!imapcl->arguments.mailbox)
                    process_error("ERROR: can't allocate the memory\n", ALLOCATION_ERROR, imapcl);
                strcpy(imapcl->arguments.mailbox, optarg);
                imapcl->arguments.b_flag = 1;
                break;
            case 'o':
                if(imapcl->arguments.out_dir)
                    process_error("ERROR: -o flag argument must be set only once!\n", ARGUMENT_ERROR, imapcl);
                if (optarg == NULL) {
                    process_error("ERROR: -o flag requires a valid argument (out_dir directory)\n", ARGUMENT_ERROR, imapcl);
                }
                if (!isalpha((unsigned char)optarg[0])) {
                    process_error("ERROR: The argument for -o must start with a letter\n", ARGUMENT_ERROR, imapcl);
                }
                imapcl->arguments.out_dir = malloc(strlen(optarg) + 1); 
                if(!imapcl->arguments.out_dir)
                    process_error("ERROR: can't allocate the memory\n", ALLOCATION_ERROR, imapcl);
                strcpy(imapcl->arguments.out_dir, optarg);
                imapcl->arguments.o_flag = 1;
                break;
            case '?':
                if(optopt == 'c' || optopt == 'C' || optopt == 'a' || optopt == 'b' || optopt == 'o' || optopt == 'p')
                    fprintf(stderr, "ERROR: %c doesn't have argument value\n", optopt);
                else 
                    fprintf(stderr, "ERROR: unknown argument\n");
                help();
                exit(ARGUMENT_ERROR);
                break;
        }
    }
    
    // check if server is set, and process it
    if (optind < argc) {
        imapcl->arguments.server = argv[optind];

        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;  // Support both IPv4 and IPv6
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(imapcl->arguments.server, NULL, &hints, &res) != 0) {
            process_error("ERROR: Failed to resolve server address\n", ARGUMENT_ERROR, imapcl);
        }
        freeaddrinfo(res);
    } else {
        process_error("ERROR: Server address is missing!\n", ARGUMENT_ERROR, imapcl);
    }

    // process non-optional arguments
    if(optind < argc) {
        while(optind < argc) {
            // check if feedfile argument is not set
            if(imapcl->arguments.feedfile)
                    process_error("ERROR: only one option must be used (feedfile or URL)!\n", ARGUMENT_ERROR, imapcl);
            optind++;
        }
    }
}

void process_auth_file(IMAPCL *imapcl) {
    imapcl->arguments.file = fopen(imapcl->arguments.auth_file, "r");
    if (!imapcl->arguments.file) {
        perror("Failed to open file");
        process_error("ERROR: can't open the file\n", ARGUMENT_ERROR, imapcl);
    }

    char line[256];

    imapcl->arguments.username = malloc(256); 
    imapcl->arguments.password = malloc(256);
    if (!imapcl->arguments.username || !imapcl->arguments.password) {
        process_error("ERROR: memory allocation failed\n", ALLOCATION_ERROR, imapcl);
    }

    while (fgets(line, sizeof(line), imapcl->arguments.file)) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) {
            continue;
        }

        if (strncmp(line, "username =", 10) == 0) {
            char *username_value = strtok(line + 11, " \t");
            if (username_value) {
                strncpy(imapcl->arguments.username, username_value, 255);
                imapcl->arguments.username[255] = '\0';
            }
            break;
        }
    }

    while (fgets(line, sizeof(line), imapcl->arguments.file)) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) {
            continue;
        }

        if (strncmp(line, "password =", 10) == 0) {
            char *password_value = strtok(line + 11, " \t");
            if (password_value) {
                strncpy(imapcl->arguments.password, password_value, 255);
                imapcl->arguments.password[255] = '\0';
            }
            break;
        }
    }

    fclose(imapcl->arguments.file);

    if (strlen(imapcl->arguments.username) == 0 || strlen(imapcl->arguments.password) == 0) {
        process_error("ERROR: login or password is empty\n", ARGUMENT_ERROR, imapcl);
    }
}

void write_request(BIO *bio, char *request) {
    while (BIO_write(bio, request, strlen(request)) <= 0)
    {
        if (! BIO_should_retry(bio)){
            fprintf(stderr, "%s", "Could not sent data to the server\n");
            // TODO DESTROY
            printf("Could not sent data to the server");
            help();
            exit(12);
            break;
        }
    }
}

void extract_plain_text_body(const char *response, FILE *f) {
    char line[1024];
    int in_plain_text_part = 0;
    int body_started = 0;
    char body[2048];
    int body_index = 0;

    char buffer[2048];
    int buffer_index = 0;

    // Time tracking for timeout
    time_t start_time = time(NULL);
    int timeout_seconds = 10;  // Timeout after 10 seconds

    const char *start = response;
    int found_close_paren = 0;  // flag closing parenthesis ')'

    while (*start) {
        // Check for timeout
        time_t current_time = time(NULL);
        if (current_time - start_time > timeout_seconds) {
            fprintf(stderr, "Error: Program timed out.\n");
            exit(1);  // Exit with error if timed out
        }

        // find the end of the current line (newline or end of string)
        const char *end = strchr(start, '\n');
        if (end == NULL) {
            end = start + strlen(start);
        }

        // copy the line into the buffer
        int line_length = end - start;
        if (line_length >= sizeof(line)) {
            line_length = sizeof(line) - 1;  // overflow
        }
        strncpy(line, start, line_length);
        line[line_length] = '\0';

        // find the line with '}'
        if (strstr(line, "}") != NULL) {
            start = end + 1;

            // next line for boundary '--'
            const char *next_line_start = start;
            const char *next_line_end = strchr(next_line_start, '\n');
            if (next_line_end == NULL) {
                next_line_end = next_line_start + strlen(next_line_start); //no newline
            }

            // copy the next line into a local buffer with size checks
            char next_line[1024];
            int next_line_length = next_line_end - next_line_start;
            if (next_line_length >= sizeof(next_line)) {
                next_line_length = sizeof(next_line) - 1;  // overflow
            }
            strncpy(next_line, next_line_start, next_line_length);
            next_line[next_line_length] = '\0';

            // if the next line starts with '--' save it in the buffer
            if (next_line[0] == '-' && next_line[1] == '-') {
                // save the boundary line in buffer
                int space_left = sizeof(buffer) - buffer_index;
                int copy_length = strlen(next_line) + 1;
                if (space_left >= copy_length) {
                    snprintf(buffer + buffer_index, space_left, "%s\n", next_line);
                    buffer_index += copy_length;
                }
                start = next_line_end + 1;
                continue;
            } else {
                // collect the subsequent lines until we find ')'
                while (*next_line_start != '\0') {
                    if (*next_line_start == ')') {
                        found_close_paren = 1;
                        break;
                    }

                    // save line in buffer with bounds checks
                    int space_left = sizeof(buffer) - buffer_index;
                    int copy_length = strlen(next_line) + 1;
                    if (space_left >= copy_length) {
                        snprintf(buffer + buffer_index, space_left, "%s\n", next_line);
                        buffer_index += copy_length;
                    }

                    next_line_start = next_line_end + 1;
                    next_line_end = strchr(next_line_start, '\n');
                    if (next_line_end == NULL) {
                        next_line_end = next_line_start + strlen(next_line_start);
                    }

                    // ensure we dont overflow the next_line buffer
                    int next_line_length = next_line_end - next_line_start;
                    if (next_line_length >= sizeof(next_line)) {
                        next_line_length = sizeof(next_line) - 1;
                    }
                    strncpy(next_line, next_line_start, next_line_length);
                    next_line[next_line_length] = '\0';
                }
                start = next_line_start;
            }
            continue;
        }

        // found the "Content-Type: text/plain;" header
        if (strstr(line, "Content-Type: text/plain") != NULL) {
            in_plain_text_part = 1;
            start = end + 1;
            continue;
        }

        if (in_plain_text_part == 0 && buffer_index > 0) {
            // write the buffered content to the file
            if (buffer_index < sizeof(buffer)) {
                fwrite(buffer, 1, buffer_index, f);
            }
            buffer_index = 0;
        }

        // accumulate the body
        if (in_plain_text_part == 1) {
            if (strcmp(line, "\r\n") == 0 || strcmp(line, "\n") == 0) {
                start = end + 1;  // skip the empty line
                continue;
            }
    
            // stop when a boundary line is found
            if (line[0] == '-' && line[1] == '-') {
                break;
            }

            // body content with bounds checks
            
            int space_left = sizeof(body) - body_index;
            int copy_length = strlen(line);
            if (space_left > copy_length) {
                strncpy(body + body_index, line, copy_length);
                body_index += copy_length;
            }
        }

        // if we have completed collecting a section and didn't find ')' skip saving
        if (found_close_paren == 0 && buffer_index > 0) {
            buffer_index = 0;
        }

        // next line in the response
        start = end + 1;
    }

    // null the body content and write it to the file
    body[body_index] = '\0';
    fprintf(f, "%s", body);
}


void extract_headers(const char *response, FILE *f) {
    char line[1024];
    int in_plain_text_part = 0;
    int body_started = 0;
    char body[2048];
    int body_index = 0;

    // response line by line
    const char *start = response;
    while (*start) {
        // end of the current line 
        const char *end = strchr(start, '\n');
        if (end == NULL) {
            end = start + strlen(start); 
        }

        // copy the line
        strncpy(line, start, end - start);
        line[end - start] = '\0';

        // found the "Content-Type: text/plain;" header
        if (strstr(line, "From:") != NULL) {
            in_plain_text_part = 1;
            start = end + 1; 
        }

        // accumulate the body
        if (in_plain_text_part) {
            // skip the empty line (CRLF between headers and body)
            if (strcmp(line, "\r\n") == 0 || strcmp(line, "\n") == 0) {
                start = end + 1;
                continue;
            }
            
            // stop when a boundary line is found
            if (line[0] == '\r') {
                break;
            }

            // body content
            strncpy(body + body_index, line, sizeof(line) - 1);
            body_index += strlen(line);
        }

        // next line in the response
        start = end + 1;
    }

    // Null the body content and write it to the file
    body[body_index] = '\0';
    fprintf(f, "%s", body);
}

void extract_ids(char *response, IMAPCL *imapcl) {
    const char *search_start = strstr(response, "* SEARCH");
    search_start += strlen("* SEARCH");
    while (*search_start == ' ' || *search_start == '\t') {
        search_start++;
    }
    // printf("Start of the first ID: %s\n", search_start);
    const char *line_end = strstr(search_start, "\r\n");
    int num_index;
    char num_str[10];
    imapcl->arguments.ids_size = 0;
    imapcl->arguments.ids = NULL;
    while (*search_start != '\r') {
        // skip any spaces
        if (*search_start == ' ') {
            search_start++;
            continue;
        }

        num_index = 0;
        // if the current character is a digit extract the number
        while (isdigit(*search_start)) {
            num_str[num_index++] = *search_start++;
        }
        num_str[num_index] = '\0';
        imapcl->arguments.ids_size++;
        imapcl->arguments.ids = realloc(imapcl->arguments.ids, sizeof(char*) * imapcl->arguments.ids_size);
        if (imapcl->arguments.ids == NULL) {
            perror("Memory allocation failed");
            return;
        }

        // allocate memory for the new ID string and copy the ID
        imapcl->arguments.ids[imapcl->arguments.ids_size - 1] = malloc(strlen(num_str) + 1);
        if (imapcl->arguments.ids[imapcl->arguments.ids_size - 1] == NULL) {
            perror("Memory allocation failed");
            return;
        }
        strcpy(imapcl->arguments.ids[imapcl->arguments.ids_size - 1], num_str);
        // printf("Extracted ID: %s\n", num_str);
        if(imapcl->arguments.ids_size >0 ){
            if (*search_start == '\0' || *search_start == '\r') {
                printf("Downloaded %d messages\n", imapcl->arguments.ids_size);
            }
        }
        
    }
}

void read_response(BIO *bio, char *out_dir, const char* id, int flag, FILE *f, IMAPCL *imapcl) {
    // int read_finished = 0;
    char read_buffer[1024];
    int content_length = 0;
    int response_length = 1;
    char *response = malloc(sizeof(char));
    response[0] = '\0';
    while((content_length = BIO_read(bio, read_buffer, 1024))){
        response_length += content_length;
        // get content from the read buffer
        char *content = malloc(content_length + 1);
        strncpy(content, read_buffer, content_length);
        content[content_length] = '\0';

        // concatenate content with response string
        response = realloc(response, sizeof(char) * response_length);
        strcat(response, content);
        free(content);
        if(strstr(response, id) != NULL) {
            break;
        }
    }
    
    if (flag == 1) {
        // printf("RESPONSE: %s\n", response);
        
        extract_plain_text_body(response, f);
    
    } else if (flag == 2) {
        // printf("RESPONSE: %s\n", response);
        // fprintf(f, "%s", response);
        extract_headers(response, f);
    } else if (flag == 3) {
        // printf("RESPONSE: %s\n", response);
        extract_ids(response, imapcl);
    }
    free(response);
}

void connect_common(IMAPCL *imapcl,const char *port) {
    if (imapcl->arguments.t_flag) {
        SSL_library_init();
        imapcl->communication.ctx = SSL_CTX_new(SSLv23_client_method());

        if((!imapcl->arguments.cf_flag) && (!imapcl->arguments.ca_flag)){
            if (!SSL_CTX_set_default_verify_paths(imapcl->communication.ctx)) {
                process_error("ERROR: can't verify certificate\n", CONNECTION_ERROR, imapcl);
            }
        }
        else {
            if(!SSL_CTX_load_verify_locations(imapcl->communication.ctx, imapcl->arguments.certfile, imapcl->arguments.certaddr)) {
                process_error("ERROR: can't verify certificate: ", CONNECTION_ERROR, imapcl);
            }
        }

        imapcl->communication.bio = BIO_new_ssl_connect(imapcl->communication.ctx);
        BIO_get_ssl(imapcl->communication.bio, &imapcl->communication.ssl);
        SSL_set_mode(imapcl->communication.ssl, SSL_MODE_AUTO_RETRY);

        char connection_str[256];
        snprintf(connection_str, sizeof(connection_str), "%s:%s", imapcl->arguments.server, port);
        BIO_set_conn_hostname(imapcl->communication.bio, connection_str);

        if (BIO_do_connect(imapcl->communication.bio) <= 0) {
            process_error("ERROR: can't open connection\n", CONNECTION_ERROR, imapcl);
        }
    } else {
        char connection_str[256];
        snprintf(connection_str, sizeof(connection_str), "%s:%s", imapcl->arguments.server, port);
        imapcl->communication.bio = BIO_new_connect(connection_str);
        if (!imapcl->communication.bio || BIO_do_connect(imapcl->communication.bio) <= 0) {
            process_error("ERROR: can't open connection\n", CONNECTION_ERROR, imapcl);
        }
    }
}

void login_and_select(IMAPCL *imapcl) {

    size_t login_str_len = strlen("A1 LOGIN ") + strlen(imapcl->arguments.username) + strlen(" ") + strlen(imapcl->arguments.password) + strlen("\r\n") + 1;
    char *login_str = malloc(login_str_len);
    if (login_str == NULL) {
        process_error("ERROR: Unable to allocate memory for login string", ALLOCATION_ERROR, imapcl);
    }
    snprintf(login_str, login_str_len, "A1 LOGIN %s %s\r\n", imapcl->arguments.username, imapcl->arguments.password);
    // printf("Generated login string: %s\n", login_str);
    write_request(imapcl->communication.bio, login_str);
    read_response(imapcl->communication.bio, imapcl->arguments.out_dir, "A1", 0, NULL, NULL);
    free(login_str);

    size_t select_str_len = strlen("A2 SELECT ") + strlen(imapcl->arguments.mailbox) + strlen("\r\n") + 1;
    char *select_str = malloc(select_str_len);
    if (select_str == NULL) {
        process_error("ERROR: Unable to allocate memory", ALLOCATION_ERROR, imapcl);
    }
    snprintf(select_str, select_str_len, "A2 SELECT %s\r\n", imapcl->arguments.mailbox);
    // printf("Generated select string: %s\n", select_str);
    write_request(imapcl->communication.bio, select_str);
    read_response(imapcl->communication.bio, imapcl->arguments.out_dir, "A2", 0, NULL, NULL);
    free(select_str);

    char *search_str;
    if (imapcl->arguments.n_flag) {
        search_str = "A3 SEARCH UNSEEN \r\n";
    } else {
        search_str = "A3 SEARCH ALL\r\n";
    }
    write_request(imapcl->communication.bio, search_str);
    read_response(imapcl->communication.bio, imapcl->arguments.out_dir, "A3", 3, NULL, imapcl);
    // printf("\n");
    for (int i = 0; i < imapcl->arguments.ids_size; i++) {
        // printf("%s\n", imapcl->arguments.ids[i]);
        char file_path[128];
        snprintf(file_path, sizeof(file_path), "%s/email_%s.txt", imapcl->arguments.out_dir, imapcl->arguments.ids[i]);
        FILE *file = fopen(file_path, "w");
        if (!file) {
            perror("Error opening file to save plain text body");
            return;
        }

        size_t fetch_str_len = strlen("A4 FETCH ") + strlen(imapcl->arguments.ids[i]) + strlen(" (BODY.PEEK[HEADER.FIELDS (DATE FROM TO SUBJECT MESSAGE-ID)])\r\n") + 1;
        char *fetch_str = malloc(fetch_str_len);
        if (fetch_str == NULL) {
            process_error("ERROR: Unable to allocate memory for fetch string", ALLOCATION_ERROR, imapcl);
        }
        snprintf(fetch_str, fetch_str_len, "A4 FETCH %s (BODY.PEEK[HEADER.FIELDS (DATE FROM TO SUBJECT MESSAGE-ID)])\r\n", imapcl->arguments.ids[i]);

        // char *fetch_str = "A4 FETCH 1 (BODY.PEEK[HEADER.FIELDS (DATE FROM TO SUBJECT MESSAGE-ID)])\r\n";
        // printf("Fetching message %s...\n", imapcl->arguments.ids[i]);
        write_request(imapcl->communication.bio, fetch_str);
        read_response(imapcl->communication.bio, imapcl->arguments.out_dir, "A4", 2, file, NULL);
        free(fetch_str);
        if(!imapcl->arguments.h_flag){
            size_t fetch_str_len = strlen("A4 FETCH ") + strlen(imapcl->arguments.ids[i]) + strlen(" BODY.PEEK[TEXT]\r\n") + 1;
            char *fetch_str = malloc(fetch_str_len);
            if (fetch_str == NULL) {
                process_error("ERROR: Unable to allocate memory for fetch string", ALLOCATION_ERROR, imapcl);
            }
            snprintf(fetch_str, fetch_str_len, "A4 FETCH %s BODY.PEEK[TEXT]\r\n", imapcl->arguments.ids[i]);
            // char *fetch_str = "A4 FETCH 1 BODY.PEEK[TEXT]\r\n";
            write_request(imapcl->communication.bio, fetch_str);
            read_response(imapcl->communication.bio, imapcl->arguments.out_dir, "A4", 1, file, NULL);
            free(fetch_str);
        }
        fclose(file);
    }
}

void connect_with_ssl(IMAPCL *imapcl) {
    connect_common(imapcl, "993");
    login_and_select(imapcl);
}

void connect_unsec(IMAPCL *imapcl) {
    connect_common(imapcl, "143");
    login_and_select(imapcl);
}

void write_to_file(const char *filename, const char *content) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("ERROR: Could not open file for writing");
        exit(1);
    }
    fprintf(file, "%s", content);
    fclose(file);
}

int main(int argc, char *argv[]) {
    IMAPCL imapcl;
    init(&imapcl);
    parse_program_arguments(argc, argv, &imapcl);
    if(imapcl.arguments.auth_file)
        process_auth_file(&imapcl);

    if(imapcl.arguments.t_flag) {
         connect_with_ssl(&imapcl);
    } else {
        connect_unsec(&imapcl);
    }

    // printf("---------------ARGUMENTS---------------\n");
    // printf("SERVER: %s\n", imapcl.arguments.server);
    // printf("USERNAME: %s\n", imapcl.arguments.username);
    // printf("PASSWORD: %s\n", imapcl.arguments.password);
    // printf("----------------------------------------\n");
    // printf("SSL flag: %d\n", imapcl.arguments.t_flag);
    // printf("UNREAD flag: %d\n", imapcl.arguments.n_flag);
    // printf("HEADER flag: %d\n", imapcl.arguments.h_flag);
    // printf("----------------------------------------\n");
    // printf("AUTH flag: %d\n", imapcl.arguments.a_flag);
    // printf("AUTH: %s\n", imapcl.arguments.auth_file);
    // printf("PORT flag: %d\n", imapcl.arguments.p_flag);
    // printf("PORT: %d\n", imapcl.arguments.port);
    // printf("MAILDIR flag: %d\n", imapcl.arguments.b_flag);
    // printf("MAILBOX: %s\n", imapcl.arguments.mailbox);
    // printf("OUTDIR flag: %d\n", imapcl.arguments.o_flag);
    // printf("OUTDIR: %s\n", imapcl.arguments.out_dir);
    // printf("CERFILE flag: %d\n", imapcl.arguments.cf_flag);
    // printf("CERFILE: %s\n", imapcl.arguments.certfile);
    // printf("CERADDR flag: %d\n", imapcl.arguments.ca_flag);
    // printf("CERADDR: %s\n", imapcl.arguments.certaddr);
    // printf("----------------------------------------\n");

    return 0;
}
