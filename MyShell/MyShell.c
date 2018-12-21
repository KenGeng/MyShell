
////  MyShell.c

#include "MyShell.h"


int BgProgCount=0;//记录后台挂起的程序数目
BgProgram BgProgTable[MaxBgProgramNum];  //记录在后台运行的程序信息
char ShellPath[1000] = {0}; //myshell的执行文件所处的绝对路径
char ReadMePath[1000] = {0}; //记录当前readme文件所处的路径 用于help命令的使用

var_list myshell_var_list=NULL;//管理myshell中环境变量与自定义变量的链表头结点
var_list var_list_rear = NULL; //myshell_var_list尾结点
int BgProgramNum = 0; //已有的后台程序数目


extern char **environ;//外部全局变量环境指针
void itoa_my(int i, char* string)
{
    int power=1;
    if (i<0) {
        *string++ = '-';
        i=-i;
    }
    int j=i;
    for(power=1;j>=10;j/=10)
        power*=10;
    
    for(;power>0;power/=10)
    {
        *(string++)='0'+i/power;
        i%=power;
    }
    *string=0;
}
//内部指令函数实现：
int ClrCmd()//clear指令：清屏
{
    printf("\033[2J\033[0;0H");//\033[2J是清屏 \033[0;0H是到将光标置顶  组合实现清屏命令
    return true;
}

int TimeCmd()//time指令：显示当前时间
{
    time_t t;
    struct tm * tm_struct;
    time (&t);//获取Unix时间戳。
    tm_struct = localtime (&t);//转为时间结构。
    //输出结果 如：2017/8/1 12:52:12
    printf ( "%d/%d/%d %d:%d:%d",tm_struct->tm_year+1900, tm_struct->tm_mon+1, tm_struct->tm_mday, tm_struct->tm_hour, tm_struct->tm_min, tm_struct->tm_sec);
    return true;
}

int QuitCmd(){//quit指令：退出
    
    return QUIT;
}

int HelpCmd()//help指令：打印用户手册
{
    FILE *fp = NULL;//文件指针
    char c;
    int len = strlen(ShellPath)-7;
    char pa[1000]={0};
    for(int i =0;i<len;i++){
        
        pa[i]=ShellPath[i];
    }
    char *r = "readme";
    strcat(ReadMePath,pa);
    strcat(ReadMePath,r);//字符串拼接 获得readme路径
    char *readme_path=ReadMePath;//设置帮助文档路径
    printf("%s\n",ReadMePath);
    if((fp=fopen(readme_path,"r"))==NULL){//打开用户手册
        printf("Open readme file failure!\n");//fp=NULL,打开失败,打印错误信息
        return ERROR;
    }else{
        while((c=fgetc(fp))!=EOF){//打印文本内容
            printf("%c",c);
        }
        
        fclose(fp);//关闭文件
        fp = NULL;
    }
    return true;
}

int CdCmd(char * target_path){//cd 指令 进入target_path目录
    if (target_path==NULL){
        PwdCmd();
        return true;
    }
    errno=0;//来自<errno.h>中的外部变量，初始值为0；存储运行chdir的错误代码
    chdir (target_path);
    if (errno==2){ //errno=2 表明该目录不存在，打印错误信息
        printf("cd: no such file or directory: %s",target_path );
        return ERROR;
    }
    
    return true;
}

int PwdCmd()//pwd指令：显示当前工作目录
{	//直接打印的版本：
    printf("%s",getcwd(NULL,0));
    // 设置当前目录缓冲区的版本：
    // char cur_path[1024];
    // getcwd(cur_path, 1024);
    // printf("%s\n", cur_path);
    return true;
}
int DirCmd(char* target_path)//dir指令：列出目录<target_path>的内容
{
    //关于目录的操作参考了博客：http://www.cnblogs.com/onlycxue/p/3154341.html
    DIR *dir = NULL;//目录指针
    
    struct dirent *filename=NULL;//目录结构指针
    dir = opendir(target_path);//打开相应的路径
    if(dir != NULL) {//当前目录下有文件
        
        while(true){
            
            filename = readdir(dir);
            if(filename == NULL)//已经没有文件可供输出
                break;
            
            else if(filename->d_name[0] == '.'||filename->d_name[0] == '..')//不打印.. 和.
                continue;
            else printf("%-15s", filename->d_name);
        }
        closedir(dir);//关闭打开的目录
    }else{
        printf("dir: Open directory error!\n");//出错信息
        return ERROR;
    }
    
    return true;
}

