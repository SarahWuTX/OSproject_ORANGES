
#include<stdio.h>
 
#define bool int//自定义bool类型
#define true 1
#define false 0
 
bool IsLeap(int year);
int GetWeek(int year,int month);
void main()
{

    printf("***************************************************\n");
	printf("*                  Calendar                       *\n");
	printf("***************************************************\n");
	printf("*  1. Enter the year and month                    *\n");
	printf("*  2. Example: 2018 08                            *\n");
	printf("*  3. Enter e to quit                             *\n");
	printf("***************************************************\n\n");

    while(1)
    {
        char bufr1[128];
        char bufr2[128];
        int y,m,week,i;
        printf("Please enter year: yyyy\n");
        i = read(0, bufr1, 128);
        if (bufr1[0] == 'e')
			break;
        y = getNum(bufr1);

        printf("Please enter month: mm\n");
        i = read(0, bufr2, 128);
        if (bufr2[0] == 'e')
			break;
        m = getNum(bufr2);

        printf("       %d %d   \n",y,m);
        printf("=====================\n");
        printf("  M  T  W  T  F  S  S\n");
        week = GetWeek(y,m);
        if(week == 0)
            week = 7;									//若week == 0，将其视作7，以便留下足够的空格
    
        for (i = 1;i < week; i++)
        {
            printf("   ");								//三个字符为一个单位，保证第一行与周数对应
        }
        int month[12] = {31,28,31,30,31,30,31,31,30,31,30,31};//以数组方式统计每个月的天数
        if (IsLeap(y) == true)							//若为闰年，将2月份的天数修改为29
            month[1] = 29;
        for(i=1;i<=month[m-1];i++)						//利用for循环，依次输出日数，并且若加上通过计算week+i-1%7来判断是否该换行
        {
            printf("%3d",i);
            if((i+week-1) % 7 == 0)						
                printf("\n");
        }
        printf("\n=====================\n");

        memset(bufr1,0,100);
        memset(bufr2,0,100);
    }
    
}
 
bool IsLeap(int year)
{
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))//判断是否为闰年的条件
        return true;
    else
        return false;
}
 
int GetWeek(int year,int month)
{
    int m[12] = {31,28,31,30,31,30,31,31,30,31,30,31};			//以数组记录每个月的天数
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

int getNum(char * bufr)
{
	int ten = 1, i = 0, res = 0;
	for (i = 0; i < strlen(bufr) - 1; i++)
	{
		ten *= 10;
	}
	for (i = 0; i < strlen(bufr); i++)
	{
		res += (bufr[i] - '0') * ten;
		ten /= 10;
	}
	return res;
}