/*******************************************************************************
 * acia.c
 *
 * Implementation of the ACIA 6550/6551 Emulation Module
 *
 * This source file provides the implementation for the ACIA 6550/6551
 * emulation, including initialization, register read/write operations, and
 * processing of transmit and receive buffers.
 *
 * Author: Anderson Costa
 * Date: 2025-01-04
 ******************************************************************************/

#include "acia.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initializes a new ACIA6550 instance
Acia6550 *acia_init(void)
{
    Acia6550 *acia = (Acia6550 *) calloc(1, sizeof(Acia6550));

    if (!acia)
    {
        fprintf(stderr, "Error: Failed to allocate memory for ACIA6550.\n");
        return NULL;
    }

    // Initialize TX buffer status
    acia->tx_head  = 0;
    acia->tx_tail  = 0;
    acia->tx_ready = true; // Transmitter is ready initially

    // Initialize RX buffer status
    acia->rx_head  = 0;
    acia->rx_tail  = 0;
    acia->rx_ready = false; // No data received initially

    // Initialize Control Register (default settings)
    acia->control_reg = 0x00; // All features disabled by default

    // Initialize Status Register
    acia->status_reg = ACIA_STATUS_TX_READY; // Transmitter ready

    return acia;
}

// Resets the ACIA6550 to its default state
void acia_reset(Acia6550 *acia)
{
    if (!acia)
        return;

    // Clear TX buffer
    memset(acia->tx_buffer, 0, sizeof(acia->tx_buffer));
    acia->tx_head  = 0;
    acia->tx_tail  = 0;
    acia->tx_ready = true; // Transmitter is ready after reset

    // Clear RX buffer
    memset(acia->rx_buffer, 0, sizeof(acia->rx_buffer));
    acia->rx_head  = 0;
    acia->rx_tail  = 0;
    acia->rx_ready = false; // No data received after reset

    // Reset Control Register
    acia->control_reg = 0x00;

    // Reset Status Register
    acia->status_reg = ACIA_STATUS_TX_READY;
}

// Destroys and frees the ACIA6550 instance
void acia_destroy(Acia6550 *acia)
{
    if (acia)
    {
        free(acia);
    }
}

// Reads a byte from the specified ACIA register address
uint8_t acia_read(Acia6550 *acia, uint16_t address)
{
    if (!acia)
        return 0;

    // Calculate register offset
    uint16_t offset = address - ACIA_START_ADDRESS;

    switch (offset)
    {
        case ACIA_REG_STATUS:
        {
            // Update status bits before returning
            uint8_t status = 0;

            // Transmitter Ready
            if (acia->tx_ready)
                status |= ACIA_STATUS_TX_READY;

            // Receiver Ready
            if (acia->rx_ready)
                status |= ACIA_STATUS_RX_READY;

            // Additional status flags can be updated here

            return status;
        }

        case ACIA_REG_DATA_RX:
        {
            if (acia->rx_ready)
            {
                // Read a byte from RX buffer
                char data = acia->rx_buffer[acia->rx_tail];

                // Update RX tail
                acia->rx_tail = (acia->rx_tail + 1) % ACIA_RX_BUFFER_SIZE;

                // Check if RX buffer is now empty
                if (acia->rx_tail == acia->rx_head)
                {
                    acia->rx_ready = false;
                    acia->status_reg &= ~ACIA_STATUS_RX_READY;
                }

                return (uint8_t) data;
            }
            else
            {
                // No data available
                return 0;
            }
        }

        case ACIA_REG_CONTROL:
        {
            // Return current control register value
            return acia->control_reg;
        }

        default:
            // Undefined register address
            return 0;
    }
}

