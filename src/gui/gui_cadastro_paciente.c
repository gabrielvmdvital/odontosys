#include "gui.h"
#include "logs.h"
#include "patient.h"
#include <string.h>
#include "database.h"

/**
 * @brief Limpa os campos de texto do formulario de cadastro basico de paciente.
 */
void limpar_campos_cadastro_basico(void) {
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_nome), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_email), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_cpf), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_data_nasc), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_telefone), "");
    
    memset(&g_current_patient, 0, sizeof(Patient)); // memset preenche um bloco de memória com um valor

    if (g_campos_cadastro.btn_salvar) {
        gtk_widget_set_sensitive(g_campos_cadastro.btn_salvar, FALSE);
    }
}

/**
 * @brief Callback disparado sempre que um caractere é digitado nos campos de texto.
 * Valida se todos os campos estão preenchidos para habilitar o botão de Salvar.
 */
static void on_cadastro_paciente_entry_changed(GtkEditable *editable, gpointer user_data) {
    (void)editable;
    CadastroCampos *campos = (CadastroCampos *)user_data;
    
    if (!campos->btn_salvar) return;

    const char *nome = gtk_editable_get_text(GTK_EDITABLE(campos->entry_nome));
    const char *cpf = gtk_editable_get_text(GTK_EDITABLE(campos->entry_cpf));
    const char *data_nasc = gtk_editable_get_text(GTK_EDITABLE(campos->entry_data_nasc));
    const char *telefone = gtk_editable_get_text(GTK_EDITABLE(campos->entry_telefone));
    
    gboolean is_valid = (strlen(nome) > 0 && strlen(cpf) > 0 && strlen(data_nasc) > 0 && strlen(telefone) > 0); // strlen calcula o tamanho da string
    
    gtk_widget_set_sensitive(campos->btn_salvar, is_valid);
}

/**
 * @brief Callback disparado ao clicar no botão "Salvar Ficha" do cadastro.
 * Valida a formatação básica e verifica a existência do paciente.
 */
