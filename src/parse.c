#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employee, char *addString)
{
    printf("%s\n", addString);

    char *name = strtok(addString, ",");
    char *addr = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");

    printf("%s %s %s\n", name, addr, hours);

    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut)
{
    if (fd < 0)
    {
        printf("Got a bad fd from user\n");
        return STATUS_ERROR;
    }

    int count = dbhdr->count;
    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL)
    {
        close(fd);
        perror("calloc");
        return STATUS_ERROR;
    }

    if (read(fd, employees, sizeof(struct employee_t) * count) != sizeof(struct employee_t) * count)
    {
        close(fd);
        perror("read");
        return STATUS_ERROR;
    }

    for (int i = 0; i < count; i++)
    {
        employees[i].hours = ntohl(employees[i].hours);
    }
    *employeesOut = employees;

    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr)
{
    if (fd < 0)
    {
        printf("Got a bad fd from user\n");
        return STATUS_ERROR;
    }

    dbhdr->count = htons(dbhdr->count);
    dbhdr->version = htons(dbhdr->version);
    dbhdr->filesize = htonl(dbhdr->filesize);
    dbhdr->magic = htonl(dbhdr->magic);

    lseek(fd, 0, SEEK_SET);
    if (write(fd, dbhdr, sizeof(*dbhdr)) != sizeof(*dbhdr))
    {
        perror("write");
        close(fd);
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut)
{
    if (fd < 0)
    {
        printf("Got a bad fd from the user\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL)
    {
        printf("Calloc failed to allocate memory for dbheader\n");
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t))
    {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->filesize = ntohl(header->filesize);
    header->magic = ntohl(header->magic);

    if (header->version != 1)
    {
        close(fd);
        printf("Improper header version\n");
        return STATUS_ERROR;
    }

    if (header->magic != HEADER_MAGIC)
    {
        close(fd);
        printf("Improper header magic\n");
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    if (fstat(fd, &dbstat) == -1)
    {
        perror("fstat");
        free(header);
        return STATUS_ERROR;
    }

    if (dbstat.st_size != header->filesize)
    {
        close(fd);
        printf("Corrupted database\n");
        return STATUS_ERROR;
    }

    *headerOut = header;

    return STATUS_SUCCESS;
}

int create_db_header(int fd, struct dbheader_t **headerOut)
{
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL)
    {
        printf("Calloc failed to allocate memory for dbheader\n");
        return STATUS_ERROR;
    }

    header->magic = HEADER_MAGIC;
    header->version = 0x1;
    header->count = 0;
    header->filesize = sizeof(struct dbheader_t);
    *headerOut = header;

    return STATUS_SUCCESS;
}
