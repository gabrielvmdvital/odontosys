#include "gui.h"
#include "logs.h"
#include <string.h>

/**
 * @brief Callback do botão Voltar da tela de laudo dedicado
 */
static void on_btn_voltar_laudo_view_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuario_view_page");
}

/**
 * @brief constroi a tela de visualização de um laudo dedicado
 */
GtkWidget* criar_tela_laudo_view(void) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    // Container horizontal para a barra superior da tela
    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    // Botao para voltar a visualizacao do paciente
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar ao Paciente");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_laudo_view_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);
    
    // Titulo da tela formatado com markup
    GtkWidget *lbl_titulo = gtk_label_new("Detalhes do Pré-Diagnóstico Cefalométrico");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_label_set_markup(GTK_LABEL(lbl_titulo), "<span size='large' weight='bold'>Detalhes do Pré-Diagnóstico Cefalométrico</span>");
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);

    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // Rotulo para exibir a data do laudo
    g_laudo_view.lbl_data = gtk_label_new("Data do Laudo: —");
    gtk_widget_set_halign(g_laudo_view.lbl_data, GTK_ALIGN_START);
    gtk_widget_set_margin_start(g_laudo_view.lbl_data, 5);
    gtk_box_append(GTK_BOX(vbox), g_laudo_view.lbl_data);

    // Quadro para agrupar visualmente as medidas clinicas
    GtkWidget *frame_medidas = gtk_frame_new(NULL);
    GtkWidget *lbl_frame_medidas = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_frame_medidas), "<b> Medidas Clínicas </b>");
    gtk_frame_set_label_widget(GTK_FRAME(frame_medidas), lbl_frame_medidas);
    gtk_widget_set_margin_top(frame_medidas, 10);

    GtkWidget *vbox_medidas = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_top(vbox_medidas, 15);
    gtk_widget_set_margin_bottom(vbox_medidas, 15);
    gtk_widget_set_margin_start(vbox_medidas, 15);
    gtk_widget_set_margin_end(vbox_medidas, 15);

    // Layout em grade (grid) para dispor as medidas em colunas
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);

    g_laudo_view.lbl_anb = gtk_label_new("Ângulo ANB: —");
    gtk_widget_set_halign(g_laudo_view.lbl_anb, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_anb, 0, 0, 1, 1);

    g_laudo_view.lbl_coa = gtk_label_new("COA: —");
    gtk_widget_set_halign(g_laudo_view.lbl_coa, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_coa, 1, 0, 1, 1);

    g_laudo_view.lbl_cogn = gtk_label_new("CO-GN: —");
    gtk_widget_set_halign(g_laudo_view.lbl_cogn, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_cogn, 0, 1, 1, 1);

    g_laudo_view.lbl_afai = gtk_label_new("AFAI: —");
    gtk_widget_set_halign(g_laudo_view.lbl_afai, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_afai, 1, 1, 1, 1);

    g_laudo_view.lbl_sngogn = gtk_label_new("SN.GO.GN: —");
    gtk_widget_set_halign(g_laudo_view.lbl_sngogn, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_sngogn, 0, 2, 1, 1);

    g_laudo_view.lbl_na1_dist = gtk_label_new("NA Sup. Dist: —");
    gtk_widget_set_halign(g_laudo_view.lbl_na1_dist, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_na1_dist, 1, 2, 1, 1);

    g_laudo_view.lbl_na1_ang = gtk_label_new("NA Sup. Âng: —");
    gtk_widget_set_halign(g_laudo_view.lbl_na1_ang, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_na1_ang, 0, 3, 1, 1);

    g_laudo_view.lbl_na2_dist = gtk_label_new("NB Inf. Dist: —");
    gtk_widget_set_halign(g_laudo_view.lbl_na2_dist, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_na2_dist, 1, 3, 1, 1);

    g_laudo_view.lbl_na2_ang = gtk_label_new("NB Inf. Âng: —");
    gtk_widget_set_halign(g_laudo_view.lbl_na2_ang, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_na2_ang, 0, 4, 1, 1);

    g_laudo_view.lbl_max_tipo = gtk_label_new("Tipo Maxila: —");
    gtk_widget_set_halign(g_laudo_view.lbl_max_tipo, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_max_tipo, 1, 4, 1, 1);

    g_laudo_view.lbl_max_desvio = gtk_label_new("Desvio Maxila: —");
    gtk_widget_set_halign(g_laudo_view.lbl_max_desvio, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_max_desvio, 0, 5, 1, 1);

    g_laudo_view.lbl_perfil = gtk_label_new("Perfil Tegumentar: —");
    gtk_widget_set_halign(g_laudo_view.lbl_perfil, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_perfil, 0, 6, 2, 1);

    gtk_box_append(GTK_BOX(vbox_medidas), grid);
    gtk_frame_set_child(GTK_FRAME(frame_medidas), vbox_medidas);
    gtk_box_append(GTK_BOX(vbox), frame_medidas);

    // Quadro para agrupar visualmente o texto de pre-diagnostico clinico
    GtkWidget *frame_destaque = gtk_frame_new(NULL);
    GtkWidget *lbl_frame_destaque = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_frame_destaque), "<b> Pré-Diagnóstico Clínico </b>");
    gtk_frame_set_label_widget(GTK_FRAME(frame_destaque), lbl_frame_destaque);
    gtk_widget_set_margin_top(frame_destaque, 15);
    
    GtkWidget *vbox_destaque = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_top(vbox_destaque, 15);
    gtk_widget_set_margin_bottom(vbox_destaque, 15);
    gtk_widget_set_margin_start(vbox_destaque, 15);
    gtk_widget_set_margin_end(vbox_destaque, 15);

    // Rotulo para exibir o texto completo do diagnostico, com quebra de linha habilitada
    g_laudo_view.lbl_diag = gtk_label_new("—");
    gtk_widget_set_halign(g_laudo_view.lbl_diag, GTK_ALIGN_START);
    gtk_label_set_wrap(GTK_LABEL(g_laudo_view.lbl_diag), TRUE);
    gtk_box_append(GTK_BOX(vbox_destaque), g_laudo_view.lbl_diag);

    gtk_frame_set_child(GTK_FRAME(frame_destaque), vbox_destaque);
    gtk_box_append(GTK_BOX(vbox), frame_destaque);

    return vbox;
}

