#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define BUFFER_SIZE 50

char *history[50];                      /* global char array to store command history */

/**
 * setup() reads in the next command line, separating it into distinct tokens
 * using whitespace as delimiters. setup() sets the args parameter as a
 * null-terminated string.
 */
 void handle_SIGTSTP() {                    /* signal handler */
      int i = 0;
      printf("\n");
      while(history[i]!=NULL){              /* print out the history of command */
        printf("[%d] %s\n",i,history[i]);
        i++;
      }
      signal(SIGTSTP,handle_SIGTSTP);
      return;
}

int setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */

    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);
    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */
    if (length < 0){
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
    }

    /* examine every character in the inputBuffer */
    for (i=0;i<length;i++) {
        switch (inputBuffer[i]){
          case ' ':
          case '\t' :               /* argument separators */
            if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
                ct++;
            }
            inputBuffer[i] = '\0'; /* add a null char; make a C string */
            start = -1;
            break;
          case '\n':                 /* should be the final char examined */
            if (start != -1){
                    args[ct] = &inputBuffer[start];
                ct++;
            }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
            break;
          default :             /* some other character */
            if (start == -1)
                start = i;
            if (inputBuffer[i] == '&'){
                *background  = 1;
                start = -1;
                inputBuffer[i] = '\0';
            }
          }
     }
     args[ct] = NULL; /* just in case the input line was > 80 */
     return ct;       /* return the number of elements in args[] */
}
/* setup function for built-in command r */
int rsetup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */

    ct = 0;

    length = strlen(inputBuffer);
    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */
    if (length < 0){
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
    }

    /* examine every character in the inputBuffer */
    for (i=0;i<length;i++) {
        switch (inputBuffer[i]){
          case ' ':
          case '\t' :               /* argument separators */
            if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
                ct++;
            }
            inputBuffer[i] = '\0'; /* add a null char; make a C string */
            start = -1;
            break;
          case '\n':                 /* should be the final char examined */
            if (start != -1){
                    args[ct] = &inputBuffer[start];
                ct++;
            }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
            break;
          default :             /* some other character */
            if (start == -1)
                start = i;
            if (inputBuffer[i] == '&'){
                *background  = 1;
                start = -1;
                inputBuffer[i] = '\0';
            }
          }
     }
     args[ct] = NULL; /* just in case the input line was > 80 */
     return ct;       /* return the number of elements in args[] */
}                     /* end of function for r command */

