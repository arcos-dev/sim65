/*******************************************************************************
 * tia.h
 *
 * Television Interface Adaptor (TIA) - Example for Atari 2600 Emulation
 *
 * This header file declares a more advanced TIA structure and functions,
 * including cycle-based emulation, partial collision logic, score mode,
 * horizontal motion, audio stubs, and more.
 *
 * Note: This is still not a fully cycle-accurate TIA implementation. It
 *       demonstrates how to expand TIA emulation with more realistic
 *       handling of scanlines, VSYNC, VBLANK, HMOVE, collisions, etc.
 *
 * Author: Anderson Costa
 * Date: 2025-01-27
 ******************************************************************************/

#ifndef TIA_H
#define TIA_H

#include <stdbool.h>
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * TV System Definitions (NTSC / PAL)
 * -----------------------------------------------------------------------------
 * We assume ~3 TIA color clocks per CPU cycle for NTSC.
 */
#define TV_SYSTEM_NTSC 0
#define TV_SYSTEM_PAL 1

#ifndef TV_SYSTEM
#define TV_SYSTEM TV_SYSTEM_NTSC
#endif

/* -----------------------------------------------------------------------------
 * TIA Timings for NTSC (approximate)
 * -----------------------------------------------------------------------------
 *  - 228 color clocks per scanline
 *  - 262 total scanlines per frame
 *    * ~3 lines of VSYNC
 *    * ~37 lines of VBLANK
 *    * 192 visible lines
 *    * ~30 overscan lines
 */
#if TV_SYSTEM == TV_SYSTEM_NTSC
#define TIA_CYCLES_PER_SCANLINE 228
#define TIA_SCANLINES_PER_FRAME 262
#define TIA_FRAMES_PER_SECOND 60
#else
/* PAL uses 312 scanlines, 284 color clocks, etc. Simplified for example. */
#define TIA_CYCLES_PER_SCANLINE 228
#define TIA_SCANLINES_PER_FRAME 312
#define TIA_FRAMES_PER_SECOND 50
#endif

/* -----------------------------------------------------------------------------
 * Address Range: TIA is mapped at 0x0000..0x003F (mirrored until 0x007F..)
 * We define read/write macros if needed.
 * -----------------------------------------------------------------------------
 */
#define TIA_BASE_ADDRESS 0x0000
#define TIA_END_ADDRESS 0x003F
#define TIA_MIRROR_MASK 0x003F

/* -----------------------------------------------------------------------------
 * TIA Registers Indices (simplified set).
 * Real TIA has 0x40 bytes (some are mirrors).
 * -----------------------------------------------------------------------------
 */
typedef enum
{
    TIA_REG_VSYNC  = 0x00,
    TIA_REG_VBLANK = 0x01,
    TIA_REG_WSYNC  = 0x02,
    TIA_REG_RSYNC  = 0x03,
    TIA_REG_COLUP0 = 0x06,
    TIA_REG_COLUP1 = 0x07,
    TIA_REG_COLUPF = 0x08,
    TIA_REG_COLUBK = 0x09,
    TIA_REG_CTRLPF = 0x0A,
    TIA_REG_PF0    = 0x0D,
    TIA_REG_PF1    = 0x0E,
    TIA_REG_PF2    = 0x0F,
    TIA_REG_RESP0  = 0x10,
    TIA_REG_RESP1  = 0x11,
    TIA_REG_RESM0  = 0x12,
    TIA_REG_RESM1  = 0x13,
    TIA_REG_RESBL  = 0x14,
    TIA_REG_AUDC0  = 0x17,
    TIA_REG_AUDF0  = 0x19,
    TIA_REG_AUDV0  = 0x1B,
    TIA_REG_AUDC1  = 0x18,
    TIA_REG_AUDF1  = 0x1A,
    TIA_REG_AUDV1  = 0x1C,
    TIA_REG_GRP0   = 0x1D,
    TIA_REG_GRP1   = 0x1E,
    TIA_REG_ENAM0  = 0x1F,
    TIA_REG_ENAM1  = 0x20,
    TIA_REG_ENABL  = 0x21,
    TIA_REG_HMP0   = 0x24,
    TIA_REG_HMP1   = 0x25,
    TIA_REG_HMM0   = 0x26,
    TIA_REG_HMM1   = 0x27,
    TIA_REG_HMBL   = 0x28,
    TIA_REG_RESMP0 = 0x15,
    TIA_REG_RESMP1 = 0x16,
    TIA_REG_VDELP0 = 0x29,
    TIA_REG_VDELP1 = 0x2A,
    TIA_REG_VDELBL = 0x2B,
    TIA_REG_RESCL  = 0x2C,
    TIA_REG_HMOVE  = 0x2D,
    TIA_REG_CXCLR  = 0x2F,
    /* collisions, input, etc. not fully enumerated here */
    TIA_REG_COUNT = 0x40 /* total 64 registers, with mirrors */
} TIA_RegIndex;

