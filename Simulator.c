#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define TRUE 1 
#define FALSE 0


#define MAX_LINES 4096 
#define SIZE 9 //included the "/0" sign. had to be checked maybe its 8 ????????????
#define LINE_SIZE 8
#define REG_NUM 16
#define IOREG_NUM 18
#define LEDS 32
#define MAX_WORD_LENGTH_HW 15
#define HW_COLS 4
#define lEDS_COLS 2
 

/*Utility func
 * /
int getHex(char* source);


changing!!!!
change2




int hex2int(char ch);
int getAddress(int address);
/*Create func*/
void createTrace(FILE* trace, int pc, char line[SIZE], int* reg[REG_NUM], int* count);
void createLastFiles(FILE* memout, char memory_out[MAX_LINES][SIZE], int max_line_counter, FILE* regout, int* reg[REG_NUM], FILE* cycles, int total_cycles, int max_line_hw, FILE* hwregtraceF, char *** hw_matrix,
	FILE* ledsF, int** Leds_matrix, int max_line_leds, FILE* displayF, int** display_matrix, int max_line_display);

/*Line Func*/
void add(int rd, int rs, int rt, int* reg[REG_NUM]);
void sub(int rd, int rs, int rt, int* reg[REG_NUM]);
void mul(int rd, int rs, int rt, int* reg[REG_NUM]);
void andf(int rd, int rs, int rt, int* reg[REG_NUM]);
void orf(int rd, int rs, int rt, int* reg[REG_NUM]);
void sll(int rd, int rs, int rt, int* reg[REG_NUM]);
void sra(int rd, int rs, int rt, int* reg[REG_NUM]);
void limm(int rd, char memory_in[MAX_LINES][LINE_SIZE], int* reg[REG_NUM], int pc);
void branch(int rd, int rs, int rt, char memory_in[MAX_LINES][LINE_SIZE], int* reg[REG_NUM], int* pc);
void jal(int* reg[REG_NUM], int* pc, char memory_in[MAX_LINES][LINE_SIZE]);
void lw(int rd, int rs, int* reg[REG_NUM], char memory_out[MAX_LINES][SIZE], int pc);
void sw(int rd, int rs, int* reg[REG_NUM], char memory_in[MAX_LINES][LINE_SIZE], char memory_out[MAX_LINES][SIZE], int pc, int *max_line_counter);
void halt(int* pc);
void decipher_line(char line[SIZE], int* reg[REG_NUM], char memory_in[MAX_LINES][LINE_SIZE], char memory_out[MAX_LINES][SIZE], int* pc, int *max_line_counter_ptr);
void irq_update(int irq0Status, int irq1Status, int irq2Status, int irq0Enable, int irq1Enable, int irq2Enable, int irqhandler, int *pc);
void irq1_helper(int disk_stating_pc, long clks, int *diskCmd, int diskStatus, int irq1status, int *irq1Enable, int diskbuffer, char** memoryout, FILE* diskOut);


