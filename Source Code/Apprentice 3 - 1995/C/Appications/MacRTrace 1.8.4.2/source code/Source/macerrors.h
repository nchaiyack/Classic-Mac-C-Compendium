/* Prototypes */
unsigned char *get_mac_error_message(short error_code);
void abortive_error(short error_code);
void abortive_string_error(char *error_string);
void terminal_error(short error_code);
void terminal_string_error(char *error_string);
void terminal_startup_error(short error_code);
void possibly_terminal_string_error(char *error_string);

