# MIPS executa linha por linha
# Começa em .text, main é considerado padrão para começar

.text # programa vai embaixo de .text, o que vai rodar

# label
main:
# registradores armazenam informação
# no MARS, podemos ver os registradores na direita ($)
# Carregar valores em um registrador:
li $t0, 5 # load immediate, registrador <- valor
li $v0, 10 # valores armazenados em hex (10 = A)

# Operandos
add $t0, $t1, $t2 # t0 <- t1 + t2
sub $t0, $t1, $t2 # t0 <- t1 - t2
mul $t0, $t1, $t2 # t0 <- t1 * t2
# div $t0, $t1, $t2 # t0 <- t1 / t2

# Encerrar execução do programa
li $v0, 10
syscall

.data # dados vão embaixo de .data, variáveis, labels, words
#  0x10010000 - Primeiro endereço
cake: .ascii "the cake is a lie" # string


.data
str: .asciiz "the answer = "
.text
    li $v0, 4       # $system call code for print_str
    la $a0, str     # $address of string to print
    syscall         # print the string

    li $v0, 1       # $system call code for print_int
    li $a0, 5       # $integer to print
    syscall         # print it
