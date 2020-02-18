#ifndef _REQUESTS_
#define _REQUESTS_

char *compute_get_request(char *host, char *url, char *url_params, 
                            char *headers, char *cookies);

char *compute_post_request(char *host, char *url, char *form_data, 
                            char *type, char* headers, char *cookies);

#endif