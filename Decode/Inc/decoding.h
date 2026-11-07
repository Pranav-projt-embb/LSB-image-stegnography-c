#ifndef DECODING_H
#define DECODING_H
#include <stdio.h>

#include "types.h" // Contains user defined types

/*
 * Structure to store information required for
 * decoding the secret file from the stego image
 */

typedef struct _DecodeInfo
{
    /* Stego Image info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Output (decoded secret) file info */
    char *output_fname;          // name given by user (NULL if not given)
    char default_fname[20];      // default name -> "decoded" + extension
    FILE *fptr_output;
    char extn_output_file[5];    // extension decoded from image
    int extn_size;               // size of extension decoded from image
    long size_output_file;       // size of secret data decoded from image

} DecodeInfo;

/* Decoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointer for stego image */
Status open_stego_file(DecodeInfo *decInfo);

/* Skip the 54 bytes bmp header */
Status skip_bmp_header(FILE *fptr_stego_image);

/* Decode and check magic string */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/* Decode extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Create the output file */
Status open_output_file(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data and write into output file */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* Decode a byte from LSBs of 8 image bytes */
char decode_byte_from_lsb(char *image_buffer);

/* Decode a size (int) from LSBs of 32 image bytes */
int decode_size_from_lsb(char *image_buffer);

#endif
