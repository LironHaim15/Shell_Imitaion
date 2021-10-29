// Liron Haim 206234635
// tested with Ron Even testing script, many thanks!
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <wait.h>
#define FALSE 0
#define TRUE 1
/** recieves 2 strings: go thorugh 'str' and saves it in 'simplified' as echo command would print it.
 * flag is changed to TRUE if last char in 'str' is '&'.
 */
void simplifyString(char str[101], char simplified[101], int* flag)
{
    // rIndex is reading index, wIndex is writing index, ignoreSpaces is a flag indication if we're between " ".
    int wIndex = 0, rIndex = 0, ignoreSpaces = TRUE, spaceCount = 0;
    while (str[rIndex] == ' ') // delete spaces at the begining
    {
        rIndex++;
    }
    while (str[rIndex] != '\0') //while haven't reached the end of str
    {
        if (str[rIndex]=='\"')
        {
            rIndex++;
            ignoreSpaces = ignoreSpaces == FALSE ? TRUE : FALSE;
            continue;
        }
        else if(ignoreSpaces == FALSE){ //save every char, incdluding spaces
            simplified[wIndex] = str[rIndex];
            wIndex++;
            rIndex++;
            spaceCount = 0;
            continue;
        }
        else
        {
            if (str[rIndex]==' ') //if space then copy once.
            {
                if(spaceCount==0)
                {
                    simplified[wIndex] = ' ';
                    wIndex++;
                    rIndex++;
                    spaceCount++;
                }
                else //(spaceCount!=0)
                    rIndex++;
                continue;
            }
            else // not space
            {
                simplified[wIndex] = str[rIndex];
                spaceCount = 0;
                wIndex++;
                rIndex++;
                continue;
            }
        }
    }
    //check if there is '&' at the end, if is then delete it and change flag to TRUE.
    if (simplified[wIndex-1]=='&' && simplified[wIndex-2]==' ')
    {
        *flag = TRUE;
        simplified[wIndex-2] = '\0';
        simplified[wIndex-1] = '\0';
    }
    else if (simplified[wIndex-1]==' ' && simplified[wIndex-2]=='&' && simplified[wIndex-3]==' ')
    {
        *flag = TRUE;
        simplified[wIndex-1] = '\0';
        simplified[wIndex-1] = '\0';
        simplified[wIndex-2] = '\0';
    }
    // change trail to null char
    while (rIndex>=wIndex)
    {
        simplified[wIndex] = '\0';
        wIndex++;
    }
}
/** print prompt and wait for user input. parses the input to tokens by spaces and saves the tokens in
 * commandVector. if last token is '&' then change flag value to TRUE. if first token is "echo" then save
 * the rest to the input as one string in echoString.
 */
void scanAndParse(char *commandVector[101], int* flag, char echoString[101],char command[101], char commandParsed[101][101])
{
    //print prompt
    printf("$ ");
    fflush(stdout);
    //user input
    if (scanf("%100[^\n]%*c",command) != 1)
    {
        printf("An error occurred\n");
    }

    char *token;
    int i = 0;
    char copy[101];
    strcpy(copy, command);

    char* echoPointer = strstr(command, "echo"); // get index of substring "echo" is exists.
    token = strtok(command, " ");
    
    //check if first token is "echo"
    if (echoPointer!= NULL && token != NULL && strcmp(token,"echo")==0)
    {
        commandVector[i] = token;
        strcpy(commandParsed[i], commandVector[i]);
        echoPointer += 5;
        // simplify the following string and save it.
        simplifyString(echoPointer, echoString, flag);
        commandVector[i + 1] = echoString;
        strcpy(commandParsed[i + 1], commandVector[i + 1]);
        commandVector[i + 2] = NULL;
        return;
    }

    token = strtok(copy, " ");
    //parse the rest of the command.
    while (token != NULL)
    {
        commandVector[i] = token;
        strcpy(commandParsed[i], commandVector[i]);
        //printf("vecor%d: %s\n",i,commandVector[i]); //////////////////////////////////////////////////////////////////////
        i++;
        token = strtok(NULL, " ");
    }
    commandVector[i] = NULL; //after last token save NULL
    commandParsed[i][0] = 0;
    
    //check if last token is '&'
    if(strcmp(commandVector[i-1], "&")==0)
    {
        *flag = TRUE;
        commandVector[i - 1] = NULL;

    }
}
/** handles history command. recieves two arrays: history array conatining all the commands entered since the beginning,
 * and historyPID array containing each command's process ID. for commands already done the ID will be -1.
 * historyIndex is the index of the last command entered.
 * prints each command and its status: DONE or RUNNING.
 */
void historyCommand(char history[101][101], pid_t* historyPID, int* historyIndex)
{
    int i = 0, stat = 0;
    // go through all the previous commands in the array.
    for (; i < (*historyIndex)-1; i++)
    {
        //check if already done.
        if (historyPID[i] == -1)
        {
            printf("%s DONE\n", history[i]);
            continue;
        }
        
        //check if child proccess is still alive.
        pid_t p = waitpid(historyPID[i], &stat, WNOHANG);
        if(p == 0)
        {
            printf("%s RUNNING\n", history[i]);
            continue;
        }
        //if done then change ID to -1.
        historyPID[i] = -1;
        printf("%s DONE\n", history[i]);
    }
    printf("history RUNNING\n");
}
/** handles jobs command. recieves two arrays: history array conatining all the commands entered since the beginning,
 * and historyPID array containing each command's process ID. for commands already done the ID will be -1.
 * historyIndex is the index of the last command entered.
 * prints each running job.
 */
