#include<stdio.h>
 
#define bool int      //自定义bool类型
#define true 1
#define false 0
 
bool IsLeap(int year);
int GetWeek(int year,int month);
void main()
{

    printf("              ===================================================\n");
	printf("              =                  Calendar                       =\n");
	printf("              =-------------------------------------------------=\n");
	printf("              =  Please enter the year and month                =\n");
	printf("              =  Example: 2018 08                               =\n");
	printf("              =  Enter e to quit                                =\n");
	printf("              ===================================================\n\n");

    while(1)
    {
        char buffer1[128];
        char buffer2[128];
        int y,m,week,i;
        printf("Please enter year: yyyy\n");
        i = read(0, buffer1, 128);
        if (buffer1[0] == 'e')
			break;
        y = getNum(buffer1);

        printf("Please enter month: mm\n");
        i = read(0, buffer2, 128);
        if (buffer2[0] == 'e')
			break;
        m = getNum(buffer2);

        printf("       %d %d   \n",y,m);
        printf("=====================\n");
        printf("  M  T  W  T  F  S  S\n");
        week = GetWeek(y,m);
        if(week == 0)
            week = 7;
    
        for (i = 1;i < week; i++)
        {
            printf("   ");								//保持对齐
        }
        int month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
        if (IsLeap(y) == true)							//若为闰年，将2月份的天数修改为29
            month[1] = 29;
        for(i=1;i<=month[m-1];i++)						//输出日数
        {
            printf("%3d",i);
            if((i+week-1) % 7 == 0)						//判断换行
                printf("\n");
        }
        printf("\n=====================\n");

        memset(buffer1,0,100);
        memset(buffer2,0,100);
    }
    
}
 
bool IsLeap(int year)
{
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))   //判断是否为闰年
        return true;
    else
        return false;
}
 
int GetWeek(int year,int month)
{
    int m[12] = {31,28,31,30,31,30,31,31,30,31,30,31};			  //记录每个月的天数
    int day=0,week,i;
    switch (month)
    {
        case 1:
            day = 1;
            break;
        case 2:
            day = m[0] + 1;
            break;
        case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
            if (IsLeap(year) == true)
                m[1] = 29;
            for (i=0;i<month-1;i++)
            {
                day = day + m[i];
            }
            day = day + 1;
            break;
 
    }
    week = ((year-1) + (year-1)/4 - (year-1)/100 + (year-1)/400 + day) % 7;
    return week;
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