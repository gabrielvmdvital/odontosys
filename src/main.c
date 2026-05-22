#include <stdio.h>
#include "app.h"
#include "gui.h"

int main(void) {
    printf("[INICIALIZACAO] Iniciando aplicacao modular em C...\n");
    
    // 1. Inicializa o estado
    AppState app;
    app_init(&app);
    
    // 2. Inicializa a interface gráfica
    if (!gui_init(&app)) {
        printf("[ERRO] Nao foi possivel iniciar a interface grafica.\n");
        return 1;
    }
    
    printf("[SISTEMA] Janela criada. Executando...\n");
    
    // 3. Loop principal
    gui_run();
    
    printf("[FINALIZACAO] Programa encerrado com sucesso.\n");
    return 0;
}
