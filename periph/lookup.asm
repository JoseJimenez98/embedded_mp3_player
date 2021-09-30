;; Boilerplate code for lookup procedure

        .thumb                          ; generate Thumb-2 code

        .text                           ; switch to code (ROM) section

;; Place declarations and references for variables here.
		.align  4                       ; force word alignment

;; Place code for lookup here
        .align  4                       ; force word alignment


        .global lookup	; added global label to be able to call this function
lookup:
		.asmfunc

		push    {LR,R4-R12}             ; save return address and other registers

		; Evaluate SIZE-1
		sub		R4, R1, #1		; (SIZE-1)

		; Check if size is not over 100
		cmp		R1, #100
		bgt		else_block		; (SIZE) > 100    NOTE: Here it's opposite though

		; Check is both row and col is bound
		cmp		R2, R4
		bgt		else_block		; ROW > (SIZE-1)    NOTE: Here it's opposite though

		cmp		R3, R4
		bgt		else_block		; COL > (SIZE-1)    NOTE: Here it's opposite though

		; return value from table at table[row][column]
		mul		R4, R1, R2	; SIZE*row
		add		R4, R4, R3	; SIZE*row + column

		ldrb		R0, [R0, R4]	; return ADDR[SIZE*row + column]

		b 		if_end

else_block:

		; return 0xff if out of bounds
		mov		R0, #0xFF	; return 0xFF

if_end:

		pop     {PC,R4-R12}             ; restore all register and return

        .endasmfunc

        .data                           ; switch to data (RAM) section
        .align  4                       ; force word alignment

;; Place any variable store in RAM here
        .end
