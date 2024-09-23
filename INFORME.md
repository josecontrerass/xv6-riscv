# INFORME Tarea 1
## Funcionamiento de las llamadas al sistema.
### Llamada al sistema `getppid()`
Esta llamada se encarga de retornar el ID del proceso padre que invoca la llamada. Esta llamada es similar a la existente `getpid()`, pero en lugar de devolver el ID del actual proceso, obtiene el ID del proceso padre (conocido como "parent").

En el archivo `sysproc.c`, se implementa `sys_getppid()` que accede al proceso actual utilizando `myproc()` y retorna el PID del proceso padre. Este valor se encuentra en la estructura del proceso como `p->parent->pid`.


### Llamada al sistema `getancestor()`


## Explicación de las modificaciones realizadas.
1. **Modificación de archivos del núcleo (`kernel/`)**:
   - Se modificó archivos `sysproc.c`, `syscall.c`, y `syscall.h` para incluir las nuevas llamadas al sistema.
    **En sysproc.c:** 
    ```
    uint64
    sys_getppid(void) {
        struct proc *p = myproc();
        return p->parent->pid;
    }
    ```
    ```
    uint64
    sys_getancestor(void) {
        int n;
        argint(0, &n);  // Obtenemos el valor de n desde los argumentos de la llamada
        struct proc *p = myproc();
        for (int i = 0; i < n; i++) {
            if (p->parent == 0)  // Si no hay más ancestros
                return -1;
            p = p->parent;
        }
        return p->pid;
    }
    ```
   - Se creó la función `sys_getppid()` que devuelve el PID del padre del proceso.
   - Se implementó la función `sys_getancestor()` que recorre la cadena de procesos padres y retorna el PID del ancestro o `-1`.
   - En `syscall.h`, se agregó las constantes `SYS_getppid` y `SYS_getancestor`.
   - En `syscall.c`, se enlazaron las nuevas llamadas al sistema con sus funciones respectivas.

2. **Modificación de archivos en carpeta de usuario y Programas de prueba en (`user/`)**
    - **En user.h_** se agregó dos lineas de código:
    ```
    int getppid(void);
    int getancestor(int n);
    ```
   - Creación de programas de prueba `yosoytupadre.c` y `yosoyantecesor.c`:
     - `yosoytupadre.c` utiliza `getppid()` para retornar o imprimir el PID del proceso padre.
     - `yosoyantecesor.c` utiliza `getancestor(n)` para mostrar por consola el PID del ancestro solicitado o informar si no existe.

3. **Modificación en Makefile de (`xv6-riscv/`):**
    - Se ajusta `Makefile` para que compile estos programas como parte del sistema.


## Dificultades encontradas y cómo se resolvieron.

1. **Error de compilación en QEMU relacionado con registros RISC-V**:
   Durante la ejecución de `make qemu`, surgieron errores relacionados con registros RISC-V (`menvcfg`, `stimecmp`). Para resolver este problema, se investigaron los registros específicos utilizados en el archivo `riscv.h`, y se comentaron o eliminaron temporalmente las líneas relacionadas con esos registros, ya que no eran necesarios para el correcto funcionamiento de las llamadas al sistema implementadas.

2. **Errores en la compilación de los programas de usuario**:
   Los programas `yosoytupadre.c` y `yosoyantecesor.c` presentaban errores de compilación al no encontrar las referencias de las nuevas llamadas al sistema (`getppid()` y `getancestor()`). El problema fue rastreado hasta la falta de registro correcto de las llamadas en `syscall.c` y `syscall.h`.

3. **Fallo en la ejecución del comando `yosoyantecesor 0`**:
   Durante la prueba con `yosoyantecesor 0`, inicialmente se produjo un fallo al ejecutar el comando. Esto se debió a un error en la implementación de `getancestor()`, donde no se manejaba adecuadamente el caso en que se solicitaba el ancestro `0` (el mismo proceso). Se corrigió el código para que `getancestor(0)` devolviera correctamente el PID del propio proceso.

## Comandos de ejecución dentro de QEMU:

    - Para probar la llamada `getppid()` :
    ```
    $ yosoytupadre
    ```

    - Para probar llamada `getancestor()`:
    ```
    $ yosoyantecesor 0
    $ yosoyantecesor 1
    $ yosoyantecesor 2
    $ yosoyantecesor 3
    ```