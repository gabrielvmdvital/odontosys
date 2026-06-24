#include "gui.h"
#include "logs.h"
#include "clinical.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Limpa os campos de entrada do formulario de pre-diagnostico cefalometrico.
 */
static void limpar_campos_pre_diagnostico(PreDiagCampos *campos) {
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_anb), "");
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_coa), "");
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_cogn), "");
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_max_desvio), "");
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_afai), "");
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_sngogn), "");
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_na1_dist), "");
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_na1_ang), "");
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_na2_dist), "");
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_na2_ang), "");
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_perf_tegu), "");
    gtk_drop_down_set_selected(GTK_DROP_DOWN(campos->dropdown_max_tipo), 0);
    
    gtk_widget_set_visible(campos->frame_resultado, FALSE);
    if (campos->btn_concluir != NULL) {
        gtk_widget_set_visible(campos->btn_concluir, FALSE);
    }
}

/**
 * @brief Callback disparado ao clicar em "Concluir e Salvar Prontuário".
 * Limpa completamente ambas as telas e retorna o fluxo à tela inicial de cadastro de pacientes.
 */
static void on_btn_concluir_fluxo_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    PreDiagCampos *campos = (PreDiagCampos *)user_data;
    
    if (g_is_standalone_laudo) {
        g_current_clinical_record.patient_id = g_selected_patient_id;
        g_current_clinical_record.dentist_id = g_logged_dentist_id;
        
        time_t t = time(NULL); // time obtém o timestamp atual do sistema
        struct tm tm = *localtime(&t); // localtime converte timestamp para estrutura de tempo local
        char temp_date[32];
        snprintf(temp_date, sizeof(temp_date), "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900); // snprintf formata com limite seguro de tamanho
        strncpy(g_current_clinical_record.diag_date, temp_date, sizeof(g_current_clinical_record.diag_date) - 1); // strncpy copia N caracteres de uma string, prevenindo overflow
        g_current_clinical_record.diag_date[sizeof(g_current_clinical_record.diag_date) - 1] = '\0';
        
        save_clinical_record(&g_current_clinical_record);
        log_message(LOG_INFO, "[GUI] Pré-diagnóstico salvo no BD.");
        
        limpar_campos_pre_diagnostico(campos);
        
        g_is_standalone_laudo = FALSE;
        
        carregar_tela_prontuario_por_id(g_selected_patient_id);
    } else {
        g_current_clinical_record.patient_id = g_current_patient.patient_id;
        g_current_clinical_record.dentist_id = g_logged_dentist_id;
        
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        char temp_date[32];
        snprintf(temp_date, sizeof(temp_date), "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
        strncpy(g_current_clinical_record.diag_date, temp_date, sizeof(g_current_clinical_record.diag_date) - 1);
        g_current_clinical_record.diag_date[sizeof(g_current_clinical_record.diag_date) - 1] = '\0';
        
        save_clinical_record(&g_current_clinical_record);
        log_message(LOG_INFO, "[GUI] Fluxo encerrado. Prontuário salvo no BD.");
        
        limpar_campos_pre_diagnostico(campos);
        limpar_campos_cadastro_basico();
        
        gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "cadastro_page");
    }
}

/**
 * @brief Callback para o botão "Voltar" da Ficha de Pré-Diagnóstico.
 * Retorna o usuário para a tela de cadastro básico do paciente.
 */
static void on_btn_voltar_pre_diag_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    log_message(LOG_INFO, "[GUI] Retornando do Pré-Diagnóstico para a tela de Cadastro...");
    limpar_campos_pre_diagnostico((PreDiagCampos *)user_data);
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "cadastro_page");
}

/**
 * @brief Callback disparado quando os campos do pre-diagnostico mudam
 */
