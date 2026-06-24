#include "gui.h"
#include "logs.h"
#include <string.h>

/**
 * @brief Callback disparado quando o botão "Cadastrar Dentista" (Apenas Administrador) é clicado.
 */
static void on_btn_cadastrar_dentista_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    log_message(LOG_INFO, "[GUI] Botão de menu clicado: Cadastrar Dentista");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "cadastro_dentista_page");
}

/**
 * @brief Callback disparado ao clicar no botão "Sair do Sistema".
 * Realiza o "logout" redirecionando para a tela de Login.
 */
static void on_btn_sair_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    log_message(LOG_INFO, "[GUI] Logout do sistema. Voltando para Login...");
    gtk_widget_set_visible(g_btn_cadastrar_dentista, FALSE); 
    
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "login_page");
}

/**
 * @brief Callback unificado para gerenciar o roteamento dos botões do menu principal.
 * Utiliza o texto embutido (label) do botão clicado para identificar qual tela carregar.
 */
static void on_menu_button_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    const char *label = gtk_button_get_label(btn);

    if (strstr(label, "Cadastrar Pacientes")) {
        log_message(LOG_INFO, "[GUI] Botão de menu clicado: Cadastrar Pacientes");
        gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "cadastro_page");
    } else if (strstr(label, "Gerenciar Prontuários")) {
        log_message(LOG_INFO, "[GUI] Botão de menu clicado: Gerenciar Prontuários");
        atualizar_lista_prontuarios("");
        gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuarios_page");
    }
}

/**
 * @brief Constrói a Tela 2 - Menu Principal (Dashboard).
 * Interface central que possibilita a navegação para os diferentes módulos.
 */
GtkWidget* criar_tela_dashboard(void) {
    // Container principal: Alinhado verticalmente no centro e centralizado horizontalmente
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);

    GtkWidget *lbl_painel = gtk_label_new("ODONTOSYS - Painel Principal");
    gtk_widget_set_margin_bottom(lbl_painel, 20);
    gtk_box_append(GTK_BOX(vbox), lbl_painel);

    // Criação dos Botões de Navegação dos Módulos
    GtkWidget *btn_prontuarios = gtk_button_new_with_label("📁 Gerenciar Prontuários");
    GtkWidget *btn_pacientes = gtk_button_new_with_label("👥 Cadastrar Pacientes");

    gtk_widget_set_size_request(btn_prontuarios, 250, 40);
    gtk_widget_set_size_request(btn_pacientes, 250, 40);

    // Associando todos os botões de módulos a um único evento roteador genérico
    g_signal_connect(btn_prontuarios, "clicked", G_CALLBACK(on_menu_button_clicked), NULL);
    g_signal_connect(btn_pacientes, "clicked", G_CALLBACK(on_menu_button_clicked), NULL);

    gtk_box_append(GTK_BOX(vbox), btn_prontuarios);
    gtk_box_append(GTK_BOX(vbox), btn_pacientes);

    // --- MÓDULO EXCLUSIVO DE ADMIN ---
    g_btn_cadastrar_dentista = gtk_button_new_with_label("👨‍⚕️ Cadastrar Dentista");
    gtk_widget_set_size_request(g_btn_cadastrar_dentista, 250, 40);
    g_signal_connect(g_btn_cadastrar_dentista, "clicked", G_CALLBACK(on_btn_cadastrar_dentista_clicked), NULL);
    gtk_widget_set_visible(g_btn_cadastrar_dentista, FALSE); // Por padrão, o botão começa invisível

    gtk_box_append(GTK_BOX(vbox), g_btn_cadastrar_dentista);

    // Botão de Logout separado visualmente pela margem superior
    GtkWidget *btn_sair = gtk_button_new_with_label("🚪 Sair do Sistema");
    gtk_widget_set_size_request(btn_sair, 250, 40);
    gtk_widget_set_margin_top(btn_sair, 20); 
    
    g_signal_connect(btn_sair, "clicked", G_CALLBACK(on_btn_sair_clicked), NULL);
    
    gtk_box_append(GTK_BOX(vbox), btn_sair);

    return vbox;
}
