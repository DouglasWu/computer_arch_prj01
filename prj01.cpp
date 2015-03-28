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

#define MEM_SIZE 1000

unsigned int imem[MEM_SIZE];
unsigned int dimg[MEM_SIZE];
unsigned int dmem[MEM_SIZE];
unsigned int sp;
unsigned int iSize, dSize;
unsigned int reg[32];
unsigned int PC, cycle;
unsigned int PC_init;
bool pcChanged;

FILE *snapshot, *error_dump;

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
    PC = PC_init;
    print_cycle();
    int i = 0;
    while( imem[i] != HALT ){
       // printf("0x%08x\n",imem[i]);

        int opcode;
        pcChanged = false;
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
        if(!pcChanged)
            PC = PC + 4;

        cycle++;
        print_cycle();
        i = (PC-PC_init)/4;
       // printf("i: %d\n",i);
        //system("pause");
    }


    fclose(snapshot);
    fclose(error_dump);

    return 0;
}
void print_cycle(void)
{
    fprintf(snapshot, "cycle %d\n",cycle);
    for(int i=0; i<32; i++)
        fprintf(snapshot, "$%02d: 0x%08X\n",i,reg[i]);
    fprintf(snapshot,"PC: 0x%08X\n\n\n",PC);
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
        pcChanged = true;
        puts("jr");
        printf("rs: %d, reg[rs]:%08x\n", rs, reg[rs]);
        break;
    default:
        puts("decode fail!");
    }
    //printf("%d %d %d\n",rs, rt, rd, C);
}
void I_type(unsigned int instr)
{
    int opcode = instr >> 26;
    unsigned int rs = (instr<<6) >>27;
    unsigned int rt = (instr<<11)>>27;
    unsigned int uC  = (instr<<16)>>16;
    int sC = ((int)(instr<<16))>>16;
    unsigned int addr = reg[rs] + sC;/**handle memory addr overflow*/
    unsigned int masks[4] = {0xffffff00, 0xffff00ff, 0xff00ffff, 0x00ffffff};
    int shift, save, tmp;
    unsigned int tmpu;


    switch(opcode){
    case ADDI:
        reg[rt] = reg[rs] + sC;
        puts("addi"); break;

    case LW:
        if( addr%4 != 0 ){
            /**error handle*/
            puts("memory aligned error!!");
        }else{
            reg[rt] = dmem[addr/4];
        }
        puts("lw"); break;
    case LH: //signed
        if( addr%2 != 0 ){
            /**error handle*/
            puts("memory aligned error!!");
        }else{
            int tmp = dmem[addr/4];
            reg[rt] = addr%4==0 ? (tmp<<16)>>16 : tmp>>16;
        }
        puts("lh"); break;
    case LHU:
        if( addr%2 != 0 ){
            /**error handle*/
            puts("memory aligned error!!");
        }else{
            unsigned int tmp = dmem[addr/4];
            reg[rt] = addr%4==0 ? (tmp<<16)>>16 : tmp>>16;
        }
        puts("lhu"); break;
    case LB:
        tmp = dmem[addr/4];
        shift = 24 - (addr%4)*8;
        reg[rt] = (tmp << shift) >> 24;

        puts("lb"); break;
    case LBU:
        tmpu = dmem[addr/4];
        shift = 24 - (addr%4)*8;
        reg[rt] = (tmpu << shift) >> 24;

        puts("lbu"); break;
    case SW:
        if( addr%4 != 0 ){
            /**error handle*/
            puts("memory aligned error!!");
        }else{
            dmem[addr/4] = reg[rt];
        }

        puts("sw"); break;
    case SH:
        if( addr%2 != 0 ){
            /**error handle*/
            puts("memory aligned error!!");
        }else{
            tmpu = dmem[addr/4];
            save = reg[rt] & 0x0000ffff;
            if(addr%4==0){
                dmem[addr/4] = ((tmpu>>16)<<16) + save;
            }else{
                dmem[addr/4] = (tmpu&0x0000ffff) + (save<<16);
            }
        }

        puts("sh"); break;
    case SB:
        tmpu = dmem[addr/4];
        save = reg[rt] & 0x000000ff;
        shift = (addr%4)*8;
        dmem[addr/4] = (tmpu & masks[addr%4]) + (save<<shift);

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
        if(reg[rs]==reg[rt]){
            PC = (PC+4) + sC*4;
            pcChanged = true;
        }

        puts("beq");
        break;
    case BNE:
        if(reg[rs]!=reg[rt]){
            printf("");
            PC = (PC+4) + sC*4;
            pcChanged = true;
        }
        puts("bne");

        //printf("%d %d %x\n",rs, rt, sC);
        break;
    default:
        puts("decode fail!");
    }
}
void J_type(unsigned int instr)
{
    int opcode = instr >> 26;
    unsigned int C = (instr<<6)>>6;


    if(opcode == JAL){
        reg[31] = PC + 4;
        puts("jal");
    }
    else puts("j");
    PC = ((PC+4) & 0xf0000000) | (C*4);
    pcChanged = true;
}
void init(void)
{
    for(int i=0; i<32; i++)
        reg[i] = ZERO;
    reg[29] = sp;

    snapshot = fopen("snapshot.rpt", "w");
    error_dump = fopen("error_dump.rpt", "w");
}
bool load_image(void)
{
    FILE *fi = fopen("open_testcase\\recur\\iimage.bin", "rb");
    FILE *fd = fopen("open_testcase\\recur\\dimage.bin", "rb");
    if(!fi || !fd) return false;

    unsigned char bytes[4];
    int i = 0;
    while( fread(bytes, 4, 1, fi) != 0 ){
        if(i==0){
            PC_init = bytes[3] | (bytes[2]<<8) | (bytes[1]<<16) | (bytes[0]<<24);
        }
        else if(i==1){
            iSize = bytes[3] | (bytes[2]<<8) | (bytes[1]<<16) | (bytes[0]<<24);
        }
        else{
            imem[i-2] = bytes[3] | (bytes[2]<<8) | (bytes[1]<<16) | (bytes[0]<<24);
            printf("%08x \n", imem[i-2]);
        }

        i++;
    }
    puts("");
    i = 0;
    while( fread(bytes, 4, 1, fd) != 0 ){
        if(i==0){
            sp = bytes[3] | (bytes[2]<<8) | (bytes[1]<<16) | (bytes[0]<<24);
        }
        else if(i==1){
            dSize = bytes[3] | (bytes[2]<<8) | (bytes[1]<<16) | (bytes[0]<<24);
        }
        else{
            dmem[i-2] = bytes[3] | (bytes[2]<<8) | (bytes[1]<<16) | (bytes[0]<<24);
            printf("%08x \n", dmem[i-2]);
        }
        i++;
    }
    fclose(fi);
    fclose(fd);
    return true;
}
