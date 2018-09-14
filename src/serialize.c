#include <stdio.h>
#include <string.h>

#include "serialize.h"

/* Write big-endian int value into request_buffer; assumes 32-bit int and 8-bit char. */

unsigned char* serialize_int (unsigned char* request_buffer, int value)
{
	request_buffer[0] = (value >> 24) & 0xFF;
	request_buffer[1] = (value >> 16) & 0xFF;
	request_buffer[2] = (value >> 8) & 0xFF;
	request_buffer[3] = value & 0xFF;

	return request_buffer + 4;
}

int deserialize_int(unsigned char* request_buffer){

	int value = 0;

	value += (request_buffer[0] << 24) & 0xFF;
	value += (request_buffer[1] << 16) & 0xFF;
	value += (request_buffer[2] << 8)  & 0xFF;
	value += (request_buffer[3])       & 0xFF;

	return value; 
}

unsigned char* serialize_string (unsigned char* request_buffer, int length, char* string){
	request_buffer[length] = '\0'; 
	return request_buffer + length;
}

void deserialize_string (unsigned char* request_buffer, int length, char* string){
}

unsigned char* serialize_request_data (unsigned char* request_buffer, struct request_data *rd)
{

  request_buffer = serialize_int(request_buffer, rd -> action_len );
  request_buffer = serialize_int(request_buffer, rd -> path_len   );
  request_buffer = serialize_int(request_buffer, rd -> wd);

  request_buffer = serialize_string(request_buffer, rd -> action_len, rd -> action);
  request_buffer = serialize_string(request_buffer, rd -> path_len, rd -> path);
	
  return request_buffer;
}

// length of each part needed to deserialize
// separators are a bad idea because components may contain a separator char as content.
// request_buffer = { <action_len> , <path_len> , wd, <action> , <path> };

// 000 3 000 8 000 255 add /var/log 

unsigned char* deserialize_request_data(unsigned char* request_buffer, struct request_data *rd)
{
        int action_length;
	int path_length;
	int value;

        action_length = deserialize_int(request_buffer);
	printf("deserialize: action_length = %d \n", action_length);
	rd -> action_len = action_length;

	request_buffer+=4;

	path_length = deserialize_int(request_buffer);
	printf("deserialize: path_length = %d \n", path_length);
	rd -> path_len = path_length; 

	request_buffer += 4;

        value = deserialize_int(request_buffer);
        rd -> wd = value;
        printf("deserialize: wd_value = %d \n", rd -> wd );

	request_buffer += 4;   

	char action_string[action_length];
	char path_string[path_length];

        rd -> action = request_buffer;	             
	printf("deserialize: action_string = %s \n", rd -> action );
	request_buffer = request_buffer + action_length;

        rd -> path = path_string;
        printf("deserialize: path_string = %s \n", rd -> path );
        request_buffer = request_buffer + path_length;
	
	return request_buffer;
	
}