static void on_btn_salvar_cadastro_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    CadastroCampos *campos = (CadastroCampos *)user_data;

    const char *nome = gtk_editable_get_text(GTK_EDITABLE(campos->entry_nome));
    const char *email = gtk_editable_get_text(GTK_EDITABLE(campos->entry_email));
    const char *raw_cpf = gtk_editable_get_text(GTK_EDITABLE(campos->entry_cpf));
    
    char cpf[20];
    clean_cpf(raw_cpf, cpf);

    const char *data_nasc = gtk_editable_get_text(GTK_EDITABLE(campos->entry_data_nasc));
    const char *telefone = gtk_editable_get_text(GTK_EDITABLE(campos->entry_telefone));

    char erro_msg[512] = "";
    
    if (strlen(cpf) != 11) {
        strcat(erro_msg, "- CPF deve conter exatamente 11 números.\n"); // strcat concatena duas strings
    }
    
    if (strlen(data_nasc) != 8) {
        strcat(erro_msg, "- Data de Nascimento deve conter exatamente 8 números (DDMMAAAA).\n");
    } else {
        int dia = (data_nasc[0] - '0') * 10 + (data_nasc[1] - '0');
        int mes = (data_nasc[2] - '0') * 10 + (data_nasc[3] - '0');
        int ano = (data_nasc[4] - '0') * 1000 + (data_nasc[5] - '0') * 100 + (data_nasc[6] - '0') * 10 + (data_nasc[7] - '0');
        if (dia < 1 || dia > 31 || mes < 1 || mes > 12 || ano < 1900 || ano > 2030) {
            strcat(erro_msg, "- Data de Nascimento inválida.\n");
        }
    }
    
    if (strlen(telefone) < 10 || strlen(telefone) > 11) {
        strcat(erro_msg, "- Telefone deve conter 10 ou 11 números (com DDD).\n");
    }
    
    if (strlen(email) > 0 && (strchr(email, '@') == NULL || strchr(email, '.') == NULL)) { // strchr encontra a primeira ocorrência do caractere na string
        strcat(erro_msg, "- E-mail informado possui formato inválido.\n");
    }

    if (strlen(erro_msg) > 0) {
        char detail_msg[1024];
        snprintf(detail_msg, sizeof(detail_msg), "Por favor, corrija os seguintes erros:\n\n%s", erro_msg); // snprintf formata com limite seguro de tamanho
        GtkAlertDialog *alert = gtk_alert_dialog_new("Dados Inválidos");
        gtk_alert_dialog_set_detail(alert, detail_msg);
        gtk_alert_dialog_show(alert, NULL);
        g_object_unref(alert);
        return;
    }

    char data_formatada[11];
    snprintf(data_formatada, sizeof(data_formatada), "%c%c/%c%c/%c%c%c%c", 
        data_nasc[0], data_nasc[1], data_nasc[2], data_nasc[3],
        data_nasc[4], data_nasc[5], data_nasc[6], data_nasc[7]);

    log_message(LOG_INFO, "[GUI] Processando novo cadastro de paciente...");
    
    uint64_t existing_id = g_current_patient.patient_id;
    memset(&g_current_patient, 0, sizeof(Patient));
    g_current_patient.patient_id = existing_id;
    
    strncpy(g_current_patient.name, nome, sizeof(g_current_patient.name)-1); // strncpy copia N caracteres de uma string, prevenindo overflow
    strncpy(g_current_patient.email, email, sizeof(g_current_patient.email)-1);
    strncpy(g_current_patient.cpf, cpf, sizeof(g_current_patient.cpf)-1);
    strncpy(g_current_patient.birth_date, data_formatada, sizeof(g_current_patient.birth_date)-1);
    strncpy(g_current_patient.phone, telefone, sizeof(g_current_patient.phone)-1);
    g_current_patient.dentist_id = g_logged_dentist_id;

    Patient existing_cpf_patient = find_patient_by_cpf(cpf);
    if (existing_cpf_patient.patient_id != (uint64_t)-1) {
        if (g_current_patient.patient_id == existing_cpf_patient.patient_id) {
            update_patient(g_current_patient.patient_id, &g_current_patient);
            log_message(LOG_INFO, "[GUI] Paciente atualizado no BD.");
        } else {
            GtkAlertDialog *alert = gtk_alert_dialog_new("Erro de Cadastro");
            gtk_alert_dialog_set_detail(alert, "Já existe um paciente cadastrado com este CPF.");
            gtk_alert_dialog_show(alert, NULL);
            g_object_unref(alert);
            return;
        }
    } else {
        if (save_patient(&g_current_patient)) {
            log_message(LOG_INFO, "[GUI] Novo paciente salvo no BD.");
        } else {
            GtkAlertDialog *alert = gtk_alert_dialog_new("Erro Interno");
            gtk_alert_dialog_set_detail(alert, "Não foi possível salvar o paciente no banco de dados.");
            gtk_alert_dialog_show(alert, NULL);
            g_object_unref(alert);
            return;
        }
    }

    log_message(LOG_INFO, "[GUI] Dados cadastrais retidos. Avançando para a página de Pré-Diagnóstico...");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "pre_diagnostico_page");
}

/**
 * @brief Callback ao clicar no botao voltar da tela de cadastro de paciente.
 */
static void on_btn_voltar_cadastro_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    log_message(LOG_INFO, "[GUI] Retornando do Cadastro para a Dashboard...");
    limpar_campos_cadastro_basico();
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
}

/**
 * @brief Limpa os campos de texto do formulario de edicao de paciente.
 */
static void limpar_campos_edicao_paciente(void) {
    gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_nome), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_email), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_cpf), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_data_nasc), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_telefone), "");
}

/**
 * @brief Callback disparado ao clicar no botao de salvar edicao de paciente.
 */
