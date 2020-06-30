                section         .text

                global          _start
_start:

                sub             rsp, 2 * 256 * 8
		mov             rdi, rsp
                mov             rcx, 256
                call            read_long
		lea             rdi, [rsp + 256 * 8]
                call            read_long
                mov             rbx, rsp
                call            mul_long_long

                call            write_long

                mov             al, 0x0a
                call            write_char

                jmp             exit

; adds two long number
;    rdi -- address of summand #1 (long number)
;    rsi -- address of summand #2 (long number)
;    rcx -- length of long numbers in qwords
; result:
;    sum is written to rdi
add_long_long:
                push            rdi
                push            rsi
                push            rcx
                push            rax

                clc
.loop:
                mov             rax, [rsi]
                lea             rsi, [rsi + 8]
                adc             [rdi], rax
                lea             rdi, [rdi + 8]
                dec             rcx
                jnz             .loop

                pop             rax
                pop             rcx
                pop             rsi
                pop             rdi
                ret

; adds 64-bit number to long number
;    rdi -- address of summand #1 (long number)
;    rax -- summand #2 (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    sum is written to rdi
add_long_short:
                push            rdi
                push            rcx
                push            rdx

                xor             rdx,rdx
.loop:
                add             [rdi], rax
                adc             rdx, 0
                mov             rax, rdx
                xor             rdx, rdx
                add             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rdx
                pop             rcx
                pop             rdi
                ret

; multiplies two long number by stolbik
;    rdi -- address of multiplier #1 (long number)
;    rbx -- multiplier #2 (long number)
;    rcx -- length of long number in qwords
; result:
;    product is written to rdi
mul_long_long:
                push            rsi
                push            rax
                push            r8
                push            r9
                push            r10
                push            r11
                push            r12
                mov             r8, rdi
                mov             r9, rbx			; копируем все регистры
        
                imul            rax, rcx, 8		; посчитали длину long в rax
                sub             rsp, rax		; выделили себе память на первый long - резерв для последующего копирования
                mov             rbx, rsp
                mov             r10, rsp		; r10 указывает на первый long, копия первого множителя
                call            copy_long		; важны регистры rdi и rbx!!!
                sub             rsp, rax		; выделили себе память на второй long - место для mul_long_short, rax больше почти не нужен
                mov             r11, rsp		; r11 указывает на второй long
                mov             rsi, r11		; заранее устанавливаем rsi для add_long_long
                call            set_zero		; ОБНУЛЯЕМ первый множитель так же легко, как и...
                add             r9, rax			; поставили r9 за последний qword второго множителя (идём с конца)
                mov             r12, rcx		; скопировали rcx, чтобы корректно передавать в функции длину
.loop:
                call            shift_left          	; сместили старый результат на qword
                mov             rdi, r10                ; установили rdi на резерв
                mov             rbx, r11		; копируем первый множитель, для последующего умножения на qword
                call            copy_long               ; важны регистры rdi и rbx!!!
                mov             rdi, r11
                lea             r9, [r9 - 8]            ; сместили r9 на предыдущий qword
                mov             rbx, [r9]
                call            mul_long_short          ; посчитали умножение на следующий qword второго множителя
                mov             rdi, r8                 ; вернули rdi на свое изначальное место (будущий ответ)
                call            add_long_long           ; прибавили к результату
                dec             r12
                jnz             .loop
                
                mov             rbx, r9			; после цикла r9 указывает на начало второго множителя
                add             rax, rax
                add             rsp, rax		; "очистили" стек
                pop             r12			; вернули обратно все использованные регистры
                pop             r11
                pop             r10
                pop             r9
                pop             r8
                pop             rax
                pop             rsi
                ret



; multiplies long number by a short
;    rdi -- address of multiplier #1 (long number)
;    rbx -- multiplier #2 (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    product is written to rdi
mul_long_short:
                push            rax
                push            rdi
                push            rcx
                push            rsi

                xor             rsi, rsi
.loop:
                mov             rax, [rdi]
                mul             rbx
                add             rax, rsi
                adc             rdx, 0
                mov             [rdi], rax
                add             rdi, 8
                mov             rsi, rdx
                dec             rcx
                jnz             .loop

                pop             rsi
                pop             rcx
                pop             rdi
                pop             rax
                ret

