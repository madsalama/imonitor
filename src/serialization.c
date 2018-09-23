#include <stdio.h>
#include <string.h>

#include "serialization.h"

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

	value += (request_buffer[0] << 24 );
	value += (request_buffer[1] << 16);
	value += (request_buffer[2] << 8);
	value += (request_buffer[3]);
	
	return value; 
}

unsigned char* serialize_string (unsigned char* request_buffer, int length, char* string){
	strncpy(request_buffer, string, length); // request_buffer[length] = '\0'; 
	return request_buffer + length;
}

void deserialize_string (unsigned char* request_buffer, int length, char* string){
	strncpy(string, request_buffer, length); // string[length] = '\0';
}

unsigned char* serialize_request_data (unsigned char* request_buffer, struct request_data *rd)
{

  request_buffer = serialize_int(request_buffer, rd -> action_len );
  request_buffer = serialize_int(request_buffer, rd -> path_len   );
  request_buffer = serialize_int(request_buffer, rd -> id);

  request_buffer = serialize_string(request_buffer, rd -> action_len, rd -> action);
  request_buffer = serialize_string(request_buffer, rd -> path_len, rd -> path);

  return request_buffer;
}

// length of each part needed to deserialize
// separators are a bad idea because components may contain a separator char as content.
// request_buffer = { <action_len> , <path_len> , id, <action> , <path> };

// 000 3 000 8 000 255 add /var/log 

void deserialize_request_data(unsigned char* request_buffer, struct request_data *rd)
{
        int action_length;
	int path_length;
	int value;

        action_length = deserialize_int(request_buffer);
	printf("deserialize: action_length = %d \n", action_length);
	rd -> action_len = action_length;
	
	request_buffer = request_buffer + 4;

	path_length = deserialize_int(request_buffer);
	printf("deserialize: path_length = %d \n", path_length);
	rd -> path_len = path_length; 

	request_buffer = request_buffer + 4;

        value = deserialize_int(request_buffer);
        rd -> id = value;
        printf("deserialize: id_value = %d \n", rd -> id );

	request_buffer = request_buffer + 4;

	strncpy( rd -> action , request_buffer, action_length);
	rd -> action[action_length] = '\0';
	printf("deserialize: action_string = %s \n", rd -> action );

	// rd -> action = action_string;	// set address of struct pointer to action_string! (after function returns this address may be corrupted and thus holds garbage or only part of the data)!
	
	// we need to set actual data pointed to by rd -> action

	request_buffer = request_buffer + action_length;

	strncpy(rd -> path, request_buffer, path_length);
        rd -> path[path_length] = '\0';
        printf("deserialize: path_string = %s \n", rd -> path );

	request_buffer = request_buffer + path_length; 

	
}





