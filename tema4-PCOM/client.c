#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson/parson.h"

#define HOST "34.246.184.49"
#define PORT 8080

#define REGISTER "/api/v1/tema/auth/register"
#define LOGIN "/api/v1/tema/auth/login"
#define PAYLOAD "application/json"
#define ACCESS "/api/v1/tema/library/access"
#define BOOKS "/api/v1/tema/library/books"
#define LOGOUT "/api/v1/tema/auth/logout"

#define SIZE 1000


char *cookie;
char *jwt_token;
char last_del_id[SIZE];

int register_account(){
    char *message;
    char *response;
    int sockfd;

    if (cookie != NULL) {
        printf("Esti logat\n");
        return -1;
    }

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // declaram username si parola
    char username[SIZE], password[SIZE];

    printf("username=");
    fgets(username, SIZE, stdin);
    username[strcspn(username, "\n")] = '\0';
    printf("password=");
    fgets(password, SIZE, stdin);
    password[strcspn(password, "\n")] = '\0';

    if (strchr(username, '\'') != NULL || strchr(password, '\'') != NULL) {
        printf("ERROR - Caractere ilegale\n");
        close_connection(sockfd);
        return -1;
    }

    //verifica daca usernameul sau parola are spatii
    if (strchr(username, ' ') != NULL || strchr(password, ' ') != NULL) {
        printf("ERROR - Username sau parola contine spatii\n");
        close_connection(sockfd);
        return -1;
    }

    //verific daca usernameul sau parola este goala
    if (strlen(username) == 0 || strlen(password) == 0) {
        printf("ERROR - Nu avem user si/sau parola\n");
        close_connection(sockfd);
        return -1;
    }

    // construim un json cu username si password
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    char* json_string = json_serialize_to_string(root_value);

    // compunem mesajul de tip POST
    message = compute_post_request(HOST, REGISTER, PAYLOAD,
    json_string, NULL, NULL);

    // trimitem mesajul
    send_to_server(sockfd, message);
    // primim raspunsul
    response = receive_from_server(sockfd);
    // afisam raspunsul

    //verificam daca username-ul este deja luat
    if (strstr(response, "HTTP/1.1 400 Bad Request") != NULL) {
        printf("ERROR - Username deja luat\n");
        close_connection(sockfd);
        return -1;
    }
    
    close_connection(sockfd);
    return 0;
}

int login(){
    char *message;
    char *response;
    int sockfd;

    if (cookie != NULL) {
        printf("Esti deja logat\n");
        return -1;
    }

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    // declaram username si parola
    char username[SIZE], password[SIZE];

    printf("username=");
    fgets(username, SIZE, stdin);
    username[strcspn(username, "\n")] = '\0';
    printf("password=");
    fgets(password, SIZE, stdin);
    password[strcspn(password, "\n")] = '\0';

    if (strchr(username, '\'') != NULL || strchr(password, '\'') != NULL) {
        printf("ERROR - Caractere ilegale\n");
        close_connection(sockfd);
        return -1;
    }

    //verifica daca usernameul sau parola are spatii
    if (strchr(username, ' ') != NULL || strchr(password, ' ') != NULL) {
        printf("ERROR - Username sau parola contine spatii\n");
        close_connection(sockfd);
        return -1;
    }

    //verific daca usernameul sau parola este goala
    if (strlen(username) == 0 || strlen(password) == 0) {
        printf("ERROR - Nu avem user si/sau parola\n");
        close_connection(sockfd);
        return -1;
    }

    // construim un json cu username si password
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);
    char* json_string = json_serialize_to_string(root_value);

    // compunem mesajul de tip POST
    message = compute_post_request(HOST, LOGIN, PAYLOAD,
    json_string, NULL, NULL);

    // trimitem mesajul
    send_to_server(sockfd, message);
    // primim raspunsul
    response = receive_from_server(sockfd);

    char* error = strstr(response, "error");
    if (error != NULL) {
        if (strstr(error, "Credentials") != NULL) {
            printf("ERROR - Credentiale gresite\n");
            close_connection(sockfd);
            return -1;
        } else if (strstr(error, "No") != NULL) {
            printf("ERROR - Nu exista userul\n");
            close_connection(sockfd);
            return -1;
        }
    }

    //parsam raspunsul
    if (strstr(response, "Set-Cookie") != NULL) {
        //preluam cookie-ul
        char* token = strstr(response, "Set-Cookie");
        token = strtok(token, " ");
        token = strtok(NULL, " ");
        cookie = strdup(token);
    }
    close_connection(sockfd);
    return 0;
}

