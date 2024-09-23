#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int main(void) {
    printf("PID del padre: %d\n", getppid());
    exit(0);
}