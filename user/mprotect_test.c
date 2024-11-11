#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"

int main() {
    // Reservar una página de memoria
    char *addr = sbrk(4096);
    if (addr == (char *)-1) {
        printf("Error al asignar memoria con sbrk.\n");
        exit(1);
    }
    printf("Memoria asignada en la dirección %p\n", addr);

    // Proteger la página con mprotect
    if (mprotect(addr, 1) == -1) {
        printf("Error: mprotect falló.\n");
        exit(1);
    } else {
        printf("mprotect exitoso: la página es de solo lectura.\n");
    }

    // Intentar escribir en la página protegida
    printf("Intentando escribir en la página protegida...\n");
    *addr = 'A';  // Esto debería fallar

    printf("ERROR: La escritura en una página de solo lectura debería haber fallado.\n");

    // Desproteger la página con munprotect
    if (munprotect(addr, 1) == -1) {
        printf("Error: munprotect falló.\n");
        exit(1);
    } else {
        printf("munprotect exitoso: la página es de lectura/escritura.\n");
    }

    // Intentar escribir en la página desprotegida
    *addr = 'B'; 
    printf("Escritura exitosa después de munprotect. Valor en %p: %c\n", addr, *addr);

    exit(0);
}