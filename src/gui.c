#include "gui.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "logs.h"
#include "dentist.h"
#include "patient.h"
#include "clinical.h"
#include "database.h"


// ============================================================================
// VARIÁVEIS GLOBAIS ESTÁTICAS (Escopo do Arquivo)
// ============================================================================
static AppState *g_app_state = NULL;       // Guarda o estado do core do programa
static GtkApplication *g_gtk_app = NULL;   // Controle do ciclo de vida do GTK
static GtkWidget *g_stack = NULL;          // O GtkStack global que controlará a troca de telas
static GtkWidget *g_btn_cadastrar_dentista = NULL; // O Botão global de cadastrar dentista

static uint64_t g_logged_dentist_id = 0;
static uint64_t g_selected_patient_id = 0;
static Patient g_current_patient;
static ClinicalRecord g_current_clinical_record;
static gboolean g_is_standalone_laudo = FALSE; // Indica se estamos cadastrando um exame avulso

void carregar_tela_prontuario_por_id(uint64_t patient_id);

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

/**
 * @brief Estrutura que contém os componentes de busca na tela de prontuários.
 * Usada para passar vários dados para o botão de busca 
 */
typedef struct {
    GtkWidget *entry_busca; // Ponteiro para o campo de texto
    GtkWidget *listbox; // Ponteiro para a lista
} BuscaCampos;

static BuscaCampos g_campos_busca;

/**
 * @brief Estrutura que agrupa os componentes de texto da tela de cadastro de pacientes.
 * Usada para capturar os dados informados e enviá-los ao callback de salvamento.
 */
struct CadastroCampos {
    GtkWidget *entry_nome;       // Ponteiro para o campo de texto do nome
    GtkWidget *entry_email;      // Ponteiro para o campo de texto do email (opcional)
    GtkWidget *entry_cpf;        // Ponteiro para o campo de texto do CPF
    GtkWidget *entry_data_nasc;  // Ponteiro para o campo de texto da data de nascimento
    GtkWidget *entry_telefone;   // Ponteiro para o campo de texto do telefone
    GtkWidget *btn_salvar;       // Ponteiro para o botão Salvar
};

// Adicionando o typedef da struct para o compilador reconhecer o tipo sem a palavra 'struct':
typedef struct CadastroCampos CadastroCampos;

// Variável global estática:
static CadastroCampos g_campos_cadastro;   // Instanciação global/estática para permitir limpeza entre telas

/**
 * @brief Estrutura que agrupa os componentes de entrada para a ficha de Pré-Diagnóstico.
 * Representa os campos que mapeiam a struct ClinicalRecord do arquivo clinical.h.
 */
typedef struct {
    GtkWidget *entry_height;        // Altura do paciente (m)
    GtkWidget *entry_weight;        // Peso do paciente (kg)
    GtkWidget *entry_age;           // Idade do paciente
    GtkWidget *entry_anb;           // Relação maxila/mandíbula (ângulo)
    GtkWidget *entry_coa;           // Comprimento da maxila (distância)
    GtkWidget *dropdown_max_tipo;   // Tipo de Maxila (0=normal, 1=protruida, -1=retruida)
    GtkWidget *entry_max_desvio;    // Desvio da maxila (em mm)
    GtkWidget *entry_cogn;          // Comprimento mandibular (distância)
    GtkWidget *entry_afai;          // Altura facial inferior (distância)
    GtkWidget *entry_sngogn;        // Padrão vertical facial (ângulo)
    GtkWidget *entry_na1_dist;      // Posição do incisivo superior (distância)
    GtkWidget *entry_na1_ang;       // Inclinação do incisivo superior (ângulo)
    GtkWidget *entry_na2_dist;      // Posição do incisivo inferior (distância)
    GtkWidget *entry_na2_ang;       // Inclinação do incisivo inferior (ângulo)
    GtkWidget *entry_perf_tegu;     // Formato do perfil (texto)
    GtkWidget *lbl_resultado;       // Guarda o rótulo do resultado na tela
    GtkWidget *frame_resultado;     // Guarda o container da caixinha gráfica
    GtkWidget *btn_diagnostico;     // Botao de gerar diagnostico
} PreDiagCampos;

/**
 * @brief Estrutura que agrupa os componentes do prontuário, como label, que diferente do entry, são só de exibição.
 */
typedef struct {
    GtkWidget *lbl_nome;
    GtkWidget *lbl_email;
    GtkWidget *lbl_cpf;
    GtkWidget *lbl_data_nasc;
    GtkWidget *lbl_telefone;
    GtkWidget *listbox_laudos;
} ProntuarioViewCampos;
static ProntuarioViewCampos g_pron_view; // labels da tela de prontuario que serão preenchidas no criar_tela_prontuario_view (futuro)

/**
 * @brief Estrutura que agrupa os componentes da tela dedicada a visualização de um Laudo
 */
typedef struct {
    GtkWidget *lbl_data;
    GtkWidget *lbl_diag;
    GtkWidget *lbl_age;
    GtkWidget *lbl_anb;
    GtkWidget *lbl_coa;
    GtkWidget *lbl_cogn;
    GtkWidget *lbl_afai;
    GtkWidget *lbl_sngogn;
    GtkWidget *lbl_na1_dist;
    GtkWidget *lbl_na1_ang;
    GtkWidget *lbl_na2_dist;
    GtkWidget *lbl_na2_ang;
    GtkWidget *lbl_max_tipo;
    GtkWidget *lbl_max_desvio;
    GtkWidget *lbl_perfil;
} LaudoViewCampos;
static LaudoViewCampos g_laudo_view;

/**
 * @brief Estrutura para Cadastro de Dentista
 */
typedef struct {
    GtkWidget *entry_nome;
    GtkWidget *entry_username;
    GtkWidget *entry_cpf;
    GtkWidget *entry_senha;
    GtkWidget *btn_salvar;
} CadastroDentistaCampos;
static CadastroDentistaCampos g_campos_cad_dentista;

/**
 * @brief Estrutura para Edicao de Paciente
 */
typedef struct {
    GtkWidget *entry_nome;
    GtkWidget *entry_email;
    GtkWidget *entry_cpf;
    GtkWidget *entry_data_nasc;
    GtkWidget *entry_telefone;
} EdicaoPacienteCampos;
static EdicaoPacienteCampos g_campos_edicao_pac;

// ============================================================================
// FUNÇÕES DE UTILIDADE E CALLBACKS
// ============================================================================

/**
 * @brief Remove caracteres especiais do CPF.
 */
static void clean_cpf(const char *input, char *output) {
    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] != '.' && input[i] != '-' && input[i] != ' ') {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

/**
 * @brief Callback disparado quando o botão "Entrar" é clicado.
 * Esta função captura os textos e faz a transição de tela usando o GtkStack.
 */
static void on_btn_entrar_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    // 1. Recupera-se a struct com os campos de texto atraves do ponteiro generico
    LoginCampos *campos = (LoginCampos *)user_data;

    // 2. Extrai as strings que o usuario digitou
    const char *usuario = gtk_editable_get_text(GTK_EDITABLE(campos->entry_usuario));
    const char *senha = gtk_editable_get_text(GTK_EDITABLE(campos->entry_senha));

    // 3. Print de debug no terminal (útil para testes acadêmicos)
    log_message(LOG_INFO, "[GUI] Botão Entrar Clicado.");
    log_message(LOG_INFO, "[GUI] Usuário inserido: %s", usuario);

    char clean_user[100];
    strcpy(clean_user, usuario);
    
    // Verifica se a string parece um CPF (apenas números, pontos e traços)
    int is_cpf = 1;
    for (int i = 0; usuario[i] != '\0'; i++) {
        if (!((usuario[i] >= '0' && usuario[i] <= '9') || usuario[i] == '.' || usuario[i] == '-')) {
            is_cpf = 0;
            break;
        }
    }
    
    // Se for um CPF, limpamos a máscara (pontos e traços)
    if (is_cpf) {
        clean_cpf(usuario, clean_user);
    }

    int role = validate_login(clean_user, senha);
    if (role != -1) {
        Dentist d = find_dentist_by_cpf(clean_user);
        g_logged_dentist_id = d.dentist_id;

        // Sucesso no login
        gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
        
        // Verifica se é admin (role == 1) para habilitar o botão
        if (role == 1) {
            gtk_widget_set_visible(g_btn_cadastrar_dentista, TRUE);
        } else {
            gtk_widget_set_visible(g_btn_cadastrar_dentista, FALSE);
        }

        // Limpa os campos
        gtk_editable_set_text(GTK_EDITABLE(campos->entry_usuario), "");
        gtk_editable_set_text(GTK_EDITABLE(campos->entry_senha), "");
    } else {
        log_message(LOG_WARNING, "[GUI] Login falhou para o usuário: %s", usuario);
        // Opcional: mostrar mensagem de erro
    }
}