/**
 * @brief Callback disparado ao clicar em um laudo na lista de prontuarios
 */
void on_laudo_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    (void)user_data;
    (void)box;
    if (row == NULL) return;

    ClinicalRecord *cr = g_object_get_data(G_OBJECT(row), "clinical_record");
    if (!cr) return;

    // Popula a struct g_laudo_view
    char buf[512];
    snprintf(buf, sizeof(buf), "Data do Laudo: %s", cr->diag_date); // snprintf formata com limite seguro de tamanho
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_data), buf);

    snprintf(buf, sizeof(buf), "%s", cr->pre_diagnosis);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_diag), buf);


    snprintf(buf, sizeof(buf), "Ângulo ANB: %.2f", cr->anb);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_anb), buf);

    snprintf(buf, sizeof(buf), "COA: %.2f", cr->coa);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_coa), buf);

    snprintf(buf, sizeof(buf), "CO-GN: %.2f", cr->cogn);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_cogn), buf);

    snprintf(buf, sizeof(buf), "AFAI: %.2f", cr->afai);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_afai), buf);

    snprintf(buf, sizeof(buf), "SN.GO.GN: %.2f", cr->sngogn);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_sngogn), buf);

    snprintf(buf, sizeof(buf), "NA Sup. Dist: %.2f", cr->na1_dist);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_na1_dist), buf);

    snprintf(buf, sizeof(buf), "NA Sup. Âng: %.2f", cr->na1_ang);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_na1_ang), buf);

    snprintf(buf, sizeof(buf), "NB Inf. Dist: %.2f", cr->nb1_dist);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_na2_dist), buf);

    snprintf(buf, sizeof(buf), "NB Inf. Âng: %.2f", cr->nb1_ang);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_na2_ang), buf);

    snprintf(buf, sizeof(buf), "Tipo Maxila: %d", cr->maxila_tipo);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_max_tipo), buf);

    snprintf(buf, sizeof(buf), "Desvio Maxila: %d", cr->maxila_desvio);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_max_desvio), buf);

    snprintf(buf, sizeof(buf), "Perfil: %s", cr->perf_tegument);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_perfil), buf);

    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "laudo_view_page");
}
