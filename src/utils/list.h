//
// utils
// list.h: Header file for list.c.
//
// Created by jenra.
// Created on July 27 2020.
//

#ifndef UTILS_LIST_H
#define UTILS_LIST_H

#include <stdlib.h>

#define list_append_element(list, size, count, type, element)	\
do																\
{																\
	if ((size) == 0)											\
		(list) = calloc(((size) = 8), sizeof(type));			\
	else if ((size) <= (count))									\
		(list) = realloc(list, ((size) <<= 1) * sizeof(type));	\
	(list)[(count)++] = (element);								\
} while (0)

#endif /* UTILS_LIST_H */
