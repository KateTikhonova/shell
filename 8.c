#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(str) 						\
{												\
	printf("Line: %d (%s)\n", __LINE__, str);	\
}

#define DEBUG_PRINT2(format, data) 								\
{																\
	printf("Line: %d "#format"\n", __LINE__, data);	            \
}

#else

#define DEBUG_PRINT(str)
#define DEBUG_PRINT2(format, data)

#endif
 
void handleSignal(int sgn)
{
	printf("[MY SHELL ] ");
	fflush(stdout);
}

int countBlank(char* str)
{
	//возвращает кол-во аргументов
	//если строка не пуста
	int number=0;
	if(*str != '\0'){
		number=1;
		while(*str != '\0'){
			if(*str == ' '){
					++number;
			}
			++str;
		}
	}
	return number;
}

int deleteBlank(char* str){
	int ret=1;
	*str='\0';
	if(*(str-1) == ' '){
			*(str-1)='\0';
	}
	if(*(str+1) == ' '){
		*(str+1)='\0';
		ret=2;
	}
	return ret;
}


void copyStr(char* to, char* from, int len)
{
	int i=0;
	while(i < len && *from !='\0'){
		*to=*from;
		to++;
		from++;
		i++;
	}
}	


int redirCmd(char* str, int* fd_in, int* fd_out){
	*fd_in=0;
	*fd_out=1;
	DEBUG_PRINT2("%s", str);
	char* fileNameIn=str;
	char* fileNameOut=str;
	int FLAG=0, flags=0, num;
	fileNameIn=strchr(str, '<');
	fileNameOut=strchr(str, '>');
	if(fileNameIn != NULL){		
		fileNameIn=deleteBlank(fileNameIn)+fileNameIn;	
	}
	if(fileNameOut != NULL){
		num=deleteBlank(fileNameOut);
		if(*(fileNameOut+1) == '>'){
			flags=1;
			fileNameOut+=deleteBlank((fileNameOut+1))+1;
		}
		else{
			fileNameOut+=num;
		}
		if(flags){
			*fd_out=open(fileNameOut,O_WRONLY | O_CREAT | O_APPEND, 0766);
		}
		else{
			*fd_out=open(fileNameOut,O_WRONLY | O_CREAT | O_TRUNC, 0766);
		}
		if(*fd_out == -1){
			printf("%s: Не получилось открыть файл\n", fileNameOut);
			FLAG=1;
		}	
	}
	if(fileNameIn != NULL){
		*fd_in=open(fileNameIn,O_RDONLY, 0766);
		if(*fd_in == -1){
			printf("%s: Не получилось открыть файл\n", fileNameIn);
			FLAG=1;
		}
		
	}
	DEBUG_PRINT2("%s", str);
	DEBUG_PRINT2("%s", fileNameOut);
	DEBUG_PRINT2("%s", fileNameIn);
	return FLAG;
	
}


void closeFd( int* fd_in, int* fd_out)
{
	if(*fd_in != 0){
			close(*fd_in);
	}
	if(*fd_out != 1){
			close(*fd_out);
	}
}

/*char** fillArgv(char* str)
{ 
	const int num=countBlank(str);
	DEBUG_PRINT2("%d", num);
	char* my_argv[num+1];
	char* tmp;
	tmp=str;
	char* nextStr;
	nextStr=str;
	for(i=0; i<num-1; ++i){
		DEBUG_PRINT("OK");
		while(*tmp != ' '){
			++tmp;
			DEBUG_PRINT2("%s", tmp);
		}
		*tmp='\0';
		++tmp;
		DEBUG_PRINT2("%s", nextStr);
		my_argv[i]=nextStr;
		nextStr=tmp;
		DEBUG_PRINT2("%s", nextStr);
	}
	my_argv[num-1]=nextStr;
	my_argv[num]=NULL;
	DEBUG_PRINT2("%d", num);
	DEBUG_PRINT2("%s", my_argv[0]);
	for(i=0; i<num; ++i){
		DEBUG_PRINT2("%s", my_argv[i]);
	}
	return my_argv;
}*/

int execCmdEnd(char* str, int* fd_in, int* fd_out, int key)
{
	int err, i, pid, status;
	int pd[2];
	pd[0]=0;
	pd[1]=1;
	if(key){
		pd[0]=*fd_in;
		pd[1]=*fd_out;
	}
	DEBUG_PRINT2("%d", pd[0]);
	DEBUG_PRINT2("%d", pd[1]);
	err=redirCmd(str, fd_in, fd_out);
	if(*str != '\0' && !err){
		
		
		const int num=countBlank(str);
		DEBUG_PRINT2("%d", num);
		char* my_argv[num+1];
		char* tmp;
		tmp=str;
		char* nextStr;
		nextStr=str;
		for(i=0; i<num-1; ++i){
			DEBUG_PRINT("OK");
			while(*tmp != ' '){
				++tmp;
				DEBUG_PRINT2("%s", tmp);
			}
			*tmp='\0';
			++tmp;
			DEBUG_PRINT2("%s", nextStr);
			my_argv[i]=nextStr;
			nextStr=tmp;
			DEBUG_PRINT2("%s", nextStr);
			}
		my_argv[num-1]=nextStr;
		my_argv[num]=NULL;
		DEBUG_PRINT2("%d", num);
		DEBUG_PRINT2("%s", my_argv[0]);
		for(i=0; i<num; ++i){
			DEBUG_PRINT2("%s", my_argv[i]);
		}
		DEBUG_PRINT2("%d", *fd_in);
		DEBUG_PRINT2("%d", *fd_out);
		DEBUG_PRINT2("%d", pd[0]);
		DEBUG_PRINT2("%d", pd[1]);
		pid=fork();
		if(pid == 0){
			DEBUG_PRINT("Команда сейчас будет запущена");
			if(key){
				close(pd[1]);
				if(*fd_in != 0){
					DEBUG_PRINT("Кk");
					dup2(*fd_in, 0);
					close(*fd_in);
				}
				else{
					dup2(pd[0], 0);
				}
				if(*fd_out != 1){
					DEBUG_PRINT("Кk");
					dup2(*fd_out, 1);
					close(*fd_out);
				}
			}
			else{
				if(*fd_in != 0){
					dup2(*fd_in, 0);
					close(*fd_in);
				}
				if(*fd_out != 1){
					dup2(*fd_out, 1);
					close(*fd_out);
				}
			}
			DEBUG_PRINT("Ок");
			i=execvp(my_argv[0], my_argv);
			if(i < 0){
				closeFd(fd_in, fd_out);
				if(key){
					close(pd[0]);
					close(pd[1]);
				}
				write(2, my_argv[0], sizeof(my_argv[0]));
				write(2,":Команда не найдена\n", 37); 
				exit(1);
			}
		}
		else{
			wait(&status);
			DEBUG_PRINT2("%d", status);
			if(key){
				close(pd[0]);
				close(pd[1]);
			}
			else{
				closeFd(fd_in, fd_out);
			}
		}
	}
	else{
		
		status=1;
	}
	return status;
}

void execCmdBack (char* str, int* fd_in, int* fd_out, int key)
{
	int err,i, pid, pid2, status,pd[2];
	if(key){
		pd[0]=*fd_in;
		pd[1]=*fd_out;
	}
	err=redirCmd(str, fd_in, fd_out);
	if(*str != '\0' && !err){


		const int num=countBlank(str);
		DEBUG_PRINT2("%d", num);
		char* my_argv[num+1];
		char* tmp;
		tmp=str;
		char* nextStr;
		nextStr=str;
		for(i=0; i<num-1; ++i){
			DEBUG_PRINT("OK");
			while(*tmp != ' '){
				++tmp;
				DEBUG_PRINT2("%s", tmp);
			}
			*tmp='\0';
			++tmp;
			DEBUG_PRINT2("%s", nextStr);
			my_argv[i]=nextStr;
			nextStr=tmp;
			DEBUG_PRINT2("%s", nextStr);
			}
		my_argv[num-1]=nextStr;
		my_argv[num]=NULL;
		DEBUG_PRINT2("%d", num);
		DEBUG_PRINT2("%s", my_argv[0]);
		for(i=0; i<num; ++i){
			DEBUG_PRINT2("%s", my_argv[i]);
		}
		
		
		pid=fork();
		if(pid == 0){
			pid2=fork();
			if(pid2 > 0){
				printf("[%d]\n", pid2);
				if(key){
					close(pd[0]);
					close(pd[1]);
				}
				closeFd(fd_in, fd_out);
				DEBUG_PRINT("Команда сейчас будет запущена");
				while(1);
			}
			else{
				
				i=setpgid(0,0);
				DEBUG_PRINT2("%d",i);
				if(*fd_in == 0){
					if(!key){
						*fd_in=open("/dev/null", O_RDONLY);
						DEBUG_PRINT2("%d",*fd_in);
					}
					else{
						*fd_in=pd[0];
					}
				}
				dup2(*fd_in, 0);
				close(*fd_in);
				if(*fd_out == 1){
					*fd_out=open("/dev/null", O_WRONLY);
					DEBUG_PRINT2("%d",*fd_out);
				}
				dup2(*fd_out, 1);
				close(*fd_out);
				if(key){
					close(pd[1]);
				}
				kill(getppid(), 9);
				i=execvp(my_argv[0], my_argv);
				if(i < 0){
					write(2, my_argv[0], sizeof(my_argv[0]));
					write(2,":Нет такой задачи\n", 33);
					DEBUG_PRINT("fff");
					closeFd(fd_in, fd_out);
					//kill(getppid(), 9);
					exit(1);
				}
			}		
		}
		else{
			wait(&status);
			if(key){
				close(pd[0]);
				close(pd[1]);
			}
			closeFd(fd_in, fd_out);
		}
	}
	
		
}

int execCmdPipe(char* str,int pd1[2],int amount, int key)
{
	int ret=0, fd_in=0, fd_out=1, pid, err, stat;
	if(amount != 0){
		//ищем следующую команду
		char* nextCmd=str;
		while(*nextCmd != '#'){
			++nextCmd;
			DEBUG_PRINT2("%c", *nextCmd);
		}
		*nextCmd ='\0';
		++nextCmd;
		if(*nextCmd == '\0'){
			++nextCmd;
		}
		DEBUG_PRINT2("%s", nextCmd);
		//заполняем argv, fd_in, fd_out
		err=redirCmd(str, &fd_in, &fd_out);
		if(*str != '\0' && !err){
			
			int i;
			const int num=countBlank(str);
			DEBUG_PRINT2("%d", num);
			char* my_argv[num+1];
			char* tmp;
			tmp=str;
			char* nextS;
			nextS=str;
			for(i=0; i<num-1; ++i){
				DEBUG_PRINT("OK");
				while(*tmp != ' '){
					++tmp;
					DEBUG_PRINT2("%s", tmp);
				}
				*tmp='\0';
				++tmp;
				DEBUG_PRINT2("%s", nextS);
				my_argv[i]=nextS;
				nextS=tmp;
				DEBUG_PRINT2("%s", nextS);
			}
			my_argv[num-1]=nextS;
			my_argv[num]=NULL;
			for(i=0; i<num; ++i){
				DEBUG_PRINT2("%s", my_argv[i]);
			}
			//argv+
			int pd2[2];
			pipe(pd2);
			pid=fork();
			if(pid == 0){
				DEBUG_PRINT("Команда сейчас будет запущена");
				close(pd2[0]);
				if(fd_in != 0){
					dup2(fd_in, 0);
					close(fd_in);
					if(pd1[0] != 0){
						close(pd1[1]);
						close(pd1[0]);
					}
				}
				else{
					if(pd1[0] != 0){
						close(pd1[1]);
						dup2(pd1[0], 0);
					}
				}
				if(fd_out != 1){
					dup2(fd_out, 1);
					close(fd_out);
					close(pd2[1]);
				}
				else{
					dup2(pd2[1], 1);
				}
				DEBUG_PRINT("ok");
				i=execvp(my_argv[0], my_argv);
				if(i<0){
					closeFd(&fd_in, &fd_out);
					/*close(pd2[1]);
					if(pd1[0] != 0){
						close(pd1[0]);
						close(pd1[1]);
					}*/
					write(2, my_argv[0], sizeof(my_argv[0]));
					write(2,":Команда не найдена\n", 37); 
					exit(1);
				}
			}
			else{
				closeFd(&fd_in, &fd_out);
				if(pd1[0] != 0){
					close(pd1[0]);
					close(pd1[1]);
				}
				ret=execCmdPipe(nextCmd, pd2, amount-1, key);
				wait(&stat);
				DEBUG_PRINT("OK");
				DEBUG_PRINT2("%d", ret);
			}
		}
	}
	else{
		if(key){
			execCmdBack(str, &pd1[0], &pd1[1], 1);
		}
		else{
			ret=execCmdEnd(str, &pd1[0],&pd1[1], 1);
		}
			
	}
	
	return ret;
}

int countPipe(char* str, int* fd_in, int* fd_out)
{
	char* tmp=str;
	int ret=0;
	DEBUG_PRINT("OK");
	while(*tmp != '\0'){
		DEBUG_PRINT2("%c", *tmp);
		if(*tmp == '|'){
			++ret;
			*tmp='#';
			if(*(tmp-1) == ' '){
			*(tmp-1)='\0';
			}
			if(*(tmp+1) == ' '){
				*(tmp+1)='\0';
				++tmp;
			}
		}
		tmp++;
	}
	DEBUG_PRINT("OK");
	DEBUG_PRINT2("%s",str);
	return 	ret;
}


int handlePipe(char* str, int* fd_in, int* fd_out,int key)
{
	//key = 0 следовательно ; или && key=1  - &
	int amount=countPipe(str, fd_in, fd_out);
	int ret=-1;
	DEBUG_PRINT2("%d",amount);
	if(amount != 0){
		int pd[2];
		pd[0]=0;
		pd[1]=1;
		ret=execCmdPipe(str, pd, amount, key);
		//ret=1;
	} 
	else{
		if(key){
			execCmdBack(str, fd_in, fd_out, 0);
		}
		else{
			ret=execCmdEnd(str, fd_in,fd_out, 0);
		}
	}
	return ret;
}

int handleStr (char* str, int* fd_in, int* fd_out,int key)
{
	char* tmp=str;
	char* nextStr;
	int i=0, num;
	int flag=0;// нужен, чтобы в обратном порядке команды не выполнялись
	while(*tmp != '\0' && !flag){
		if (*tmp == ';'){
			flag=1;
			num=deleteBlank(tmp);
			nextStr=tmp + num;
			DEBUG_PRINT2("%s", str);
			break;
		}
		if (*tmp == '&'){
			if(*(tmp+1) == '&'){
				flag=2;
				deleteBlank(tmp);
				num=deleteBlank(tmp+1);
				nextStr=tmp + num+1;
			}
			else{
				flag=3;
				num=deleteBlank(tmp);
				nextStr=tmp + num;
			}
			break;
		}
		tmp++;
	}
	switch (flag){
	case 0:
		if(!key){
			i=handlePipe(str, fd_in, fd_out, 0);
		}
		break;
	
	case 1:
		if(!key){
			i=handlePipe(str, fd_in, fd_out, 0);
		}
		handleStr(nextStr, fd_in, fd_out, 0);
		break;
	case 2:
		if(!key){
				i=handlePipe(str, fd_in, fd_out,0);
				handleStr(nextStr, fd_in, fd_out, i);
			}
			else{
				handleStr(nextStr, fd_in, fd_out, key);
			}	
			break;
	case 3:
		if(!key){
			handlePipe(str, fd_in, fd_out, 1);
		}
		i=handleStr(nextStr, fd_in, fd_out, 0);	
	}	
	return i;
}

int main()
{
	signal(SIGINT, handleSignal);
	//char* s=(char*)malloc(sizeof(char)*256);
	char s[256];
	char* t;
	t=s;
	int i=-1, err=1, index=0, fd_in=0, fd_out=1;
	printf("[MY SHELL ] ");
	while(err != EOF){
		err=scanf("%c", t);
		if(*t == ' '){
			if(t == s){
				continue;
			}
			if(index != 0){
				continue;
			}
			++index;	
		}
		else{
			index=0;
		}
		if(*t == '\n'){
			*t='\0';
			--t;
			while(*t == ' '){
				*t='\0';
				--t;
			}
			//DEBUG_PRINT2("%s", s);
			i=handleStr (s,&fd_in,&fd_out,0);
			//execCmdBack(s, &fd_in,&fd_out);
			//i=handlePipe(s, &fd_in,&fd_out, 1);
			//printf("\n%d\n", i);
			printf("\n[MY SHELL ] ");
			t=s;
		}
		else{
			t++;
		}
	}
	return 0;
}