int main(int argc, const char* argv[])
{
	char hw_matrix[MAX_LINES][HW_COLS][MAX_WORD_LENGTH_HW]; //hwregtrace matrix
	int leds_matrix[MAX_LINES][lEDS_COLS]; // leds matrix
	int display_matrix[MAX_LINES][lEDS_COLS];// display matrix
	char memory_in[MAX_LINES][LINE_SIZE], memory_out[MAX_LINES][SIZE], disk_out_matrix[MAX_LINES][LINE_SIZE];
	int max_line_disk_out = 0; int irq0enable = 0; int irq1enable = 0; int irq2enable = 0; int irq0status = 0; int irq1status = 0; int irq2status = 0;
	int irqhandler = 0; int irqreturn = 0; long clks = 0; int leds[32]; int* display[8]; int timerenable = 0;
	long timercurrent = 0; int timermax = 0; int diskcmd = 0; int disksector = 0; int diskbuffer = 0; int diskstatus = 0; int disk_starting_pc = 0;
	int disk_stating_pc; int total_cycles = 0; int max_line_hw = 0; int max_line_leds = 0; int max_line_display = 0;
	if (argc < 12)
	{
		printf("Arg Amount Error");
		return 0;
	}
	FILE* memin, *diskin, *irq2in, *memout, *regout, *trace, *hwregtrace, *cycles, *ledsF, *displayF, *diskout;
	memin = fopen(argv[1], "r");
	diskin = fopen(argv[2], "r");
	irq2in = fopen(argv[3], "r");
	memout = fopen(argv[4], "w");
	regout = fopen(argv[5], "w");
	trace = fopen(argv[6], "w");
	hwregtrace = fopen(argv[7], "w");
	cycles = fopen(argv[8], "w");
	ledsF = fopen(argv[9], "w");
	displayF = fopen(argv[10], "w");
	diskout = fopen(argv[11], "w");

	if (memin == NULL || diskin == NULL || irq2in == NULL || memout == NULL || regout == NULL || trace == NULL || hwregtrace == NULL || cycles == NULL || leds == NULL || display == NULL || diskout == NULL)
	{
		printf("FILE Error");
		return 0;
	}

	int cnt = 0, pc = 0; int max_line_counter = 0;
	int *max_line_counter_ptr = &max_line_counter;
	int reg_b[REG_NUM] = { 0 };
	int* reg[REG_NUM];
	
	int* IOreg[IOREG_NUM];
	int leds[LEDS];
	//register number assignments
	for (int i = 0; i < REG_NUM ; i++)
	{
		reg[i] = &reg_b[i];
	}
	
	
	clone_matrix(disk_out_matrix, diskin, &max_line_disk_out);
	int* counter = &cnt; int* pcp = &pc;

	while (!feof(memin))//memory_in and memory_out are filled
	{
		int check;
		if (check = fscanf(memin, "%s", memory_in[max_line_counter]) != 0) { //Consider \n in fscanf and avoid using memout because probably there are changes to the code line pheraps there should be. filling the matrix of memory_in.
			snprintf(memory_out[max_line_counter], SIZE, "%s%c", memory_in[max_line_counter], '\n'); 
			max_line_counter++;
		}
	}
	fclose(memin);


	while (pc > -1)
	{	
		irq1_helper(disk_starting_pc, clks, &diskcmd, diskbuffer, diskstatus, &irq1enable,diskbuffer, memory_out, &diskout);
		irq_update(irq0status, irq1status, irq2status, irq0enable, irq1enable, irq2enable, irqhandler, &pc);
		char line[SIZE];
		snprintf(line, SIZE, "%s%c", memory_in[pc], '\0');
		printf("%s", line);
		createTrace(trace, pc, line, reg, counter);
		printf(" %d\n", *counter);
		decipher_line(line, reg, memory_in, memory_out, pcp, max_line_counter_ptr);
		pc_loop(&clks);// clock increment
		total_cycles += 1;
		
	}
	//Exit:	
	fclose(trace);
	createLastFiles(memout, memory_out, max_line_counter, regout, reg, &cycles, total_cycles, max_line_hw, &hwregtrace, hw_matrix, &ledsF, leds_matrix, max_line_leds, &displayF, display_matrix, max_line_display);
	fclose(memout);
	fclose(regout);
	fclose(cycles);
}



/*Utility Func*/

// initiazling the content of file"diskin" to matrix disk_out_matrixs
void clone_matrix(char ** disk_out_matrix, FILE* disk_in, int* max_line_disk_out) {
	int max_line_disk_out = 0;
	while (!feof(disk_in))//memory_in and memory_out are filled
	{
		int check;
		if (check = fscanf(disk_in, "%s", disk_out_matrix[*max_line_disk_out]) != 0) { //Consider \n in fscanf and avoid using memout because probably there are changes to the code line pheraps there should be. filling the matrix of memory_in.
			max_line_disk_out++;
		}
	}
}
void pc_loop(int *clks) {
	if (*clks == 4294967295) { // convert of 0xFFFFFFFF
		*clks = 0;
	}
	else (*clks)++;
	}
