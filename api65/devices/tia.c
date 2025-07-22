/**
 * @file tia.c
 * @brief Implementação do Television Interface Adapter (TIA)
 * @author Anderson Costa
 * @version 3.1.0
 * @date 2025-01-06
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../../include/devices/tia.h"

/* ============================================================================
 *                              ESTRUTURA PRINCIPAL
 * ============================================================================ */

struct tia {
    // Registradores
    uint8_t registers[0x30];

    // Estado do vídeo
    tia_video_t video;

    // Estado do áudio
    tia_audio_t audio;

    // Estado dos gráficos
    tia_graphics_t graphics;

    // Estado do input
    tia_input_t input;

    // Estado das colisões
    tia_collision_t collision;

    // Callbacks
    tia_video_callback_t video_callback;
    tia_audio_callback_t audio_callback;
    tia_input_callback_t input_callback;
    void *video_user_data;
    void *audio_user_data;
    void *input_user_data;

    // Configuração
    double clock_frequency;
    bool ntsc_mode;
    int cycle_count;
};

/* ============================================================================
 *                              PALETA DE CORES NTSC
 * ============================================================================ */

static const tia_color_t ntsc_palette[256] = {
    {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x4A, 0x4A, 0x4A}, {0x4A, 0x4A, 0x4A},
    {0x6F, 0x6F, 0x6F}, {0x6F, 0x6F, 0x6F}, {0x8E, 0x8E, 0x8E}, {0x8E, 0x8E, 0x8E},
    {0xAA, 0xAA, 0xAA}, {0xAA, 0xAA, 0xAA}, {0xC0, 0xC0, 0xC0}, {0xC0, 0xC0, 0xC0},
    {0xD6, 0xD6, 0xD6}, {0xD6, 0xD6, 0xD6}, {0xEC, 0xEC, 0xEC}, {0xEC, 0xEC, 0xEC},
    {0x48, 0x48, 0x00}, {0x48, 0x48, 0x00}, {0x69, 0x69, 0x00}, {0x69, 0x69, 0x00},
    {0x8A, 0x8A, 0x00}, {0x8A, 0x8A, 0x00}, {0xA7, 0xA7, 0x00}, {0xA7, 0xA7, 0x00},
    {0xC5, 0xC5, 0x00}, {0xC5, 0xC5, 0x00}, {0xE3, 0xE3, 0x00}, {0xE3, 0xE3, 0x00},
    {0xFF, 0xFF, 0x00}, {0xFF, 0xFF, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00},
    // ... (continua com todas as cores NTSC)
};

/* ============================================================================
 *                              FUNÇÕES AUXILIARES
 * ============================================================================ */

static void tia_init_palette(tia_t *tia) {
    memcpy(tia->video.palette, ntsc_palette, sizeof(ntsc_palette));
}

static void tia_clear_screen(tia_t *tia) {
    memset(tia->video.pixels, 0, TIA_SCREEN_WIDTH * TIA_SCREEN_HEIGHT);
}

static void tia_update_video(tia_t *tia) {
    // Implementação básica de renderização
    // Em uma implementação completa, isso seria muito mais complexo

    if (tia->video_callback) {
        tia->video_callback(tia->video.pixels, TIA_SCREEN_WIDTH, TIA_SCREEN_HEIGHT,
                           tia->video_user_data);
    }
}

static void tia_update_audio(tia_t *tia) {
    // Implementação básica de áudio
    // Em uma implementação completa, isso geraria samples reais

    if (tia->audio_callback && tia->audio.buffer_index > 0) {
        tia->audio_callback(tia->audio.buffer, tia->audio.buffer_index,
                           tia->audio_user_data);
        tia->audio.buffer_index = 0;
    }
}

/* ============================================================================
 *                              IMPLEMENTAÇÃO DA API
 * ============================================================================ */

tia_t* tia_create(void) {
    tia_t *tia = malloc(sizeof(tia_t));
    if (!tia) {
        return NULL;
    }

    // Inicializa com zeros
    memset(tia, 0, sizeof(tia_t));

    // Configuração padrão
    tia->clock_frequency = TIA_CLOCK_FREQUENCY;
    tia->ntsc_mode = true;

    return tia;
}

void tia_destroy(tia_t *tia) {
    if (!tia) {
        return;
    }

    // Destrói mutex de áudio
    pthread_mutex_destroy(&tia->audio.audio_mutex);

    free(tia);
}

