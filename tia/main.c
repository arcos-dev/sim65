/*******************************************************************************
 * main.c
 *
 * Exemplo de como integrar:
 *   - Emulação de CPU 6502 (com Bus)
 *   - TIA para gerar framebuffer
 *   - Renderização via Raylib
 *
 * Autor: Anderson Costa
 * Data: 2025-02-02
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"

// -----------------------------------------------------------------------------
// Inclua aqui seus cabeçalhos de emulação do emulador principal (src/)
// -----------------------------------------------------------------------------
#include "../src/acia.h"
#include "../src/bus.h"
#include "../src/cpu.h"
#include "../src/tia.h"
#include "../src/clock.h"
#include "../src/memory.h"
#include "../src/via.h"

int main(int argc, char **argv)
{
    //-------------------------------------------------------------------------
    // 1. CONFIGURAÇÕES INICIAIS
    //-------------------------------------------------------------------------
    // Exemplo de frequência de clock: 1 MHz
    double clockFrequency = 1000000.0;

    // Endereço padrão para carregar a ROM (ajuste conforme seu mapeamento)
    uint16_t romStartAddress = 0xF000;

    // Se for passado via linha de comando, pega o caminho
    const char *romFilePath = (argc >= 2) ? argv[1] : NULL;

    //-------------------------------------------------------------------------
    // 2. CRIAR INSTÂNCIAS: ACIA (opcional), TIA, BUS, CPU
    //-------------------------------------------------------------------------
    // Se você quiser ACIA:
    Acia6550 *acia = acia_init();
    if (!acia)
    {
        fprintf(stderr, "[MAIN] Falha ao criar ACIA.\n");
        return EXIT_FAILURE;
    }

    // Cria TIA no modo NTSC (ou PAL, dependendo do seu define/enum)
    TIA *tia = tia_init(TV_SYSTEM_NTSC);
    if (!tia)
    {
        fprintf(stderr, "[MAIN] Falha ao criar TIA.\n");
        acia_destroy(acia);
        return EXIT_FAILURE;
    }

    // Inicializa o BUS
    //   - 64 KB de memória (0x10000 bytes)
    //   - clockFrequency (por exemplo, 1 MHz)
    //   - ACIA e TIA
    bus_t bus;
    if (bus_init(&bus, 0x10000, clockFrequency, acia, tia) != 0)
    {
        fprintf(stderr, "[MAIN] Falha ao inicializar o BUS.\n");
        acia_destroy(acia);
        tia_destroy(tia);
        return EXIT_FAILURE;
    }

    // Inicializa a CPU 6502 usando o bus
    if (cpu6502_init(&bus) != 0)
    {
        fprintf(stderr, "[MAIN] Erro ao inicializar CPU.\n");
        bus_destroy(&bus);
        return EXIT_FAILURE;
    }

    //-------------------------------------------------------------------------
    // 3. CARREGAR PROGRAMA/BINÁRIO (se houver)
    //-------------------------------------------------------------------------
    if (romFilePath)
    {
        if (bus_load_program(&bus, romFilePath, romStartAddress) != 0)
        {
            fprintf(stderr, "[MAIN] Falha ao carregar arquivo ROM: %s\n",
                    romFilePath);
        }
        else
        {
            printf("[MAIN] ROM carregada via linha de comando: %s\n",
                   romFilePath);

            // (Opcional) Ajustar vetor de Reset para romStartAddress
            // bus_write_memory(&bus, 0xFFFC, (uint8_t)(romStartAddress &
            // 0xFF)); bus_write_memory(&bus, 0xFFFD, (uint8_t)((romStartAddress
            // >> 8) & 0xFF));
        }
    }
    else
    {
        printf("[MAIN] Nenhum arquivo ROM especificado.\n");
        printf(
            "[MAIN] Você pode arrastar e soltar um arquivo ROM na janela.\n");
    }

    // Reseta CPU para garantir que PC aponte para o vetor de Reset
    if (cpu6502_reset() < 0)
    {
        fprintf(stderr, "[MAIN] Falha no reset da CPU.\n");
        bus_destroy(&bus);
        return EXIT_FAILURE;
    }

    //-------------------------------------------------------------------------
    // 4. INICIALIZAR A JANELA RAYLIB
    //-------------------------------------------------------------------------
    const int screenWidth  = 640;
    const int screenHeight = 480;

    InitWindow(screenWidth, screenHeight, "Emulador 6502 + TIA com Raylib");
    SetTargetFPS(60);

    //-------------------------------------------------------------------------
    // Cria Image e Texture a partir do framebuffer do TIA
    //-------------------------------------------------------------------------
    Image tiaImage   = {0};
    tiaImage.data    = (void*)tia_get_framebuffer(tia);
    tiaImage.width   = TIA_SCREEN_WIDTH;
    tiaImage.height  = TIA_SCREEN_HEIGHT;
    tiaImage.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    tiaImage.mipmaps = 1;

    Texture2D tiaTexture = LoadTextureFromImage(tiaImage);

    //-------------------------------------------------------------------------
    // 5. LOOP PRINCIPAL DE EMULAÇÃO + RENDERIZAÇÃO
    //-------------------------------------------------------------------------
    while (!WindowShouldClose())
    {
        // ---------------------------------------------------------------------
        // 5.1 TRATAR ARQUIVOS ARRASTADOS (DRAG & DROP) - opcional
        // ---------------------------------------------------------------------
        if (IsFileDropped())
        {
            FilePathList droppedFiles = LoadDroppedFiles();
            if (droppedFiles.count > 0)
            {
                if (bus_load_program(&bus, droppedFiles.paths[0],
                                     romStartAddress) == 0)
                {
                    printf("[MAIN] ROM carregada via Drag&Drop: %s\n",
                           droppedFiles.paths[0]);
                    // (opcional) Ajustar vetor de Reset
                    // bus_write_memory(&bus, 0xFFFC, (uint8_t)(romStartAddress
                    // & 0xFF)); bus_write_memory(&bus, 0xFFFD,
                    // (uint8_t)((romStartAddress >> 8) & 0xFF));
                    cpu6502_reset();
                }
                else
                {
                    fprintf(stderr, "[MAIN] Falha ao carregar ROM: %s\n",
                            droppedFiles.paths[0]);
                }
            }
            UnloadDroppedFiles(droppedFiles);
        }

        // ---------------------------------------------------------------------
        // 5.2 RODAR A CPU (EXEMPLO SIMPLES)
        //
        //    - Você pode decidir quantos ciclos ou instruções quer rodar
        //      a cada frame. Se o clock for 1MHz e você quer ~60 FPS,
        //      pode estimar ~1.000.000 / 60 ≈ 16666 ciclos por frame.
        //    - Aqui faremos algo bem simples: rodar um número fixo
        //      de instruções por frame (p.ex. 3000).
        // ---------------------------------------------------------------------
        // Verifica se a CPU não está parada (implementação pode variar)
        // if (!cpu->halted) // Este campo pode não existir na implementação atual
        {
            for (int i = 0; i < 16666; i++)
            {
                int cycles = cpu6502_step();

                if (cycles < 0)
                {
                    fprintf(stderr, "[CPU] Erro ou opcode ilegal.\n");
                    break;
                }

                // Se houver ACIA, processa TX/RX
                if (acia)
                {
                    acia_process_tx(acia);
                    // acia_process_rx(acia); // se houver implementação
                }
            }
        }

        // ---------------------------------------------------------------------
        // 5.3 RENDER DO TIA
        //    - A ideia é que o TIA use os registradores escritos pela CPU
        //      (no bus) para gerar o conteúdo do framebuffer.
        //    - Aqui chamamos algo como 'tia_render(tia)', que você ajusta
        //      conforme a sua implementação.
        // ---------------------------------------------------------------------
        // tia_render(tia); // Se existir essa função

        // Para o TIA atual, podemos fazer cycles para atualizar o framebuffer
        for (int i = 0; i < TIA_CYCLES_PER_SCANLINE; i++) {
            tia_cycle(tia);
        }

        // ---------------------------------------------------------------------
        // 5.4 ATUALIZAR A TEXTURA COM O FRAMEBUFFER
        // ---------------------------------------------------------------------
        UpdateTexture(tiaTexture, tia_get_framebuffer(tia));

        // ---------------------------------------------------------------------
        // 5.5 DESENHAR COM RAYLIB
        // ---------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(BLACK);

        // Desenha escalado para preencher a janela
        Rectangle srcRect  = {0, 0, (float) TIA_SCREEN_WIDTH,
                              (float) TIA_SCREEN_HEIGHT};
        Rectangle destRect = {0, 0, (float) screenWidth, (float) screenHeight};
        Vector2 origin     = {0, 0};

        DrawTexturePro(tiaTexture, srcRect, destRect, origin, 0.0f, WHITE);

        // Instruções na tela
        DrawText("Arraste/solte um arquivo .bin / .rom para carregar", 10, 10,
                 18, RAYWHITE);
        DrawText("Pressione ESC para sair", 10, 35, 18, RAYWHITE);
        EndDrawing();
    }

    //-------------------------------------------------------------------------
    // 6. ENCERRAR E LIBERAR RECURSOS
    //-------------------------------------------------------------------------
    UnloadTexture(tiaTexture);
    CloseWindow();

    // Destrói CPU, bus, TIA, ACIA...
    cpu6502_destroy();
    bus_destroy(&bus); // isto deve chamar tia_destroy(tia) e acia_destroy(acia)
                       // dependendo da sua implementação
    // Caso não chame, faça manualmente:
    // tia_destroy(tia);
    // acia_destroy(acia);

    return EXIT_SUCCESS;
}
