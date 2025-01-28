/*******************************************************************************
 * palette.h
 *
 * Defines comprehensive color palettes for the Atari 2600 (128 colors each)
 * Each color is defined in RGBA format (0xRRGGBBAA)
 *  - 128 colors for NTSC
 *  - 128 colors for PAL
 *
 * Author: Anderson Costa
 * Date: 2025-01-05
 ******************************************************************************/

#ifndef PALETTE_H
#define PALETTE_H

#include <stdint.h>

#define COLOR_PALETTE_SIZE 128

// NTSC Palette (128 colors)
static const uint32_t COLOR_PALETTE_NTSC[COLOR_PALETTE_SIZE] = {
    0x444444FF, // Black
    0x504034FF, // Gray 1
    0x644834FF, // Gray 2
    0x804830FF, // Gray 3
    0x944820FF, // Gray 4s
    0xA44814FF, // Gray 5
    0xB04410FF, // Gray 6
    0xB84008FF, // Gray 7
    0xB84008FF, // Red 1
    0xB44420FF, // Red 2
    0xA84840FF, // Red 3
    0x944858FF, // Red 4
    0x7C4C70FF, // Red 5
    0x605084FF, // Red 6
    0x405090FF, // Red 7
    0x285094FF, // Red 8
    0x285094FF, // Blue 1
    0x085884FF, // Blue 2
    0x00606CFF, // Blue 3
    0x006850FF, // Blue 4
    0x007030FF, // Blue 5
    0x007814FF, // Blue 6
    0x187C00FF, // Blue 7
    0x387C00FF, // Blue 8
    0x547800FF, // Green 1
    0x6C7000FF, // Green 2
    0x846400FF, // Green 3
    0x9C5800FF, // Green 4
    0xB44C00FF, // Green 5
    0xC84400FF, // Green 6
    0xD83800FF, // Green 7
    0xE42400FF, // Green 8
    0xE42400FF, // Orange 1
    0xE03828FF, // Orange 2
    0xD4484CFF, // Orange 3
    0xC0586CFF, // Orange 4
    0xA46C88FF, // Orange 5
    0x847CA0FF, // Orange 6
    0x6088B0FF, // Orange 7
    0x3890BCFF, // Orange 8
    0x3890BCFF, // Light Blue 1
    0x1098A8FF, // Light Blue 2
    0x00A08CFF, // Light Blue 3
    0x00A870FF, // Light Blue 4
    0x00AC54FF, // Light Blue 5
    0x00B034FF, // Light Blue 6
    0x1CAC00FF, // Light Blue 7
    0x44AC00FF, // Light Blue 8
    0x68A800FF, // Yellow Green 1
    0x8C9C00FF, // Yellow Green 2
    0xAC9000FF, // Yellow Green 3
    0xCC8000FF, // Yellow Green 4
    0xE47000FF, // Yellow Green 5
    0xF86000FF, // Yellow Green 6
    0xFF4C00FF, // Yellow Green 7
    0xFF4018FF, // Yellow Green 8
    0xFF4018FF, // Yellow 1
    0xF84C40FF, // Yellow 2
    0xE86060FF, // Yellow 3
    0xD0747CFF, // Yellow 4
    0xB48898FF, // Yellow 5
    0x9498B0FF, // Yellow 6
    0x70A4C0FF, // Yellow 7
    0x48ACCCFF, // Yellow 8
    0x48ACCCFF, // Light Cyan 1
    0x20B4B8FF, // Light Cyan 2
    0x00BC9CFF, // Light Cyan 3
    0x00C07CFF, // Light Cyan 4
    0x00C458FF, // Light Cyan 5
    0x00C830FF, // Light Cyan 6
    0x34CC00FF, // Light Cyan 7
    0x5CCC00FF, // Light Cyan 8
    0x80C800FF, // Lime 1
    0xA4BC00FF, // Lime 2
    0xC4AC00FF, // Lime 3
    0xE09C00FF, // Lime 4
    0xF88800FF, // Lime 5
    0xFF7400FF, // Lime 6
    0xFF5C00FF, // Lime 7
    0xFF4C20FF, // Lime 8
    0xFF4C20FF, // Light Orange 1
    0xF85C54FF, // Light Orange 2
    0xE4747CFF, // Light Orange 3
    0xC88C98FF, // Light Orange 4
    0xA8A0B4FF, // Light Orange 5
    0x84B0CCFF, // Light Orange 6
    0x5EC0E0FF, // Light Orange 7
    0x34CCF0FF, // Light Orange 8
    0x34CCF0FF, // Light Violet 1
    0x0CD4DCFF, // Light Violet 2
    0x00D8C0FF, // Light Violet 3
    0x00DCA4FF, // Light Violet 4
    0x00E080FF, // Light Violet 5
    0x00E458FF, // Light Violet 6
    0x2CE828FF, // Light Violet 7
    0x5CE810FF, // Light Violet 8
    0x80E400FF, // Yellow 1
    0xA4E000FF, // Yellow 2
    0xC8D400FF, // Yellow 3
    0xE8C400FF, // Yellow 4
    0xFFB000FF, // Yellow 5
    0xFF9C00FF, // Yellow 6
    0xFF8400FF, // Yellow 7
    0xFF7024FF, // Yellow 8
    0xFF7024FF, // Pink 1
    0xF87C68FF, // Pink 2
    0xE4908CFF, // Pink 3
    0xC8A8A8FF, // Pink 4
    0xA4B8C4FF, // Pink 5
    0x80C8DCFF, // Pink 6
    0x58D8F0FF, // Pink 7
    0x2CE0FFFF, // Pink 8
    0x2CE0FFFF, // Light Pink 1
    0x00E8ECFF, // Light Pink 2
    0x00ECE0FF, // Light Pink 3
    0x00F0C8FF, // Light Pink 4
    0x00F4A0FF, // Light Pink 5
    0x00F874FF, // Light Pink 6
    0x20FC44FF, // Light Pink 7
    0x5CFC28FF, // Light Pink 8
    0x80FC08FF, // White 1
    0xA4FC00FF, // White 2
    0xC8F800FF, // White 3
    0xECE800FF, // White 4
    0xFFD400FF, // White 5
    0xFFBC00FF, // White 6
    0xFFA000FF, // White 7
    0xFF8C30FF  // White 8
};

