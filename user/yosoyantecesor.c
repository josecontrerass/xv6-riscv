#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <n>\n", argv[0]);
        exit(1);
    }
    int n = atoi(argv[1]);
    int pid = getancestor(n);
    if (pid == -1) {
        printf("Ancestro no encontrado\n");
    } else {
        printf("PID del ancestro %d: %d\n", n, pid);
    }
    exit(0);
}