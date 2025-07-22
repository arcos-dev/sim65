; Ben Eater 6502 - Binary Counter Example
; Displays a binary counter on VIA Port B
; Based on Ben Eater's breadboard computer series
;
; This program counts from 0 to 255 in binary, displaying
; the result on the VIA 6522 Port B pins (LEDs)

        .org $8000         ; ROM start address

; VIA 6522 registers
VIA_BASE = $6000          ; Base address for VIA
VIA_PORTB = VIA_BASE + $00 ; Port B data register
VIA_PORTA = VIA_BASE + $01 ; Port A data register
VIA_DDRB = VIA_BASE + $02  ; Port B data direction register
VIA_DDRA = VIA_BASE + $03  ; Port A data direction register

COUNTER = $00             ; Zero page counter variable

RESET:
        ; Initialize CPU
        sei               ; Disable interrupts
        cld               ; Clear decimal mode
        ldx #$ff
        txs               ; Initialize stack pointer to $01FF

        ; Initialize VIA
        lda #$ff
        sta VIA_DDRB      ; Set Port B as all outputs (for LEDs)
        lda #$00
        sta VIA_DDRA      ; Set Port A as all inputs (for buttons/switches)

        ; Initialize counter
        lda #$00
        sta COUNTER

MAIN_LOOP:
        ; Display counter value on Port B (LEDs)
        lda COUNTER
        sta VIA_PORTB

        ; Simple delay loop
        jsr DELAY

        ; Increment counter
        inc COUNTER

        ; Loop forever
        jmp MAIN_LOOP

; Simple delay routine
DELAY:
        ldx #$ff          ; Outer loop counter
DELAY_OUTER:
        ldy #$ff          ; Inner loop counter
DELAY_INNER:
        dey
        bne DELAY_INNER
        dex
        bne DELAY_OUTER
        rts

; Interrupt vectors
        .org $fffa
        .word RESET       ; NMI vector
        .word RESET       ; Reset vector
        .word RESET       ; IRQ vector