int EnvironCmd()//environ命令：打印目前的环境变量
{
    char **env = environ;//直接循环打印
    while(*env){
        
        printf("%s\n", *env++);
    }
    return true;
}

int EchoCmd(char *comment)//echo命令：打印给定字符串
{
    int length=strlen(comment);
    //char* result=(char *)malloc()
    
    char* value_out=(char*)malloc(sizeof(char)*100);
    memset(value_out, 0, 100);
    char *cur_pos = strchr(comment,'$');
    if (cur_pos!=NULL){//遇到$,检查是否已声明相应变量，对其值进行展开
        //支持 “echo 第一个变量:$1” 这样的显示
        cur_pos++;//cur现在指向变量名指针
        ptr_var_node cur_node = myshell_var_list;
        int flag=0;//表明是否找到了相应变量
        while (cur_node!=NULL){
            if (strcmp(cur_pos,cur_node->name)==0){//找到了相应变量
                
                flag=1;//可改进：变量名之后有空格的情况的处理
                if (cur_node->value==NULL){
                    //变量无值输出空
                    break;
                }
                else{
                    strcpy(value_out, cur_node->value);//字符展开
                    break;
                }
                
            }
            cur_node=cur_node->next;
        }
    }
    //打印结果
    for (int i = 0; i < length; i++){
        if (comment[i]=='"'&&i==0)	continue;//过滤开头和结尾的引号
        if (comment[i]=='"'&&i==length-1)	continue;
        if (comment[i]=='$') {//处理遇到变量的情况
            printf("%s",value_out);
            break;
        }
        else printf("%c",comment[i]);
    }
    return true;
}
int ExitCmd(){//exit命令：退出shell
    return EXIT;
}
int JobsCmd()//jobs命令：列出在后台运行的程序
{
    for(int i = 0; i < BgProgCount && i<MaxBgProgramNum; ++i){
        if (BgProgTable[i].state!=unused){
            
            printf("%d %d %s", BgProgTable[i].pid,BgProgTable[i].state,BgProgTable[i].name);
        }
    }
    return true;
}

int GetOp(ptrCmd cmd_in){//解析test命令输入的参数，返回相应操作
    //各参数详细注释函数
    if(strcmp(cmd_in->parameter_0[2], "-eq") == 0) return 1;
    if(strcmp(cmd_in->parameter_0[2], "-ne") == 0) return 2;
    if(strcmp(cmd_in->parameter_0[2], "-ge") == 0) return 3;
    if(strcmp(cmd_in->parameter_0[2], "-gt") == 0) return 4;
    if(strcmp(cmd_in->parameter_0[2], "-le") == 0) return 5;
    if(strcmp(cmd_in->parameter_0[2], "-lt") == 0) return 6;
    if(strcmp(cmd_in->parameter_0[1], "-n") == 0)  return 7;
    if(strcmp(cmd_in->parameter_0[1], "-z") == 0)  return 8;
    if(strcmp(cmd_in->parameter_0[2], "=") == 0)   return 9;
    if(strcmp(cmd_in->parameter_0[2], "!=") == 0)  return 10;
    if(strcmp(cmd_in->parameter_0[1], "-f") == 0)  return 11;
    if(strcmp(cmd_in->parameter_0[1], "-r") == 0)  return 12;
    if(strcmp(cmd_in->parameter_0[1], "-w") == 0)  return 13;
    if(strcmp(cmd_in->parameter_0[1], "-x") == 0)  return 14;
    else return ERROR;
    
}
int TestCmd(ptrCmd cmd_in)//test命令：测试表达式的真伪
{
    int operation=GetOp(cmd_in);
    int int1,int2;
    int1=int2=0;
    if (cmd_in->parameter_0[1]!=NULL&&cmd_in->parameter_0[3]!=NULL){
        int1 = atoi(cmd_in->parameter_0[1]);//整数比较
        int2 = atoi(cmd_in->parameter_0[3]);
    }
    
    switch (operation) {
            //整数比较
        case 1://-eq
            return int1 == int2; //test 整数1 –eq 整数2  整数相等返回为真,否则为假
            break;
        case 2://ne
            return int1 != int2; //test 整数1 –ne 整数2  int1!=int2返回为真,否则为假
            break;
        case 3://ge
            return int1 >= int2;//test 整数1 –ge 整数2  int1 >= int2返回为真,否则为假
            break;
        case 4://gt
            return int1 > int2;//test 整数1 –gt 整数2 int1 > int2返回为真,否则为假
            break;
        case 5://le
            return int1 <= int2;//test 整数1 –le 整数2 int1 <= int2返回为真,否则为假
            break;
        case 6://lt
            return int1 < int2;//test 整数1 –lt 整数2  int1 < int2返回为真,否则为假
            break;
            //字符串比较
        case 7://-n
            return strlen(cmd_in->parameter_0[2]) != 0;// test -n string 字符串长度非0返回为真,否则为假
            break;
        case 8://-z
            return strlen(cmd_in->parameter_0[2]) == 0;//test -z string 字符串的长度为零返回为真,否则为假
            break;
        case 9://=
            return strcmp(cmd_in->parameter_0[1], cmd_in->parameter_0[3]) == 0;//test 字符串1＝字符串2      字符串相等返回为真,否则为假
            break;
        case 10://!=
            return strcmp(cmd_in->parameter_0[1], cmd_in->parameter_0[3]) != 0;//test 字符串1！＝字符串2    字符串不等返回为真,否则为假
            break;
        case 11://–f
            return (access(cmd_in->parameter_0[2],F_OK) == 0);//test –e File  文件存在并且是正规文件返回为真
            break;
        case 12://test –r File
            return access(cmd_in->parameter_0[2],R_OK) == 0;//test –r File  文件存在并且可读返回为真
            break;
        case 13://test –w File
            return access(cmd_in->parameter_0[2],W_OK) == 0;//test –w File  文件存在并且可写返回为真
            break;
        case 14://test –x File
            return access(cmd_in->parameter_0[2],X_OK) == 0;//test –x File  文件存在并且可执行返回为真
            break;
            
        default:
            return ERROR;
            break;
    }
}

