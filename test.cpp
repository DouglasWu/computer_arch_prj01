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
    int a1 = -123456;
    int a2 = 2147483647;
    int b = a1-a2;
    printf("%08x\n",-a2>>31);
    if( (a1>>31)==( -a2>>31) && (a1>>31)!=(b>>31) )
        puts("number overflow");
    else
        puts("not overflow");
    unsigned int num = 5;
    int m = -num;
    printf("%d\n",m);


   /* printf("%08x\n",result);


    printf("%d\n",n>1);
    printf("%d\n",(int)n > 1);*/


    return 0;
}