int get_book_access() {
    char *message;
    char *response;
    int sockfd;

    //verificam daca suntem logati
    if (cookie == NULL) {
        printf("ERROR - Nu esti logat\n");
        return -1;
    }

    //verificam daca suntem deja in biblioteca
    if (jwt_token != NULL) {
        printf("Esti deja in biblioteca\n");
        return -1;
    }

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // compunem mesajul de tip GET
    message = compute_get_request(HOST, ACCESS, NULL, cookie, 1, NULL);

    // trimitem mesajul
    send_to_server(sockfd, message);
    // primim raspunsul
    response = receive_from_server(sockfd);

    //extragem token-ul
    if (strstr(response, "token") != NULL) {
        //preluam token-ul
        char* token = strstr(response, "token");
        token = strtok(token, ":");
        token = strtok(NULL, "\"");
        jwt_token = strdup(token);
    }

    close_connection(sockfd);
    return 0;
}

int get_books() {
    char *message;
    char *response;
    int sockfd;

    if (jwt_token == NULL) {
        printf("ERROR - Nu ai acces\n");
        return -1;
    }

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // compunem mesajul de tip GET
    message = compute_get_request(HOST, BOOKS, NULL, cookie, 1, jwt_token);
    send_to_server(sockfd, message);

    // primim raspunsul
    response = receive_from_server(sockfd);

    //selectam doar cartile
    char* books = strstr(response, "[");
    printf("%s\n", books);

    close_connection(sockfd);
    return 0;
}

int get_book() {
    char *message;
    char *response;
    int sockfd;

    if (jwt_token == NULL) {
        printf("ERROR - Nu ai acces\n");
        return -1;
    }

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    char book_id[SIZE];
    printf("id=");
    scanf("%s", book_id);

    // compunem mesajul de tip GET
    message = compute_get_request(HOST, BOOKS, book_id, cookie, 1, jwt_token);
    send_to_server(sockfd, message);

    // primim raspunsul
    response = receive_from_server(sockfd);
    
    char *error = strstr(response, "HTTP/1.1 404 Not Found");
    if (error != NULL) {
        printf("ERROR - ID-ul nu exista\n");
        close_connection(sockfd);
        return -1;
    }

    //selectam doar cartea
    char* book = strstr(response, "{");
    printf("%s\n", book);

    close_connection(sockfd);

    return 0;
}

int add_book() {
    char *message;
    char *response;
    int sockfd;

    if (jwt_token == NULL) {
        printf("ERROR - Nu ai acces\n");
        return -1;
    }

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    char title[SIZE];
    char author[SIZE];
    char genre[SIZE];
    char publisher[SIZE];
    char pages[SIZE];

    // citim datele cartii
    printf("title=");
    fgets(title, SIZE, stdin);
    title[strcspn(title, "\n")] = '\0';
    printf("author=");
    fgets(author, SIZE, stdin);
    author[strcspn(author, "\n")] = '\0';
    printf("publisher=");
    fgets(publisher, SIZE, stdin);
    publisher[strcspn(publisher, "\n")] = '\0';
    printf("genre=");
    fgets(genre, SIZE, stdin);
    genre[strcspn(genre, "\n")] = '\0';
    printf("page_count=");
    fgets(pages, SIZE, stdin);
    pages[strcspn(pages, "\n")] = '\0';

    //verificam daca datele sunt valide
    if (strlen(title) == 0 || strlen(author) == 0
    || strlen(genre) == 0 || strlen(publisher) == 0
    || strlen(pages) == 0) {
        printf("ERROR - Nu avem toate datele\n");
        close_connection(sockfd);
        return -1;
    }

    //verificam daca numarul de pagini este numar
    int res = atoi(pages);
    if (res <= 0) {
        printf("ERROR - Numarul de pagini nu e numar\n");
        close_connection(sockfd);
        return -1;
    }

    //construim un json cu datele cartii
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "publisher", publisher);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_string(root_object, "page_count", pages);
    char* json_string = json_serialize_to_string(root_value);
    // compunem mesajul de tip POST
    message = compute_post_request(HOST, BOOKS, PAYLOAD, json_string,
                        cookie, jwt_token);
    send_to_server(sockfd, message);

    // primim raspunsul
    response = receive_from_server(sockfd);

    close_connection(sockfd);
    return 0;
}

