jal main
#                                           CS 240, Lab #3
# 
#                                          IMPORTANT NOTES:
# 
#                       Write your assembly code only in the marked blocks.
# 
#                     	DO NOT change anything outside the marked blocks.
# 
#               Remember to fill in your name, student ID in the designated sections.
# 
#
j main
###############################################################
#                           Data Section
.data
# 
# Fill in your name, student ID in the designated sections.
# 
student_name: .asciiz "Bryce Jarboe"
student_id: .asciiz "825033151"

new_line: .asciiz "\n"
space: .asciiz " "
testing_label: .asciiz "Testing "
unsigned_addition_label: .asciiz "Unsigned Addition (Hexadecimal Values)\nExpected Output:\n0154B8FB06E97360 BAC4BABA1BBBFDB9 00AA8FAD921FE305 \nObtained Output:\n"
fibonacci_label: .asciiz "Fibonacci\nExpected Output:\n0 1 5 55 6765 3524578 \nObtained Output:\n"
file_label: .asciiz "File Read\nObtained Output:\n"

addition_test_data_A:	.word 0xeee94560, 0x0154a8d0, 0x09876543, 0x000ABABA, 0xFEABBAEF, 0x00a9b8c7
addition_test_data_B:	.word 0x18002e00, 0x0000102a, 0x12349876, 0xBABA0000, 0x93742816, 0x0000d6e5

fibonacci_test_data:	.word 0, 1, 5, 10, 20, 33

hex_digits: .byte '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'

file_name:
	.asciiz	"lab3_data.dat"	# File name
	.word	0
read_buffer:
	.space	300			# Place to store character
###############################################################
#                           Text Section
.text
# Utility function to print hexadecimal numbers
print_hex:
move $t0, $a0
li $t1, 8 # digits
lui $t2, 0xf000 # mask
mask_and_print:
# print last hex digit
and $t4, $t0, $t2 
srl $t4, $t4, 28
la    $t3, hex_digits  
add   $t3, $t3, $t4 
lb    $a0, 0($t3)            
li    $v0, 11                
syscall 
# shift 4 times
sll $t0, $t0, 4
addi $t1, $t1, -1
bgtz $t1, mask_and_print
exit:
jr $ra
###############################################################
###############################################################
###############################################################
#                           PART 1 (Unsigned Addition)
# You are given two 64-bit numbers A,B located in 4 registers
# $t0 and $t1 for lower and upper 32-bits of A and $t2 and $t3
# for lower and upper 32-bits of B, You need to store the result
# of the unsigned addition in $t4 and $t5 for lower and upper 32-bits.
#
.globl Unsigned_Add_64bit
Unsigned_Add_64bit:
move $t0, $a0
move $t1, $a1
move $t2, $a2
move $t3, $a3
############################## Part 1: your code begins here ###

# t6 = 0
add $t6, $zero, $zero
 
# t4 = t0 + t2
add $t4, $t0, $t2

# if (t4 < t0) || (t4 < t2)
#	t6 = 1
bgeu $t4, $t0, DONE1
bgeu $t4, $t2, DONE1
addi $t6, $zero, 1
DONE1:

# t5 = t1 + t3
add $t5, $t1, $t3

# t5 = t5 + t6
add $t5, $t5, $t6



############################## Part 1: your code ends here   ###
move $v0, $t4
move $v1, $t5
jr $ra
###############################################################
###############################################################
###############################################################

###############################################################
###############################################################
###############################################################
#                            PART 2 (Fibonacci)
#
# 	The Fibonacci Sequence is the series of numbers:
#
#		0, 1, 1, 2, 3, 5, 8, 13, 21, 34, ...

#	The next number is found by adding up the two numbers before it.
	
#	The 2 is found by adding the two numbers before it (1+1)
#	The 3 is found by adding the two numbers before it (1+2),
#	And the 5 is (2+3),
#	and so on!
#
# You should compute and return the nth digit of the Fibonacci sequence.
# The digit you need to compute will be in $a0.
# Return your digit in $a1.
# 
.globl fibonacci
fibonacci:
move $a0, $s0
############################## Part 2: your code begins here ###
# Base cases:

# if a0 == 0,  t2 = 0
addi $t2, $zero, 0
beq $t2, $a0, DONE2

# if a0 == 1,  t2 = 1
addi $t2, $zero, 1
beq $t2, $a0, DONE2


# t0 = 2 # first two cases tested separately
addi $t0, $zero, 2

# t1 = 0
addi $t1, $zero, 0

# t2 = 1
addi $t2, $zero, 1

# while itr < a0
#	temp = t2
#	t2 = t1 + t2
#	t1 = temp
#	++itr
WHILE2: bgt $t0, $a0, DONE2	
add $t3, $zero, $t2
add $t2, $t1, $t2
add $t1, $zero, $t3
addi $t0, $t0, 1
j WHILE2

DONE2:
# a1 = t2
add $a1, $zero, $t2





