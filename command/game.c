#include <stdio.h>
#define chartonumber(x) (x-'0')
int main(int argc, char * argv[]){
	while(1)
	{
	printf("              ===================================================\n");
	printf("              =                     Game                        =\n");
	printf("              =-------------------------------------------------=\n");
	printf("              =   Please choose the game:                       =\n");
	printf("              =   1. Guess number                               =\n");
	printf("              =   2. Tic-Tac-Toe                                =\n");
	printf("              =   3. N queen game                               =\n");
	printf("              =   Enter e to quit                               =\n");
	printf("              ===================================================\n\n");
	
	char buffer[128];
	read(0, buffer, 128);
	if('1' == buffer[0])
		guess();
	if('2' == buffer[0])
		Tic();
	if('3' == buffer[0])
		queen();
	if('e' == buffer[0])
		break;
	printf("\n\n\n\n\n");
	}
	return 0;
}



/***************************Guess number********************************/
int my_atoi(const char *s)
{
	int num, i;
	char ch;
	num = 0;
	for (i = 0; i < 3; i++)
	{
		ch = s[i];
		if (ch < '0' || ch > '9')
			break;
		num = num*10 + (ch - '0');
	}
	return num;
}
void guess()
{
	printf("              ===================================================\n");
	printf("              =               Guess number                      =\n");
	printf("              =-------------------------------------------------=\n");
	printf("              =    number:1~999                                 =\n");
	printf("              =    Enter e to quit                              =\n");
	printf("              ===================================================\n\n");
	int stop = 0;
	int a,b;
	char c;
	a = 489;
	printf("There is a number between 1 and 999.\nPlease enter your first guess.\n");
	char buffer[128];
	read(0, buffer, 128);
	b=my_atoi(buffer);
	while (b!= -1)
	{
		if (b == a)
		{
			printf("Excellent! You guessed the number!\n Would you like to play again(y or n)?");
			char buffer[128];
			read(0, buffer, 128);
			switch (buffer[0]) {
			case 'y':
				printf("There is a number between 1 and 999.\nPlease enter your first guess.\n");
				read(0, buffer, 128);
				break;
			case 'n':
				stop = 1;
				break;
			}
			if (stop == 1)
				break;
		}
		while (b<a)
		{
			printf("Too low.Try again.\n");
			read(0, buffer, 128);
			b=my_atoi(buffer);
		}
		while (b>a)
		{
			printf("Too high.Try again.\n");
			read(0, buffer, 128);
			b=my_atoi(buffer);
		}
		if(buffer[0]=='e')
			break;
	}
}



/****************************N queen game*******************************/
void huisu(int l);

int jc(int l, int i);
int n, h[100];
int x;

void queen()
{
		printf("              ===================================================\n");
		printf("              =                   N Queen game                  =\n");
		printf("              =-------------------------------------------------=\n");
		printf("              =                                                 =\n");
		printf("              =                 Enter e to quit                 =\n");
		printf("              =                                                 =\n");
		printf("              ===================================================\n\n");

		printf("N=");
		char bur[128];
		read(0, bur, 128);
		n = chartonumber(bur[0]);
		x = 0;
		huisu(1);
		printf("There are %d stacking methods\n", x);
}

void huisu(int l)
{
	int i, j;
	if (l == n + 1)
	{
		x = x + 1;
		printf("stacking methods are:\n", x);
		for (i = 1; i <= n; i++)
			printf("%d", h[i]);
		printf("\n");
	}

	for (i = 1; i <= n; i++)
	{
		h[l] = i;
		if (jc(l, i) != 1)
			huisu(l + 1);
	}
}

int jc(int l, int i)
{
	int k;
	for (k = 1; k<l; k++)
		if ((l - k == h[k] - i) || i == h[k])
			return 1;
	return 0;
}



