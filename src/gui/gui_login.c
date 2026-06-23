#include "gui.h"
#include "logs.h"
#include "dentist.h"
#include <string.h>

/**
 * @brief Callback disparado quando o botão "Entrar" é clicado.
 * Executa a lógica de extração dos textos, validação dummy de login e troca de tela.
 */
static void on_btn_entrar_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    LoginCampos *campos = (LoginCampos *)user_data;

    const char *usuario = gtk_editable_get_text(GTK_EDITABLE(campos->entry_usuario));
    const char *senha = gtk_editable_get_text(GTK_EDITABLE(campos->entry_senha));

    log_message(LOG_INFO, "[GUI] Botão Entrar Clicado.");
    log_message(LOG_INFO, "[GUI] Usuário inserido: %s", usuario);

    char clean_user[100];
    strcpy(clean_user, usuario); // strcpy copia a string da origem para o destino
    
    int is_cpf = 1;
    for (int i = 0; usuario[i] != '\0'; i++) {
        if (!((usuario[i] >= '0' && usuario[i] <= '9') || usuario[i] == '.' || usuario[i] == '-')) {
            is_cpf = 0;
            break;
        }
    }
    
    if (is_cpf) {
        clean_cpf(usuario, clean_user);
    }

    int role = validate_login(clean_user, senha);
    if (role != -1) {
        Dentist d = find_dentist_by_cpf(clean_user);
        g_logged_dentist_id = d.dentist_id;

        // Muda a tela visual do aplicativo para a dashboard do usuário
        gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
        
        if (role == 1) {
            gtk_widget_set_visible(g_btn_cadastrar_dentista, TRUE);
        } else {
            gtk_widget_set_visible(g_btn_cadastrar_dentista, FALSE);
        }

        gtk_editable_set_text(GTK_EDITABLE(campos->entry_usuario), "");
        gtk_editable_set_text(GTK_EDITABLE(campos->entry_senha), "");
    } else {
        log_message(LOG_WARNING, "[GUI] Login falhou para o usuário: %s", usuario);
    }
}

/**
 * @brief Constrói a Tela 1 - Interface de Login do Sistema.
 * Recebe o ponteiro da struct de campos para mapear os widgets criados na memória da main().
 */
GtkWidget* criar_tela_login(LoginCampos *campos) {
    // Container vertical principal com espacamento interno padrao
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER); 
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER); 

    // Titulo da tela de login
    GtkWidget *lbl_titulo = gtk_label_new("ODONTOSYS");
    gtk_widget_set_margin_bottom(lbl_titulo, 15); 
    gtk_box_append(GTK_BOX(vbox), lbl_titulo);

    // Rotulo e campo de entrada para o nome de usuario ou CPF
    GtkWidget *lbl_usuario = gtk_label_new("Usuário ou CPF:");
    gtk_widget_set_halign(lbl_usuario, GTK_ALIGN_START); 
    gtk_box_append(GTK_BOX(vbox), lbl_usuario);

    campos->entry_usuario = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_usuario), "Usuário ou CPF");
    gtk_widget_set_size_request(campos->entry_usuario, 250, -1); 
    gtk_box_append(GTK_BOX(vbox), campos->entry_usuario);

    // Rotulo e campo de entrada de senha
    GtkWidget *lbl_senha = gtk_label_new("Senha:");
    gtk_widget_set_halign(lbl_senha, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_senha);

    campos->entry_senha = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(campos->entry_senha), FALSE); // Oculta o texto digitado

    gtk_box_append(GTK_BOX(vbox), campos->entry_senha);

    // Botao para confirmar o login
    GtkWidget *btn_entrar = gtk_button_new_with_label("Entrar");
    gtk_widget_set_margin_top(btn_entrar, 15);
    
    // Conexao do clique do botao com o callback de entrada
    g_signal_connect(btn_entrar, "clicked", G_CALLBACK(on_btn_entrar_clicked), campos);
    
    gtk_box_append(GTK_BOX(vbox), btn_entrar);

    // Retorna o container com a tela pronta
    return vbox; 
}
