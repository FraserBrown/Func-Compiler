	.data
a:	 .space 8
	 .text 
main: 
	li $t8,0 
	li $t9,4 
	mul $t8, $t8, $t9 
	li $t9,2 
	sw $t9, a($t8)
	li $t8,1 
	li $t9,4 
	mul $t8, $t8, $t9 
	li $t9,5 
	sw $t9, a($t8)
	li $t8,0 
	li $t9,4 
	mul $t8, $t8, $t9 
	lw $t9, a($t8)
	li $v0, 1
	move $a0, $t9
	syscall
	li $t8,1 
	li $t9,4 
	mul $t8, $t8, $t9 
	lw $t9, a($t8)
	li $v0, 1
	move $a0, $t9
	syscall