/**
 * @brief Callback disparado quando o botão "Cadastrar Dentista" no login é clicado.
 */
static void on_btn_cadastrar_dentista_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    log_message(LOG_INFO, "[GUI] Botão de menu clicado: Cadastrar Dentista");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "cadastro_dentista_page");
}

/**
 * @brief Função auxiliar para limpar tela de busca
 */
static void limpar_tela_busca(void) {
    if (g_campos_busca.entry_busca) {
        gtk_editable_set_text(GTK_EDITABLE(g_campos_busca.entry_busca), "");
    }
}

/**
 * @brief Função auxiliar para limpar tela de cadastro de dentista
 */
static void limpar_campos_cadastro_dentista(void) {
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cad_dentista.entry_nome), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cad_dentista.entry_username), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cad_dentista.entry_cpf), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cad_dentista.entry_senha), "");
}

/**
 * @brief Callback disparado quando os campos do cadastro de dentista mudam
 */
static void on_cadastro_dentista_entry_changed(GtkEditable *editable, gpointer user_data) {
    (void)editable;
    CadastroDentistaCampos *campos = (CadastroDentistaCampos *)user_data;
    const char *nome = gtk_editable_get_text(GTK_EDITABLE(campos->entry_nome));
    const char *username = gtk_editable_get_text(GTK_EDITABLE(campos->entry_username));
    const char *cpf = gtk_editable_get_text(GTK_EDITABLE(campos->entry_cpf));
    const char *senha = gtk_editable_get_text(GTK_EDITABLE(campos->entry_senha));

    gboolean pronto = (strlen(nome) > 0 && strlen(username) > 0 && strlen(cpf) > 0 && strlen(senha) > 0);
    gtk_widget_set_sensitive(campos->btn_salvar, pronto);
}

/**
 * @brief Callback disparado quando salvar o dentista
 */
static void on_btn_salvar_dentista_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    CadastroDentistaCampos *campos = (CadastroDentistaCampos *)user_data;
    Dentist d;
    
    strncpy(d.name, gtk_editable_get_text(GTK_EDITABLE(campos->entry_nome)), sizeof(d.name)-1);
    strncpy(d.username, gtk_editable_get_text(GTK_EDITABLE(campos->entry_username)), sizeof(d.username)-1);
    const char *raw_cpf = gtk_editable_get_text(GTK_EDITABLE(campos->entry_cpf));
    clean_cpf(raw_cpf, d.cpf);
    strncpy(d.password, gtk_editable_get_text(GTK_EDITABLE(campos->entry_senha)), sizeof(d.password)-1);
    
    // Validação do CPF (11 dígitos)
    if (strlen(d.cpf) != 11) {
        GtkAlertDialog *alert = gtk_alert_dialog_new("Dados Inválidos");
        gtk_alert_dialog_set_detail(alert, "O CPF deve conter exatamente 11 números.");
        gtk_alert_dialog_show(alert, NULL);
        g_object_unref(alert);
        return;
    }

    // Verificar duplicidade de CPF no banco de dados (já que username ou cpf são únicos para login)
    Dentist existing = find_dentist_by_cpf(d.cpf);
    if (existing.dentist_id != (uint64_t)-1) {
        GtkAlertDialog *alert = gtk_alert_dialog_new("Erro de Cadastro");
        gtk_alert_dialog_set_detail(alert, "Já existe um dentista cadastrado com este CPF.");
        gtk_alert_dialog_show(alert, NULL);
        g_object_unref(alert);
        return;
    }
    
    d.role = 2; // Dentista padrão
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
 * @brief Callback para voltar do cadastro de dentista
 */
static void on_btn_voltar_cad_dentista_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    log_message(LOG_INFO, "[GUI] Retornando do Cadastro de Dentista para a Dashboard...");
    limpar_campos_cadastro_dentista();
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
}

/**
 * @brief Callback disparado quando o botão "Sair" da Dashboard é clicado.
 * Faz o GtkStack reverter a animação de volta para a tela de login.
 */
static void on_btn_sair_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    log_message(LOG_INFO, "[GUI] Logout do sistema. Voltando para Login...");
    gtk_widget_set_visible(g_btn_cadastrar_dentista, FALSE); // Oculta por segurança no logout
    
    // Faz o Stack exibir a tela de login novamente com animação reversa automática
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "login_page");
}

static void atualizar_lista_prontuarios(const char *filtro);

/**
 * @brief Callback disparado ao clicar em qualquer botão do Menu Principal (Dashboard).
 * Redireciona para a tela correspondente no Stack.
 */
static void on_menu_button_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    const char *label = gtk_button_get_label(btn);

    if (strstr(label, "Cadastrar Pacientes")) {
        log_message(LOG_INFO, "[GUI] Botão de menu clicado: Cadastrar Pacientes");
        gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "cadastro_page");
    } else if (strstr(label, "Gerenciar Prontuários")) {
        log_message(LOG_INFO, "[GUI] Botão de menu clicado: Gerenciar Prontuários");
        // Atualiza a lista antes de mostrar a tela
        atualizar_lista_prontuarios("");
        gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuarios_page");
    }
}

/**
 * @brief Callback para o botão "Voltar" da Tela de Prontuários.
 * Faz o GtkStack deslizar de volta para a Dashboard.
 */
static void on_btn_voltar_prontuarios_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    log_message(LOG_INFO, "[GUI] Retornando dos Prontuários para a Dashboard...");
    limpar_tela_busca();
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
}

/**
 * @brief Atualiza a listbox de pacientes com base no filtro de busca.
 */
static void atualizar_lista_prontuarios(const char *filtro) {
    if (!g_campos_busca.listbox) return;

    // Limpa filhos atuais da listbox
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
            if (filtro && strlen(filtro) > 0) {
                // simple case insensitive check
                char nome_lower[256];
                strncpy(nome_lower, pacientes[i].name, 255);
                nome_lower[255] = '\0';
                for (int j = 0; nome_lower[j]; j++) nome_lower[j] = tolower((unsigned char)nome_lower[j]);
                if (strstr(nome_lower, filtro) == NULL) {
                    matches = FALSE;
                }
            }

            if (matches) {
                char label_str[256];
                snprintf(label_str, sizeof(label_str), "📝 %s (CPF: %s)", pacientes[i].name, pacientes[i].cpf);
                GtkWidget *label = gtk_label_new(label_str);
                gtk_widget_set_halign(label, GTK_ALIGN_START);
                
                GtkWidget *row = gtk_list_box_row_new();
                gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
                
                // Anexa o ID ao row para ser pego quando clicado
                uint64_t *id_ptr = g_malloc(sizeof(uint64_t));
                *id_ptr = pacientes[i].patient_id;
                g_object_set_data_full(G_OBJECT(row), "patient_id", id_ptr, g_free);
                
                gtk_list_box_append(GTK_LIST_BOX(g_campos_busca.listbox), row);
                achou = TRUE;
            }
        }
        free(pacientes);
    }

    if (!achou) {
        GtkWidget *lbl_vazio = gtk_label_new("  ❌ Nenhum paciente encontrado.");
        gtk_list_box_append(GTK_LIST_BOX(g_campos_busca.listbox), lbl_vazio);
    }
}

