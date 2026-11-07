#include <stdio.h>
#include <string.h>
#include "decoding.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *dot;

    // step 1 -> check argv[2] is having .bmp or not
    dot = strrchr(argv[2], '.');
    if (dot == NULL || strcmp(dot, ".bmp") != 0)
    {
        fprintf(stderr, "ERROR: Stego image %s is not a .bmp file\n", argv[2]);
        return e_failure;
    }
    // yes -> store the file name into decInfo -> stego_image_fname = argv[2]
    decInfo->stego_image_fname = argv[2];

    // step 2 -> check argv[3] is NULL or not
    if (argv[3] != NULL)
    {
        // no -> user gave the output file name
        decInfo->output_fname = argv[3];
    }
    else
    {
        // yes -> default name will be made after extension is decoded
        decInfo->output_fname = NULL;
    }

    // return e_success
    return e_success;
}

Status open_stego_file(DecodeInfo *decInfo)
{
    // Stego Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status skip_bmp_header(FILE *fptr_stego_image)
{
    // step 1 -> move file pointer to 54th byte (after the header)
    if (fseek(fptr_stego_image, 54, SEEK_SET) != 0)
    {
        return e_failure;
    }
    return e_success;
}

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;

    for (int i = 0; i < strlen(magic_string); i++)
    {
        //step 1 -> read 8 bytes from stego image
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }

        //step 2 -> call decode_byte_from_lsb(buffer)
        ch = decode_byte_from_lsb(buffer);

        //step 3 -> compare with magic string character
        if (ch != magic_string[i])
        {
            return e_failure;
        }
        // repeat this for strlen(magic_string) times from step 1
    }
    return e_success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];

    //step 1 -> read 32 bytes from stego image
    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    //step 2 -> call decode_size_from_lsb(buffer) and store the size
    decInfo->extn_size = decode_size_from_lsb(buffer);

    // extn_output_file can hold only 4 characters + '\0'
    if (decInfo->extn_size < 0 || decInfo->extn_size > 4)
    {
        return e_failure;
    }
    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];

    for (int i = 0; i < decInfo->extn_size; i++)
    {
        //step 1 -> read 8 bytes from stego image
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }

        //step 2 -> call decode_byte_from_lsb(buffer) and store the character
        decInfo->extn_output_file[i] = decode_byte_from_lsb(buffer);
        // repeat this for extn_size times from step 1
    }
    // end the extension string
    decInfo->extn_output_file[decInfo->extn_size] = '\0';

    return e_success;
}

Status open_output_file(DecodeInfo *decInfo)
{
    // user did not give output name -> default name = "decoded" + extension
    if (decInfo->output_fname == NULL)
    {
        strcpy(decInfo->default_fname, "decoded");
        strcat(decInfo->default_fname, decInfo->extn_output_file);
        decInfo->output_fname = decInfo->default_fname;
    }

    // Output file
    decInfo->fptr_output = fopen(decInfo->output_fname, "w");
    // Do Error handling
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->output_fname);

        return e_failure;
    }
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];

    //step 1 -> read 32 bytes from stego image
    if (fread(buffer, 1, 32, decInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    //step 2 -> call decode_size_from_lsb(buffer) and store the size
    decInfo->size_output_file = decode_size_from_lsb(buffer);

    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;

    for (long i = 0; i < decInfo->size_output_file; i++)
    {
        //step 1 -> read 8 bytes from stego image
        if (fread(buffer, 1, 8, decInfo->fptr_stego_image) != 8)
        {
            return e_failure;
        }

        //step 2 -> call decode_byte_from_lsb(buffer)
        ch = decode_byte_from_lsb(buffer);

        //step 3 -> write the character into output file
        fwrite(&ch, 1, 1, decInfo->fptr_output);
        // repeat this for size_output_file times from step 1
    }
    return e_success;
}

char decode_byte_from_lsb(char *image_buffer)
{
    char ch = 0;

    // Repeat for 8 bits (MSB first)
    for (int i = 0; i < 8; i++)
    {
        ch = ((ch << 1) | (image_buffer[i] & 1));
    }
    return ch;
}

int decode_size_from_lsb(char *image_buffer)
{
    int size = 0;

    // Same logic as a byte, but for 32 bits (MSB first)
    for (int i = 0; i < 32; i++)
    {
        size = ((size << 1) | (image_buffer[i] & 1));
    }
    return size;
}

Status do_decoding(DecodeInfo *decInfo)
{
    //step 1 -> check open_stego_file(decInfo) is returning e_success or not
    printf("INFO: Opening required files\n");
    if (open_stego_file(decInfo) == e_failure)
    {
        //no -> print error return e_failure
        fprintf(stderr, "ERROR: Opening stego image failed\n");
        return e_failure;
    }
    //yes -> print success and goto next step
    printf("INFO: Opened %s\n", decInfo->stego_image_fname);

    //step 2 -> skip the 54 bytes bmp header
    if (skip_bmp_header(decInfo->fptr_stego_image) == e_failure)
    {
        fprintf(stderr, "ERROR: Skipping bmp header failed\n");
        return e_failure;
    }

    //step 3 -> call decode_magic_string(MAGIC_STRING, decInfo)
    printf("INFO: Decoding magic string signature\n");
    if (decode_magic_string(MAGIC_STRING, decInfo) == e_failure)
    {
        fprintf(stderr, "ERROR: Magic string not matching, %s is not a stego image\n", decInfo->stego_image_fname);
        return e_failure;
    }
    printf("INFO: Done\n");

    //step 4 -> call decode_secret_file_extn_size(decInfo)
    printf("INFO: Decoding output file extension size\n");
    if (decode_secret_file_extn_size(decInfo) == e_failure)
    {
        fprintf(stderr, "ERROR: Decoding extension size failed\n");
        return e_failure;
    }
    printf("INFO: Done\n");

    //step 5 -> call decode_secret_file_extn(decInfo)
    printf("INFO: Decoding output file extension\n");
    if (decode_secret_file_extn(decInfo) == e_failure)
    {
        fprintf(stderr, "ERROR: Decoding extension failed\n");
        return e_failure;
    }
    printf("INFO: Done\n");

    //step 6 -> create the output file
    printf("INFO: Opening output file\n");
    if (open_output_file(decInfo) == e_failure)
    {
        fprintf(stderr, "ERROR: Creating output file failed\n");
        return e_failure;
    }
    printf("INFO: Opened %s\n", decInfo->output_fname);

    //step 7 -> call decode_secret_file_size(decInfo)
    printf("INFO: Decoding file size\n");
    if (decode_secret_file_size(decInfo) == e_failure)
    {
        fprintf(stderr, "ERROR: Decoding file size failed\n");
        return e_failure;
    }
    printf("INFO: Done\n");

    //step 8 -> call decode_secret_file_data(decInfo)
    printf("INFO: Decoding file data\n");
    if (decode_secret_file_data(decInfo) == e_failure)
    {
        fprintf(stderr, "ERROR: Decoding file data failed\n");
        return e_failure;
    }
    printf("INFO: Done\n");

    // close all files
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    // return e_success
    return e_success;
}
