
enum RETURN_TYPE { RETURN_POSITIVE, RETURN_ZERO };

void unwrap_exit(int exit_value, const char *exit_str,
                 RETURN_TYPE return_type = RETURN_POSITIVE);
