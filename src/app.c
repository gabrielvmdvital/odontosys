#include "app.h"
#include "database.h"
#include <stdio.h>
#include <locale.h>

/*
 * Inicializa o estado principal da aplicacao e do banco de dados
 */
void app_init(AppState *state) {
    if (state == NULL) return;
    
    // Inicializa variaveis de estado
    state->valor = 0;
    database_init();
}
