#ifndef SERIALIZE_H
#define SERIALIZE_H

struct request_data {
        int wd;
	int action_len;
	int path_len;
	char* action;
        char* path;
};

unsigned char* serialize_int (unsigned char* request_buffer, int value);
int deserialize_int(unsigned char* request_buffer);

unsigned char* serialize_string (unsigned char* request_buffer, int length, char* string);
void deserialize_string (unsigned char* request_buffer, int length, char* string);

unsigned char* serialize_request_data (unsigned char* request_buffer, struct request_data *rd);
unsigned char* deserialize_request_data(unsigned char* request_buffer, struct request_data *rd);

#endif /*SERIALIZE_H*/
