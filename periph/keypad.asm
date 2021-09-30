;;
;; keypad.asm: Boilerplate code for keypad procedures
;;
;;  Created on:  July 21, 2019
;;      Author: khughes
;;
        .thumb                          ; generate Thumb-2 code
        .text                           ; switch to code (ROM) section

;; Place declarations and references for variables here.

        .align  4                       ; force word alignment
;; Place definitions for peripheral registers here (base
;; addresses, register offsets, and field constants).  Follow the
;; example for the SYSCTL RCGCCPIO register from Lab 3.
SYSCTL:                 .word   0x400fe000
SYSCTL_RCGCGPIO         .equ    0x608
SYSCTL_RCGCGPIO_PORTC   .equ    (1<<10)

GPIO_PORTA:				.word	0x40058000		; added Port A GPIO base address
GPIO_DATA				.equ	0x000		; added GPIO_DATA offset
GPIO_DIR				.equ	0x400		; added GPIO_DIR offset
GPIO_AFSEL				.equ	0x420		; added GPIO_AFSEL offset
GPIO_ODR				.equ	0x50c		; added GPIO_ODR offset
GPIO_PUR				.equ	0x510		; added GPIO_PUR offset
GPIO_DEN				.equ	0x51c		; added GPIO_DEN offset

; Declare global variables
CPORT:					.word	cport
RPORT:					.word	rport
LASTROW:				.word	lastrow

;; void initRCGCGPIO( volatile uint32_t * base );
        .align  4                       ; force word alignment

