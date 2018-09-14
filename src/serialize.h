#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <limits.h>

struct request_data {
	int action_len;
	int path_len;
	int wd;
	char action[100];
        char path[PATH_MAX];
};

unsigned char* serialize_int (unsigned char* request_buffer, int value);
int deserialize_int(unsigned char* request_buffer);

unsigned char* serialize_string (unsigned char* request_buffer, int length, char* string);
void deserialize_string (unsigned char* request_buffer, int length, char* string);

unsigned char* serialize_request_data (unsigned char* request_buffer, struct request_data *rd);
struct request_data* deserialize_request_data(unsigned char* request_buffer, struct request_data *rd);

#endif /*SERIALIZE_H*/
