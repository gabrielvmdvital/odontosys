#include "app.h"
#include <stdio.h>

void app_init(AppState *state) {
    if (state == NULL) return;
    
    // Inicialize suas variáveis aqui
    state->valor = 0;
}