int UmaskCmd(ptrCmd cmd_in)//Umask命令：修改系统掩码值
{
    if(cmd_in->parameter_0[1] != NULL){//用户处于设置状态
        mode_t mask =(mode_t)atoi(cmd_in->parameter_0[1]);//mode_t其实就是unsigned int
        umask(mask);
        printf("%-4.o", mask);//打印umask值
        return true;
    }
    else{
        mode_t mask = umask(S_IRUSR | S_IWUSR | S_IXUSR);//权限：用户读写执行
        umask(mask);
        printf("%-4.o", mask);//打印umask值
        return true;
    }
}
int SetCmd(ptrCmd cmd_in)//set命令：打印或者设置环境变量
{
    if(cmd_in->parameter_0[1]==0 ){//不加参数 查看shell变量 该功能的实现不完全，这里仅打印shell中自己定义的最后一个变量
        
        if (var_list_rear!=NULL&&strcmp(var_list_rear->name, "9") != 0){
            printf("%s: %s",var_list_rear->name,var_list_rear->value );//打印shell中自己定义的最后一个变量信息
        }
        return true;
    }
    else{
        //有参数 插入变量
        ptr_var_node cur = var_list_rear;//在链表尾创建并插入新节点
        ptr_var_node tmp = malloc(sizeof(struct variable_node));
        memset(tmp->name, 0, MaxVariableNameLength);
        cur->next = tmp;
        var_list_rear = tmp;//更新rear节点
        return true;
    }
}
int UnsetCmd(ptrCmd cmd_in)//unset命令：删除现有的变量（支持从链表删除自定义变量）
{
    
    ptr_var_node cur = myshell_var_list->next;//指向第一个shell变量
    ptr_var_node pre = myshell_var_list;//前驱节点
    
    while(cur != NULL){//遍历链表
        
        if(strcmp(cur->name, cmd_in->parameter_0[1]) == 0) {//存在自定义的环境变量
            ptr_var_node tmp = cur;
            pre->next = cur->next;//从链表中移除该节点
            cur = cur->next;
            
            if(tmp == var_list_rear)//如果要删除尾部节点
            {
                var_list_rear = pre;//调整全局的var_list_rear
            }
            free(tmp->value);//把节点值的空间释放掉
            free(tmp);//释放节点空间
            break; //完成链表里的删除
        }
        pre = pre->next;//检查下一个结点
        cur = cur->next;//bug
    }
    
    return true;
}