int getHex(char* source)
{
	int n = (int)strtol(source, NULL, 16);
	if (n > 0x7fff)
		n -= 0x10000;
	return n;
}
int hex2int(char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	printf("Char given, %c, is invalid - non hex character.\n", ch);
	return -1;
}
int getAddress(int address)
{
	if (address < 0)
	{
		printf("Address given, %X, is invalid - negative.\n", address);
		return -10;
	}

	if (address >= 4096)
	{
		printf("Address given, %X, is invalid - exceeds limited space in memory.\n", address);
		address = address & 0x0FFF; //if given address is too high, take only 12 LSBs.
		printf("Simulator refers only to 12 LSBs, %X in this case.\n", address);
	}

	return address;
}

/*
 1.updates the irq1status and irq1enable.
 2.checking if 1024 clock cycles have pass - if yes execute the read/write operation
 
*/
void irq1_helper(int disk_stating_pc, long clks, int *diskCmd, int diskStatus,int irq1status, int *irq1Enable,int diskbuffer,char** memoryout, FILE* diskOut) {

}
/*
 1.updates the irq0status and irq0enable.
 2.updates the irq2status and irq2enable.
 3.For the timer case(irq0) update the timer respectively (if timerEnable = 1)
*/
void irq02_status_update(int* irq0Status, int* irq1Status, int* irq2Status, int irq0Enable, int irq1Enable, int irq2Enable, int[] irq2arr, int clock) {
		
}
/*
1. return array contain the pc's(addresses) values of external interrupts in those addresses(irq2 type)
*/
int* irq2_array(FILE* irq2in) {

}

void irq_update(int irq0Status, int irq1Status, int irq2Status, int irq0Enable, int irq1Enable, int irq2Enable, int irqhandler, int *pc) {
	int res;
	res=((irq0Enable & irq0Status) | (irq1Enable & irq1Status) | (irq2Enable & irq2Status));
	if (res != 0) {
		*pc = irqhandler;
	}
}


/*Create Func*/
/*
  Updating hwregrtace matrix (and more anf more)
*/
void createLastFiles(FILE* memout, char memory_out[MAX_LINES][SIZE], int max_line_counter, FILE* regout, int* reg[REG_NUM], FILE* cycles, int total_cycles, int max_line_hw, FILE* hwregtraceF, char *** hw_matrix,
FILE* ledsF, int** Leds_matrix, int max_line_leds, FILE* displayF, int** display_matrix, int max_line_display, int max_line_disk_out, FILE* disk_out, char disk_out_matrix[MAX_LINES][SIZE])
{
	for (int i = 0; i < REG_NUM; i++)
	{
		fprintf(regout, "%08X\n", *reg[i]);
	}
	for (int i = 0; i < max_line_counter; i++)
	{
		int instruction = getHex(memory_out[i]);
		if (instruction < 0)
			instruction -= 0xFFFFFFFF00000000;
		fprintf(memout, "%08X\n", instruction);
	}
	fprintf(cycles, "%d\n", total_cycles);
	//hwregtrace matrix loading
	for (int i = 0; i < max_line_hw; i++){
		fprintf(hwregtraceF, "%s %s %s %s %s\n", hw_matrix[i][0], hw_matrix[i][1], hw_matrix[i][2], hw_matrix[i][3]);
	}
	//leds matrix update
	for (int i = 0; i < max_line_leds; i++){
		fprintf(ledsF, "%d %08X\n", Leds_matrix[0], Leds_matrix[1]);
	}
	// display matrix update
	for (int i = 0; i < max_line_leds; i++) {
		fprintf(displayF, "%d %08X\n", display_matrix[0], display_matrix[1]);
	}
	// updates into diskout file the matrix 'disk-out-matrix'
	for (int i = 0; i < max_line_disk_out; i++)
	{
		fprintf(disk_out, "%08X\n", disk_out_matrix[i]);
	}
	






}
void createTrace(FILE* trace, int pc, char line[SIZE], int* reg[REG_NUM], int* count)
{
	fprintf(trace, "%08X %s ", pc, line);
	for (int i = 0; i < 16; i++)
	{
		int instruction = *reg[i];
		if (instruction < 0)
			instruction -= 0xFFFFFFFF00000000;//?????????
		fprintf(trace, "%08X ", instruction);
	}
	fprintf(trace, "\n");
	(*count)++;
}


