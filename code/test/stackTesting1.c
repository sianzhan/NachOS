#include "syscall.h"
main()
	{
		int arr[20];
		int i;
		for(i =0; i < 20; i++){
			arr[i] = i + 100;
			PrintInt(arr[i]);
		}
	}