void Init(){
    //进行myshell的初始化
    //change 变量管理可以用数组？
    //readlink()会将参数path的符号链接内容存储到参数buf所指的内存空间，返回字符串的字符数
    //proc/self/exe 它代表当前程序，所以可以用readlink读取它的源路径就可以获取当前程序的绝对路径
    readlink("/proc/self/exe", ShellPath, 1000); //读取当前myshell所处的路径
    chdir(ShellPath);//设置默认初始工作目录
    setenv("SHELL", ShellPath, 1);//改变或增加环境变量 第三个参数为1，表示会用ShellPath覆盖原来的SHELL值 bug
    myshell_var_list = malloc(sizeof(struct variable_node));
    var_list cur = myshell_var_list;
    
    for (int i = 0; i <= 9; ++i){//模仿Bash定义的9个位置变量 第0个节点为dummy head $? 1-9对应$1-$9变量//bug
        memset(cur->name, 0, MaxVariableNameLength);
        itoa_my(i,cur->name);//将数字转换成字符串,作为变量名
        cur->value = NULL;
        cur->next = malloc(sizeof(struct variable_node));//分配内存
        cur = cur->next;
    }
    
    cur->next = NULL;
    var_list_rear = cur;//初始化rear指针
    for (int i = 0; i < MaxBgProgramNum; ++i){//初始化bgtable
        for (int j=0; j<MaxProgramNameLength; j++) {
            BgProgTable[i].name[j]=0;
            BgProgTable[i].parameter[j]=0;
        }
        
        BgProgTable[i].state=unused;
    }
    //fflush(stdout);//强制刷新标准输出 清空输出缓冲区，并把缓冲区内容输出 可以试试不加 这个是防止打印出错 bug
}

int ShiftCmd(ptrCmd cmd_in){//shift操作:左移
    
    int shiftNum;//左移位数
    if (cmd_in->parameter_0[1] == NULL){
        shiftNum=1;//默认为1
    }
    else shiftNum=atoi(cmd_in->parameter_0[1]);
    
    ptr_var_node tmp=NULL;
    for(int i = 0; i < shiftNum; i++){
        tmp=myshell_var_list->next;
        myshell_var_list->next = myshell_var_list->next->next;//直接移动dummy head
        free(tmp->value);//把值给释放
        free(tmp);
    }
    ptr_var_node cur = NULL;
    cur = myshell_var_list->next;//cur指向实际的头结点，即dummy head的下一个节点
    for(int i = 0; i < 9-shiftNum; i++){
        itoa_my(i+1,cur->name);
        cur = cur->next;
    }//跳出循环之后，cur指向null或者第一个自定义变量
    ptr_var_node tmp2=cur;//tmp2指向第一个自定义变量或null
    
    for (int i = 0; i < shiftNum; i++){//恢复为9个位置变量 //bug
        
        //构造一个新节点
        ptr_var_node newnode = (ptr_var_node)malloc(sizeof(struct variable_node));
        memset(newnode->name, 0, MaxVariableNameLength);
        itoa_my(i+10-shiftNum,newnode->name);//将数字转换成字符串,作为变量名
        newnode->value = NULL;
        cur->next=newnode;
        cur = cur->next;
    }
    cur->next=tmp2;//让新恢复的第9个参数节点指向第一个自定义变量节点
    return true;
    
}

