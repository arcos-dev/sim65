/*******************************************************************************
 * tia.c
 *
 * Television Interface Adaptor (TIA) - Example for Atari 2600 Emulation
 *
 * This file implements a more advanced TIA structure with:
 *  - Scanline-based logic
 *  - Partial cycle-based drawing
 *  - VSync / VBlank / Overscan sections
 *  - Collisions stubs
 *  - Score mode & reflection stubs
 *  - Audio stubs
 *
 * Note: This is still not a fully cycle-accurate implementation.
 *       It's meant as an illustrative extension to show how to
 *       progress from a basic TIA approach to something more realistic.
 *
 * Author: Anderson Costa
 * Date: 2025-01-27
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tia.h"

/* Extern palettes or your color-lookup function.
 * For instance, you might have:
 *   #include "palette.h"
 * Then do a function that maps TIA color codes (0..127) => RGBA.
 * We'll define a simple stub here:
 */
static uint32_t tia_color_lut_ntsc[128] = {
    0xFF0000FF, /* etc. fill in or reference palette */};
static uint32_t tia_color_lut_pal[128] = {
    0x00FF00FF, /* etc. fill in or reference palette */};

/* Forward declarations */
static void tia_apply_write_side_effects(TIA *tia, uint8_t reg, uint8_t data);
static uint32_t tia_color_to_rgba(TIA *tia, uint8_t color_code);
static bool get_playfield_pixel(TIA *tia, int x);
static bool get_player_pixel(uint8_t grp, int sprite_x, int x);
static void hmove_objects(TIA *tia);

/* -----------------------------------------------------------------------------
 * tia_init
 * -----------------------------------------------------------------------------
 */
TIA *tia_init(int tv_system)
{
    TIA *tia = (TIA *) calloc(1, sizeof(TIA));
    if (!tia)
    {
        fprintf(stderr, "tia_create: allocation failed\n");
        return NULL;
    }

    tia->tv_system   = tv_system;
    tia->color_clock = 0;
    tia->scanline    = 0;
    tia->frame_count = 0;
    tia->frame_done  = false;

    /* Default positions, might be at left edge or something. */
    tia->p0_x = 0;
    tia->p1_x = 0;

    /* Clear entire framebuffer to black. */
    memset(tia->framebuffer, 0, sizeof(tia->framebuffer));

    return tia;
}

/* -----------------------------------------------------------------------------
 * tia_destroy
 * -----------------------------------------------------------------------------
 */
void tia_destroy(TIA *tia)
{
    if (tia)
    {
        free(tia);
    }
}

/* -----------------------------------------------------------------------------
 * tia_read
 *
 * Mirror the address to 0..63 and return the corresponding register (if it's
 * a readable TIA reg). In real hardware, many are "write-only", returning
 * open bus or collision latches. We'll do a simplified approach.
 * -----------------------------------------------------------------------------
 */
uint8_t tia_read(TIA *tia, uint16_t address)
{
    if (!tia)
        return 0;
    uint16_t offset = address & 0x3F; /* mirror 0..63 */
    return tia->registers[offset];
}

/* -----------------------------------------------------------------------------
 * tia_write
 *
 * Writes to TIA register, then applies side effects immediately.
 * This is critical for cycle-accurate or near-cycle-accurate emulation, as many
 * writes (RESP0, HMOVE, etc.) affect the next pixels drawn in the same line.
 * -----------------------------------------------------------------------------
 */
void tia_write(TIA *tia, uint16_t address, uint8_t data)
{
    if (!tia)
        return;
    uint16_t offset = address & 0x3F;

    tia->registers[offset] = data;
    tia_apply_write_side_effects(tia, (uint8_t) offset, data);
}

/* -----------------------------------------------------------------------------
 * tia_apply_write_side_effects
 *
 * Called by tia_write() to handle immediate changes that occur upon writing to
 * certain TIA registers (like RESP0, HMOVE, etc.).
 * -----------------------------------------------------------------------------
 */
