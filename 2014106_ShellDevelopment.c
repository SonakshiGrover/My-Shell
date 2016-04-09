
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include<stdio.h>
#include <signal.h>
#include <fcntl.h>
 

FILE *fp;
int cnt;
int num=4;

int mycd(char **arg)
{
      if (arg[1] == NULL) 
          fprintf(stderr,"give argument to cd\n");
      else 
      {
        if (chdir(arg[1]) != 0) 
          perror("Error : ");
        
      }
      return 1;
}


int myhelp(char **arg)
{
     
      printf("\n\nMY SHELL\n");
      printf("This shell has following built in commands:\n");
      printf("1. help(displays the built in commands\n2.history (to display all the previous commands) \n3. history n (for displaying last 'n' commands ) \n4. cd (for changing directory)  \n5. exit(to exit out of the terminal)\n6. piping \n7. redirection \n8. ctrl + c \n\n");
      
      return 1;
}


int myexit(char **arg)
{
    return 0;
}


int myhistory(char **arg)
{
   int n,i;
   if(arg[1]!=NULL)
       n=arg[1][0]-48;    
    //printf("n= %d\n cnt =%d\n",n,cnt);
    char str[100];
    fp=fopen("myhistory.txt","r");
    i=0;
    while(fgets (str, sizeof(str), fp))
    {
       if(arg[1]!=NULL)
       {  
          //printf("i=%d cnt-n= %d\n",i,(cnt-n));

          if(i>=(cnt-n))
           printf("%s",str);
       } 
       else
           printf("%s",str);
  
       i++;
    }
     
    fclose(fp);
    return 1;
}