int Assignment(char* cmd_string){//赋值操作
    char *fir_pos = strchr(cmd_string, '=');
    int length = strlen(cmd_string)-strlen(fir_pos);
    char *new_var_name = malloc(sizeof(char) * ( length+ 1));//+1是为了'\0'
    memset(new_var_name, 0, length + 1);
    strncpy(new_var_name, cmd_string, strlen(cmd_string)-strlen(fir_pos));//获得x=1中的变量名x
    
    char* cur_pos = ++fir_pos;//让当前指针指向=后面的字符 即变量的值
    char *new_var_value = malloc(sizeof(char) * strlen(cur_pos));
    memset(new_var_value, 0, strlen(cur_pos));
    strcpy(new_var_value, cur_pos);//提取变量值
    
    ptr_var_node cur = myshell_var_list->next;
    while(cur){//循环，遍历是否有结点与欲插入值相同
        
        if(strcmp(cur->name, new_var_name) == 0){//当前变量已经存在
            
            free(cur->value);
            cur->value = malloc(sizeof(char) * strlen(new_var_value));//为新值分配空间
            strcpy(cur->value, new_var_value);//赋新值
            return true;//返回成功
        }
        cur = cur->next;
    }
    //没有直接返回，说明是新变量
    var_list_rear->next = malloc(sizeof(struct variable_node));//创建新的结点
    var_list_rear = var_list_rear->next;
    strcpy(var_list_rear->name, new_var_name);//写入名字
    var_list_rear->value = malloc(sizeof(char) * strlen(new_var_value));//为值分配空间
    strcpy(var_list_rear->value, new_var_value);//写入值
    free(new_var_name);//释放空间
    free(new_var_value);
    return true;//创建新节点成功
}
int GetInternalCmd(char* cmd_string){//获取内部命令值 注释见下文
    if(strcmp(cmd_string, "cd") == 0) return 1;
    if(strcmp(cmd_string, "clr") == 0) return 2;
    if(strcmp(cmd_string, "dir") == 0) return 3;
    if(strcmp(cmd_string, "echo") == 0) return 4;
    if(strcmp(cmd_string, "environ") == 0) return 5;
    if(strcmp(cmd_string, "exit") == 0) return 6;
    if(strcmp(cmd_string, "exec") == 0) return 7;
    if(strcmp(cmd_string, "help") == 0) return 8;
    if(strcmp(cmd_string, "jobs") == 0)  return 9;
    if(strcmp(cmd_string, "pwd") == 0)  return 10;
    if(strcmp(cmd_string, "quit") == 0)  return 11;
    if(strcmp(cmd_string, "set") == 0)  return 12;
    if(strcmp(cmd_string, "shift") == 0)  return 13;
    if(strcmp(cmd_string, "time") == 0)  return 14;
    if(strcmp(cmd_string, "test") == 0)  return 15;
    if(strcmp(cmd_string, "umask") == 0)  return 16;
    if(strcmp(cmd_string, "unset") == 0)  return 17;
    else return ERROR;
}
int InternalCmd(ptrCmd cmd_in)
{
    char *fir_pos = strchr(cmd_in->parameter_0[0], '=');//判断是否为赋值操作
    if(fir_pos  != NULL) {//变量赋值操作
        Assignment(cmd_in->parameter_0[0]);
        return true;
    }
    else{
        int cmd_op=GetInternalCmd(cmd_in->parameter_0[0]);//获得操作类型
        switch (cmd_op) {
            case 1://切换工作目录
                return CdCmd(cmd_in->parameter_0[1]);
                break;
            case 2://清屏
                return ClrCmd();
                break;
            case 3://列出当前工作目录下文件
                return DirCmd(cmd_in->parameter_0[1]);
                break;
            case 4://打印字符串
                EchoCmd(cmd_in->parameter_0[1]);
                break;
            case 5://显示环境变量
                EnvironCmd();
                break;
            case 6://退出shell
                return EXIT;
                break;
            case 7://执行外部程序
                execvp(cmd_in->parameter_0[1], NULL);
                return EXIT;
                break;
            case 8://显示帮助手册
                return HelpCmd();
                break;
            case 9://显示后台程序
                return JobsCmd();
                break;
            case 10://显示当前工作目录
                return PwdCmd();
                break;
            case 11://退出
                return QUIT;
                break;
            case 12://设置shell变量
                return SetCmd(cmd_in);//可以改
                break;
            case 13://对参数的移动(左移)
                return ShiftCmd(cmd_in);
                break;
            case 14://显示时间
                return TimeCmd();
                break;
            case 15:{//判断表达式真假
                int res = TestCmd(cmd_in);
                printf("test reslut is: %d", res);
                return true;
                break;
            }
            case 16://掩码操作
                return	UmaskCmd(cmd_in);
                break;
            case 17://删除shell变量
                return UnsetCmd(cmd_in);
                break;
            default:
                break;
        }
        
    }
    return 1;
}
char* GetWord(char* s, int* cur){//分词函数
    char* word;
    size_t begin,end;
    
    while ((s[*cur] == ' '||s[*cur]=='\t')&&s[*cur]!=0){
        (*cur)++;
    }//忽略空格 制表符
    begin = *cur;//词头
    
    while (s[*cur]!=' '&&s[*cur]!='\n'&&s[*cur]!=0) {
        (*cur)++;
    }
    end = *cur;//词尾
    if (begin < end) {
        word=(char *)malloc(sizeof(char)*(end-begin+1));//提取
        strncpy(word,s+begin,end-begin);
        word[end-begin] =0;
    }
    else word = NULL;
    return word;//返回
    
}