static void on_btn_salvar_edicao_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    EdicaoPacienteCampos *campos = (EdicaoPacienteCampos *)user_data;
    Patient p = find_patient_by_id(g_selected_patient_id);
    
    if (p.patient_id == (uint64_t)-1) {
        log_message(LOG_ERROR, "[GUI] Erro ao editar paciente inexistente.");
        return;
    }

    strncpy(p.name, gtk_editable_get_text(GTK_EDITABLE(campos->entry_nome)), sizeof(p.name)-1);
    strncpy(p.email, gtk_editable_get_text(GTK_EDITABLE(campos->entry_email)), sizeof(p.email)-1);
    const char *raw_cpf = gtk_editable_get_text(GTK_EDITABLE(campos->entry_cpf));
    clean_cpf(raw_cpf, p.cpf);
    strncpy(p.birth_date, gtk_editable_get_text(GTK_EDITABLE(campos->entry_data_nasc)), sizeof(p.birth_date)-1);
    strncpy(p.phone, gtk_editable_get_text(GTK_EDITABLE(campos->entry_telefone)), sizeof(p.phone)-1);
    
    update_patient(g_selected_patient_id, &p);
    log_message(LOG_INFO, "[GUI] Paciente editado: %s", p.name);
    
    limpar_campos_edicao_paciente();
    atualizar_lista_prontuarios("");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuario_view_page");
}

/**
 * @brief Callback ao clicar no botao voltar da tela de edicao de paciente.
 */
static void on_btn_voltar_edicao_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    limpar_campos_edicao_paciente();
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuario_view_page");
}

/**
 * @brief Constrói o Passo 1 (Visual) da Tela 4 - Cadastrar Pacientes.
 * Recebe o ponteiro da struct de campos para mapear os widgets de texto de cadastro.
 */
GtkWidget* criar_tela_cadastro_pacientes(CadastroCampos *campos) {
    // Container principal vertical com espacamento interno
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    // Container horizontal para o topo da tela
    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_bottom(hbox_topo, 10);
    
    // Botao para voltar a tela anterior
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_cadastro_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);

    // Titulo da tela de cadastro
    GtkWidget *lbl_titulo = gtk_label_new("Novo Cadastro de Paciente");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);
    
    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // Campo de entrada para o nome completo
    GtkWidget *lbl_nome = gtk_label_new("Nome Completo:");
    gtk_widget_set_halign(lbl_nome, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_nome);
    campos->entry_nome = gtk_entry_new();
    gtk_box_append(GTK_BOX(vbox), campos->entry_nome);

    // Campo de entrada para o e-mail (opcional)
    GtkWidget *lbl_email = gtk_label_new("E-mail (Opcional):");
    gtk_widget_set_halign(lbl_email, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_email);
    campos->entry_email = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_email), "exemplo@email.com");
    gtk_box_append(GTK_BOX(vbox), campos->entry_email);

    // Campo de entrada para o CPF
    GtkWidget *lbl_cpf = gtk_label_new("CPF:");
    gtk_widget_set_halign(lbl_cpf, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_cpf);
    campos->entry_cpf = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_cpf), "000.000.000-00");
    gtk_box_append(GTK_BOX(vbox), campos->entry_cpf);

    // Campo de entrada para a data de nascimento
    GtkWidget *lbl_data_nasc = gtk_label_new("Data de Nascimento:");
    gtk_widget_set_halign(lbl_data_nasc, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_data_nasc);
    campos->entry_data_nasc = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_data_nasc), "DD/MM/AAAA");
    gtk_box_append(GTK_BOX(vbox), campos->entry_data_nasc);

    // Campo de entrada para o telefone de contato
    GtkWidget *lbl_telefone = gtk_label_new("Telefone / WhatsApp:");
    gtk_widget_set_halign(lbl_telefone, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_telefone);
    campos->entry_telefone = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_telefone), "(81) 99999-9999");
    gtk_box_append(GTK_BOX(vbox), campos->entry_telefone);

    // Botao para salvar o cadastro do paciente
    campos->btn_salvar = gtk_button_new_with_label("💾 Salvar Ficha");
    gtk_widget_set_margin_top(campos->btn_salvar, 15);
    gtk_widget_set_size_request(campos->btn_salvar, -1, 40);
    
    // Conectando o clique do botão salvar ao callback mapeado com a struct de campos
    g_signal_connect(campos->btn_salvar, "clicked", G_CALLBACK(on_btn_salvar_cadastro_clicked), campos);
    
    gtk_box_append(GTK_BOX(vbox), campos->btn_salvar);
    
    // Define como inativo inicialmente
    gtk_widget_set_sensitive(campos->btn_salvar, FALSE);

    // Conecta o evento 'changed' nos campos obrigatórios
    g_signal_connect(campos->entry_nome, "changed", G_CALLBACK(on_cadastro_paciente_entry_changed), campos);
    g_signal_connect(campos->entry_cpf, "changed", G_CALLBACK(on_cadastro_paciente_entry_changed), campos);
    g_signal_connect(campos->entry_data_nasc, "changed", G_CALLBACK(on_cadastro_paciente_entry_changed), campos);
    g_signal_connect(campos->entry_telefone, "changed", G_CALLBACK(on_cadastro_paciente_entry_changed), campos);

    return vbox;
}

