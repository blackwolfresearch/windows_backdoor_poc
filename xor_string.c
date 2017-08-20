#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *xorit(unsigned char xor_val, char *str, int len) {
    char *buf=malloc(len+1);
    int i=0;

    for (i=0;i<len;i++) {
        buf[i]=(str[i] ^ (unsigned char)xor_val);
    }

    return buf;
}

int main(int argc, char *argv[1]) {
	unsigned char xor_val;
	unsigned char *str1, *str2, *str3;
	int i, len;

	if (argc < 2) {
		printf("Usage: xor_string string xor_num\n\n");
		exit(0);
	}

	if (argc > 2) {
		xor_val=(unsigned char)atoi(argv[2]);
	} else {
		srandom(time(NULL));
		xor_val=(unsigned char)random();
	}

	str1=argv[1];
	len=strlen(str1);
	str2=xorit(xor_val, str1, len);	
	str3=xorit(xor_val, str2, len);

	printf("// %s\n", str3);
	printf("xorit(0x%02X, \"", xor_val);

	for (i=0;i<len;i++) {
		printf("\\x%02x", str2[i]);
	}
	
	printf("\",%d)\n\n", len);
}
