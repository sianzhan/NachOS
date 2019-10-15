#include "syscall.h"
int main()
{
	int n;
	for(n=0; n<4; n++){
		Sleep(1000000);
		PrintInt2(n);
	}
}