int SetCmdIn(ptrCmd cmd_in) {//处理用户输入的指令，设置cmd_in内容，便于后续操作
    
    
    char *begin = cmd_in->content;
    
    int p_count = 0;
    int cur=0;
    fgets(cmd_in->content,MaxLine , stdin);//从标准输入读取一行 注意fgets读入换行符到数组中
    //if (fgets(info->prompt, MAX_LINE, stdin)==NULL) return 99;
    if (feof(stdin)!=0)//读入脚本文件终止符
    {
        return ShellScript;
    }
    while(true){
        
        if(begin[cur] == '\n' ){//换行符结束处理
            if(p_count == 0) return -1;//-1：无参数
            break;
        }
        
        if(p_count == 0) {//第一个参数
            
            cmd_in->command_0 = GetWord(cmd_in->content,&cur);
            cmd_in->parameter_0[0] =cmd_in->command_0 ;
            p_count++;
        }
        else if(p_count <= MaxArgNum){//没有超过最大参数数目
            
            cmd_in->parameter_0[p_count] = GetWord(cmd_in->content,&cur);
            p_count++;
        }
        else break;
        
    }
    cmd_in->num = p_count;
    return p_count;
}
void CmdInit(ptrCmd cmd_in)//将语义分析结构体进行重新初始化，为接收下一条指令做准备
{
    cmd_in->type = 0;
    cmd_in->num = 0;
    cmd_in->command_0 = NULL;
    cmd_in->command_1 = NULL;
    cmd_in->parameter_1 = NULL;//改
    cmd_in->content = (char*)malloc(sizeof(char) * MaxLine);
    cmd_in->parameter_0 = (char**)malloc(sizeof(char*)*(MaxArgNum + 2));
}
void PrintPrompt(char *argv[]){//打印命令提示符
    
    if(argv[1] && strcmp(argv[1], "1") == 0)//是否为脚本模式
        return;//是脚本则直接返回
    //参考：http://blog.csdn.net/small_qch/article/details/9198833
    int cnt = readlink("/proc/self/exe", ShellPath, 1000);//获取当前myshell的脚本地址
    if(cnt < 0 || cnt >= 1000){//地址过长或者获取失败
        printf("Open MyShell fail!");
        exit(-1);//返回
    }
    
    char buf[100];
    getcwd(buf, 100);    //获得当前的工作目录
    printf("\nMyShell:%s/$ ", buf);
}

