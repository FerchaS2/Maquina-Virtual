#include "mv.h"
#include "loader.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main()
{
    MV mv;
    int err;
    char prueba[25];
    ini_mv(&mv);
    const char * filename = "sample.vmx";
    carga_prog(&mv, filename, &err);
    printf("%x", mv.memoria[1]);
    scanf("%s", prueba);

    return 0;
}