static void tia_apply_write_side_effects(TIA *tia, uint8_t reg, uint8_t data)
{
    switch (reg)
    {
        case TIA_REG_VSYNC:
            /* bit1 => VSYNC on/off */
            tia->vsync = (data & 0x02) != 0;
            break;

        case TIA_REG_VBLANK:
            /* bit7 => VBLANK */
            tia->vblank = (data & 0x80) != 0;
            break;

        case TIA_REG_RSYNC:
            /* Force horizontal sync reset? We'll just reset color_clock. */
            tia->color_clock = 0;
            break;

        case TIA_REG_COLUP0:
            tia->colup0 = data;
            break;
        case TIA_REG_COLUP1:
            tia->colup1 = data;
            break;
        case TIA_REG_COLUPF:
            tia->colupf = data;
            break;
        case TIA_REG_COLUBK:
            tia->colubk = data;
            break;
        case TIA_REG_CTRLPF:
            tia->ctrlpf = data;
            break;

        case TIA_REG_PF0:
            tia->pf0 = data;
            break;
        case TIA_REG_PF1:
            tia->pf1 = data;
            break;
        case TIA_REG_PF2:
            tia->pf2 = data;
            break;

        case TIA_REG_RESP0:
            /* Position of Player0 = current color_clock.
               Real hardware has a 15-tick offset. We'll omit that detail. */
            tia->p0_x = tia->color_clock;
            break;
        case TIA_REG_RESP1:
            tia->p1_x = tia->color_clock;
            break;

        case TIA_REG_GRP0:
            tia->grp0 = data;
            break;
        case TIA_REG_GRP1:
            tia->grp1 = data;
            break;

        case TIA_REG_HMOVE:
            /* Apply horizontal motion for P0, P1, etc. real hardware's
               behavior is more complex. We'll do something basic. */
            hmove_objects(tia);
            break;

        /* Audio registers: store them in audio channel struct. */
        case TIA_REG_AUDC0:
            tia->audio0.AUDC = data;
            break;
        case TIA_REG_AUDF0:
            tia->audio0.AUDF = data;
            break;
        case TIA_REG_AUDV0:
            tia->audio0.AUDV = data;
            break;
        case TIA_REG_AUDC1:
            tia->audio1.AUDC = data;
            break;
        case TIA_REG_AUDF1:
            tia->audio1.AUDF = data;
            break;
        case TIA_REG_AUDV1:
            tia->audio1.AUDV = data;
            break;

            /* Many more registers possible. */

        default:
            /* No immediate side effect. */
            break;
    }
}

/* -----------------------------------------------------------------------------
 * hmove_objects
 *
 * Called when writing to HMOVE register. Real hardware uses HMP0..HMP1..
 * 4-bit twos complement nibbles to shift objects. We do a placeholder.
 * -----------------------------------------------------------------------------
 */
static void hmove_objects(TIA *tia)
{
    /* Example: read HMP0, shift p0_x by that nibble. */
    int8_t shift_p0 =
        (int8_t) (tia->registers[TIA_REG_HMP0] << 4) >> 4; // sign-extend 4 bits

    int new_p0_x = (int)tia->p0_x + shift_p0;
    if (new_p0_x < 0)
        new_p0_x += TIA_SCREEN_WIDTH;
    tia->p0_x = (uint16_t)(new_p0_x % TIA_SCREEN_WIDTH);

    int8_t shift_p1 = (int8_t) (tia->registers[TIA_REG_HMP1] << 4) >> 4;

    int new_p1_x = (int)tia->p1_x + shift_p1;
    if (new_p1_x < 0)
        new_p1_x += TIA_SCREEN_WIDTH;
    tia->p1_x = (uint16_t)(new_p1_x % TIA_SCREEN_WIDTH);

    /* Similar for missiles/ball if implemented. */
}

/* -----------------------------------------------------------------------------
 * tia_cycle
 *
 * Advance the TIA by 1 color clock. Typically you call this 3 times per
 * CPU cycle in NTSC. We:
 *  1) Render the pixel at [color_clock, scanline] if it's in the visible area.
 *  2) Update collisions if necessary.
 *  3) Increment color_clock, and if it hits TIA_CYCLES_PER_SCANLINE, we
 *     increment scanline and do end-of-line tasks (detect end of frame).
 * -----------------------------------------------------------------------------
 */
void tia_cycle(TIA *tia)
{
    if (!tia)
        return;
    tia_render_pixel(tia);

    /* Move to next color clock. */
    tia->color_clock++;
    if (tia->color_clock >= (uint16_t)TIA_CYCLES_PER_SCANLINE)
    {
        tia->color_clock = 0;
        tia->scanline++;

        /* If we passed the last scanline, that's the end of the frame. */
        if (tia->scanline >= (uint16_t)TIA_SCANLINES_PER_FRAME)
        {
            tia->scanline   = 0;
            tia->frame_done = true;
            tia->frame_count++;
        }
    }
}