static void on_pre_diag_fields_changed(GtkEditable *editable, gpointer user_data) {
    (void)editable;
    PreDiagCampos *c = (PreDiagCampos *)user_data;
    gboolean is_filled = TRUE;
    
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_anb))) == 0) is_filled = FALSE; // strlen calcula o tamanho da string
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_coa))) == 0) is_filled = FALSE;
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_cogn))) == 0) is_filled = FALSE;
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_max_desvio))) == 0) is_filled = FALSE;
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_afai))) == 0) is_filled = FALSE;
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_sngogn))) == 0) is_filled = FALSE;
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_na1_dist))) == 0) is_filled = FALSE;
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_na1_ang))) == 0) is_filled = FALSE;
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_na2_dist))) == 0) is_filled = FALSE;
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_na2_ang))) == 0) is_filled = FALSE;
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_perf_tegu))) == 0) is_filled = FALSE;

    gtk_widget_set_sensitive(c->btn_diagnostico, is_filled);
}

/**
 * @brief Callback disparado quando o botão "Gerar Diagnóstico" é clicado.
 * Captura todos os inputs técnicos de ClinicalRecord para disponibilizar para a árvore de decisão futuramente.
 */
static void on_btn_gerar_diagnostico_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    PreDiagCampos *campos = (PreDiagCampos *)user_data;

    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(campos->dropdown_max_tipo));
    int valor_maxila_tipo = 0; 
    if (idx == 1) valor_maxila_tipo = 1;   
    if (idx == 2) valor_maxila_tipo = -1;  

    memset(&g_current_clinical_record, 0, sizeof(ClinicalRecord)); // memset preenche um bloco de memória com um valor
    g_current_clinical_record.anb = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_anb))); // atof converte de string para double/float
    g_current_clinical_record.coa = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_coa)));
    g_current_clinical_record.maxila_tipo = valor_maxila_tipo;
    g_current_clinical_record.maxila_desvio = atoi(gtk_editable_get_text(GTK_EDITABLE(campos->entry_max_desvio))); // atoi converte string para inteiro numérico padrão
    g_current_clinical_record.cogn = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_cogn)));
    g_current_clinical_record.afai = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_afai)));
    g_current_clinical_record.sngogn = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_sngogn)));
    g_current_clinical_record.na1_dist = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_na1_dist)));
    g_current_clinical_record.na1_ang = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_na1_ang)));
    g_current_clinical_record.nb1_dist = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_na2_dist)));
    g_current_clinical_record.nb1_ang = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_na2_ang)));
    strncpy(g_current_clinical_record.perf_tegument, gtk_editable_get_text(GTK_EDITABLE(campos->entry_perf_tegu)), sizeof(g_current_clinical_record.perf_tegument)-1);

    clinical_formular_diag(&g_current_clinical_record);

    log_message(LOG_INFO, "[GUI] Coletando métricas estruturadas de ClinicalRecord para processamento:");
    log_message(LOG_INFO, "[INFERENCIA] Pré-diagnóstico: %s", g_current_clinical_record.pre_diagnosis);
    log_message(LOG_INFO, "[GUI] Registro clínico consolidado com sucesso!");

    char markup[2048];
    snprintf(markup, sizeof(markup),
        "<span size='x-large' weight='bold' foreground='#1b5e20'>Pré-Diagnóstico</span>\n\n"
        "<span size='large' foreground='#424242'>%s</span>\n\n"
        "<i>Ficha consolidada com sucesso e pronta para persistência.</i>",
        g_current_clinical_record.pre_diagnosis
    );

    gtk_label_set_markup(GTK_LABEL(campos->lbl_resultado), markup);

    gtk_widget_set_visible(campos->frame_resultado, TRUE);
    gtk_widget_set_visible(campos->btn_concluir, TRUE);
}

/**
 * @brief Constroi a tela de pre-diagnostico.
 */