// PAL Palette (128 colors)
static const uint32_t COLOR_PALETTE_PAL[COLOR_PALETTE_SIZE] = {
    0x444444FF, // Black
    0x444444FF, // Gray 1
    0x5C4024FF, // Gray 2
    0x743C14FF, // Gray 3
    0x8C3808FF, // Gray 4
    0xA03400FF, // Gray 5
    0xB03000FF, // Gray 6
    0xBC2800FF, // Gray 7
    0xBC2800FF, // Red 1
    0xB4281CFF, // Red 2
    0xA42C3CFF, // Red 3
    0x8C345CFF, // Red 4
    0x704078FF, // Red 5
    0x50488CFF, // Red 6
    0x34549CFF, // Red 7
    0x1C5CA8FF, // Red 8
    0x1C5CA8FF, // Blue 1
    0x086094FF, // Blue 2
    0x006078FF, // Blue 3
    0x005C58FF, // Blue 4
    0x005838FF, // Blue 5
    0x005818FF, // Blue 6
    0x106000FF, // Blue 7
    0x2C6800FF, // Blue 8
    0x446C00FF, // Green 1
    0x5C7000FF, // Green 2
    0x747400FF, // Green 3
    0x8C7800FF, // Green 4
    0xA07C00FF, // Green 5
    0xB48000FF, // Green 6
    0xC48000FF, // Green 7
    0xD07C00FF, // Green 8
    0xD07C00FF, // Orange 1
    0xCC6C00FF, // Orange 2
    0xC45800FF, // Orange 3
    0xBC4400FF, // Orange 4
    0xB03410FF, // Orange 5
    0xA02824FF, // Orange 6
    0x901C38FF, // Orange 7
    0x80104CFF, // Orange 8
    0x80104CFF, // Light Blue 1
    0x6C1464FF, // Light Blue 2
    0x581878FF, // Light Blue 3
    0x402088FF, // Light Blue 4
    0x2C2894FF, // Light Blue 5
    0x18349CFF, // Light Blue 6
    0x0040A0FF, // Light Blue 7
    0x004C9CFF, // Light Blue 8
    0x005890FF, // Yellow Green 1
    0x00647CFF, // Yellow Green 2
    0x007064FF, // Yellow Green 3
    0x007C48FF, // Yellow Green 4
    0x008824FF, // Yellow Green 5
    0x009400FF, // Yellow Green 6
    0x14A000FF, // Yellow Green 7
    0x30AC00FF, // Yellow Green 8
    0x4CBC00FF, // Yellow 1
    0x64C800FF, // Yellow 2
    0x7CC400FF, // Yellow 3
    0x94BC00FF, // Yellow 4
    0xACAC00FF, // Yellow 5
    0xC09C00FF, // Yellow 6
    0xD48800FF, // Yellow 7
    0xE47000FF, // Yellow 8
    0xE47000FF, // Light Orange 1
    0xE05800FF, // Light Orange 2
    0xD84000FF, // Light Orange 3
    0xCC2C00FF, // Light Orange 4
    0xBC1800FF, // Light Orange 5
    0xAC0800FF, // Light Orange 6
    0x980000FF, // Light Orange 7
    0x840008FF, // Light Orange 8
    0x840008FF, // Light Violet 1
    0x700028FF, // Light Violet 2
    0x580044FF, // Light Violet 3
    0x400060FF, // Light Violet 4
    0x2C0078FF, // Light Violet 5
    0x14008CFF, // Light Violet 6
    0x0000A0FF, // Light Violet 7
    0x0000B0FF, // Light Violet 8
    0x0000C4FF, // Violet 1
    0x0000D4FF, // Violet 2
    0x0000E4FF, // Violet 3
    0x0000F0FF, // Violet 4
    0x0000FCFF, // Violet 5
    0x0020FCFF, // Violet 6
    0x0040FCFF, // Violet 7
    0x005CFCFF, // Violet 8
    0x005CFCFF, // Light Pink 1
    0x0074E8FF, // Light Pink 2
    0x008CD4FF, // Light Pink 3
    0x00A4B4FF, // Light Pink 4
    0x00BC94FF, // Light Pink 5
    0x00D074FF, // Light Pink 6
    0x00E450FF, // Light Pink 7
    0x00F42CFF, // Light Pink 8
    0x00FC08FF, // White 1
    0x28FC00FF, // White 2
    0x48FC00FF, // White 3
    0x64FC00FF, // White 4
    0x80FC00FF, // White 5
    0x9CFC00FF, // White 6
    0xBCFC00FF, // White 7
    0xD4FC00FF  // White 8
};

#endif // PALETTE_H
