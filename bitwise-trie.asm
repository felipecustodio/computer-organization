#    SSC-0112 - Organizacao de Computadores Digitais
#    Turma A - 2018/01
#    Prof Paulo Sergio Lopes de Souza
#
#    Implementacao de uma Bitwise Trie em Assembly MIPS
#
#    Alunos:
#        Felipe Scrochio Custodio - 9442688
#        Gabriel Henrique Scalici - 9292970
#        Juliano Fantozzi - 9791218
#        Andre Luis Storino Junior - 9293668
#
#     Montado e executado utilizando simulador MARS.
#
#   NOMENCLATURA DAS FUNCOES
#       funcionalidade_funcao.
#       Todas as funcoes foram declaradas com o nome principal seguido
#       de sua utilidade, utilizando Snake Case. Assim, fica mais facil
#       pesquisar todas as funcoes de insercao de noh, por exemplo, pois
#       todas comecam com insert_node. Isso tambem nos auxiliou ao utilizar
#       editores com completacao automatica.
#
#   USO DE REGISTRADORES
# +--------------+--------------------------+
# | Registrador  |        Usado para        |
# +--------------+--------------------------+
# | $s0-$s4      | Opcoes do Menu           |
# | $t0          | Input do Menu            |
# | $s5          | Endereco da raiz da Trie |
# | $s6          | Contador (Trie)          |
# | $s7          | Contador (Pilha)         |
# | $a1          | Endereco de 'Chave'      |
# +--------------+--------------------------+
#
#   ESTRUTURA DA BITWISE TRIE
# +----------------------+--------------+---------+
# |       Atributo       | Tipo de Dado | Tamanho |
# +----------------------+--------------+---------+
# | Endereco noh esquerda | Ponteiro    | 4 bytes |
# | Endereco noh direita  | Ponteiro    | 4 bytes |
# | Flag de noh terminal  | Char        | 1 byte  |
# +----------------------+--------------+---------+

.data

    .align 2

    # Menu principal

    # Strings de menu
    str_menu: .asciiz "\n\nBITWISE TRIE\n\n    1. Insercao\n    2. Remocao\n    3. Busca\n    4. Visualizacao\n    5. Sair\n    Escolha uma opcao (1 a 5): "

    # Strings das opcoes do menu
    str_insert: .asciiz "Digite o binario para insercao: "
    str_remove: .asciiz "Digite o binario para remocao: "
    str_search: .asciiz "Digite o binario para busca: "

    str_repeat: .asciiz "Binario ja existente na arvore."
    str_sucess: .asciiz "Sucesso!\n"

    str_found: .asciiz "Chave encontrada na arvore: "
    str_not_found: .asciiz "Chave nao encontrada na arvore: "

    str_duplicated: .asciiz "Chave repetida. Insercao nao permitida.\n"
    str_invalid: .asciiz "Chave invalida. Insira somente numeros binarios (ou -1 retorna ao menu)\n"
    str_return: .asciiz "Retornando ao menu.\n"

    str_exit: .asciiz "Saindo...\n"

    # Strings da visualizacao
    str_vis_n: .asciiz "N"
    str_vis_space: .asciiz " "
    str_vis_p1: .asciiz "("
    str_vis_p2: .asciiz ")"

    srt_vis_info_t: .asciiz "T"
    str_vis_info_nt: .asciiz "NT"
    str_vis_null: .asciiz ", null"

    str_vis_root: .asciiz "raiz"
    str_vis_zero: .asciiz "0"
    str_viz_one: .asciiz "1"
    str_vis_t: .asciiz ", T"

    str_vis_nt: .asciiz ", NT"

    str_vis_comma: .asciiz ", "
    str_vis_nl: .asciiz "\n"
    # Input
    chave: .space 16 # 16 digitos = 16 bytes

