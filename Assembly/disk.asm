#disktest
#############################################
# $t0 = sector number                       #
# $t1 = read/write mode to diskcmd          #
#############################################
Main:
					add $a0 , $zero , $imm , 1024              # $a0 = 1024
					add $t0 , $zero , $imm , 1                 # $t0 = 1
					out $t0 , $zero , $imm , 1                 # irq1enable = 1
					add $t0 , $zero , $imm , 6                 # $t0 = 6
					out $imm , $zero , $t0 , Interrupt         # set irqhandler label
					jal $imm , $zero , $zero , diskInitialized #jump to diskfunction
					halt $zero , $zero , $zero , 0             #finish program

DiskInitialized:
					add  $sp, $sp, $imm, -1                   # space for 1 slot
					sw   $a0 , $sp , $zero , 0				  # saving $a0
					add $t0 , $zero , $zero , 0               # $t0=0
					add $t1 , $zero , $zero ,1                # $t1=1

DiskReadWrite:
			
					out $t0 , $zero , $imm , 15               #setting disksector
					out $a0 , $zero , $imm , 16               #setting diskbuffer to 1024
					out $t1 , $zero , $imm , 14               #setting diskcmd as $t1

Cool-down:

					beq $imm , $zero , $zero , Cool-down
					beq $imm , $zero , $zero , DiskReadWrite

Interrupt: 

					out $zero , $zero , $imm , 4             # irq1status = 0
					add $t2 , $zero , $imm , 1               # $t2 = 1
					bne $imm , $t1 , $t2 , Reading-switch    # jump to the Reading-switch epoch becasue the last diskcmd was writing ---> bench if the last diskcmd was writing
					add $t1 , $zero , $imm , 2               # $t1 = 2
					add $t0 , $t0 , $imm , 4                 # $t0+=4
					reti $zero , $zero , $zero , 0           # return from interrupt (Cool-down second line)

Reading-switch:

					add $t1 , $zero , $imm , 1               # $t1 = 1 indicating on reading mode
					add $t0 , $t0 , imm , -3                 # $t0-=3
					beq $ra , $t0 , $imm , 4                 # if we reached to read from sector number 4 - break!
					reti $zero , $zero , $zero , 0           # return to read-write another sector



		






		


