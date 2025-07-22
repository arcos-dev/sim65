; Ben Eater 6502 - Hello World LCD Example
; Displays "Hello, World!" on a 16x2 LCD via VIA 6522
; Based on Ben Eater's LCD interfacing videos
;
; LCD connections via VIA Port A:
; PA0-PA7: LCD data bus (D0-D7)
; PB0: LCD Register Select (RS)
; PB1: LCD Enable (E)
; PB2: LCD Read/Write (R/W)

        .org $8000

; VIA 6522 registers
VIA_BASE = $6000
VIA_PORTB = VIA_BASE + $00
VIA_PORTA = VIA_BASE + $01
VIA_DDRB = VIA_BASE + $02
VIA_DDRA = VIA_BASE + $03

; LCD control bits (Port B)
LCD_E  = %00000010        ; Enable bit (PB1)
LCD_RW = %00000100        ; Read/Write bit (PB2)
LCD_RS = %00000001        ; Register Select bit (PB0)

RESET:
        ; Initialize CPU
        sei
        cld
        ldx #$ff
        txs

        ; Initialize VIA
        lda #$ff
        sta VIA_DDRA      ; Port A all outputs (LCD data)
        lda #%00000111
        sta VIA_DDRB      ; PB0,PB1,PB2 outputs (LCD control)

        ; Initialize LCD
        jsr LCD_INIT

        ; Print "Hello, World!"
        ldx #$00
PRINT_LOOP:
        lda HELLO_MSG,x
        beq DONE          ; If zero, we're done
        jsr LCD_PRINT_CHAR
        inx
        jmp PRINT_LOOP

DONE:
        jmp DONE          ; Loop forever

; LCD initialization sequence
LCD_INIT:
        ; Wait for LCD to power up
        jsr DELAY_LONG

        ; Function Set: 8-bit, 2-line, 5x8 font
        lda #%00111000
        jsr LCD_COMMAND

        ; Display On/Off: Display on, cursor off, blink off
        lda #%00001100
        jsr LCD_COMMAND

        ; Entry Mode Set: Increment cursor, no shift
        lda #%00000110
        jsr LCD_COMMAND

        ; Clear Display
        lda #%00000001
        jsr LCD_COMMAND
        jsr DELAY_LONG    ; Clear needs extra time

        rts

; Send command to LCD (A = command byte)
LCD_COMMAND:
        sta VIA_PORTA     ; Put command on data bus
        lda #$00          ; RS=0 (command mode), RW=0 (write), E=0
        sta VIA_PORTB
        lda #LCD_E        ; Set Enable high
        sta VIA_PORTB
        lda #$00          ; Set Enable low
        sta VIA_PORTB
        jsr DELAY         ; Wait for command to execute
        rts

; Print character to LCD (A = character)
LCD_PRINT_CHAR:
        sta VIA_PORTA     ; Put character on data bus
        lda #LCD_RS       ; RS=1 (data mode), RW=0 (write), E=0
        sta VIA_PORTB
        lda #(LCD_RS | LCD_E) ; Set Enable high, keep RS high
        sta VIA_PORTB
        lda #LCD_RS       ; Set Enable low, keep RS high
        sta VIA_PORTB
        jsr DELAY
        rts

; Delay routines
DELAY:
        pha               ; Save A
        lda #$ff
DELAY1:
        sec
        sbc #$01
        bne DELAY1
        pla               ; Restore A
        rts

DELAY_LONG:
        pha
        ldx #$10          ; Longer delay
DELAY2:
        jsr DELAY
        dex
        bne DELAY2
        pla
        rts

; Message to display
HELLO_MSG:
        .byte $48, $65, $6C, $6C, $6F, $2C, $20    ; "Hello, "
        .byte $57, $6F, $72, $6C, $64, $21, $00    ; "World!", $00

; Interrupt vectors
        .org $fffa
        .word RESET
        .word RESET
        .word RESET