############################## Part 2: your code ends here   ###
move $v0, $a1
jr $ra
###############################################################
###############################################################
###############################################################

###############################################################
###############################################################
###############################################################
#                           PART 3 (ReadFile)
#
# You will read characters (bytes) from a file (lab3_data.dat) and print them. Valid characters are defined to be
# alphanumeric characters (a-z, A-Z, 0-9),
# " " (space),
# "." (period),
# (new line).
#
# We have loaded the file for you and placed its contents in a buffer (300 bytes length)
# $a1 contains the address of the input buffer
# You need to print all valid characters in the buffer, while ignoring all invalid ones.
# Do not print anything else, including extra new lines.
#
# Hint: Remember the ascii table. You will NOT need to create a reference array of valid chars
#
.globl file_read
file_read:


############################### Part 3: your code begins here ##
# Open file_name
li $v0, 13
la $a0, file_name
li $a1, 0
li $a2, 0
syscall
# Store File Descriptor into t0
la $t0, ($v0)

# Read file_name
li $v0, 14
move $a0, $t0
la $a1, read_buffer
li $a2, 300
syscall
# Store charSize into t1
la $t1, ($v0)

# Close file_name
li $v0, 16
la $a0, ($t0)
syscall

# Print chars

# valid ascii's: 10, 32, 46, 48-57, 65-90, 97-122

# while itr <= charSize
#	if char is valid ascii
#		print char
#	++itr
###

# t9, itr
li $t9, 0

# t0 = File Descriptor	# t1 = charSize	# t2 = charByte
# t4 will be used for comparison
WHILE3:
bgt $t9, $t1, DONE3
lbu $t2, 0($a1)

# VALID if...
#	if t2 == 10
#	if t2 == 32
#	if t2 == 46
#	if 47 < t2 < 58
#	if 64 < t2 < 91
#	if 96 < t2 < 123
####

# if t2 == 10
li $t4, 10
beq $t2, $t4, VALID

# if t2 == 32
li $t4, 32
beq $t2, $t4, VALID

# if t2 == 46
li $t4, 46
beq $t2, $t4, VALID

# if 47 < t2 < 58
li $t4, 47
blt $t4, $t2, CONDITION47
j INVALID

CONDITION47:
li $t4, 58
bgt $t4, $t2, VALID


# if 64 < t2 < 91
li $t4, 64
blt $t4, $t2, CONDITION64
j INVALID

CONDITION64:
li $t4, 91
bgt $t4, $t2, VALID


# if 96 < t2 < 123
li $t4, 96
blt $t4, $t2, CONDITION96
j INVALID

CONDITION96:
li $t4, 123
bgt $t4, $t2, VALID


# else, INVALID char
j INVALID

VALID:
# print char
li $v0, 11
move $a0, $t2
syscall

INVALID:

# increment itr and charAddress
addi $t9, $t9, 1
addi $a1, $a1, 1

j WHILE3
DONE3:






############################### Part 3: your code ends here   ##
jr $ra
###############################################################
###############################################################
###############################################################

#                          Main Function
main:

li $v0, 4
la $a0, student_name
syscall
la $a0, new_line
syscall  
la $a0, student_id
syscall 
la $a0, new_line
syscall
la $a0, new_line
syscall
##############################################
##############################################
test_64bit_Add_Unsigned:
li $s0, 3
li $s1, 0
la $s2, addition_test_data_A
la $s3, addition_test_data_B
li $v0, 4
la $a0, testing_label
syscall
la $a0, unsigned_addition_label
syscall
##############################################
test_add:
add $s4, $s2, $s1
add $s5, $s3, $s1
# Pass input parameter
lw $a0, 0($s4)
lw $a1, 4($s4)
lw $a2, 0($s5)
lw $a3, 4($s5)
jal Unsigned_Add_64bit

move $s6, $v0
move $a0, $v1
jal print_hex
move $a0, $s6
jal print_hex

li $v0, 4
la $a0, space
syscall

addi $s1, $s1, 8
addi $s0, $s0, -1
bgtz $s0, test_add

li $v0, 4
la $a0, new_line
syscall
##############################################
##############################################
test_fibonacci:
li $v0, 4
la $a0, new_line
syscall
li $v0, 4
la $a0, testing_label
syscall
la $a0, fibonacci_label
syscall 

li $s1, 6 #num test cases
la $s2, fibonacci_test_data
li $s3, 0
test_fib:
beqz $s1, fib_test_done
add $s4, $s2, $s3
lw $s0, 0($s4)

jal fibonacci

move $a0, $v0
li $v0, 1
syscall
li $v0, 4
la $a0, space
syscall

add $s3, $s3, 4
add $s1, $s1, -1
j test_fib

fib_test_done:
li $v0, 4
la $a0, new_line
syscall
##############################################
##############################################
test_file_read:
li $v0, 4
la $a0, new_line
syscall
li $s0, 0
li $v0, 4
la $a0, testing_label
syscall
la $a0, file_label
syscall 
jal file_read
end:
# end program
li $v0, 10
syscall
