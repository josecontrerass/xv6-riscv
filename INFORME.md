# **INFORME Tarea 4** María Josefa Contreras Vergara
## Sistema de Permisos y Archivos Inmutables en xv6
El objetivo principal de esta tarea fue extender el sistema de archivos de `xv6` para incluir un sistema de permisos básico y la capacidad de marcar archivos como inmutables, mediante la implementación de permisos básicos (lectura, escritura, inmutabilidad) para archivos. Esto involucró la creación de una nueva llamada al sistema `chmod`, la adaptación de las funciones existentes para respetar los permisos asignados a cada archivo y pruebas exhaustivas para garantizar la funcionalidad. En especifico, se modifica la estructura de los inodos, fueron añadidas validaciones en las funciones principales de manejo de archivos y se creó una nueva syscall. 

### Lógica de Implementación, Lógica del sistema y Funcionamiento
**Sistema de Permisos**
Se implementó un sistema de permisos basado en bits, con los siguientes valores:
-   0: Sin permisos.
-   1: Solo lectura.
-   2: Solo escritura.
-   3: Lectura y escritura.
-   5: Inmutable (solo lectura, no permite cambiar permisos).

**Llamada al Sistema `chmod`:** Se creó una nueva syscall chmod(char *path, int mode) que permite cambiar los permisos de un archivo especificado. La lógica garantiza:
-   Validación del modo o los argumentos: Si el modo es inválido (< 0 o > 5), la llamada falla.
-   Archivos inmutables: No permite cambiar los permisos de archivos marcados como inmutables (perm == 5), esto mejora la seguridad y funcionalidad del sistema de las siguientes maneras, protege contra modificaciones accidentales y da un mayor control y eficiencia. 
-   Actualización: Cambia el campo perm en el inodo y libera los recursos de forma segura, asegurando que los cambios se reflejen en el disco.

## Explicación de las modificaciones realizadas.

1. **Modificación de archivos del núcleo (`kernel/`)**:

    Se modificó archivos `file.c`, `file.h`, `fs.c`, `fs.h`, `syscall.c`, `syscall.h` y `sysfile.c`.

    **a. Operaciones de archivos (`file.c`):** Se agrego elementos en las funciones fileread y filewrite, las cuales son fundamentales para las operaciones de lectura y escritura en los archivos. Estas funciones sirven como intermediarias entre el kernel y los programas de usuario para gestionar las operaciones de entrada/salida sobre archivos o dispositivos. 

    `fileread`: Se añadió una validación para comprobar que el archivo tiene permisos de lectura antes de proceder con la operación. Si no tiene el permiso correspondiente, la función devuelve un error.
        
        ```c
        if (!f->readable) {
            return -1; // El archivo no tiene permisos de lectura
        }
        ```

    Este cambio evita accesos indebidos a archivos protegidos.

    `filewrite`: Los cambios realizados tienen la finalidad de bloquear escrituras en archivos sin permisos de escritura (f->writable == 0) y proteger archivos marcados como inmutables (perm == 5). 
        
        Se añadió lógica para validar permisos antes de permitir escritura o lectura.

        ```
        if ((f->ip->perm & 2) == 0) {
            return -1; // No tiene permiso de escritura
        }
        if (f->ip->perm == 5) {
            printf("filewrite: No se permite escribir en un archivo inmutable\n");
            return -1; // Archivo inmutable
        }
        ``` 

    **b. En sysfile.c, syscall.c, syscall.h:** Definen y registran las nuevas llamadas al sistema.

        - Se añade las entradas de la nueva llamada al sistema (SYS_chmod) en el arreglo syscalls. Esto permite que el kernel reconozca y redirija la llamada a su respectiva función.

        - Definiciones para la nueva llamada al sistema SYS_chmod en `syscall.h`. Para asignar un número único a la nueva llamada al sistema, permitiendo al kernel identificar correctamente.
        
        - Adiciones en `sysfile.c`:

        1. Modificación de la función sys_open()

        ```
        if ((ip->perm & 0x2) == 0 && (omode & O_WRONLY || omode & O_RDWR)) {
            iunlockput(ip);
            end_op();
            return -1; // Error: No se permite la escritura
        }
        ```

        2. Difinir sys_chmod(): extrae los argumentos desde el espacio de usuario y llama a chmod para ejecutar la operación.

        ```
        uint64 sys_chmod(void) {
            char path[MAXPATH];
            int mode;

            if (argstr(0, path, sizeof(path)) < 0)
                return -1;
            argint(1, &mode);
            return chmod(path, mode);
        }
        ```

        Propósito:
        -   Verificar los permisos del archivo al momento de abrirlo.
        -   Si el archivo no tiene el permiso de escritura (perm & 0x2), y el modo de apertura requiere escritura (O_WRONLY o O_RDWR), la operación falla.
        -   Garantiza que no se puedan abrir archivos de solo lectura o inmutables para operaciones de escritura.

    Las operaciones relacionadas con archivos, como sys_open, sys_write, y ahora sys_chmod, están centralizadas en `sysfile.c`, siguiendo la lógica modular de xv6.


    **c. En fs.h:** La estructura de los inodos, definida en el archivo fs.h, se amplió para incluir un nuevo campo perm, que almacena los permisos asociados a cada archivo.

        Cambio realizado:

        ```
        struct inode {
            ...
            int perm; // Nuevo campo para permisos: 0, 1, 2, 3, 5
            ...
        };
        ```

    Inicialización: La función ialloc, que asigna nuevos inodos, fue modificada en fs.c para inicializar este campo con el valor 3, indicando que los archivos recién creados tienen permisos de lectura y escritura por defecto.

    Se implementó la función chmod en fs.c, encargada de cambiar los permisos de un archivo. Esta función realiza las siguientes validaciones:
    -   Verifica que el modo de permisos proporcionado sea válido.
    -   Comprueba que el archivo no sea inmutable, en cuyo caso rechaza la solicitud.
    -   Actualiza el campo perm en el inodo y asegura que los cambios se reflejen en disco mediante iupdate.

    Además, las funciones relacionadas con la apertura y escritura de archivos, como sys_open y filewrite en file.c, fueron ajustadas para respetar estos permisos. Por ejemplo, si un archivo tiene permisos de solo lectura, no se permite abrirlo en modo escritura, y si tiene permisos inmutables, cualquier intento de modificación es bloqueado.

    **d. En fs.c:**  Implementación de la función chmod, se implementa su lógica para cambiar los permisos de archivos.

        ```
        int chmod(char *path, int mode) {
            struct inode *ip = 0;

            if (mode < 0 || mode > MAX_PERM) {
                end_op();
                return -1;
            }

            begin_op();
            if ((ip = namei(path)) == 0) {
                end_op();
                return -1; // Archivo no encontrado
            }

            ilock(ip);
            if (ip->perm == 5) { // Archivo inmutable
                iunlockput(ip);
                end_op();
                return -1;
            }

            ip->perm = mode; // Cambiar permisos
            iupdate(ip);     // Actualizar en disco
            iunlockput(ip);
            end_op();
            return 0; // Éxito
        }

        ```

    `chmod` Verifica que el modo sea válido y que el archivo no sea inmutable antes de realizar cambios.


