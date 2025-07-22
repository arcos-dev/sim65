; Ben Eater 6502 - Interrupt-driven Timer Example
; Uses VIA Timer 1 to generate regular interrupts
; Displays a blinking pattern on Port B LEDs
; Demonstrates proper interrupt handling
;
; Timer generates interrupts every ~0.1 seconds
; Each interrupt toggles different LED patterns

        .org $8000

; VIA 6522 registers
VIA_BASE = $6000
VIA_PORTB = VIA_BASE + $00
VIA_PORTA = VIA_BASE + $01
VIA_DDRB = VIA_BASE + $02
VIA_DDRA = VIA_BASE + $03
VIA_T1CL = VIA_BASE + $04  ; Timer 1 Counter Low
VIA_T1CH = VIA_BASE + $05  ; Timer 1 Counter High
VIA_T1LL = VIA_BASE + $06  ; Timer 1 Latch Low
VIA_T1LH = VIA_BASE + $07  ; Timer 1 Latch High
VIA_ACR = VIA_BASE + $0B   ; Auxiliary Control Register
VIA_IFR = VIA_BASE + $0D   ; Interrupt Flag Register
VIA_IER = VIA_BASE + $0E   ; Interrupt Enable Register

; Zero page variables
PATTERN = $00             ; Current LED pattern
COUNTER = $01             ; Pattern change counter

; Timer constants (for ~0.1 second intervals at 1MHz)
TIMER_LOW = $00           ; Low byte of timer value
TIMER_HIGH = $27          ; High byte (~10000 cycles)

RESET:
        ; Initialize CPU
        sei               ; Disable interrupts initially
        cld
        ldx #$ff
        txs

        ; Initialize VIA
        lda #$ff
        sta VIA_DDRB      ; Port B all outputs

        ; Initialize variables
        lda #%10101010    ; Initial pattern
        sta PATTERN
        lda #$00
        sta COUNTER

        ; Setup Timer 1 for continuous interrupts
        lda #%11000000    ; Timer 1 continuous, free-run mode
        sta VIA_ACR

        ; Load timer with initial value
        lda #TIMER_LOW
        sta VIA_T1CL      ; This starts the timer
        lda #TIMER_HIGH
        sta VIA_T1CH

        ; Enable Timer 1 interrupts
        lda #%11000000    ; Set bit 7 (enable) and bit 6 (Timer 1)
        sta VIA_IER

        ; Display initial pattern
        lda PATTERN
        sta VIA_PORTB

        cli               ; Enable interrupts

MAIN_LOOP:
        ; Main program can do other work here
        ; For now, just loop
        nop
        nop
        nop
        jmp MAIN_LOOP

; Interrupt Service Routine
IRQ_HANDLER:
        ; Save registers
        pha
        txa
        pha
        tya
        pha

        ; Check if this was a Timer 1 interrupt
        lda VIA_IFR
        and #%01000000    ; Timer 1 interrupt flag
        beq IRQ_EXIT      ; Not Timer 1, exit

        ; Clear the interrupt by reading T1CL
        lda VIA_T1CL      ; This clears the Timer 1 interrupt flag

        ; Update LED pattern
        inc COUNTER
        lda COUNTER
        and #%00000111    ; Use lower 3 bits for pattern selection
        tax
        lda PATTERNS,x    ; Get pattern from table
        sta PATTERN
        sta VIA_PORTB     ; Display new pattern

IRQ_EXIT:
        ; Restore registers
        pla
        tay
        pla
        tax
        pla
        rti

; Pattern table for different blink effects
PATTERNS:
        .byte %10101010   ; Alternating
        .byte %11001100   ; Double alternating
        .byte %11110000   ; Half and half
        .byte %10010010   ; Every third
        .byte %11111111   ; All on
        .byte %00000000   ; All off
        .byte %00110011   ; Double alternating (opposite)
        .byte %01010101   ; Alternating (opposite)

; Interrupt vectors
        .org $fffa
        .word RESET       ; NMI vector
        .word RESET       ; Reset vector
        .word IRQ_HANDLER ; IRQ vector
