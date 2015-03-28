#include <stdio.h>
#include <stdlib.h>
int main()
{
    int a;
    unsigned int n = 0xff00ffff;
    unsigned int C = 4;
    unsigned int result = (int(n<<16))>>16;
    unsigned int tmp = 0xffff0000;
    result = (tmp)>>16;
    printf("%08x\n",result);


    printf("%d\n",n>1);
    printf("%d\n",(int)n > 1);


    return 0;
}
