#ifndef COMMONDEFS_H
#define COMMONDEFS_H 1
extern void stab(void);
extern void warn(const char *s);
extern int error(const char *s);
extern int file_exists(const char*);
extern int can_read_file(const char*);
extern const char* get_home_dir();
#endif
