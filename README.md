# MyShell
A minishell written by C. Support commands like ls/cd/bg/echo/exec...

# English Introduction
## MyShell Basic Introduction
### shell concept
The shell provides a way to communicate with your operating system. This communication can be done interactively (either from the keyboard and immediately responsive) or in a shell script (non-interactive). A shell script is a bunch of shell and operating system commands placed in a file that can be reused. Essentially, shell scripts are simple combinations of command line commands into a single file.
Shell is basically a command interpreter, similar to the command under DOS. It receives user commands (such as ls, etc.) and then calls the corresponding application. The more common shells are the standard Bourne shell (sh) and C shell (csh).

The Shell under Linux is a bridge between users and Linux, and is an interface program between the user and the Linux kernel.
Each shell also has its own built-in commands and internal variables.

### order
1. There are three execution commands in MyShell.
- Built-in commands
MyShell does not spawn new processes when executing built-in commands, but is executed directly by the shell program.
cd, echo, exec, exit, pwd, set, shift, test, time, umask, unset, etc.

2. External commands
Execution of external commands in MyShell creates a child process, and the exec function is called by the child process to execute an external command. At this point, the environment of the child process will be replaced by the environment of the corresponding external command.

3. Execute a shell script
When executing the script, MyShell will first determine if the command is a MyShell internal command or an external command. If it is an internal command, it is directly called by MyShell, and if it is an external command, a new process is created. Can be intuitively understood as an input redirection.

4. MyShell basic functions

4.1 Built-in commands


|Command | Instruction Format | Description |
| ---- | ---- | ---- |
|cd |cd [dir] | Change the current working directory |
|clr |clr |Clear screen operation |
|echo |echo [arg ...] |Print variables or characters to standard output |
|dir |dir [dir] |List all files in the specified directory |
|environ | environ | Print all environment variables |
|exit |exit |Exit MyShell normally |
|help |help [inst] | Show help list for built-in commands |
|MyShell| MyShell [filename]| Execute shell script file |
|pwd | pwd | Show current working directory |
|set |set [command or arg ...]| New, set variables |
|shift |shift [n] | Shift position variable |
|time | time | Shows the current date and time of the system |
|test |test expression | judge the expression authenticity|
|unset |unset [name] | Delete set variables |

among them:  
In my implementation, echo parses variables and quote symbols. But after $, all characters must be variable names
Sets without parameters display the latest custom variables by default, which is different from the normal set.

4.2 Pipeline
MyShell only supports single-layer pipes. There can only be one pipe command at a time, such as echo /home | dir

4.3 Execution script
MyShell can extract commands by reading in the file.
The script can be read using the following command: MyShell filename
The MyShell command can process commands in a file by reading in a file. When reading in a file, MyShell will read and interpret the contents of the file line by line, and when it reaches the end of the file, MyShell will exit. Note that each line of the file must be a separate command.

4.4 Background Execution Program
Adding a space and a after the command allows the command to execute in the background.
MyShell implements the background execution of the program by creating a child process. The main process jumps directly to wait for the next command input, without waiting for the end of the subprocess's operation. The child process will automatically end the process after executing the corresponding command. Note: There are some bugs in this part of the implementation.

4.5 variables
- System environment variables
Environment variables in the Linux system are initialized when the operating system is started, regardless of the shell version used by the system. Commonly used environment variables are PATH, HOME, USER, PWD, and so on. In MyShell, environment variables are obtained through the external global variable env.
- local variables
Variables set inside the shell. The local variables of each shell version are independent and cannot be accessed or modified by other shells.
In MyShell, the user sets the local variable with an equal sign. The left side of the equal sign is the variable name. The right side of the equal sign is the value of the variable. Variable assignment supports instruction assignment, which assigns the result of the instruction output to a variable.
It is also possible to assign values ​​directly to variables.
The unset directive removes local variables created by the set.
- Positional parameters
The location parameter is a special kind of local variable in the shell that can be used by the command line to pass parameters to the shell script. $1 represents the first parameter and $2 represents the second parameter...
In MyShell you can assign values ​​directly to positional parameters. Such as $1=hi
There are some special positional parameters in the normal shell. $@ represents all values ​​of the positional parameter, and $# represents the number of positional parameters.
The shift operation can move the positional parameters. For example, shift n: move the position parameter forward by n bits, that is, the n+1th bit moves to the 1st bit, n+2 bit to the 2nd bit... The first n bits are discarded

