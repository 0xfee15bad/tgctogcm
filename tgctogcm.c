// tgctogcm.c -- Plootid 2004

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#if defined(__linux__) || defined(__unix__)
#include <arpa/inet.h>
#elif defined(_WIN32)
#include <winsock.h>
#endif

int main(int argc, char **argv)
{
    FILE        *tgcin, *gcmout;
    uint32_t    tgcheader[14];
    uint32_t    temp[3];
    int         i;

    if (argc < 2)
    {
        printf("Usage: %s <infile.tgc> <outfile.gcm>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if ((tgcin = fopen(argv[1], "rb")) == NULL)
    {
        printf("Error: Couldn't open file %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    fread(tgcheader, sizeof(tgcheader[0]), (sizeof(tgcheader) / sizeof(tgcheader[0])), tgcin);

    for (i = 0; i < (int)(sizeof(tgcheader) / (sizeof(tgcheader[0]))); i++)
    {
        tgcheader[i] = ntohl(tgcheader[i]); //Byte-swap
    }
    tgcheader[4] = tgcheader[4] - tgcheader[2]; // Update FST location
    tgcheader[7] = tgcheader[7] - tgcheader[2]; // Update DOL location
    tgcheader[9] = tgcheader[9] - tgcheader[2]; // Update File Area location
    //  for (i = 0; i < 14; i++) printf("%08X\n", tgcheader[i]); // Print tgc header

    if (tgcheader[0] != 0xAE0F38A2)
    {
        printf("Error: Input file does not appear to be a .tgc file\n");
        fclose(tgcin);
        return EXIT_FAILURE;
    }

    if (!(gcmout = fopen(argv[2], "w+b")))
    {
        printf("Error: Couldn't open file %s\n", argv[2]);
        fclose(tgcin);
        return EXIT_FAILURE;
    }

    printf("TGCtoGCM by Plootid 2004\n\n");

    printf("Target filesize: %d bytes\n", (tgcheader[9] + tgcheader[10]));
    printf("Copying GCM, please wait...\n");
    fseek(tgcin, tgcheader[2], SEEK_SET);
    while ((i = getc(tgcin)) != EOF)
    {
        putc(i, gcmout); //Copy gcm from tgc
    }
    fflush(gcmout);
    fclose(tgcin);

    printf("Updating GCM...\n");

    // Update DOL and FST locations
    temp[0] = htonl(tgcheader[7]);
    temp[1] = htonl(tgcheader[4]);
    fseek(gcmout, 0x420L, SEEK_SET);
    fwrite(temp, 4, 2, gcmout);

    // Update FST
    fseek(gcmout, tgcheader[4], SEEK_SET);
    fread(temp, 4, 3, gcmout);
    for (i = (ntohl(temp[2]) - 1); i > 0; i--)
    {
        fread(temp, 4, 3, gcmout);
        if (!(ntohl(temp[0]) & 0x01000000))
        {
            temp[1] = htonl(ntohl(temp[1]) - tgcheader[13] + tgcheader[9]);
            fseek(gcmout, -12, SEEK_CUR);
            fwrite(temp, 4, 3, gcmout);
            fflush(gcmout);
        }
    }
    fclose(gcmout);
    printf("All Done!\n");
    return EXIT_SUCCESS;
}
