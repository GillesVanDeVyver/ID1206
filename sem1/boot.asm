bits 16

start:
		mov ax, 0x07C0	 	; 0x07C0 is were we are
		add ax, 0x20		; add 0x20 (when shifted 512)
		mov ss, ax			; set the stack segment
		mov sp, 0x1000		; set the stsck pointer

		mov ax, 0x07C0		; set data segment
		mov ds, ax			; more about this later

		mov si, msg			; pointer to the message in SI
		mov ah, 0x0E		; print char BIOS procedure

.next:
		lodsb				; next byte to AL, increment SI
		cmp al, 0			; if the byte is zero
		je .done			; jump to done
		int 0x10			; invoke the BIOS system call
		jmp .next			; loop

.done:
		jmp $				; loop forever

msg:	db 'Hello', 0		; the string we want to print

		times 510-($-$$) db 0
		dw 0xAA55