/***************************Tic-Tac-Toe*********************************/
typedef char chess[10];
typedef int temparr[10];
chess arr;
temparr brr;
int number, suc, n3, c3, n2, c2, n1, c1;
char ch;
void inarrdata(chess a)
{
	a[1] = '1'; a[2] = '2'; a[3] = '3';
	a[4] = '4'; a[5] = '5'; a[6] = '6';
	a[7] = '7'; a[8] = '8'; a[9] = '9';
}
void display(chess a)
{
	printf("              \n"); printf("\n");
	printf("               %c | %c | %c\n", a[1], a[2], a[3]);
	printf("               ---------\n");
	printf("               %c | %c | %c\n", a[4], a[5], a[6]);
	printf("               ---------\n");
	printf("               %c | %c | %c\n", a[7], a[8], a[9]);
	printf("              \n"); printf("\n");
}
int arrfull()
{
	int i;
	int arrf = 0;
	for (i = 1; i <= 9; i++)
		if (i == arr[i] - 48)
			arrf = 1;
	return arrf;
}
void cn(int line)
{
	switch (line)
	{
	case 0:c3 = c3 + 1; break;
	case 1:n2 = n2 + 1; break;
	case 2:c2 = c2 + 1; break;
	case 3:n1 = n1 + 1; break;
	case 4:c1 = c1 + 1; break;
	case 5:n3 = n3 + 1; break;
	}
}
int linenum(char a, char b, char c)
{
	int ln = 6;
	if ((a == 'X') && (b == 'X') && (c == 'X'))
		ln = 0;
	if (((a == 'O') && (b == 'O') && (c != 'O')) || ((a == 'O') && (b != 'O') && (c == 'O')) || ((a != 'O') && (b == 'O') && (c == 'O')))
		ln = 1;
	if (((a == 'X') && (b == 'X') && (c != 'X')) || ((a == 'X') && (b != 'X') && (c == 'X')) || ((a != 'X') && (b == 'X') && (c == 'X')))
		ln = 2;
	if (((a == 'O') && (b != 'O') && (c != 'O')) || ((a != 'O') && (b == 'O') && (c != 'O')) || ((a != 'O') && (b != 'O') && (c == 'O')))
		ln = 3;
	if (((a == 'X') && (b != 'X') && (c != 'x')) || ((a != 'X') && (b == 'X') && (c != 'X')) || ((a != 'X') && (b != 'X') && (c == 'X')))
		ln = 4;
	if ((a == 'O') && (b == 'O') && (c == 'O'))
		ln = 5;
	return ln;
}
int maxbrr(int *br)
{
	int temp, i, mb;
	temp = -888;
	for (i = 1; i <= 9; i++)
	{
		if (temp <= br[i])
		{
			temp = br[i];
			mb = i;
		}
	}
	return mb;
}
void manstep()       //人走棋处理模块
{
	int j;
	display(arr);
	if (arrfull())   //如果棋盘上还有下棋的位置，给人走一步棋
	{
		printf("Which step are you going to take? please enter a number(1--9):");
		char bur[128];
		read(0, bur, 128);
		j = chartonumber(bur[0]);

		arr[j] = 'O';
		c3 = 0; n2 = 0; c2 = 0; n1 = 0; c1 = 0;
		number = linenum(arr[1], arr[2], arr[3]); cn(number);
		number = linenum(arr[4], arr[5], arr[6]); cn(number);
		number = linenum(arr[7], arr[8], arr[9]); cn(number);
		number = linenum(arr[1], arr[4], arr[7]); cn(number);
		number = linenum(arr[2], arr[5], arr[8]); cn(number);
		number = linenum(arr[3], arr[6], arr[9]); cn(number);
		number = linenum(arr[1], arr[5], arr[9]); cn(number);
		number = linenum(arr[3], arr[5], arr[7]); cn(number);
		if (n3 == 0)     //你赢了
		{
			display(arr);
			printf("\n");
			printf("You win!!!\n");
		
			suc = 0;
		}
	}
}
void computerstep()       //计算机走棋处理模块
{
	int i;
	if (arrfull())        //如果棋盘上还有可下棋的位置，则计算机走棋
	{
		for (i = 1; i <= 9; i++) //对每一步可走的棋进行计算
		{
			if (i == arr[i] - 48)
			{
				c3 = 0; n2 = 0; c2 = 0; n1 = 0; c1 = 0;
				arr[i] = 'X';
				number = linenum(arr[1], arr[2], arr[3]); cn(number);
				number = linenum(arr[4], arr[5], arr[6]); cn(number);
				number = linenum(arr[7], arr[8], arr[9]); cn(number);
				number = linenum(arr[1], arr[4], arr[7]); cn(number);
				number = linenum(arr[2], arr[5], arr[8]); cn(number);
				number = linenum(arr[3], arr[6], arr[9]); cn(number);
				number = linenum(arr[1], arr[5], arr[9]); cn(number);
				number = linenum(arr[3], arr[5], arr[7]); cn(number);
				brr[i] = (128 * c3 - 63 * n2 + 31 * c2 - 15 * n1 + 7 * c1); //计算此步权值
				arr[i] = i + 48;
			}
			else
				brr[i] = -999;
		}
		arr[maxbrr(brr)] = 'X';      //确定计算机走哪一步，权值最大的一步
		c3 = 0; n2 = 0; c2 = 0; n1 = 0; c1 = 0;
		number = linenum(arr[1], arr[2], arr[3]); cn(number);
		number = linenum(arr[4], arr[5], arr[6]); cn(number);
		number = linenum(arr[7], arr[8], arr[9]); cn(number);
		number = linenum(arr[1], arr[4], arr[7]); cn(number);
		number = linenum(arr[2], arr[5], arr[8]); cn(number);
		number = linenum(arr[3], arr[6], arr[9]); cn(number);
		number = linenum(arr[1], arr[5], arr[9]); cn(number);
		number = linenum(arr[3], arr[5], arr[7]); cn(number);
		if (c3 != 0)        //计算机已赢
		{
			display(arr);
			printf("\n");
			printf("PC win!!!\n");

			suc = 0;
		}
	}
	else
		suc = 0;

}
void Tic()
{

	printf("              ===================================================\n");
	printf("              =                   Tic-Tac-Toe                   =\n");
	printf("              =-------------------------------------------------=\n");
	printf("              =                                                 =\n");
	printf("              =                 Enter e to quit                 =\n");
	printf("              =                                                 =\n");
	printf("              ===================================================\n\n");

	inarrdata(arr);       //棋盘坐标编号
	display(arr);         //显示初始棋盘
	suc = 1;
	printf("Do you want to go first?(y/n)");
	char bufr[128];
	read(0, bufr, 128);
	if ((bufr[0] == 'y') || (bufr[0] == 'Y')) //输入Y，表示人先走棋
	{
		while (suc)
		{
			manstep();
			computerstep();
		}
		display(arr);
	}
	else //计算机先走棋
	{
		while (suc)
		{
			computerstep();
			if (suc)
				manstep();
		}
	}
	if(!arrfull())
		printf("\nNo winer !\n");
}