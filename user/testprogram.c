#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
    int n, pid;
    for(n = 0; n < 20; n++) {
        pid = fork();
        if(pid < 0) {  // Error en fork
            printf("Error: fork falló\n");
            exit(1);
        }
        if(pid == 0) {  // Proceso hijo
            sleep(10 * (n + 1));  // Simular trabajo
            if(setpriority(getpid(), 20 - n) < 0) {  // Verificar si setpriority falla
                printf("Error: setpriority falló para PID %d\n", getpid());
                exit(1);
            }
            if(setboost(getpid(), n % 2 == 0 ? 1 : -1) < 0) {  // Verificar si setboost falla
                printf("Error: setboost falló para PID %d\n", getpid());
                exit(1);
            }
            printf("Ejecutando proceso %d con PID %d, Prioridad: %d, Boost %d\n",
                   n, getpid(), 20 - n, n % 2 == 0 ? 1 : -1);
            exit(0);
        }
    }
    for(n = 0; n < 20; n++) {
        wait(0);  // Esperar que todos los hijos terminen
    }
    exit(0);
}
