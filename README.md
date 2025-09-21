# Maquina-Virtual
Trabajo práctico máquina virtual FI - UNDMP

## Resumen

    Programa: Máquina Virtual (MV) implementada en C.
    Desarrollo en: Windows 10 (compilada y probada en VSCode con gcc / MinGW).

    La MV carga archivos binarios .vmx, ejecuta las instrucciones definidas en el enunciado y opcionalmente muestra un desensamblado con -d.

    Formato: vmx filename.vmx -d

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
    vmx.exe archivo.vmx [-d]

 - archivo.vmx — archivo binario del programa a cargar.

 - -d — opcional: activa el modo desensamblador, muestra el código en assembler del programa.
