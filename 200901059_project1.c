//*********************************************************************************/
//Karan Poddar
//200901059
//********************************************************************************/
//Assumptions:
//		i>Basic Interuptions of Keyboard like Ctrl+l,Ctrl+z,etc. will not work
//		ii>Commands typed will not be more then 1000
//		iii>Regexp, ; ,etc will not be shown
//		iv>Scrap commands will not work
//		v>Shortcuts like !,Ctrl+R,etc will not work
//*********************************************************************************/
#include<stdio.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<errno.h>

char parsed_com[100][100];
struct declare{
	char name[100];
	int pid;
	int flag;
}process_list[1000];
int process_count=0;
char *user;
char com[1000]={0},current_dir[1000]={0},home_dir[1000]={0},host[1000]={0};
int com_count=0,bg;



void signal_handler_fg(int signum)
{
	return;
}

void signal_handler(int signum)
{
	int loops,pid=wait(NULL);
	for(loops=0;loops<process_count;loops++)
		if(process_list[loops].pid==pid)
		{
			process_list[loops].flag=0;
			printf("\nchild process ended %s %d\n",process_list[loops].name,pid);
			break;
		}
	return;
}

void cd()
{
	int err;
	if(com_count==1)
		err=chdir(home_dir);
	else
	{   
		if(parsed_com[1][0]=='~')
		{   
			char temp[1000];    
			strcpy(temp,home_dir);
			strcat(temp,(parsed_com[1])+1);
			strcpy(parsed_com[1],temp);
		}   
		err=chdir(parsed_com[1]);
	}
	if(err==-1)
		perror("");
	return;
}

int command()
{
	char * argv[com_count];
	int loops;
	for(loops=0;loops<com_count;loops++)
		argv[loops]=parsed_com[loops];
	argv[com_count]=(char *) 0;
	if(execvp(parsed_com[0],argv)==-1)
	{
		perror("");
		return 1;
	}
	exit(0);
	return 0;
}

void pid(char * args[])
{
	int loops;
	if(com_count==1)
	{
		printf("command name: %s process id: %d\n",args[0],getpid());
		return;
	}
	if(strcmp(parsed_com[1],"current")==0)
	{   
		printf("List of currently executing processes spawned from this shell:\n");
		for(loops=0;loops<process_count;loops++)
			if(process_list[loops].flag==1)
				printf("command name: %s process id: %d\n",process_list[loops].name,process_list[loops].pid);
	}   
	else if(strcmp(parsed_com[1],"all")==0)
	{   
		printf("List of all processes spawned from this shell:\n");
		for(loops=0;loops<process_count;loops++)
			printf("command name: %s process id: %d\n",process_list[loops].name,process_list[loops].pid);
	} 
	else
	{
		for(loops=0;loops<com_count;loops++)
		{
			if(loops==0)
				printf("%s",parsed_com[loops]);
			else
				printf(" %s",parsed_com[loops]);
		}
		printf(": command not found\n");
	}
	return;
}

void run(char * args[])
{
	if(strcmp(parsed_com[0],"cd")==0)
		cd();
	else if(strcmp(parsed_com[0],"pid")==0)
		pid(args);
	else
	{
		int pid,err=0;
		if(bg!=1)
			signal(SIGCHLD,signal_handler_fg);
		pid=fork();
		if(pid==0)
			err=command();
		else
		{
			if(!err)
			{
				strcpy(process_list[process_count].name,parsed_com[0]);
				process_list[process_count].pid=pid;
				process_list[process_count].flag=0;
				process_count++;
				if(bg==1)
					process_list[process_count-1].flag=1;
			}
			if(bg!=1)
				waitpid(pid,NULL,0);
		}
	}
	return;
}

void find_dir()
{
	int loops,temp,temp2;
	getcwd(current_dir,sizeof(current_dir));
	if(home_dir[0]=='\0')
		strcpy(home_dir,current_dir);
	temp=strlen(home_dir);
	temp2=strlen(current_dir);
	if(temp2>=temp)
	{
		current_dir[0]='~';
		for(loops=1;loops<strlen(current_dir);loops++)
			current_dir[loops]=current_dir[temp+loops-1];
	}
	return;
}

void parse(char * com)
{
	int loops;
	char *temp,temp2[1000];
	const char delimits[]=" ,\t";
	temp=strchr(com,'&');
	if(temp!=NULL)
	{
		*temp='\0';
		bg=1;
	}
	strcpy(temp2,com);
	temp=strtok(temp2,delimits);
	while(1)
	{   
		if(temp==NULL)
			break;
		strcpy(parsed_com[com_count++],temp);
		temp=strtok(NULL,delimits);
	}   
	return;
}

int main(int argc,char * args[])
{
	char com[1000]={0};
	int loops;
	process_count=0;
	while(1)
	{
		bg=0;
		signal(SIGCHLD,signal_handler);
		for(loops=0;loops<=com_count;loops++)
			strcpy(parsed_com[loops],"\0");
		com_count=0;
		user=getlogin();
		gethostname(host,sizeof(host));
		find_dir();
		printf("<%s@%s:%s> ",user,host,current_dir);
		fflush(stdout);
		gets(com);
		if(strcmp(com,"exit")==0)
			return 0;
		parse(com);
		if(com_count>0)
			run(args);
	}
}
