#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "instruction.h"
#include "parameter.h"
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
unsigned int convert(unsigned char*);
int main()
{
    if(!load_image()){
        puts("Cannot load the images");
        return 0;
    }
    init();

    cycle = 0;
    error_halt = false;
    print_cycle();
    int i = PC/4;
    while( (imem[i]>>26) != HALT && i < MEM_SIZE/4){
       // printf("0x%08x\n",imem[i]);
        int opcode = imem[i] >> 26;
        pcChanged = false;
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
        //printf("cycle %d  %08x  0x%02x \n",cycle, imem[i], opcode);

        //system("pause");

        i = PC/4;
       // printf("i: %d\n",i);
        //system("pause");
    }


    fclose(snapshot);
    fclose(error_dump);

    return 0;
}
void init(void)
{
    for(int i=0; i<32; i++)
        reg[i] = ZERO;
    reg[29] = sp;

    snapshot = fopen("snapshot.rpt", "w");
    error_dump = fopen("error_dump.rpt", "w");
}
void print_cycle(void)
{
    fprintf(snapshot, "cycle %d\n",cycle);
    for(int i=0; i<32; i++)
        fprintf(snapshot, "$%02d: 0x%08X\n",i,reg[i]);
    fprintf(snapshot,"PC: 0x%08X\n\n\n",PC);
}
unsigned int convert(unsigned char bytes[])
{
    return bytes[3] | (bytes[2]<<8) | (bytes[1]<<16) | (bytes[0]<<24);
}
bool load_image(void)
{
    FILE *fi = fopen("iimage.bin", "rb");
    FILE *fd = fopen("dimage.bin", "rb");
    if(!fi || !fd) return false;

    for(int i = 0; i<MEM_SIZE/4; i++)
        imem[i] = dmem[i] = ZERO;

    unsigned char bytes[4];
    int i = 0;
    while( fread(bytes, 4, 1, fi) != 0 ){
        if(i==0){
            PC = convert(bytes);
        }
        else if(i==1){
            iSize = convert(bytes);
        }
        else{
            if(i-2 >= iSize) break;
            imem[i-2 + PC/4] = convert(bytes);
           // printf("%08x \n", imem[i-2]);
        }

        i++;
    }
   // puts("");

    i = 0;
    while( fread(bytes, 4, 1, fd) != 0 ){
        if(i==0){
            sp = convert(bytes);
        }
        else if(i==1){
            dSize = convert(bytes);
        }
        else{
            if(i-2 >= dSize) break;
            dmem[i-2] = convert(bytes);
            //printf("%08x \n", dmem[i-2]);
        }
        i++;
    }
    fclose(fi);
    fclose(fd);
    return true;
}
