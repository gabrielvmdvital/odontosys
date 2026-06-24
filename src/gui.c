#include "gui.h"
#include "logs.h"
#include "dentist.h"
#include "patient.h"
#include "clinical.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// ============================================================================
// VARIÁVEIS GLOBAIS DE ESTADO E REFERÊNCIAS VISUAIS
// ============================================================================

/** @brief Referencia global para a aplicacao GTK. */
GtkApplication *g_gtk_app = NULL;
/** @brief Estado global do aplicativo. */
AppState *g_app_state = NULL;
/** @brief Gerenciador de empilhamento de telas (Stack). */
GtkWidget *g_stack = NULL;

/** @brief ID do dentista logado no momento. */
uint64_t g_logged_dentist_id = 0;
/** @brief ID do paciente selecionado no momento. */
uint64_t g_selected_patient_id = 0;

/** @brief Memoria temporaria do paciente atual em processamento. */
Patient g_current_patient = {0};
/** @brief Memoria temporaria do prontuario atual em processamento. */
ClinicalRecord g_current_clinical_record = {0};

/** @brief Define se o fluxo de laudo esta sendo feito de forma autonoma ou amarrado a um cadastro. */
gboolean g_is_standalone_laudo = FALSE;

/** @brief Estrutura instanciada para reter campos do cadastro de paciente. */
CadastroCampos g_campos_cadastro = {0};
/** @brief Estrutura instanciada para reter campos da edicao de paciente. */
EdicaoPacienteCampos g_campos_edicao_pac = {0};
/** @brief Estrutura instanciada para reter campos do cadastro de dentista. */
CadastroDentistaCampos g_campos_cad_dentista = {0};
/** @brief Estrutura instanciada para reter o estado da tela de busca. */
BuscaCampos g_campos_busca = {0};
/** @brief Estrutura instanciada para visualizacao de prontuario. */
ProntuarioViewCampos g_pron_view = {0};
/** @brief Estrutura instanciada para visualizacao detalhada de laudo cefalometrico. */
LaudoViewCampos g_laudo_view = {0};

/** @brief Referencia para o botao de cadastro de dentista, visivel apenas para admin. */
GtkWidget *g_btn_cadastrar_dentista = NULL;

// ============================================================================
// INICIALIZAÇÃO E FLUXO PRINCIPAL DO GTK
// ============================================================================

/**
 * @brief Evento principal de ativação do GTK. Constrói a janela e a pilha de telas.
 */
static void on_app_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    // Instancia as estruturas dos campos de forma estática para persistir na memória do app
    static LoginCampos campos_login;
    static PreDiagCampos campos_pre_diag; 

    // 1. Criação da janela principal do Windows
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "ODONTOSYS");
    gtk_window_set_default_size(GTK_WINDOW(window), 550, 525); 

    // 2. Criação do componente GtkStack (Gerenciador de Empilhamento de Telas)
    g_stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(g_stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);

    // 3. Construção das telas independentes passando as referências de memória das structs de captura
    GtkWidget *tela_login = criar_tela_login(&campos_login);
    GtkWidget *tela_dashboard = criar_tela_dashboard();
    GtkWidget *tela_cadastro_dentista = criar_tela_cadastro_dentista(&g_campos_cad_dentista);
    GtkWidget *tela_cadastro_pac = criar_tela_cadastro_pacientes(&g_campos_cadastro);
    GtkWidget *tela_edicao_pac = criar_tela_edicao_paciente(&g_campos_edicao_pac);
    GtkWidget *tela_pre_diag = criar_tela_pre_diagnostico(&campos_pre_diag);
    GtkWidget *tela_prontuarios = criar_tela_prontuarios();
    GtkWidget *tela_prontuario_view = criar_tela_prontuario_view();
    GtkWidget *tela_laudo_view = criar_tela_laudo_view();

    // 4. Adicionando as telas dentro do Stack e dando um "nome" de identificação para cada uma
    gtk_stack_add_named(GTK_STACK(g_stack), tela_login, "login_page");
    gtk_stack_add_named(GTK_STACK(g_stack), tela_dashboard, "dashboard_page");
    gtk_stack_add_named(GTK_STACK(g_stack), tela_cadastro_dentista, "cadastro_dentista_page");
    gtk_stack_add_named(GTK_STACK(g_stack), tela_cadastro_pac, "cadastro_page");
    gtk_stack_add_named(GTK_STACK(g_stack), tela_edicao_pac, "edicao_paciente_page");
    gtk_stack_add_named(GTK_STACK(g_stack), tela_pre_diag, "pre_diagnostico_page");
    gtk_stack_add_named(GTK_STACK(g_stack), tela_prontuarios, "prontuarios_page");
    gtk_stack_add_named(GTK_STACK(g_stack), tela_prontuario_view, "prontuario_view_page");
    gtk_stack_add_named(GTK_STACK(g_stack), tela_laudo_view, "laudo_view_page");

    // 6. Define o GtkStack como o filho principal (conteúdo) da nossa janela
    gtk_window_set_child(GTK_WINDOW(window), g_stack);

    // 5. Define qual página o Stack deve exibir assim que o programa abrir
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "login_page");

    // 7. Renderiza a janela na tela do usuário
    gtk_window_present(GTK_WINDOW(window));
}

/*
 * Inicializa a interface grafica do GTK e os estados da aplicacao
 * Retorna true em caso de sucesso, false caso contrario
 */
bool gui_init(AppState *app_state) {
    if (app_state == NULL) return false;
    g_app_state = app_state;

    g_gtk_app = gtk_application_new("com.odontosys.app", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(g_gtk_app, "activate", G_CALLBACK(on_app_activate), NULL);

    return true;
}

/*
 * Executa o loop principal da aplicacao grafica GTK
 */
void gui_run(void) {
    if (g_gtk_app != NULL) {
        g_application_run(G_APPLICATION(g_gtk_app), 0, NULL);
        g_object_unref(g_gtk_app);
    }
}

/**
 * @brief Remove caracteres nao numericos de uma string de CPF.
 */
void clean_cpf(const char *input, char *output) {
    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] >= '0' && input[i] <= '9') {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}