/**
 * @brief Callback do botão de buscar.
 */
static void on_btn_buscar_clicked(GtkButton *btn, gpointer user_data) { 
    (void)btn;
    //Muda o user_data pata BuscaCampos agr levando ao ponteiro da busca
    BuscaCampos *busca = (BuscaCampos *)user_data;
    const char *texto = gtk_editable_get_text(GTK_EDITABLE(busca->entry_busca));

    printf("[GUI] Buscando por: %s\n", texto);

    // 3. Atualiza a interface
    gchar *busca_lower = g_utf8_strdown(texto, -1);
    atualizar_lista_prontuarios(busca_lower);

    g_free(busca_lower); 
}

/**
 * @brief Callback disparado quando o botão "Salvar Ficha" da tela de Cadastro é clicado.
 * Captura os dados textuais inseridos na interface visual e redireciona para a tela de pré-diagnóstico.
 */
static void on_btn_salvar_cadastro_clicked(GtkButton *btn, gpointer user_data) {
    (void)btn;
    // 1. Recupera a struct com os campos de entrada atraves do ponteiro generico
    CadastroCampos *campos = (CadastroCampos *)user_data;

    // 2. Extrai as strings contidas em cada GtkEntry mapeado
    const char *nome = gtk_editable_get_text(GTK_EDITABLE(campos->entry_nome));
    const char *email = gtk_editable_get_text(GTK_EDITABLE(campos->entry_email));
    const char *raw_cpf = gtk_editable_get_text(GTK_EDITABLE(campos->entry_cpf));
    
    char cpf[20];
    clean_cpf(raw_cpf, cpf);

    const char *data_nasc = gtk_editable_get_text(GTK_EDITABLE(campos->entry_data_nasc));
    const char *telefone = gtk_editable_get_text(GTK_EDITABLE(campos->entry_telefone));

    char erro_msg[512] = "";
    
    if (strlen(cpf) != 11) {
        strcat(erro_msg, "- CPF deve conter exatamente 11 números.\n");
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
    
    if (strlen(email) > 0 && (strchr(email, '@') == NULL || strchr(email, '.') == NULL)) {
        strcat(erro_msg, "- E-mail informado possui formato inválido.\n");
    }

    if (strlen(erro_msg) > 0) {
        char detail_msg[1024];
        snprintf(detail_msg, sizeof(detail_msg), "Por favor, corrija os seguintes erros:\n\n%s", erro_msg);
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

    // 3. Print de debug estruturado usando o logger unificado do sistema
    log_message(LOG_INFO, "[GUI] Processando novo cadastro de paciente...");
    /*log_message(LOG_INFO, " |- Nome: %s", nome);
    log_message(LOG_INFO, " |- Email: %s", (strlen(email) == 0) ? "(Não Informado)" : email);
    log_message(LOG_INFO, " |- CPF: %s", cpf);
    log_message(LOG_INFO, " |- Data de Nasc: %s", data_formatada);
    log_message(LOG_INFO, " |- Telefone: %s", telefone);
    */
    
    // Preenche variável global
    uint64_t existing_id = g_current_patient.patient_id;
    memset(&g_current_patient, 0, sizeof(Patient));
    g_current_patient.patient_id = existing_id;
    
    strncpy(g_current_patient.name, nome, sizeof(g_current_patient.name)-1);
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

    // 4. TRANSIÇÃO DE FLUXO: Avança para a tela de preenchimento cefalométrico/clínico
    log_message(LOG_INFO, "[GUI] Dados cadastrais retidos. Avançando para a página de Pré-Diagnóstico...");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "pre_diagnostico_page");
}

/**
 * @brief Função auxiliar interna para limpar todos os campos da tela de pré-diagnóstico
 * preparando a interface para um próximo paciente do ciclo.
 */
static void limpar_campos_pre_diagnostico(PreDiagCampos *campos) {
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_height), "");
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_weight), "");
    gtk_editable_set_text(GTK_EDITABLE(campos->entry_age), "");
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
    
    // Oculta o card de resultado novamente para o próximo caso
    gtk_widget_set_visible(campos->frame_resultado, FALSE);
}

/**
 * @brief Limpa os campos da tela de Cadastro Básico (tela 1)
 */
static void limpar_campos_cadastro_basico() {
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_nome), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_email), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_cpf), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_data_nasc), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_telefone), "");
    
    memset(&g_current_patient, 0, sizeof(Patient));

    if (g_campos_cadastro.btn_salvar) {
        gtk_widget_set_sensitive(g_campos_cadastro.btn_salvar, FALSE);
    }
}

/**
 * @brief Callback disparado quando os campos obrigatórios do cadastro são alterados
 */
static void on_cadastro_paciente_entry_changed(GtkEditable *editable, gpointer user_data) {
    (void)editable;
    CadastroCampos *campos = (CadastroCampos *)user_data;
    
    if (!campos->btn_salvar) return;

    const char *nome = gtk_editable_get_text(GTK_EDITABLE(campos->entry_nome));
    const char *cpf = gtk_editable_get_text(GTK_EDITABLE(campos->entry_cpf));
    const char *data_nasc = gtk_editable_get_text(GTK_EDITABLE(campos->entry_data_nasc));
    const char *telefone = gtk_editable_get_text(GTK_EDITABLE(campos->entry_telefone));
    
    gboolean is_valid = (strlen(nome) > 0 && strlen(cpf) > 0 && strlen(data_nasc) > 0 && strlen(telefone) > 0);
    
    gtk_widget_set_sensitive(campos->btn_salvar, is_valid);
}

/**
 * @brief Forward declaration do callback de geração de diagnóstico para remontagem do ponteiro.
 */
static void on_btn_gerar_diagnostico_clicked(GtkButton *btn, gpointer user_data);

/**
 * @brief Callback disparado ao clicar em "Concluir e Novo Cadastro".
 * Limpa completamente ambas as telas e retorna o fluxo à tela inicial de cadastro de pacientes.
 */