2. **Modificación de archivos en carpeta de usuario y Programa de prueba en (`user/`)**

    - **En user.h** se agregó las declaraciones:
    
    ```
    int chmod(char*, int);
    ```

    - **Creación de programa de prueba `chmod_prueba.c`:** para validar las funcionalidades implementadas:
        -   Crear un archivo y escribir datos en él.
        -   Cambiar los permisos a solo lectura y verificar que no se pueda escribir.
        -   Cambia los permisos a inmutable y verificar que no se puedan modificar ni los datos ni los permisos.
    Las pruebas aseguran que los permisos sean respetados en todas las operaciones críticas del sistema de archivos.

3. **Modificación en Makefile de (`xv6-riscv/`):**
    - Se añadió el archivo `chmod_prueba.c` ($U/_chmod_prueba) en el Makefile para compilarlo como parte del sistema y hacer que esté disponible en la imagen de xv6.

## Dificultades encontradas y cómo se resolvieron.

1. **Restricciones de Permisos:** Se ajustó la lógica para garantizar que los archivos inmutables no puedan ser modificados ni reconfigurados.

2. **Pruebas en el Espacio de Usuario:** Diseñar casos de prueba para cubrir todas las combinaciones de permisos (lectura, escritura, inmutabilidad).

3. **Problemas con la estructura dinode:** Inicialmente, el campo perm fue añadido directamente a la estructura dinode en el archivo fs.h. Esto provocó un error relacionado con el tamaño de la estructura dinode y la alineación de los bloques de disco en xv6. El mensaje de error fue:

    ```
    mkfs: mkfs.c:90: main: Assertion `(BSIZE % sizeof(struct dinode)) == 0' failed.
    ```
El problema se debió a que el tamaño de la estructura dinode ya no se ajustaba correctamente al tamaño de bloque (BSIZE), violando una de las suposiciones fundamentales del sistema de archivos en xv6. Para resolver esto, el campo perm fue trasladado a la estructura inode, donde desde un inicio debia estar y que se maneja únicamente en memoria, evitando así modificar el diseño en disco y los problemas de alineación.

4. **Errores en el Makefile:** Resolver problemas de compilación al integrar el programa de prueba.

## Comandos de ejecución dentro de QEMU:

- Para probar y ejecutar el programa de prueba:

    ```
    $ chmod_prueba

    ```
- Salida esperada: Se muestra un mensaje detallado con los intentos de cambio de permiso, escritura. 

    ```
    Creado archivo: testfile
    Permisos cambiados a solo lectura
    Intentando abrir en modo escritura (esperando fallo)...
    Bloqueo de escritura verificado correctamente
    Permisos restaurados exitosamente
    Permisos cambiados a inmutable
    Bloqueo de cambio de permisos desde inmutable verificado
    Intentando abrir archivo inmutable en modo escritura...
    Bloqueo de escritura para archivo inmutable verificado
    Pruebas completadas con éxito
    ```