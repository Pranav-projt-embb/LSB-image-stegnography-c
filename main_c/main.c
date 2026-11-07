#include <stdio.h>
#include "encode.h"
#include "decoding.h"
#include "types.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[]) // .aout -e <source_file> <secret_data_file> <optional>
{
    // no arguments -> print usage
    if (argc < 2)
    {
        printf("Usage:\n");
        printf("Encoding : ./a.out -e <.bmp_file> <.text_file> [output file]\n");
        printf("Decoding : ./a.out -d <.bmp_file> [output file]\n");
        return 1;
    }

    //step 1 -> check check_operation_type(argv[1]) is returning e_encode or not
    OperationType op = check_operation_type(argv[1]);

    if (op == e_encode)
    {
        // encode needs at least: -e <bmp> <secret>, and at most one optional output
        if (argc < 4 || argc > 5)
        {
            printf("Usage: ./a.out -e <.bmp_file> <.text_file> [output file]\n");
            return 1;
        }

        // yes -> EncodeInfo encInfo;
        EncodeInfo encInfo;

        // check read_and_validate_encode_args(argv, &encInfo) is returning e_success or e_failure
        if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            // failure -> print error msg and stop
            fprintf(stderr, "ERROR: Read and validate encode arguments failed\n");
            return 1;
        }
        // success -> print success msg
        printf("INFO: Read and validate encode arguments is successful\n");

        // check do_encoding(&encInfo) is returning e_success or e_failure
        if (do_encoding(&encInfo) == e_failure)
        {
            // failure -> print error msg and stop
            fprintf(stderr, "ERROR: Encoding failed\n");
            return 1;
        }
        // success -> print success msg and stop
        printf("INFO: Encoding Done Successfully\n");
    }
    else if (op == e_decode)
    {
        // decode needs: -d <bmp>, and one optional output file
        if (argc < 3 || argc > 4)
        {
            printf("Usage: ./a.out -d <.bmp_file> [output file]\n");
            return 1;
        }

        DecodeInfo decInfo;

        if (read_and_validate_decode_args(argv, &decInfo) == e_failure)
        {
            fprintf(stderr, "ERROR: Read and validate decode arguments failed\n");
            return 1;
        }
        printf("INFO: Read and validate decode arguments is successful\n");

        if (do_decoding(&decInfo) == e_failure)
        {
            fprintf(stderr, "ERROR: Decoding failed\n");
            return 1;
        }
        printf("INFO: Decoding Done Successfully\n");
    }
    else
    {
        // unsupported option
        printf("Usage:\n");
        printf("Encoding : ./a.out -e <.bmp_file> <.text_file> [output file]\n");
        printf("Decoding : ./a.out -d <.bmp_file> [output file]\n");
        return 1;
    }

    return 0;
}

OperationType check_operation_type(char *symbol)
{
    //step 1 -> check symbol is -e or not
    if (strcmp(symbol, "-e") == 0)
    {
        // yes -> return e_encode
        return e_encode;
    }

    // step 2 -> check symbol is -d or not
    if (strcmp(symbol, "-d") == 0)
    {
        // yes -> return e_decode
        return e_decode;
    }

    // return e_unsupported
    return e_unsupported;
}
