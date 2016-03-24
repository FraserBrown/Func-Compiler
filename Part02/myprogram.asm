	.data
d:	 .space 40
	 .text 
main: 
	li $t8,0 
	li $t9,4 
	mul $t8, $t8, $t9 
	li $t9,2 
	sw $t9, d($t8)
	li $t8,0 
	li $t9,4 
	mul $t8, $t8, $t9 
	lw $t9, d($t8)
	addi $sp,$sp,-4
	sw $t9,0($sp) 
	li $t9,3
	lw $t8,0($sp)
	addi $sp,$sp,4 
	add $t8, $t8, $t9
	addi $sp,$sp,-4
	sw $t8,0($sp) 
	li $t8,1 
	li $t9,4 
	mul $t8, $t8, $t9 
	lw $s0,0($sp)
	addi $sp,$sp,4 
	li $t8,1 
	li $t9,4 
	mul $t8, $t8, $t9 
	lw $t9, d($t8)
	li $v0, 1
	move $a0, $t9
	syscall