; multiplies long number by 2^64
;    rdi -- address of multiplier #1 (long number)
;    rcx -- length of long number in qwords
; result:
;    product is written to rdi
shift_left:
                push            rcx
		push 		rsi
                push            rdi

		dec		rcx
		lea		rdi, [rdi + 8 * rcx]
.loop:
		lea		rsi, [rdi - 8]
		mov		rsi, [rsi]
                mov             [rdi], rsi
		lea		rdi, [rdi - 8]
                dec             rcx
                jnz             .loop

                pop             rdi
                pop             rsi
                pop             rcx
		mov		QWORD [rdi], 0
                ret


; divides long number by a short
;    rdi -- address of dividend (long number)
;    rbx -- divisor (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    quotient is written to rdi
;    rdx -- remainder
div_long_short:
                push            rdi
                push            rax
                push            rcx

                lea             rdi, [rdi + 8 * rcx - 8]
                xor             rdx, rdx

.loop:
                mov             rax, [rdi]
                div             rbx
                mov             [rdi], rax
                sub             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rcx
                pop             rax
                pop             rdi
                ret

;    rdi -- from (long number)
;    rbx -- to (long number)
;    rcx -- length of long number in qwords
copy_long:
                push            rax
                push            rdi
                push            rcx
                push            rbx
.loop:
                mov             rax, [rdi]
                lea             rdi, [rdi + 8]
                mov             [rbx], rax
                lea             rbx, [rbx + 8]
                dec             rcx
                jnz             .loop

                pop             rbx
                pop             rcx
                pop             rdi
                pop             rax
                ret


; assigns a zero to long number
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
set_zero:
                push            rax
                push            rdi
                push            rcx

                xor             rax, rax
                rep stosq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; checks if a long number is a zero
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
; result:
;    ZF=1 if zero
is_zero:
                push            rax
                push            rdi
                push            rcx

                xor             rax, rax
                rep scasq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; read long number from stdin
;    rdi -- location for output (long number)
;    rcx -- length of long number in qwords
read_long:
                push            rcx
                push            rdi

                call            set_zero
.loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              .done
                cmp             rax, '0'
                jb              .invalid_char
                cmp             rax, '9'
                ja              .invalid_char

                sub             rax, '0'
                mov             rbx, 10
                call            mul_long_short
                call            add_long_short
                jmp             .loop

.done:
                pop             rdi
                pop             rcx
                ret

.invalid_char:
                mov             rsi, invalid_char_msg
                mov             rdx, invalid_char_msg_size
                call            print_string
                call            write_char
                mov             al, 0x0a
                call            write_char

.skip_loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              exit
                jmp             .skip_loop

; write long number to stdout
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
write_long:
                push            rax
                push            rcx

                mov             rax, 20
                mul             rcx
                mov             rbp, rsp
                sub             rsp, rax

                mov             rsi, rbp

.loop:
                mov             rbx, 10
                call            div_long_short
                add             rdx, '0'
                dec             rsi
                mov             [rsi], dl
                call            is_zero
                jnz             .loop

                mov             rdx, rbp
                sub             rdx, rsi
                call            print_string

                mov             rsp, rbp
                pop             rcx
                pop             rax
                ret

; read one char from stdin
; result:
;    rax == -1 if error occurs
;    rax \in [0; 255] if OK
read_char:
                push            rcx
                push            rdi

                sub             rsp, 1
                xor             rax, rax
                xor             rdi, rdi
                mov             rsi, rsp
                mov             rdx, 1
                syscall

                cmp             rax, 1
                jne             .error
                xor             rax, rax
                mov             al, [rsp]
                add             rsp, 1

                pop             rdi
                pop             rcx
                ret
.error:
                mov             rax, -1
                add             rsp, 1
                pop             rdi
                pop             rcx
                ret

; write one char to stdout, errors are ignored
;    al -- char
write_char:
                sub             rsp, 1
                mov             [rsp], al

                mov             rax, 1
                mov             rdi, 1
                mov             rsi, rsp
                mov             rdx, 1
                syscall
                add             rsp, 1
                ret

exit:
                mov             rax, 60
                xor             rdi, rdi
                syscall

; print string to stdout
;    rsi -- string
;    rdx -- size
print_string:
                push            rax

                mov             rax, 1
                mov             rdi, 1
                syscall

                pop             rax
                ret


                section         .rodata
invalid_char_msg:
                db              "Invalid character: "
invalid_char_msg_size: equ             $ - invalid_char_msg