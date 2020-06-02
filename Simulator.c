#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define TRUE 1 
#define FALSE 0

#define MAX_LINES 4096 
#define SIZE 9 //included the "/0" sign
#define LINE_SIZE 8
#define REG_NUM 16
#define IOREG_NUM 18
#define LEDS 32
/*Utility func*/
int getHex(char* source);
int hex2int(char ch);
int getAddress(int address);
/*Create func*/
void createTrace(FILE* trace, int pc, char line[SIZE], int* reg[REG_NUM], int* count);
void createLastFiles(FILE* memout, char memory_out[MAX_LINES][SIZE], int max_line_counter, FILE* regout, int* reg[REG_NUM], FILE* count, int counter);

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




int main(int argc, const char* argv[])
{
	if (argc < 12)
	{
		printf("Arg Amount Error");
		return 0;
	}
	FILE* memin, *diskin, *irq2in, *memout, *regout, *trace, *hwregtrace, *cycles, *leds, *display, *diskout;
	memin = fopen(argv[1], "r");
	diskin = fopen(argv[2], "r");
	irq2in = fopen(argv[3], "r");
	memout = fopen(argv[4], "w");
	regout = fopen(argv[5], "w");
	trace = fopen(argv[6], "w");
	hwregtrace = fopen(argv[7], "w");
	cycles = fopen(argv[8], "w");
	leds = fopen(argv[9], "w");
	display = fopen(argv[10], "w");
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
	for (int i = 0; i < IOREG_NUM; i++)
	{
		IOreg[i] = 0;
	}
	IOreg[9] = leds;
	

	
	char memory_in[MAX_LINES][LINE_SIZE], memory_out[MAX_LINES][SIZE];//????
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
		char line[SIZE];
		snprintf(line, SIZE, "%s%c", memory_in[pc], '\0');
		printf("%s", line);
		createTrace(trace, pc, line, reg, counter);
		printf(" %d\n", *counter);
		decipher_line(line, reg, memory_in, memory_out, pcp, max_line_counter_ptr);

	}
	//Exit:	
	fclose(trace);
	createLastFiles(memout, memory_out, max_line_counter, regout, reg, count, *counter);
	fclose(memout);
	fclose(regout);
	fclose(count);
}



/*Utility Func*/
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


/*Create Func*/
void createLastFiles(FILE* memout, char memory_out[MAX_LINES][SIZE], int max_line_counter, FILE* regout, int* reg[REG_NUM], FILE* count, int counter)
{
	for (int i = 0; i < REG_NUM; i++)
	{
		fprintf(regout, "%04X\n", *reg[i]);
	}
	for (int i = 0; i < max_line_counter; i++)
	{
		int instruction = getHex(memory_out[i]);
		if (instruction < 0)
			instruction -= 0xFFFF0000;
		fprintf(memout, "%04X\n", instruction);
	}
	fprintf(count, "%d\n", counter);
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

	int opcode = hex2int(line[0]);
	int rd = hex2int(line[1]);
	int rs = hex2int(line[2]);
	int rt = hex2int(line[3]);//where is the immediate??????????
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

