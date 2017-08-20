#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LINE_LENGTH 20


void main(int argc, char *argv[]) {
	FILE *infile, *outfile;
	char *bin_file, *c_file, *var_prefix;
	unsigned char buf[LINE_LENGTH], xor_val;
	int total_bytes, bytes_read,i;

	if (argc != 4) {
		printf("Usage: encode_bin bin_file c_source_file var_prefix\n\n");
		exit(0);
	}

	bin_file = argv[1];
	c_file = argv[2];
	var_prefix = argv[3];
                
	srandom(time(NULL));
        xor_val=(unsigned char)random();

	infile=fopen(bin_file, "rb");
	if (!infile) {
		perror(bin_file);
		exit(0);
	}

	outfile=fopen(c_file, "wt");
	if (!outfile) {
		perror(c_file);
		exit(0);
	}

	fprintf(outfile, "unsigned char *%s_data=\n", var_prefix);

	total_bytes=0;
	do {
		bytes_read=fread(buf, 1, LINE_LENGTH, infile);
		if (bytes_read) {
			total_bytes += bytes_read;

			fprintf(outfile, "\t\"");
			for (i=0;i<bytes_read;i++) {
				fprintf(outfile, "\\x%02x", (buf[i] ^ xor_val));
			}

			if (bytes_read==LINE_LENGTH) {
				fprintf(outfile, "\"\n");	
			} else {
				fprintf(outfile, "\";\n\n");	
				fprintf(outfile, "int %s_data_len=%d;\n\n", var_prefix, total_bytes); 
				fprintf(outfile, "unsigned char %s_xor_val=0x%02x;\n\n", var_prefix, xor_val);
			}
		}
	} while (bytes_read == LINE_LENGTH);

	fclose(infile);
	fclose(outfile);
}