static void on_btn_concluir_fluxo_clicked(GtkButton *btn, gpointer user_data) {
    PreDiagCampos *campos = (PreDiagCampos *)user_data;
    
    if (g_is_standalone_laudo) {
        // Fluxo de laudo avulso para paciente existente
        g_current_clinical_record.patient_id = g_selected_patient_id;
        g_current_clinical_record.dentist_id = g_logged_dentist_id;
        
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        char temp_date[32];
        snprintf(temp_date, sizeof(temp_date), "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
        strncpy(g_current_clinical_record.diag_date, temp_date, sizeof(g_current_clinical_record.diag_date) - 1);
        g_current_clinical_record.diag_date[sizeof(g_current_clinical_record.diag_date) - 1] = '\0';
        
        save_clinical_record(&g_current_clinical_record);
        log_message(LOG_INFO, "[GUI] Laudo avulso salvo no BD.");
        
        // Limpa a tela
        limpar_campos_pre_diagnostico(campos);
        
        // Restaura a semântica e funcionalidade do botão original do formulário
        gtk_button_set_label(btn, "🧠 Gerar Pré-Diagnóstico");
        g_signal_handlers_disconnect_by_func(btn, G_CALLBACK(on_btn_concluir_fluxo_clicked), campos);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_btn_gerar_diagnostico_clicked), campos);
        
        g_is_standalone_laudo = FALSE;
        
        // Retorna para a tela de prontuario atualizada
        carregar_tela_prontuario_por_id(g_selected_patient_id);
    } else {
        // Fluxo original de cadastro completo
        // Salvar laudo no BD (paciente já foi salvo na etapa anterior)
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
        
        // 1. Limpa os dados de ambas as etapas visuais para evitar vazamento de memória visual
        limpar_campos_pre_diagnostico(campos);
        limpar_campos_cadastro_basico();
        
        // 2. Restaura a semântica e funcionalidade do botão original do formulário
        gtk_button_set_label(btn, "🧠 Gerar Pré-Diagnóstico");
        g_signal_handlers_disconnect_by_func(btn, G_CALLBACK(on_btn_concluir_fluxo_clicked), campos);
        g_signal_connect(btn, "clicked", G_CALLBACK(on_btn_gerar_diagnostico_clicked), campos);
        
        // 3. Joga o dentista de volta na primeira tela de dados cadastrais
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
    
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_height))) == 0) is_filled = FALSE;
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_weight))) == 0) is_filled = FALSE;
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_age))) == 0) is_filled = FALSE;
    if (strlen(gtk_editable_get_text(GTK_EDITABLE(c->entry_anb))) == 0) is_filled = FALSE;
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
    PreDiagCampos *campos = (PreDiagCampos *)user_data;

    // 1. Coleta e decodificação do índice selecionado no GtkDropDown (0=Normal, 1=Protruída, 2=Retruída)
    guint idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(campos->dropdown_max_tipo));
    int valor_maxila_tipo = 0; 
    if (idx == 1) valor_maxila_tipo = 1;   
    if (idx == 2) valor_maxila_tipo = -1;  

    // Preenche g_current_clinical_record
    memset(&g_current_clinical_record, 0, sizeof(ClinicalRecord));
    g_current_clinical_record.age = atoi(gtk_editable_get_text(GTK_EDITABLE(campos->entry_age)));
    g_current_clinical_record.anb = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_anb)));
    g_current_clinical_record.coa = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_coa)));
    g_current_clinical_record.maxila_tipo = valor_maxila_tipo;
    g_current_clinical_record.maxila_desvio = atoi(gtk_editable_get_text(GTK_EDITABLE(campos->entry_max_desvio)));
    g_current_clinical_record.cogn = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_cogn)));
    g_current_clinical_record.afai = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_afai)));
    g_current_clinical_record.sngogn = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_sngogn)));
    g_current_clinical_record.na1_dist = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_na1_dist)));
    g_current_clinical_record.na1_ang = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_na1_ang)));
    g_current_clinical_record.nb1_dist = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_na2_dist)));
    g_current_clinical_record.nb1_ang = atof(gtk_editable_get_text(GTK_EDITABLE(campos->entry_na2_ang)));
    strncpy(g_current_clinical_record.perf_tegument, gtk_editable_get_text(GTK_EDITABLE(campos->entry_perf_tegu)), sizeof(g_current_clinical_record.perf_tegument)-1);

    // Formular diagnóstico através da árvore de decisão!
    clinical_formular_diag(&g_current_clinical_record);

    // 2. Impressão estruturada de diagnóstico nos logs do terminal
    log_message(LOG_INFO, "[GUI] Coletando métricas estruturadas de ClinicalRecord para processamento:");
    log_message(LOG_INFO, " |- Laudo: %s", g_current_clinical_record.pre_diagnosis);
    log_message(LOG_INFO, "[GUI] Registro clínico consolidado com sucesso!");

    // --- GRÁFICO DO CARD DE RESULTADO RENDERIZADO COM ESTILO PANGO ---
    char markup[2048];
    snprintf(markup, sizeof(markup),
        "<span size='x-large' weight='bold' foreground='#1b5e20'>Pré-Diagnóstico</span>\n\n"
        "<span size='large' foreground='#424242'>%s</span>\n\n"
        "<i>Ficha consolidada com sucesso e pronta para persistência.</i>",
        g_current_clinical_record.pre_diagnosis
    );

    gtk_label_set_markup(GTK_LABEL(campos->lbl_resultado), markup);

    // Torna visível a estrutura gráfica do card na tela
    gtk_widget_set_visible(campos->frame_resultado, TRUE);

    // Configura o botão para avançar em loop de cadastro
    gtk_button_set_label(btn, "✨ Concluir e Novo Cadastro");

    // Desconecta e chaveia o fluxo para a função de reset e loop infinito
    g_signal_handlers_disconnect_by_func(btn, G_CALLBACK(on_btn_gerar_diagnostico_clicked), campos);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_btn_concluir_fluxo_clicked), campos);
}
/**
 * @brief Callback do botão Voltar da tela de laudo dedicado
 */
static void on_btn_voltar_laudo_view_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuario_view_page");
}

/**
 * @brief Callback disparado ao clicar em um laudo na listagem do prontuário
 */
void on_laudo_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    (void)user_data;
    (void)box;
    if (row == NULL) return;

    ClinicalRecord *cr = g_object_get_data(G_OBJECT(row), "clinical_record");
    if (!cr) return;

    // Popula a struct g_laudo_view
    char buf[512];
    snprintf(buf, sizeof(buf), "Data do Laudo: %s", cr->diag_date);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_data), buf);

    snprintf(buf, sizeof(buf), "%s", cr->pre_diagnosis);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_diag), buf);

    snprintf(buf, sizeof(buf), "Idade: %d", cr->age);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_age), buf);

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

    snprintf(buf, sizeof(buf), "Perfil Tegumentar: %s", cr->perf_tegument);
    gtk_label_set_text(GTK_LABEL(g_laudo_view.lbl_perfil), buf);

    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "laudo_view_page");
}

/**
 * @brief constroi a tela de visualização de um laudo dedicado
 */
