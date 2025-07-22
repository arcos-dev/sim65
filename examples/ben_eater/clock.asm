; Ben Eater 6502 - Digital Clock/Stopwatch
; Implements a simple digital clock using VIA Timer
; Displays seconds on Port B LEDs (binary)
; Uses Port A for control buttons:
; PA0: Start/Stop
; PA1: Reset
; PA2: Mode (Clock/Stopwatch)

        .org $8000

; VIA 6522 registers
VIA_BASE = $6000
VIA_PORTB = VIA_BASE + $00  ; LED display (seconds)
VIA_PORTA = VIA_BASE + $01  ; Control buttons
VIA_DDRB = VIA_BASE + $02
VIA_DDRA = VIA_BASE + $03
VIA_T1CL = VIA_BASE + $04   ; Timer 1 Counter Low
VIA_T1CH = VIA_BASE + $05   ; Timer 1 Counter High
VIA_ACR = VIA_BASE + $0B    ; Auxiliary Control Register
VIA_IFR = VIA_BASE + $0D    ; Interrupt Flag Register
VIA_IER = VIA_BASE + $0E    ; Interrupt Enable Register

; Zero page variables
SECONDS = $00               ; Current seconds (0-59)
MINUTES = $01               ; Current minutes (0-59)
HOURS = $02                 ; Current hours (0-23)
TICK_COUNT = $03            ; Interrupt counter for 1-second timing
CLOCK_RUNNING = $04         ; 0=stopped, 1=running
CLOCK_MODE = $05            ; 0=stopwatch, 1=clock
BUTTON_STATE = $06          ; Previous button states
DISPLAY_MODE = $07          ; 0=seconds, 1=minutes, 2=hours

; Timer constants for 1-second intervals
; At 1MHz, we need 1,000,000 cycles per second
; VIA timer counts down, so we need to divide this
; Using Timer 1 with prescaler: 10,000 ticks per interrupt
; 100 interrupts = 1 second
TIMER_LOW = $10             ; Low byte (16 decimal = $10)
TIMER_HIGH = $27            ; High byte (39 decimal = $27)
                            ; Total: $2710 = 10,000 decimal
TICKS_PER_SECOND = 100      ; Number of timer interrupts per second

RESET:
        sei
        cld
        ldx #$ff
        txs

        ; Initialize VIA
        lda #$ff
        sta VIA_DDRB      ; Port B all outputs (LEDs)
        lda #$00
        sta VIA_DDRA      ; Port A all inputs (buttons)

        ; Initialize clock variables
        lda #$00
        sta SECONDS
        sta MINUTES
        sta HOURS
        sta TICK_COUNT
        sta CLOCK_RUNNING
        sta CLOCK_MODE    ; Start in stopwatch mode
        sta DISPLAY_MODE  ; Start displaying seconds

        ; Setup Timer 1
        lda #%11000000    ; Timer 1 continuous mode
        sta VIA_ACR

        ; Load timer
        lda #TIMER_LOW
        sta VIA_T1CL
        lda #TIMER_HIGH
        sta VIA_T1CH

        ; Enable Timer 1 interrupts
        lda #%11000000    ; Enable Timer 1 interrupts
        sta VIA_IER

        ; Read initial button state
        lda VIA_PORTA
        and #%00000111    ; Mask to 3 buttons
        sta BUTTON_STATE

        ; Display initial time
        jsr UPDATE_DISPLAY

        cli               ; Enable interrupts

MAIN_LOOP:
        ; Check buttons
        jsr CHECK_BUTTONS

        ; Update display
        jsr UPDATE_DISPLAY

        ; Small delay
        jsr SHORT_DELAY

        jmp MAIN_LOOP

; Check for button presses
CHECK_BUTTONS:
        lda VIA_PORTA
        and #%00000111    ; Mask to 3 buttons
        sta TEMP_BUTTONS

        ; Check Start/Stop button (PA0)
        lda TEMP_BUTTONS
        and #%00000001
        beq CHECK_RESET   ; Button not pressed

        lda BUTTON_STATE
        and #%00000001
        bne CHECK_RESET   ; Button was already pressed

        ; Button just pressed - toggle running state
        lda CLOCK_RUNNING
        eor #$01
        sta CLOCK_RUNNING

CHECK_RESET:
        ; Check Reset button (PA1)
        lda TEMP_BUTTONS
        and #%00000010
        beq CHECK_MODE    ; Button not pressed

        lda BUTTON_STATE
        and #%00000010
        bne CHECK_MODE    ; Button was already pressed

        ; Reset clock
        lda #$00
        sta SECONDS
        sta MINUTES
        sta HOURS
        sta TICK_COUNT

CHECK_MODE:
        ; Check Mode button (PA2)
        lda TEMP_BUTTONS
        and #%00000100
        beq BUTTONS_DONE  ; Button not pressed

        lda BUTTON_STATE
        and #%00000100
        bne BUTTONS_DONE  ; Button was already pressed

        ; Cycle display mode
        inc DISPLAY_MODE
        lda DISPLAY_MODE
        cmp #3
        bne BUTTONS_DONE
        lda #$00
        sta DISPLAY_MODE

BUTTONS_DONE:
        lda TEMP_BUTTONS
        sta BUTTON_STATE
        rts

TEMP_BUTTONS = $08          ; Temporary button state

; Update LED display based on current mode
UPDATE_DISPLAY:
        lda DISPLAY_MODE
        beq DISPLAY_SECONDS
        cmp #1
        beq DISPLAY_MINUTES
        ; Display hours
        lda HOURS
        jmp SHOW_VALUE

DISPLAY_MINUTES:
        lda MINUTES
        jmp SHOW_VALUE

DISPLAY_SECONDS:
        lda SECONDS

SHOW_VALUE:
        sta VIA_PORTB
        rts

; Timer interrupt handler
IRQ_HANDLER:
        pha
        txa
        pha
        tya
        pha

        ; Check if Timer 1 interrupt
        lda VIA_IFR
        and #%01000000
        beq IRQ_EXIT

        ; Clear interrupt
        lda VIA_T1CL

        ; Only count if clock is running
        lda CLOCK_RUNNING
        beq IRQ_EXIT

        ; Increment tick counter
        inc TICK_COUNT
        lda TICK_COUNT
        cmp #TICKS_PER_SECOND
        bne IRQ_EXIT

        ; One second has elapsed
        lda #$00
        sta TICK_COUNT

        ; Increment seconds
        inc SECONDS
        lda SECONDS
        cmp #60
        bne IRQ_EXIT

        ; Minute overflow
        lda #$00
        sta SECONDS
        inc MINUTES
        lda MINUTES
        cmp #60
        bne IRQ_EXIT

        ; Hour overflow
        lda #$00
        sta MINUTES
        inc HOURS
        lda HOURS
        cmp #24
        bne IRQ_EXIT

        ; Day overflow (reset to 0 for stopwatch, or continue for clock)
        lda CLOCK_MODE
        bne IRQ_EXIT      ; In clock mode, keep going
        lda #$00          ; In stopwatch mode, reset
        sta HOURS

IRQ_EXIT:
        pla
        tay
        pla
        tax
        pla
        rti

; Short delay routine
SHORT_DELAY:
        pha
        lda #$10
DELAY_LOOP:
        sec
        sbc #$01
        bne DELAY_LOOP
        pla
        rts

; Interrupt vectors
        .org $fffa
        .word RESET       ; NMI
        .word RESET       ; Reset
        .word IRQ_HANDLER ; IRQ
