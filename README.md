# Maquina-Virtual
    Trabajo práctico máquina virtual FI - UNDMP

    Materia: Fundamentos de la Arquitectura de Computadoras

## Resumen

    Programa: Máquina Virtual (MV) implementada en C.
    Desarrollo en: Windows 10 (compilada y probada en VSCode con gcc / MinGW).

    La MV carga archivos binarios .vmx, ejecuta las instrucciones definidas en el enunciado y opcionalmente muestra un desensamblado con -d.

    Formato: vmx [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 param2 ... paramN]

### Contenido del repositorio (principales)

 - src/ — código fuente en C (main.c, mv.c, loader.c, memory.c, decoder.c, instrucciones.c, desensamblador.c, etc.).

 - vmx.exe — (opcional) ejecutable ya compilado.

 - .vscode/tasks.json — task para compilar desde VSCode con gcc (opcional).

### Requerimientos previos

 - Windows 10 (entorno de desarrollo).

 - GCC (MinGW) instalado.

 - Visual Studio Code con extensión C/C++.

### Cómo ejecutar

Desde la carpeta del proyecto en la consola:

    cd C:\ruta\del\proyecto

    vmx [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 param2 ... paramN]

Donde:

- vmx es el programa ejecutable del proceso Ejecutor o Máquina Virtual.
- filename.vmx (opcional*) es la ruta y nombre del archivo con el programa en lenguaje máquina (puede ser cualquier nombre con extensión .vmx).
- filename.vmi (opcional*) es la ruta y nombre del archivo donde se almacenará o desde donde se cargará la imagen del estado de la máquina virtual (puede ser cualquier nombre con extensión .vmi).
- m=M (opcional) permite indicar el tamaño de la memoria principal, donde M es un valor expresado en KiB. Si se omite, el valor por defecto sigue siendo 16 KiB.
- -d (opcional) es un flag que fuerza a la máquina virtual a mostrar el código Assembler correspondiente al código máquina cargado en la memoria principal, junto con el contenido del Const Segment (KS).
- -p (opcional) es un flag que sirve para indicar los parámetros (param1 param2 ... paramN) que se le deben pasar a la subrutina principal del proceso. Siempre se debe escribir al final del comando.

Nota: para la ejecución es obligatorio al menos uno de los dos archivos: .vmx y/o .vmi.
En caso de no especificarse un archivo .vmx, se ignoran los parámetros -p.