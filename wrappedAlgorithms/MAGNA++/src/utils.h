#ifndef _MAGNA_UTILS_H
#define _MAGNA_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

extern int ISWIN32;

void mg_quit(int status);
void mg_error(const char *s, ...);
char *read_line(FILE *fid);
char *trimwhitespace(char *str);
int file_exists(const char *filename);
int is_dir(const char *filename);

#endif
