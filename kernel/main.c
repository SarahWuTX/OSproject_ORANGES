
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

#define	MAX_USER		2
#define	MAX_USER_FILE	100
#define MAX_USER_DIR	16
#define	MAX_PSWD_LEN	12

#define	MAX_FILES		80
#define	MAX_DIRS		32


char location[MAX_FILENAME_LEN] = "root";
char files[MAX_FILES][MAX_FILENAME_LEN];
int  filequeue[MAX_FILES];
int  filecount = 0;
char dirs[MAX_DIRS][MAX_FILENAME_LEN];
int  dirqueue[MAX_FILES];
int  dircount = 0;

void shabby_shell(const char * tty_name);
/*****************************************/
int checkFilename(const char * arg1);
int updateDir(char * filename, int op);
void getFullname(char * fullname, char * filename);

int miniRead(char * fullname, char * buf);
int miniWrite(char * fullname, char * buf);
void truncateFile(char * filename);
void extendFile(char * filename);
void rewriteFile(char * filename);

int dirmap[MAX_DIRS];
char dir[MAX_DIRS][MAX_FILENAME_LEN];
/*****************************************/
int isDir(const char * filepath);

void getFilepath(char *filepath, char * filename);
void getDirFilepath(char *filepath, char * filename);
void getDirpathAndFilename(char * dirpath, char * filename, char * filepath);

int getFreeFilePos();
int getFreeDirPos();
int getPosInDirQueue(char * filepath);


void addFileIntoDir(const char * dirpath, char * filename);
void deleteFileFromDir(const char * dirpath, char * filename);

void initFS();
void welcome();
void clear();
void showProcess();
void killProcess();
void makeProcess();
void help();
void createFile(char *filename, char * buf);
void createDir(char *filename);
void readFile(char * filename);
void editAppand(const char * filepath, char * str);
void editCover(const char * filepath, char * str);
void deleteFile(char * filepath);
void deleteDir(char * filepath);
void ls();
void cd(char * arg1);



/*****************************************************************************
*                               kernel_main
*****************************************************************************/
/**
* jmp from kernel.asm::_start.
*
*****************************************************************************/
PUBLIC int kernel_main()

{

	disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
		"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

	int i, j, eflags, prio;
	u8  rpl;
	u8  priv; /* privilege */

	struct task * t;
	struct proc * p = proc_table;

	char * stk = task_stack + STACK_SIZE_TOTAL;

	for (i = 0; i < NR_TASKS + NR_PROCS; i++, p++, t++) {
		if (i >= NR_TASKS + NR_NATIVE_PROCS) {
			p->p_flags = FREE_SLOT;
			continue;
		}

		if (i < NR_TASKS) {     /* TASK */
			t = task_table + i;
			priv = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202;/* IF=1, IOPL=1, bit 2 is always 1 */
			prio = 15;
		}

		else {                  /* USER PROC */
			t = user_proc_table + (i - NR_TASKS);
			priv = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202;	/* IF=1, bit 2 is always 1 */
			prio = 5;
		}

		strcpy(p->name, t->name);	/* name of the process */
		p->p_parent = NO_TASK;

		if (strcmp(t->name, "INIT") != 0) {
			p->ldts[INDEX_LDT_C] = gdt[SELECTOR_KERNEL_CS >> 3];
			p->ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DS >> 3];

			/* change the DPLs */
			p->ldts[INDEX_LDT_C].attr1 = DA_C | priv << 5;
			p->ldts[INDEX_LDT_RW].attr1 = DA_DRW | priv << 5;
		}

		else {		/* INIT process */
			unsigned int k_base;
			unsigned int k_limit;
			int ret = get_kernel_map(&k_base, &k_limit);
			assert(ret == 0);
			init_desc(&p->ldts[INDEX_LDT_C],
				0, /* bytes before the entry point
				   * are useless (wasted) for the
				   * INIT process, doesn't matter
				   */
				(k_base + k_limit) >> LIMIT_4K_SHIFT,
				DA_32 | DA_LIMIT_4K | DA_C | priv << 5);

			init_desc(&p->ldts[INDEX_LDT_RW],
				0, /* bytes before the entry point
				   * are useless (wasted) for the
				   * INIT process, doesn't matter
				   */

				(k_base + k_limit) >> LIMIT_4K_SHIFT,
				DA_32 | DA_LIMIT_4K | DA_DRW | priv << 5);
		}

		p->regs.cs = INDEX_LDT_C << 3 | SA_TIL | rpl;
		p->regs.ds =
			p->regs.es =
			p->regs.fs =
			p->regs.ss = INDEX_LDT_RW << 3 | SA_TIL | rpl;

		p->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p->regs.eip = (u32)t->initial_eip;
		p->regs.esp = (u32)stk;
		p->regs.eflags = eflags;

		p->ticks = p->priority = prio;
		p->p_flags = 0;
		p->p_msg = 0;
		p->p_recvfrom = NO_TASK;
		p->p_sendto = NO_TASK;
		p->has_int_msg = 0;
		p->q_sending = 0;
		p->next_sending = 0;

		for (j = 0; j < NR_FILES; j++)
			p->filp[j] = 0;

		stk -= t->stacksize;
	}

	k_reenter = 0;
	ticks = 0;

	p_proc_ready = proc_table;

	init_clock();
	init_keyboard();

	restart();

	while (1) {}
}