static GtkWidget* criar_tela_laudo_view(void) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    // parte de cima da tela
    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar ao Paciente");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_laudo_view_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);
    
    GtkWidget *lbl_titulo = gtk_label_new("Detalhes do Pré-Diagnóstico Cefalométrico");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_label_set_markup(GTK_LABEL(lbl_titulo), "<span size='large' weight='bold'>Detalhes do Pré-Diagnóstico Cefalométrico</span>");
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);

    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // Data do laudo, logo abaixo do título
    g_laudo_view.lbl_data = gtk_label_new("Data do Laudo: —");
    gtk_widget_set_halign(g_laudo_view.lbl_data, GTK_ALIGN_START);
    gtk_widget_set_margin_start(g_laudo_view.lbl_data, 5);
    gtk_box_append(GTK_BOX(vbox), g_laudo_view.lbl_data);

    // Separador removido pois os frames já delimitam visualmente

    // Frame para Medidas Clínicas
    GtkWidget *frame_medidas = gtk_frame_new(NULL);
    GtkWidget *lbl_frame_medidas = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_frame_medidas), "<b> Medidas Clínicas </b>");
    gtk_frame_set_label_widget(GTK_FRAME(frame_medidas), lbl_frame_medidas);
    gtk_widget_set_margin_top(frame_medidas, 10);
    GtkWidget *vbox_medidas = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 40);
    gtk_widget_set_margin_start(grid, 15);
    gtk_widget_set_margin_top(grid, 10);
    gtk_widget_set_margin_bottom(grid, 15);

    g_laudo_view.lbl_age = gtk_label_new("Idade: —");
    gtk_widget_set_halign(g_laudo_view.lbl_age, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_age, 0, 0, 1, 1);

    g_laudo_view.lbl_anb = gtk_label_new("Ângulo ANB: —");
    gtk_widget_set_halign(g_laudo_view.lbl_anb, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_anb, 1, 0, 1, 1);

    g_laudo_view.lbl_coa = gtk_label_new("COA: —");
    gtk_widget_set_halign(g_laudo_view.lbl_coa, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_coa, 0, 1, 1, 1);

    g_laudo_view.lbl_cogn = gtk_label_new("CO-GN: —");
    gtk_widget_set_halign(g_laudo_view.lbl_cogn, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_cogn, 1, 1, 1, 1);

    g_laudo_view.lbl_afai = gtk_label_new("AFAI: —");
    gtk_widget_set_halign(g_laudo_view.lbl_afai, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_afai, 0, 2, 1, 1);

    g_laudo_view.lbl_sngogn = gtk_label_new("SN.GO.GN: —");
    gtk_widget_set_halign(g_laudo_view.lbl_sngogn, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_sngogn, 1, 2, 1, 1);

    g_laudo_view.lbl_na1_dist = gtk_label_new("NA Sup. Dist: —");
    gtk_widget_set_halign(g_laudo_view.lbl_na1_dist, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_na1_dist, 0, 3, 1, 1);

    g_laudo_view.lbl_na1_ang = gtk_label_new("NA Sup. Âng: —");
    gtk_widget_set_halign(g_laudo_view.lbl_na1_ang, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_na1_ang, 1, 3, 1, 1);

    g_laudo_view.lbl_na2_dist = gtk_label_new("NB Inf. Dist: —");
    gtk_widget_set_halign(g_laudo_view.lbl_na2_dist, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_na2_dist, 0, 4, 1, 1);

    g_laudo_view.lbl_na2_ang = gtk_label_new("NB Inf. Âng: —");
    gtk_widget_set_halign(g_laudo_view.lbl_na2_ang, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_na2_ang, 1, 4, 1, 1);

    g_laudo_view.lbl_max_tipo = gtk_label_new("Tipo Maxila: —");
    gtk_widget_set_halign(g_laudo_view.lbl_max_tipo, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_max_tipo, 0, 5, 1, 1);

    g_laudo_view.lbl_max_desvio = gtk_label_new("Desvio Maxila: —");
    gtk_widget_set_halign(g_laudo_view.lbl_max_desvio, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_max_desvio, 1, 5, 1, 1);

    g_laudo_view.lbl_perfil = gtk_label_new("Perfil Tegumentar: —");
    gtk_widget_set_halign(g_laudo_view.lbl_perfil, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), g_laudo_view.lbl_perfil, 0, 6, 2, 1);

    gtk_box_append(GTK_BOX(vbox_medidas), grid);
    gtk_frame_set_child(GTK_FRAME(frame_medidas), vbox_medidas);
    gtk_box_append(GTK_BOX(vbox), frame_medidas);

    // Frame de destaque para o Diagnóstico
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

    g_laudo_view.lbl_diag = gtk_label_new("—");
    gtk_widget_set_halign(g_laudo_view.lbl_diag, GTK_ALIGN_START);
    // Permite quebra de linha caso o texto seja muito longo
    gtk_label_set_wrap(GTK_LABEL(g_laudo_view.lbl_diag), TRUE);
    gtk_box_append(GTK_BOX(vbox_destaque), g_laudo_view.lbl_diag);

    gtk_frame_set_child(GTK_FRAME(frame_destaque), vbox_destaque);
    gtk_box_append(GTK_BOX(vbox), frame_destaque);

    return vbox;
}

/**
 * @brief Callback do botao Novo Laudo
 */
static void on_btn_novo_laudo_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    g_is_standalone_laudo = TRUE;
    // O ID do paciente já está em g_selected_patient_id
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "pre_diagnostico_page");
}

void carregar_tela_prontuario_por_id(uint64_t patient_id) {
    g_selected_patient_id = patient_id;
    Patient p = find_patient_by_id(patient_id);
    if (p.patient_id == (uint64_t)-1) {
        log_message(LOG_ERROR, "[GUI] Falha ao carregar paciente selecionado (ID: %llu).", (unsigned long long)patient_id);
        return;
    }

    // Atualiza Interface (g_pron_view)
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

    // Limpar filhos atuais do listbox de laudos
    if (g_pron_view.listbox_laudos) {
        GtkWidget *child = gtk_widget_get_first_child(g_pron_view.listbox_laudos);
        while (child != NULL) {
            GtkWidget *next = gtk_widget_get_next_sibling(child);
            gtk_list_box_remove(GTK_LIST_BOX(g_pron_view.listbox_laudos), child);
            child = next;
        }
    }

    // Busca ClinicalRecord
    int total_records = 0;
    ClinicalRecord *records = load_clinical_records(patient_id, &total_records);
    if (records != NULL && total_records > 0) {
        for (int i = 0; i < total_records; i++) {
            ClinicalRecord *cr = g_malloc(sizeof(ClinicalRecord));
            memcpy(cr, &records[i], sizeof(ClinicalRecord));

            char row_label_str[1024];
            snprintf(row_label_str, sizeof(row_label_str), "📄 Pré-Diagnóstico (%s) - %s", cr->diag_date, cr->pre_diagnosis);
            
            GtkWidget *row_label = gtk_label_new(row_label_str);
            gtk_widget_set_halign(row_label, GTK_ALIGN_START);
            
            GtkWidget *row_list = gtk_list_box_row_new();
            gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row_list), row_label);
            
            // Passa ownership da memória do laudo para a linha
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

/**
 * @brief Callback para quando é clicado em um paciente da lista de prontuários.
 */
static void on_prontuario_row_activated(GtkListBox *listbox, GtkListBoxRow *row, gpointer user_data) {
    (void)user_data;
    (void)listbox;
    if (row == NULL) return;

    // Recupera o ID salvo na Row
    uint64_t *id_ptr = g_object_get_data(G_OBJECT(row), "patient_id");
    if (!id_ptr) {
        log_message(LOG_ERROR, "[GUI] Ponteiro de ID de paciente ausente na row.");
        return;
    }
    carregar_tela_prontuario_por_id(*id_ptr);
}
/**
 * @brief callback do botão voltar da tela de prontuarios 
 */
static void on_btn_voltar_pron_view_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuarios_page");
}

/**
 * @brief Função auxiliar para limpar tela de edição de paciente
 */
static void limpar_campos_edicao_paciente(void) {
    gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_nome), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_email), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_cpf), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_data_nasc), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_telefone), "");
}

/**
 * @brief Callback disparado quando salvar a edição
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
    // Atualiza a view do paciente
    atualizar_lista_prontuarios("");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuarios_page");
}

/**
 * @brief Callback para voltar da edição de paciente
 */
static void on_btn_voltar_edicao_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    limpar_campos_edicao_paciente();
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuario_view_page");
}

/**
 * @brief callback pro botão editar (para o futuro)
 */
static void on_btn_editar_pron_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    log_message(LOG_INFO, "[GUI] Editar prontuário — carregando dados para edição.");
    Patient p = find_patient_by_id(g_selected_patient_id);
    if (p.patient_id != (uint64_t)-1) {
        gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_nome), p.name);
        gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_email), p.email);
        gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_cpf), p.cpf);
        gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_data_nasc), p.birth_date);
        gtk_editable_set_text(GTK_EDITABLE(g_campos_edicao_pac.entry_telefone), p.phone);
    }
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "edicao_paciente_page");
}

/**
 * @brief callback do botão excluir  (ainda somente visual)
 */

static void on_btn_excluir_pron_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    log_message(LOG_INFO, "[GUI] Excluindo prontuário e paciente (ID: %" PRIu64 ").", g_selected_patient_id);
    delete_patient(g_selected_patient_id);
    atualizar_lista_prontuarios("");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuarios_page");
}
// ============================================================================
// CONSTRUTORES DE INTERFACES (TELAS)
// ============================================================================

/**
 * @brief Constrói e organiza os elementos visuais da Tela de Login.
 * @return Retorna um GtkWidget (GtkBox) contendo toda a interface de login pronta.
 */
