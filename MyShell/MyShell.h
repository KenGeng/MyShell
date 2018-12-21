#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h> //umask


#define ERROR -1
#define MaxVariableNameLength 50
#define MaxProgramNameLength 200
#define MaxLine 200
#define MaxBgProgramNum 5
#define MaxArgNum 10//最大参数数目
#define PIPE_TYPE 1003
#define EXIT -2
#define QUIT -3
#define ShellScript -4 //用于执行脚本时，判断脚本内容是否执行完毕
#define RedirectOverwrite 99
#define RedirectAddition 100
#define StdandardOut 1
#define FgMark 1004
#define BgMark 1005

enum {running=999,stopped=1000,unused=1001};
enum {true = 1,false = 0};

typedef struct variable_node//变量链表的节点
{
    char name[MaxVariableNameLength];//变量名
    char *value;//变量值
    struct variable_node *next;
}* var_list;//变量链表
typedef var_list ptr_var_node;//变量指针
typedef struct
{
    char name[MaxProgramNameLength];//命令
    char parameter[MaxProgramNameLength];//命令参数
    int state;//状态
    int pid;//进程id
} BgProgram;//后台程序信息 元素
typedef struct command_input
{
    int type;
    int num;//命令的参数个数
    int  fd;//file descriptor文件描述符 用于重定向命令
    char * filename;//文件名 用于重定向命令
    char *content;//原始输入
    char *command_0;//命令0的文本 本shell实现的大多内部命令如cd test等 只使用该变量
    char *command_1;//命令1的文本（只在管道命令中使用，最多有一个管道）
    char **parameter_0;//命令0的参数数组
    char **parameter_1;//命令1的参数数组
} command;
typedef struct command_input* ptrCmd;//定义指向command类型的指针类型

