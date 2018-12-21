//
//  main.c
//  MyShell
//
//  Copyright © 2017年 apple. All rights reserved.
//

#include "MyShell.h"


int main(int argc, char *argv[]){
    
    
    ptrCmd cmd_in = (ptrCmd)malloc(sizeof(command));
    
    //用来存储输入的命令各个parameter存在的地方
    Init();//初始化
    CmdInit(cmd_in);//初始化结构体
    PrintPrompt(argv);
    fflush(stdout);

    while(1){
        int state=Shell(cmd_in,argc,argv);//执行命令
        if( state== QUIT ||state==ERROR||state== ShellScript) break;
    }
    free(cmd_in->parameter_0);//释放指针
    free(cmd_in);
    return 0;
}
