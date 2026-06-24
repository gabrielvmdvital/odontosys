#include "gui.h"
#include "logs.h"
#include "patient.h"
#include "clinical.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "database.h"

/**
 * @brief Limpa o campo de texto da barra de busca de pacientes.
 */
static void limpar_tela_busca(void) {
    if (g_campos_busca.entry_busca) {
        gtk_editable_set_text(GTK_EDITABLE(g_campos_busca.entry_busca), "");
    }
}

/**
 * @brief Atualiza a listagem de pacientes (prontuários).
 * Caso receba um 'filtro', faz a busca no nome convertendo para minúsculas.
 */
void atualizar_lista_prontuarios(const char *filtro) {
    if (!g_campos_busca.listbox) return;

    GtkWidget *child = gtk_widget_get_first_child(g_campos_busca.listbox);
    while (child != NULL) {
        GtkWidget *next = gtk_widget_get_next_sibling(child);
        gtk_list_box_remove(GTK_LIST_BOX(g_campos_busca.listbox), child);
        child = next;
    }

    int total_count = 0;
    Patient *pacientes = get_all_patients(&total_count);

    gboolean achou = FALSE;

    if (pacientes && total_count > 0) {
        for (int i = 0; i < total_count; i++) {
            gboolean matches = TRUE;
            if (filtro && strlen(filtro) > 0) { // strlen calcula o tamanho da string
                char nome_lower[256];
                strncpy(nome_lower, pacientes[i].name, 255); // strncpy copia N caracteres de uma string, prevenindo overflow
                nome_lower[255] = '\0';
                for (int j = 0; nome_lower[j]; j++) nome_lower[j] = tolower((unsigned char)nome_lower[j]); // tolower converte o caractere para minúsculo
                if (strstr(nome_lower, filtro) == NULL) {
                    matches = FALSE;
                }
            }

            if (matches) {
                char label_str[256];
                snprintf(label_str, sizeof(label_str), "📝 %s (CPF: %s)", pacientes[i].name, pacientes[i].cpf); // snprintf formata com limite seguro de tamanho
                GtkWidget *label = gtk_label_new(label_str);
                gtk_widget_set_halign(label, GTK_ALIGN_START);
                
                GtkWidget *row = gtk_list_box_row_new();
                gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
                
                uint64_t *id_ptr = g_malloc(sizeof(uint64_t));
                *id_ptr = pacientes[i].patient_id;
                g_object_set_data_full(G_OBJECT(row), "patient_id", id_ptr, g_free);
                
                gtk_list_box_append(GTK_LIST_BOX(g_campos_busca.listbox), row);
                achou = TRUE;
            }
        }
        free(pacientes); // free libera a memória dinâmica alocada
    }

    if (!achou) {
        GtkWidget *lbl_vazio = gtk_label_new("  ❌ Nenhum paciente encontrado.");
        gtk_list_box_append(GTK_LIST_BOX(g_campos_busca.listbox), lbl_vazio);
    }
}

/**
 * @brief Callback disparado quando o botão "Buscar" é clicado.
 */
static void on_btn_buscar_clicked(GtkButton *btn, gpointer user_data) { 
    (void)btn;
    BuscaCampos *busca = (BuscaCampos *)user_data;
    const char *texto = gtk_editable_get_text(GTK_EDITABLE(busca->entry_busca));

    log_message(LOG_INFO, "[GUI] Buscando por: %s", texto);

    gchar *busca_lower = g_utf8_strdown(texto, -1);
    atualizar_lista_prontuarios(busca_lower);

    g_free(busca_lower); 
}

/**
 * @brief Callback disparado quando o botão "Voltar" da listagem de prontuários é clicado.
 */
static void on_btn_voltar_prontuarios_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    log_message(LOG_INFO, "[GUI] Retornando dos Prontuários para a Dashboard...");
    limpar_tela_busca();
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
}

/**
 * @brief Callback disparado ao clicar em um paciente na lista.
 */
static void on_prontuario_row_activated(GtkListBox *listbox, GtkListBoxRow *row, gpointer user_data) {
    (void)user_data;
    (void)listbox;
    if (row == NULL) return;

    uint64_t *id_ptr = g_object_get_data(G_OBJECT(row), "patient_id");
    if (!id_ptr) {
        log_message(LOG_ERROR, "[GUI] Ponteiro de ID de paciente ausente na row.");
        return;
    }
    carregar_tela_prontuario_por_id(*id_ptr);
}