int GrammarAnylysis(ptrCmd cmd_in){
    int i = 0;
    while (i < cmd_in->num ){
        char *cur_pos = strchr(cmd_in->parameter_0[i],'$');
        if (cur_pos!=NULL) {//遇到$,检查是否已声明相应变量，对其值进行展开
            
            cur_pos++;//cur现在指向变量名指针
            ptr_var_node cur_node=myshell_var_list->next;
            int flag=0;//表明是否找到了相应变量
            while (cur_node!=NULL){
                if (strcmp(cur_pos,cur_node->name)==0){//找到了相应变量
                    
                    flag=1;
                    if (cur_node->value==NULL){
                        cmd_in->parameter_0[i] = malloc(sizeof(char));
                        *(cmd_in->parameter_0[i]) = '\0';//变量无值输出空
                        break;
                    }
                    else{
                        cmd_in->parameter_0[i]=malloc(sizeof(char) * strlen(cur_node->value));//可能会导致内存泄漏
                        
                        strcpy(cmd_in->parameter_0[i], cur_node->value);//字符展开
                        break;
                    }
                    
                }
                cur_node=cur_node->next;
            }
            if (flag==0){//没找到
                
                cmd_in->parameter_0[i] = malloc(sizeof(char));
                *(cmd_in->parameter_0[i]) = '\0';//无该变量 输出空
            }
        }
        
        //输出重定向
        if(strcmp(cmd_in->parameter_0[i], ">") == 0 ||strcmp(cmd_in->parameter_0[i], ">>") == 0){
            
            if(strcmp(cmd_in->parameter_0[i], ">") == 0)
                cmd_in->type = RedirectOverwrite;//bug
            if(strcmp(cmd_in->parameter_0[i], ">>") == 0)
                cmd_in->type = RedirectAddition;//bug
            
            cmd_in->filename = cmd_in->parameter_0[i+1]; //标识重定向后的目标输出文件
            if(strcmp(cmd_in->filename,"1")==0) //标准输出
                cmd_in->fd = StdandardOut;
            
            cmd_in->parameter_0[i] = NULL;
            i += 2;
        }
        
        else if(strcmp(cmd_in->parameter_0[i], "&") == 0) { //打上后台运行的标记
            
            cmd_in->type= BgMark;
            
            cmd_in->parameter_0[i] = NULL;
            i++;
            strcpy(BgProgTable[BgProgCount++].name, cmd_in->command_0);
        }
        else if(strcmp(cmd_in->parameter_0[i], "|") == 0){ //管道指令
            
            cmd_in->type = PIPE_TYPE;
            cmd_in->command_1 = cmd_in->parameter_0[i+1];    //获取管道后的指令
            cmd_in->parameter_1 = &(cmd_in->parameter_0[i + 1]);  //获取管道后的指令的第一个参数
            cmd_in->parameter_0[i] = NULL;
            i += 2;
        }
        else if(strcmp(cmd_in->parameter_0[i], "fg") == 0){//前台运行命令
            
            cmd_in->type= FgMark;//打上前台运行标记
            if (cmd_in->parameter_0[i+1]!=NULL)
            {
                int j_id = atoi(cmd_in->parameter_0[i+1]);
                if (j_id<=BgProgCount)
                {
                    cmd_in->command_0 = BgProgTable[j_id].name;//运行后台命令命令
                    cmd_in->parameter_0 = &(BgProgTable[BgProgCount].parameter);
                }
            }
            else {
                int j_id = BgProgCount;//无参数时，默认运行最后挂起的命令
                cmd_in->command_0 = BgProgTable[j_id].name;//运行后台命令命令
                cmd_in->parameter_0 = &(BgProgTable[BgProgCount].parameter);
                
            }
            
            --(cmd_in->num);//fg不算在主体命令的分词中
            i++;
        }
        else if(strcmp(cmd_in->parameter_0[i], "bg") == 0)//后台运行命令的标记
        {
            cmd_in->type= BgMark;
            cmd_in->command_0 = cmd_in->parameter_0[i+1];//重新指向命令
            cmd_in->parameter_0 = &(cmd_in->parameter_0[i+1]);
            cmd_in->num--;
            i++;
            strcpy(BgProgTable[BgProgCount].name, cmd_in->command_0);
            strcpy(BgProgTable[BgProgCount].parameter, *cmd_in->parameter_0);//记录下后台命令
            BgProgTable[BgProgCount].pid=BgProgCount;
            BgProgTable[BgProgCount].state=running;
            BgProgCount++;
        }
        else
            i++;//不需要处理
        
    }
    return true;
}



