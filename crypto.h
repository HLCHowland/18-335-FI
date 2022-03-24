#include <stdio.h>
#include <sodium.h>

#define CHUNK_SIZE 1000000
#define CRYPTO_HEADER_SIZE 24
#define CRYPTO_ABYTE_SIZE 17
#define CRYPTO_FINAL_SIZE 3

int encryptfile(char *source_file, char key[crypto_secretstream_xchacha20poly1305_KEYBYTES]);
int decryptfile( char *source_file, char key[crypto_secretstream_xchacha20poly1305_KEYBYTES]);