/*LINE Func*/
void decipher_line(char line[SIZE], int* reg[REG_NUM], char memory_in[MAX_LINES][LINE_SIZE], char memory_out[MAX_LINES][SIZE], int* pc, int* max_line_counter_ptr) {

	int opcode = hex2int(line[0]) * 16 + hex2int(line[1]);
	int rd = hex2int(line[2]);
	int rs = hex2int(line[3]);
	int rt = hex2int(line[4]);
	*reg[1] = hex2int(line[5]) * 16 * 16 + hex2int(line[6]) * 16 + hex2int(line[7]); // immediate update
	
	/*if (opcode == 20) //.word
	{
		halt(pc);
		return;
	}*/
	
	switch (opcode)
	{
	case 0:	add(rd, rs, rt, reg); break;	//The opcode is add
	case 1: sub(rd, rs, rt, reg); break;    //The opcode is sub
	case 2: andf(rd, rs, rt, reg); break;    //The opcode is and
	case 3: orf(rd, rs, rt, reg); break;    //The opcode is or
	case 4: sll(rd, rs, rt, reg); break;    //The opcode is sll
	case 5: sra(rd, rs, rt, reg); break;    //The opcode is sra
	case 6: srl(rd, rs, rt, reg); break;    //The opcode is srl?????????????????????????????????????????????????????????????
	case 7: beq(rd, rs, rt, memory_in, reg, pc); break;    //The opcode is beq?????????????????????????????????????????????????????????????
	case 8: bne(rd, rs, rt, memory_in, reg, pc); break;    //The opcode is ben?????????????????????????????????????????????????????????????
	case 9: blt(rd, rs, rt, memory_in, reg, pc); break;    //The opcode is blt?????????????????????????????????????????????????????????????
	case 10: bgt(rd, rs, rt, memory_in, reg, pc); break;    //The opcode is bgt?????????????????????????????????????????????????????????????
	case 11: ble(rd, rs, rt, memory_in, reg, pc); break;    //The opcode is ble?????????????????????????????????????????????????????????????
	case 12: bge(rd, rs, rt, memory_in, reg, pc); break;    //The opcode is bge?????????????????????????????????????????????????????????????
	case 13: jal(reg, pc, memory_in); break;    //The opcode is jal
	case 14: lw(rd, rs, reg, memory_out, *pc); break;    //The opcode is lw
	case 15: sw(rd, rs, reg, memory_in, memory_out, *pc, max_line_counter_ptr); break;    //The opcode is sw
	case 16: reti(); break;
	case 17: in(); break;
	case 18: out(); break;
	case 19: halt(); break;

	}
	*pc += 1;
}
void add(int rd, int rs, int rt, int* reg[REG_NUM])
{
	*reg[rd] = *reg[rs] + *reg[rt];
}
void sub(int rd, int rs, int rt, int* reg[REG_NUM])
{
	*reg[rd] = *reg[rs] - *reg[rt];
}
void andf(int rd, int rs, int rt, int* reg[REG_NUM])
{
	*reg[rd] = *reg[rs] & *reg[rt];
}
void orf(int rd, int rs, int rt, int* reg[REG_NUM])
{
	*reg[rd] = *reg[rs] | *reg[rt];
}
void sll(int rd, int rs, int rt, int* reg[REG_NUM])
{
	*reg[rd] = *reg[rs] << *reg[rt];
}
void sra(int rd, int rs, int rt, int* reg[REG_NUM])
{
	if (*reg[rs] >= (int)pow(2, 15))
	{
		*reg[rd] = *reg[rs] >> *reg[rt];
		int p = 15;
		for (int i = 0; i < *reg[rt]; i++, p--)
			*reg[rd] += (int)pow(2, p);

	}
	else
		*reg[rd] = *reg[rs] >> *reg[rt];
}
void srl(int rd, int rs, int rt, int* reg[REG_NUM]) {//?????????????????????????????????
	
}
void beq(int rd, int rs, int rt, char memory_in[MAX_LINES][LINE_SIZE], int* reg[REG_NUM], int* pc) {
	
}
void bne(int rd, int rs, int rt, char memory_in[MAX_LINES][LINE_SIZE], int* reg[REG_NUM], int* pc) {

}
void blt(int rd, int rs, int rt, char memory_in[MAX_LINES][LINE_SIZE], int* reg[REG_NUM], int* pc) {

}
void bgt(int rd, int rs, int rt, char memory_in[MAX_LINES][LINE_SIZE], int* reg[REG_NUM], int* pc) {

}
void ble(int rd, int rs, int rt, char memory_in[MAX_LINES][LINE_SIZE], int* reg[REG_NUM], int* pc) {

}
void bge(int rd, int rs, int rt, char memory_in[MAX_LINES][LINE_SIZE], int* reg[REG_NUM], int* pc) {

}
/*
######################## IN AND OUT #####################################
1. It is necessary to check diskStatus and update the values accordingly. 
2. Update IORegisters
3. Update hwregtrace matrix(Strings) + max_line_hw
4. Update the lEDS matrix 
5.while updating mem-out, we have to increment by 1 the variable "max_line_disk_out"
###############################Naor Ashtar the(my) monkey###########################
*/