// Writes a byte to the specified ACIA register address
void acia_write(Acia6550 *acia, uint16_t address, uint8_t data)
{
    if (!acia)
        return;

    // Calculate register offset
    uint16_t offset = address - ACIA_START_ADDRESS;

    switch (offset)
    {
        case ACIA_REG_DATA_TX:
        {
            // Check if Transmitter is enabled
            if (!(acia->control_reg & ACIA_CONTROL_ENABLE_TX))
            {
                // Transmitter is disabled; ignore the write
                return;
            }

            // Check if TX buffer has space
            uint16_t next_head = (acia->tx_head + 1) % ACIA_TX_BUFFER_SIZE;

            if (next_head != acia->tx_tail)
            {
                // Insert data into TX buffer
                acia->tx_buffer[acia->tx_head] = (char) data;
                acia->tx_head                  = next_head;
                acia->tx_ready                 = false; // Transmitter is busy

                // Update status register
                acia->status_reg &= ~ACIA_STATUS_TX_READY;

                // Optionally, trigger a transmit interrupt if enabled
                if (acia->control_reg & ACIA_CONTROL_INTERRUPT_TX)
                {
                    // Implement interrupt triggering mechanism here
                }
            }
            else
            {
                // TX buffer overflow; set overrun error
                acia->status_reg |= ACIA_STATUS_OVERRUN;
            }

            break;
        }

        case ACIA_REG_CONTROL:
        {
            // Update Control Register
            acia->control_reg = data;

            // Handle enabling/disabling transmitter and receiver
            if (acia->control_reg & ACIA_CONTROL_ENABLE_TX)
            {
                acia->tx_ready = (acia->tx_head == acia->tx_tail);
                if (acia->tx_ready)
                    acia->status_reg |= ACIA_STATUS_TX_READY;
            }
            else
            {
                acia->tx_ready = false;
                acia->status_reg &= ~ACIA_STATUS_TX_READY;
            }

            if (acia->control_reg & ACIA_CONTROL_ENABLE_RX)
            {
                // Receiver enabled; RX ready if buffer has data
                acia->rx_ready = (acia->rx_head != acia->rx_tail);
                if (acia->rx_ready)
                    acia->status_reg |= ACIA_STATUS_RX_READY;
            }
            else
            {
                acia->rx_ready = false;
                acia->status_reg &= ~ACIA_STATUS_RX_READY;
            }

            break;
        }

        default:
            // Undefined register address; ignore write
            break;
    }
}

// Processes the transmit buffer, sending data out to the virtual serial
void acia_process_tx(Acia6550 *acia)
{
    if (!acia)
        return;

    // Check if transmitter is enabled and ready
    if (!(acia->control_reg & ACIA_CONTROL_ENABLE_TX) || acia->tx_ready)
        return;

    // Transmit data from TX buffer
    while (acia->tx_tail != acia->tx_head)
    {
        char data = acia->tx_buffer[acia->tx_tail];

        // Send data to stdout or a virtual serial port
        fputc(data, stdout);
        fflush(stdout);

        // Update TX tail
        acia->tx_tail = (acia->tx_tail + 1) % ACIA_TX_BUFFER_SIZE;

        // Check if TX buffer is empty
        if (acia->tx_tail == acia->tx_head)
        {
            acia->tx_ready = true;
            acia->status_reg |= ACIA_STATUS_TX_READY;

            // Optionally, trigger a transmit interrupt if enabled
            if (acia->control_reg & ACIA_CONTROL_INTERRUPT_TX)
            {
                // Implement interrupt triggering mechanism here
            }

            break;
        }
    }
}

// Provides input data to the receive buffer, simulating incoming serial data
void acia_provide_input(Acia6550 *acia, const char *data)
{
    if (!acia || !data)
        return;

    while (*data)
    {
        // Check if RX buffer has space
        uint16_t next_head = (acia->rx_head + 1) % ACIA_RX_BUFFER_SIZE;

        if (next_head == acia->rx_tail)
        {
            // RX buffer is full; set overrun error
            acia->status_reg |= ACIA_STATUS_OVERRUN;
            break;
        }

        // Insert data into RX buffer
        acia->rx_buffer[acia->rx_head] = *data++;
        acia->rx_head                  = next_head;
        acia->rx_ready                 = true;

        // Update status register
        acia->status_reg |= ACIA_STATUS_RX_READY;

        // Optionally, trigger a receive interrupt if enabled
        if (acia->control_reg & ACIA_CONTROL_INTERRUPT_RX)
        {
            // Implement interrupt triggering mechanism here
        }
    }
}
