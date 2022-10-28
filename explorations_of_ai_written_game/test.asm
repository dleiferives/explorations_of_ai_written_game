;NASM code to compute addition
;64 bit

SECTION .text
global _add

_add: ; function to compute the sum of (a) and (b)
move rax, [rdi] ; load a
add rax, [rsi] ; add b
ret




