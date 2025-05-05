#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <common.h>
#include "file.h"
#include "parse.h"

void print_usage(char **argv)
{
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t -n created a new database file\n");
    printf("\t -f (required) path to database file\n");
}

int main(int argc, char **argv)
{
    int c = 0;
    bool newFile = false;
    char *filepath = NULL;
    struct dbheader_t *dbhd = NULL;

    while ((c = getopt(argc, argv, "nf:")) != -1)
    {
        switch (c)
        {
        case 'n':
            newFile = true;
            break;
        case 'f':
            filepath = optarg;
            break;
        case '?':
            printf("Unknown option- %c\n", c);
            break;
        default:
            return -1;
        }
    }

    if (filepath == NULL)
    {
        printf("file path is a required argument\n");
        print_usage(argv);

        return 0;
    }

    if (newFile)
    {
        int dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR)
        {
            printf("Unable to create database file\n");
            return -1;
        }
        if (create_db_header(dbfd, &dbhd) == STATUS_ERROR)
        {
            printf("Failed to create database header\n");
            return -1;
        }
    }
    else
    {
        int dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR)
        {
            printf("Unable to open database file\n");
            return -1;
        }
    }

    printf("NewFile: %d\n", newFile);
    printf("Filepath: %s\n", filepath);

    return 0;
}