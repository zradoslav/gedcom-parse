/* Basic file encoding */
#ifndef __ENCODING_H
#define __ENCODING_H

typedef enum _ENC {
  ONE_BYTE = 0,
  TWO_BYTE_HILO = 1,
  TWO_BYTE_LOHI = 2
} ENCODING;

int open_conv_to_internal(char* fromcode);
void close_conv_to_internal();
char* to_internal(char* str, size_t len);
void init_encodings();
void set_encoding_width(ENCODING enc);

#endif /* __ENCODING_H */