/**
 * @brief Constrói a Tela de Edição de Pacientes.
 */
GtkWidget* criar_tela_edicao_paciente(EdicaoPacienteCampos *campos) {
    // Container principal vertical da tela de edicao
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    // Container horizontal para o cabecalho
    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_bottom(hbox_topo, 10);
    
    // Botao para voltar a visualizacao do prontuario
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_edicao_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);

    // Titulo da tela de edicao
    GtkWidget *lbl_titulo = gtk_label_new("Edição de Paciente");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);
    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // Campo de entrada para alterar o nome
    GtkWidget *lbl_nome = gtk_label_new("Nome Completo:");
    gtk_widget_set_halign(lbl_nome, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_nome);
    campos->entry_nome = gtk_entry_new();
    gtk_box_append(GTK_BOX(vbox), campos->entry_nome);

    // Campo de entrada para alterar o e-mail
    GtkWidget *lbl_email = gtk_label_new("E-mail:");
    gtk_widget_set_halign(lbl_email, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_email);
    campos->entry_email = gtk_entry_new();
    gtk_box_append(GTK_BOX(vbox), campos->entry_email);

    // Campo de entrada para alterar o CPF
    GtkWidget *lbl_cpf = gtk_label_new("CPF:");
    gtk_widget_set_halign(lbl_cpf, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_cpf);
    campos->entry_cpf = gtk_entry_new();
    gtk_box_append(GTK_BOX(vbox), campos->entry_cpf);

    // Campo de entrada para alterar a data de nascimento
    GtkWidget *lbl_data_nasc = gtk_label_new("Data de Nascimento:");
    gtk_widget_set_halign(lbl_data_nasc, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_data_nasc);
    campos->entry_data_nasc = gtk_entry_new();
    gtk_box_append(GTK_BOX(vbox), campos->entry_data_nasc);

    // Campo de entrada para alterar o telefone
    GtkWidget *lbl_telefone = gtk_label_new("Telefone:");
    gtk_widget_set_halign(lbl_telefone, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_telefone);
    campos->entry_telefone = gtk_entry_new();
    gtk_box_append(GTK_BOX(vbox), campos->entry_telefone);

    // Botao para submeter as alteracoes
    GtkWidget *btn_salvar = gtk_button_new_with_label("💾 Salvar Edição");
    gtk_widget_set_margin_top(btn_salvar, 15);
    gtk_widget_set_size_request(btn_salvar, -1, 40);
    g_signal_connect(btn_salvar, "clicked", G_CALLBACK(on_btn_salvar_edicao_clicked), campos);
    gtk_box_append(GTK_BOX(vbox), btn_salvar);

    return vbox;
}