/* -----------------------------------------------------------------------------
 * Screen Dimensions (typical minimal resolution)
 * Real is 160 visible pixels wide, 192 visible lines (NTSC).
 * We store a 160x192 RGBA buffer for display.
 * -----------------------------------------------------------------------------
 */
#define TIA_SCREEN_WIDTH 160
#define TIA_SCREEN_HEIGHT 192

/* -----------------------------------------------------------------------------
 * Score Mode / Reflection bits (CTRLPF)
 * bit 1 => Score Mode
 * bit 0 => Reflect Playfield
 * -----------------------------------------------------------------------------
 */
#define CTRLPF_REFLECT_BIT (1 << 0)
#define CTRLPF_SCORE_BIT (1 << 1)

/* -----------------------------------------------------------------------------
 * Collision Flags (example only, real TIA has more latches).
 * -----------------------------------------------------------------------------
 */
typedef struct
{
    bool p0_p1;
    bool p0_pf;
    bool p1_pf;
    bool m0_p1;
    /* etc... */
} TIA_Collisions;

/* -----------------------------------------------------------------------------
 * Audio Channel Structure (highly simplified).
 * Real TIA uses polynomial counters, frequency dividers, etc.
 * ------------------------------------------------------------------------- */
typedef struct
{
    uint8_t AUDC; /* control (waveform) */
    uint8_t AUDF; /* frequency divider */
    uint8_t AUDV; /* volume */
    /* internal counters, shift regs, etc. for real TIA audio */
    float phase;
    float freq;
} TIA_AudioChannel;

/* -----------------------------------------------------------------------------
 * TIA Device Structure
 * -----------------------------------------------------------------------------
 */
typedef struct TIA
{
    /* The 64 registers (with mirrors). We'll store only up to 0x3F. */
    uint8_t registers[TIA_REG_COUNT];

    /* A 160x192 RGBA buffer for the final rendered image. */
    uint32_t framebuffer[TIA_SCREEN_HEIGHT][TIA_SCREEN_WIDTH];

    /* The TV system in use (NTSC or PAL). */
    int tv_system;

    /* The current color clock cycle in a scanline [0..227 for NTSC]. */
    int color_clock;

    /* The current scanline [0..261 for NTSC]. */
    int scanline;

    /* For toggling vertical sync, blank, overscan, etc. */
    bool vsync;
    bool vblank;

    /* Internal counters for vsync lines, etc. (if you wish). */
    int vsync_lines;
    int vblank_lines;

    /* Frame counter. Increments each time we pass the last scanline. */
    uint64_t frame_count;

    /* Collision status. */
    TIA_Collisions collisions;

    /* Player sprite registers (positions, patterns, etc.). */
    int p0_x;
    int p1_x;
    uint8_t grp0;
    uint8_t grp1;
    int p0_motion; /* from HMP0, etc. */
    int p1_motion;

    /* Playfield registers. */
    uint8_t pf0;
    uint8_t pf1;
    uint8_t pf2;
    uint8_t ctrlpf;

    /* Color registers. */
    uint8_t colup0;
    uint8_t colup1;
    uint8_t colupf;
    uint8_t colubk;

    /* Audio channels. */
    TIA_AudioChannel audio0;
    TIA_AudioChannel audio1;

    /* Whether we completed a frame in this pass. */
    bool frame_done;

} TIA;

/* -----------------------------------------------------------------------------
 * Function Prototypes
 * -----------------------------------------------------------------------------
 */

/**
 * @brief Create and initialize a TIA device.
 * @param tv_system TV_SYSTEM_NTSC or TV_SYSTEM_PAL
 * @return Pointer to new TIA or NULL on failure.
 */
TIA *tia_init(int tv_system);

/**
 * @brief Destroy and free a TIA device.
 */
void tia_destroy(TIA *tia);

/**
 * @brief Read from TIA register (mirroring address).
 */
uint8_t tia_read(TIA *tia, uint16_t address);

/**
 * @brief Write to TIA register (mirroring address).
 * This should handle immediate side effects for things like RESP0,
 * HMOVE, etc.
 */
void tia_write(TIA *tia, uint16_t address, uint8_t data);

/**
 * @brief Advance the TIA by 1 color clock (cycle).
 * This increments color_clock, handles sprite drawing, collisions, etc.
 * Called ~3 times per CPU cycle in NTSC (approx).
 */
void tia_cycle(TIA *tia);

/**
 * @brief Render the final line/pixel into the framebuffer.
 * Called inside tia_cycle() once per color clock.
 */
void tia_render_pixel(TIA *tia);

/**
 * @brief Check collisions after drawing a pixel, for example.
 */
void tia_check_collisions(TIA *tia, int x, bool p0, bool p1,
                          bool pf /* etc. */);

/**
 * @brief Retrieve pointer to the TIA's 160x192 RGBA framebuffer.
 */
const uint32_t *tia_get_framebuffer(TIA *tia);

/**
 * @brief Simple audio generation step (stub).
 */
void tia_audio_step(TIA *tia, float host_sample_rate, float host_dt,
                    float *left, float *right);

#endif /* TIA_H */
