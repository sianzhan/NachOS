#include "syscall.h"
main()
	{
		int	n;
		int arr[500];
		for(n = 0; n < 500; n++){
			arr[n] = n + 1;
			PrintInt(arr[n]);
			// PrintInt(n);
		}
	}