/*****************************************************************************
*                                get_ticks
*****************************************************************************/
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}


/**
* @struct posix_tar_header
* Borrowed from GNU `tar'
*/
struct posix_tar_header
{				/* byte offset */
	char name[100];		/*   0 */
	char mode[8];		/* 100 */
	char uid[8];		/* 108 */
	char gid[8];		/* 116 */
	char size[12];		/* 124 */
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100];	/* 157 */
	char magic[6];		/* 257 */
	char version[2];	/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];	/* 329 */
	char devminor[8];	/* 337 */
	char prefix[155];	/* 345 */
						/* 500 */
};

/*****************************************************************************
*                                untar
*****************************************************************************/
/**
* Extract the tar file and store them.
*
* @param filename The tar file.
*****************************************************************************/
void untar(const char * filename)
{
	printf("[extract `%s'", filename);
	int fd = open(filename, O_RDWR);
	assert(fd != -1);

	char buf[SECTOR_SIZE * 16];
	int chunk = sizeof(buf);

	while (1) {
		read(fd, buf, SECTOR_SIZE);
		if (buf[0] == 0)
			break;

		struct posix_tar_header * phdr = (struct posix_tar_header *)buf;

		/* calculate the file size */
		char * p = phdr->size;
		int f_len = 0;
		while (*p)
			f_len = (f_len * 8) + (*p++ - '0'); /* octal */

		int bytes_left = f_len;
		int fdout = open(phdr->name, O_CREAT | O_RDWR);
		if (fdout == -1) {
			printf("    failed to extract file: %s\n", phdr->name);
			printf(" aborted]");
			return;
		}
		printf("    %s (%d bytes)", phdr->name, f_len);
		while (bytes_left) {
			int iobytes = min(chunk, bytes_left);
			read(fd, buf,
				((iobytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
			write(fdout, buf, iobytes);
			bytes_left -= iobytes;
		}
		close(fdout);
	}

	close(fd);

	printf(" done]\n");
}


/*****************************************************************************
*                                Init
*****************************************************************************/
/**
* The hen.
*
*****************************************************************************/
void Init()
{
	int fd_stdin = open("/dev_tty0", O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open("/dev_tty0", O_RDWR);
	assert(fd_stdout == 1);

	//printf("Init() is running ...\n");

	/* extract `cmd.tar' */
	untar("/cmd.tar");
	//spin("a");

	char * tty_list[] = { "/dev_tty0" };

	int i;
	for (i = 0; i < sizeof(tty_list) / sizeof(tty_list[0]); i++) {
		int pid = fork();
		if (pid != 0) { /* parent process */
		}
		else {	/* child process */
			close(fd_stdin);
			close(fd_stdout);

			shabby_shell(tty_list[i]);
			assert(0);
		}
	}

	while (1) {
		int s;
		int child = wait(&s);
		printf("child (%d) exited with status: %d.\n", child, s);
	}

	assert(0);
}


/*======================================================================*
				TestA
*======================================================================*/
void TestA()
{
	for (;;);
}

/*======================================================================*
				TestB
*======================================================================*/
void TestB()
{
	for (;;);
}

/*======================================================================*
				TestC
*======================================================================*/
void TestC()
{
	for (;;);
}

/*****************************************************************************
*                                panic
*****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}

/*****************************************************************************
*                                shabby_shell
*****************************************************************************/
/**
* A very simple shell.
*
* @param tty_name  TTY file name.
*****************************************************************************/
void shabby_shell(const char * tty_name)
{
	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char rdbuf[256];
	char cmd[64];
	char arg1[MAX_FILENAME_LEN];
	char arg2[MAX_FILENAME_LEN];
	char filepath[MAX_FILENAME_LEN];
	char username[MAX_FILENAME_LEN]="User";
	char locat[MAX_FILENAME_LEN]="root";

	clear();
	welcome();

	initFS();

	while (1) {

		memset(rdbuf, 0, 256);
		memset(cmd, 0, 64);
		memset(arg1, 0, MAX_FILENAME_LEN);
		memset(arg2, 0, MAX_FILENAME_LEN);

		printf("%s@Bochs: %s$ ", username, location);
		int r = read(0, rdbuf, 256);
		rdbuf[r] = 0;


		int argc = 0;
		char * argv[PROC_ORIGIN_STACK];
		char * p = rdbuf;
		char * s;
		int word = 0;
		char ch;
		do {
			ch = *p;
			if (*p != ' ' && *p != 0 && !word) {
				s = p;
				word = 1;
			}
			if ((*p == ' ' || *p == 0) && word) {
				word = 0;
				argv[argc++] = s;
				*p = 0;
			}
			p++;
		} while (ch);
		argv[argc] = 0;
		int fd=-1;
		if(strcmp(argv[0],"game")== 0 || strcmp(argv[0],"calculator")== 0 || strcmp(argv[0],"calendar")== 0)
		{
			fd = open(argv[0], O_RDWR);
		}
		if (fd == -1) {
			if (rdbuf[0]) {
				int i = 0, j = 0;
				/* get command */
				while (rdbuf[i] != ' ' && rdbuf[i] != 0)
				{
					cmd[i] = rdbuf[i];
					i++;
				}
				i++;
				/* get arg1 */
				while (rdbuf[i] != ' ' && rdbuf[i] != 0)
				{
					arg1[j] = rdbuf[i];
					i++;
					j++;
				}
				i++;
				j = 0;
				/* get arg2 */
				while (rdbuf[i] != ' ' && rdbuf[i] != 0)
				{
					arg2[j] = rdbuf[i];
					i++;
					j++;
				}

				/* welcome */
				if (strcmp(cmd, "welcome") == 0)
				{
					welcome();
				}

				/* clear screen */
				else if (strcmp(cmd, "clear") == 0)
				{
					clear();
				}

				/* show process*/
				else if (strcmp(cmd, "proc") == 0)
				{
					showProcess();
				}

				/* kill a process */
				else if (strcmp(cmd, "kill") == 0)
				{
					// printf("Process killed successfullly, pid: %s\n", arg1);
					killProcess(arg1);
				}

				/* make a new process */
				else if (strcmp(cmd, "mkpro") == 0)
				{
					makeProcess(arg1);
				}

				/* show help message */
				else if (strcmp(cmd, "help") == 0)
				{
					help();
				}

				/* create a file */
				else if (strcmp(cmd, "mkfile") == 0)
				{
					if(checkFilename(arg1)==0)continue;
					createFile(arg1, arg2);
				}

				/* create a directory */
				else if (strcmp(cmd, "mkdir") == 0)
				{
					if(checkFilename(arg1)==0)continue;
					createDir(arg1);
				}

				/* read a file */
				else if (strcmp(cmd, "read") == 0)
				{
					if(checkFilename(arg1)==0)continue;
					readFile(arg1);
				}

				/* edit a file cover */
				else if (strcmp(cmd, "edit") == 0)
				{
					if(checkFilename(arg2)==0)continue;

					if(strcmp(arg1, "-ad") == 0)
					{
						readFile(arg2);
						extendFile(arg2);
					}
					else if(strcmp(arg1, "-rw") == 0)
					{
						readFile(arg2);
						rewriteFile(arg2);
					}
					else if(strcmp(arg1, "-tr") == 0)
					{
						truncateFile(arg2);
					}
					else
					{
						printf("Command not found\n");
					}
					
				}

				/* remove a file */
				else if (strcmp(cmd, "rm") == 0)
				{
					if(checkFilename(arg1)==0)continue;

					deleteFile(arg1);
					memset(filepath, 0, MAX_FILENAME_LEN);
				}

				/* remove a directory */
				else if (strcmp(cmd, "deletedir") == 0)
				{
					if(checkFilename(arg1)==0)continue;

					strcpy(filepath, location);
					getDirFilepath(filepath, arg1);
					deleteDir(filepath);
					memset(filepath, 0, MAX_FILENAME_LEN);
				}

				/* ls */
				else if (strcmp(cmd, "ls") == 0)
				{
					ls();
				}

				/* cd */
				else if (strcmp(cmd, "cd") == 0)
				{
					cd(arg1);
				}

				/* information */
				else if (strcmp(cmd, "information") == 0)
				{
					information();
				}

				/* print */
				else if (strcmp(cmd, "print") == 0)
				{
					printf("%s\n", arg1);
				}

				else
				{
					printf("Command not found\n");
				}
			}
		}
		else {
			close(fd);
			int pid = fork();
			if (pid != 0) { /* parent */
				int s;
				wait(&s);
			}
			else {	/* child */
				execv(argv[0], argv);
			}
		}
	}

	close(1);
	close(0);
}
/*****************************************************************************
/*					        checkFilename
*****************************************************************************/
int checkFilename(const char * arg1)
{
	int n = strlen(arg1);
	//printf("checking......n=%d......checking......\n",n);
	if(n>MAX_FILENAME_LEN)
	{
		printf("Please use a shorter filename\n");
		return 0;
	}
	if(n<=0)
	{
		printf("Filename cannot be empty\n");
		return 0;
	}
	char ch;
	for(int i=0;i<n;i++){
		ch = *(arg1+i);
		//printf("checking......ch=%c......checking......\n",ch);
		if(ch=='/' || ch=='.' || ch==' ' || ch=='#')
		{
			printf("Filename cannot contain \'#\'\'.\'\'/\'\' \'\n");
			return 0;
		}
	}
	
	return 1;
}

/*****************************************************************************
/*					        getFullname
*****************************************************************************/
void getFullname(char * fullname, char * filename)
{
	memset(fullname, 0, MAX_FILENAME_LEN);
	strcpy(fullname, location);
	//printf("fullname:%s\n",fullname);
	strcat(fullname, "/");
	strcat(fullname, filename);
	//printf("fullname:%s\n",fullname);
	strrpl(fullname, '/', '_');
	//printf("fullname:%s\n",fullname);
}
/*****************************************************************************
*								Welcome
*****************************************************************************/
void welcome()
{

	printf(" =============================================================================\n");
	printf(" =                                                                           =\n");
	printf(" =                     Welcome to our Operating System                       =\n");
	printf(" =                                                                           =\n");
	printf(" =                       +-------------------------+                         =\n");
	printf(" =                       |  1652677    Wu Tongxin  |                         =\n");
	printf(" =                       |  1652695    Wen Yue     |                         =\n");
	printf(" =                       +-------------------------+                         =\n");
	printf(" =                                                                           =\n");
	printf(" =============================================================================\n");
	printf(" =                          HELP - List all commands                         =\n");
	printf(" =============================================================================\n");
	printf("\n\n\n\n");
}

/*****************************************************************************
*								Clear
*****************************************************************************/
void clear()
{
	int i = 0;
	for (i = 0; i < 20; i++)
		printf("\n");
}

/*****************************************************************************
*								Quit
*****************************************************************************/
void off()
{
	return 0;
}

/*****************************************************************************
*							  Show Process
*****************************************************************************/
void showProcess()
{
	int i = 0;
	printf("********************************************************************************\n");
	printf("     id      |     name      |      priority     |       flags(0 is runable)    \n");
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	for (i = 0; i < NR_TASKS + NR_PROCS; i++)
	{
		if(proc_table[i].p_flags == FREE_SLOT) {
			continue;
		}
		// char * newString = formatString(proc_table[i].name);
		// printf("%s\n", newString);
		printf("%s%d%s", 
			"      ",
			i,
			"     ");
		if(i < 10) {
			printf(" ");
		}
		showFormatString(proc_table[i].name);
		printf(" %d%s",
			proc_table[i].priority, 
			"                   ");
		if(proc_table[i].priority < 10) {
			printf(" ");
		}
		printf("%d\n", proc_table[i].p_flags);
	}
	printf("********************************************************************************\n");
}

/*****************************************************************************
*							  kill Process
*****************************************************************************/
void killProcess(char str[])
{
	// call a function str2Int() to transfer char[] to int, defined in klib.c
	int i = str2Int(str);
	// printf("pid is(in INT form: %d\n", i);
	if(i >= NR_TASKS + NR_PROCS || i < 0) {
		printf("Error! Pid %d exceeded the range\n", i);
	}
	else if(i < NR_TASKS) {
		printf("System tasks cannot be killed.\n");
	}
	else if(proc_table[i].priority == 0 || proc_table[i].p_flags == FREE_SLOT) 
	{
		printf("Process with pid = %d not found.\n", i);
	}
	else {
		proc_table[i].priority = 0;
		proc_table[i].p_flags = FREE_SLOT;
		printf("Process with pid = %d is finished.\n", i);	
	}
}

/*****************************************************************************
*							  folk new Process
*****************************************************************************/
void makeProcess(char str[])
{
	int pid = fork();
	int childPID;
	for(int i = 0; i < NR_TASKS + NR_PROCS; i++)
	{
		if(proc_table[i].p_flags == FREE_SLOT)
		{
			childPID = i;
			break;
		}
	}
	if(getSize(str) <= 0) {
		strcpy(str, "Unnamed");
	}
	if (pid != 0) { /* parent */
		childPID = pid;
		int s;
		wait(&s);
	}
	else {	/* child */
		// printf("priority: %d, pid: %d, \n", proc_table[childPID].priority, childPID);
		printf("successfullly make a new process.\n");
		strcpy(proc_table[childPID].name, str);
		proc_table[childPID].p_flags = RECEIVING;
		proc_table[childPID].priority = 5;
	}
	showProcess();
}


/*****************************************************************************
*							Show Help Message
*****************************************************************************/
void help()
{
	printf("===============================================================================\n");
	printf("=       Command                        Description                            =\n");
	printf("=-----------------------------------------------------------------------------=\n");
	printf("=       welcome                        Print welcome page                     =\n");
	printf("=       clear                          Clean the screen                       =\n");
	printf("=       cd       [path]                Get into the path                      =\n");
	printf("=       ls                             List all the files in current directory=\n");
	printf("=       help                           List all commands                      =\n");
	//printf("=       proc                           List all process's message             =\n");
	//printf("=       kill   [id]                    Kill a process with this pid           =\n");
	//printf("=       mkpro  [name]                  Folk and start a new process           =\n");
	printf("=       mkdir    [name]                Create a directory                     =\n");
	printf("=       mkfile   [file] [content]      Create a textfile                      =\n");
	printf("=       read     [file]                Read a file                            =\n");
	printf("=       rm       [file]                Delete a file                          =\n");
	//printf("=       deletedir[file]               Delete a directory                     =\n");
	printf("=       edit     -ad    [filename]     Edit file, add content behind          =\n");
	printf("=       edit     -rw    [filename]     Edit file, rewrite/cover the file      =\n");
	printf("=       edit     -tr    [filename]     Truncate the file                      =\n");
    printf("=       calendar                       Print calendar                         =\n");
    printf("=       clear                          Clean the screen                       =\n");
    printf("=       calculator                     Use calculator                         =\n");
    printf("=       game                           Play games                             =\n");
	printf("===============================================================================\n");

}


/*****************************************************************************

*								Information

*****************************************************************************/

void information()

{

	printf(" MEMORYSIZE:%dMB\n", memory_size / (1024 * 1024));

	printf(" STACK_SIZE_TOTAL:%dMB\n", STACK_SIZE_TOTAL / (1024 * 1024));

	printf(" MMBUF_SIZE:%dMB\n", MMBUF_SIZE / (1024 * 1024));

	printf(" FSBUF_SIZE:%dMB\n", FSBUF_SIZE / (1024 * 1024));

}
/*****************************************************************************
*							File System
*****************************************************************************/

/*****************************************************************************
*								Init FS
*****************************************************************************/
void initFS()
{
	int fd = -1, n = 0;
	char bufr[1024];
	char filepath[MAX_FILENAME_LEN];
	char dirpath[MAX_FILENAME_LEN];
	char filename[MAX_FILENAME_LEN];

	memset(filequeue, 0, MAX_FILES);
	memset(dirqueue, 0, MAX_DIRS);
	
	fd = open("root", O_CREAT | O_RDWR);
	//printf("fd=%d\n",fd);
	close(fd);

	fd = open("root", O_RDWR);
	for(int i=0;i<1024;i++){
		bufr[i]=0;
	}
	write(fd, bufr, 1024);
	close(fd);
	
	memset(dirmap, 0, MAX_DIRS);
	for(int i=0;i<MAX_DIRS;i++){
		dirmap[i]=0;
		strcpy(dir[i], "");
	}
	dirmap[0]=1;
	strcpy(dir[0], "root");
}

/*****************************************************************************
*							 isDir
*return 1 if it is a directory, otherwise 0
*****************************************************************************/
int isDir(const char * dirpath)
{
	for(int i=0;i<MAX_DIRS;i++){
		if(strcmp(dir[i], dirpath)==0)
			return 1;
	}
	return 0;
}

/*****************************************************************************
*                             Get Filepath
*****************************************************************************/
void getFilepath(char *filepath, char * filename)
{
	strjin(filepath, filename, '_');
}

/*****************************************************************************
*                         Get Directory Filepath
*****************************************************************************/
void getDirFilepath(char *filepath, char * filename)
{
	strcat(filepath, "_");
	strjin(filepath, filename, '#');
}

/*****************************************************************************
*                   Get Dirpath And Filename/Dirname From Filepath
*****************************************************************************/
void getDirpathAndFilename(char * dirpath, char * filename, char * filepath)
{

	char str[MAX_FILENAME_LEN];
	int i, k;

	memset(dirpath, 0, MAX_FILENAME_LEN);
	memset(filename, 0, MAX_FILENAME_LEN);

	for (i = 0, k = 0; filepath[i] != 0; i++)
	{
		if (filepath[i] != '_')
		{
			str[k] = filepath[i];
			k++;
		}
		else
		{
			strcat(dirpath, str);
			strcat(dirpath, "_");
			memset(str, 0, MAX_FILENAME_LEN);
			k = 0;
		}
	}
	dirpath[strlen(dirpath) - 1] = 0;
	strcpy(dirpath, dirpath);
	strcpy(filename, str);

}

/*****************************************************************************
*						Get a Free Pos in FileQueue
*****************************************************************************/
int getFreeFilePos()
{
	int i = 0;
	for (i = 0; i < MAX_FILES; i++)
	{
		if (filequeue[i] == 0)
			return i;
	}
	printf("The number of files is full!!\n");
	return -1;
}

/*****************************************************************************
*						Get a Free Pos in DirQueue
*****************************************************************************/
int getFreeDirPos()
{
	int i = 0;
	for (i = 0; i < MAX_DIRS; i++)
	{
		if (dirqueue[i] == 0)
			return i;
	}
	printf("The number of folders is full!!\n");
	return -1;
}

/*****************************************************************************
*						Get Dir's Pos in FileQueue
*****************************************************************************/
int getPosInDirQueue(char * filepath)
{
	int i = 0;
	for (i = 0; i < MAX_FILES; i++)
	{
		if (strcmp(dirs[i], filepath) == 0)
			return i;
	}
	return -1;
}


/*****************************************************************************
*						Add Filename Into Dir
*****************************************************************************/
void addFileIntoDir(const char * dirpath, char * filename)
{
	int fd = -1;

	if (strcmp(dirpath, "root") == 0)
	{
		fd = open("root", O_RDWR);
		
	}
	else
	{
		fd = open(dirpath, O_RDWR);
	}

	if (fd == -1)
	{
		printf("%s has not been found!\n", dirpath);
		return;
	}

	strcat(filename, " ");
	editAppand(dirpath, filename);
}

/*****************************************************************************
*						Delete Filename From Dir
*****************************************************************************/
void deleteFileFromDir(const char * dirpath, char * filename)
{

	/*char bufr[MAX_USER_FILE * MAX_FILENAME_LEN];
	char bufw[MAX_USER_FILE * MAX_FILENAME_LEN];*/
	char bufr[1024];
	char bufw[1024];
	char buf[MAX_FILENAME_LEN];
	int fd = -1, n = 0;

	fd = open(dirpath, O_RDWR);

	if (fd == -1)
	{
		printf("%s has not been found!!\n", dirpath);
		return;
	}

	n = read(fd, bufr, 1024);

	int i, k;
	for (i = 0, k = 0; i < n; i++)
	{
		if (bufr[i] != ' ')
		{
			buf[k] = bufr[i];
			k++;
		}
		else
		{
			buf[k] = 0;
			k = 0;

			if (strcmp(buf, filename) == 0)
				continue;

			strcat(bufw, buf);
			strcat(bufw, " ");
		}
	}
	printf("%s\n", bufw);
	
	editCover(dirpath, bufw);

	close(fd);
}

/*****************************************************************************
*							 Create File
*****************************************************************************/
void createFile(char * filename, char * buf)
{
	/* create & open new file */
	int fd = -1;//, pos = -1;
	char fullname[MAX_FILENAME_LEN];
	getFullname(fullname, filename);
	fd = open(fullname, O_CREAT | O_RDWR);
	////////////////////////////////
	printf("fullname: %s\nfilename: %s\n content: %s\n", fullname, filename, buf);
	///////////////////////////////
	if (fd == -1)
	{
		printf("Oops! Internal error!\n");
		return;
	}
	else if (fd == -2)
	{
		printf("File \'%s\' exists!\n",filename);
		return;
	}

	/* write file */
	write(fd, buf, strlen(buf)+1);

	/* close file */
	close(fd);

	/* update dir */
	updateDir(filename,1);
	
	///////////////////////////////////////
/*	pos = getFreeFilePos();
	filequeue[pos] = 1;
	strcpy(files[pos], filepath);
	filecount++;

	addFileIntoDir(location, filename);*/
}
/*****************************************************************************
*							 Update Directory
*****************************************************************************?
/*
*@param	filename	
*@param op		ADD(1),DELETE(2)
*
*return Zero if success, otherwise -1
*****************************************************************************/
int updateDir(char * filename, int op)
{
	/* get full path of dir */
	char * dirpath;
	strcpy(dirpath, location);
	strrpl(dirpath,'/','_');
////////////////////////////
	printf("updateDir()dirpath:%s\n",dirpath);
///////////////////////////////

	/* read dir */
	char bufr[1024];
	if(miniRead(dirpath, bufr) != 0)
	{
		return -1;
	}
/////////////////////
	printf("before bufr:%s\n",bufr);
///////////////////

	/* update dir */
	int namelen = strlen(filename);
	char temp[MAX_FILENAME_LEN];	/* store the dir name temporally */
	char * t = temp;	/* pointer of temp */
	char * it = bufr;	/* pointer of bufr */
	switch (op) {
		case 1:/* add new item */
			strcat(bufr, filename);
			strcat(bufr, " ");
			break;
		case 2:/* delete item */
			while(*it){
				if(*it == ' ')
				{
					printf("get[%s]target[%s]\n",temp,filename);
					if(strcmp(filename,temp)==0)
					{
						/* found */
						it++;
						do {
							*(it-namelen-1)=*it;
							it++;
						}while(*it);
						break;
					}
					/* not found , reset */
					it++;
					t = temp;
					memset(temp, 0, MAX_FILENAME_LEN);
				}
				else 
				{
					*t++ = *it++;
				}
			}
			break;
		default:
			break;
	}

	
	miniWrite(dirpath, bufr);
/////////////////////
	printf("after bufr:%s\n",bufr);
///////////////////
	char bufcheck[1024];
	miniRead(dirpath, bufcheck);
/////////////////////
	printf("now dir:%s\n",bufcheck);
///////////////////
	return 0;
}
/*****************************************************************************
*							 Create Directory
*****************************************************************************/
void createDir(char * filename)
{
	/* get fullname of dir */
	char fullname[MAX_FILENAME_LEN];
	getFullname(fullname, filename);

	/* update global data */
	/* find a slot in dirmap */
	int i=0;
	for(;i<MAX_DIRS;i++){
		if(dirmap[i]==0)
			break;
	}
	if(i>=MAX_DIRS)	/* {FS} is full */
	{
		printf("There is no room for new directory!");
		return;
	}
	/* add item into dir[][] */
	dirmap[i]=1;
	strcpy(dir[i], fullname);
//////////////////////////////////////
	printf("i=%d,dir[i]=%s,fullname=%s\n",i,dir[i],fullname);
	printf("*************this is dir list**********\n");
	for(int j=0;j<MAX_DIRS;j++){
		if(dirmap[j]==1)
			printf("pos:%d,%s\n",j,dir[j]);
////////////////////////////////////

	}

	/* create & open new dir */
	int fd = -1;
	//spin("open?");
	fd = open(fullname, O_CREAT | O_RDWR);
///////////////////////////////
	printf("Folder fullname: %s\n", fullname);
//////////////////////////////
	if (fd == -1)
	{
		printf("New folder failed. Please check and try again!!\n");
		/* roll back */
		for(int k=0;k<MAX_DIRS;k++){
			if(strcmp(dir[k], fullname)==0)
			{
				memset(dir[k], 0, MAX_FILENAME_LEN);
				dirmap[k]=0;
				break;
			}
		}
		return;
	}
	else if (fd == -2)
	{
		printf("File \'%s\' exists!!\n", filename);
		/* roll back */
		for(int k=0;k<MAX_DIRS;k++){
			printf("dir[k]:%s,fn:%s\n",dir[k], fullname);
			if(strcmp(dir[k], fullname)==0)
			{
				memset(dir[k], 0, MAX_FILENAME_LEN);
				dirmap[k]=0;
				break;
			}
			printf("yes i ve been here\n");
		}
		return;
	}
	/* initialize dir */
	char dirbuf[1024];
	memset(dirbuf, 0, 1024);
	write(fd, dirbuf, 1024);

	/* close dir */
	close(fd);
	
	/* update dir */
	updateDir(filename,1);
}

/*****************************************************************************
*								Read File
*****************************************************************************/
void readFile(char * filename)
{
	/* check if it is a dir */
	char fullname[MAX_FILENAME_LEN];
	getFullname(fullname, filename);
	if (isDir(fullname)==1)
	{
		printf("\'%s\' is not a file!\n",filename);
		return;
	}
////////////////////////////
	printf("inside readFile()fullname:%s\n",fullname);
///////////////////////////////

	/* read file */
	char bufr[1024];
	if(miniRead(fullname, bufr) != 0)
	{
		return;
	}
	/* display */
	char pathname[MAX_FILENAME_LEN];
	memset(pathname, 0, MAX_FILENAME_LEN);
	strcpy(pathname, fullname);
	strrpl(pathname,'_','/');
	printf("--------------------------------------------------\n");
	printf("Filepath: %s\n", pathname);
	printf("Content: \n%s\n", bufr);
	printf("--------------------------------------------------\n");

}

/*****************************************************************************
*							Edit File Cover
*****************************************************************************/
void editCover(const char * filepath, char * str)
{
	char empty[1024];
	int fd = -1;
	fd = open(filepath, O_RDWR);
	if (fd == -1)
	{

		printf("Opening file error. Please check and try again!!\n");
		return;
	}
	memset(empty, 0, 1024);
	write(fd, empty, 1024);
	close(fd);
	fd = open(filepath, O_RDWR);
	write(fd, str, strlen(str));
	close(fd);
}

/*****************************************************************************
*							Edit File Appand
*****************************************************************************/
void editAppand(const char * filepath, char * str)
{
	int fd = -1;
	char bufr[1024];
	char empty[1024];

	fd = open(filepath, O_RDWR);
	if (fd == -1)
	{
		printf("Opening file error. Please check and try again!!\n");
		return;
	}

	read(fd, bufr, 1024);
	close(fd);

	fd = open(filepath, O_RDWR);
	write(fd, empty, 1024);
	close(fd);

	strcat(bufr, str);

	fd = open(filepath, O_RDWR);
	write(fd, bufr, strlen(bufr));
	close(fd);
}
/*****************************************************************************
*							   Delete File
*****************************************************************************/
void deleteFile(char * filename)
{
	/* unlink file */
	char fullname[MAX_FILENAME_LEN];
	getFullname(fullname, filename);
	if (unlink(fullname) != 0)
	{
		printf("Error encountered deleting file. Please check and try again!\n");
		return;
	}

	/* update dir */
	updateDir(filename,2);
	

	/* delete filename from user's dir */
	/*
	char dirpath[MAX_FILENAME_LEN];
	char filename[MAX_FILENAME_LEN];
	getDirpathAndFilename(dirpath, filename, filepath);

	deleteFileFromDir(dirpath, filename);*/
}

/*****************************************************************************
*							 Delete Directory
*****************************************************************************/
void deleteDir(char * filepath)
{
	char dirfile[MAX_FILENAME_LEN];
	char rdbuf[1024];
	int fd = -1, n = 0;
	char filename[MAX_FILENAME_LEN];
	fd = open(filepath, O_RDWR);
	if (fd == -1)
	{
		printf("Deleting folder error. Please check and try again!!\n");
		return;
	}

	n = read(fd, rdbuf, 1024);

	int i, k;
	for (i = 0, k = 0; i < n; i++)
	{

		if (rdbuf[i] != ' ')
		{
			dirfile[k] = rdbuf[i];
			k++;
		}
		else
		{
			dirfile[k] = 0;
			k = 0;

			char path[MAX_FILENAME_LEN];
			strcpy(path, filepath);
			strjin(path, filename, '_');

			if (dirfile[0] == '#')
			{
				deleteDir(path);
			}
			else
			{
				deleteFile(path);
			}
		}
	}
	close(fd);

	if (unlink(filepath) != 0)
	{
		printf("Deleting folder error. Please check and try again!\n");
		return;
	}

	for (i = 0; i < dircount; i++)
	{
		if (strcmp(dirs[i], filepath) == 0)
		{
			memset(dirs[i], 0, MAX_FILENAME_LEN);
			dirqueue[i] = 0;
			dircount++;
			break;
		}
	}

	char dirpath[MAX_FILENAME_LEN];

	getDirpathAndFilename(dirpath, filename, filepath);
	deleteFileFromDir(dirpath, filename);
}

/*****************************************************************************
*					ls
*****************************************************************************/
void ls()
{
	/* get dir fullname */
	char dirpath[MAX_FILENAME_LEN];
	memset(dirpath, 0, MAX_FILENAME_LEN);
	strcpy(dirpath, location);
	strrpl(dirpath,'/','_');
///////////////////////////////
	printf("inside ls()dirpath=%s\n",dirpath);
/////////////////////////////////
	/* open dir */
	char bufr[1024];
	miniRead(dirpath, bufr);

	printf("%s\n", bufr);
}

/*****************************************************************************
*									cd
*****************************************************************************/
void cd(char * arg1)
{
	char location2[MAX_FILENAME_LEN];	/* copy of location */
	strcpy(location2, location);
	char * lo = location2+strlen(location2); /* pointer of location2 */
	char * ar = arg1; 			/* pointer of arg1 */
////////////////////////////////////////
	printf("location2:%s,lo:%s,ar:%s\n",location2,lo-1,ar);
////////////////////////////////////////

	/* root */
	if(strcmp(arg1, "/")==0)
	{
		printf("what1");
		strcpy(location, "root");
		return;
	}

	/* start from root */
	if(*ar == '/')
	{
		printf("what2");
		strcpy(location2, "root/");
		lo=location2+strlen(location2);
		ar++;
	} 
	else
	{
		*lo = '/';
		lo++;
	}
	
	/* get new path */
	while(*ar){
		if(*ar == '.' && *(ar+1) == '.')	/* go back */
		{
			//printf("2\n");////////////////
			lo -= 2;
			*(lo+2) = 0;
			*(lo+1) = 0;
			//printf("now lo:%s\n",lo);
			while(*lo != '/'){
				lo--;
				*(lo+1)=0;
				if(lo == location2)
				{
					printf("cd \'%s\':Invalid path!\n", arg1);
					return;
				}
			}
			*lo = 0;
			ar += 2;		
		}
		else
		{
			//printf("3\n");////////////////////
			*lo++ = *ar++;
		}

	}
////////////////////////////////////////
	//printf("before check location2:%s,lo:%s\n",location2,lo);
////////////////////////////////////////
	/* remove '/' at the end of location2 if there is one */
	lo = location2+strlen(location2)-1;
	if(*lo == '/')
	{
		lo--;
		*(lo+1) = 0;
	}
////////////////////////////////////////
	//printf("after location2:%s\n",location2);
////////////////////////////////////////	

	/* check if the dirctory exists */
	char dirpath[MAX_FILENAME_LEN];
	strcpy(dirpath, location2);
	strrpl(dirpath,'/','_');
	if (isDir(dirpath)==0)
	{
		printf("cd \'%s\':No such directory!\n", location2);
		return;
	}
	
	/* change the real location */
	strcpy(location, location2);
	
}

/*****************************************************************************
*					miniRead
*return 0 if success
*****************************************************************************/
int miniRead(char * fullname, char * buf)
{
	int fd = -1;
	int n;
	memset(buf, 0, 1024);

	/* open file */
	fd = open(fullname, O_RDWR);
	
	if (fd == -1)
	{
		printf("File not found!\n");
		return -1;
	}

	/* read file */
	n = read(fd, buf, 1024);
	buf[n] = 0;

	/* close file */
	close(fd);
	return 0;	
}
/*****************************************************************************
*					miniWrite
*return 0 if success
*****************************************************************************/
int miniWrite(char * fullname, char * buf)
{
	int fd = -1;

	/* open file */
	fd = open(fullname, O_RDWR);

	if (fd == -1)
	{
		printf("File not found!\n");
		return -1;
	}

	/* write file */
	write(fd, buf, strlen(buf)+1);

	/* close file */
	close(fd);
	return 0;
}
/*****************************************************************************
*					truncate File
*****************************************************************************/
void truncateFile(char * filename)
{
	/* get full name */
	char fullname[MAX_FILENAME_LEN];
	memset(fullname, 0, MAX_FILENAME_LEN);
	getFullname(fullname, filename);

	/* truncate file */
	char buf[1024];
	memset(buf, 0, 1024);
	miniWrite(fullname, buf);
}
/*****************************************************************************
*					extend File
*****************************************************************************/
void extendFile(char * filename)
{
	/* get full name */
	char fullname[MAX_FILENAME_LEN];
	memset(fullname, 0, MAX_FILENAME_LEN);
	getFullname(fullname, filename);

	/* read file, extend content, write file */
	char buf[1024];
	miniRead(fullname, buf);
	
	/* get update content */
	char content[256];
	memset(content, 0, 256);
	printf("Input content:\n");
	int r2 = read(0, content, 256);
	content[r2] = 0;
///////////////////////////	
	printf("rdbuf:%s\nlen:%d\nbuf:%s\nlen:%d\n",content,strlen(content),buf,strlen(buf));
//////////////////////////////
	strcat(buf, content);
	printf("buf:%s\n",buf);
	miniWrite(fullname, buf);

	/* display result */
	char bufr[1024];
	miniRead(fullname, bufr);
	printf("--------------------------------------------------\n");
	printf("(Result) Content:\n%s\n", bufr);
	printf("--------------------------------------------------\n");
	
}
/*****************************************************************************
*					rewrite File
*****************************************************************************/
void rewriteFile(char * filename)
{
	/* get full name */
	char fullname[MAX_FILENAME_LEN];
	memset(fullname, 0, MAX_FILENAME_LEN);
	getFullname(fullname, filename);

	/* get update content */
	char content[256];
	memset(content, 0, 256);
	printf("Input content:\n");
	int r2 = read(0, content, 256);
	content[r2] = 0;
///////////////////////
	printf("rdbuf:%s\n",content);	
/////////////////////

	/* rewrite file */
	miniWrite(fullname, content);

	/* display result */
	char bufr[1024];
	miniRead(fullname, bufr);
	printf("--------------------------------------------------\n");
	printf("(Result) Content:\n%s\n", bufr);
	printf("--------------------------------------------------\n");
}