static GtkWidget* criar_tela_login(LoginCampos *campos) {
    // Cria uma caixa vertical com 10 pixels de espacamento entre os componentes
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER); 
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER); 

    // Componente Título
    GtkWidget *lbl_titulo = gtk_label_new("ODONTOSYS");
    gtk_widget_set_margin_bottom(lbl_titulo, 15); 
    gtk_box_append(GTK_BOX(vbox), lbl_titulo);

    // Componente Campo Usuário
    GtkWidget *lbl_usuario = gtk_label_new("Usuário ou CPF:");
    gtk_widget_set_halign(lbl_usuario, GTK_ALIGN_START); 
    gtk_box_append(GTK_BOX(vbox), lbl_usuario);

    campos->entry_usuario = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_usuario), "Usuário ou CPF");
    gtk_widget_set_size_request(campos->entry_usuario, 250, -1); 
    gtk_box_append(GTK_BOX(vbox), campos->entry_usuario);

    // Componente  Campo Senha
    GtkWidget *lbl_senha = gtk_label_new("Senha:");
    gtk_widget_set_halign(lbl_senha, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_senha);

    campos->entry_senha = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(campos->entry_senha), FALSE); 
    gtk_box_append(GTK_BOX(vbox), campos->entry_senha);

    // Area do Botão de Envio
    GtkWidget *btn_entrar = gtk_button_new_with_label("Entrar");
    gtk_widget_set_margin_top(btn_entrar, 15);
    
    // Conecta o evento de clique do botão à nossa função de callback 
    g_signal_connect(btn_entrar, "clicked", G_CALLBACK(on_btn_entrar_clicked), campos);
    
    gtk_box_append(GTK_BOX(vbox), btn_entrar);

    return vbox; 
}

/**
 * @brief Constrói e organiza os elementos visuais da Tela de Cadastro de Dentista
 */
static GtkWidget* criar_tela_cadastro_dentista(CadastroDentistaCampos *campos) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_cad_dentista_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);

    GtkWidget *lbl_titulo = gtk_label_new("Cadastro de Dentista");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);
    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // Nome
    GtkWidget *lbl_nome = gtk_label_new("Nome:");
    gtk_widget_set_halign(lbl_nome, GTK_ALIGN_START);
    campos->entry_nome = gtk_entry_new();
    g_signal_connect(campos->entry_nome, "changed", G_CALLBACK(on_cadastro_dentista_entry_changed), campos);
    gtk_box_append(GTK_BOX(vbox), lbl_nome);
    gtk_box_append(GTK_BOX(vbox), campos->entry_nome);

    // Username
    GtkWidget *lbl_username = gtk_label_new("Nome de Usuário (Login):");
    gtk_widget_set_halign(lbl_username, GTK_ALIGN_START);
    campos->entry_username = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_username), "Username");
    g_signal_connect(campos->entry_username, "changed", G_CALLBACK(on_cadastro_dentista_entry_changed), campos);
    gtk_box_append(GTK_BOX(vbox), lbl_username);
    gtk_box_append(GTK_BOX(vbox), campos->entry_username);

    // CPF
    GtkWidget *lbl_cpf = gtk_label_new("CPF:");
    gtk_widget_set_halign(lbl_cpf, GTK_ALIGN_START);
    campos->entry_cpf = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_cpf), "000.000.000-00");
    g_signal_connect(campos->entry_cpf, "changed", G_CALLBACK(on_cadastro_dentista_entry_changed), campos);
    gtk_box_append(GTK_BOX(vbox), lbl_cpf);
    gtk_box_append(GTK_BOX(vbox), campos->entry_cpf);

    // Senha
    GtkWidget *lbl_senha = gtk_label_new("Senha:");
    gtk_widget_set_halign(lbl_senha, GTK_ALIGN_START);
    campos->entry_senha = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(campos->entry_senha), FALSE);
    g_signal_connect(campos->entry_senha, "changed", G_CALLBACK(on_cadastro_dentista_entry_changed), campos);
    gtk_box_append(GTK_BOX(vbox), lbl_senha);
    gtk_box_append(GTK_BOX(vbox), campos->entry_senha);

    campos->btn_salvar = gtk_button_new_with_label("💾 Salvar");
    gtk_widget_set_margin_top(campos->btn_salvar, 15);
    gtk_widget_set_sensitive(campos->btn_salvar, FALSE); // Começa desabilitado
    g_signal_connect(campos->btn_salvar, "clicked", G_CALLBACK(on_btn_salvar_dentista_clicked), campos);
    gtk_box_append(GTK_BOX(vbox), campos->btn_salvar);

    return vbox;
}

/**
 * @brief Constrói e organiza a interface real da Tela 2 (Dashboard / Painel de Controle).
 * @return Retorna um GtkWidget (GtkBox) com o menu principal ajustado sem agendamentos.
 */
static GtkWidget* criar_tela_dashboard(void) {
    // 1. Cria a caixa vertical principal com 15 pixels de espacamento
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

    // 4. Criando o Botão "Cadastrar Dentista" (restrito para admins)
    g_btn_cadastrar_dentista = gtk_button_new_with_label("👨‍⚕️ Cadastrar Dentista");
    gtk_widget_set_size_request(g_btn_cadastrar_dentista, 250, 40);
    g_signal_connect(g_btn_cadastrar_dentista, "clicked", G_CALLBACK(on_btn_cadastrar_dentista_clicked), NULL);
    gtk_widget_set_visible(g_btn_cadastrar_dentista, FALSE); // Oculto por padrão
    gtk_box_append(GTK_BOX(vbox), g_btn_cadastrar_dentista);

    // 5. Criando o Botão "Sair" com destaque inferior
    GtkWidget *btn_sair = gtk_button_new_with_label("🚪 Sair do Sistema");
    gtk_widget_set_size_request(btn_sair, 250, 40);
    gtk_widget_set_margin_top(btn_sair, 20); 
    
    // Conecta o evento de clique para reverter para a tela de login
    g_signal_connect(btn_sair, "clicked", G_CALLBACK(on_btn_sair_clicked), NULL);
    
    gtk_box_append(GTK_BOX(vbox), btn_sair);

    return vbox;
}

/**
 * @brief Função que atualiza a listbox de pacientes
 */


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
    
    g_campos_busca.entry_busca = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(g_campos_busca.entry_busca), "Buscar paciente pelo nome...");
    gtk_widget_set_hexpand(g_campos_busca.entry_busca, TRUE);
    gtk_box_append(GTK_BOX(hbox_busca), g_campos_busca.entry_busca);

    GtkWidget *btn_buscar = gtk_button_new_with_label("🔍 Buscar");
    gtk_box_append(GTK_BOX(hbox_busca), btn_buscar);
    gtk_box_append(GTK_BOX(vbox), hbox_busca);

    // --- LISTA DE PRONTUÁRIOS (Dados simulados / Mocados) ---
    GtkWidget *lbl_lista = gtk_label_new("Prontuários Recentes:");
    gtk_widget_set_halign(lbl_lista, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_lista);

    g_campos_busca.listbox = gtk_list_box_new(); // Salva na struct 
    g_signal_connect(g_campos_busca.listbox, "row-activated", G_CALLBACK(on_prontuario_row_activated), NULL);
    gtk_widget_set_vexpand(g_campos_busca.listbox, TRUE); // Faz a lista ocupar o espaço restante

    // A lista será preenchida ao clicar no botão do dashboard (on_menu_button_clicked)

    // conecta o botão ao callback
    g_signal_connect(btn_buscar, "clicked", G_CALLBACK(on_btn_buscar_clicked), &g_campos_busca);

    gtk_box_append(GTK_BOX(vbox), g_campos_busca.listbox);

    return vbox;
}
/**
 * @brief constroi a tela de visualização do prontuario do paciente selecionado.
 */
static void on_btn_novo_laudo_clicked(GtkButton *btn, gpointer user_data);

/**
 * @brief constroi a tela de visualização do prontuario do paciente selecionado.
 */
