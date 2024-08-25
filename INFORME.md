# INFORME DE INSTALACIÓN DE XV6

## Pasos Seguidos para Instalar xv6
1. Clonar el repositorio oficial de xv6
    git clone https://github.com/mit-pdos/xv6-riscv.git
    cd xv6-riscv
2. Crear una nueva rama
    git checkout -b josecontreras_t0
3. Instalar las dependencias necesarias
    sudo apt-get update && sudo apt-get upgrade
    sudo apt-get install build-essential qemu-system-riscv64 gcc-riscv64-unknown-elf


4. Clonar toolchain en ~/xv6-riscv
    git clone https://github.com/riscv/riscv-gnu-toolchain
    cd riscv-gnu-toolchain
    - Configuración de toolchain:
    ./configure --prefix=/opt/riscv
    ./configure --prefix=$HOME/riscv
    make
    sudo make install

5. Configurar qemu
    Ejecutar el comando para configurar QEMU para la arquitectura RISC-V:
        cd riscv-gnu-toolchain/qemu
        ./configure --target-list=riscv64-softmmu
        - Este comando asegura que QEMU esté configurado correctamente para emular la arquitectura RISC-V necesaria para ejecutar xv6.
        make
        sudo make install
6. Se agrega el PATH
    export PATH=$PATH:/opt/riscv/bin
    source ~/.bashrc

7. Compilar xv6:
    make
Como no funciono en este paso, se realizó:
    - Se intentó una compilación adicional de QEMU para resolver el problema:
      ```bash
      git clone https://github.com/qemu/qemu.git
      cd qemu
      ./configure --target-list=riscv64-softmmu
      make
      sudo make install
      ```
    - Después de esto, la compilación de xv6 se completó exitosamente.

7. Ejecutar xv6 en QEMU:
    make qemu

## Problemas Encontrados y Soluciones
- Problema: Falta del paquete tomli.
Solución: Instalé el paquete usando pip install tomli.
    $ pip install tomli
- Problema: Error de permisos durante la instalación. (usado solo en algunos casos)
Solución: Utilicé sudo para realizar la instalación con permisos adecuados.
- Problema: Herramientas riscv no encontradas por el sistema.
Solución: Añadí la ruta de las herramientas de riscv al PATH en mi archivo de configuración del shell (.bashrc).
    export PATH=$PATH:/opt/riscv/bin
    source ~/.bashrc

Ejecuté los comandos ls, echo "Hola xv6", y cat README en QEMU, y todos funcionaron correctamente.
*se adjunta pantallazo en entrega

## Pasos realizados en git  
Para moverse entre ramas:
    git checkout riscv
    git checkout -b josecontreras_t0
En la rama josecontreras_t0, para agregar y subir cambios
    git add .
    #Configuración de usuario
    git config --global user.name "María Josefa Contreras"
    git config --global user.email "maria01josefa@gmail.com"
    #PAra el commit
    git commit -m "Instalación y ejecución de xv6"
    git push origin josecontreras_t0 
    #PAra corroborar o rehacer el fork:
    git remote add fork https://github.com/josecontrerass/xv6-riscv.git
    git push fork josecontreras_t0 