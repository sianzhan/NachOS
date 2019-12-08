#include "syscall.h"
main()
	{
		int	n;
		int arr[347];
		arr[0] = 321;
		arr[345] = 123;
		PrintInt(arr[0]);		
		PrintInt(arr[345]);
		// arr[2] = 1;
		// arr[3] = 2;
		// arr[4] = 123;
		// for(n = 0; n < 500; n++){
		// 	arr[n] = n + 1;
		// 	PrintInt(arr[n]);
		// 	// PrintInt(n);
		// }
	}
