; Ben Eater 6502 - Simple Calculator
; Performs basic arithmetic operations
; Uses VIA Port A for input (switches) and Port B for output (LEDs)
;
; Port A layout:
; PA0-PA3: First operand (0-15)
; PA4-PA5: Operation (00=ADD, 01=SUB, 10=MUL, 11=DIV)
; PA6-PA7: Second operand lower bits
;
; Additional input via reading Port A multiple times
; Port B shows result (0-255)

        .org $8000

; VIA 6522 registers
VIA_BASE = $6000
VIA_PORTB = VIA_BASE + $00  ; Result display (LEDs)
VIA_PORTA = VIA_BASE + $01  ; Input switches
VIA_DDRB = VIA_BASE + $02
VIA_DDRA = VIA_BASE + $03

; Zero page variables
OPERAND1 = $00              ; First operand
OPERAND2 = $01              ; Second operand
OPERATION = $02             ; Operation code
RESULT = $03                ; Calculation result
PREV_INPUT = $04            ; Previous input state

; Operation codes
OP_ADD = 0
OP_SUB = 1
OP_MUL = 2
OP_DIV = 3

RESET:
        sei
        cld
        ldx #$ff
        txs

        ; Initialize VIA
        lda #$ff
        sta VIA_DDRB      ; Port B all outputs (result LEDs)
        lda #$00
        sta VIA_DDRA      ; Port A all inputs (switches)

        ; Initialize variables
        lda #$00
        sta RESULT
        sta PREV_INPUT

        ; Show startup pattern
        lda #%10101010
        sta VIA_PORTB
        jsr DELAY

MAIN_LOOP:
        ; Read input from switches
        jsr READ_INPUTS

        ; Perform calculation
        jsr CALCULATE

        ; Display result
        lda RESULT
        sta VIA_PORTB

        ; Small delay before next calculation
        jsr SHORT_DELAY

        jmp MAIN_LOOP

; Read operands and operation from Port A
READ_INPUTS:
        ; Read current input
        lda VIA_PORTA

        ; Check if input has changed (simple debouncing)
        cmp PREV_INPUT
        beq INPUT_STABLE
        sta PREV_INPUT
        jsr DELAY         ; Debounce delay
        rts

INPUT_STABLE:
        ; Extract first operand (PA0-PA3)
        and #%00001111
        sta OPERAND1

        ; Read again for operation and second operand
        lda VIA_PORTA

        ; Extract operation (PA4-PA5)
        lsr               ; Shift right 4 times to get PA4-PA5 in bits 0-1
        lsr
        lsr
        lsr
        and #%00000011
        sta OPERATION

        ; For second operand, use PA6-PA7 as lower bits
        ; and re-read Port A for upper bits
        lda VIA_PORTA
        and #%11000000    ; PA6-PA7
        lsr               ; Shift to positions 4-5
        lsr
        sta OPERAND2

        ; Add some variety by using PA0-PA1 as upper bits of operand2
        lda VIA_PORTA
        and #%00000011    ; PA0-PA1
        asl               ; Shift to positions 2-3
        asl
        asl
        asl
        ora OPERAND2      ; Combine with lower bits
        and #%00001111    ; Keep it in 4-bit range
        sta OPERAND2

        rts

; Perform calculation based on operation
CALCULATE:
        lda OPERATION
        cmp #OP_ADD
        beq DO_ADD
        cmp #OP_SUB
        beq DO_SUB
        cmp #OP_MUL
        beq DO_MUL
        cmp #OP_DIV
        beq DO_DIV

        ; Invalid operation - show error pattern
        lda #%11110000
        sta RESULT
        rts

DO_ADD:
        lda OPERAND1
        clc
        adc OPERAND2
        sta RESULT
        rts

DO_SUB:
        lda OPERAND1
        sec
        sbc OPERAND2
        sta RESULT
        rts

DO_MUL:
        ; Simple multiplication by repeated addition
        lda #$00
        sta RESULT
        lda OPERAND2
        beq MUL_DONE      ; If multiplying by 0, result is 0

        tax               ; X = number of times to add
MUL_LOOP:
        lda RESULT
        clc
        adc OPERAND1
        sta RESULT
        dex
        bne MUL_LOOP

MUL_DONE:
        rts

DO_DIV:
        ; Simple division by repeated subtraction
        lda OPERAND2
        beq DIV_BY_ZERO   ; Check for division by zero

        lda OPERAND1
        sta TEMP_DIVIDEND
        lda #$00
        sta RESULT        ; Result will count subtractions

DIV_LOOP:
        lda TEMP_DIVIDEND
        cmp OPERAND2      ; Compare with divisor
        bcc DIV_DONE      ; If less than divisor, we're done

        sec
        sbc OPERAND2      ; Subtract divisor
        sta TEMP_DIVIDEND
        inc RESULT        ; Increment quotient
        jmp DIV_LOOP

DIV_DONE:
        rts

DIV_BY_ZERO:
        ; Show error pattern for division by zero
        lda #%11111111
        sta RESULT
        rts

TEMP_DIVIDEND = $05

; Delay routines
SHORT_DELAY:
        pha
        lda #$10
DELAY1:
        sec
        sbc #$01
        bne DELAY1
        pla
        rts

DELAY:
        pha
        ldx #$40
DELAY2:
        jsr SHORT_DELAY
        dex
        bne DELAY2
        pla
        rts

; Interrupt vectors
        .org $fffa
        .word RESET
        .word RESET
        .word RESET