int Shell(ptrCmd cmd_in,int argc , char* argv[] ){
    int pipe_fd[2] = { 0, 0 };
    int status = 0;
    pid_t childPID = 0;
    int  re_out_fd;
    
    int mode = SetCmdIn(cmd_in);//读取指令并进行预处理
    if (mode == ShellScript) {
        return ShellScript;
    }
    if(cmd_in->num==-1)  return 0;
    GrammarAnylysis(cmd_in);//对指令进行语义分析
    if(strcmp(cmd_in->parameter_0[0], "MyShell") == 0){
        cmd_in->type = 12;
    }
    if(cmd_in->type == 0){//内部指令
        
        int state=InternalCmd(cmd_in);
        
        if(state==QUIT) return QUIT;
        CmdInit(cmd_in);//初始化结构体
        if(getpid() != 0){
            PrintPrompt(argv);
            
        }
        
        if (state==ERROR) {
            return ERROR;
        }else return true;
    }
    //运行脚本
    else if(strcmp(cmd_in->parameter_0[0], "MyShell") == 0){
        int script_pid;
        script_pid = fork();
        if(script_pid < 0){//创建新的进程以来执行脚本
            printf("Create Shell Script Process Failure!");
            return ERROR;
        }
        else if(script_pid == 0){ // 脚本子进程
            int re_in_fd = 0;
            re_in_fd = open(cmd_in->parameter_0[1], O_RDONLY | O_CREAT, 0777);
            dup2(re_in_fd,fileno(stdin));//将输入重定向为脚本
            char *shell_argvs[100] = { NULL };
            
            shell_argvs[0] = ShellPath;
            execvp(ShellPath, shell_argvs);//调用execvp函数，运行子shell
            exit(0);//结束子进程
            
        }
        else{//父进程
            
            waitpid(script_pid, NULL, 0);//等待子进程返回
            PrintPrompt(argv);//打印新的命令提示符
            CmdInit(cmd_in);//初始化结构体
            return true;
        }
    }

    else if(cmd_in->type == PIPE_TYPE){
        if(pipe(pipe_fd) < 0){//创建管道，为下面新建子进程做准备
            
            perror("Create pipe Failure!");
            return ERROR;
        }
    }//else 即为其他重定向等操作
    
    if((childPID = fork()) < 0){
        perror("Create child process Failure!");
        return ERROR;
    }
    else if(childPID == 0){//子进程
        
        if(cmd_in->type == PIPE_TYPE){//管道操作
            
            close(fileno(stdout));//关闭标准输出
            close(pipe_fd[0]);//关闭管道读
            
            dup2(pipe_fd[1], fileno(stdout)); //重定向输出，将结果写入管道
            //write(pipe_fd[1], buf, 100);
            close(pipe_fd[1]);//关闭管道写
            
        }
        else {// 不存在管道操作
            
            if(cmd_in->type== RedirectOverwrite){ // 重定向输出为>
                
                re_out_fd = open(cmd_in->filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                //打开输出文件，打开方式为只写，不存在时创建，覆盖写，创建新文件的时候权限为777
                close(fileno(stdout));
                dup2(re_out_fd, fileno(stdout));//将原先与标准输出对应的接口接到管道的输入端上
                close(re_out_fd);
            }
            else if(cmd_in->type == RedirectAddition){//重定向为>>
                re_out_fd = open(cmd_in->filename, O_WRONLY | O_CREAT | O_APPEND, 0777);
                //打开输出文件，打开方式为只写，不存在时创建，添加写，创建新文件的时候权限为777
                close(fileno(stdout));
                dup2(re_out_fd, fileno(stdout));//将原先与标准输出对应的接口接到管道的输入端上
                close(re_out_fd);
            }
        }
        sleep(0.5);
        
        InternalCmd(cmd_in);//设置好全部的重定向管道信息以后执行相关的指令
        exit(0);
    }
    else//父进程
    {
        //注：管道的实现参考了源代码：https://github.com/KagerouClass/
        int pipeChildPID = 0;
        int i = 0;
        if(cmd_in->type==PIPE_TYPE)//存在管道指令，将开新的子进程接收来自管道的数据
        {
            if((pipeChildPID = fork()) < 0)
            {
                perror("Create child process Failure!");
            }
            else if (pipeChildPID == 0)//子进程，接收管道数据
            {
                close(pipe_fd[1]);//关闭管道写
                close(fileno(stdin));
             
                if(cmd_in->type == BgMark)//存在后台运行
                {
                    PrintPrompt(argv);
                
                    fflush(stdout);
                    
                }
                dup2(pipe_fd[0], fileno(stdin));//将原本与标准输入相对应的接口转接到管道的出口上
                char buf_r[100]={0};
                read(pipe_fd[0], buf_r, 100);
                close(pipe_fd[0]);
                char *shell_argvs[100] = { NULL };
                int i = 0;
                while (cmd_in->parameter_1[i])
                {
                    shell_argvs[i] = cmd_in->parameter_1[i];
                    i++;
                }
                shell_argvs[i] = buf_r;
                
                i = 0;
                
                execvp(shell_argvs[0], shell_argvs);//执行外部程序
                exit(0);
            }
            else
            {
                waitpid(pipeChildPID, &status, 0);//等待管道信息接收并执行完毕
            }
        }
        if (cmd_in->type == BgMark) {//如果是后台运行的进程，无需等待其结束即可进入下一指令的接收
            PrintPrompt(argv);
            fflush(stdout);
            CmdInit(cmd_in);//初始化结构体
            return true;
        }
        else if (cmd_in->type == FgMark)//理由同上
        {
            CmdInit(cmd_in);//初始化结构体
            return true;
        }
        else
        {
            waitpid(childPID, NULL, 0);//等待子进程结束才能进行下一步操作
            PrintPrompt(argv);
            CmdInit(cmd_in);//初始化结构体
            return true;
        }
    }
}
