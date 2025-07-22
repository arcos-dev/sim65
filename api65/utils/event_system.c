/**
 * @file event_system.c
 * @brief Implementação do sistema de eventos para o emulador 6502
 * @author Anderson Costa
 * @version 3.0.0
 * @date 2025-01-06
 */

#include <stdlib.h>
#include <string.h>
#include "../../include/utils/event_system.h"

/* ============================================================================
 *                              ESTRUTURA INTERNA
 * ============================================================================ */

#define MAX_CALLBACKS_PER_EVENT 16

typedef struct {
    event_callback_t callback;
    void *user_data;
} event_callback_info_t;

typedef struct {
    event_callback_info_t callbacks[MAX_CALLBACKS_PER_EVENT];
    int callback_count;
} event_type_callbacks_t;

struct event_system {
    event_type_callbacks_t *event_types;
    int max_event_types;
    int event_type_count;
};

/* ============================================================================
 *                              FUNÇÕES AUXILIARES
 * ============================================================================ */

static event_type_callbacks_t* find_or_create_event_type(event_system_t *system, int event_type) {
    (void)event_type; // Suprime warning de parâmetro não usado

    // Procura pelo tipo de evento existente
    for (int i = 0; i < system->event_type_count; i++) {
        if (system->event_types[i].callbacks[0].callback == NULL) {
            // Slot vazio encontrado, reutiliza
            return &system->event_types[i];
        }
    }

    // Cria novo tipo de evento se houver espaço
    if (system->event_type_count < system->max_event_types) {
        return &system->event_types[system->event_type_count++];
    }

    return NULL;
}

/* ============================================================================
 *                              IMPLEMENTAÇÃO DA API
 * ============================================================================ */

event_system_t* event_system_create(void) {
    event_system_t *system = malloc(sizeof(event_system_t));
    if (!system) {
        return NULL;
    }

    system->max_event_types = 32;  // Número máximo de tipos de eventos
    system->event_type_count = 0;

    system->event_types = calloc(system->max_event_types, sizeof(event_type_callbacks_t));
    if (!system->event_types) {
        free(system);
        return NULL;
    }

    return system;
}

void event_system_destroy(event_system_t *system) {
    if (system) {
        free(system->event_types);
        free(system);
    }
}

void event_system_register_callback(event_system_t *system, int event_type,
                                   event_callback_t callback, void *user_data) {
    if (!system || !callback) {
        return;
    }

    event_type_callbacks_t *event_type_info = find_or_create_event_type(system, event_type);
    if (!event_type_info) {
        return;  // Não há espaço para novos tipos de eventos
    }

    if (event_type_info->callback_count < MAX_CALLBACKS_PER_EVENT) {
        event_type_info->callbacks[event_type_info->callback_count].callback = callback;
        event_type_info->callbacks[event_type_info->callback_count].user_data = user_data;
        event_type_info->callback_count++;
    }
}

void event_system_unregister_callback(event_system_t *system, int event_type,
                                     event_callback_t callback) {
    (void)event_type; // Suprime warning de parâmetro não usado

    if (!system || !callback) {
        return;
    }

    // Procura pelo tipo de evento
    for (int i = 0; i < system->event_type_count; i++) {
        event_type_callbacks_t *event_type_info = &system->event_types[i];

        for (int j = 0; j < event_type_info->callback_count; j++) {
            if (event_type_info->callbacks[j].callback == callback) {
                // Remove o callback movendo o último para esta posição
                if (j < event_type_info->callback_count - 1) {
                    event_type_info->callbacks[j] = event_type_info->callbacks[event_type_info->callback_count - 1];
                }
                event_type_info->callback_count--;
                return;
            }
        }
    }
}

void event_system_trigger_event(event_system_t *system, const event_t *event) {
    if (!system || !event) {
        return;
    }

    // Procura pelo tipo de evento
    for (int i = 0; i < system->event_type_count; i++) {
        event_type_callbacks_t *event_type_info = &system->event_types[i];

        // Executa todos os callbacks registrados para este tipo de evento
        for (int j = 0; j < event_type_info->callback_count; j++) {
            if (event_type_info->callbacks[j].callback) {
                event_type_info->callbacks[j].callback((void*)event, event_type_info->callbacks[j].user_data);
            }
        }
    }
}

void event_system_process_events(event_system_t *system) {
    // Implementação mínima - não há fila de eventos pendentes
    // Os eventos são processados imediatamente em trigger_event
    (void)system;  // Evita warning de parâmetro não utilizado
}