int main(void)
{
    char inputBuffer[MAX_LINE];      /* buffer to hold the command entered */
    int background,pid,i,j,length,rlen;      /* background equals 1 if a command is followed by '&' */
    int status,prompt;
    long num;                       /* used in r command to store args[1] */
    char *args[(MAX_LINE/2)+1];  /* command line (of 80) has max of 40 arguments */
    char *argsarray = (char*)malloc(40*sizeof(char));           /* stores a copy of args[]*/
    char command[80];               /* used in exit command */

    printf("Welcome to GWShell. My pid is %d\n", getpid());
    prompt = 0;                      /* prompt stores how many commands so far */

    /* set up the signal handler */
    struct sigaction handler;
      handler.sa_handler = handle_SIGTSTP;
      handler.sa_flags = SA_RESTART;
      sigaction(SIGTSTP, &handler, NULL);

        /* wait for <control> <Z> */

    while (1){            /* Program terminates normally inside setup */
       history[prompt] = (char*)malloc(80*sizeof(char));  /* allocate address space for history[] */
       prompt+=1;
       background = 0;
       printf("GWSHELL[%d]->\n",prompt);
       length = setup(inputBuffer,args,&background);       /* get next command */

        /* store args[] into history[] */
       strcpy(history[prompt-1],args[0]);
       strcat(history[prompt-1]," ");
       for (i=1;i<length;i++){
        strcpy(argsarray,args[i]);
        strcat(argsarray," ");
        strcat(history[prompt-1],argsarray);
        }                                               /* end of store process */

       if (strcmp(args[0],"whisper") == 0)              /* built-in command whisper */
       {
            for (i=1; i<length; i++)
           {
               strcpy(argsarray,args[i]);
               for (j=0;j<strlen(argsarray);j++){
                argsarray[j]=tolower(argsarray[j]);     /* tolower to change upper case into lower case */
               }
               printf("%s ",argsarray);
           }
           printf("\n");
       }else if (strcmp(args[0],"exit") == 0)           /* built-in exit command */
       {
           sprintf(argsarray,"%d",getpid());
           strcpy(command,"ps -o pid,ppid,pcpu,pmem,etime,user,command -p ");
           strcat(command,argsarray);
           system(command);
           exit(0);
       }else if (strcmp(args [0],"r") == 0)             /* built-in r command */
       {
           if (length == 1){                            /* if the command is only r */
            printf("%s\n",history[prompt-2]);           /* print out the last command */
            strcpy(history[prompt-1],history[prompt-2]);/* store last command in history[] instead of r */
            strcpy(inputBuffer,history[prompt-1]);
            rlen=rsetup(inputBuffer,args,&background);  /* call rsetup() to tokenize inputBuffer again */

            if (strcmp(args[0],"whisper") == 0)         /* if the last command is whisper */
            {
                for (i=1; i<rlen; i++)
                {
                    strcpy(argsarray,args[i]);
                    for (j=0;j<strlen(argsarray);j++){
                        argsarray[j]=tolower(argsarray[j]);
                    }
                    printf("%s ",argsarray);
                }
                printf("\n");
            }
            else {                                      /* if the last command is not a built-in command */
                pid = fork();
                if (pid < 0){
                    printf("Fork Failed");
                    return 1;
                }
                if (pid == 0){                            /* child process */
                    printf("Child pid: %d, Background = ",getpid());
                    if (background == 0){
                        printf("False\n");
                    }else {printf("True\n");}
                    execvp(args[0],args);
                }
                if ((pid > 0)&&(background == 0)) {      /* parent process and child run in foreground */
                    waitpid(pid,&status,0);
                    printf("Child Complete\n");
                    }
                }
            }
            if (length == 2){                           /* if the command is r followed by a number */
                num = strtol(args[1],NULL,0);           /* convert the second element to long int */
                num = num+1;                            /* adjust the index to be stored in history */
                printf("%s\n",history[prompt-num]);
                strcpy(history[prompt-1],history[prompt-num]);

                strcpy(inputBuffer,history[prompt-1]);
                rlen=rsetup(inputBuffer,args,&background);  /* call rsetup() to tokenize inputBuffer again */

                if (strcmp(args[0],"whisper") == 0)         /* if the command is whisper */
                {
                    for (i=1; i<rlen; i++)
                    {
                        strcpy(argsarray,args[i]);
                        for (j=0;j<strlen(argsarray);j++){
                        argsarray[j]=tolower(argsarray[j]);
                    }
                    printf("%s ",argsarray);
                    }
                    printf("\n");
                }
                else {                                      /* if the command is not a built-in command */
                    pid = fork();
                    if (pid < 0){
                        printf("Fork Failed");
                        return 1;
                    }
                    if (pid == 0){                            /* child process */
                        printf("Child pid: %d, Background = ",getpid());
                        if (background == 0){
                            printf("False\n");
                        }else {printf("True\n");}
                        execvp(args[0],args);
                    }
                    if ((pid > 0)&&(background == 0)) {      /* parent process and child run in foreground */
                        waitpid(pid,&status,0);
                        printf("Child Complete\n");
                        }
                    }
                }
       }else {                                        /* if the command is not a built-in command */
            pid = fork();                             /* fork() a child process */
            if (pid < 0){                             /* fork failed */
                printf("Fork Failed");
                return 1;
            }
            if (pid == 0){                            /* child process */
                printf("Child pid: %d, Background = ",getpid());
                if (background == 0){
                    printf("False\n");
                }else {printf("True\n");}
                execvp(args[0],args);
            }
            if ((pid > 0)&&(background == 0)) {      /* parent process and child run in foreground */
                waitpid(pid,&status,0);
                printf("Child Complete\n");
            }
        }

       }

      /* the steps are:
       (0) if built-in command, handle internally
       (1) if not, fork a child process using fork()
       (2) the child process will invoke execvp()
       (3) if background == 0, the parent will wait,
            otherwise returns to the setup() function. */
}
