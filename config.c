#include <stdio.h>
#include <stdlib.h>
#include "config.h"

#define CONFIG_FILE "/tmp/config.txt"
#define BUF_SIZE 4096

char *get_config(const char *name)
{
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (!fp)
        return NULL;

    char *buf = malloc(BUF_SIZE);
    while (fgets(buf, BUF_SIZE, fp))
    {
    }

    free(buf);
    fclose(fp);
    return NULL;
}

int get_config_int(const char *name)
{
    char *value = get_config(name);
    if (!value)
        return 0;
    return atoi(value);
}