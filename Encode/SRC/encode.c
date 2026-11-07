#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#include <string.h>

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
    fseek(fptr, 0, SEEK_END);
    // return secret file size
    return ftell(fptr);
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *dot;

    // step 1 -> check argv[2] is having .bmp or not
    dot = strrchr(argv[2], '.');
    if (dot == NULL || strcmp(dot, ".bmp") != 0)
    {
        fprintf(stderr, "ERROR: Source image %s is not a .bmp file\n", argv[2]);
        return e_failure;
    }
    //yes -> store the file name into encInfo -> src_image_fname = argv[2]
    encInfo->src_image_fname = argv[2];

    // step 2 -> check argv[3] is having extn there or not
    dot = strrchr(argv[3], '.');
    if (dot == NULL || dot[1] == '\0')
    {
        fprintf(stderr, "ERROR: Secret file %s has no extension\n", argv[3]);
        return e_failure;
    }
    // extn_secret_file can hold only 4 characters + '\0'
    if (strlen(dot) > 4)
    {
        fprintf(stderr, "ERROR: Secret file extension %s is too long (max 4 characters including '.')\n", dot);
        return e_failure;
    }
    // yes - > store the file name into encInfo -> secret_fname = argv[3]
    encInfo->secret_fname = argv[3];
    // store the extension (like .txt) for encoding later
    strcpy(encInfo->extn_secret_file, dot);

    // step 3 -> check argv[4] is NULL or not
    if (argv[4] != NULL)
    {
        // yes -> check argv[4] is having .bmp or not
        dot = strrchr(argv[4], '.');
        if (dot == NULL || strcmp(dot, ".bmp") != 0)
        {
            fprintf(stderr, "ERROR: Output image %s is not a .bmp file\n", argv[4]);
            return e_failure;
        }
        // yes -> store the file name into encInfo -> stego_image_fname = argv[4]
        encInfo->stego_image_fname = argv[4];
    }
    else
    {
        // no -> store default name the encInfo -> stego_image_fname = "stego.bmp"
        encInfo->stego_image_fname = "stego.bmp";
    }

    // return e_success;
    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    // Magic                 = 2 x 8 = 16
    // Extension size        = 4 x 8 = 32
    // Extension             = 4 x 8 = 32
    // Secret size           = 4 x 8 = 32
    // Secret data           = 9 x 8 = 72

    //step 1 -> encInfo -> image_capacity = call get_image_size_for_bmp(encoInfo -> fptr_src_image)
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    //step 2 -> encInfo -> size_secret_file = call get_file_size(encInfo -> fptr_secret)
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // secret file must not be empty
    if (encInfo->size_secret_file == 0)
    {
        fprintf(stderr, "ERROR: Secret file %s is empty.No secret message!!!.\n", encInfo->secret_fname);
        return e_failure;
    }

    //step 3 -> check encInfo -> image_capacity > 16 + 32 + (strlen(encInfo -> extn_secret_file) * 8) + 32 + (encInfo -> size_secret_file * 8)
    if (encInfo->image_capacity > 16 + 32 + (strlen(encInfo->extn_secret_file) * 8) + 32 + (encInfo->size_secret_file * 8))
    {
        //t -> return e_success;
        return e_success;
    }
    //f -> return e_failure
    return e_failure;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // step 1 -> rewind source file pointer
    rewind(fptr_src_image);
    // declare one character buffer of size 54
    char buffer[54];

    // step 2 -> read 54 bytes from source file fread()
    if (fread(buffer, 1, 54, fptr_src_image) != 54)
    {
        return e_failure;
    }

    // step 3 -> write 54 bytes to dest file fwrite()
    if (fwrite(buffer, 1, 54, fptr_dest_image) != 54)
    {
        return e_failure;
    }

    // return e_success
    return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];

    for (int i = 0; i < strlen(magic_string); i++)
    {
        //step 1 -> read 8 bytes from source file
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }

        //step 2 -> call encode_byte_to_lsb(magic_string[i], buffer)
        encode_byte_to_lsb(magic_string[i], buffer);

        //step 3 -> write the buffer into dest file
        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
        // repeat this for strlen(magic_string) times from step 1
    }
    return e_success;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char buffer[32];

    //step 1 -> read 32 bytes from source file
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    //step 2 -> call encode_size_to_lsb(size, buffer)
    encode_size_to_lsb(size, buffer);

    //step 3 -> write the buffer into dest file
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];

    for (int i = 0; i < strlen(file_extn); i++)
    {
        //step 1 -> read 8 bytes from source file
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }

        //step 2 -> call encode_byte_to_lsb(file_extn[i], buffer)
        encode_byte_to_lsb(file_extn[i], buffer);

        //step 3 -> write the buffer into dest file
        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
        // repeat this for strlen(file_extn) times from step 1
    }
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];

    //step 1 -> read 32 bytes from source file
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    //step 2 -> call encode_size_to_lsb(file_size, buffer)
    encode_size_to_lsb(file_size, buffer);

    //step 3 -> write the buffer into dest file
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    // secret_data array can hold only 100 bytes
    if (encInfo->size_secret_file > (long)sizeof(encInfo->secret_data))
    {
        fprintf(stderr, "ERROR: Secret file is larger than %lu bytes, not supported\n", (unsigned long)sizeof(encInfo->secret_data));
        return e_failure;
    }

    // Move pointer to beginning of secret file
    rewind(encInfo->fptr_secret);

    //step 1 -> read secret data from file and store it into encInfo -> secret_data
    if (fread(encInfo->secret_data, 1, encInfo->size_secret_file, encInfo->fptr_secret) != encInfo->size_secret_file)
    {
        return e_failure;
    }

    char buffer[8];

    for (int i = 0; i < encInfo->size_secret_file; i++)
    {
        //step 2 -> read 8 bytes from source file
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
        {
            return e_failure;
        }

        //step 3 -> call encode_byte_to_lsb(encInfo -> secret_data[i], buffer)
        encode_byte_to_lsb(encInfo->secret_data[i], buffer);

        //step 4 -> write the buffer into dest file
        if (fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }
        // repeat this for size_secret_file times from step 2
    }
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;

    while(fread(&ch, 1, 1, fptr_src) == 1)   // Read remaining bytes
    {
        fwrite(&ch, 1, 1, fptr_dest);       // Write to destination
    }

    if(ftell(fptr_src) == ftell(fptr_dest))  // Check if both pointers match
    {
        return e_success;
    }
    return e_failure;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // Repeat for 8 bits (MSB first)
    for (int i = 0; i < 8; i++)
    {
        // Clear LSB of image byte
        image_buffer[i] = image_buffer[i] & (~1);

        // Extract bit (7 - i) from data
        int get = data & (1 << (7 - i));

        // Put data bit into image LSB
        image_buffer[i] = image_buffer[i] | (get >> (7 - i));
    }
    return e_success;
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    // Same logic as a byte, but for 32 bits (MSB first)
    for (int i = 0; i < 32; i++)
    {
        // Clear LSB of image byte
        imageBuffer[i] = imageBuffer[i] & (~1);

        // Extract bit (31 - i) from size
        int get = (size >> (31 - i)) & 1;

        // Put the bit into image LSB
        imageBuffer[i] = imageBuffer[i] | get;
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    //step 1 -> check open_files(encInfo) is returning e_success or not
    printf("INFO: Opening required files\n");
    if (open_files(encInfo) == e_failure)
    {
        //no -> print error return e_failure
        fprintf(stderr, "ERROR: Opening files failed\n");
        return e_failure;
    }
    //yes -> print success and goto next step
    printf("INFO: Opened all required files\n");

    //step 2 -> check check_capacity(encInfo) is returning e_success or not
    printf("INFO: Checking for %s size\n", encInfo->secret_fname);
    if (check_capacity(encInfo) == e_failure)
    {
        //no -> print error return e_failure
        fprintf(stderr, "ERROR: %s cannot be encoded in %s (empty file or not enough capacity)\n", encInfo->secret_fname, encInfo->src_image_fname);
        return e_failure;
    }
    //yes -> print success and goto next step
    printf("INFO: Image has enough capacity to store the secret data\n");

    //step 3 -> call copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)is returning e_success or not
    printf("INFO: Copying image header\n");
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        //no -> print error return e_failure
        fprintf(stderr, "ERROR: Copying bmp header failed\n");
        return e_failure;
    }
    //yes -> print success and goto next step
    printf("INFO: Header copied successfully\n");

    //step 4 -> call encode_magic_string(MAGIC_STRING, encInfo)
    printf("INFO: Encoding magic string signature\n");
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        fprintf(stderr, "ERROR: Encoding magic string failed\n");
        return e_failure;
    }
    printf("INFO: Done\n");

    //step 5 -> call encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file), encInfo)
    printf("INFO: Encoding secret file extension size\n");
    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_failure)
    {
        fprintf(stderr, "ERROR: Encoding extension size failed\n");
        return e_failure;
    }
    printf("INFO: Done\n");

    //step 6 -> call encode_secret_file_extn(encInfo -> extn_secret_file, encInfo)
    printf("INFO: Encoding secret file extension\n");
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
    {
        fprintf(stderr, "ERROR: Encoding extension failed\n");
        return e_failure;
    }
    printf("INFO: Done\n");

    //step 7 -> call encode_secret_file_size(encInfo ->  size_secret_file, encInfo)
    printf("INFO: Encoding secret file size\n");
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        fprintf(stderr, "ERROR: Encoding secret file size failed\n");
        return e_failure;
    }
    printf("INFO: Done\n");

    //step 8 -> call encode_secret_file_data(encInfo)
    printf("INFO: Encoding secret file data\n");
    if (encode_secret_file_data(encInfo) == e_failure)
    {
        fprintf(stderr, "ERROR: Encoding secret file data failed\n");
        return e_failure;
    }
    printf("INFO: Done\n");

    //step 9 -> copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)is returning e_success or not
    printf("INFO: Copying left over data\n");
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        //no -> print error return e_failure
        fprintf(stderr, "ERROR: Copying remaining image data failed\n");
        return e_failure;
    }
    //yes -> print success
    printf("INFO: Done\n");

    // close all files
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    // return e_success
    return e_success;
}