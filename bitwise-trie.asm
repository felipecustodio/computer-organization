#	SSC-0112 - Organização de Computadores Digitais
#
#	Implementação de uma Bitwise Trie em Assembly MIPS
#
#	Alunos:
#		Felipe Scrochio Custódio - 9442688
#		Gabriel Henrique Scalici - 9292970
#		Juliano Fantozzi - 9791218
#		André Luis Storino Junior - 9293668
#
# Montado e executado utilizando MARS
#
# USO DE REGISTRADORES
# ╔═════════════╦══════════════════════════╗
# ║ Registrador ║        Usado para        ║
# ╠═════════════╬══════════════════════════╣
# ║ $s0-$s4     ║ Opções do Menu           ║
# ║ $t0         ║ Input do Menu            ║
# ║ $s5         ║ Endereço inicial da Trie ║
# ║ $s6         ║ Contador (Trie)          ║
# ║ $s7         ║ Contador (Pilha)         ║
# ║ $a1         ║ Endereço de 'Chave'      ║
# ╚═════════════╩══════════════════════════╝
#
# ESTRUTURA DE DADOS
# ╔══════════════════════╦══════════════╦═════════╗
# ║       Atributo       ║ Tipo de Dado ║ Tamanho ║
# ╠══════════════════════╬══════════════╬═════════╣
# ║ Endereço nó esquerda ║ Ponteiro     ║ 4 bytes ║
# ║ Endereço nó direita  ║ Ponteiro     ║ 4 bytes ║
# ╚══════════════════════╩══════════════╩═════════╝

.data
	.align 2
	# Strings de menu
	str_menu: .asciiz "\n\nBITWISE TRIE\n\n    1. Inserção\n    2. Remoção\n    3. Busca\n    4. Visualização\n    5. Sair\n    Escolha uma opção (1 a 5): "

	# Strings das opções do menu
	str_insert: .asciiz "Digite o binário para inserção: "
	str_remove: .asciiz "Digite o binário para remoção: "
	str_search: .asciiz "Digite o binário para busca: "
	str_repeat: .asciiz "Binario já existente na arvore."
	str_sucess: .asciiz "Sucesso!\n"
	str_duplicated: .asciiz "Chave repetida. Inserção não permitida.\n"
	str_invalid: .asciiz "Chave inválida. Insira somente números binários (ou -1 retorna ao menu)\n"
	str_return: .asciiz "Retornando ao menu.\n"
	str_not_found: .asciiz "Chave não encontrada."
	str_exit: .asciiz "Saindo...\n"

	# Strings da visualização
	str_vis_root: .asciiz "raiz"
	str_vis_n: .asciiz "N"
	str_vis_p1: .asciiz "("
	str_vis_p2: .asciiz ")\n"
	srt_vis_info_t: .asciiz "T"
	str_vis_info_nt: .asciiz "NT"
	str_vis_null: .asciiz "null"

	# Input
	chave: .space 16 # 16 dígitos = 16 bytes
