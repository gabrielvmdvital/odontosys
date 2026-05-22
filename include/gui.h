#ifndef GUI_H
#define GUI_H

#include <stdbool.h>
#include "app.h"

/**
 * @brief Inicializa a janela principal do Windows e associa o estado do programa.
 */
bool gui_init(AppState *app_state);

/**
 * @brief Roda o loop principal de processamento de mensagens do Windows.
 */
void gui_run(void);

#endif // GUI_H
