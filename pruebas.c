#include "mv.h"
#include "decoder.h"
#include "loader.h"
#include "instrucciones.h"
#include "memory.h"

int main()
{
    MV mv;
    int err;
    char prueba[25];
    ini_mv(&mv);
    const char * filename = "pruebas.vmx";
    carga_prog(&mv, filename, &err);
    //printf("%x", mv.memoria[0]);

    ejecutar(&mv);
    printf("EAX = %x = %d\nEBX = %x = %d\n", mv.registros[10], mv.registros[10], mv.registros[11], mv.registros[11]);

    scanf("%s", prueba);


    return 0;
}
