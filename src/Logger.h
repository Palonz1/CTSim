/*
 * Logger.h
 *
 *  Created on: 13.06.2013
 *      Author: Alexander
 */
#ifndef LOGGER_H_
#define LOGGER_H_

#define OFF 0
#define FATAL 1
#define ERROR 2
#define WARN 3
#define INFO 4
#define DEBUG 5
#define TRACE 6
#define ALL 7
#define LOGLEVEL WARN



void logIt(int messageLevel, const char *message);





#endif /* LOGGER_H_ */