static GtkWidget* criar_tela_prontuario_view(void) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    // parte de cima da tela
    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_pron_view_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);
    
    GtkWidget *lbl_titulo = gtk_label_new("Prontuário do Paciente");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    // Podemos deixar o titulo em bold
    gtk_label_set_markup(GTK_LABEL(lbl_titulo), "<span size='large' weight='bold'>Prontuário do Paciente</span>");
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);

    // Botoes de acao rapida no topo direito
    GtkWidget *btn_editar = gtk_button_new_with_label("✏️ Editar Dados");
    g_signal_connect(btn_editar, "clicked", G_CALLBACK(on_btn_editar_pron_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_editar);

    GtkWidget *btn_excluir = gtk_button_new_with_label("🗑️ Excluir Paciente");
    g_signal_connect(btn_excluir, "clicked", G_CALLBACK(on_btn_excluir_pron_clicked), NULL);
    // Vamos pintar de vermelhinho se usassemos css, mas deixamos padrao por enquanto
    gtk_box_append(GTK_BOX(hbox_topo), btn_excluir);

    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // Frame para Dados pessoais
    GtkWidget *frame_pessoal = gtk_frame_new(" Dados Pessoais ");
    GtkWidget *vbox_pessoal = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_top(vbox_pessoal, 10);
    gtk_widget_set_margin_bottom(vbox_pessoal, 10);
    gtk_widget_set_margin_start(vbox_pessoal, 10);
    
    g_pron_view.lbl_nome = gtk_label_new("Nome: —");
    gtk_widget_set_halign(g_pron_view.lbl_nome, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox_pessoal), g_pron_view.lbl_nome);

    g_pron_view.lbl_email = gtk_label_new("E-mail: —");
    gtk_widget_set_halign(g_pron_view.lbl_email, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox_pessoal), g_pron_view.lbl_email);

    g_pron_view.lbl_cpf = gtk_label_new("CPF: —");
    gtk_widget_set_halign(g_pron_view.lbl_cpf, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox_pessoal), g_pron_view.lbl_cpf);

    g_pron_view.lbl_data_nasc = gtk_label_new("Data de Nasc.: —");
    gtk_widget_set_halign(g_pron_view.lbl_data_nasc, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox_pessoal), g_pron_view.lbl_data_nasc);

    g_pron_view.lbl_telefone = gtk_label_new("Telefone: —");
    gtk_widget_set_halign(g_pron_view.lbl_telefone, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox_pessoal), g_pron_view.lbl_telefone);

    gtk_frame_set_child(GTK_FRAME(frame_pessoal), vbox_pessoal);
    gtk_box_append(GTK_BOX(vbox), frame_pessoal);

    // Separador
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_top(separator, 10);
    gtk_widget_set_margin_bottom(separator, 10);
    gtk_box_append(GTK_BOX(vbox), separator);

    // Secao de Laudos
    GtkWidget *hbox_laudos_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget *lbl_sec2 = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(lbl_sec2), "<span size='medium' weight='bold'>Histórico Clínico (Cefalometrias)</span>");
    gtk_widget_set_hexpand(lbl_sec2, TRUE);
    gtk_widget_set_halign(lbl_sec2, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_laudos_topo), lbl_sec2);

    GtkWidget *btn_novo_laudo = gtk_button_new_with_label("➕ Novo pré-diagnóstico");
    g_signal_connect(btn_novo_laudo, "clicked", G_CALLBACK(on_btn_novo_laudo_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_laudos_topo), btn_novo_laudo);

    gtk_box_append(GTK_BOX(vbox), hbox_laudos_topo);

    // Scroll e ListBox para laudos
    GtkWidget *scrolled_laudos = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled_laudos, TRUE); // Ocupa o restante da tela
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled_laudos), 200);

    g_pron_view.listbox_laudos = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(g_pron_view.listbox_laudos), GTK_SELECTION_SINGLE);
    
    // forward declaration pro callback de clique no laudo q sera declarado a frente
    void on_laudo_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data);
    g_signal_connect(g_pron_view.listbox_laudos, "row-activated", G_CALLBACK(on_laudo_row_activated), NULL);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_laudos), g_pron_view.listbox_laudos);
    gtk_box_append(GTK_BOX(vbox), scrolled_laudos);

    return vbox;
}

/**
 * @brief Callback para o botão "Voltar" da Tela de Cadastro de Pacientes.
 */
static void on_btn_voltar_cadastro_clicked(GtkButton *btn, gpointer user_data) {
    (void)user_data;
    (void)btn;
    log_message(LOG_INFO, "[GUI] Retornando do Cadastro para a Dashboard...");
    limpar_campos_cadastro_basico();
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
}


/**
 * @brief Constrói a tela de preenchimento do Pré-Diagnóstico (Métricas Clínicas e Cefalométricas).
 * Organiza os campos da struct ClinicalRecord em uma tabela simétrica (GtkGrid).
 */
static GtkWidget* criar_tela_pre_diagnostico(PreDiagCampos *campos) {
    // Caixa principal vertical com rolagem embutida para telas menores
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(vbox, 15);
    gtk_widget_set_margin_bottom(vbox, 15);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    // --- TOPO (Botão Voltar e Título Técnico) ---
    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_pre_diag_clicked), campos);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);

    GtkWidget *lbl_titulo = gtk_label_new("Ficha de Pré-Diagnóstico Cefalométrico");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);
    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // --- CRIAÇÃO DA TABELA DE INPUTS (GtkGrid) ---
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_widget_set_margin_top(grid, 10);

    // Coluna 1: Métricas Gerais Básicas
    GtkWidget *lbl_h = gtk_label_new("Altura (m):");
    gtk_widget_set_halign(lbl_h, GTK_ALIGN_START);
    campos->entry_height = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_height), "Ex: 1.75");
    gtk_grid_attach(GTK_GRID(grid), lbl_h, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_height, 0, 1, 1, 1);

    GtkWidget *lbl_w = gtk_label_new("Peso (kg):");
    gtk_widget_set_halign(lbl_w, GTK_ALIGN_START);
    campos->entry_weight = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_weight), "Ex: 72.5");
    gtk_grid_attach(GTK_GRID(grid), lbl_w, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_weight, 0, 3, 1, 1);

    GtkWidget *lbl_age = gtk_label_new("Idade:");
    gtk_widget_set_halign(lbl_age, GTK_ALIGN_START);
    campos->entry_age = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl_age, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_age, 0, 5, 1, 1);

    GtkWidget *lbl_anb = gtk_label_new("Ángulo ANB (°):");
    gtk_widget_set_halign(lbl_anb, GTK_ALIGN_START);
    campos->entry_anb = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl_anb, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_anb, 0, 7, 1, 1);

    GtkWidget *lbl_coa = gtk_label_new("Comprimento COA (mm):");
    gtk_widget_set_halign(lbl_coa, GTK_ALIGN_START);
    campos->entry_coa = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl_coa, 0, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_coa, 0, 9, 1, 1);

    GtkWidget *lbl_cogn = gtk_label_new("Comprimento CO-GN (mm):");
    gtk_widget_set_halign(lbl_cogn, GTK_ALIGN_START);
    campos->entry_cogn = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), lbl_cogn, 0, 10, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_cogn, 0, 11, 1, 1);

    // Coluna 2: Métricas Cefalometria Avançadas
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

    // Linha de Largura Total Completa inferior (Perfil Tegumentar)
    GtkWidget *lbl_perf = gtk_label_new("Perfil Tegumentar:");
    gtk_widget_set_halign(lbl_perf, GTK_ALIGN_START);
    campos->entry_perf_tegu = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_perf_tegu), "Ex: Perfil Convexo / Reto / Côncavo");
    gtk_grid_attach(GTK_GRID(grid), lbl_perf, 0, 12, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), campos->entry_perf_tegu, 0, 13, 2, 1);

    gtk_box_append(GTK_BOX(vbox), grid);

    // --- GRÁFICO INVESTIDO DO CARD DE RESULTADO ---
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

    campos->btn_diagnostico = gtk_button_new_with_label("🧠 Gerar Pré-Diagnóstico");
    gtk_widget_set_sensitive(campos->btn_diagnostico, FALSE); // Começa bloqueado
    gtk_widget_set_margin_top(campos->btn_diagnostico, 15);
    gtk_widget_set_size_request(campos->btn_diagnostico, -1, 42);
    g_signal_connect(campos->btn_diagnostico, "clicked", G_CALLBACK(on_btn_gerar_diagnostico_clicked), campos);
    gtk_box_append(GTK_BOX(vbox), campos->btn_diagnostico);

    // Liga os sinais de mudanca de texto
    g_signal_connect(campos->entry_height, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);
    g_signal_connect(campos->entry_weight, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);
    g_signal_connect(campos->entry_age, "changed", G_CALLBACK(on_pre_diag_fields_changed), campos);
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

    // Adiciona uma barra de rolagem (ScrolledWindow) para o formulário caber confortavelmente em qualquer resolução
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), vbox);
    gtk_widget_set_vexpand(scrolled_window, TRUE);

    return scrolled_window;
}