GtkWidget* criar_tela_pre_diagnostico(PreDiagCampos *campos) {
    // Container principal vertical da tela
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(vbox, 15);
    gtk_widget_set_margin_bottom(vbox, 15);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    // Container horizontal para a barra de navegacao superior
    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    // Botao para voltar a tela anterior
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_pre_diag_clicked), campos);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);

    // Titulo da tela de pre-diagnostico
    GtkWidget *lbl_titulo = gtk_label_new("Ficha de Pré-Diagnóstico Cefalométrico");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);
    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // Layout em grade (grid) para organizar os campos do formulario
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_widget_set_margin_top(grid, 10);

    // Metricas Gerais Basicas (Primeira Coluna)
    GtkWidget *lbl_anb = gtk_label_new("Ângulo ANB (°):");
    gtk_widget_set_halign(lbl_anb, GTK_ALIGN_START);
    campos->entry_anb = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl_anb, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_anb, 0, 1, 1, 1);

    GtkWidget *lbl_coa = gtk_label_new("Comprimento COA (mm):");
    gtk_widget_set_halign(lbl_coa, GTK_ALIGN_START);
    campos->entry_coa = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl_coa, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_coa, 0, 3, 1, 1);

    GtkWidget *lbl_cogn = gtk_label_new("Comprimento CO-GN (mm):");
    gtk_widget_set_halign(lbl_cogn, GTK_ALIGN_START);
    campos->entry_cogn = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl_cogn, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_cogn, 0, 5, 1, 1);

    // Metricas de Cefalometria Avancadas (Segunda Coluna)
    GtkWidget *lbl_tipo = gtk_label_new("Tipo da Maxila:");
    gtk_widget_set_halign(lbl_tipo, GTK_ALIGN_START);
    campos->dropdown_max_tipo = gtk_drop_down_new_from_strings((const char *[]) {"Normal", "Protruída", "Retruída", NULL});
    gtk_grid_attach(GTK_GRID(grid), lbl_tipo, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->dropdown_max_tipo, 1, 1, 1, 1);

    GtkWidget *lbl_desvio = gtk_label_new("Desvio Maxila (mm):");
    gtk_widget_set_halign(lbl_desvio, GTK_ALIGN_START);
    campos->entry_max_desvio = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl_desvio, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_max_desvio, 1, 3, 1, 1);

    GtkWidget *lbl_afai = gtk_label_new("AFAI (mm):");
    gtk_widget_set_halign(lbl_afai, GTK_ALIGN_START);
    campos->entry_afai = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl_afai, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_afai, 1, 5, 1, 1);

    GtkWidget *lbl_sngogn = gtk_label_new("SN.GO.GN (°):");
    gtk_widget_set_halign(lbl_sngogn, GTK_ALIGN_START);
    campos->entry_sngogn = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl_sngogn, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_sngogn, 1, 7, 1, 1);

    GtkWidget *lbl_na1 = gtk_label_new("Incisivo Sup. NA (dist/ang):");
    gtk_widget_set_halign(lbl_na1, GTK_ALIGN_START);
    GtkWidget *hbox_na1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    campos->entry_na1_dist = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_na1_dist), "Dist (mm)");
    campos->entry_na1_ang = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_na1_ang), "Âng (°)");
    gtk_box_append(GTK_BOX(hbox_na1), campos->entry_na1_dist);
    gtk_box_append(GTK_BOX(hbox_na1), campos->entry_na1_ang);
    gtk_grid_attach(GTK_GRID(grid), lbl_na1, 1, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), hbox_na1, 1, 9, 1, 1);

    GtkWidget *lbl_na2 = gtk_label_new("Incisivo Inf. NB (dist/ang):");
    gtk_widget_set_halign(lbl_na2, GTK_ALIGN_START);
    GtkWidget *hbox_na2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    campos->entry_na2_dist = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_na2_dist), "Dist (mm)");
    campos->entry_na2_ang = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_na2_ang), "Âng (°)");
    gtk_box_append(GTK_BOX(hbox_na2), campos->entry_na2_dist);
    gtk_box_append(GTK_BOX(hbox_na2), campos->entry_na2_ang);
    gtk_grid_attach(GTK_GRID(grid), lbl_na2, 1, 10, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), hbox_na2, 1, 11, 1, 1);

    // Campo de largura total para o perfil tegumentar
    GtkWidget *lbl_perf = gtk_label_new("Perfil Tegumentar:");
    gtk_widget_set_halign(lbl_perf, GTK_ALIGN_START);
    campos->entry_perf_tegu = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_perf_tegu), "Ex: Perfil Convexo / Reto / Côncavo");
    gtk_grid_attach(GTK_GRID(grid), lbl_perf, 0, 12, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_perf_tegu, 0, 13, 2, 1);

    gtk_box_append(GTK_BOX(vbox), grid);

    // Quadro (Frame) para exibir o resultado do diagnostico processado
    campos->frame_resultado = gtk_frame_new(" 📊 LAUDO CEFALOMÉTRICO PROCESSADO ");
    gtk_widget_set_margin_top(campos->frame_resultado, 20);
    gtk_widget_set_margin_bottom(campos->frame_resultado, 10);
    gtk_widget_set_visible(campos->frame_resultado, FALSE);

    GtkWidget *box_card_interno = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_top(box_card_interno, 12);
    gtk_widget_set_margin_bottom(box_card_interno, 12);
    gtk_widget_set_margin_start(box_card_interno, 15);
    gtk_widget_set_margin_end(box_card_interno, 15);

    campos->lbl_resultado = gtk_label_new(NULL);
    gtk_label_set_justify(GTK_LABEL(campos->lbl_resultado), GTK_JUSTIFY_CENTER);
    
    gtk_box_append(GTK_BOX(box_card_interno), campos->lbl_resultado);
    gtk_frame_set_child(GTK_FRAME(campos->frame_resultado), box_card_interno);
    gtk_box_append(GTK_BOX(vbox), campos->frame_resultado);

    // Botao para processar os dados e gerar o pre-diagnostico
    campos->btn_diagnostico = gtk_button_new_with_label("🧠 Gerar Pré-Diagnóstico");
    gtk_widget_set_sensitive(campos->btn_diagnostico, FALSE);
    gtk_widget_set_margin_top(campos->btn_diagnostico, 15);
    gtk_widget_set_size_request(campos->btn_diagnostico, -1, 42);
    g_signal_connect(campos->btn_diagnostico, "clicked", G_CALLBACK(on_btn_gerar_diagnostico_clicked), campos);
    gtk_box_append(GTK_BOX(vbox), campos->btn_diagnostico);

    // Botao para concluir o fluxo e salvar o prontuario no banco de dados
    campos->btn_concluir = gtk_button_new_with_label("✨ Concluir e Salvar Prontuário");
    gtk_widget_set_visible(campos->btn_concluir, FALSE);
    gtk_widget_set_margin_top(campos->btn_concluir, 10);
    gtk_widget_set_size_request(campos->btn_concluir, -1, 42);
    g_signal_connect(campos->btn_concluir, "clicked", G_CALLBACK(on_btn_concluir_fluxo_clicked), campos);
    gtk_box_append(GTK_BOX(vbox), campos->btn_concluir);

    // Conexao de sinais de mudanca de texto para validacao dinamica do formulario
    g_signal_connect(campos->entry_anb, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);
    g_signal_connect(campos->entry_coa, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);
    g_signal_connect(campos->entry_cogn, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);
    g_signal_connect(campos->entry_max_desvio, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);
    g_signal_connect(campos->entry_afai, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);
    g_signal_connect(campos->entry_sngogn, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);
    g_signal_connect(campos->entry_na1_dist, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);
    g_signal_connect(campos->entry_na1_ang, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);
    g_signal_connect(campos->entry_na2_dist, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);
    g_signal_connect(campos->entry_na2_ang, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);
    g_signal_connect(campos->entry_perf_tegu, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);

    // Container de rolagem (ScrolledWindow) para adaptar a tela a diferentes resolucoes
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), vbox);
    gtk_widget_set_vexpand(scrolled_window, TRUE);

    return scrolled_window;
}
