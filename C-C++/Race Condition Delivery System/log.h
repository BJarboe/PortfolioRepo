/** @name log header
 *  @file log.h
 *  Not the @author Bryce Jarboe | RedID 825033151
 *  @date 4/18/2024
 *  @brief logging functions for main food delivery program 
*/

#ifndef LOG_H
#define LOG_H
#include "fooddelivery.h"

void log_added_request (RequestAdded requestAdded);

void log_removed_request(RequestRemoved requestRemoved);

void log_production_history(unsigned int produced[], unsigned int *consumed[]);

#endif