int tia_init(tia_t *tia) {
    if (!tia) {
        return -1;
    }

    // Inicializa mutex de áudio
    pthread_mutex_init(&tia->audio.audio_mutex, NULL);

    // Inicializa paleta
    tia_init_palette(tia);

    // Inicializa áudio
    tia->audio.sample_rate = TIA_AUDIO_SAMPLE_RATE;
    tia->audio.buffer_index = 0;

    // Inicializa canais de áudio
    for (int i = 0; i < 2; i++) {
        tia->audio.channels[i].channel = i;
        tia->audio.channels[i].enabled = false;
        tia->audio.channels[i].volume = 0;
        tia->audio.channels[i].frequency = 0;
        tia->audio.channels[i].control = 0;
        tia->audio.channels[i].counter = 0;
        tia->audio.channels[i].waveform = 0;
    }

    return 0;
}

void tia_reset(tia_t *tia) {
    if (!tia) {
        return;
    }

    // Limpa registradores
    memset(tia->registers, 0, sizeof(tia->registers));

    // Reseta vídeo
    tia_clear_screen(tia);
    tia->video.current_scanline = 0;
    tia->video.current_cycle = 0;
    tia->video.vsync_active = false;
    tia->video.vblank_active = false;
    tia->video.hblank_active = false;

    // Reseta áudio
    pthread_mutex_lock(&tia->audio.audio_mutex);
    tia->audio.buffer_index = 0;
    for (int i = 0; i < 2; i++) {
        tia->audio.channels[i].enabled = false;
        tia->audio.channels[i].volume = 0;
        tia->audio.channels[i].counter = 0;
    }
    pthread_mutex_unlock(&tia->audio.audio_mutex);

    // Reseta gráficos
    memset(&tia->graphics, 0, sizeof(tia_graphics_t));

    // Reseta input
    memset(&tia->input, 0, sizeof(tia_input_t));

    // Reseta colisões
    memset(&tia->collision, 0, sizeof(tia_collision_t));

    tia->cycle_count = 0;
}

void tia_cycle(tia_t *tia) {
    if (!tia) {
        return;
    }

    tia->cycle_count++;

    // Atualiza posição horizontal
    tia->video.current_cycle++;
    if (tia->video.current_cycle >= 228) { // Ciclos por scanline
        tia->video.current_cycle = 0;
        tia->video.current_scanline++;

        if (tia->video.current_scanline >= TIA_SCANLINES) {
            tia->video.current_scanline = 0;
            tia_update_video(tia);
        }
    }

    // Processa áudio
    if (tia->cycle_count % 3 == 0) { // Áudio a cada 3 ciclos
        tia_update_audio(tia);
    }
}

uint8_t tia_read_byte(tia_t *tia, uint8_t address) {
    if (!tia || address > 0x0D) {
        return 0;
    }

    switch (address) {
        case TIA_CXM0P:
            return tia->collision.collision_matrix[0];
        case TIA_CXM1P:
            return tia->collision.collision_matrix[1];
        case TIA_CXP0FB:
            return tia->collision.collision_matrix[2];
        case TIA_CXP1FB:
            return tia->collision.collision_matrix[3];
        case TIA_CXM0FB:
            return tia->collision.collision_matrix[4];
        case TIA_CXM1FB:
            return tia->collision.collision_matrix[5];
        case TIA_CXBLPF:
            return tia->collision.collision_matrix[6];
        case TIA_CXPPMM:
            return tia->collision.collision_matrix[7];
        case TIA_INPT0:
            return tia->input.joystick0;
        case TIA_INPT1:
            return tia->input.joystick1;
        case TIA_INPT2:
            return tia->input.paddle0;
        case TIA_INPT3:
            return tia->input.paddle1;
        case TIA_INPT4:
            return tia->input.fire_button0 ? 0x80 : 0x00;
        case TIA_INPT5:
            return tia->input.fire_button1 ? 0x80 : 0x00;
        default:
            return 0;
    }
}