int redirect(char *arg1,char *arg2)
{
 
   int rc = fork();

   if (rc < 0) 
   {          

      fprintf(stderr, "fork failed\n");

       exit(1);

   }
   else if (rc == 0) 
   { 

    close(STDOUT_FILENO);

    open(arg2, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
    

    char *myargs[2];

    myargs[0] = strdup(arg1);   
    myargs[1] = NULL;           
    execvp(myargs[0], myargs);  

   } 
   else
   {              
    void myloop(void);
    int wc = wait(NULL);
    myloop();
   }

    return 0;

}

int mypipe(char *arg1,char *arg2)
{
    //printf(" mypipe has entered \n");
    pid_t childpid;
    
    childpid=fork();
    if (childpid == -1)
    {
        perror("Error");
        exit(1);
    }
    if (childpid)   
    {
        void myloop(void);
      //  printf(" grand parent \n");
        wait(&childpid);       
      // printf(" \n\ngrand parent after wait \n");    
       myloop();    
    }
    if (childpid==0)  
    {
         int fd[2];
         pipe(fd);
    
        //printf("parent\n");  
        pid_t grandchildpid;
        grandchildpid=fork();
       if (grandchildpid == -1)
      {
        perror("Error");
        exit(1);
      }
        if (grandchildpid)   
      {
        //printf("parent again after fork\n");

        wait(&grandchildpid); 
        //printf(" parent after wait\n");      
        close(fd[1]);
        close(0);       
        dup2(fd[0],0);
        execlp(arg2,arg2,NULL);

      }
      if (grandchildpid==0)  
      {//printf("grandchild\n");

        close(fd[0]);  
        close(1);      
        dup2(fd[1],1);
        execlp(arg1,arg1,NULL);
      }



        /*close(fd[0]);  
        close(1);      
        dup2(fd[1],1);
        execlp(arg1,arg1,NULL);
         */
    }
    return 1;

}



int mylaunchproc(char **arg)
{
     
    pid_t proc_id;
    int stat;
    proc_id = fork();
         
    if (proc_id == 0) 
    {
   
         if (execvp(arg[0], arg) == -1)
         {
             perror("command not found or ");
         }
   
         exit(EXIT_FAILURE);
    }
    else if (proc_id < 0) 
    {
        
         perror("error ");
    }
    else 
    {
        do 
        {
           waitpid(proc_id, &stat, WUNTRACED);
        } while (!WIFEXITED(stat) && !WIFSIGNALED(stat));  
    }
    
    return 1;
}


int myexecute(char **arg)
{
      // printf("\n\n inside myexecute\n");
      


       int i;
       if (arg[0] == NULL)
       {
              
             return 1;
       }
       for (i = 0; i < num; i++) 
       {
           if((arg[1]!=NULL) &&strcmp(arg[1],">")==0)
           {
                
             return redirect(arg[0],arg[2]);
           } 
           else
           if((arg[1]!=NULL) &&strcmp(arg[1],"|")==0)
           {
                
             return mypipe(arg[0],arg[2]);
           } 
           else
           if(strcmp(arg[0],"cd")==0)
              {   
             return mycd(arg);}
           else
           if(strcmp(arg[0],"help")==0) 
           {
              
             return myhelp(arg);
           }
           else
           if(strcmp(arg[0],"exit")==0)
               return myexit(arg);
           else
           if(strcmp(arg[0],"history")==0)
               return myhistory(arg);
           
       }
       return mylaunchproc(arg);
}



char **mysplitline(char *line)
{

       // printf("\n\ninside mysplitline\n");
        cnt++;
        fp=fopen("myhistory.txt","a+");
        fprintf(fp,"%d %s",cnt,line);
        fprintf(fp,"\n");
        fclose(fp);
   

        int size = 64, pos = 0;
        char **tokenarr = malloc(size * sizeof(char*));
        char *token, **tokensbackup;
        if (!tokenarr) 
         {
            fprintf(stderr, "Allocation Error\n");
            exit(EXIT_FAILURE);
         }
     
 
                  
         token = strtok(line, " \t\r\n\a");
         while (token != NULL) 
         {
            tokenarr[pos] = token;
            pos++; 
            if (pos >= size) 
            {

               size += 64;
               tokensbackup = tokenarr;
               tokenarr = realloc(tokenarr, size * sizeof(char*));
               if (!tokenarr)
               {
                   free(tokensbackup);
                   fprintf(stderr, "Allocation Error\n");
                   exit(EXIT_FAILURE);
               }
             }
            token = strtok(NULL, " \t\r\n\a");
          }
          tokenarr[pos] = NULL;
          return tokenarr;
}
char *myreadline(void)
{
       int size = 1024;
       int pos = 0;
      // char *buffer = malloc(sizeof(char) * size);
       char *buffer = malloc(sizeof(char) * size);
       memset(buffer,0,1024);
       int d;
       
       if (!buffer)
       {
           fprintf(stderr, "Allocation Error\n");
           exit(EXIT_FAILURE);
       }
       

       while (1)
       {
         d = getchar();
        // if((Control.ModifiersKeys&Keys.Shift)!=0)
         // {  printf("hhhhkkk");
         if (d == '\n'||d == EOF ) 
          {
             buffer[pos] = '\0';
       
             return buffer;
          }
          else 
          {
             buffer[pos] = d;
          }
          pos++;
       
           
          if (pos >= size) 
          {
               size = size + 1024 ;
               buffer = realloc(buffer, size);
               if (buffer==0) 
               {
                  fprintf(stderr, "Allocation Error\n");
                  exit(EXIT_FAILURE);
               }
          }
        }
}
void sigintHandler(int sig_num)
{
    void myloop(void);
    signal(SIGINT, sigintHandler);
    printf("\n");
    myloop();
    //fflush(stdout);
}


void myloop(void)
{
    char *line;
    char **arg;
    int status;
    char buffpwd[PATH_MAX +1];
    signal(SIGINT, sigintHandler);
 
    do
    {
        printf("%s$ ",getcwd(buffpwd,PATH_MAX +1));
       line = myreadline();
       arg = mysplitline(line);
       status = myexecute(arg);
       free(line);
       free(arg);
    }
    while (status);
    if(strcmp(line,"exit")==0)
      exit(0);
}


int main()
{
   cnt=0;

   printf("MY SHELL HAS STARTED\n\n");
    
    
    myloop();      
   return EXIT_SUCCESS;
}


