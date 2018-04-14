#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include<stdlib.h>

int main() {
    /* 输入的命令行 */
  //  int z=0;
    int wstatus;
    char cmd[256];
    /* 命令行拆解成的各部分，以空指针结尾 */
    char *args[128];
    int pipe_num=0;
    char wd[4096];
    char *pipe_args[128];
    while (1) {
//        z++;
        int prev = -1;
        pipe_num = 0;
        /* 提示符 */
//        printf("%d# ",z);
        printf("# ");
        fflush(stdin);
        fgets(cmd, 256, stdin);
        /* 清理结尾的换行符 */
        int i;
        int j;
        for (i = 0; cmd[i] != '\n'; i++)
            ;
        cmd[i] = '\0';
        /* 拆解命令行 */
        args[0] = cmd;
        for (i = 0; *args[i]; i++)
            for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++)
                if (*args[i+1] == ' ') {
                    while(*args[i+1] == ' '){
                    *args[i+1] = '\0';
                    args[i+1]++;}
                    break;
                }
        args[i] = NULL;
        for(i=0;args[i]!=NULL;i++) if(strcmp(args[i],"|") == 0 ) pipe_num++;
        /* 没有输入命令 */
        if (!args[0])
            continue;

        /* 内建命令 */
        if (strcmp(args[0], "cd") == 0) {
            if (args[1])
                chdir(args[1]);
            continue;
        }
        if ((strcmp(args[0], "pwd") == 0 )&&(pipe_num == 0)) {
            puts(getcwd(wd, 4096));
            continue;
        }
        if (strcmp(args[0], "exit") == 0)
            return 0;
        if (strcmp(args[0],"export")==0) {
            char a[256];
            char c[256];
            int b=0,d=0;
            if(args[1]==NULL || args[1][0]=='\0')
            {
            continue;
            }
            for(;args[1][b]!='=';b++)
            {
              a[b]=args[1][b];
            }
            a[b]='\0';
            for(b++;args[1][b]!='\0';d++,b++)
            {
              c[d]=args[1][b];
            }
            c[d]='\0';
            setenv(a,c,0);
            continue;
        }
        if (strcmp(args[0],"unset")==0){
            int k;
            if(args[1]==NULL || args[1][0]=='\0'){
              continue;
            }
            else{
              for(k=1;args[k]!=NULL;k++)
              {
                unsetenv(args[k]);
              }
            }
            continue;
        }

        /* 外部命令 */
      if (pipe_num == 0){
          pid_t pid = fork();
          if (pid == 0) {
              /* 子进程 */
              execvp(args[0], args);
              /* execvp失败 */
              return 255;
          }
        /* 父进程 */
//          wait(NULL);
//          fflush(stdout);
//从man waitpid上直接复制下来的
do {
pid_t      w = waitpid(pid, &wstatus, WUNTRACED | WCONTINUED);
           if (w == -1) {
               perror("waitpid");
               exit(EXIT_FAILURE);
           }
/*
           if (WIFEXITED(wstatus)) {
               printf("exited, status=%d\n", WEXITSTATUS(wstatus));
           } else if (WIFSIGNALED(wstatus)) {
               printf("killed by signal %d\n", WTERMSIG(wstatus));
           } else if (WIFSTOPPED(wstatus)) {
               printf("stopped by signal %d\n", WSTOPSIG(wstatus));
           } else if (WIFCONTINUED(wstatus)) {
               printf("continued\n");
           }
*/       } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
      }
      else{
      for(i=0,j=0;i<=pipe_num;i++)
      {
        int con;
        int pipefd[2];
        pid_t cpid;
        for(con=0;(args[j] != NULL);j++,con++)
        {
          if(strcmp(args[j],"|") == 0) break;
          pipe_args[con] = args[j];
        }
        pipe_args[con] = NULL;
        j++;
        if(pipe(pipefd) == -1)
        {
          perror("pipe");
          break;
        }
        //子进程
        if((cpid = fork())==0)
        {
          close(pipefd[0]);//关闭读口
          if(i != pipe_num) //最后一个直接输出
          {
            if(dup2(pipefd[1],STDOUT_FILENO)<0)
            {
            perror("dup out fail");
            exit(0);
            }
            close(pipefd[1]);
            //将输出重定向
          }
          if(i != 0) //第一个不用读前一个的
          {
            if((dup2(prev,STDIN_FILENO))<0)
            {
              perror("dup in fail");
              exit(0);
            }
            close(prev);
          }
          execvp(pipe_args[0],pipe_args);
        }
        //父进程
        close(pipefd[1]);//父进程不用写
        close(prev);
        if(i!=pipe_num){
        prev=pipefd[0];}
      //  else{wait(NULL);fflush(stdout);}
        else{
          do {
              pid_t     w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
                   if (w == -1) {
                       perror("waitpid");
                       exit(EXIT_FAILURE);
                   }

        /*           if (WIFEXITED(wstatus)) {
                       printf("exited, status=%d\n", WEXITSTATUS(wstatus));
                   } else if (WIFSIGNALED(wstatus)) {
                       printf("killed by signal %d\n", WTERMSIG(wstatus));
                   } else if (WIFSTOPPED(wstatus)) {
                       printf("stopped by signal %d\n", WSTOPSIG(wstatus));
                     } else if (WIFCONTINUED(wstatus)) {
                       printf("continued\n");
                     }*/
               } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
        }
      }
      //输出结束
    }
    }
}
