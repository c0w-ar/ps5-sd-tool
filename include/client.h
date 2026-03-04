#ifndef __LISTENER_H__
#define __LISTENER_H__

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "helpers.h"

#define APP_PORT 6666

int wait_client(void);
void close_client(void);
void printf_s(const char *format, ...);
char read_char(void);

int read_n_digits(void* buf, int n_digits);
uint64_t read_uint64(void);
uint32_t read_uint32(void);

#endif