/**
 * @brief Callback disparado quando o botão "Voltar" da visão do paciente é clicado.
 */
static void on_btn_voltar_pron_view_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuarios_page");
}

/**
 * @brief Callback para editar dados cadastrais do paciente atual
 */
static void on_btn_editar_pron_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    log_message(LOG_INFO, "[GUI] Editar prontuário - carregando dados para edição.");
    Patient p = find_patient_by_id(g_selected_patient_id);
    if (p.patient_id != (uint64_t)-1) {
        gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_nome), p.name);
        gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_email), p.email);
        gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_cpf), p.cpf);
        gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_data_nasc), p.birth_date);
        gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_telefone), p.phone);
        gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "edicao_paciente_page");
    }
}

/**
 * @brief Callback que trata a resposta do dialogo de confirmacao de exclusao.
 */
static void on_confirmar_exclusao_response(GObject *source_object, GAsyncResult *res, gpointer user_data) {
    (void)user_data;
    GtkAlertDialog *alert = GTK_ALERT_DIALOG(source_object);
    GError *error = NULL;
    int response = gtk_alert_dialog_choose_finish(alert, res, &error);

    if (error != NULL) {
        log_message(LOG_ERROR, "[GUI] Erro no dialog de exclusão: %s", error->message);
        g_error_free(error);
        return;
    }

    if (g_selected_patient_id == 0) {
        log_message(LOG_ERROR, "[GUI] ID do paciente inválido ao confirmar exclusão.");
        return;
    }

    // response 1 = "Excluir"
    if (response == 1) {
        log_message(LOG_INFO, "[GUI] Excluindo prontuário e paciente.");
        delete_patient(g_selected_patient_id);
        atualizar_lista_prontuarios("");
        gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuarios_page");
    }
}

/**
 * @brief Callback do botão de excluir paciente
 */
static void on_btn_excluir_pron_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    GtkWidget *parent_window = GTK_WIDGET(gtk_widget_get_root(GTK_WIDGET(btn)));
    
    GtkAlertDialog *alert = gtk_alert_dialog_new("Tem certeza que deseja excluir o paciente e todo o seu histórico clínico?");
    const char *buttons[] = {"Cancelar", "Excluir", NULL};
    gtk_alert_dialog_set_buttons(alert, buttons);
    gtk_alert_dialog_set_cancel_button(alert, 0);
    gtk_alert_dialog_set_default_button(alert, 1);
    
    gtk_alert_dialog_choose(alert, GTK_WINDOW(parent_window), NULL, on_confirmar_exclusao_response, NULL);
    g_object_unref(alert);
}

/**
 * @brief Callback disparado ao clicar em "Novo pré-diagnóstico"
 */
static void on_btn_novo_laudo_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    g_is_standalone_laudo = TRUE;
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "pre_diagnostico_page");
}

void on_laudo_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data);

/**
 * @brief Constrói a Tela 3 - Listagem de Prontuários com barra de busca.
 */
GtkWidget* criar_tela_prontuarios(void) {
    // Container principal vertical da tela
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    // Container horizontal para o topo
    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    // Botao para voltar a tela anterior
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_prontuarios_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);

    // Titulo da tela
    GtkWidget *lbl_titulo = gtk_label_new("Gerenciador de Prontuários");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);
    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // Container horizontal para a barra de busca
    GtkWidget *hbox_busca = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_margin_bottom(hbox_busca, 15);

    // Campo de entrada de texto para a busca
    g_campos_busca.entry_busca = gtk_entry_new();
    gtk_widget_set_hexpand(g_campos_busca.entry_busca, TRUE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(g_campos_busca.entry_busca), "Buscar por nome...");
    gtk_box_append(GTK_BOX(hbox_busca), g_campos_busca.entry_busca);

    // Botao para acionar a busca
    GtkWidget *btn_buscar = gtk_button_new_with_label("🔍 Buscar");
    g_signal_connect(btn_buscar, "clicked", G_CALLBACK(on_btn_buscar_clicked), &g_campos_busca);
    gtk_box_append(GTK_BOX(hbox_busca), btn_buscar);
    gtk_box_append(GTK_BOX(vbox), hbox_busca);

    // Area de rolagem para a lista de prontuarios
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled_window), 200);

    // Lista interativa (ListBox) para organizar os resultados visualmente
    g_campos_busca.listbox = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(g_campos_busca.listbox), GTK_SELECTION_SINGLE);
    
    g_signal_connect(g_campos_busca.listbox, "row-activated", G_CALLBACK(on_prontuario_row_activated), NULL);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), g_campos_busca.listbox);
    gtk_box_append(GTK_BOX(vbox), scrolled_window);

    return vbox;
}

