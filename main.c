#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "app.h"
#include "gui.h"
#include "clinical.h"
#include "database.h"
#include "logs.h"

/**
 * @brief Função principal do sistema.
 * Ponto de entrada do programa que inicializa todos os subsistemas essenciais.
 */
int main(void) {
    // Alimenta a semente do gerador de números aleatórios com o tempo atual
    srand(time(NULL)); // time obtém o timestamp atual do sistema
    
    // Garante que a pasta e os arquivos CSV existam e possuam seus respectivos cabeçalhos
    database_init();

    // Registra no log o início da execução
    log_message(LOG_INFO, "[SISTEMA] Iniciando aplicacao..");

    // Instancia o estado principal que manterá informações globais, se necessário
    AppState app;
    app_init(&app);
    
    // Inicializa o framework GTK e cria as janelas
    if (!gui_init(&app)) {
        // Se houver falha (ex: falta de bibliotecas ou display), registra erro e encerra com código 1
        log_message(LOG_ERROR, "[SISTEMA] Nao foi possivel iniciar a interface grafica.");
        return 1;
    }
    
    // Confirma que a interface foi carregada com sucesso
    log_message(LOG_INFO, "[SISTEMA] Janela criada. Executando...");
    
    // Trava o fluxo de execução aqui para processar eventos de interface (cliques, digitação, etc)
    gui_run();
    
    // Quando o loop principal do GTK é encerrado (janela fechada), o fluxo continua aqui
    log_message(LOG_INFO, "[SISTEMA] Programa encerrado com sucesso.");
    
    // Retorna 0 para o SO indicando execução normal
    return 0;
}
