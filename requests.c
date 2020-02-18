#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"


char *compute_get_request(char *host, char *url, char *url_params, char *header, char* cookies){

    char* message = calloc( BUFLEN, sizeof(char) );
    char* line = calloc( LINELEN, sizeof(char) );
   
    if ( url_params != NULL )   
        sprintf( line, "GET %s?%s HTTP/1.1", url, url_params );
    else    sprintf( line, "GET %s HTTP/1.1", url );

    compute_message( message, line );

    sprintf( line, "Host: %s", host );
    compute_message( message, line );

    if( header != NULL )    compute_message( message, header );
    if( cookies != NULL )     compute_message( message, cookies );
    
    compute_message( message, "" );

    free(line);

    
    return message;
}



char *compute_post_request(char *host, char *url, char *form_data, 
                            char *type, char* headers, char *cookies){

    char* message = (char*) calloc( BUFLEN, sizeof(char) );
    char* line = (char*) calloc( LINELEN, sizeof(char) );
    
    sprintf( line, "POST %s HTTP/1.1", url );
    compute_message( message, line );

    sprintf( line, "Host: %s", host );
    compute_message( message, line );

    sprintf( line, "Content-Type: %s", type );
    compute_message( message, line );

    compute_message( message, cookies );

    sprintf( line, "Content-Length: %ld", strlen(form_data) );
    compute_message( message, line );

    if( headers != NULL )   compute_message(message, headers);
  
    compute_message( message, "" );

    strcat( message, form_data );


    return message;
    
}