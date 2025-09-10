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
    int32_t r1 = (int32_t) mv.registros[10], r2 = (int32_t) mv.registros[11];
    printf("EAX = %x = %d\nEBX = %x = %d\n", mv.registros[10], r1, mv.registros[11], r2);

    scanf("%s", prueba);


    return 0;
}
