#ifndef COMMONDEFS_H
#define COMMONDEFS_H 1

#include <stdio.h>

extern void stab(void);
extern void warn(const char *s);
extern int error(const char *s);
extern int file_exists(const char*);
extern int can_read_file(const char*);
extern const char* get_home_dir();
extern int create_file_descriptor(FILE*);
extern void free_file_descriptor(int fd);
extern FILE* get_file_pointer(int fd);
extern int create_dir(const char* path);

#endif