/* -----------------------------------------------------------------------------
 * tia_render_pixel
 *
 * Renders a single pixel into the framebuffer if we are within visible area
 * and not in VBLANK or overscan.
 * Basic approach:
 *  - If vsync==true, it's top lines
 *  - If vblank==true, we skip drawing
 *  - Otherwise, if scanline < 192, color_clock < 160 => we draw.
 * -----------------------------------------------------------------------------
 */
void tia_render_pixel(TIA *tia)
{
    uint16_t x = tia->color_clock;
    uint16_t y = tia->scanline;

    /* If in vsync or in vblank or outside visible area, do nothing. */
    if (tia->vsync)
        return;
    if (tia->vblank)
        return;
    if (y >= (uint16_t)TIA_SCREEN_HEIGHT)
        return;
    if (x >= (uint16_t)TIA_SCREEN_WIDTH)
        return;

    /* Determine background color code. */
    uint8_t color_code = tia->colubk;

    /* Check playfield bit. */
    bool pf = get_playfield_pixel(tia, x);

    /* Check players. */
    bool p0 = get_player_pixel(tia->grp0, tia->p0_x, x);
    bool p1 = get_player_pixel(tia->grp1, tia->p1_x, x);

    /* Score mode bit => if set, left half uses color_up0,
     * right half color_up1 for PF. */
    bool score_mode = (tia->ctrlpf & CTRLPF_SCORE_BIT) != 0;

    /* If PF is set, pick colupf or handle score mode. */
    if (pf)
    {
        if (!score_mode)
        {
            color_code = tia->colupf;
        }
        else
        {
            if (x < 80)
                color_code = tia->colup0; /* left half */
            else
                color_code = tia->colup1; /* right half */
        }
    }

    /* Player 0 has priority over PF if p0==true. */
    if (p0)
    {
        color_code = tia->colup0;
    }
    /* Player 1 has priority over PF if p1==true. (What if both p0 and p1?) */
    if (p1)
    {
        color_code = tia->colup1;
    }

    /* (Optional) check missile, ball, etc. for final priority. Omitted here. */

    /* Convert color code => RGBA and store in the framebuffer. */
    tia->framebuffer[y][x] = tia_color_to_rgba(tia, color_code);

    /* Check collisions. (Simplified) */
    tia_check_collisions(tia, x, p0, p1, pf);
}

/* -----------------------------------------------------------------------------
 * tia_check_collisions
 *
 * Real TIA sets hardware collision latches as soon as an overlap occurs.
 * Here we do a minimal version (p0 vs p1, p0 vs pf, etc.).
 * -----------------------------------------------------------------------------
 */
void tia_check_collisions(TIA *tia, int x, bool p0, bool p1, bool pf)
{
    (void) x;

    if (p0 && p1)
    {
        tia->collisions.p0_p1 = true;
        /* If using real latches, you'd set CXM0P, etc. registers, or
           store them in the TIA collision registers. */
    }
    if (p0 && pf)
    {
        tia->collisions.p0_pf = true;
    }
    if (p1 && pf)
    {
        tia->collisions.p1_pf = true;
    }
    /* etc. for missiles, ball, etc. */
}

/* -----------------------------------------------------------------------------
 * get_playfield_pixel
 *
 * Determine if the playfield bit is set at x.
 * The real TIA uses 20 bits: PF0 (4 bits), PF1 (8 bits), PF2 (8 bits).
 * Reflection or no reflection depends on CTRLPF bit0.
 * -----------------------------------------------------------------------------
 */
