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
    printf("[GUI] Botão do menu clicado: %s\n", label);

    if (g_str_has_prefix(label, "📁")) {
        gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuarios_page");
    }
    // Se clicar no botão de Pacientes (👥), abre o cadastro
    else if (g_str_has_prefix(label, "👥")) {
        gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "cadastro_page");
    }
}

/**
 * @brief Callback para o botão "Voltar" da Tela de Prontuários.
 * Faz o GtkStack deslizar de volta para a Dashboard.
 */
static void on_btn_voltar_prontuarios_clicked(GtkButton *btn, gpointer user_data) {
    printf("[GUI] Retornando dos Prontuários para a Dashboard...\n");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
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

/**
 * @brief Constrói o Passo 1 (Visual) da Tela 3 - Gerenciar Prontuários.
 */
static GtkWidget* criar_tela_prontuarios(void) {
    // Caixa principal vertical com margens internas
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    // --- LINHA SUPERIOR (Botão Voltar e Título) ---
    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_prontuarios_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);

    GtkWidget *lbl_titulo = gtk_label_new("Gerenciamento de Prontuários");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);
    
    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // --- BARRA DE BUSCA (Passo 2 do seu Fluxograma - Futuro) ---
    GtkWidget *hbox_busca = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget *entry_busca = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_busca), "Buscar paciente pelo nome...");
    gtk_widget_set_hexpand(entry_busca, TRUE);
    gtk_box_append(GTK_BOX(hbox_busca), entry_busca);

    GtkWidget *btn_buscar = gtk_button_new_with_label("🔍 Buscar");
    gtk_box_append(GTK_BOX(hbox_busca), btn_buscar);

    gtk_box_append(GTK_BOX(vbox), hbox_busca);

    // --- LISTA DE PRONTUÁRIOS (Dados simulados / Mocados) ---
    GtkWidget *lbl_lista = gtk_label_new("Prontuários Recentes:");
    gtk_widget_set_halign(lbl_lista, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_lista);

    GtkWidget *listbox = gtk_list_box_new();
    gtk_widget_set_vexpand(listbox, TRUE); // Faz a lista ocupar o espaço restante

    // Adicionando pacientes fictícios para o protótipo
    gtk_list_box_append(GTK_LIST_BOX(listbox), gtk_label_new("  📝 João Silva (Invisalign - Manutenção)"));
    gtk_list_box_append(GTK_LIST_BOX(listbox), gtk_label_new("  📝 Maria Souza (Canal Pendor)"));
    gtk_list_box_append(GTK_LIST_BOX(listbox), gtk_label_new("  📝 Carlos Eduardo (Avaliação Inicial)"));

    gtk_box_append(GTK_BOX(vbox), listbox);

    return vbox;
}

/**
 * @brief Callback para o botão "Voltar" da Tela de Cadastro de Pacientes.
 */
static void on_btn_voltar_cadastro_clicked(GtkButton *btn, gpointer user_data) {
    printf("[GUI] Retornando do Cadastro para a Dashboard...\n");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
}

/**
 * @brief Constrói o Passo 1 (Visual) da Tela 4 - Cadastrar Pacientes.
 */
static GtkWidget* criar_tela_cadastro_pacientes(void) {
    // Caixa principal vertical com espaçamento e margens
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    // --- TOPO (Botão Voltar e Título) ---
    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_cadastro_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);

    GtkWidget *lbl_titulo = gtk_label_new("Novo Cadastro de Paciente");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);
    
    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // --- FORMULÁRIO DE CADASTRO (Passo 1 - Desenhar Visual) ---
    GtkWidget *lbl_nome = gtk_label_new("Nome Completo:");
    gtk_widget_set_halign(lbl_nome, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_nome);
    GtkWidget *entry_nome = gtk_entry_new();
    gtk_box_append(GTK_BOX(vbox), entry_nome);

    GtkWidget *lbl_cpf = gtk_label_new("CPF:");
    gtk_widget_set_halign(lbl_cpf, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_cpf);
    GtkWidget *entry_cpf = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_cpf), "000.000.000-00");
    gtk_box_append(GTK_BOX(vbox), entry_cpf);

    GtkWidget *lbl_telefone = gtk_label_new("Telefone / WhatsApp:");
    gtk_widget_set_halign(lbl_telefone, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_telefone);
    GtkWidget *entry_telefone = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_telefone), "(81) 99999-9999");
    gtk_box_append(GTK_BOX(vbox), entry_telefone);

    // --- BOTÃO SALVAR ---
    GtkWidget *btn_salvar = gtk_button_new_with_label("💾 Salvar Ficha");
    gtk_widget_set_margin_top(btn_salvar, 15);
    gtk_widget_set_size_request(btn_salvar, -1, 40);
    gtk_box_append(GTK_BOX(vbox), btn_salvar);

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
    GtkWidget *layout_prontuarios = criar_tela_prontuarios();
    GtkWidget *layout_cadastro = criar_tela_cadastro_pacientes(); // <-- INCLUÍDO NO PASSO 3

    // 4. Adicionando as telas dentro do Stack e dando um "nome" de identificação para cada uma
    gtk_stack_add_named(GTK_STACK(g_stack), layout_login, "login_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_dashboard, "dashboard_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_prontuarios, "prontuarios_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_cadastro, "cadastro_page"); // <-- INCLUÍDO NO PASSO 4

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