void tia_write_byte(tia_t *tia, uint8_t address, uint8_t value) {
    if (!tia || address > 0x2C) {
        return;
    }

    tia->registers[address] = value;

    switch (address) {
        case TIA_VSYNC:
            tia->video.vsync_active = (value & 0x02) != 0;
            break;
        case TIA_VBLANK:
            tia->video.vblank_active = (value & 0x02) != 0;
            break;
        case TIA_WSYNC:
            // Aguarda até o final da linha atual
            tia->video.current_cycle = 227;
            break;
        case TIA_RSYNC:
            // Reseta contador horizontal
            tia->video.current_cycle = 0;
            break;
        case TIA_AUDC0:
            tia->audio.channels[0].control = value;
            tia->audio.channels[0].enabled = (value & 0x0F) != 0;
            break;
        case TIA_AUDC1:
            tia->audio.channels[1].control = value;
            tia->audio.channels[1].enabled = (value & 0x0F) != 0;
            break;
        case TIA_AUDF0:
            tia->audio.channels[0].frequency = value;
            break;
        case TIA_AUDF1:
            tia->audio.channels[1].frequency = value;
            break;
        case TIA_AUDV0:
            tia->audio.channels[0].volume = value & 0x0F;
            break;
        case TIA_AUDV1:
            tia->audio.channels[1].volume = value & 0x0F;
            break;
        case TIA_GRP0:
            memcpy(tia->graphics.player0_data, &value, 1);
            break;
        case TIA_GRP1:
            memcpy(tia->graphics.player1_data, &value, 1);
            break;
        case TIA_ENAM0:
            tia->graphics.missile0_enabled = (value & 0x02) != 0;
            break;
        case TIA_ENAM1:
            tia->graphics.missile1_enabled = (value & 0x02) != 0;
            break;
        case TIA_ENABL:
            tia->graphics.ball_enabled = (value & 0x02) != 0;
            break;
        case TIA_CXCLR:
            // Limpa latches de colisão
            memset(tia->collision.collision_latches, 0, sizeof(tia->collision.collision_latches));
            break;
    }
}

const uint8_t* tia_get_video_buffer(tia_t *tia) {
    return tia ? tia->video.pixels : NULL;
}

const tia_color_t* tia_get_palette(tia_t *tia) {
    return tia ? tia->video.palette : NULL;
}

void tia_register_video_callback(tia_t *tia, tia_video_callback_t callback, void *user_data) {
    if (tia) {
        tia->video_callback = callback;
        tia->video_user_data = user_data;
    }
}

void tia_get_video_info(tia_t *tia, tia_video_t *video) {
    if (tia && video) {
        *video = tia->video;
    }
}

int tia_get_audio_samples(tia_t *tia, int16_t *samples, int max_samples) {
    if (!tia || !samples) {
        return 0;
    }

    pthread_mutex_lock(&tia->audio.audio_mutex);
    int count = (tia->audio.buffer_index < max_samples) ? tia->audio.buffer_index : max_samples;
    memcpy(samples, tia->audio.buffer, count * sizeof(int16_t));
    pthread_mutex_unlock(&tia->audio.audio_mutex);

    return count;
}

void tia_register_audio_callback(tia_t *tia, tia_audio_callback_t callback, void *user_data) {
    if (tia) {
        tia->audio_callback = callback;
        tia->audio_user_data = user_data;
    }
}

void tia_set_audio_sample_rate(tia_t *tia, int sample_rate) {
    if (tia && sample_rate > 0) {
        tia->audio.sample_rate = sample_rate;
    }
}

void tia_get_audio_info(tia_t *tia, tia_audio_t *audio) {
    if (tia && audio) {
        pthread_mutex_lock(&tia->audio.audio_mutex);
        *audio = tia->audio;
        pthread_mutex_unlock(&tia->audio.audio_mutex);
    }
}

void tia_set_input(tia_t *tia, const tia_input_t *input) {
    if (tia && input) {
        tia->input = *input;
    }
}

void tia_get_input(tia_t *tia, tia_input_t *input) {
    if (tia && input) {
        *input = tia->input;
    }
}

void tia_register_input_callback(tia_t *tia, tia_input_callback_t callback, void *user_data) {
    if (tia) {
        tia->input_callback = callback;
        tia->input_user_data = user_data;
    }
}

void tia_get_graphics_info(tia_t *tia, tia_graphics_t *graphics) {
    if (tia && graphics) {
        *graphics = tia->graphics;
    }
}

void tia_set_player_data(tia_t *tia, int player, const uint8_t *data) {
    if (tia && data && (player == 0 || player == 1)) {
        memcpy(tia->graphics.player0_data + player * 8, data, 8);
    }
}

void tia_get_collision_info(tia_t *tia, tia_collision_t *collision) {
    if (tia && collision) {
        *collision = tia->collision;
    }
}

bool tia_check_collision(tia_t *tia, int obj1, int obj2) {
    if (!tia || obj1 < 0 || obj1 >= 8 || obj2 < 0 || obj2 >= 8) {
        return false;
    }

    return tia->collision.collision_latches[obj1] && tia->collision.collision_latches[obj2];
}

void tia_set_clock_frequency(tia_t *tia, double frequency) {
    if (tia && frequency > 0) {
        tia->clock_frequency = frequency;
    }
}

double tia_get_clock_frequency(tia_t *tia) {
    return tia ? tia->clock_frequency : 0.0;
}

void tia_set_ntsc_mode(tia_t *tia, bool ntsc) {
    if (tia) {
        tia->ntsc_mode = ntsc;
        tia_init_palette(tia);
    }
}

bool tia_is_ntsc_mode(tia_t *tia) {
    return tia ? tia->ntsc_mode : false;
}