.text

	main:

		# Opções do Menu ficam armazenadas
		# nos registradores $sX
		li $s0, 1 # 1 - Inserção
		li $s1, 2 # 2 - Remoção
		li $s2, 3 # 3 - Busca
		li $s3, 4 # 4 - Visualizar
		li $s4, 5 # 5 - Sair
		# Alocar nó raiz
		li $v0, 9 # alocar memória
		la $a0, 8 # 1 nó = 8 bytes (2 endereços/ponteiros)
		syscall

		# Armazenar endereço inicial da Trie
		move $s5, $v0
		

	# Funcionalidade do Menu
	menu:

		li $v0, 4 # imprimir string
		la $a0, str_menu
		syscall

		li $v0, 5 # ler inteiro
		syscall
		move $t0, $v0 # guardar input em $t0

		# ir para opção escolhida
		beq $t0, $s0, insert_node # 1
		beq $t0, $s1, delete_node # 2
		beq $t0, $s2, search_node # 3
		beq $t0, $s3, print_trie # 4
		beq $t0, $s4, exit # 5
		j menu # loop (opção inválida)

	# Funcionalidades da Trie
	insert_node:
		li $v0, 4 # imprimir string
		la $a0, str_insert
		syscall

		li $v0, 8 # ler string
		la $a0, chave # armazenar 'chave'
		li $a1, 16 # preparar para ler 16 bytes
		syscall
		# checar se chave é valida
		jal check_input
		bne $v0, 1, insert_node
		# checar se chave já existe
		jal search_node
		bne $v0, -1, print_repeat_error
		# se busca retornar -1, continuar
		j menu
		print_repeat_error:
			li $v0, 4
			la, $a0, str_repeat
			syscall
			j insert_node

	delete_node:
		li $v0, 4 # imprimir string
		la $a0, str_remove
		syscall

		li $v0, 8 # ler string
		la $a0, chave
		li $a1, 16
		syscall

		jal check_input # verifica se o input esta correto
		bne $v0, 1, delete_node
		
		jal search_node # verifica se a chave a ser deletada existe de fato
		beq $v0, -1, str_not_found
		#setup para o fatorial
		la $a1, chave #a1 = input
		move $v0, $s5 #v0 = root
		lb $t0, 0($a1) #carrega primeiro digito do input
		jal factorial_deletion_loop #função auxiliar

	search_node:
		#t1 = '0'
		#t2 = '1'
		#t4 = '\n' 
		la $a1, chave #a1 = input
		move $a0, $s5 #a0 = root
		lb $t0, 0($a1) #carrega primeiro digito do input
		
		search_node_loop:
			beq $t0, $t4, node_found #fim da leitura
			beqz $t0, node_found #fim da leitura
			beq $t0, $t1, case_zero #caso byte == '0'
			beq $t0, $t2, case_one #caso byte == '1'
			addi $a1, $a1, 1 #movendo string
			lw $t1, 4($s5) #carrega endereço "1" da arvore
			j search_node_loop
					
		case_zero:
			lw $t0, 0($s5) #carrega endereço "0" da arvore
			bnez $t0, increment_input_vector #caso haja um endereço, continue percorrendo o vetor
			beqz $t0, node_not_found #caso nao haja endereço, retornar "input nao encontrado"
		
		
		case_one:
			lw $t0, 4($s5) #carrega endereço "1" da arvore
			bnez $t0, increment_input_vector #caso haja um endereço, continue percorrendo o vetor
			beqz $t0, node_not_found #caso nao haja endereço, retornar "input nao encontrado
		
		increment_input_vector:
			addi $a1, $a1, 1
			lb $t0, 0($a1)
			j search_node_loop
		
		node_not_found:
			li $v0, -1 # return -1
			jr $ra
		node_found:	
			li $v0, 1 # return 1
			jr $ra
		j menu

	print_trie:
		# li $v0, 4 # imprimir string
		# la $a0, str_vis
		# syscall
		j menu

	 
 ######################## Funções auxiliares ########################
	check_input:
		# Percorrer string de entrada
		li $t1, 48 # 0 em ASCII
		li $t2, 49 # 1 em ASCII
		li $t3, 45 # - em ASCII
		li $t4, 10 # \n em ASCII
		la $a1, chave # carregar endereço de chave em $a1

		check_input_loop:
			# Carregar valor de endereço em a1 e colocar em $t0
			lb $t0, 0($a1)
			# Verificar se bit atual é 0, 1
			beq $t0, $t1, check_input_continue # checa se é 0
			beq $t0, $t2, check_input_continue # checa se é 1
			beq $t0, $t3, check_input_return # checa se é -
			beq $t0, $t4, check_input_pass # verifica se é '\n', se chegou no fim
			beqz $t0, check_input_pass # o \0 tambem determina fim
			# não é 0, 1 ou -1
			j check_input_error

		# é 0 ou 1, continua
		check_input_continue:
			# Ver se está na posição final da entrada
			lb $t0, 1($a1) # carrega proximo byte da string
			addi $a1, $a1, 1 # Andar para o próximo char
			j check_input_loop #reinicia check_input_looooOOoop

		# voltar ao menu (-1)
		check_input_return:
			# Exibir string de retorno
			# Imprimir string
			li $v0, 4 
			la $a0, str_return
			syscall
			j menu

		check_input_error:
			# exibir string de chave inválida
			li $v0, 4 # imprimir string
			la $a0, str_invalid
			syscall

			li $v0, -1 # -1 no retorno = erro
			jr $ra # retornar

		check_input_pass:			
			li $v0, 1 # 1 no retorno = sucesso
			jr $ra
####################################################################################################
	
	factorial_deletion_loop:	
		#t0 = recebe de $a1 o byte da chave de entrada (input do usuario)	
		#t1 = '0' ||  #a0 = contem o endereço da posição atual na arvore
		#t2 = '1' ||  #a1 = input string
		#t4 = '\n'||  #v0 = retorno
		main_loop:
			sw $v0, 0($sp)
			sw $ra, -4($sp)
			addi $sp, $sp, -8
			beq $t0, $t1, case_zero
			beq $t0, $t2, case_one
			beq $t0, $t4, node_found

					
		case_zero:
			lw $v0, 0($a0) #carrega endereço "0" da arvore
			move, $a0, $v0 #ponteiro recebe nó filho
			addi, $a1, $a1, 1
			lb $t0, 0($a1)
			jal main_loop
			#desempilhando
			#fazer a verificação: posso remover nó?
			#se sim, remover nó e continuar(j node_found)
			#se não, não fazer nada e continuar
			j node_found
		
		case_one:
			lw $v0, 4($a0) #carrega endereço "0" da arvore
			move, $a0, $v0 #ponteiro recebe nó filho
			addi, $a1, $a1, 1
			lb $t0, 0($a1)
			jal main_loop
			#desempilhando
			#fazer a verificação: posso remover nó?
			#se sim, remover nó e continuar(j node_found)
			#se não, não fazer nada e continuar
			j node_found
			
		node_found:
			lw $v0, 0($sp)
			lw $ra, 4($sp)
			addi, $sp, $sp, 8
			jr $ra
			
	exit:
		li $v0, 4 # imprimir string
		la $a0, str_exit
		syscall

		li $v0, 10 # finalizar execução
		syscall

# FIM DO PROGRAMA
