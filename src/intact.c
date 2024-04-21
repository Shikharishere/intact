#include <stdio.h> 
#include <stdlib.h>
#include <gcrypt.h>
#include <string.h>
#include <unistd.h>

/* defines */ 
#define ALGO GCRY_CIPHER_AES128 
#define MODE GCRY_CIPHER_MODE_CTR 

void init(void);
void input(int argc, char *argv[]);
void core(int utility, char *filename, char *newFilename); 
void encrypt(gcry_cipher_hd_t handle, unsigned char *key, char *filename, char *newFilename);
void decrypt(gcry_cipher_hd_t handle, unsigned char *key, char *filename, char *newFilename);
int check_file(char *filename, int use);
int hasExtension(char *filename, char *extension);
char *generateRandomString(int length);


int main(int argc, char *argv[]) {
    init();
    input(argc, argv);
}


void encrypt(gcry_cipher_hd_t h, unsigned char *key, char *filename, char *newFilename) { 
     FILE *file1 = fopen(filename, "rb+");
     FILE *file2 = fopen(newFilename, "w");
    // FILE *newFile = fopen(newFilename

    if (!file1) {
        fprintf(stderr, "Error: Failed to open file"); 
        exit(1);
    }

    fseek(file1, 0, SEEK_END); 
    long fileSize = ftell(file1);
    rewind(file1);

    unsigned char *plaintext = malloc(fileSize); 
    unsigned char *ciphertext = malloc(fileSize); 

    if(!plaintext || !ciphertext) {
        fprintf(stderr, "Error: Malloc failed\n"); 
        exit(1);
    } 

    size_t bytesRead = fread(plaintext, 1, fileSize, file1);

    if (gcry_cipher_encrypt(h, ciphertext, fileSize, plaintext, bytesRead) != 0) {
        fprintf(stderr, "Error: Encryption failed!!\n"); 
        exit(1);
    }

    fwrite(ciphertext, 1, bytesRead, file2);

    free(plaintext); 
    free(ciphertext);
    fclose(file1); 
    fclose(file2);
}



void decrypt(gcry_cipher_hd_t h, unsigned char *key, char *filename, char *newFilename) {
    FILE *file1 = fopen(filename, "rb+"); 
    FILE *file2 = fopen(newFilename, "wb");

    if (!file1) {
        fprintf(stderr, "Error: Failed to open file\n");
        exit(1);
    }
    if (!file2) {
        fprintf(stderr, "Failed to open output file for writing\n");
        fclose(file1);
        exit(1);
    }
    fseek(file1, 0, SEEK_END); 
    long fileSize = ftell(file1); 
    rewind(file1);

    unsigned char *plaintext = malloc(fileSize); 
    unsigned char *ciphertext = malloc(fileSize);

    if (!plaintext || !ciphertext) {
        fprintf(stderr, "Error: Malloc failed\n");
        fclose(file1);
        fclose(file2);
        exit(1);
    }

    size_t bytesRead = fread(ciphertext, 1, fileSize, file1);

    if (gcry_cipher_decrypt(h, plaintext, fileSize, ciphertext, bytesRead) != 0) {
        fprintf(stderr, "Error: Decryption failed\n"); 
        fclose(file1);
        fclose(file2);
        exit(1);
    }

    fwrite(plaintext, 1, bytesRead, file2);

    free(plaintext); 
    free(ciphertext);
    fclose(file1);
    fclose(file2);
}


// Function to generate a random string of specified length
char *generateRandomString(int length) {
    // Define the characters that can be used in the random string
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    // Allocate memory for the random string (+1 for the null terminator)
    char *randomString = (char *)malloc((length + 1) * sizeof(char));
    if (randomString == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    // Seed the random number generator
    srand(time(NULL));

    // Generate random characters and build the string
    for (int i = 0; i < length; ++i) {
        randomString[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    randomString[length] = '\0'; // Null-terminate the string

    return randomString;
}


void core(int utility, char *filename, char *newFilename) {
    /* Initialization before encryption or decryption */

    /* Handle */ 
    gcry_cipher_hd_t h; 
    if (gcry_cipher_open(&h, ALGO, MODE, 0) != 0) {
        fprintf(stderr, "Error: Handle Couldn't be created\n");
        exit(1);
    };

    /* key */
    unsigned char *key = (unsigned char *)"CWxVGb8foNKCBQRL";
    size_t keylen = strlen((char *) key);
    if (gcry_cipher_setkey(h, key, keylen) != 0) {
        fprintf(stderr, "Error: Can't set key\n"); 
        exit(1);
    }
    
    if (utility == 0) {
        // encrypt
        char *nFilename = (char *)malloc(50 * sizeof(char));
        sprintf(nFilename, "%s.enc", filename);
        encrypt(h, key, filename, nFilename);
    } else if (utility == 1) {
        // decrypt
        decrypt(h, key, filename, newFilename);
    }
    gcry_cipher_close(h); 
}


void init(void) {
    if(!gcry_check_version(GCRYPT_VERSION)) {
        fprintf(stderr, "Libgcrypt: Version mismatch!\n");
        exit(1);
    }
}


int check_file(char *filename, int use) {
    if (access(filename, F_OK) == -1) {
        return -1;
    }
    // for encryption file
    if (use == 0) {
        if(hasExtension(filename, "txt")) {
            return 0;
        }
        else {
            return -1;
        }
    }
    // for encryption file
    else {
        if(hasExtension(filename, "enc")) {
            return 0;
        }
        else {
            return -1;
        }
    }
}


int hasExtension(char *filename, char *extension) {
    // Find the last occurrence of '.' in the filename
    const char *dot = strrchr(filename, '.');
    if (dot != NULL) {
        // Compare the extension
        if (strcmp(dot + 1, extension) == 0) {
            return 1; // Extension matches
        }
    }
    return 0; // Extension doesn't match or no extension found
}


void input(int argc, char *argv[]) {
   if (argc < 2) {
       printf("""Welcome to Intact.\nAn easy-to-use simple file encryption and decryption utility.\nUse -h or --help for more.\nYou can also read README.md for more info.\n"); 
       exit(1);
    } if (strcmp(argv[1], "-e") == 0 && argc == 3) {
        /* user wants to encrypt a file */
        if(check_file(argv[2], 0) != -1) {
            core(0, argv[2], ""); // 0 for encryption
            printf("Your file: %s will be encrypted\n", argv[2]); 
        } else {
            fprintf(stderr, "Error: Either fIlE not found or Maybe you are encrypting a wrong file!\n");
            exit(1);
        }
    } else if (strcmp(argv[1], "-d") == 0 && argc == 4) {
        /* user wants to decrypt a file */ 
        if (check_file(argv[2], 1) != -1) {
            core(1, argv[2], argv[3]); // 1 for decryption
            printf("Your file: %s will be decrypted\n", argv[2]);
        } else {
            fprintf(stderr, "Error: EitherfIlE NoT fOuNd!! or Maybe you are decrypting a wrong file!\n");
            exit(1);
        }
    } else if ((strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) && argc == 2) {
        printf("Here is help for ya!!!!\n");
        exit(1);
    } else {
        fprintf(stderr, "Error: Use intact -h or intact --help for help!!\n");
        exit(1);
    }
}
