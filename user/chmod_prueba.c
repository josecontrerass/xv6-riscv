#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "user.h"
#include "../kernel/fcntl.h"

// Función para verificar errores
void check_error(int result, const char *message) {
    if (result < 0) {
        printf("Error: %s\n", message);
        exit(1);
    }
}

int main() {
    char *filename = "testfile";
    int fd;

    // Crear archivo
    fd = open(filename, O_CREATE | O_RDWR);
    check_error(fd, "creando archivo");
    check_error(close(fd), "cerrando archivo");

    printf("Creado archivo: %s\n", filename);

    // Cambiar permisos a solo lectura
    check_error(chmod(filename, 1), "cambiando permisos a solo lectura");
    printf("Permisos cambiados a solo lectura\n");

    // Intentar abrir en modo escritura (debería fallar)
    printf("Intentando abrir en modo escritura (esperando fallo)...\n");
    fd = open(filename, O_WRONLY);
    if (fd >= 0) {
        printf("Error: Se pudo abrir en escritura un archivo solo lectura\n");
        close(fd);
        exit(1);
    }
    printf("Bloqueo de escritura verificado correctamente\n");

    // Restaurar permisos
    check_error(chmod(filename, 3), "restaurando permisos");
    printf("Permisos restaurados exitosamente\n");

    // Prueba de permisos inmutables
    check_error(chmod(filename, 5), "cambiando permisos a inmutable");
    printf("Permisos cambiados a inmutable\n");

    // Intentar cambiar permisos desde inmutable
    if (chmod(filename, 3) == 0) {
        printf("Error: Se cambiaron permisos desde inmutable\n");
        exit(1);
    }
    printf("Bloqueo de cambio de permisos desde inmutable verificado\n");

    // Intentar abrir en modo escritura (debería fallar)
    printf("Intentando abrir archivo inmutable en modo escritura...\n");
    fd = open(filename, O_WRONLY);
    if (fd >= 0) {
        printf("Error: Se pudo abrir en escritura un archivo inmutable\n");
        close(fd);
        exit(1);
    }
    printf("Bloqueo de escritura para archivo inmutable verificado\n");

    printf("Pruebas completadas con éxito\n");
    exit(0);
}
