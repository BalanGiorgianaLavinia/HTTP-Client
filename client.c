#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
#include <sys/types.h>



//functie pentru obtinerea adresei IP a unui site dorit
//functia este preluata din laboratorul de DNS 
char* get_ip(char* url){

	struct addrinfo options, *result, *list;

	memset(&options, 0, sizeof(options));

	options.ai_socktype = SOCK_STREAM;
	options.ai_family = PF_INET;

	if( getaddrinfo( url, NULL, &options, &result ) != 0 )  return NULL;

	void *address;

	list = result;

    char* buffer;

	while( list != NULL ) {

		switch ( list -> ai_family ) {

            case AF_INET:
                address =  &( ( struct sockaddr_in* ) list->ai_addr )->sin_addr;
                break;
            
            case AF_INET6:
                address =  &((struct sockaddr_in *) list->ai_addr)->sin_addr;
                break;

		}

		buffer = (char*) calloc(1, BUFLEN);
        if(buffer == NULL)  return NULL;

		inet_ntop(list->ai_family, address, buffer, BUFLEN);

		break;

	}

	free(result);

    return buffer;

}




int main( int argc, char *argv[] ){

    char* message;
    char* response;
    int socket_fd;

    ////////////////////////task-1

    //trimit o cerere de tip get serverului pentru 
    //url-ul specificat in enunt
    socket_fd = open_connection(IP_SERVER, PORT_SERVER, 
                                    AF_INET, SOCK_STREAM, 0);

    message = compute_get_request(IP_SERVER, "/task1/start", 
                                    NULL, NULL, NULL);
                                    
    send_to_server(socket_fd, message);

    response = receive_from_server(socket_fd);
    printf("\nRaspuns task-1:\n%s\n\n\n", response);

    close_connection(socket_fd);
   




    /////////////////////////task-2
    char* cookies = (char*) calloc(1, BUFLEN);
    if( cookies == NULL )   return -1;

    strcpy( cookies, "Cookie: " );

    //iau fiecare linie din raspuns
    char* line = strtok( response, "\n" );

    while( line != NULL ){

        line = strtok( NULL, "\n" );
        if( line == NULL ) break;

        //daca contine cookie
        if( strstr( line, "Set-Cookie: " ) != NULL ){

            //retin linia
            char* aux = strdup( line );
            if( aux == NULL )   return -2;

            //iau cookie-ul
            char* currentCookie = strsep( &aux, " " );
            currentCookie = strsep( &aux, ";" );

            //si il adaug in string-ul cu cookies
            strcat( cookies, currentCookie );
            strcat( cookies, "; " );

        }else{

            //daca este de tip json
            if( strstr( line, "{" ) != NULL ){

                //iau continutul text din json si il convertesc in obiect
                //pentru a-i putea accesa campurile
                JSON_Value* newJson = json_parse_string( line );
                JSON_Object* objJson = json_object( newJson );

                //extrag datele din json
                char* url = (char*) json_object_get_string( objJson, "url" );
                char* method = (char*) json_object_get_string( objJson, "method" );
                char* type = (char*) json_object_get_string( objJson, "type" );

                //campul data este un obiect la randul lui care contine alte campuri
                JSON_Object* data = json_object_get_object( objJson, "data" );

                char* content = calloc(1, BUFLEN);
                if( content == NULL ) return -3;

                //parsez campurile din "data"
                //adica username si password
                //si le adaug in continutul mesajului 
                //de trimis la server pentru logare
                int i = 0;
                for(; i < json_object_get_count(data); i++){

                    strcat( content, (const char*) json_object_get_name(data, i) );
                    strcat( content, "=" );

                    JSON_Value* val = json_object_get_value_at( data, i );
                    strcat( content, json_value_get_string(val) );

                    if( i == json_object_get_count(data) - 1 )  break;

                    strcat(content, "&");

                }


                free(message);
                free(response);

                //compun mesajul si il trimit
                if(strcmp(method, "POST") == 0)
                    message = compute_post_request(IP_SERVER, url, 
                                            content, type, NULL, cookies);
                
                socket_fd = open_connection( IP_SERVER, PORT_SERVER, 
                                            AF_INET, SOCK_STREAM, 0 );

                send_to_server(socket_fd, message);
                response = receive_from_server(socket_fd);
                printf("\nRaspuns task-2:\n%s\n\n\n", response);
            
                close_connection(socket_fd);

                //eliberez memoria alocata
                free(cookies);
                free(message);
                json_value_free(newJson);


                break;

            }
        }
    }






    /////////////////////////////////////task-3
    char* header;

    cookies = (char*) calloc(1, BUFLEN);
    if( cookies == NULL )   return -4;

    strcpy( cookies, "Cookie: " );
    line = strtok( response, "\n" );

    //iau fiecare linie si verific daca am cookiuri
    //daca am, le retin pentru a trimite mai departe prajituricile :))
    while( line != NULL ){

        line = strtok( NULL, "\n" );
        if( line == NULL )    break;

        if( strstr( line, "Set-Cookie: " ) != NULL ){

            char* aux = strdup( line );
            if( aux == NULL )   return -5;

            char* currentCookie = strsep( &aux, " " );
            currentCookie = strsep(&aux, ";");

            strcat(cookies, currentCookie);
            strcat(cookies, "; ");

        }else{ 

            //daca am json
            if( strstr(line, "{") != NULL ){

                //iau continutul text din json si il convertesc in obiect
                //pentru a-i putea accesa campurile
                JSON_Value* newJson = json_parse_string(line);
                JSON_Object* objJson = json_object(newJson);

                //parsez jsonul si iau campurile "url", "method", 
                //"data" si "queryParams"
                char* url = (char*) json_object_get_string( objJson, "url" );
                char* method = (char*) json_object_get_string
                                        ( objJson, "method" );

                JSON_Object* data = json_object_get_object( objJson, "data" );


                char* url_params = calloc(1, BUFLEN);
                if( url_params == NULL )    return -6;

                //construiesc parametrii cererii 
                //adaugand raspunsurile la cele doua intrebari
                strcat( url_params, "raspuns1=omul&raspuns2=numele&" );
                
                //preiau cheia(token-ul) din campul "data" al jsonului
                JSON_Value* token = json_object_get_value( data, "token" );

                header = calloc(1, BUFLEN);
                if( header == NULL )    return -7;

                //adaug in header-ul mesajului tokenul JWT pentru autorizare
                strcat( header, "Authorization: Basic " );
                strcat( header, json_value_get_string(token) );

                JSON_Object* queryParams = json_object_get_object
                                            ( data, "queryParams" );
                
                //preiau paametrii din campul "queryParams" al json-ului
                int i = 0;
                for(; i < json_object_get_count(queryParams); i++){

                    strcat( url_params, (const char*) json_object_get_name
                                                        (queryParams, i) );
                    strcat(url_params, "=");

                    JSON_Value* val = json_object_get_value_at(queryParams, i);
                    strcat( url_params, json_value_get_string(val) );

                    if( i == json_object_get_count(queryParams) - 1 )   break;

                    strcat( url_params, "&" );

                    json_value_free(val);

                }


                free(message);
                free(response);

                //compun mesajul si il trimit la server
                //afisez raspunsul server-ului
                if( strcmp(method, "GET") == 0 )
                    message = compute_get_request( IP_SERVER, url, 
                                                url_params, header, cookies );


                socket_fd = open_connection( IP_SERVER, PORT_SERVER, 
                                            AF_INET, SOCK_STREAM, 0 );

                send_to_server( socket_fd, message );
                response = receive_from_server(socket_fd);
                printf( "\nRaspuns task-3:\n%s\n\n\n", response );

                close_connection(socket_fd);


                free(message);
                free(cookies);
                free(url_params);
                json_value_free(newJson);


                break;
            }
        }
    }








    /////////////////////////////////////////task-4
    cookies = (char*) calloc(1, BUFLEN);
    if( cookies == NULL )   return -8;

    strcpy( cookies, "Cookie: " );
    line = strtok( response, "\n" );

    //preiau cookie-urile ca la task-urile anterioare
    while( line != NULL ){

        line = strtok( NULL, "\n" );
        if( line == NULL )    break;

        if( strstr( line, "Set-Cookie: " ) != NULL ){

            char* aux = strdup(line);
            if( aux == NULL )   return -9;

            char* currentCookie = strsep( &aux, " " );
            currentCookie = strsep( &aux, ";" );

            strcat(cookies, currentCookie);
            strcat(cookies, "; ");

        }else{

            //cand gasesc json-ul
            if( strstr(line, "{") != NULL ){

                JSON_Value* newJson = json_parse_string(line);
                JSON_Object* objJson = json_object(newJson);

                //parsez campurile json-ului si retin noul url si metoda
                char* url = (char*) json_object_get_string(objJson, "url");
                char* method = (char*) json_object_get_string(objJson, "method");
                              

                free(message);
                free(response);

                //compun mesajul si il trimit catre server cu 
                //tokenul JWT pentru autorizare(preluat la taskul precedent)
                if( strcmp( method, "GET" ) == 0 )
                    message = compute_get_request(IP_SERVER, url, 
                                                NULL, header, cookies);


                socket_fd = open_connection( IP_SERVER, PORT_SERVER, 
                                            AF_INET, SOCK_STREAM, 0 );

                send_to_server( socket_fd, message );
                response = receive_from_server(socket_fd);
                printf( "\nRaspuns task-4:\n%s\n\n\n", response );

                close_connection(socket_fd);


                free(message);
                free(cookies);
                json_value_free(newJson);


                break;
            }
        }
    }






    //////////////////////////////////////task-5
    cookies = (char*) calloc(1, BUFLEN);
    if( cookies == NULL )   return -10;

    strcpy( cookies, "Cookie: " );

    line = strtok( response, "\n" );

    //preiau cookie-urile
    while( line != NULL ){

        line = strtok( NULL, "\n" );
        if( line == NULL )    break;

        if( strstr( line, "Set-Cookie: " ) != NULL ){

            char* aux = strdup(line);
            if( aux == NULL )   return -11;

            char* currentCookie = strsep( &aux, " " );
            currentCookie = strsep(&aux, ";");

            strcat( cookies, currentCookie );
            strcat( cookies, "; " );

        }else{

            //daca e json
            if( strstr( line, "{" ) != NULL ){

                JSON_Value* newJson = json_parse_string(line);
                JSON_Object* objJson = json_object(newJson);

                //preiau din json informatiile necesare
                char* url = (char*) json_object_get_string( objJson, "url" );
                char* method = (char*) json_object_get_string( objJson, "method" );
                char* type = (char*) json_object_get_string( objJson, "type" );


                JSON_Object* data = json_object_get_object( objJson, "data" );

                char* data_url = (char*) json_object_get_string( data, "url" );
                char* data_method = (char*) json_object_get_string( data, "method" );

                
                char* url_params = (char*) calloc(1, BUFLEN);
                if( url_params == NULL )    return -12;

                //preiau parametrii cererii
                JSON_Object* queryParams = json_object_get_object
                                            ( data, "queryParams" );
                
                int i = 0;
                for(; i < json_object_get_count(queryParams); i++){

                    strcat( url_params, (const char*) json_object_get_name
                                                    ( queryParams, i ) );
                    strcat( url_params, "=" );

                    JSON_Value* val = json_object_get_value_at( queryParams, i );
                    strcat( url_params, json_value_get_string(val) );

                    if( i == json_object_get_count(queryParams) - 1 )   break;

                    strcat( url_params, "&" );


                    json_value_free(val);

                }


                free(response);


                //parsez url-ul din campul "data" al json-ului
                //luand numele server-ului si url-ul paginii 
                char* aux_data_url = strdup(data_url);
                if( aux_data_url == NULL )  return -13;

                char* site = strtok( aux_data_url, "/" );

                //aflu adresa IP a site-ului
                char* ip_site = get_ip(site);

                char* new_url = strstr( data_url, "/" );
            
                //creez mesajul pentru cererea de tip get catre site-ul de vreme
                if( strcmp( data_method, "GET" ) == 0 )
                    message = compute_get_request( ip_site, new_url, 
                                                url_params, NULL, NULL );
                

                int newsockfd = open_connection( ip_site, PORT_HTTP, 
                                                AF_INET, SOCK_STREAM, 0 );

                send_to_server( newsockfd, message );
                
                free(message);

                response = receive_from_server(newsockfd);
                printf( "\nRaspunsul de la site este:\n%s\n\n", response );

                close_connection(newsockfd);

                //parsez raspunsul de la site-ul de vreme 
                //mai exact, retin json-ul
                char* json_message = strstr( response, "{" );


                //compun mesajul de trimis catre server cu raspunsul
                //de la site-ul de vreme 
                if( strcmp( method, "POST" ) == 0 )
                    message = compute_post_request( IP_SERVER, url, 
                                            json_message, type, header, cookies );
                

                free(response);


                socket_fd = open_connection( IP_SERVER, PORT_SERVER, 
                                            AF_INET, SOCK_STREAM, 0 );

                send_to_server( socket_fd, message );
                response = receive_from_server(socket_fd);
                printf( "\nRaspuns task-5:\n%s\n\n\n", response );

                close_connection(socket_fd);



                //eliberez memoria alocata
                free(message);
                free(response);
                free(cookies);
                free(url_params);
                free(header);
                json_value_free(newJson);
                free(ip_site);



                break;
            }
        }
    }




    return 0;
}