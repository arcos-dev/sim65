; Simple TIA demo - displays colored background and playfield
; Assembled for 6502 processor, loads at $F000
; Author: Anderson Costa

; TIA Registers
VSYNC  EQU $00
VBLANK EQU $01
WSYNC  EQU $02
RSYNC  EQU $03
COLUP0 EQU $06
COLUP1 EQU $07
COLUPF EQU $08
COLUBK EQU $09
CTRLPF EQU $0A
PF0    EQU $0D
PF1    EQU $0E
PF2    EQU $0F
RESP0  EQU $10
RESP1  EQU $11

; Start at $F000
    ORG $F000

START:
    CLD                 ; Clear decimal mode
    LDX #$FF           ; Initialize stack pointer
    TXS

    ; Clear TIA registers
    LDX #$00
    LDA #$00
CLEAR_LOOP:
    STA $00,X
    INX
    BPL CLEAR_LOOP

MAIN_LOOP:
    ; Start vertical sync (3 scanlines)
    LDA #$02           ; VSYNC on
    STA VSYNC

    ; Wait 3 scanlines for VSYNC
    STA WSYNC          ; Wait for sync
    STA WSYNC
    STA WSYNC

    LDA #$00           ; VSYNC off
    STA VSYNC

    ; Vertical blank (37 scanlines)
    LDA #$02           ; VBLANK on
    STA VBLANK

    ; Set colors during VBLANK
    LDA #$0E           ; White background
    STA COLUBK

    LDA #$44           ; Blue playfield
    STA COLUPF

    ; Set playfield pattern
    LDA #$F0           ; Left half playfield
    STA PF0
    LDA #$FF           ; Middle playfield
    STA PF1
    LDA #$0F           ; Right half playfield
    STA PF2

    ; Wait for end of VBLANK (37 scanlines)
    LDX #$25           ; 37 lines
VBLANK_LOOP:
    STA WSYNC
    DEX
    BNE VBLANK_LOOP

    LDA #$00           ; VBLANK off
    STA VBLANK

    ; Visible area (192 scanlines)
    LDX #$C0           ; 192 lines
VISIBLE_LOOP:
    STA WSYNC          ; Wait for horizontal sync
    DEX
    BNE VISIBLE_LOOP

    ; Overscan (30 scanlines)
    LDX #$1E           ; 30 lines
OVERSCAN_LOOP:
    STA WSYNC
    DEX
    BNE OVERSCAN_LOOP

    JMP MAIN_LOOP      ; Repeat frame

; Reset vectors
    ORG $FFFC
    DW START        ; Reset vector
    DW START        ; IRQ vector