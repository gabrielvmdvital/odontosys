#include "gui.h"
#include <gtk/gtk.h>
#include <stdio.h>

// ============================================================================
// VARIÁVEIS GLOBAIS ESTÁTICAS (Escopo do Arquivo)
// ============================================================================
static AppState *g_app_state = NULL;       // Guarda o estado do core do programa
static GtkApplication *g_gtk_app = NULL;   // Controle do ciclo de vida do GTK
static GtkWidget *g_stack = NULL;          // O GtkStack global que controlará a troca de telas

// ============================================================================
// ESTRUTURAS DE DADOS (PONTES DE COMUNICAÇÃO)
// ============================================================================
/**
 * @brief Estrutura que agrupa os componentes da tela de login.
 * É usada para passar múltiplos dados/componentes para a função de clique do botão.
 */
typedef struct {
    GtkWidget *entry_usuario; // Ponteiro para o campo de texto do usuário
    GtkWidget *entry_senha;   // Ponteiro para o campo de texto da senha
} LoginCampos;

// ============================================================================
// FUNÇÕES DE CALLBACK (EVENTOS)
// ============================================================================

/**
 * @brief Callback disparado quando o botão "Entrar" é clicado.
 * Esta função captura os textos e faz a transição de tela usando o GtkStack.
 */
static void on_btn_entrar_clicked(GtkButton *btn, gpointer user_data) {
    // 1. Recuperamos a struct com os campos de texto através do ponteiro genérico
    LoginCampos *campos = (LoginCampos *)user_data;

    // 2. Extraímos as strings que o usuário digitou
    const char *usuario = gtk_editable_get_text(GTK_EDITABLE(campos->entry_usuario));
    const char *senha = gtk_editable_get_text(GTK_EDITABLE(campos->entry_senha));

    // 3. Print de debug no terminal (útil para testes acadêmicos)
    printf("\n[GUI] Botão Entrar Clicado.\n");
    printf("[GUI] Usuário inserido: %s\n", usuario);
    
    // 4. TRANSIÇÃO DE TELA (O Segredo do GtkStack):
    // Como o 'g_stack' é global, nós dizemos para ele mudar a página visível 
    // para a página que batizamos de "dashboard_page".
    // Sem validação por enquanto, conforme o escopo do Trello.
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
}

/**
 * @brief Callback disparado quando o botão "Sair" da Dashboard é clicado.
 * Faz o GtkStack reverter a animação de volta para a tela de login.
 */
static void on_btn_sair_clicked(GtkButton *btn, gpointer user_data) {
    printf("[GUI] Usuário deslogou da sessão. Retornando ao Login...\n");
    
    // Faz o Stack exibir a tela de login novamente com animação reversa automática
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "login_page");
}

/**
 * @brief Callback temporário para os botões do menu interno.
 */
static void on_menu_button_clicked(GtkButton *btn, gpointer user_data) {
    const char *label = gtk_button_get_label(btn);
    printf("[GUI] Botão do menu clicado: %s (Espaço reservado para a equipe)\n", label);
    
    // [FUTURAMENTE AQUI MANDAREMOS O STACK EXIBIR A TELA 3 DE PRONTUÁRIOS]
}

// ============================================================================
// CONSTRUTORES DE INTERFACES (TELAS)
// ============================================================================

/**
 * @brief Constrói e organiza os elementos visuais da Tela de Login.
 * @return Retorna um GtkWidget (GtkBox) contendo toda a interface de login pronta.
 */
static GtkWidget* criar_tela_login(LoginCampos *campos) {
    // Criamos uma caixa vertical com 10 pixels de espaçamento entre os componentes
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER); // Centraliza horizontalmente na tela
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER); // Centraliza verticalmente na tela

    // Componente Título
    GtkWidget *lbl_titulo = gtk_label_new("ODONTOSYS");
    gtk_widget_set_margin_bottom(lbl_titulo, 15); 
    gtk_box_append(GTK_BOX(vbox), lbl_titulo);

    // Componente Campo Usuário
    GtkWidget *lbl_usuario = gtk_label_new("Usuário:");
    gtk_widget_set_halign(lbl_usuario, GTK_ALIGN_START); // Garante alinhamento à esquerda
    gtk_box_append(GTK_BOX(vbox), lbl_usuario);

    campos->entry_usuario = gtk_entry_new();
    gtk_widget_set_size_request(campos->entry_usuario, 250, -1); // Define largura de 250px
    gtk_box_append(GTK_BOX(vbox), campos->entry_usuario);

    // Componente  Campo Senha
    GtkWidget *lbl_senha = gtk_label_new("Senha:");
    gtk_widget_set_halign(lbl_senha, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_senha);

    campos->entry_senha = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(campos->entry_senha), FALSE); // Mascara os caracteres (***)
    gtk_box_append(GTK_BOX(vbox), campos->entry_senha);

    // Area do Botão de Envio
    GtkWidget *btn_entrar = gtk_button_new_with_label("Entrar");
    gtk_widget_set_margin_top(btn_entrar, 15);
    
    // Conecta o evento de clique do botão à nossa função de callback 
    g_signal_connect(btn_entrar, "clicked", G_CALLBACK(on_btn_entrar_clicked), campos);
    
    gtk_box_append(GTK_BOX(vbox), btn_entrar);

    return vbox; // Retorna a caixa completa para ser inserida no Stack
}

