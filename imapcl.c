#define _XOPEN_SOURCE 700
#include "imapcl.h"


/// @brief program usage
void help() {
    printf("IMAPCL\n\n");
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
    imapcl->arguments.u_flag = 0;
    imapcl->arguments.n_flag = 0;
    imapcl->arguments.h_flag = 0;
    imapcl->arguments.p_flag = 0;

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
        help();
        exit(error_type);
        break;
    case ALLOCATION_ERROR:
    case CONNECTION_ERROR:
        fprintf(stderr, "%s", error_msg);
        destroy(imapcl);
        help();
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
    imapcl->arguments.mailbox = NULL;
    imapcl->arguments.out_dir = NULL;

    imapcl->arguments.t_flag = 0;
    imapcl->arguments.a_flag = 0;
    imapcl->arguments.u_flag = 0;

    imapcl->arguments.n_flag = 0;
    imapcl->arguments.h_flag = 0;
    imapcl->arguments.p_flag = 0;

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
    // process optional arguments
    while((argument = getopt_long(argc, argv, "p:Tc:C:nha:b:o:", NULL, NULL)) != -1) {
        // check option flag
        switch(argument) {
            case 'p':
                // check if it is the first usage of the argument
                if(imapcl->arguments.port)
                    process_error("ERROR: -p flag argument must be set only once!\n", ARGUMENT_ERROR, imapcl);
                imapcl->arguments.port = atoi(optarg);
                break;
            case 'T':
                // check if it is the first usage of the argument
                if(imapcl->arguments.t_flag)
                    process_error("ERROR: -T flag argument must be set only once!\n", ARGUMENT_ERROR, imapcl);
                imapcl->arguments.t_flag = 1;
                break;
            case 'c':
                // check if it is the first usage of the argument
                if(imapcl->arguments.certfile)
                    process_error("ERROR: certfile must be set only once!\n", ARGUMENT_ERROR, imapcl);
                imapcl->arguments.certfile = malloc(strlen(optarg) + 1);
                if(!imapcl->arguments.certfile)
                    process_error("ERROR: can't allocate the memory\n", ALLOCATION_ERROR, imapcl);
                strcpy(imapcl->arguments.certfile, optarg);
                break;
            case 'C':
                // check if it is the first usage of the argument
                if(imapcl->arguments.certaddr)
                    process_error("ERROR: certaddr must be set only once!\n", ARGUMENT_ERROR, imapcl);
                imapcl->arguments.certaddr = malloc(strlen(optarg) + 1);
                if(!imapcl->arguments.certaddr)
                    process_error("ERROR: can't allocate the memory\n", ALLOCATION_ERROR, imapcl);
                strcpy(imapcl->arguments.certaddr, optarg);
                break;
            case 'n':
                // check if it is the first usage of the argument
                if(imapcl->arguments.n_flag)
                    process_error("ERROR: -n flag argument must be set only once!\n", ARGUMENT_ERROR, imapcl);
                imapcl->arguments.n_flag = 1;
                break;
            case 'h':
                // check if it is the first usage of the argument
                if(imapcl->arguments.n_flag)
                    process_error("ERROR: -h flag argument must be set only once!\n", ARGUMENT_ERROR, imapcl);
                imapcl->arguments.h_flag = 1;
                break;
            case 'a':
                // check if it is the first usage of the argument
                if(imapcl->arguments.auth_file)
                    process_error("ERROR: -a flag argument must be set only once!\n", ARGUMENT_ERROR, imapcl);
                imapcl->arguments.auth_file = malloc(strlen(optarg) + 1);
                if(!imapcl->arguments.auth_file)
                    process_error("ERROR: can't allocate the memory\n", ALLOCATION_ERROR, imapcl);
                strcpy(imapcl->arguments.auth_file, optarg);
                break;
            case 'b':
                // check if it is the first usage of the argument
                if(imapcl->arguments.mailbox)
                    process_error("ERROR: -b flag argument must be set only once!\n", ARGUMENT_ERROR, imapcl);
                imapcl->arguments.mailbox = malloc(strlen(optarg) + 1);
                if(!imapcl->arguments.mailbox)
                    process_error("ERROR: can't allocate the memory\n", ALLOCATION_ERROR, imapcl);
                strcpy(imapcl->arguments.mailbox, optarg);
                break;
            case 'o':
                // check if it is the first usage of the argument
                if(imapcl->arguments.out_dir)
                    process_error("ERROR: -o flag argument must be set only once!\n", ARGUMENT_ERROR, imapcl);
                imapcl->arguments.out_dir = malloc(strlen(optarg) + 1);
                if(!imapcl->arguments.out_dir)
                    process_error("ERROR: can't allocate the memory\n", ALLOCATION_ERROR, imapcl);
                strcpy(imapcl->arguments.out_dir, optarg);
                break;
            case '?':
                if(optopt == 'c' || optopt == 'C' || optopt == 'a' || optopt == 'b' || optopt == 'o')
                    fprintf(stderr, "ERROR: %c doesn't have argument value\n", optopt);
                else 
                    fprintf(stderr, "ERROR: unknown argument\n");
                help();
                exit(ARGUMENT_ERROR);
                break;
        }
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
        process_error("ERROR: can't open the feedfile\n", ARGUMENT_ERROR, imapcl);
    }

    char line[256];
    char login[256];
    char passwd[256];

    while (fgets(line, sizeof(line), imapcl->arguments.file)) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) {
            continue;
        }

        if (strncmp(line, "username =", 10) == 0) {
            strcpy(login, line + 11);
            login[strcspn(login, "\n")] = 0;
            printf("%s\n", login);
            break;
        }
    }

    while (fgets(line, sizeof(line), imapcl->arguments.file)) {
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) {
            continue;
        }

        if (strncmp(line, "password =", 10) == 0) {
            strcpy(passwd, line + 11);
            passwd[strcspn(passwd, "\n")] = 0;
            printf("%s\n", passwd);
            break;
        } 
    }

    fclose(imapcl->arguments.file);

    if (strlen(login) == 0 || strlen(passwd) == 0) {
        process_error("ERROR: login or password is empty\n", ARGUMENT_ERROR, imapcl);
    }
}