4.6 Input and Output Redirection
The default standard input (stdin=0) and output (stdout=1) in LINUX correspond to the screen of the terminal. Input and output redirection, is the redirection of the output or input of a file, command, program, script to another file, command, program, script. Essentially the standard input and output are each a file (which is already open at system startup with file descriptors of 0 and 1 respectively).

In the shell, the output redirection is implemented by > or >>, that is, the input and output of the table are output to the target file instead of the screen end.

The file on the right may or may not exist. If the file does not exist, create a new one. ">"Overwrite the output of the file. ">>" means that the output will be added at the end of the file.
Similarly, input redirection can be achieved.

4.7 Pipeline operation
In Linux you can use the pipe character ("|") to import or redirect the standard output of a command to the standard output of the next command. For example, command1 | command2 means to operate the output of commamd1 as input to command2. Such as echo /home | dir

### Introduction to some programming implementations
Set, unset, shift
A global list of variables is maintained in the main function. Set is to add variables at the end of the list; unset deletes the corresponding node of the linked list; shift is achieved by right shifting the dummy head of the linked list and renaming the various position parameters.
2. Script execution
Called the execvp function, re-executed a MyShell, and redirected input to the script file to read in and execute the script command
3. Input and output redirection
Directly call dup2 to redirect the output to the file. The difference between > and >> is the parameter setting of the open file.
4. Pipeline commands
Create a new child process, and pass the output of the first command to the input of the second command through the pipeline to implement the pipeline command.

The above is my introduction to MyShell. In the process of implementation, I checked a lot of information on the Internet. Some of the referenced code added the original author's URL to the source code comment of the program. In addition, I also consulted several students and seniors about the implementation of the pipeline operation.
Finally, thank you for reading and using!
# 中文介绍

