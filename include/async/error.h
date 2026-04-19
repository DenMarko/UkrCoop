#ifndef ASYNC_ERROR_H
#define ASYNC_ERROR_H

enum class error_code : int {
    ok = 0,

    // загальні
    invalid_argument,
    not_initialized,
    already_exists,
    not_found,
    timeout,

    // файлова система / лог
    file_open_failed,
    write_failed,

    // мережа / grpc
    connection_failed,
    request_cancelled,
    request_failed,

    // внутрішня логіка
    unexpected_state,

    mysql_error,
};

// зручно для логування
inline const char* to_string(error_code e) noexcept {
    switch (e) {
    case error_code::ok:                return "ok";
    case error_code::invalid_argument:  return "invalid_argument";
    case error_code::not_initialized:   return "not_initialized";
    case error_code::already_exists:    return "already_exists";
    case error_code::not_found:         return "not_found";
    case error_code::timeout:           return "timeout";
    case error_code::file_open_failed:  return "file_open_failed";
    case error_code::write_failed:      return "write_failed";
    case error_code::connection_failed: return "connection_failed";
    case error_code::request_cancelled: return "request_cancelled";
    case error_code::request_failed:    return "request_failed";
    case error_code::unexpected_state:  return "unexpected_state";
    case error_code::mysql_error:       return "mysql_error";
    default:                            return "unknown";
    }
}

#endif // ASYNC_ERROR_H