#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <limits.h>

struct request_data {
	int action_len;
	int path_len;
	int id;
	char action[10];
        char path[PATH_MAX];
};

unsigned char* serialize_int (unsigned char* request_buffer, int value);
int deserialize_int(unsigned char* request_buffer);

unsigned char* serialize_string (unsigned char* request_buffer, int length, char* string);
void deserialize_string (unsigned char* request_buffer, int length, char* string);

unsigned char* serialize_request_data (unsigned char* request_buffer, struct request_data *rd);
void deserialize_request_data(unsigned char* request_buffer, struct request_data *rd);

#endif /*SERIALIZATION_H*/