initRCGCGPIO:
		.asmfunc

		push    {LR}             ; save return address

		; R0 = base

		; Enable gate clock of specified base address
		; R1 = GPIO_PORTA base address
		ldr		R1, GPIO_PORTA			; load base address of port A
		sub		R0, R0, R1
		lsr		R0, R0, #12
		mov		R1, #1
		lsl		R0, R1, R0				; 1 << (((uint32_t)base - (uint32_t)GPIO_PORTA) >> 12)

		; R2 = SYSCTL[RCGC_GPIO]
        ldr     R1, SYSCTL              	; get SYSCTL base address
        ldr     R2, [R1, #SYSCTL_RCGCGPIO] 	; read RCGC_GPIO
        orr     R2, R0						; OR in the bits
        str     R2, [R1, #SYSCTL_RCGCGPIO] 	; store RCGC_GPIO

        pop     {PC}             ; return

        .endasmfunc

;; void setrow( uint8_t row );
        .align  4                       ; force word alignment

setrow:
		.asmfunc

		push    {LR, R4-R12}             ; save return address and registers

		; R0 = row

		; pins = rport->pins[0] | rport->pins[1] | rport->pins[2] | rport->pins[3];
		; R1 = rport base address
		; R2 = pins
		ldr		R1, RPORT
		ldr		R1, [R1]
		ldrb	R2, [R1, #0]
		ldrb	R3, [R1, #1]
		ldrb	R4, [R1, #2]
		ldrb	R5, [R1, #3]
		orr		R2, R2, R3
		orr		R2, R2, R4
		orr		R2, R2, R5

		; lastrow = row & 0x03;
		; R3 = lastrow base address
		; R4 = lastrow
		ldr		R3, LASTROW
		ldrb	R4, [R3]
		and		R4, R0, #0x03
		strb	R4, [R3]

		; rport->base[pins] = pins ^ rport->pins[lastrow];
		; R5 = rport->base
		; R6 = pins ^ rport->pins[lastrow]
		ldr		R5, [R1, #4]
		ldrb	R6, [R1, R4]
		eor		R6, R6, R2
		str		R6, [R5, R2, LSL #2]

        pop     {PC, R4-R12}             ; return program counter and registers

        .endasmfunc

;; void initKeypad( const struct portinfo *col, const struct portinfo *row );
        .align  4                       ; force word alignment
        .global initKeypad	; added global label to be able to call this function

initKeypad:
		.asmfunc

		push    {LR, R4-R12}             ; save return address and registers

		; R0 = *c	base address
		; R1 = *r	base address
		; R2 = pins

		; save c & r in cport and rport
		ldr		R3, CPORT
		ldr		R4, RPORT
		str		R0, [R3]
		str		R1, [R4]

		; initRCGCGPIO( cport->base );
		push	{R0-R3}
		ldr		R0, [R0, #4]
		bl		initRCGCGPIO
		pop		{R0-R3}

		; initRCGCGPIO( rport->base );
		push	{R0-R3}
		mov		R0, R1
		ldr		R0, [R0, #4]
		bl		initRCGCGPIO
		pop		{R0-R3}

;; ROWS initialization
		; pins = rport->pins[0] | rport->pins[1] | rport->pins[2] | rport->pins[3];
		ldrb	R2, [R1, #0]
		ldrb	R3, [R1, #1]
		ldrb	R4, [R1, #2]
		ldrb	R5, [R1, #3]
		orr		R2, R2, R3
		orr		R2, R2, R4
		orr		R2, R2, R5

		; load rport->base
		ldr		R1, [R1, #4]

		; set direction as output
		ldr     R3, [R1, #GPIO_DIR] 		; read GPIO_DIR
        orr     R3, R2						; OR in the bits
        str     R3, [R1, #GPIO_DIR] 		; store GPIO_DIR

        ; turn off pull-ups
		ldr     R3, [R1, #GPIO_PUR] 		; read GPIO_PUR
        bic     R3, R2						; clear bits
        str     R3, [R1, #GPIO_PUR] 		; store GPIO_PUR

        ; ensure open-drain
		ldr     R3, [R1, #GPIO_ODR] 		; read GPIO_ODR
        orr     R3, R2						; OR in the bits
        str     R3, [R1, #GPIO_ODR] 		; store GPIO_ODR

        ; turn off alternate functions
		ldr     R3, [R1, #GPIO_AFSEL] 		; read GPIO_AFSEL
        bic     R3, R2						; clear bits
        str     R3, [R1, #GPIO_AFSEL] 		; store GPIO_AFSEL

        ; enable row pins
		ldr     R3, [R1, #GPIO_DEN] 		; read GPIO_DEN
        orr     R3, R2						; OR in the bits
        str     R3, [R1, #GPIO_DEN] 		; store GPIO_DEN

;; COLS initialization
		; pins = cport->pins[0] | cport->pins[1] | cport->pins[2] | cport->pins[3];
		ldrb	R2, [R0, #0]
		ldrb	R3, [R0, #1]
		ldrb	R4, [R0, #2]
		ldrb	R5, [R0, #3]
		orr		R2, R2, R3
		orr		R2, R2, R4
		orr		R2, R2, R5

		; load cport->base
		ldr		R0, [R0, #4]

		; set direction as output
		ldr     R3, [R0, #GPIO_DIR] 		; read GPIO_DIR
        bic     R3, R2						; OR in the bits
        str     R3, [R0, #GPIO_DIR] 		; store GPIO_DIR

        ; turn off pull-ups
		ldr     R3, [R0, #GPIO_PUR] 		; read GPIO_PUR
        orr     R3, R2						; clear bits
        str     R3, [R0, #GPIO_PUR] 		; store GPIO_PUR

        ; ensure open-drain
		ldr     R3, [R0, #GPIO_ODR] 		; read GPIO_ODR
        bic     R3, R2						; OR in the bits
        str     R3, [R0, #GPIO_ODR] 		; store GPIO_ODR

        ; turn off alternate functions
		ldr     R3, [R0, #GPIO_AFSEL] 		; read GPIO_AFSEL
        bic     R3, R2						; clear bits
        str     R3, [R0, #GPIO_AFSEL] 		; store GPIO_AFSEL

        ; enable cols pins
		ldr     R3, [R0, #GPIO_DEN] 		; read GPIO_DEN
        orr     R3, R2						; OR in the bits
        str     R3, [R0, #GPIO_DEN] 		; store GPIO_DEN

		; setrow(0)
		mov		R0, #0
		bl		setrow

        pop     {PC, R4-R12}             ; return program counter and registers

        .endasmfunc

;; bool getKey( uint8_t *col, uint8_t *row );
        .align  4                       ; force word alignment
        .global getKey					; added global label to be able to call this function
getKey:
		.asmfunc

		push    {LR, R4-R12}             ; save return address and registers

		; R0 = address of col
		; R1 = address of row
		; R2 = cport base address
		; R3 = pins
		; R4 = cport->base
		; R5 = cport->base[pins]
		; R6 = lastrow

		; R9 = cport->base[cport->pins[i]]
		; R10 = cport->pins[i]
		; R12 = i

		; pins = cport->pins[0] | cport->pins[1] | cport->pins[2] | cport->pins[3];
		ldr		R2, CPORT
		ldr		R2, [R2]
		ldrb	R3, [R2, #0]
		ldrb	R4, [R2, #1]
		ldrb	R5, [R2, #2]
		ldrb	R6, [R2, #3]
		orr		R3, R3, R4
		orr		R3, R3, R5
		orr		R3, R3, R6

		; cport->base[pins]
		ldr		R4, [R2, #4]
		ldr		R5, [R4, R3, LSL #2]

		; lastrow;
		ldr		R6, LASTROW
		ldrb	R6, [R6]

		; if( cport->base[pins] == pins )
		cmp 	R5, R3
		bne		if_end_1

		add		R0, R6, #1				; lastrow + 1
		bl		setrow					; setrow( lastrow + 1 )

		mov		R0, #0					; return false
		pop     {PC, R4-R12}            ; return program counter and registers

; end of if block 1
if_end_1:
		strb	R6, [R1]				;*row = lastrow;
		mov		R12, #0					; R12 = i

; start of for loop
for_loop:
		cmp 	R12, #3
		bge		end_for					; if(i >= 3) end for loop

		ldrb	R10, [R2, R12]			; cport->pins[i]
		ldrb	R9, [R4, R10, LSL #2]	; cport->base[cport->pins[i]]
		cmp		R9, #0
		bne		if_end_2				; if( cport->base[cport->pins[i]] == 0 )

		strb	R12, [R0]				; *col = i;
		mov		R0, #1					; return true

		pop     {PC, R4-R12}            ; return program counter and registers

; end of if block 2
if_end_2:
		add		R12, R12, #1			; i++
		b		for_loop				; branch to top of loop

; end of for loop
end_for:
		mov		R11, #3
		strb	R11, [R0]				; *col = 3;
		mov		R0, #1					; return true

		pop     {PC, R4-R12}            ; return program counter and registers

        .endasmfunc

        .data                           ; switch to data (RAM) section
        .align  4                       ; force word alignment

;; Place any variables stored in RAM here

		.bss	cport,4,4	;static const struct portinfo *cport;
		.bss	rport,4,4	;static const struct portinfo *rport;
		.bss	lastrow,1,1 ;static uint8_t lastrow;

        .end