void jobsCommand(char history[101][101], pid_t* historyPID, int* historyIndex)
{
    int i = 0, stat = 0;
    for (; i < *historyIndex; i++)
    {
        //check if child proccess is alive, if yes then print its command.
        pid_t p = waitpid(historyPID[i], &stat, WNOHANG);
        if (p == 0 && historyPID[i] != -1)
        {
            printf("%s\n", history[i]);
        }
    }
}
/** handles change directory command. recieves two arrays: commandVector array conatining parsed command, lastDir is a String
 * containig the last path, and working path used to save the current path before changing it (to support the command "cd -").
 * supports the command: "cd" with the flags ..,~,-.
 */
void chdirCommand(char commandVector[101][101], char lastDir[101],char workingPath[101])
{
    //if more than 2 tokens in the command. cd and one argument
    if(commandVector[2][0]!=0){
        printf("Too many arguments\n");
        return;
    }
    //cd ~ command
    else if (commandVector[1][0]=='\0' || (commandVector[1][0]=='~' && commandVector[1][1]=='\0'))
    {
        
        if(getcwd(workingPath,sizeof(char)*101)==NULL)
            printf("An error occurred\n");
        
        if (chdir(getenv("HOME")) != 0)
        {
            printf("chdir failed\n");
            return;
        }
        strcpy(lastDir, workingPath);
    }
    else if (commandVector[1][0]=='~' && commandVector[1][1]=='/')
    {
        if(getcwd(workingPath,sizeof(char)*101)==NULL)
            printf("An error occurred\n");
        strcpy(lastDir, workingPath);

        workingPath[0] = '\0';
        strcat(workingPath, getenv("HOME"));
        strcat(workingPath, &commandVector[1][1]);
        if (chdir(workingPath) != 0)
        {
            printf("chdir failed\n");
            if(getcwd(workingPath,sizeof(char)*101)==NULL)
                printf("An error occurred\n");
            strcpy(lastDir, workingPath);
            return;
        }
    }
    //cd - command
    else if (commandVector[1][0]=='-' && commandVector[1][1]=='\0')
    {
        if(lastDir[0]==0)
            return;// or error????
        else
        {
            if(getcwd(workingPath,sizeof(char)*101)==NULL)
                printf("An error occurred\n");
            if (chdir(lastDir) != 0)
            {
                printf("chdir failed\n");
                return;
            }
            strcpy(lastDir, workingPath);
        }
    }
    //cd .. command
    else if (commandVector[1][0]=='.' && commandVector[1][1]=='.' )
    {
        if(getcwd(workingPath,sizeof(char)*101)==NULL)
            printf("An error occurred\n");
        if (chdir(commandVector[1]) != 0)
        {
            printf("chdir failed\n");
            return;
        }
        strcpy(lastDir, workingPath);
    }
    //cd command
    else
    {
        if (chdir(commandVector[1]) != 0)
        {
            printf("chdir failed\n");
            return;
        }
    }
    
    
}

/** the main program behaves as a shell, supports built-in commands: history,jobs,cd,exit. 
 * any other command used exec to be executed.
 */
int main(int argc, char* argv[]){
    
    pid_t pid= getpid();
    int stat=0, waited;
    char command[101];
    char *commandVector[101]={0};
    char commandParsed[101][101]={0};
    char echoString[101];

    char* vector[101];
    char lastDir[101] = {0};

    char workingPath[101];
    
    //history and PID arrays 
    char history[101][101] = {""};
    pid_t historyPID[101] = {0};
    int historyIndex = 0;

    while (TRUE)
    {
        if (pid == 0)
        {
            // CHILD - call exec with tha parsed command vecotr.
            execvp(vector[0], vector);
            printf("exec failed\n"); // print error if exec fails.
            exit(1);
        }
        else
        {
            //PARENT
            int backgroundFlag = 0;
            //ask for input and parse it. also check if it is "echo" command.
            scanAndParse(commandVector, &backgroundFlag, echoString, command, commandParsed);
            int i = 0;
            //copy the parese command into another array for backup. and create strings for the history array
            //by stiching back together the parsed command.
            for (; commandVector[i] != NULL; i++)
            {
                //strcpy(commandParsed[i], commandVector[i]);
                //strcpy(vector[i], commandParsed[i]);
                commandVector[i] = commandParsed[i];
                vector[i] = commandParsed[i];
                strcat(history[historyIndex], commandVector[i]);
                if (commandVector[i+1] == NULL) //skip the strcat of the space after the last command token.
                    continue;
                strcat(history[historyIndex], " ");
            }
            //end command vector with NULL for exec call.
            commandParsed[i][0] = 0;
            vector[i] = NULL;

            historyPID[historyIndex] = -1;
            historyIndex++;
            //check if built-in commands.
            if(!strcmp(commandVector[0],"cd"))
            {
                chdirCommand(commandParsed, lastDir,workingPath);
                continue;
            }
            if(!strcmp(commandVector[0],"jobs"))
            {
                jobsCommand(history, historyPID, &historyIndex);
                continue;
            }
            if(!strcmp(commandVector[0],"history"))
            {   
                historyCommand(history, historyPID, &historyIndex);
                continue;
            }
            if(!strcmp(commandVector[0],"exit"))
            {
                //free(vector);
                return 0;
            }
            
            //create child proccess
            pid = fork();
            //save its PID
            historyIndex--;
            historyPID[historyIndex] = pid;
            historyIndex++;

            if (pid < 0)
            {
                printf("fork failed\n"); 
            }
            if (!backgroundFlag)
            {
                //foreground - wait for child proccess to terminate.
                waited = waitpid(pid,&stat,0);
                historyIndex--;
                historyPID[historyIndex] = -1;
                historyIndex++;
            }
            //else - background, continue.
        }
    }
}