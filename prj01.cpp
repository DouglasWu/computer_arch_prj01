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

#define MEM_SIZE 1024

#define WRITE_ZERO 0
#define NUMBER_OVERFLOW 1
#define ADDRESS_OVERFLOW 2
#define MISALIGNMENT 3

unsigned int imem[MEM_SIZE/4];
unsigned int dmem[MEM_SIZE/4];
unsigned int sp;
unsigned int iSize, dSize;
unsigned int reg[32];
unsigned int PC, cycle;
unsigned int PC_init;
bool pcChanged;
bool error_halt;

FILE *snapshot, *error_dump;

bool load_image(void);
void init(void);
void print_cycle(void);
void R_type(unsigned int);
void I_type(unsigned int);
void J_type(unsigned int);
void print_error(int,int);
bool has_overflow(int, int, int);
void check_errors(int, int, int, int, int);
int main()
{
    if(!load_image()){
        puts("Cannot load the images");
        return 0;
    }
    init();

    cycle = 0;
    PC = PC_init;
    error_halt = false;
    print_cycle();
    int i = 0;
    while( imem[i] != HALT ){
       // printf("0x%08x\n",imem[i]);

        int opcode = imem[i] >> 26;;
        pcChanged = false;
        printf("%08x  0x%02x ",imem[i], opcode);
        cycle++;//先增值因為error message會用到
        if(opcode==0x00){
            R_type(imem[i]);
        }
        else if(opcode == J || opcode == JAL){
            J_type(imem[i]);
        }
        else{
            I_type(imem[i]);
        }
        if(error_halt){
            break;
        }

        if(!pcChanged)
            PC = PC + 4;
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
void print_error(int type, int cyc)
{
    switch(type){
        case WRITE_ZERO:
            fprintf( error_dump , "In cycle %d: Write $0 Error\n", cyc);
            break;
        case NUMBER_OVERFLOW:
            fprintf(error_dump , "In cycle %d: Number Overflow\n", cyc);
            break;
        case ADDRESS_OVERFLOW:
            fprintf(error_dump , "In cycle %d: Address Overflow\n", cyc);
            break;
        case MISALIGNMENT:
            fprintf(error_dump , "In cycle %d: Misalignment Error\n", cyc);
            break;
    }
}
bool has_overflow(int c, int a, int b)
{
    if( (a>>31)==(b>>31) && (a>>31)!=(c>>31) )
        return true;
    return false;
}
void R_type(unsigned int instr)
{
    int funct = ( instr << 26 ) >> 26;
    unsigned int rs = (instr<<6) >>27;
    unsigned int rt = (instr<<11)>>27;
    unsigned int rd = (instr<<16)>>27;
    unsigned int C  = (instr<<21)>>27;

    if(funct!=JR && rd==0 && instr!=0x00000000){
        print_error(WRITE_ZERO, cycle);
        return;
    }

    switch(funct){
    case ADD:
        reg[rd] = reg[rs] + reg[rt];
        if( has_overflow(reg[rd], reg[rs], reg[rt]) )
            print_error(NUMBER_OVERFLOW, cycle);

        puts("add"); break;
    case SUB:
        reg[rd] = reg[rs] - reg[rt];
        if( has_overflow(reg[rd], reg[rs], -reg[rt]) )
            print_error(NUMBER_OVERFLOW, cycle);

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
        break;
    default:
        puts("decode fail!");
    }

    //printf("%d %d %d\n",rs, rt, rd, C);
}
void check_errors(int rt, int addr, int reg_s, int sC, int op)
{
    if(rt==0)
        print_error(WRITE_ZERO, cycle);
    if( has_overflow(addr, reg_s, sC) )
        print_error(NUMBER_OVERFLOW, cycle);
    if(addr >= MEM_SIZE){
        print_error(ADDRESS_OVERFLOW, cycle);
        error_halt = true;
    }
    int unit;
    if(op==LW || op==SW) unit = 4;
    else if(op==LH || op==LHU || SH) unit = 2;
    else unit = 1;

    if(addr%unit!=0){
        print_error(MISALIGNMENT, cycle);
        error_halt = true;
    }
}
void I_type(unsigned int instr)
{
    int opcode = instr >> 26;
    unsigned int rs = (instr<<6) >>27;
    unsigned int rt = (instr<<11)>>27;
    unsigned int uC  = (instr<<16)>>16;
    int sC = ((int)(instr<<16))>>16;
    unsigned int masks[4] = {0xffffff00, 0xffff00ff, 0xff00ffff, 0x00ffffff};
    int shift, save, tmp;
    unsigned int tmpu;
    bool overflow = false;
    unsigned int addr = reg[rs] + sC;/**handle memory addr overflow*/

    switch(opcode){
    case ADDI:
        if(rt==0){
            print_error(WRITE_ZERO, cycle);
        }
        reg[rt] = reg[rs] + sC;
        if( has_overflow(reg[rt], reg[rs], sC) )
             print_error(NUMBER_OVERFLOW, cycle);
        if(rt==0) reg[rt] = 0;

        puts("addi"); break;

    case LW:
        check_errors(rt, addr, reg[rs], sC, LW);
        if(error_halt) return;
        reg[rt] = dmem[addr/4];
        if(rt==0) reg[rt]=0;

        puts("lw"); break;
    case LH: //signed
        check_errors(rt, addr, reg[rs], sC, LH);
        if(error_halt) return;
        tmp = dmem[addr/4];
        reg[rt] = addr%4==0 ? (tmp<<16)>>16 : tmp>>16;
        if(rt==0) reg[rt]=0;

        puts("lh"); break;
    case LHU:
        check_errors(rt, addr, reg[rs], sC, LHU);
        if(error_halt) return;
        tmpu = dmem[addr/4];
        reg[rt] = addr%4==0 ? (tmpu<<16)>>16 : tmpu>>16;
        if(rt==0) reg[rt]=0;
        puts("lhu"); break;
    case LB:
        check_errors(rt, addr, reg[rs], sC, LB);
        tmp = dmem[addr/4];
        shift = 24 - (addr%4)*8;
        reg[rt] = (tmp << shift) >> 24;
        if(rt==0) reg[rt]=0;

        puts("lb"); break;
    case LBU:
        check_errors(rt, addr, reg[rs], sC, LBU);
        tmpu = dmem[addr/4];
        shift = 24 - (addr%4)*8;
        reg[rt] = (tmpu << shift) >> 24;
        if(rt==0) reg[rt]=0;

        puts("lbu"); break;
    case SW:
        check_errors(-1, addr, reg[rs], sC, SW);//SW不會有write to zero reg的問題所以rt設-1
        if(error_halt) return;
        dmem[addr/4] = reg[rt];

        puts("sw"); break;
    case SH:
        check_errors(-1, addr, reg[rs], sC, SH);
        if(error_halt) return;
        tmpu = dmem[addr/4];
        save = reg[rt] & 0x0000ffff;
        if(addr%4==0){
            dmem[addr/4] = ((tmpu>>16)<<16) + save;
        }else{
            dmem[addr/4] = (tmpu&0x0000ffff) + (save<<16);
        }

        puts("sh"); break;
    case SB:
        check_errors(-1, addr, reg[rs], sC, SB);
        if(error_halt) return;
        tmpu = dmem[addr/4];
        save = reg[rt] & 0x000000ff;
        shift = (addr%4)*8;
        dmem[addr/4] = (tmpu & masks[addr%4]) + (save<<shift);

        puts("sb"); break;

    case LUI:
        if(rt==0){
            print_error(WRITE_ZERO, cycle);
            return;
        }
        reg[rt] = uC << 16;
        puts("lui"); break;
    case ANDI:
        if(rt==0){
            print_error(WRITE_ZERO, cycle);
            return;
        }
        reg[rt] = reg[rs] & uC;
        puts("andi"); break;
    case ORI:
        if(rt==0){
            print_error(WRITE_ZERO, cycle);
            return;
        }
        reg[rt] = reg[rs] | uC;
        puts("ori"); break;
    case NORI:
        if(rt==0){
            print_error(WRITE_ZERO, cycle);
            return;
        }
        reg[rt] = ~(reg[rs] | uC);
        puts("nori"); break;
    case SLTI:
        if(rt==0){
            print_error(WRITE_ZERO, cycle);
            return;
        }
        reg[rt] = ( (int)reg[rs] < sC );
        puts("slti"); break;

    case BEQ:
        if(reg[rs]==reg[rt]){
            if( has_overflow( PC+4+sC*4, PC+4, sC*4) )
                print_error(NUMBER_OVERFLOW, cycle);
            PC = (PC+4) + sC*4;
            pcChanged = true;
        }

        puts("beq");
        break;
    case BNE:
        if(reg[rs]!=reg[rt]){
            if( has_overflow( PC+4+sC*4, PC+4, sC*4) )
                print_error(NUMBER_OVERFLOW, cycle);
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
    for(int i= dSize; i<256; i++)
        dmem[i] = ZERO;

    snapshot = fopen("snapshot.rpt", "w");
    error_dump = fopen("error_dump.rpt", "w");
}
bool load_image(void)
{
    FILE *fi = fopen("open_testcase\\error2\\iimage.bin", "rb");
    FILE *fd = fopen("open_testcase\\error2\\dimage.bin", "rb");
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
