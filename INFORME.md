# **INFORME Tarea 3** María Josefa Contreras Vergara
## Funcionamiento y Lógica de la Protección de Memoria
El objetivo principal es implementar un sistema de protección de memoria en xv6 mediante dos nuevas llamadas al sistema: mprotect y munprotect. Estos permiten modificar los permisos de páginas específicas de la memoria de usuario, de manera que puedan ser marcadas como solo lectura o restauradas a su estado original.
-   mprotect(void *addr, int len): Esta función recibe una dirección addr y una longitud len, y cambia las páginas especificadas para ser de solo lectura. Es útil para proteger regiones de memoria de cambios accidentales, mejorando la seguridad y la estabilidad de los programas.
-   munprotect(void *addr, int len): Esta función deshace los efectos de mprotect, restaurando el permiso de escritura en las páginas especificadas. De esta manera, las páginas marcadas previamente como solo lectura pueden ser modificadas nuevamente.

### Lógica de Implementación, Lógica del sistema
-   Cada llamada toma una dirección y un número de páginas a modificar.
-   Se utiliza la función walk() en vm.c para recorrer la tabla de páginas del proceso y acceder a las entradas (PTE) de cada página.
-   Protección (mprotect): Se desactiva el bit PTE_W (escritura) de cada entrada de página para hacerla de solo lectura.
-   Desprotección (munprotect): Se reactiva el bit PTE_W de cada entrada de página para restaurar los permisos de escritura.
-   Se usa sfence_vma() en ambas funciones para asegurar que los cambios en la tabla de páginas se reflejen en la TLB, garantizando la coherencia en los permisos de memoria.

## Explicación de las modificaciones realizadas.

1. **Modificación de archivos del núcleo (`kernel/`)**:

    Se modificó archivos `proc.h`, `trap.c`, `vm.c`, `syscall.c`, `syscall.h` y `sysproc.c`.

    - **Estructura de procesos (`proc.h`):** Se agrega declaraciones de campos `mprotect` y `munprotect` para que estén disponibles en otros archivos del kernel.

        ```c
        int mprotect(void *addr, int len);
        int munprotect(void *addr, int len);
        ```


    - **En sysproc.c, syscall.c, syscall.h:** Definen y registran las nuevas llamadas al sistema, se agregan llamadas al sistema. Ambas funciones fueron registradas en syscall.c y declaradas en syscall.h.

        - Se añade las entradas de las nuevas llamadas al sistema (SYS_mprotect y SYS_munprotect) en el arreglo syscalls. Esto permite que el kernel reconozca y redirija las llamadas mprotect y munprotect a sus respectivas funciones.

        - Definiciones para las nuevas llamadas al sistema SYS_mprotect y SYS_munprotect en `syscall.h`. Para asignar un número único a cada nueva llamada al sistema, permitiendo al kernel identificar correctamente mprotect y munprotect.
        
        - Adiciones en `sysproc.c`:

        Llamada `mprotect()`
        ```
        uint64 sys_mprotect(void) {
            uint64 addr;
            int len;

            argaddr(0, &addr);  // Obtener el argumento sin esperar retorno
            argint(1, &len);    // Obtener el segundo argumento sin esperar retorno

            return mprotect((void*)addr, len);
        }
        ```

        Llamada `numprotect()`
        ```
        uint64 sys_munprotect(void) {
            uint64 addr;
            int len;

            argaddr(0, &addr);  // Obtener el argumento sin esperar retorno
            argint(1, &len);    // Obtener el segundo argumento sin esperar retorno

            return munprotect((void*)addr, len);
        }
        ```

        Las funciones sys_mprotect y sys_munprotect reciben la dirección y el tamaño de la memoria a proteger o desproteger y llaman a las funciones de memoria (mprotect y munprotect) implementadas en vm.c.

    - **En trap.c:** Agrega manejo de errores para capturar violaciones de acceso y manejar adecuadamente intentos de escritura en memoria de solo lectura.

        Se incluye código para capturar errores de protección de página, generando un mensaje que indica el PID del proceso que intentó escribir en una página de solo lectura. Esto permite al kernel detectar y reportar accesos ilegales, mostrando el PID y la dirección de memoria del intento de acceso. Además, da una mejor visualización en la imagen

    - **En vm.c:** Toda la funcionalidad de protección de memoria para `mprotect` y `munprotect` se manejó en vm.c, donde reside la lógica de la administración de memoria.

        Define las funciones de protección y desprotección de memoria, manejando la configuración de permisos de lectura y escritura. Estas funciones recorren la tabla de páginas del proceso para modificar los permisos de las páginas de memoria, activando o desactivando el bit de solo lectura (PTE_W).

        ```
        int mprotect(void *addr, int len) {
            if (addr == 0 || len <= 0) return -1;  // Validación de argumentos

            uint64 addr_aligned = PGROUNDDOWN((uint64) addr);  // Alinear dirección al inicio de página
            uint64 end_addr = (uint64) addr + len * PGSIZE;    // Dirección final de la región

            for (uint64 a = addr_aligned; a < end_addr; a += PGSIZE) {
                pte_t *pte = walk(myproc()->pagetable, a, 0);  // Obtén la PTE sin crear una nueva
                if (!pte || (*pte & PTE_V) == 0)  // Verificar que PTE es válida y presente
                    return -1;

                *pte &= ~PTE_W;  // Desactivar el bit de escritura para solo lectura
            }

            sfence_vma();  // Recargar TLB en RISC-V
            return 0;
        }
        ```
        ```
        int munprotect(void *addr, int len) {
            if (addr == 0 || len <= 0) return -1;  // Validación de argumentos
            ...
        }
        ```

    `mprotect` desactiva el permiso de escritura para hacer que las páginas sean de solo lectura, mientras que `munprotect` restaura los permisos de escritura.

    - Agregar sfence_vma() en mprotect y munprotect es una medida preventiva y de seguridad que refresca los permisos de acceso en la TLB, evita accesos no autorizados según los permisos modificados y asegura que los cambios en la tabla de páginas se reflejan inmediatamente en el hardware. Esto se agrego debido a un error surgido en la ejecución. 