void delete_book() {
    char *message;
    char *response;
    int sockfd;

    if (jwt_token == NULL) {
        printf("ERROR - Nu ai acces\n");
        return;
    }

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // citim id-ul cartii
    char book_id[SIZE];
    printf("id=");
    scanf("%s", book_id);
    
    if (strcmp(book_id, last_del_id) == 0) {
        printf("ERROR - ID-ul a fost sters deja\n");
        close_connection(sockfd);
        return;
    }

    // compunem mesajul de tip DELETE
    message = compute_delete_request(HOST, BOOKS, book_id, cookie, 1, jwt_token);
    send_to_server(sockfd, message);

    // primim raspunsul
    response = receive_from_server(sockfd);

    //verificam daca exista id-ul
    char* error = strstr(response, "HTTP/1.1 404 Not Found");
    if (error != NULL) {
        printf("ERROR - Nu exista ID-ul dat\n");
        close_connection(sockfd);
        return;
    }

    strcpy(last_del_id, book_id);

    printf("Cartea cu id-ul %s a fost stearsa cu succes!\n", book_id);
    close_connection(sockfd);
}

int logout() {
    char *message;
    char *response;
    int sockfd;

    if (cookie == NULL) {
        printf("ERROR - Nu esti logat\n");
        return -1;
    }

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // compunem mesajul de tip GET
    message = compute_get_request(HOST, LOGOUT, NULL, cookie, 1, jwt_token);
    send_to_server(sockfd, message);

    // primim raspunsul
    response = receive_from_server(sockfd);

    cookie = NULL;
    jwt_token = NULL;

    close_connection(sockfd);
    return 0;
}


int main(int argc, char *argv[])
{

    char command[SIZE];

    while(1) {
    fgets(command, SIZE, stdin);
        if (strcmp(command, "register\n") == 0) {
        int reg = register_account();
        if (reg == -1) {
            continue;
        } else {
            printf("Utilizator înregistrat cu succes!\n");
            continue;
        }
    } else if (strcmp(command, "login\n") == 0) {
        int log = login();
        if (log == -1) {
            continue;
        } else {
            printf("Utilizatorul s-a logat cu succes!\n");
            continue;
        }
    } else if (strcmp(command, "enter_library\n") == 0) {
        int rez = get_book_access();
        if (rez == -1) {
            continue;
        } else {
            printf("SUCCES - Utilizatorul are acces la biblioteca!\n");
            continue;
        }
    } else if (strcmp(command, "get_books\n") == 0) {
        get_books();
        continue;
    } else if (strcmp(command, "get_book\n") == 0) {
        get_book();
        continue;
    } else if (strcmp(command, "add_book\n") == 0) {
        int new_book = add_book();
        if (new_book == -1){
            continue;
        } else {
            printf("SUCCESS - Carte adaugata\n");
            continue;
        }
    } else if (strcmp(command, "delete_book\n") == 0) {
        delete_book();
        continue;
    } else if (strcmp(command, "logout\n") == 0) {
        int rez = logout();
        if (rez == -1) {
            continue;
        } else {
            printf("SUCCES - Utilizator s-a delogat\n");
            continue;
        }
        continue;
    } else if (strcmp(command, "exit\n") == 0) {
        exit(0);
    }
    }
       
    return 0;
}
