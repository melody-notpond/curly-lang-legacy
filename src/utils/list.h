// 
// utils
// list.h: Header file for list.c.
// 
// Created by jenra.
// Created on July 27 2020.
// 

#ifndef utils_list_h
#define utils_list_h

#include <stdlib.h>

#define list_append_element(list, size, count, type, element)	\
do																\
{																\
	if ((size) < (count))										\
		(list) = realloc(list, ((size) <<= 1) * sizeof(type));	\
	(list)[(count)++] = (element);									\
} while (0)

#endif /* utils_list_h */
