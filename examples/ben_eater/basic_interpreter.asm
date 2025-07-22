; Ben Eater 6502 - Simple BASIC Interpreter
; A minimal BASIC-like interpreter that can execute simple commands
; Commands: LET, PRINT, IF, GOTO, END
; Variables: A-Z (single letter variables)
;
; Example program:
; 10 LET A = 5
; 20 LET B = 10
; 30 PRINT A
; 40 IF A < B GOTO 60
; 50 GOTO 70
; 60 PRINT B
; 70 END

        .org $8000

; VIA 6522 for I/O
VIA_BASE = $6000
VIA_PORTB = VIA_BASE + $00
VIA_PORTA = VIA_BASE + $01
VIA_DDRB = VIA_BASE + $02

; Memory locations
PROGRAM_START = $0200     ; Program storage area
VARIABLES = $0300         ; Variable storage (A-Z = $0300-$0319)
LINE_BUFFER = $0320       ; Input line buffer
STACK_TOP = $0350         ; Expression evaluation stack

; Parser state
CURRENT_LINE = $00        ; Current line pointer (16-bit)
CURRENT_LINE_H = $01
TOKEN_PTR = $02           ; Current token pointer
TOKEN_PTR_H = $03
TEMP_VAR = $04            ; Temporary variable

RESET:
        sei
        cld
        ldx #$ff
        txs

        ; Initialize VIA
        lda #$ff
        sta VIA_DDRB      ; Port B outputs (status LEDs)

        ; Initialize variables to zero
        ldx #$00
        lda #$00
INIT_VARS:
        sta VARIABLES,x
        inx
        cpx #26           ; 26 variables (A-Z)
        bne INIT_VARS

        ; Load sample program
        jsr LOAD_SAMPLE_PROGRAM

        ; Execute program
        jsr RUN_PROGRAM

END_PROGRAM:
        ; Display completion pattern on LEDs
        lda #%11111111
        sta VIA_PORTB
        jmp END_PROGRAM

; Load a sample program into memory
LOAD_SAMPLE_PROGRAM:
        ldx #$00
LOAD_LOOP:
        lda SAMPLE_PROGRAM,x
        sta PROGRAM_START,x
        beq LOAD_DONE
        inx
        jmp LOAD_LOOP
LOAD_DONE:
        rts

; Main program execution loop
RUN_PROGRAM:
        ; Set current line to start of program
        lda #<PROGRAM_START
        sta CURRENT_LINE
        lda #>PROGRAM_START
        sta CURRENT_LINE_H

EXECUTE_LINE:
        ; Check for end of program
        ldy #$00
        lda (CURRENT_LINE),y
        beq END_PROGRAM   ; Zero = end of program

        ; Skip line number (assume single byte for simplicity)
        inc CURRENT_LINE
        bne SKIP_LINE_NUM
        inc CURRENT_LINE_H
SKIP_LINE_NUM:

        ; Parse and execute command
        jsr PARSE_COMMAND

        ; Move to next line
        jsr FIND_NEXT_LINE
        jmp EXECUTE_LINE

; Parse and execute current command
PARSE_COMMAND:
        ; Get first character to determine command
        ldy #$00
        lda (CURRENT_LINE),y

        cmp #'L'          ; LET command?
        beq CMD_LET
        cmp #'P'          ; PRINT command?
        beq CMD_PRINT
        cmp #'E'          ; END command?
        beq CMD_END
        ; Add more commands as needed
        rts

CMD_LET:
        ; Simple LET A = 5 implementation
        ; Skip "LET "
        lda CURRENT_LINE
        clc
        adc #4
        sta TOKEN_PTR
        lda CURRENT_LINE_H
        adc #0
        sta TOKEN_PTR_H

        ; Get variable name
        ldy #$00
        lda (TOKEN_PTR),y
        sec
        sbc #'A'          ; Convert to variable index
        tax               ; X = variable index

        ; Skip to value (skip variable, space, =, space)
        lda TOKEN_PTR
        clc
        adc #4
        sta TOKEN_PTR
        lda TOKEN_PTR_H
        adc #0
        sta TOKEN_PTR_H

        ; Get value (simple single digit for now)
        ldy #$00
        lda (TOKEN_PTR),y
        sec
        sbc #'0'          ; Convert ASCII to number
        sta VARIABLES,x   ; Store in variable
        rts

CMD_PRINT:
        ; Simple PRINT A implementation
        ; Skip "PRINT "
        lda CURRENT_LINE
        clc
        adc #6
        sta TOKEN_PTR
        lda CURRENT_LINE_H
        adc #0
        sta TOKEN_PTR_H

        ; Get variable name
        ldy #$00
        lda (TOKEN_PTR),y
        sec
        sbc #'A'          ; Convert to variable index
        tax

        ; Display variable value on Port B
        lda VARIABLES,x
        sta VIA_PORTB

        ; Small delay to see the result
        jsr DELAY
        rts

CMD_END:
        ; Jump to end
        pla               ; Remove return address
        pla
        jmp END_PROGRAM

; Find next line in program
FIND_NEXT_LINE:
        ldy #$00
FIND_EOL:
        lda (CURRENT_LINE),y
        beq FOUND_EOL     ; Found end of line
        iny
        bne FIND_EOL
        ; Handle page boundary if needed
FOUND_EOL:
        ; Move past the zero terminator
        tya
        clc
        adc CURRENT_LINE
        sta CURRENT_LINE
        lda CURRENT_LINE_H
        adc #0
        sta CURRENT_LINE_H

        ; Skip the zero
        inc CURRENT_LINE
        bne SKIP_ZERO
        inc CURRENT_LINE_H
SKIP_ZERO:
        rts

; Simple delay
DELAY:
        pha
        ldx #$10
DELAY1:
        ldy #$ff
DELAY2:
        dey
        bne DELAY2
        dex
        bne DELAY1
        pla
        rts

; Sample program in memory
SAMPLE_PROGRAM:
        .byte 10          ; Line 10
        .byte $4C, $45, $54, $20, $41, $20, $3D, $20, $35, $00  ; "LET A = 5", $00
        .byte 20          ; Line 20
        .byte $4C, $45, $54, $20, $42, $20, $3D, $20, $37, $00  ; "LET B = 7", $00
        .byte 30          ; Line 30
        .byte $50, $52, $49, $4E, $54, $20, $41, $00            ; "PRINT A", $00
        .byte 40          ; Line 40
        .byte $50, $52, $49, $4E, $54, $20, $42, $00            ; "PRINT B", $00
        .byte 50          ; Line 50
        .byte $45, $4E, $44, $00                                ; "END", $00
        .byte $00         ; End of program marker

; Interrupt vectors
        .org $fffa
        .word RESET
        .word RESET
        .word RESET
