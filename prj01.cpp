#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HALT 0xffffffff
#define ZERO 0x00000000

#define ADD 0x20
#define SUB 0x22
#define AND 0x24
#define OR 0x25
#define XOR 0x26
#define NOR 0x27
#define NAND 0x28
#define SLT 0x2A
#define SLL 0x00
#define SRL 0x02
#define SRA 0x03
#define JR 0x08

#define ADDI 0x08
#define LW 0x23
#define LH 0x21
#define LHU 0x25
#define LB 0x20
#define LBU 0x24
#define SW 0x2B
#define SH 0x29
#define SB 0x28
#define LUI 0x0F
#define ANDI 0x0C
#define ORI 0x0D
#define NORI 0x0E
#define SLTI 0x0A
#define BEQ 0x04
#define BNE 0x05

#define J 0x02
#define JAL 0x03

unsigned int imem[1000];
unsigned int dimg[1000];
unsigned int dmem[1000];
int sp;
int imem_len;

unsigned int reg[32];
unsigned int PC, cycle;
bool load_image(void);
void init(void);
void print_cycle(void);
void R_type(unsigned int);
void I_type(unsigned int);
void J_type(unsigned int);
int main()
{
    if(!load_image()){
        puts("Cannot load the images");
        return 0;
    }
    init();
    cycle = 0;
    int i = 2;
   // puts("");
    printf("%d\n",imem[1]);
    while(i < imem_len + 2){
       // printf("0x%08x\n",imem[i]);
        int opcode;
        opcode = imem[i] >> 26;
        printf("%08x  0x%02x ",imem[i], opcode);
        if(opcode==0x00)
            R_type(imem[i]);
        else if(opcode == J || opcode == JAL){
            J_type(imem[i]);
        }
        else{
            I_type(imem[i]);
        }


        i++;
    }

    /*unsigned int a = 0x00001235, b = 0xffffffff;
    printf("0x%08x\n",a+b);*/

    return 0;
}
void R_type(unsigned int instr)
{
    int funct = ( instr << 26 ) >> 26;
    unsigned int rs, rt, rd, C;
    rs = (instr<<6) >>27;
    rt = (instr<<11)>>27;
    rd = (instr<<16)>>27;
    C  = (instr<<21)>>27;

    switch(funct){
    case ADD:
        reg[rd] = reg[rs] + reg[rt];
        puts("add"); break;
    case SUB:
        reg[rd] = reg[rs] - reg[rt];
        puts("sub"); break;
    case AND:
        reg[rd] = reg[rs] & reg[rt];
        puts("and"); break;
    case OR:
        reg[rd] = reg[rs] | reg[rt];
        puts("or"); break;
    case XOR:
        reg[rd] = reg[rs] ^ reg[rt];
        puts("xor"); break;
    case NOR:
        reg[rd] = ~(reg[rs] | reg[rt]);
        puts("nor"); break;
    case NAND:
        reg[rd] = ~(reg[rs] & reg[rt]);
        puts("nand"); break;
    case SLT:
        reg[rd] = ( (int)reg[rs] < (int)reg[rt] );  //signed comparison
        puts("slt"); break;
    case SLL:
        reg[rd] = reg[rt] << C;
        puts("sll"); break;
    case SRL:
        reg[rd] = reg[rt] >> C;
        puts("srl"); break;
    case SRA:
        reg[rd] = (int)reg[rt] >> C;
        puts("sra"); break;
    case JR:
        PC = reg[rs];
        puts("jr"); break;
    default:
        puts("decode fail!");
    }
    printf("%d %d %d\n",rs, rt, rd, C);
}
void I_type(unsigned int instr)
{
    int opcode = instr >> 26;
    unsigned int rs, rt, uC;
    int sC;
    rs = (instr<<6) >>27;
    rt = (instr<<11)>>27;
    uC  = (instr<<16)>>16;
    sC = ((int)(instr<<16))>>16;

    switch(opcode){
    case ADDI:
        reg[rt] = reg[rs] + sC;
        puts("addi"); break;
    case LW:
        puts("lw"); break;
    case LH:
        puts("lh"); break;
    case LHU:
        puts("lhu"); break;
    case LB:
        puts("lb"); break;
    case LBU:
        puts("lbu"); break;
    case SW:
        puts("sw"); break;
    case SH:
        puts("sh"); break;
    case SB:
        puts("sb"); break;

    case LUI:
        reg[rt] = uC << 16;
        puts("lui"); break;
    case ANDI:
        reg[rt] = reg[rs] & uC;
        puts("andi"); break;
    case ORI:
        reg[rt] = reg[rs] | uC;
        puts("ori"); break;
    case NORI:
        reg[rt] = ~(reg[rs] | uC);
        puts("nori"); break;
    case SLTI:
        reg[rt] = ( (int)reg[rs] < sC );
        puts("slti"); break;

    case BEQ:
        puts("beq"); break;
    case BNE:
        puts("bne"); break;
    default:
        puts("decode fail!");
    }
}
void J_type(unsigned int instr)
{
    int opcode = instr >> 26;
    switch(opcode){
    case J:
        puts("j"); break;
    case JAL:
        puts("jal"); break;
    default:
        puts("decode fail!");
    }
}
void init(void)
{
    for(int i=0; i<32; i++)
        reg[i] = ZERO;
    reg[29] = dimg[0];
    sp = reg[29]/4;

    PC = imem[0];
    imem_len = imem[1];
}
bool load_image(void)
{
    FILE *fi = fopen("open_testcase\\func\\iimage.bin", "rb");
    FILE *fd = fopen("open_testcase\\func\\dimage.bin", "rb");
    if(!fi || !fd) return false;

    unsigned char bytes[4];
    int i = 0;
    while( fread(bytes, 4, 1, fi) != 0 ){
        imem[i] = bytes[3] | (bytes[2]<<8) | (bytes[1]<<16) | (bytes[0]<<24);
        //printf("%08x \n", imem[i]);
       i++;
    }
  //  puts("");
    i = 0;
    while( fread(bytes, 4, 1, fd) != 0 ){
        dimg[i] = bytes[3] | (bytes[2]<<8) | (bytes[1]<<16) | (bytes[0]<<24);
       // printf("%08x \n", dimg[i]);
       i++;
    }
    fclose(fi);
    fclose(fd);
    return true;
}