/**
 * @brief Constrói a Tela de Visualização do Prontuário de um paciente específico.
 */
GtkWidget* criar_tela_prontuario_view(void) {
    // Container principal vertical da tela
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    // Container horizontal para a barra de acoes e titulo
    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    // Botao de retorno
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_pron_view_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);
    
    // Titulo da tela de visualizacao
    GtkWidget *lbl_titulo = gtk_label_new("Prontuário do Paciente");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);

    // Botao para edicao dos dados do paciente
    GtkWidget *btn_editar = gtk_button_new_with_label("✏️ Editar Dados");
    g_signal_connect(btn_editar, "clicked", G_CALLBACK(on_btn_editar_pron_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_editar);

    // Botao para exclusao do paciente
    GtkWidget *btn_excluir = gtk_button_new_with_label("🗑️ Excluir Paciente");
    g_signal_connect(btn_excluir, "clicked", G_CALLBACK(on_btn_excluir_pron_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_excluir);

    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // Quadro (Frame) para agrupar as informacoes pessoais do paciente
    GtkWidget *frame_pessoal = gtk_frame_new(" Dados Pessoais ");
    GtkWidget *vbox_pessoal = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_top(vbox_pessoal, 10);
    gtk_widget_set_margin_bottom(vbox_pessoal, 10);
    gtk_widget_set_margin_start(vbox_pessoal, 10);
    
    // Rotulo para exibicao do nome
    g_pron_view.lbl_nome = gtk_label_new("Nome: -");
    gtk_widget_set_halign(g_pron_view.lbl_nome, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox_pessoal), g_pron_view.lbl_nome);

    // Rotulo para exibicao do e-mail
    g_pron_view.lbl_email = gtk_label_new("E-mail: -");
    gtk_widget_set_halign(g_pron_view.lbl_email, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox_pessoal), g_pron_view.lbl_email);

    // Rotulo para exibicao do CPF
    g_pron_view.lbl_cpf = gtk_label_new("CPF: -");
    gtk_widget_set_halign(g_pron_view.lbl_cpf, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox_pessoal), g_pron_view.lbl_cpf);

    // Rotulo para exibicao da data de nascimento
    g_pron_view.lbl_data_nasc = gtk_label_new("Data de Nasc.: -");
    gtk_widget_set_halign(g_pron_view.lbl_data_nasc, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox_pessoal), g_pron_view.lbl_data_nasc);

    // Rotulo para exibicao do telefone
    g_pron_view.lbl_telefone = gtk_label_new("Telefone: -");
    gtk_widget_set_halign(g_pron_view.lbl_telefone, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox_pessoal), g_pron_view.lbl_telefone);

    gtk_frame_set_child(GTK_FRAME(frame_pessoal), vbox_pessoal);
    gtk_box_append(GTK_BOX(vbox), frame_pessoal);

    // Container horizontal para a secao de historico de laudos
    GtkWidget *hbox_laudos_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_top(hbox_laudos_topo, 10);
    
    // Titulo da secao de historico
    GtkWidget *lbl_laudos_titulo = gtk_label_new("<b>Histórico de Laudos (Cefalometria)</b>");
    gtk_label_set_use_markup(GTK_LABEL(lbl_laudos_titulo), TRUE);
    gtk_widget_set_hexpand(lbl_laudos_titulo, TRUE);
    gtk_widget_set_halign(lbl_laudos_titulo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_laudos_topo), lbl_laudos_titulo);

    // Botao para iniciar um novo pre-diagnostico para este paciente
    GtkWidget *btn_novo_laudo = gtk_button_new_with_label("➕ Novo pré-diagnóstico");
    g_signal_connect(btn_novo_laudo, "clicked", G_CALLBACK(on_btn_novo_laudo_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_laudos_topo), btn_novo_laudo);

    gtk_box_append(GTK_BOX(vbox), hbox_laudos_topo);

    // Area de rolagem para a lista de laudos do paciente
    GtkWidget *scrolled_laudos = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled_laudos, TRUE);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled_laudos), 200);

    // Lista interativa para acomodar cada laudo salvo
    g_pron_view.listbox_laudos = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(g_pron_view.listbox_laudos), GTK_SELECTION_SINGLE);
    
    g_signal_connect(g_pron_view.listbox_laudos, "row-activated", G_CALLBACK(on_laudo_row_activated), NULL);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_laudos), g_pron_view.listbox_laudos);
    gtk_box_append(GTK_BOX(vbox), scrolled_laudos);

    return vbox;
}

