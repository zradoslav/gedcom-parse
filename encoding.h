int open_conv_to_internal(char* fromcode);
void close_conv_to_internal();
char* to_internal(char* str, size_t len);
void init_encodings();
char* get_encoding(char* gedcom_name);
