#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FRAME_LEN 18

int hex_to_byte(const char *hex)
{
    int value;
    sscanf(hex, "%x", &value);
    return value & 0xFF;
}

int main(void)
{
    FILE *in = fopen("input.txt", "r");
    FILE *out = fopen("output.csv", "w");

    if (!in || !out)
    {
        perror("Datei konnte nicht geöffnet werden");
        return 1;
    }

    char line[256];
    unsigned char frame[FRAME_LEN];
    int collecting = 0;
    int index = 0;

    while (fgets(line, sizeof(line), in))
    {
        char *p = strstr(line, "RX data:");
        if (!p)
            continue;

        char hex[3] = {0};
        p += 9; // hinter "RX data:"
        while (*p == ' ')
            p++;

        if (!isxdigit(p[0]) || !isxdigit(p[1]))
            continue;

        hex[0] = p[0];
        hex[1] = p[1];

        unsigned char byte = hex_to_byte(hex);

        if (!collecting)
        {
            if (byte == 0xF3)
            {
                collecting = 1;
                index = 0;
                frame[index++] = byte;
            }
        }
        else
        {
            frame[index++] = byte;

            if (index == FRAME_LEN)
            {
                // CSV-Zeile schreiben
                for (int i = 0; i < FRAME_LEN; i++)
                {
                    fprintf(out, "%02X", frame[i]);
                    if (i < FRAME_LEN - 1)
                        fprintf(out, ",");
                }
                fprintf(out, "\n");

                collecting = 0;
                index = 0;
            }
        }
    }

    fclose(in);
    fclose(out);

    printf("Fertig! CSV-Datei erstellt.\n");
    return 0;
}
