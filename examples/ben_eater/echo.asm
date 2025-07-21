; Ben Eater 6502 - Serial Echo Example (VIA 6522)
; Assemble with: vasm6502_oldstyle -Fbin -dotdir -o echo.bin echo.asm

        .org $8000         ; Typical ROM address for Ben Eater

RESET:  sei               ; Disable interrupts
        ldx #$ff
        txs               ; Initialize stack pointer

LOOP:   lda VIA_IFR       ; Clear SR interrupt flag
        lda VIA_SR        ; Read received byte (SR)
        sta VIA_TMP       ; Save temporarily

WAIT_TX:
        lda VIA_IFR
        and #$10          ; SR interrupt flag
        beq WAIT_TX       ; Wait until transmission is ready

        lda VIA_TMP
        sta VIA_SR        ; Write to SR to transmit
        jmp LOOP

VIA_BASE = $E000
VIA_SR   = VIA_BASE + $0A
VIA_IFR  = VIA_BASE + $0D
VIA_TMP  = $00            ; Zero page temp

        .org $FFFC
        .word RESET       ; Reset vector
        .word RESET       ; IRQ/NMI vector 