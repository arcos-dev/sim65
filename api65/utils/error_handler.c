/**
 * @file error_handler.c
 * @brief Implementação do sistema de tratamento de erros para o emulador 6502
 * @author Anderson Costa
 * @version 3.0.0
 * @date 2025-01-06
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../include/utils/error_handler.h"

/* ============================================================================
 *                              ESTRUTURA INTERNA
 * ============================================================================ */

#define MAX_ERROR_CALLBACKS 8

typedef struct {
    error_callback_t callback;
    void *user_data;
} error_callback_info_t;

struct error_handler {
    error_info_t last_error;
    bool has_error;
    error_callback_info_t callbacks[MAX_ERROR_CALLBACKS];
    int callback_count;
    int total_errors;
    int errors_by_level[4];  // Para cada nível de erro
};

/* ============================================================================
 *                              FUNÇÕES AUXILIARES
 * ============================================================================ */

static uint64_t get_timestamp(void) {
    return (uint64_t)time(NULL);
}

/* ============================================================================
 *                              IMPLEMENTAÇÃO DA API
 * ============================================================================ */

error_handler_t* error_handler_create(void) {
    error_handler_t *handler = malloc(sizeof(error_handler_t));
    if (!handler) {
        return NULL;
    }

    memset(handler, 0, sizeof(error_handler_t));
    handler->has_error = false;
    handler->callback_count = 0;
    handler->total_errors = 0;

    return handler;
}

void error_handler_destroy(error_handler_t *handler) {
    if (handler) {
        free(handler);
    }
}

void error_handler_set_error(error_handler_t *handler, error_level_t level, int code,
                            const char *message, const char *function,
                            const char *file, int line) {
    if (!handler) {
        return;
    }

    // Atualiza informações do erro
    handler->last_error.level = level;
    handler->last_error.code = code;
    handler->last_error.line = line;
    handler->last_error.timestamp = get_timestamp();

    // Copia strings com segurança
    strncpy(handler->last_error.message, message ? message : "", sizeof(handler->last_error.message) - 1);
    handler->last_error.message[sizeof(handler->last_error.message) - 1] = '\0';

    strncpy(handler->last_error.function, function ? function : "", sizeof(handler->last_error.function) - 1);
    handler->last_error.function[sizeof(handler->last_error.function) - 1] = '\0';

    strncpy(handler->last_error.file, file ? file : "", sizeof(handler->last_error.file) - 1);
    handler->last_error.file[sizeof(handler->last_error.file) - 1] = '\0';

    handler->has_error = true;
    handler->total_errors++;
    handler->errors_by_level[level]++;

    // Executa callbacks
    for (int i = 0; i < handler->callback_count; i++) {
        if (handler->callbacks[i].callback) {
            handler->callbacks[i].callback(&handler->last_error, handler->callbacks[i].user_data);
        }
    }
}

const char* error_handler_get_last_error(error_handler_t *handler) {
    if (!handler || !handler->has_error) {
        return "";
    }
    return handler->last_error.message;
}

bool error_handler_get_last_error_info(error_handler_t *handler, error_info_t *error) {
    if (!handler || !error || !handler->has_error) {
        return false;
    }

    *error = handler->last_error;
    return true;
}

void error_handler_clear_error(error_handler_t *handler) {
    if (!handler) {
        return;
    }

    handler->has_error = false;
    memset(&handler->last_error, 0, sizeof(error_info_t));
}

void error_handler_register_callback(error_handler_t *handler, error_callback_t callback, void *user_data) {
    if (!handler || !callback) {
        return;
    }

    if (handler->callback_count < MAX_ERROR_CALLBACKS) {
        handler->callbacks[handler->callback_count].callback = callback;
        handler->callbacks[handler->callback_count].user_data = user_data;
        handler->callback_count++;
    }
}

void error_handler_unregister_callback(error_handler_t *handler, error_callback_t callback) {
    if (!handler || !callback) {
        return;
    }

    for (int i = 0; i < handler->callback_count; i++) {
        if (handler->callbacks[i].callback == callback) {
            // Remove o callback movendo o último para esta posição
            if (i < handler->callback_count - 1) {
                handler->callbacks[i] = handler->callbacks[handler->callback_count - 1];
            }
            handler->callback_count--;
            return;
        }
    }
}

int error_handler_get_error_count(error_handler_t *handler) {
    if (!handler) {
        return 0;
    }
    return handler->total_errors;
}

int error_handler_get_error_count_by_level(error_handler_t *handler, error_level_t level) {
    if (!handler || level < 0 || level >= 4) {
        return 0;
    }
    return handler->errors_by_level[level];
}