/**
 * @brief Popula a interface do prontuário com base no patient_id.
 */
void carregar_tela_prontuario_por_id(uint64_t patient_id) {
    g_selected_patient_id = patient_id;
    Patient p = find_patient_by_id(patient_id);
    if (p.patient_id == (uint64_t)-1) {
        log_message(LOG_ERROR, "[GUI] Falha ao carregar paciente selecionado (ID: %llu).", (unsigned long long)patient_id);
        return;
    }

    char buf[1024];
    
    snprintf(buf, sizeof(buf), "Nome: %s", p.name);
    gtk_label_set_text(GTK_LABEL(g_pron_view.lbl_nome), buf);

    snprintf(buf, sizeof(buf), "E-mail: %s", p.email);
    gtk_label_set_text(GTK_LABEL(g_pron_view.lbl_email), buf);

    snprintf(buf, sizeof(buf), "CPF: %s", p.cpf);
    gtk_label_set_text(GTK_LABEL(g_pron_view.lbl_cpf), buf);

    snprintf(buf, sizeof(buf), "Data de Nasc.: %s", p.birth_date);
    gtk_label_set_text(GTK_LABEL(g_pron_view.lbl_data_nasc), buf);

    snprintf(buf, sizeof(buf), "Telefone: %s", p.phone);
    gtk_label_set_text(GTK_LABEL(g_pron_view.lbl_telefone), buf);

    if (g_pron_view.listbox_laudos) {
        GtkWidget *child = gtk_widget_get_first_child(g_pron_view.listbox_laudos);
        while (child != NULL) {
            GtkWidget *next = gtk_widget_get_next_sibling(child);
            gtk_list_box_remove(GTK_LIST_BOX(g_pron_view.listbox_laudos), child);
            child = next;
        }
    }

    int total_records = 0;
    ClinicalRecord *records = load_clinical_records(patient_id, &total_records);
    if (records != NULL && total_records > 0) {
        for (int i = 0; i < total_records; i++) {
            ClinicalRecord *cr = g_malloc(sizeof(ClinicalRecord));
            memcpy(cr, &records[i], sizeof(ClinicalRecord));

            char row_label_str[2048];
            snprintf(row_label_str, sizeof(row_label_str), "📄 Pré-Diagnóstico (%s) - %s", cr->diag_date, cr->pre_diagnosis);
            
            GtkWidget *row_label = gtk_label_new(row_label_str);
            gtk_widget_set_halign(row_label, GTK_ALIGN_START);
            
            GtkWidget *row_list = gtk_list_box_row_new();
            gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row_list), row_label);
            
            g_object_set_data_full(G_OBJECT(row_list), "clinical_record", cr, g_free);

            gtk_list_box_append(GTK_LIST_BOX(g_pron_view.listbox_laudos), row_list);
        }
        free(records);
    } else {
        GtkWidget *lbl_vazio = gtk_label_new("  ❌ Nenhum laudo cefalométrico cadastrado.");
        gtk_widget_set_halign(lbl_vazio, GTK_ALIGN_START);
        gtk_list_box_append(GTK_LIST_BOX(g_pron_view.listbox_laudos), lbl_vazio);
    }

    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuario_view_page");
}
