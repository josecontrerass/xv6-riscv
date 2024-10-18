# **INFORME Tarea 2** María Josefa Contreras Vergara
## 1. Funcionamiento y lógica del sistema de prioridades y boost
El objetivo de la modificación es implementar un sistema de programación de procesos que asigne prioridades a los procesos en ejecución. Este sistema está basado en dos nuevos campos que se agregaron a la estructura del proceso: prioridad y boost.
- Prioridad: Cada proceso tiene un valor de prioridad que afecta cuándo será programado para ejecutarse. Un número menor indica una mayor prioridad.
- Boost: El campo boost ajusta el incremento o decremento de la prioridad de un proceso. Un valor positivo aumenta la prioridad, mientras que uno negativo la reduce.

### Incorporación de Prioridades
En esta tarea, se modificó el programador de procesos en xv6 para agregar un campo de **prioridad** a cada proceso. La prioridad determina el orden en el que los procesos son programados. A menor número de prioridad, mayor será la prioridad de ejecución del proceso. Inicialmente, se estableció que la prioridad de un proceso se inicializa en `0`.

### Incorporación del Boost
Además, se agregó un campo de **boost** que ajusta dinámicamente la prioridad de los procesos. El **boost** comienza en `1`, y se ajusta de acuerdo a las siguientes reglas:
- Si la prioridad de un proceso alcanza `9`, el boost cambia a `-1`, reduciendo la prioridad.
- Si la prioridad baja a `0`, el boost vuelve a `1`.

Esta mecánica asegura que los procesos con baja prioridad reciban oportunidades de ejecución después de un ciclo de ajustes.

### **Lógica del sistema**
La lógica central del programador se basa en actualizar las prioridades en cada ciclo de ejecución. Cada proceso tiene un ciclo de prioridad que se ajusta según su boost actual. Este comportamiento se implementó en la función `scheduler()` de `proc.c`, que ajusta la prioridad de todos los procesos no zombies antes de que sean programados.


## Explicación de las modificaciones realizadas.
1. **Modificación de archivos del núcleo (`kernel/`)**:

    Se modificó archivos `proc.c`, `proc.h`, `syscall.c`, `syscall.h` y `sysproc.c`.

    - **Estructura de procesos (`proc.h`)**

        Se agregaron los campos `priority` y `boost` a la estructura `proc`:
        ```c
        int priority;
        int boost;
        ```
    Estos campos almacenan la prioridad y el ajuste dinámico de cada proceso.

    - **En sysproc.c, syscall.c, syscall.h:** se agregan llamadas al sistema. Ambas funciones fueron registradas en syscall.c y declaradas en syscall.h

        Llamada `setpriority()`

        ```
        uint64 sys_setpriority(void) {
            int pid, priority;
            argint(0, &pid);
            argint(1, &priority);
            return (uint64)set_priority(pid, priority);
        }
        ```

        Llamada `setboost()`

        ```
        uint64 sys_setboost(void) {
            int pid, boost;
            argint(0, &pid);
            argint(1, &boost);
            return (uint64)set_boost(pid, boost);
        }
        ```
    - **En proc.c:**

    En la función `allocproc()`, se inicializó la prioridad en 0 y el boost en 1:

        ```
        p->priority = 0;
        p->boost = 1;
        ```

    Se implementó la lógica para incrementar o decrementar la prioridad de un proceso en la función `scheduler()`, para ajustar el scheduler. 

2. **Modificación de archivos en carpeta de usuario y Programas de prueba en (`user/`)**

    - **En user.h** se agregó dos lineas de código:
    ```
    int setpriority(int pid, int priority);
    int setboost(int pid, int boost);
    ```

    - Creación de programa de prueba `testprogram.c`: Se creó el programa de prueba testprogram.c que genera 20 procesos usando fork. Cada proceso recibe una prioridad decreciente y un boost alternante. El programa imprime la prioridad y boost de cada proceso al ejecutarse:

3. **Modificación en Makefile de (`xv6-riscv/`):**
    - Se ajusta `Makefile` para que compile estos programas como parte del sistema.


## Dificultades encontradas y cómo se resolvieron.

1. **Conflicto con Makefile**: Al agregar las nuevas llamadas al sistema y el programa de prueba, fue necesario modificar el Makefile para compilar correctamente los programas de usuario. Se añadió el nuevo programa testprogram a las reglas del Makefile nuevamente, pues se habia cometido un error. 

2. **Errores en la compilación de los programas de usuario**: El programa `testprograma.c` presentaban errores de compilación al no encontrar las referencias de las nuevas llamadas al sistema (`setpriority()` y `setboost()`). El problema fue rastreado hasta la falta de registro correcto de las llamadas en `usys.pl`. Aquí se debio agregar dos líneas de codigo para solucionar. 

    ```
    entry("setpriority");
    entry("setboost");
    ```

3. **Manejo de limites en la prioridad**: Se encontraron problemas con los valores límite de prioridad (0 y 9). El boost no se ajustaba adecuadamente cuando los procesos alcanzaban estos valores. Esto se resolvió implementando una lógica condicional que cambia el boost de 1 a -1 y viceversa cuando la prioridad alcanza estos límites.

## Comandos de ejecución dentro de QEMU:

- Para probar la llamada y ejecutar el programa de prueba :
    ```
    $ testprogram

    ```
- Salida esperada: 
    ```
    Ejecutando proceso 0 con PID 4, Prioridad: 20, Boost 1
    Ejecutando proceso 1 con PID 5, Prioridad: 19, Boost -1
    ...
    ```