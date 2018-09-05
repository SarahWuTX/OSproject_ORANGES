#include <stdio.h>

int main(int argc, char * argv[])
{
	int i, num1 = 0, num2 = 0, flag = 1, res = 0;
    char buffer[128];
	char buffer1[128];
    char buffer2[128];

	printf("                ===================================================\n");
	printf("                =                  Calculator                     =\n");
	printf("                =-------------------------------------------------=\n");
	printf("                =  Please enter two positive integers             =\n");
	printf("                =  Example: 10+2                                  =\n");
	printf("                =  Enter e to quit                                =\n");
	printf("                ===================================================\n");

	while(flag == 1){	
		printf("\n");
		printf("Please input num1:");
		i = read(0, buffer1, 128);
		if (buffer1[0] == 'e')
			break;
		num1 = getNum(buffer1);
		printf("num1: %d\n", num1);

		printf("Please input num2:");
		i = read(0, buffer2, 128);	
		if (buffer2[0] == 'e')
			break;
		num2 = getNum(buffer2);
		printf("num2: %d\n", num2);

		printf("Please input op( + - * / ):");
		i = read(0, buffer, 1);
		switch(buffer[0])
		{
			case '+':
				res = num1 + num2;
				printf("%d + %d = %d\n", num1, num2, res);
				break;
			case '-':
				res = num1 - num2;
				printf("%d - %d = %d\n", num1, num2, res);
				break;
			case '*':
				res = num1 * num2;
                printf("%d * %d = %d\n", num1, num2, res);
				break;
			case '/':
				if(num2 == 0)
				{
					printf("Num2 = Zero!\n");
					break;
				}
				res = num1 / num2;
				printf("%d / %d = %d\n", num1, num2, res);
				break;
			case 'e':
				flag = 0;
				break;
			default:
				printf("No such command!\n");
		}
        memset(buffer,0,100);
        memset(buffer,0,100);
        memset(buffer,0,100);
	}
	return 0;
}

int getNum(char * buffer)
{
	int ten = 1, i = 0, res = 0;
	for (i = 0; i < strlen(buffer) - 1; i++)
	{
		ten *= 10;
	}
	for (i = 0; i < strlen(buffer); i++)
	{
		res += (buffer[i] - '0') * ten;
		ten /= 10;
	}
	return res;
}