void in(int rd, int rs, int rt, char memory_in[MAX_LINES][LINE_SIZE], int* reg[REG_NUM], int* pc, int* max_line_hw) {

}

/*void branch(int rd, int rs, int rt, char memory_in[MAX_LINES][LINE_SIZE], int* reg[REG_NUM], int* pc)
{
	char line[SIZE];
	snprintf(line, SIZE, "%s%c", memory_in[*pc + 1], '\0');
	int imm = getHex(line);
	switch (rd)
	{
	case 0:
		if (*reg[rs] == *reg[rt])  //beq
			*pc = getAddress(imm);
		else *pc += 2;
		break;
	case 1:
		if (*reg[rs] != *reg[rt])  //bne
			*pc = getAddress(imm);
		else *pc += 2;
		break;
	case 2:
		if (*reg[rs] > *reg[rt]) //branch if greater than - bgt
			*pc = getAddress(imm);
		else *pc += 2;
		break;
	case 3:
		if (*reg[rs] < *reg[rt]) // branch if smaller than - bst
			*pc = getAddress(imm);
		else *pc += 2;
		break;
	case 4:
		if (*reg[rs] >= *reg[rt]) // beq or bgt
			*pc = getAddress(imm);
		else *pc += 2;
		break;
	case 5:
		if (*reg[rs] <= *reg[rt]) // beq or bst
			*pc = getAddress(imm);
		else *pc += 2;
		break;
	case 6: *pc = getAddress(*reg[rs]);
	}
}*/
void jal(int** reg, int* pc, char memory_in[MAX_LINES][LINE_SIZE])
{
	*reg[15] = *pc + 2;
	char line[SIZE];
	snprintf(line, SIZE, "%s%c", memory_in[*pc + 1], '\0');
	int imm = getHex(line);
	*pc = getAddress(imm);
}
void lw(int rd, int rs, int* reg[REG_NUM], char memory_out[MAX_LINES][SIZE], int pc)
{
	char line[SIZE], line1[SIZE];
	snprintf(line, SIZE, "%s%c", memory_out[pc + 1], '\0');
	int imm = getHex(line);
	int address = getAddress(imm + *reg[rs]);
	snprintf(line1, SIZE, "%s%c", memory_out[address], '\0');
	*reg[rd] = getHex(line1);
}
void sw(int rd, int rs, int* reg[REG_NUM], char memory_in[MAX_LINES][LINE_SIZE], char memory_out[MAX_LINES][SIZE], int pc, int *max_line_counter_ptr)
{
	char line[SIZE];
	snprintf(line, SIZE, "%s%c", memory_in[pc + 1], '\0');
	int imm = getHex(line);
	int address = getAddress(imm + *reg[rs]);
	snprintf(memory_out[address], SIZE, "%04X%c", *reg[rd], '\n');
	if (*max_line_counter_ptr < address)//&& *reg[rd] != 0
		*max_line_counter_ptr = address;
}
void reti() {

}
void in() {

}
void out() {

}


void halt(int* pc)
{
	*pc = -10;
}

