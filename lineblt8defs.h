#ifndef LINEBLT8DEFS_H
#define LINEBLT8DEFS_H 1
#include <sys/types.h> /* for uint8_t */
#include "lispemul.h" /* for LispPTR, DLword */
void lineBlt8(DLword *srcbase, int offset, uint8_t *destl, int width,
              uint8_t color0, uint8_t color1, LispPTR sourcetype, LispPTR operation);
#endif
