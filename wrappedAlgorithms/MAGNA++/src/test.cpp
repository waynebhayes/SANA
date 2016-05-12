#include "carrier.h"


void mg_quit(int status) {
    exit(status);
}
void mg_error(const char *fmt, ...) {
    va_list arg;
    va_start(arg,fmt);
    vfprintf(stderr,fmt,arg);
    va_end(arg);
    fprintf(stderr,"\n");
}


int main(int argc, char **argv) {

    FILE *fid;
    if (argc < 2) return 0;
    fid = fopen(argv[1],"r");
    char *line;
    while (1) {
        if ((line = read_line(fid)) == NULL) break;
        printf("-%s-\n",line);
        free(line);
    }
    fclose(fid);
    return 0;
}
    
