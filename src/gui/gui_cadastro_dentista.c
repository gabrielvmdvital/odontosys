#include "gui.h"
#include "logs.h"
#include "dentist.h"
#include <string.h>
#include "database.h"

/**
 * @brief Limpa os campos de texto do formulario de cadastro de dentista.
 */
static void limpar_campos_cadastro_dentista(void) {
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cad_dentista.entry_nome), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cad_dentista.entry_username), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cad_dentista.entry_cpf), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cad_dentista.entry_senha), "");
}

/**
 * @brief Callback disparado ao alterar os campos de cadastro de dentista
 */
static void on_cadastro_dentista_entry_changed(GtkEditable *editable, gpointer user_data) {
    (void)editable;
    CadastroDentistaCampos *campos = (CadastroDentistaCampos *)user_data;
    const char *nome = gtk_editable_get_text(GTK_EDITABLE(campos->entry_nome));
    const char *username = gtk_editable_get_text(GTK_EDITABLE(campos->entry_username));
    const char *cpf = gtk_editable_get_text(GTK_EDITABLE(campos->entry_cpf));
    const char *senha = gtk_editable_get_text(GTK_EDITABLE(campos->entry_senha));

    gboolean pronto = (strlen(nome) > 0 && strlen(username) > 0 && strlen(cpf) > 0 && strlen(senha) > 0); // strlen calcula o tamanho da string
    gtk_widget_set_sensitive(campos->btn_salvar, pronto);
}

/**
 * @brief Callback ao clicar no botao de salvar dentista
 */
static void on_btn_salvar_dentista_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    CadastroDentistaCampos *campos = (CadastroDentistaCampos *)user_data;
    Dentist d;
    
    strncpy(d.name, gtk_editable_get_text(GTK_EDITABLE(campos->entry_nome)), sizeof(d.name)-1); // strncpy copia N caracteres de uma string, prevenindo overflow
    strncpy(d.username, gtk_editable_get_text(GTK_EDITABLE(campos->entry_username)), sizeof(d.username)-1);
    const char *raw_cpf = gtk_editable_get_text(GTK_EDITABLE(campos->entry_cpf));
    clean_cpf(raw_cpf, d.cpf);
    strncpy(d.password, gtk_editable_get_text(GTK_EDITABLE(campos->entry_senha)), sizeof(d.password)-1);
    
    if (strlen(d.cpf) != 11) {
        GtkAlertDialog *alert = gtk_alert_dialog_new("Dados Inválidos");
        gtk_alert_dialog_set_detail(alert, "O CPF deve conter exatamente 11 números.");
        gtk_alert_dialog_show(alert, NULL);
        g_object_unref(alert);
        return;
    }

    Dentist existing = find_dentist_by_cpf(d.cpf);
    if (existing.dentist_id != (uint64_t)-1) {
        GtkAlertDialog *alert = gtk_alert_dialog_new("Erro de Cadastro");
        gtk_alert_dialog_set_detail(alert, "Já existe um dentista cadastrado com este CPF.");
        gtk_alert_dialog_show(alert, NULL);
        g_object_unref(alert);
        return;
    }
    
    d.role = 2; 
    d.dentist_id = generate_unique_id();
    
    log_message(LOG_INFO, "[GUI] Processando novo cadastro de dentista...");
    log_message(LOG_INFO, " |- Nome: %s", d.name);
    log_message(LOG_INFO, " |- Username: %s", d.username);
    log_message(LOG_INFO, " |- CPF: %s", d.cpf);
    
    save_dentist(&d);
    log_message(LOG_INFO, "[GUI] Dentista salvo no BD com sucesso.");
    
    limpar_campos_cadastro_dentista();
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
}

/**
 * @brief Callback ao clicar no botao voltar do cadastro de dentista
 */
static void on_btn_voltar_cad_dentista_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    log_message(LOG_INFO, "[GUI] Retornando do Cadastro de Dentista para a Dashboard...");
    limpar_campos_cadastro_dentista();
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
}

/**
 * @brief Constroi a tela de cadastro de dentista
 */
GtkWidget* criar_tela_cadastro_dentista(CadastroDentistaCampos *campos) {
    // Container principal vertical com espacamento interno
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    // Container horizontal para o topo da tela
    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_cad_dentista_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);

    // Titulo da tela
    GtkWidget *lbl_titulo = gtk_label_new("Cadastro de Dentista");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);
    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // Campo de entrada de nome do dentista
    GtkWidget *lbl_nome = gtk_label_new("Nome:");
    gtk_widget_set_halign(lbl_nome, GTK_ALIGN_START);
    campos->entry_nome = gtk_entry_new();
    g_signal_connect(campos->entry_nome, "changed", G_CALLBACK(on_cadastro_dentista_entry_changed), campos);
    gtk_box_append(GTK_BOX(vbox), lbl_nome);
    gtk_box_append(GTK_BOX(vbox), campos->entry_nome);

    // Campo de entrada de nome de usuario (login)
    GtkWidget *lbl_username = gtk_label_new("Nome de Usuário (Login):");
    gtk_widget_set_halign(lbl_username, GTK_ALIGN_START);
    campos->entry_username = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_username), "Username");
    g_signal_connect(campos->entry_username, "changed", G_CALLBACK(on_cadastro_dentista_entry_changed), campos);
    gtk_box_append(GTK_BOX(vbox), lbl_username);
    gtk_box_append(GTK_BOX(vbox), campos->entry_username);

    // Campo de entrada de CPF
    GtkWidget *lbl_cpf = gtk_label_new("CPF:");
    gtk_widget_set_halign(lbl_cpf, GTK_ALIGN_START);
    campos->entry_cpf = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_cpf), "000.000.000-00");
    g_signal_connect(campos->entry_cpf, "changed", G_CALLBACK(on_cadastro_dentista_entry_changed), campos);
    gtk_box_append(GTK_BOX(vbox), lbl_cpf);
    gtk_box_append(GTK_BOX(vbox), campos->entry_cpf);

    // Campo de entrada de senha (com texto oculto)
    GtkWidget *lbl_senha = gtk_label_new("Senha:");
    gtk_widget_set_halign(lbl_senha, GTK_ALIGN_START);
    campos->entry_senha = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(campos->entry_senha), FALSE);
    g_signal_connect(campos->entry_senha, "changed", G_CALLBACK(on_cadastro_dentista_entry_changed), campos);
    gtk_box_append(GTK_BOX(vbox), lbl_senha);
    gtk_box_append(GTK_BOX(vbox), campos->entry_senha);

    // Botao de submissao do cadastro
    campos->btn_salvar = gtk_button_new_with_label("💾 Salvar");
    gtk_widget_set_margin_top(campos->btn_salvar, 15);
    gtk_widget_set_sensitive(campos->btn_salvar, FALSE);
    g_signal_connect(campos->btn_salvar, "clicked", G_CALLBACK(on_btn_salvar_dentista_clicked), campos);
    gtk_box_append(GTK_BOX(vbox), campos->btn_salvar);

    return vbox;
}
