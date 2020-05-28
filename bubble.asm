#bubble.asm
Main:
			 add $sp,$zero,$imm,500 # maximal row length in input
			.word 1024 _ # set A[0] = 
			.word 1025 _ # set A[1] = 
			.word 1026 _ # set A[2] = 
			.word 1027 _ # set A[3] = 
			.word 1028 _ # set A[4] = 
			.word 1029 _ # set A[5] = 
			.word 1030 _ # set A[6] = 
			.word 1031 _ # set A[7] = 
			.word 1032 _ # set A[8] = 
			.word 1033 _ # set A[9] = 
			.word 1034 _ # set A[10] =
			.word 1035 _ # set A[11] =
			.word 1036 _ # set A[12] =
			.word 1037 _ # set A[13] =
			.word 1038 _ # set A[14] =
			.word 1039 _ # set A[15] = 
			add $a0 , $zero , $imm ,1024 # loading the address of the first element of the array
			add $a1 , $zero , $zero , 16 # loading the number of elements in the array
			jal  $zero , $zero , $zero , BubbleSort # unconditional jump to BubbleSort
			halt $zero, $zero, $zero, 0
BubbleSort:		
			add  $sp, $sp, $imm, -6 # space for 6 slots
			sw   $s0 , $sp , $zero , 0 
			sw   $s1 , $sp , $zero , 1
			sw   $s2 , $sp , $zero , 2
			sw   $a0 , $sp , $zero , 3
			sw   $a1 , $sp , $zero , 4
			sw   $v0 , $sp , $zero , 5
			add  $a1 , $a1 , $imm , -1 # a1 - stores number of iteration (0--->15) inclusive
OuterLoop # if zero swapps in inner loop - break and return
			add $v0 , $zero , $imm , 0    # $v0 stores: 0 if no swaps 1 otherwise
			add $s2 , $zero , $imm, -1    # set j = -1
InnerLoop:
			add $s2 , $s2 , $imm , 1      # j++
			bge $s2 , $a1 , Condition     # checking if j>=15 
			add $t0 , $a0 , $s2           # array +j
			lw  $s0 , $t0 , $imm , 0      # $s0 = array[j]
			lw  $s1 , $t0 , $imm , 1      # $s1 = array[j+1]
			blt $s1 , $s0 , Swap          # if(array[j] > array[j+1])
			beq $imm , $zero , $zero , InnerLoop # unconditional jump
Swap:
			sw  $s1 , $t0 , $imm , 0            #set array[j] = array[j+1]
			sw  $s0 , $t0 , $imm , 1            #set A[j+1] = A[j]
			add $v0 , $t0 , $imm, 0             #set v0 = 1 (true) in other words, swap occured
			beq $imm , $zero, $zero, InnerLoop  #unconditional jump

Condition:

			bne $imm , $v0 , $zero , OuterLoop  #if swaps occured back to outerLoop
			beq $imm , $zero , $zero , Finish   #if no swaps - function execution can stop ---> break

Finish:

			lw   $s0 , $sp , $0 , 0  # restore $s0 from stack
			lw   $s1 , $sp , $0 , 1  # restore $s1 from stack
			lw   $s2 , $sp , $0 , 2  # restore $s2 from stack
			lw   $a0 , $sp , $0 , 3  # restore array address
			lw   $a1 , $sp , $0 , 4  # restore argument - number of elements
			lw   $v0 , $sp , $0 , 5  # restore return address
			add  $sp , $sp , $imm, 6 # release allocated space
			beq  $ra, $zero, $zero, 0 # return fron function
			



			


			
			
			

