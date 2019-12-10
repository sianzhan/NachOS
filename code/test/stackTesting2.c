#include "syscall.h"
main()
	{
		int arr[20];
		int i;
		for(i =0; i < 20; i++){
			arr[i] = 200 + i;
			PrintInt2(arr[i]);
		}
	}