.text

    main:
        # Opcoes do Menu ficam armazenadas
        # nos registradores $sX
        li $s0, 1 # 1 - Insercao
        li $s1, 2 # 2 - Remocao
        li $s2, 3 # 3 - Busca
        li $s3, 4 # 4 - Visualizar
        li $s4, 5 # 5 - Sair

        # Alocar noh raiz
        li $v0, 9 # alocar memoria
        la $a0, 9 # 1 noh = 9 bytes (2 enderecos/ponteiros + 1 flag indicando se o noh eh terminal)
        syscall

        # Colocando o valor null nos ponteiros da raiz
        # endereco a esquerda = null
        sw $zero, 0($v0)
        # endereco a direita = null
        sw $zero, 4($v0)
        # flag indicando se eh noh terminal = false
        sb $zero, 8($v0)

        # Armazenar endereco inicial da Trie (raiz)
        move $s5, $v0

        # Armazenar endereco inicial da stack
        add $s7, $sp, $zero

    # Funcionalidade do Menu
    menu:

        li $v0, 4 # imprimir string
        la $a0, str_menu
        syscall

        li $v0, 5 # ler inteiro
        syscall
        move $t0, $v0 # guardar input em $t0

        # ir para opcao escolhida
        beq $t0, $s0, insert_node # 1
        beq $t0, $s1, remove_node # 2
        beq $t0, $s2, search_node_new # 3
        beq $t0, $s3, visualize # 4
        beq $t0, $s4, exit # 5
        j menu # loop (opcao invalida)

    # Funcionalidades da Trie

    # +----------+
    # | INSERCAO |
    # +----------+
    insert_node_repeat:
    	li $v0, 4 # imprimir string
        la $a0, str_duplicated
        syscall

    insert_node:
        li $v0, 4 # imprimir string
        la $a0, str_insert
        syscall

        li $v0, 8 # ler string
        la $a0, chave # armazenar input do usuario em 'chave'
        li $a1, 16 # preparar para ler 16 bytes
        syscall

        jal check_input # verificar se input eh valido (volta ao menu se -1)
        beq $v0, -1, insert_node # pede nova chave caso seja invalida

        # verificar se chave ja existe
        jal search_node
        bne $v0, -1, insert_node_repeat # pede nova chave caso seja repetida

        # acessar 'chave' do usuario
        # $a1 eh nosso ponteiro para iterar sobre a chave
        la $a1, chave

        # acessar noh raiz
        # $t1 = sempre noh atual
        # la $t1, root
        move $t1, $s5

        li $t3, 48 # 0 em ASCII
        li $t4, 49 # 1 em ASCII

        insert_node_loop:
            # percorrer chave do usuario
            # $t0 = caractere atual da chave
            # $a1 = endereco do caractere atual da chave
            lb $t0, 0($a1) # $ a1 sempre estara atualizado
            beq $t0, $t3, insert_node_left # 0 = inserir a esquerda
            beq $t0, $t4, insert_node_right # 1 = inserir a direita

            # fim da string significa que noh atual eh noh terminal de chave
            # marcar flag como '1'
            sb $s0, 8($t1)

            j insert_node # encerrou, pedir nova string ou retorno ao menu

        insert_node_right:
            # verificar se existe filho ? direita
            # $t2 = ponteiro temporario para filhos
            lw $t2, 4($t1)
            bnez $t2, insert_descend_right

            # se &dir == null, criar e inserir novo noh
            insert_node_right_new:
                # vamos alocar e inserir
                li $v0, 9 # alocar memoria
                li $a0, 12 # 1 noh = 12 bytes (2 enderecos/ponteiros + 1 flag)
                syscall # $v0 contem endereco inicial do novo noh

                # coloca o valor null nos ponteiros do novo noh
                # endereco a esquerda = null
                sw $zero, 0($v0)
                # endereco a esquerda = null
                sw $zero, 4($v0)
                # flag indicando se eh noh terminal = false
                sb $zero, 8($v0)

                sw $v0, 4($t1) # novo noh eh armazenado como filho direito do noh atual
                # descer para novo noh e continuar loop

            # se &dir != null, descer para ele e voltar ao loop
            insert_descend_right:
                # descendo na arvore, t1 = &dir do noh em que estavamos
                lw $t1, 4($t1)
                addi $a1, $a1, 1 # ir para proximo caractere na chave
                j insert_node_loop

        insert_node_left:
            # verificar se existe filho a esquerda
            # $t2 = ponteiro temporario para filhos
            lw $t2, 0($t1)
            bnez $t2, insert_descend_left

            # se &dir == null, criar e inserir novo noh
            insert_node_left_new:
                # vamos alocar e inserir
                li $v0, 9 # alocar memoria
                li $a0, 12 # 1 noh = 8 bytes (2 enderecos/ponteiros + 1 flag)
                syscall # $v0 contem endereco inicial do novo noh

                # coloca o valor null nos ponteiros do novo noh
                # endereco a esquerda = null
                sw $zero, 0($v0)
                # endereco a esquerda = null
                sw $zero, 4($v0)
                # flag indicando se eh noh terminal = false
                sb $zero, 8($v0)

                sw $v0, 0($t1) # novo noh eh armazenado como filho esquerdo do noh atual
                # descer para novo noh e continuar loop

            # se &esq != null, descer para ele e voltar ao loop
            insert_descend_left:
                # descendo na arvore, t1 = &esq do noh em que estavamos
                lw $t1, 0($t1)
                addi $a1, $a1, 1 # ir para proximo caractere na chave
                j insert_node_loop


    # +-------+
    # | BUSCA |
    # +-------+
    search_node_new:
        li $v0, 4 # imprimir string
        la $a0, str_search
        syscall

        li $v0, 8 # ler string
        la $a0, chave # armazenar input do usuario em 'chave'
        li $a1, 16 # preparar para ler 16 bytes
        syscall

        jal check_input # verificar se input eh valido (volta ao menu se -1)
        beq $v0, -1, search_node_new # pede nova chave caso seja invalida

        jal search_node
        beq $v0, 1, search_node_new_sucess # se retorno 1 = sucesso na busca

        # se retorno != 1, falha na busca
        search_node_new_failure:
            li $v0, 4 # imprimir string
            la $a0, str_not_found # imprimir que chave nao foi encontrada
            syscall

            li $v0, 4 # imprimir string
            la $a0, chave # imprimir a chave
            syscall
            j search_node_new

        search_node_new_sucess:
            li $v0, 4 # imprimir string
            la $a0, str_found # imprimir que chave foi encontrada
            syscall

            li $v0, 4 # imprimir string
            la $a0, chave # imprimir a chave
            syscall
            j search_node_new

    search_node:
        # carregar valores de comparacao
        li $t1, 48 # 0 em ASCII
        li $t2, 49 # 1 em ASCII
        li $t4, 10 # \n em ASCII

        la $a1, chave # a1 = input
        move $a0, $s5 # a0 = root
        lb $t0, 0($a1) # $t0 = carrega digito da chave

        search_node_loop:
            beq $t0, $t4, search_last_node_found # fim da leitura '\n'
            beq $t0, $zero, search_last_node_found # fim da leitura '\0'
            beq $t0, $t1, search_node_zero # caso byte == '0' goto search_node_zero
            beq $t0, $t2, search_node_one # caso byte == '1' goto search_node_one
            j search_node_loop

        search_node_zero:
            lw $t0, 0($a0) # carrega endereco "0" da arvore
            bnez $t0, search_next_node_case_zero # caso haja um endereco, continue percorrendo o vetor
            beqz $t0, search_return_failure # caso nao haja endereco, retornar "input nao encontrado"

        search_node_one:
            lw $t0, 4($a0) # carrega endereco "1" da arvore
            bnez $t0, search_next_node_case_one # caso haja um endereco, continue percorrendo o vetor
            beqz $t0, search_return_failure # caso nao haja endereco, retornar "input nao encontrado

        search_next_node_case_zero:
            addi $a1, $a1, 1
            lb $t0, 0($a1)
            lw $a0, 0($a0)
            j search_node_loop

        search_next_node_case_one:
            addi $a1, $a1, 1
            lb $t0, 0($a1)
            lw $a0, 4($a0)
            j search_node_loop

        search_last_node_found:
            lb $t0, 8($a0) #load da flag
            beq $t0, $zero, search_return_failure #caso n?o exista a flag, nao eh um noh terminal. Se for um noh terminal, a chave foi encontrada
            beq $t0, 1, search_return_sucess #caso exista a flag, ? um noh terminal. Se for um noh terminal, a chave foi encontrada

        search_return_sucess:
            li $v0, 1 # return 1
            jr $ra

        search_return_failure:
            li $v0, -1 # return -1
            jr $ra

    # +---------+
    # | REMOCAO |
    # +---------+
    remove_node:
        li $v0, 4 # imprimir string
        la $a0, str_remove
        syscall

        li $v0, 8 # ler string
        la $a0, chave
        li $a1, 16
        syscall

        jal check_input # verifica se o input esta correto
        bne $v0, 1, remove_node # pede nova chave caso esteja incorreto

        jal search_node # verifica se a chave a ser deletada existe de fato
        beq $v0, -1, remove_node # pede nova chave caso chave nao exista

        # setup para a recursao
        la $a1, chave # a1 = input
        move $v0, $s5 # v0 = root
        lb $t0, 0($a1) # carrega primeiro digito do input
        jal remove_node_recursion # chama recursao
        j menu # volta ao menu

        remove_node_recursion:
            #t0 = recebe de $a1 o byte da chave de entrada (input do usuario)
            #t1 = '0'
            #t2 = '1'
            #t4 = '\n'
            #a0 = auxiliar para descer na recursao
            #a1 = input string
            #v0 = endereco do noh sendo processado
            remove_node_recursion_loop:
                #push da recursao
                sw $v0, 0($sp)
                sw $ra, -4($sp)
                addi $sp, $sp, -8
                #jump para os casos
                beq $t0, $t1, remove_node_zero
                beq $t0, $t2, remove_node_one
                beq $t0, $t4, remove_node_last # caso base, quando $t0 = \n
                beqz $t0, remove_node_last # caso base, quando $t0 = \0

            remove_node_zero:
                move $a0, $v0 # $a0 salva o endereco do noh atual
                lw $v0, 0($a0) # carrega endereco "0" da arvore (noh a esquerda)
                addi, $a1, $a1, 1  # incrementando o input do usuario
                lb $t0, 0($a1) # carregando o proximo elemento da string
                jal remove_node_recursion_loop
                j remove_return_recursion

            remove_node_one:
                move $a0, $v0 # $a0 salva o endereco do noh atual
                lw $v0, 4($a0) # carrega endereco "1" da arvore (noh a direita)
                addi, $a1, $a1, 1 # incrementando o input do usuario
                lb $t0, 0($a1) # carregando o proximo elemento da string
                jal remove_node_recursion_loop

            remove_return_recursion:
                lw $t6, 0($v0) # $t6 = filho a esquerda do noh processado
            lw $t7, 4($v0) # $t7 = filho a direita do noh processado
            lb $t5, 8($v0) # load da flag para verificar se ? noh terminal ou n?o

            # caso ele tenha dois filhos ou seja um noh terminal, a recursao acaba
            bnez $t5, end_recursion    # eh noh terminal
            add $t5, $t6, $t7    # $t5 = fesq + fdir
            #se $t5 for igual a $t6 ou $t7 ele soh tem um filho, se nao ele tem 2 filhos
            beq $t5, $t6, remove_node_next
            beq $t5, $t7, remove_node_next

            end_recursion:
            addi $sp, $sp, -8
            j remove_assign_null

            remove_node_next:
                lw $ra, 4($sp) #pop da pilha
                lw $v0, 8($sp)
                beq $s5, $v0, remove_assign_null # se for a raiz a recursao acaba
                addi, $sp, $sp, 8
                jr $ra

            remove_node_last:
            sb $zero, 8($v0) #setando a flag de terminal para 0
            lw $t6, 0($v0) # $t6 recebe o endereco do filho a esquerda
            lw $t7, 4($v0) # $t7 recebe o endereco do filho a direita
            sub $t6, $t6, $t7 # checando se o ultimo noh tem algum filho
            bnez $t6, remove_end # o ultimo noh tem filhos
            addi $sp, $sp, 8    # ignorando a etapa de recursao do ultimo noh
            lw $v0, 8($sp)    # checando se eh a raiz (chave de 1 digito)
            beq $v0, $s5, remove_assign_null
            j remove_node_next # retornando na recursao

            # caso um dos filhos do noh tenha de ser atribuido null
            remove_assign_null:
            lw $t6, 0($v0) # filho a esquerda
            lw $t7, 4($v0) # filho a direita
            lw $t3, 0($sp) # de onde veio na recursao
            beq $t3, $t6, remove_assign_null_left
            beq $t3, $t7, remove_assign_null_right

            remove_assign_null_left:
            sw $zero, 0($v0)
            j remove_end

            remove_assign_null_right:
            sw $zero, 4($v0)

            remove_end:    # a remocao termina
            move $sp, $s7 # voltando stack pointer na posicao inicial
            lw $ra, -4($sp) # carregando endereco para sair da funcao de remover
            jr $ra

    # +--------------+
    # | VISUALIZACAO |
    # +--------------+
    # +-------------+------------------------------------------------------------------+
    # | Registrador |                            Utilidade                             |
    # +-------------+------------------------------------------------------------------+
    # | $t0         | endereco do no checado                                           |
    # | $t1         | valores obtidos da memoria a partir de $t0                       |
    # | $t2         | contador do nivel da arvore                                      |
    # | $t3         | armazena o endereco da estrutura alocada que sera obtida da fila |
    # | $t4         | valores obtidos da memoria a partir de $t3                       |
    # | $t5         | primeiro elemento da fila utilizada                              |
    # | $t6,$t7     | utilizados nas funcoes da fila                                   |
    # | $a3         | endereco do noh sentinela para contar o nivel da arvore          |
    # +-------------+------------------------------------------------------------------+

    # Imprime a arvore em largura, com informacoes relevantes de cada noh
    visualize:

        # armazenando endereco da raiz
        move $t0, $s5

        # nivel inicial = 0
        li $t2, 0

        # inicializando fila
        li $t5, 0

        # adicionando elemento sentinela na fila para contar os niveis da arvore
        add $a0, $t5, $zero
        li $a1, 0 # endereco null sera colocado nesta sentinela
        li $a2, 0 # ignorar digito do sentinela
        jal enqueue
		move $t5, $v0 # armazena o primeiro elemento da fila
        move $a3, $v0 # $a3 recebe o endereco do sentinela

        # imprimindo informacoes da raiz
        # "N0 (raiz, NT, &esq, &dir) \n"
        li $v0, 4 # imprimir string "N"
        la $a0, str_vis_n
        syscall

        li $v0, 1 #imprimir 0 (nivel da raiz)
        add $a0, $t2, $zero
        syscall

        li $v0, 4 # imprimir string " "
        la $a0, str_vis_space
        syscall

        li $v0, 4 # imprimir string "("
        la $a0, str_vis_p1
        syscall

        li $v0, 4 # imprimir string "raiz"
        la $a0, str_vis_root
        syscall

        li $v0, 4 # imprimir string ", NT"
        la $a0, str_vis_nt
        syscall

        # Checa se o endereco do filho a esquerda != null
        vis_check_left:
            lw $t1, 0($t0)
            bnez $t1, vis_print_left
            jal vis_print_null

        # Checa se o endereco do filho a direita != null
        vis_check_right:
            lw $t1, 4($t0)
            bnez $t1, vis_print_right
            jal vis_print_null
            j vis_next_node

        # imprime o endereco do filho a esquerda
        # adiciona o filho a esquerda na fila
        vis_print_left:
            li $v0, 4 # imprimir string ", "
            la $a0, str_vis_comma
            syscall

            li $v0, 1 # imprimir o endereco do filho a esquerda
            add $a0, $t1, $zero
            syscall

            # adicionar filho a esquerda na fila
            add $a0, $t5, $zero
            add $a1, $t1, $zero
            li $a2, 0
            jal enqueue
            add $t5, $v0, $zero #novo endereco da cabeca da fila

            j vis_check_right

        # imprime o endereco do filho a direita
        # adiciona o filho a direita na fila
        vis_print_right:
            li $v0, 4 # imprimir string ", "
            la $a0, str_vis_comma
            syscall

            li $v0, 1 # imprimir o endereco do filho a direita
            add $a0, $t1, $zero
            syscall

            # adicionar filho a direita na fila
            add $a0, $t5, $zero
            add $a1, $t1, $zero
            li $a2, 1
            jal enqueue
            add $t5, $v0, $zero #novo endereco da cabeca da fila

            j vis_next_node

        vis_print_null:
            li $v0, 4 # imprimir string ", null"
            la $a0, str_vis_null
            syscall
            jr $ra

        # checa se a fila esta vazia, caso contrario imprime o proximo noh
        vis_next_node:
            li $v0, 4 # imprimir string ") "
            la $a0, str_vis_p2
            syscall

            vis_next_dequeue:
                # pega o proximo endereco da fila
                add $a0, $t5, $zero # $a0 recebe a cabeca da fila
                jal dequeue
                add, $t5, $v1, $zero # nova cabeca da fila
                add, $t3, $v0, $zero # elemento removido da fila

                # checando se o elemento removido eh o elemento sentinela
                beq $t3, $a3, vis_next_level
                # $t0 recebe endereco do noh checado (que estava armazenado na estrutura da fila)
                lw $t0, 0($t3)
                # $t4 recebe o digito do noh checado
                lw $t4, 4($t3)
				j vis_print_node_info
				
                vis_next_level:	
					
                    li $v0, 4 # imprimir string "\n"
                    la $a0, str_vis_nl
                    syscall

                    # checando se a fila esta vazia
                    move $a0, $t5
                    jal queue_empty
                    beqz $v0, menu # TERMINA A VISUALIZACAO DA ARVORE
					
					# remove o elemento da fila para ser o proximo a ser checado
					jal dequeue
					add, $t5, $v1, $zero # nova cabeca da fila
					add, $t3, $v0, $zero # elemento removido da fila
					# $t0 recebe endereco do noh checado (que estava armazenado na estrutura da fila)
					lw $t0, 0($t3)
					# $t4 recebe o digito do noh checado
					lw $t4, 4($t3)

                    # adiciona outro sentinela caso nao tenha acabado a visualizacao
                    add $a0, $t5, $zero
                    li $a1, 0 # endereco null sera colocado nesta sentinela
                    li $a2, 0
                    jal enqueue
					add $t5, $v0, $zero
                    add $a3, $v1, $zero # $a3 recebe o novo sentinela

                    # incrementa o contador de nivel e imprime informacao do novo nivel
                    addi $t2, $t2, 1
                    li $v0, 4 # imprimir string "N"
                    la $a0, str_vis_n
                    syscall

                    li $v0, 1 #imprimir nivel da arvore
                    add $a0, $t2, $zero
                    syscall

                    li $v0, 4 # imprimir string " "
                    la $a0, str_vis_space
                    syscall

        vis_print_node_info:
            # imprimindo informacoes do noh
            # "(0 ou 1, NT, &esq, &dir) "

            li $v0, 4 # imprimir string "("
            la $a0, str_vis_p1
            syscall

            # imprimir string "0" ou "1"
            bnez $t4, vis_print_one

            vis_print_zero:
                li $v0, 1
                li $a0, 0
                syscall
                j vis_print_continue

            vis_print_one:
                li $v0, 1
                li $a0, 1
                syscall

        vis_print_continue:
            # imprimir string ", NT" ou ", T"
            lw $t1, 8($t0)
            bnez $t1, vis_print_T

            vis_print_NT:
                li $v0, 4
                la $a0, str_vis_nt
                syscall
                j vis_check_left
            vis_print_T:
                li $v0, 4
                la $a0, str_vis_t
                syscall
                j vis_check_left


    # Funcoes auxiliares
    # +--------------+
    # |    FILA      |
    # +--------------+
    #
    # ESTRUTURA DE DADOS
    # +---------------------+--------------+---------+
    # |      Atributo       | Tipo de Dado | Tamanho |
    # +---------------------+--------------+---------+
    # | Noh da Arvore       | Ponteiro    | 4 bytes |
    # | Digito obtido       | Inteiro      | 4 bytes |
    # | Elemento antecessor | Ponteiro     | 4 bytes |
    # +---------------------+--------------+---------+
    #
    # $a0 tera o endereco do primeiro elemento da fila
    # a fila estara vazia quando $a0 = null

    # $a0 tem o primeiro elemento da fila
    # $v0 ira retornar o valor 0 caso a fila esteja vazia
    # $v0 ira retornar o endereco da primeira estrutura alocada na fila caso contrario
    queue_empty:
        move $v0, $a0
        jr $ra


    # $a0 tem o primeiro elemento da fila
    # $a1 tem o endereco do noh a ser armazenado
    # $a2 tem o valor do digito daquele noh
    # $v0 ira retornar o endereco da cabeca da fila
	# $v1 ira retornar o endereco do elemento inserido na fila
    enqueue:
        # aloca memoria para um novo noh
        add $t6, $a0, $zero # salvando primeiro elemento da fila em $t6

        li $v0, 9 # alocando memoria para um novo elemento na fila
        li $a0, 12
        syscall
		move $v1, $v0 # salvando endereco da nova estrutura
        # adicionando informacoes na nova estrutura
        sw $a1, 0($v0)
        sw $a2, 4($v0)
        sw $zero, 8($v0)

        # percorrendo a fila ate o ultimo noh
        # adicionando a referencia para o novo noh alocado
        beqz $t6, enqueue_empty    # fila vazia
        add $a0, $t6, $zero # $a0 recebe o endereco da cabeca da fila

        enqueue_loop:
            lw $t7, 8($a0) # endereco do antecessor deste elemento na fila
            beqz $t7, enqueue_end_loop # caso nao haja antecessor, insira elemento
            move $a0, $t7 # vai para o proximo elemento da fila
            j enqueue_loop
        enqueue_end_loop:
            sw $v0, 8($a0)    # novo elemento inserido na fila
            add $v0, $t6, $zero # $v0 recebe a referencia para a cabeca da fila
        enqueue_empty:
            jr $ra # termina a insercao

    # $a0 tem o primeiro elemento da fila
    # $v0 ira retornar o endereco do elemento removido da fila, null caso a fila esteja vazia
    # $v1 ira retornar o endereco da nova cabeca da fila, null caso a fila esteja vazia
    # lembrar de desalocar 12 bytes da estrutura retornada em $v0, caso desejar
    dequeue:
        beqz $a0, dequeue_empty # fila vazia
        add $v0, $a0, $zero # $v0 recebe o elemento na cabeca da fila
        lw $v1 8($a0) # $v1 tem a referencia do proximo elemento, a nova cabeca da fila
        jr $ra
        dequeue_empty:
            li $v0, 0
            li $v1, 0
            jr $ra

    # +--------------+
    # | CHECAR INPUT |
    # +--------------+
    check_input:
        # Percorrer string de entrada
        li $t1, 48 # 0 em ASCII
        li $t2, 49 # 1 em ASCII
        li $t3, 45 # - em ASCII
        li $t4, 10 # \n em ASCII
        la $a1, chave # carregar endereco de chave em $a1

        check_input_loop:
            # Carregar valor de endereco em a1 e colocar em $t0
            lb $t0, 0($a1)
            # Verificar se bit atual eh 0, 1
            beq $t0, $t1, check_input_continue # checa se eh 0
            beq $t0, $t2, check_input_continue # checa se eh 1
            beq $t0, $t3, check_input_return1 # checa se eh -
            beq $t0, $t4, check_input_pass # verifica se eh '\n', se chegou no fim
            beq $t0, $zero, check_input_pass # verifica se eh '\0'
            # nao eh 0, 1, - ou final de string
            j check_input_error

        # eh 0 ou 1, continua
        check_input_continue:
            # Ver se esta na posicao final da entrada
            lb $t0, 1($a1) # carrega proximo byte da string
            addi $a1, $a1, 1 # Andar para o proximo char
            j check_input_loop # reinicia check_input_loop

        # voltar ao menu (-1)
        # checando se o byte seguido do '-' equivale ao digito '1'
        check_input_return1:
            lb $t0 1($a1)
            beq $t0, $t2, check_input_return2
            j check_input_error

        # checando se a string acabou apos o "-1"
        check_input_return2:
            lb $t0 2($a1)
            # checando se eh '\n'
            beq $t0, $t4, check_input_return3
            # checando se eh '\0'
            beq $t0, $zero, check_input_return3
            j check_input_error

        check_input_return3:
            # Exibir string de retorno
            # Imprimir string
            li $v0, 4
            la $a0, str_return
            syscall
            j menu

        check_input_error:
            # exibir string de chave invalida
            li $v0, 4 # imprimir string
            la $a0, str_invalid
            syscall

            li $v0, -1 # -1 no retorno = erro
            jr $ra # retornar

        check_input_pass:
            # retornar com sucesso
            li $v0, 1 # 1 no retorno = sucesso
            jr $ra

    # +------+
    # | SAIR |
    # +------+
    exit:
        li $v0, 4 # imprimir string
        la $a0, str_exit
        syscall

        li $v0, 10 # finalizar execucao
        syscall