/**
 * @brief Constrói e organiza a interface real da Tela 2 (Dashboard / Painel de Controle).
 * @return Retorna um GtkWidget (GtkBox) com o menu principal ajustado sem agendamentos.
 */
static GtkWidget* criar_tela_dashboard(void) {
    // 1. Criamos a caixa vertical principal com 15 pixels de espaçamento
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);

    // 2. Título do Painel de Controle
    GtkWidget *lbl_painel = gtk_label_new("ODONTOSYS - Painel Principal");
    gtk_widget_set_margin_bottom(lbl_painel, 20);
    gtk_box_append(GTK_BOX(vbox), lbl_painel);

    // 3. Criando apenas os botões de funcionalidades reais do escopo acordado
    GtkWidget *btn_prontuarios = gtk_button_new_with_label("📁 Gerenciar Prontuários");
    GtkWidget *btn_pacientes = gtk_button_new_with_label("👥 Cadastrar Pacientes");

    // Definindo o tamanho padrão idêntico para simetria visual
    gtk_widget_set_size_request(btn_prontuarios, 250, 40);
    gtk_widget_set_size_request(btn_pacientes, 250, 40);

    // Conectando os botões ao callback de clique do menu
    g_signal_connect(btn_prontuarios, "clicked", G_CALLBACK(on_menu_button_clicked), NULL);
    g_signal_connect(btn_pacientes, "clicked", G_CALLBACK(on_menu_button_clicked), NULL);

    // Empacotando os botões na caixa vertical
    gtk_box_append(GTK_BOX(vbox), btn_prontuarios);
    gtk_box_append(GTK_BOX(vbox), btn_pacientes);

    // 4. Criando o Botão "Sair" com destaque inferior
    GtkWidget *btn_sair = gtk_button_new_with_label("🚪 Sair do Sistema");
    gtk_widget_set_size_request(btn_sair, 250, 40);
    gtk_widget_set_margin_top(btn_sair, 25); // Margem para separar visualmente das funções
    
    // Conecta o evento de clique para reverter para a tela de login
    g_signal_connect(btn_sair, "clicked", G_CALLBACK(on_btn_sair_clicked), NULL);
    
    gtk_box_append(GTK_BOX(vbox), btn_sair);

    return vbox;
}

// ============================================================================
// INICIALIZAÇÃO E FLUXO PRINCIPAL DO GTK
// ============================================================================

/**
 * @brief Evento principal de ativação do GTK. Constrói a janela e a pilha de telas.
 */
static void on_app_activate(GtkApplication *app, gpointer user_data) {
    // Instancia a estrutura dos campos de forma estática para persistir na memória do app
    static LoginCampos campos;

    // 1. Criação da janela principal do Windows
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "ODONTOSYS");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);

    // 2. Criação do componente GtkStack (Gerenciador de Empilhamento de Telas)
    g_stack = gtk_stack_new();
    
    // Configura uma animação de transição suave (deslizar para os lados) ao mudar de tela
    gtk_stack_set_transition_type(GTK_STACK(g_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(g_stack), 300); // 300 milissegundos de efeito

    // 3. Construção das telas independentes
    GtkWidget *layout_login = criar_tela_login(&campos);
    GtkWidget *layout_dashboard = criar_tela_dashboard();

    // 4. Adicionando as telas dentro do Stack e dando um "nome" de identificación para cada uma
    gtk_stack_add_named(GTK_STACK(g_stack), layout_login, "login_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_dashboard, "dashboard_page");

    // 5. Define qual página o Stack deve exibir assim que o programa abrir
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "login_page");

    // 6. Define o GtkStack como o filho principal (conteúdo) da nossa janela
    gtk_window_set_child(GTK_WINDOW(window), g_stack);

    // 7. Renderiza a janela na tela do usuário
    gtk_window_present(GTK_WINDOW(window));
}

bool gui_init(AppState *app_state) {
    if (app_state == NULL) return false;
    g_app_state = app_state;

    g_gtk_app = gtk_application_new("com.odontosys.app", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(g_gtk_app, "activate", G_CALLBACK(on_app_activate), NULL);

    return true;
}

void gui_run(void) {
    if (g_gtk_app != NULL) {
        g_application_run(G_APPLICATION(g_gtk_app), 0, NULL);
        g_object_unref(g_gtk_app);
    }
}