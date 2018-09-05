



#Operating System

###系统开发环境
Ubuntu-16.04 32位
bochs-2.6.9

###操作系统组成
boot 引导
command 应用集
fs 文件系统
include 头文件集
kernel 内核
lib 可用代码库
mm 内存调度系统

##项目设计说明
###功能分析

+ 文件系统

  Orange源码中的文件系统只是一个简单的一级文件系统。在对源码进行学习分析之后，针对于它的缺陷，我们选择将文件系统进行完善，

- 控制台

- 应用程序




###功能汇总

| 命令 | 参数 | 概述 |   |
|:---:|:----:|:---:| :---: |
| welcome | - | 打印欢迎语句 |  |
| clear | - | 清屏并打印欢迎语句 |  |
| help | - | 展示当前所有可用指令 |  |
| mkdir | name | 创建文件目录 |  |
| mkfile | filename content | 创建新文件 |  |
| read | filename | 读取文件 |  |
| rm | filename | 删除文件 |  |
| edit -ad| filename | 编辑文件，追加文件内容 |  |
| edit -rw| filename | 编辑文件，覆盖文件内容 |  |
| edit -tr| filename |  |  |
| calculator | - | 打开应用程序 计算器 |  |
| calendar | - | 打开应用程序 日历 |  |
| game | - | 打开应用程序 游戏|  |



##操作系统基本功能介绍
###基本界面
- Welcome：在控制台输入welcome，展示欢迎界面 

  ![](pics/welcome.bmp)


+ Clear:在控制台输入clear，清空控制台内容

  ![](pics/clear.bmp)


- Help：在控制台输入help，展示当前所有可用指令

  ![](pics/help.bmp)


###文件管理
Mkdir:
Mkfile
Read
Rm
Edit

###应用程序
- Calculator: 在控制台输入calculator，进入应用程序“计算器”界面

  该计算器可以实现由两位正整数参与的简单的加减乘除四则运算。

  ![](pics/calculator.bmp)


- Calendar：在控制台输入calendar，进入应用程序“日历”界面

  日历可以展示由用户输入的年份月份确定的当月日历

  ![](pics/calendar.bmp)


- Game：在控制台输入game，进入应用程序“游戏”界面

  在游戏界面中， 用户可以选择三种不同的简单小游戏进行玩耍。

  ![](pics/game.bmp)



  + 猜数字

    ![](pics/guess.bmp)

  + 井字棋

    ![](pics/tic1.bmp)

    ![](pics/tic2.bmp)

  + N皇后游戏

    ![](pics/queen1.bmp)![](pics/queen2.bmp)