// Функция для сохранения писем в файлы
void save_email(const char *directory, int email_index, const char *content) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/email_%d.eml", directory, email_index);

    FILE *file = fopen(filepath, "w");
    if (!file) {
        perror("Не удалось сохранить письмо");
        exit(1);
    }

    fprintf(file, "%s", content);
    fclose(file);
    printf("Письмо сохранено: %s\n", filepath);
    printf("Обработка письма %d...\n", email_index);


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

void read_response(BIO *bio, const char* id) {
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
    printf("RESPONSE: %s\n", response);
    free(response);
}

void connect_common(IMAPCL *imapcl, const char *hostname, const char *port) {
    if (imapcl->arguments.t_flag) {
        SSL_library_init();
        imapcl->communication.ctx = SSL_CTX_new(SSLv23_client_method());
        if (!SSL_CTX_set_default_verify_paths(imapcl->communication.ctx)) {
            process_error("ERROR: can't verify certificate\n", CONNECTION_ERROR, imapcl);
        }

        imapcl->communication.bio = BIO_new_ssl_connect(imapcl->communication.ctx);
        BIO_get_ssl(imapcl->communication.bio, &imapcl->communication.ssl);
        SSL_set_mode(imapcl->communication.ssl, SSL_MODE_AUTO_RETRY);

        char connection_str[256];
        snprintf(connection_str, sizeof(connection_str), "%s:%s", hostname, port);
        BIO_set_conn_hostname(imapcl->communication.bio, connection_str);

        if (BIO_do_connect(imapcl->communication.bio) <= 0) {
            process_error("ERROR: can't open connection\n", CONNECTION_ERROR, imapcl);
        }
    } else {
        char connection_str[256];
        snprintf(connection_str, sizeof(connection_str), "%s:%s", hostname, port);
        imapcl->communication.bio = BIO_new_connect(connection_str);
        if (!imapcl->communication.bio || BIO_do_connect(imapcl->communication.bio) <= 0) {
            process_error("ERROR: can't open connection\n", CONNECTION_ERROR, imapcl);
        }
    }
}

