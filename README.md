# Shell_Imitaion

## About
A small program I had to create as part of Operating Systems course assignment. The program imitates 4 standard Linux Shell commands, with additional option of running them in multiple processes in the background, while supporting the rest of the commands by rerouting them for execution. 

## The 4 Imiitated Commands
### `jobs`
* Display a list of the commands that are currently running in the background, in an ascending chronological order.
### `history`
* Display a list of all the commands entered in during the run so far (both background and non-background runs) in an ascending chronological order. Also, aside each command, print "DONE" or "RUNNING" to indicite whether the process it is still running on is alive or not.
* The `history` command itself is included in the list as a "RUNNING" and latest command.
### `cd`
* Change the working directory of the proccess.
* Supports the flags: -, .., ~. And their combination. 
### `exit`
* Exits the program with return value 0.


For any other built-in Shell commands, the program will call exec function and reroute it for execution as is.

In order to call one of the four implemented commands for a non-background run, add " &" as last flag to the command.


## Goal
This program's goal is to practice with proccess and child proccess as well as getting to know Shell commands.  