## MyShell基础介绍
###  shell的概念
shell提供了你与操作系统之间通讯的方式。这种通讯可以以交互方式（从键盘输入，并且可以立即得到响应），或者以shell script(非交互）方式执行。shell script是放在文件中的一串shell和操作系统命令，它们可以被重复使用。本质上，shell script是命令行命令简单的组合到一个文件里面。
Shell基本上是一个命令解释器，类似于DOS下的command。它接收用户命令（如ls等），然后调用相应的应用程序。较为通用的shell有标准的Bourne shell (sh）和C shell (csh）。

Linux 下的Shell是用户使用Linux的桥梁，是用户和Linux内核之间的接口程序。
每一个 Shell 程序也拥有自己的一些内建命令和内部变量。 

### 命令
1. 在MyShell中有三种执行命令
- 内置命令 
   MyShell在执行内置命令的时候不会派生新的进程，而是由shell程序直接执行。
   cd、echo 、exec 、exit、pwd 、set 、shift 、test 、time 、umask 、unset等  

2. 外部命令  
   MyShell中的外部命令的执行会创建出子进程，通过子进程调用exec函数来执行外部命令。这时子进程的环境会被相应外部命令的环境所取代。

3. 执行shell脚本  
	​	在执行脚本程序时，MyShell会先判断命令是MyShell内部命令还是外部命令。如果是内部命令则由MyShell直接调用执行，若是外部命令则创建新的进程执行。可以直观地理解成一种输入的重定向。

4. MyShell基本功能

4.1 内建命令  


|命令| 指令格式 |说明 |
| ---- | ---- | ---- |
|cd       |cd [dir]                  |  改变当前的工作目录  |
|clr      |clr                         |清屏操作  |
|echo     |echo [arg ...]              |将变量或者字符打印到标准输出 |
|dir      |dir [dir]                   |列出指定目录下的所有文件  |
|environ | environ               |      打印所有的环境变量  |
|exit     |exit                        |正常退出MyShell  |
|help     |help [inst]            |     显示内建命令的帮助列表  |
|MyShell|  MyShell [filename]|          执行shell脚本文件  |
|pwd     | pwd                        | 显示当前的工作目录  |
|set      |set  [command or arg ...]|   新建、设置变量  |
|shift    |shift [n]                  | 左移位置变量  |
|time    | time                       | 显示出系统当前的日期和时间 |
|test     |test 表达式 	|				 判断表达式真伪|
|unset    |unset [name]      |          删除set的变量  |

其中：  
在我的实现中，echo可解析变量和引号符号。但是在$之后所有的字符必须都为变量名   
不加参数的set默认显示最新的自定义变量，与正常的set不同

4.2 管道
MyShell只支持单层管道 即一次只能有一个管道命令 如echo /home | dir

4.3 执行脚本  
​	MyShell可以通过读入文件提取命令。  
可以使用以下命令进行脚本读入：MyShell filename  
MyShell命令可以通过读入文件处理文件中的命令。读入文件时，MyShell会对文件内容进行逐行读入并解释，当到达文件尾时，MyShell会退出。注意，文件每行必须为一个单独的命令。  

4.4 后台执行程序  
​	在命令后加上空格和&，便可以让命令在后台执行。  
​	MyShell通过创建子进程来实现程序的后台执行。主进程则直接跳转等待下一次命令输入，而不用等待子进程的操作结束。子进程在执行完毕相应的命令后也会自动结束进程。注：该部分的实现有些bug  

4.5 变量
- 系统环境变量  
	​	Linux系统中的环境变量在操作系统启动的时候被初始化，与系统所使用的Shell版本无关。常用的环境变量有PATH、HOME、USER、PWD等。在MyShell中，环境变量通过外部全局变量env获得。  
- 本地变量  
	​	Shell内部设置的变量。各个shell版本的本地变量各自独立，不能被其他shell访问或者修改。  
	​	在MyShell中用户用等号设置本地变量。等号左边是变量名。等号右边是变量的值。变量赋值支持指令赋值，即将指令输出的结果赋值给变量。  
	​	也可以直接将值赋值给变量。
	​	unset指令可删除set创建的本地变量。  
- 位置参数  
	​	位置参数是shell中一类特殊的本地变量，可以用于命令行向shell脚本中传递参数使用。$1代表第1个参数，$2代表第二个参数…   
	​	在MyShell中可以直接给位置参数赋值。如$1=hi  
	​	在正常的shell中还有一些特殊的位置参数。$@表示位置参数的所有的值，$#表示位置参数的个数。  
	​	shift操作可以实现位置参数的移动。例如 shift n：将位置参数向前移动n位，即第n+1位移动到第1位，n+2位到第2位… 前n位被舍弃 

4.6 输入与输出重定向
LINUX 中默认的标准输入(stdin=0)和输出(stdout=1)对应的都是终端的屏幕。而输入输出重定向，就是将一个文件、命令、程序、脚本的输出或输入重新定向到另一个文件，命令，程序，脚本。实质上标准输入输出各自也都是一个文件(在系统启动时就已经打开，其文件描述符分别为0和1)。  

在shell中通过>或>>来实现输出重定向，即将表准输入输出到目标文件中而不是屏幕端。  

右边的文件可以存在也可以不存在。如果文件不存在，就新建文件。">"将输出结果覆盖文件内容。">>"表示将将输出结果添加在文件末尾。  
类似地可以实现输入重定向。  

4.7 管道操作
在linux中可以使用管道符（”|”）将一个命令的标准输出被导入或者重定向为下一个命令的标准输出。例如 command1 | command2 意思是将commamd1的输出作为command2的输入进行操作。 如echo /home | dir

### 一些编程实现的介绍
1. set、unset、shift
在main函数中维护了一个全局的变量链表。set即在链表尾添加变量；unset即删除链表相应节点；shift的实现是通过右移链表的dummy head，并重命名各个位置参数实现的。
2.脚本执行
调用了execvp函数，重新执行一个MyShell，并重定向输入到脚本文件来读入、执行脚本命令
3.输入与输出重定向
直接调用dup2将输出重定向到文件,>与>>实现差别在于打开文件的参数设置。
4.管道命令
新建子进程，通过管道将第一个命令的输出传入第二个的命令的输入，实现管道命令。

以上就是我的MyShell介绍。在实现的过程中，我上网查阅了不少资料，部分参考的代码在程序源代码注释中添加了原作者的网址，另外关于管道操作的实现也咨询了几位同学和学长，一并感谢。
最后，感谢您的阅读与使用！