void login_and_select(IMAPCL *imapcl) {
    char *login_str = "A1 LOGIN xfedor14 sa6wahNgus\r\n";
    printf("Logging in...\n");
    write_request(imapcl->communication.bio, login_str);
    read_response(imapcl->communication.bio, "A1");
    printf("\n");

    char *select_str = "A2 SELECT INBOX\r\n";
    printf("Selecting INBOX mailbox...\n");
    write_request(imapcl->communication.bio, select_str);
    read_response(imapcl->communication.bio, "A2");
    printf("\n");

    char *search_str = "A3 SEARCH ALL\r\n";
    printf("Searching ALL...\n");
    write_request(imapcl->communication.bio, search_str);
    read_response(imapcl->communication.bio, "A3");
    printf("\n");

    char *fetch_str = "A4 FETCH 2374 BODY[]\r\n";
    printf("Fetching message 2374...\n");
    write_request(imapcl->communication.bio, fetch_str);
    read_response(imapcl->communication.bio, "A4");
}

void connect_with_ssl(IMAPCL *imapcl) {
    printf("Connecting with SSL...\n");
    connect_common(imapcl, "eva.fit.vutbr.cz", "993");
    login_and_select(imapcl);
}

void connect_unsec(IMAPCL *imapcl) {
    printf("Connecting without SSL...\n");
    connect_common(imapcl, "eva.fit.vutbr.cz", "143");
    login_and_select(imapcl);
}



// Функция для записи в файл
void write_to_file(const char *filename, const char *content) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("ERROR: Could not open file for writing");
        exit(1);
    }
    fprintf(file, "%s", content);
    fclose(file);
}

void process_email_message(IMAPCL *imapcl) {
    // Строки для хранения заголовков
    char date[256] = {0};
    char from[256] = {0};
    char to[256] = {0};
    char subject[256] = {0};
    char message_id[256] = {0};
    char body[4096] = {0};  // Для тела письма

    char buffer[4096];  // Буфер для чтения данных из BIO
    int bytes_read;

    // Чтение данных из соединения

        buffer[bytes_read] = '\0';  // Завершаем строку
        // Извлечение заголовков
        if (strstr(buffer, "Date:") != NULL) {
            sscanf(buffer, "Date: %[^\r\n]", date);
        }
        if (strstr(buffer, "From:") != NULL) {
            sscanf(buffer, "From: %[^\r\n]", from);
        }
        if (strstr(buffer, "To:") != NULL) {
            sscanf(buffer, "To: %[^\r\n]", to);
        }
        if (strstr(buffer, "Subject:") != NULL) {
            sscanf(buffer, "Subject: %[^\r\n]", subject);
        }
        if (strstr(buffer, "Message-ID:") != NULL) {
            sscanf(buffer, "Message-ID: %[^\r\n]", message_id);
        }

        // Если тело письма начинается после "BODY[]", извлекаем его
        if (strstr(buffer, "BODY[]") != NULL) {
            // Простая проверка начала тела
            const char *body_start = strstr(buffer, "BODY[] {");
            if (body_start) {
                body_start += 8;  // Пропускаем "BODY[] {"
                strncpy(body, body_start, sizeof(body) - 1);
                body[sizeof(body) - 1] = '\0';  // Обрезаем, если больше, чем позволяет размер
            }
        }

    // Формирование содержимого в формате RFC 5322
    char email_content[8192] = {0};
    snprintf(email_content, sizeof(email_content),
             "Date: %s\r\n"
             "From: %s\r\n"
             "To: %s\r\n"
             "Subject: %s\r\n"
             "Message-ID: %s\r\n"
             "\r\n"  // Пустая строка перед телом письма
             "%s",   // Тело письма
             date, from, to, subject, message_id, body);

    // Запись в файл
    write_to_file("email_message.txt", email_content);
}

int main(int argc, char *argv[]) {
    IMAPCL imapcl;
    init(&imapcl);
    parse_program_arguments(argc, argv, &imapcl);
    if(imapcl.arguments.auth_file)
        process_auth_file(&imapcl);
    if(imapcl.arguments.t_flag) {
         connect_with_ssl(&imapcl);
         printf("Connect with SSL\n");
    }
    else {
        connect_unsec(&imapcl);
        printf("Connect without SSL\n");
    }
    process_email_message(&imapcl);

    return 0;
}
