#clocks.asm


Main:
				.word 1024 0x195955                  # starting time
				.word 1025 0x1fffff                  # one second before the switch of 200000
				add $t0  ,  $zero , $imm , 1         # $t0 = 1
				out $t0  ,  $zero , $imm , 0         # irq0 enable
				add $t0  ,  $zero , $imm , 6         # $t0 = 6
				out $imm ,  $t0 , $zero  , Interrupt # set irqhandeler-->PC of interrupt handler
				jal $imm , $zero , $zero , clockRun  # (unconditional jump) startin the function of clock counting from 19:59:00 to 20:00:05
				halt $zero , $zero , $zero           # finishing clock-test

clockRun:
               add $t0 , $zero , $imm , 254          # $t0 =  254
			   out $t0 , $zero , $imm , 13           # timermax = 254 clock cycles
			   lw  $t0 , $zero , $imm , 1024         # storing the starting time (19:59:55)to $t0
			   out $t0 , $zero , $imm , 10           # enabling the display 
			   add $t1 , $zero , $imm , 1            # $t1 = 1
			   out $t1 , $zero , $imm , 11           # timerenable = 1

Cool-down:
				
			  beq $imm , $zero , $zero , Cool-down   # staying in this loop until timermax reached and before moving to Interrupt
			  beq $imm , $zero , $zero , Cool-down   # staying in the loop 255 cycles

Interrupt:
	
			add $t0 , $t0 , $imm , 1                 # $t0 = $t0 +1
			out $t0 , $zero , $imm , 10              # display the next second. here we finish 256 clock cycles which equally to 1 sec
			add $t1 , $zero , $imm , 3               # $t1 = 3
			out $t1 , $zero , $imm , 0               # turning off irq0status after timermax reached
			and $t1 , $t0 , $imm , 15                # masking the 4 LSB bits with 1's mask
			add $t2 , $zero , $imm , 9               # $s2 = 9
			beq $imm , $t1 , $t2 , Corner-case       # fixing corner case of 19:59:59
			and $t1 , $t0 , $imm , 15                # masking the 4 LSB bits with 1's mask 
			beq $ra , $t1 , $imm 5                   # cheacking if the last digit(4LSB's) is 5 if yes break
			reti $zero , $zero , $zero , 0           # return from the Interrupt

Corner-case:

			lw $t0 , $zero , $imm , 1025             # loading the time second before 200000
			out $zero , $zero , $imm , 3             # irq0status = 0
			reti , $zero , $zero , $zero , 0         # return from Interrunpt
