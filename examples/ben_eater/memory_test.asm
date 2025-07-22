; Ben Eater 6502 - Memory Test Program
; Tests RAM memory by writing and reading patterns
; Displays test progress on VIA Port B LEDs
; Tests memory from $0000 to $7FFF (32KB)
;
; LED patterns indicate:
; %10101010 = Testing in progress
; %11110000 = Test pattern write phase
; %00001111 = Test pattern read/verify phase
; %11111111 = All tests passed
; %00000000 = Test failed (stuck here)

        .org $8000

; VIA 6522 registers
VIA_BASE = $6000
VIA_PORTB = VIA_BASE + $00
VIA_DDRB = VIA_BASE + $02

; Memory test parameters
TEST_START = $0000          ; Start of RAM to test
TEST_END = $7FFF            ; End of RAM to test (32KB)
TEST_PATTERNS = 8           ; Number of test patterns

; Zero page variables (using high addresses to avoid test area)
CURRENT_ADDR = $F0          ; Current test address (16-bit)
CURRENT_ADDR_H = $F1
PATTERN_INDEX = $F2         ; Current test pattern index
ERROR_COUNT = $F3           ; Number of errors found
TEMP_VALUE = $F4            ; Temporary storage

RESET:
        sei
        cld
        ldx #$ff
        txs

        ; Initialize VIA
        lda #$ff
        sta VIA_DDRB      ; Port B all outputs

        ; Show startup pattern
        lda #%10101010
        sta VIA_PORTB
        jsr LONG_DELAY

        ; Initialize test variables
        lda #$00
        sta ERROR_COUNT
        sta PATTERN_INDEX

START_TEST:
        ; Show which test pattern we're running
        lda PATTERN_INDEX
        sta VIA_PORTB
        jsr SHORT_DELAY

PATTERN_LOOP:
        ; Get current test pattern
        ldx PATTERN_INDEX
        lda TEST_PATTERN_TABLE,x
        sta CURRENT_PATTERN

        ; Initialize address pointer
        lda #<TEST_START
        sta CURRENT_ADDR
        lda #>TEST_START
        sta CURRENT_ADDR_H

        ; Show write phase indicator
        lda #%11110000
        sta VIA_PORTB

WRITE_PHASE:
        ; Write pattern to current address
        lda CURRENT_PATTERN
        ldy #$00
        sta (CURRENT_ADDR),y

        ; Increment address
        jsr INCREMENT_ADDRESS

        ; Check if we've reached the end
        lda CURRENT_ADDR_H
        cmp #>TEST_END
        bcc WRITE_PHASE
        lda CURRENT_ADDR
        cmp #<TEST_END
        bcc WRITE_PHASE

        ; Show read phase indicator
        lda #%00001111
        sta VIA_PORTB

        ; Reset address for read phase
        lda #<TEST_START
        sta CURRENT_ADDR
        lda #>TEST_START
        sta CURRENT_ADDR_H

READ_PHASE:
        ; Read value from current address
        ldy #$00
        lda (CURRENT_ADDR),y
        sta TEMP_VALUE

        ; Compare with expected pattern
        cmp CURRENT_PATTERN
        beq READ_OK

        ; Error found!
        inc ERROR_COUNT
        jsr SHOW_ERROR

READ_OK:
        ; Increment address
        jsr INCREMENT_ADDRESS

        ; Check if we've reached the end
        lda CURRENT_ADDR_H
        cmp #>TEST_END
        bcc READ_PHASE
        lda CURRENT_ADDR
        cmp #<TEST_END
        bcc READ_PHASE

        ; Move to next test pattern
        inc PATTERN_INDEX
        lda PATTERN_INDEX
        cmp #TEST_PATTERNS
        bne PATTERN_LOOP

        ; All patterns tested - check results
        lda ERROR_COUNT
        beq ALL_TESTS_PASSED

        ; Some tests failed
        jsr SHOW_FAILURE
        jmp END_TESTS

ALL_TESTS_PASSED:
        ; Show success pattern
        lda #%11111111
        sta VIA_PORTB
        jmp END_TESTS

SHOW_ERROR:
        ; Flash error pattern
        pha
        lda #%11111111
        sta VIA_PORTB
        jsr SHORT_DELAY
        lda #%00000000
        sta VIA_PORTB
        jsr SHORT_DELAY
        pla
        rts

SHOW_FAILURE:
        ; Continuous blink to show failure
FAILURE_LOOP:
        lda #%11111111
        sta VIA_PORTB
        jsr LONG_DELAY
        lda #%00000000
        sta VIA_PORTB
        jsr LONG_DELAY
        jmp FAILURE_LOOP

INCREMENT_ADDRESS:
        inc CURRENT_ADDR
        bne INC_DONE
        inc CURRENT_ADDR_H
INC_DONE:
        rts

; Delay routines
SHORT_DELAY:
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

LONG_DELAY:
        pha
        ldx #$80
DELAY3:
        jsr SHORT_DELAY
        dex
        bne DELAY3
        pla
        rts

; Test patterns to write to memory
TEST_PATTERN_TABLE:
        .byte %00000000   ; All zeros
        .byte %11111111   ; All ones
        .byte %10101010   ; Alternating 1
        .byte %01010101   ; Alternating 2
        .byte %11001100   ; Double alternating 1
        .byte %00110011   ; Double alternating 2
        .byte %11110000   ; Half and half 1
        .byte %00001111   ; Half and half 2

CURRENT_PATTERN = $F5     ; Current test pattern

END_TESTS:
        nop               ; Infinite loop
        jmp END_TESTS

; Interrupt vectors
        .org $fffa
        .word RESET
        .word RESET
        .word RESET