2. **Modificación de archivos en carpeta de usuario y Programa de prueba en (`user/`)**

    - **En user.h** se agregó las declaraciones:
    ```
    int mprotect(void *addr, int len);
    int munprotect(void *addr, int len);
    ```

    - **Creación de programa de prueba `mprotect_test.c`:** Este programa verifica que `mprotect` y `munprotect` funcionen como se espera.
        -   Reserva de Memoria: Usa sbrk para reservar una página.
        -   Protección de Memoria: Se aplica mprotect para hacer la página de solo lectura.
        -   Intento de Escritura: El programa intenta escribir en la página protegida, lo cual debería fallar.
        -   Desprotección de Memoria: Con munprotect se restauraran los permisos de escritura.
        -   Escritura Después de Desprotección: Se realiza una escritura que debería tener éxito después de la desprotección.

3. **Modificación en Makefile de (`xv6-riscv/`):**
    - Se añadió el archivo `mprotect_test.c` en el Makefile para compilarlo como parte del sistema y hacer que esté disponible en la imagen de xv6.

## Dificultades encontradas y cómo se resolvieron.

1. **Conflicto con Makefile:** Al agregar las nuevas llamadas al sistema y el programa de prueba, fue necesario modificar el Makefile para compilar correctamente los programas de usuario. Se añadió el nuevo programa testprogram a las reglas del Makefile nuevamente, pues se habia cometido un error. 

2. **Modificación de PTE:** La manipulación de permisos en las PTE puede ser compleja; se ajustaron los bits de escritura. 

3. **Captura de Errores:** Fue necesario añadir en `trap.c` un manejo específico para detectar violaciones de permisos de página.
    
    El mensaje obtenido por consola al ejecutar el programa:
    ```
    usertrap(): unexpected scause 0xf pid=3
            sepc=0x54 stval=0x4000
    ```
4. **Importación de archivos necesarios:** En `vm.c` se incluye "spinlock.h" para la corrección de errores arrojados por consola e incluye "proc.h", con la finalidad de poder utilizar elementos como PGROUNDDOWN o PGSIZE. 

## Comandos de ejecución dentro de QEMU:

- Para probar y ejecutar el programa de prueba:

    ```
    $ mprotect_test

    ```
- Salida esperada: Al intentar escribir en una página protegida, se muestra un mensaje de error similar a
    ```
    Memoria asignada en la dirección 0x0000000000004000
    mprotect fue exitoso: página en 0x0000000000004000 ahora es de solo lectura.
    Intentando escribir en la página protegida en 0x0000000000004000...
    ```

    ```
    "Intento de escritura en página de solo lectura. PID=[PID del proceso]".
    ```