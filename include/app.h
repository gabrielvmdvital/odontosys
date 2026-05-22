#ifndef APP_H
#define APP_H

/**
 * @brief Estrutura que representa o estado global do programa.
 * Guarde todas as variáveis de estado aqui (pontuação, posições, dados, etc).
 */
typedef struct {
    int valor; // Exemplo de dado do estado da aplicação
} AppState;

/**
 * @brief Inicializa o estado do programa com valores padrão.
 */
void app_init(AppState *state);

#endif // APP_H
