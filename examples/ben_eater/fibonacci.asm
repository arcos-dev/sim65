; Ben Eater 6502 - Fibonacci Sequence Calculator
; Calculates and displays Fibonacci numbers
; Displays results on VIA Port B as binary LEDs
; Uses Port A bit 0 as a "next" button
;
; Fibonacci sequence: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233...

        .org $8000

; VIA 6522 registers
VIA_BASE = $6000
VIA_PORTB = VIA_BASE + $00  ; Output port (LEDs)
VIA_PORTA = VIA_BASE + $01  ; Input port (button)
VIA_DDRB = VIA_BASE + $02
VIA_DDRA = VIA_BASE + $03

; Zero page variables
FIB_CURR = $00            ; Current Fibonacci number
FIB_PREV = $01            ; Previous Fibonacci number
FIB_TEMP = $02            ; Temporary storage
BUTTON_STATE = $03        ; Previous button state

RESET:
        ; Initialize CPU
        sei
        cld
        ldx #$ff
        txs

        ; Initialize VIA
        lda #$ff
        sta VIA_DDRB      ; Port B all outputs (LEDs)
        lda #$00
        sta VIA_DDRA      ; Port A all inputs (button on PA0)

        ; Initialize Fibonacci sequence
        lda #$00
        sta FIB_PREV      ; F(0) = 0
        lda #$01
        sta FIB_CURR      ; F(1) = 1
        sta VIA_PORTB     ; Display F(1)

        ; Initialize button state
        lda VIA_PORTA
        and #$01
        sta BUTTON_STATE

MAIN_LOOP:
        ; Check for button press (PA0)
        jsr CHECK_BUTTON
        bcc MAIN_LOOP     ; If no press, keep checking

        ; Calculate next Fibonacci number
        jsr CALC_NEXT_FIB

        ; Display result
        lda FIB_CURR
        sta VIA_PORTB

        ; Small delay to debounce button
        jsr DELAY

        jmp MAIN_LOOP

; Check for button press (return with carry set if pressed)
CHECK_BUTTON:
        lda VIA_PORTA
        and #$01          ; Isolate PA0
        cmp BUTTON_STATE  ; Compare with previous state
        beq NO_PRESS      ; Same state = no change

        sta BUTTON_STATE  ; Update button state
        cmp #$01          ; Check if button is now pressed
        beq BUTTON_PRESSED

NO_PRESS:
        clc               ; Clear carry = no press
        rts

BUTTON_PRESSED:
        sec               ; Set carry = button pressed
        rts

; Calculate next Fibonacci number
; F(n) = F(n-1) + F(n-2)
CALC_NEXT_FIB:
        ; Check for overflow (if current > 127, reset sequence)
        lda FIB_CURR
        cmp #$80          ; Check if bit 7 is set (> 127)
        bcs RESET_SEQUENCE

        ; Calculate F(n-1) + F(n-2)
        lda FIB_CURR      ; A = F(n-1)
        sta FIB_TEMP      ; Save current as temp
        clc
        adc FIB_PREV      ; A = F(n-1) + F(n-2)
        sta FIB_CURR      ; Store new current
        lda FIB_TEMP
        sta FIB_PREV      ; Previous = old current
        rts

RESET_SEQUENCE:
        ; Reset to start of sequence when overflow occurs
        lda #$00
        sta FIB_PREV      ; F(0) = 0
        lda #$01
        sta FIB_CURR      ; F(1) = 1
        rts

; Simple delay routine
DELAY:
        pha
        ldx #$20
DELAY1:
        ldy #$ff
DELAY2:
        dey
        bne DELAY2
        dex
        bne DELAY1
        pla
        rts

; Interrupt vectors
        .org $fffa
        .word RESET
        .word RESET
        .word RESET
