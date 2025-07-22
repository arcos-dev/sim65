#include <stdlib.h>
#include <string.h>
#include "../../include/devices/memory.h"

/* ============================================================================
 *                              FUNÇÕES INTERNAS
 * ============================================================================ */

/**
 * @brief Função interna de leitura de byte da memória
 * @param device Ponteiro para o dispositivo de memória
 * @param address Endereço a ser lido
 * @return Byte lido do endereço especificado
 */
static uint8_t memory_read_impl(memory_t *device, uint16_t address) {
    if (!device || address >= device->size) {
        return 0;  // Retorna 0 para endereços inválidos
    }
    return device->data[address];
}

/**
 * @brief Função interna de escrita de byte na memória
 * @param device Ponteiro para o dispositivo de memória
 * @param address Endereço a ser escrito
 * @param value Valor a ser escrito
 */
static void memory_write_impl(memory_t *device, uint16_t address, uint8_t value) {
    if (!device || address >= device->size) {
        return;  // Ignora escritas em endereços inválidos
    }
    device->data[address] = value;
}

/* ============================================================================
 *                              IMPLEMENTAÇÃO DA API
 * ============================================================================ */

memory_t* memory_create(size_t size) {
    memory_t *mem = malloc(sizeof(memory_t));
    if (!mem) {
        return NULL;
    }

    mem->data = malloc(size);
    if (!mem->data) {
        free(mem);
        return NULL;
    }

    // Inicializa dados da memória com zeros
    memset(mem->data, 0, size);
    mem->size = size;

    // Configura ponteiros de função
    mem->read = memory_read_impl;
    mem->write = memory_write_impl;

    return mem;
}

void memory_destroy(memory_t *mem) {
    if (mem) {
        free(mem->data);
        free(mem);
    }
}

uint8_t memory_read_byte(memory_t *mem, uint16_t address) {
    if (!mem || !mem->read) {
        return 0;
    }
    return mem->read(mem, address);
}

void memory_write_byte(memory_t *mem, uint16_t address, uint8_t value) {
    if (!mem || !mem->write) {
        return;
    }
    mem->write(mem, address, value);
}