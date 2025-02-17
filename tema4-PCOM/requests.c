#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char *cookie, int cookies_count, char *jwt_token)
{
    char *line = calloc(LINELEN, sizeof(char));

    // scriem metoda, URL-ul si protocolul + query_params (daca exista)
    if (query_params != NULL) {
        sprintf(line, "GET %s/%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    // scriem host-ul + port-ul
    strcat(line, "\r\nHost: ");
    strcat(line, host);
    strcat(line, ":8080");
    strcat(line, "\r\nConnection: keep-alive");

    // verificam daca avem cookie si token JWT
    if (cookie != NULL) {
        strcat(line, "\r\nCookie: ");
        strcat(line, cookie);
    }
    if (jwt_token != NULL) {
        strcat(line, "\r\nAuthorization: Bearer ");
        strcat(line, jwt_token);
    }
    
    strcat(line, "\r\n");
    strcat(line, "\r\n");
   
    return line;
}

char *compute_post_request(char *host, char *url, char* content_type, char *body_data,
                            char *cookie, char *jwt_token)
{
    char *line = calloc(LINELEN, sizeof(char));

    // scriem metoda, URL-ul si protocolul
    sprintf(line, "POST %s HTTP/1.1", url);
    
    // scriem host-ul + port-ul
    strcat(line, "\r\nHost: ");
    strcat(line, host);
    strcat(line, ":8080");
    
    // scriem tipul de continut si lungimea continutului
    strcat(line, "\r\nContent-Type: ");
    strcat(line, content_type);
    strcat(line, "\r\nContent-Length: ");
    char content_length[1000];
    sprintf(content_length, "%ld", strlen(body_data));
    strcat(line, content_length);
    
    // verificam daca avem cookie si token JWT
    if (cookie != NULL) {
        strcat(line, "\r\nCookie: ");
        strcat(line, cookie);
    }
    if (jwt_token != NULL) {
        strcat(line, "\r\nAuthorization: Bearer ");
        strcat(line, jwt_token);
    }

    // adaugam body-ul
    strcat(line, "\r\n\r\n");
    strcat(line, body_data);
    strcat(line, "\r\n");

    return line;
}

char* compute_delete_request(char* host, char* url, char* query_params,
                            char* cookie, int cookies_count, char* jwt_token)
{
    char *line = calloc(LINELEN, sizeof(char));

    // scriem metoda, URL-ul si protocolul + query_params (daca exista)
    if (query_params != NULL) {
        sprintf(line, "DELETE %s/%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    // scriem host-ul + port-ul
    strcat(line, "\r\nHost: ");
    strcat(line, host);
    strcat(line, ":8080");
    strcat(line, "\r\nConnection: keep-alive");

    // verificam daca avem cookie si token JWT
    if (cookie != NULL) {
        strcat(line, "\r\nCookie: ");
        strcat(line, cookie);
    }
    if (jwt_token != NULL) {
        strcat(line, "\r\nAuthorization: Bearer ");
        strcat(line, jwt_token);
    }

    strcat(line, "\r\n");
    strcat(line, "\r\n");

    return line;
}