static bool get_playfield_pixel(TIA *tia, int x)
{
    bool reflect = (tia->ctrlpf & CTRLPF_REFLECT_BIT) != 0;

    /* Compute a 0..19 index from x. Each 4 horizontal pixels map to 1 PF bit.
     */
    int index = x / 4; /* x: 0..159 => index: 0..39 if no reflection. We'll
                          limit to 0..19. */

    if (index > 19)
        index = 19; /* just clamp */
    if (reflect)
    {
        /* Mirror horizontally => x < 80 uses PF0..PF2 normally, x >=80 uses
         * reversed bits. */
        /* A simpler approach: reflect the index. */
        if (index >= 20)
            index = 39 - index;
        /* but this is a simplistic approach. Implementation details vary. */
    }

    /* PF0 => bits [4..7], PF1 => [0..7], PF2 => [0..7], combined => 20 bits. */
    if (index < 4)
    {
        /* belongs to PF0, bits [4..7]. bit(7 - index)? */
        int shift = 7 - index;
        return (tia->pf0 & (1 << shift)) != 0;
    }
    else if (index < 12)
    {
        int shift = 11 - index;
        return (tia->pf1 & (1 << shift)) != 0;
    }
    else
    {
        int shift = 19 - index;
        return (tia->pf2 & (1 << shift)) != 0;
    }
}

/* -----------------------------------------------------------------------------
 * get_player_pixel
 *
 * Check if the 8-bit GRP is “on” at position x, given the sprite_x (RESPx).
 * Real TIA has "delayed" mode, repeating copies, stretching, etc.
 * We'll do a basic approach:
 *  - If x is in [sprite_x..sprite_x+7], we map bits of grp (from left to
 * right).
 * -----------------------------------------------------------------------------
 */
static bool get_player_pixel(uint8_t grp, int sprite_x, int x)
{
    int rel = x - sprite_x;
    if (rel < 0 || rel > 7)
        return false;
    /* bit 7 => leftmost pixel in usual approach. So we do: */
    int shift = 7 - rel;
    return (grp & (1 << shift)) != 0;
}

/* -----------------------------------------------------------------------------
 * tia_color_to_rgba
 *
 * Convert TIA color code [0..127] to RGBA. For demonstration,
 * we look up in a table.
 * -----------------------------------------------------------------------------
 */
static uint32_t tia_color_to_rgba(TIA *tia, uint8_t color_code)
{
    color_code &= 0x7F; /* restrict to [0..127]. */

    if (tia->tv_system == TV_SYSTEM_NTSC)
        return tia_color_lut_ntsc[color_code];
    else
        return tia_color_lut_pal[color_code];
}

/* -----------------------------------------------------------------------------
 * tia_get_framebuffer
 *
 * Returns a pointer to the 160x192 RGBA buffer, if needed by the host program
 * to render the final image.
 * -----------------------------------------------------------------------------
 */
const uint32_t *tia_get_framebuffer(TIA *tia)
{
    if (!tia)
        return NULL;
    return (const uint32_t *) tia->framebuffer;
}

/* -----------------------------------------------------------------------------
 * tia_audio_step
 *
 * A stub function to demonstrate how you might generate audio from TIA's
 * two channels. Real hardware uses polynomial counters for some waveforms.
 * This function would be called regularly at the host's audio sample rate,
 * e.g. 44,100 Hz, to fill an audio buffer.
 *
 * host_sample_rate = e.g. 44100
 * host_dt = 1.0f / host_sample_rate
 * left, right are pointers to accumulators or sample buffers.
 * -----------------------------------------------------------------------------
 */
void tia_audio_step(TIA *tia, float host_sample_rate, float host_dt,
                    float *left, float *right)
{
    (void) host_sample_rate;

    if (!tia || !left || !right)
        return;

    /* Example: for each channel, create a simple square wave.
       In real TIA, AUDC selects wave type, AUDF sets freq, AUDV sets volume. */

    /* Channel 0: */
    float freq0 = 30.0f + (tia->audio0.AUDF * 10.0f); /* example mapping */
    tia->audio0.phase += (freq0 * host_dt);
    if (tia->audio0.phase > 1.0f)
        tia->audio0.phase -= 1.0f;
    float sample0 = (tia->audio0.phase < 0.5f) ? 1.0f : -1.0f;
    sample0 *= (tia->audio0.AUDV / 15.0f); /* volume range 0..15? */

    /* Channel 1: */
    float freq1 = 30.0f + (tia->audio1.AUDF * 12.0f);
    tia->audio1.phase += (freq1 * host_dt);
    if (tia->audio1.phase > 1.0f)
        tia->audio1.phase -= 1.0f;
    float sample1 = (tia->audio1.phase < 0.5f) ? 1.0f : -1.0f;
    sample1 *= (tia->audio1.AUDV / 15.0f);

    /* Mix them. We'll do a simple L=R= sum of both channels. */
    float mix = sample0 + sample1;
    *left     = mix * 0.5f;
    *right    = mix * 0.5f;
}