// ============================================================================
// INICIALIZAÇÃO E FLUXO PRINCIPAL DO GTK
// ============================================================================


/**
 * @brief Constrói o Passo 1 (Visual) da Tela 4 - Cadastrar Pacientes.
 * Recebe o ponteiro da struct de campos para mapear os widgets de texto de cadastro.
 */
static GtkWidget* criar_tela_cadastro_pacientes(CadastroCampos *campos) {
    // Caixa principal vertical com espaçamento de 8px entre os rótulos e campos
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    // --- TOPO (Botão Voltar e Título) ---
    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_bottom(hbox_topo, 10);
    
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_cadastro_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);

    GtkWidget *lbl_titulo = gtk_label_new("Novo Cadastro de Paciente");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);
    
    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // --- FORMULÁRIO DE CADASTRO ---
    // 1. Nome Completo
    GtkWidget *lbl_nome = gtk_label_new("Nome Completo:");
    gtk_widget_set_halign(lbl_nome, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_nome);
    campos->entry_nome = gtk_entry_new();
    gtk_box_append(GTK_BOX(vbox), campos->entry_nome);

    // 2. Email (Não Obrigatório)
    GtkWidget *lbl_email = gtk_label_new("E-mail (Opcional):");
    gtk_widget_set_halign(lbl_email, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_email);
    campos->entry_email = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_email), "exemplo@email.com");
    gtk_box_append(GTK_BOX(vbox), campos->entry_email);

    // 3. CPF
    GtkWidget *lbl_cpf = gtk_label_new("CPF:");
    gtk_widget_set_halign(lbl_cpf, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_cpf);
    campos->entry_cpf = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_cpf), "000.000.000-00");
    gtk_box_append(GTK_BOX(vbox), campos->entry_cpf);

    // 4. Data de Nascimento
    GtkWidget *lbl_data_nasc = gtk_label_new("Data de Nascimento:");
    gtk_widget_set_halign(lbl_data_nasc, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_data_nasc);
    campos->entry_data_nasc = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_data_nasc), "DD/MM/AAAA");
    gtk_box_append(GTK_BOX(vbox), campos->entry_data_nasc);

    // 5. Telefone
    GtkWidget *lbl_telefone = gtk_label_new("Telefone / WhatsApp:");
    gtk_widget_set_halign(lbl_telefone, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_telefone);
    campos->entry_telefone = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(campos->entry_telefone), "(81) 99999-9999");
    gtk_box_append(GTK_BOX(vbox), campos->entry_telefone);

    // --- BOTÃO SALVAR ---
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
static GtkWidget* criar_tela_edicao_paciente(EdicaoPacienteCampos *campos) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    GtkWidget *hbox_topo = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_bottom(hbox_topo, 10);
    
    GtkWidget *btn_voltar = gtk_button_new_with_label("⬅️ Voltar");
    g_signal_connect(btn_voltar, "clicked", G_CALLBACK(on_btn_voltar_edicao_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_topo), btn_voltar);

    GtkWidget *lbl_titulo = gtk_label_new("Edição de Paciente");
    gtk_widget_set_hexpand(lbl_titulo, TRUE);
    gtk_widget_set_halign(lbl_titulo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);
    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // Nome Completo
    GtkWidget *lbl_nome = gtk_label_new("Nome Completo:");
    gtk_widget_set_halign(lbl_nome, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_nome);
    campos->entry_nome = gtk_entry_new();
    gtk_box_append(GTK_BOX(vbox), campos->entry_nome);

    // Email
    GtkWidget *lbl_email = gtk_label_new("E-mail:");
    gtk_widget_set_halign(lbl_email, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_email);
    campos->entry_email = gtk_entry_new();
    gtk_box_append(GTK_BOX(vbox), campos->entry_email);

    // CPF
    GtkWidget *lbl_cpf = gtk_label_new("CPF:");
    gtk_widget_set_halign(lbl_cpf, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_cpf);
    campos->entry_cpf = gtk_entry_new();
    gtk_box_append(GTK_BOX(vbox), campos->entry_cpf);

    // Data de Nascimento
    GtkWidget *lbl_data_nasc = gtk_label_new("Data de Nascimento:");
    gtk_widget_set_halign(lbl_data_nasc, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_data_nasc);
    campos->entry_data_nasc = gtk_entry_new();
    gtk_box_append(GTK_BOX(vbox), campos->entry_data_nasc);

    // Telefone
    GtkWidget *lbl_telefone = gtk_label_new("Telefone:");
    gtk_widget_set_halign(lbl_telefone, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_telefone);
    campos->entry_telefone = gtk_entry_new();
    gtk_box_append(GTK_BOX(vbox), campos->entry_telefone);

    GtkWidget *btn_salvar = gtk_button_new_with_label("💾 Salvar Edição");
    gtk_widget_set_margin_top(btn_salvar, 15);
    gtk_widget_set_size_request(btn_salvar, -1, 40);
    g_signal_connect(btn_salvar, "clicked", G_CALLBACK(on_btn_salvar_edicao_clicked), campos);
    gtk_box_append(GTK_BOX(vbox), btn_salvar);

    return vbox;
}


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
    
    // Configura uma animação de transição suave (deslizar para os lados) ao mudar de tela
    gtk_stack_set_transition_type(GTK_STACK(g_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(g_stack), 300); // 300 milissegundos de efeito

    // 3. Construção das telas independentes passando as referências de memória das structs de captura
    GtkWidget *layout_login = criar_tela_login(&campos_login);
    GtkWidget *layout_cad_dentista = criar_tela_cadastro_dentista(&g_campos_cad_dentista);
    GtkWidget *layout_dashboard = criar_tela_dashboard();
    GtkWidget *layout_prontuarios = criar_tela_prontuarios();
    GtkWidget *layout_pron_view = criar_tela_prontuario_view();
    GtkWidget *layout_cadastro = criar_tela_cadastro_pacientes(&g_campos_cadastro); 
    GtkWidget *layout_edicao = criar_tela_edicao_paciente(&g_campos_edicao_pac);
    GtkWidget *layout_pre_diag = criar_tela_pre_diagnostico(&campos_pre_diag); // <-- NOVA TELA INSTANCIADA
    GtkWidget *layout_laudo_view = criar_tela_laudo_view(); // Nova tela de view do laudo

    // 4. Adicionando as telas dentro do Stack e dando um "nome" de identificação para cada uma
    gtk_stack_add_named(GTK_STACK(g_stack), layout_login, "login_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_cad_dentista, "cadastro_dentista_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_dashboard, "dashboard_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_prontuarios, "prontuarios_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_pron_view, "prontuario_view_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_laudo_view, "laudo_view_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_cadastro, "cadastro_page"); 
    gtk_stack_add_named(GTK_STACK(g_stack), layout_edicao, "edicao_paciente_page"); 
    gtk_stack_add_named(GTK_STACK(g_stack), layout_pre_diag, "pre_diagnostico_page"); 

    // 5. Define qual página o Stack deve exibir assim que o programa abrir
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "login_page");

    // 6. Define o GtkStack como o filho principal (conteúdo) da nossa janela
    gtk_window_set_child(GTK_WINDOW(window), g_stack);

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