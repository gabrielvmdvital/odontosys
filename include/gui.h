#ifndef GUI_H
#define GUI_H

#include <stdbool.h>
#include <gtk/gtk.h>
#include "app.h"
#include "patient.h"
#include "clinical.h"

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

/**
 * @brief Estrutura que contém os componentes de busca na tela de prontuários.
 * Usada para passar vários dados para o botão de busca 
 */
typedef struct {
    GtkWidget *entry_busca; // Ponteiro para o campo de texto
    GtkWidget *listbox; // Ponteiro para a lista
} BuscaCampos;

/**
 * @brief Estrutura que agrupa os componentes de texto da tela de cadastro de pacientes.
 * Usada para capturar os dados informados e enviá-los ao callback de salvamento.
 */
typedef struct CadastroCampos {
    GtkWidget *entry_nome;       // Ponteiro para o campo de texto do nome
    GtkWidget *entry_email;      // Ponteiro para o campo de texto do email (opcional)
    GtkWidget *entry_cpf;        // Ponteiro para o campo de texto do CPF
    GtkWidget *entry_data_nasc;  // Ponteiro para o campo de texto da data de nascimento
    GtkWidget *entry_telefone;   // Ponteiro para o campo de texto do telefone
    GtkWidget *btn_salvar;       // Ponteiro para o botão Salvar
} CadastroCampos;

/**
 * @brief Estrutura que agrupa os componentes de entrada para a ficha de Pré-Diagnóstico.
 * Representa os campos que mapeiam a struct ClinicalRecord do arquivo clinical.h.
 */
typedef struct {
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
    GtkWidget *btn_concluir;        // Botao de concluir o prontuario
} PreDiagCampos;

/**
 * @brief Estrutura que agrupa os componentes do prontuário, como label, que diferente do entry, são só de exibição.
 */
typedef struct {
    GtkWidget *lbl_nome;            // Exibe o nome do paciente
    GtkWidget *lbl_email;           // Exibe o email do paciente
    GtkWidget *lbl_cpf;             // Exibe o CPF do paciente
    GtkWidget *lbl_data_nasc;       // Exibe a data de nascimento do paciente
    GtkWidget *lbl_telefone;        // Exibe o telefone do paciente
    GtkWidget *listbox_laudos;      // Lista gráfica dos laudos cefalométricos gerados
} ProntuarioViewCampos;

/**
 * @brief Estrutura que agrupa os componentes da tela dedicada a visualização de um Laudo
 */
typedef struct {
    GtkWidget *lbl_data;            // Exibe a data do laudo
    GtkWidget *lbl_diag;            // Exibe o texto completo do pré-diagnóstico
    GtkWidget *lbl_anb;             // Exibe o ângulo ANB
    GtkWidget *lbl_coa;             // Exibe a medida COA
    GtkWidget *lbl_cogn;            // Exibe a medida CO-GN
    GtkWidget *lbl_afai;            // Exibe o AFAI
    GtkWidget *lbl_sngogn;          // Exibe o ângulo SN.GO.GN
    GtkWidget *lbl_na1_dist;        // Exibe a distância NA Superior
    GtkWidget *lbl_na1_ang;         // Exibe o ângulo NA Superior
    GtkWidget *lbl_na2_dist;        // Exibe a distância NB Inferior
    GtkWidget *lbl_na2_ang;         // Exibe o ângulo NB Inferior
    GtkWidget *lbl_max_tipo;        // Exibe o tipo da maxila
    GtkWidget *lbl_max_desvio;      // Exibe o desvio da maxila
    GtkWidget *lbl_perfil;          // Exibe o perfil tegumentar
} LaudoViewCampos;

/**
 * @brief Estrutura para Cadastro de Dentista
 */
typedef struct {
    GtkWidget *entry_nome;          // Campo de texto para o nome do dentista
    GtkWidget *entry_username;      // Campo de texto para o login (username)
    GtkWidget *entry_cpf;           // Campo de texto para o CPF
    GtkWidget *entry_senha;         // Campo de texto para a senha de acesso
    GtkWidget *btn_salvar;          // Botão de salvar o cadastro
} CadastroDentistaCampos;

/**
 * @brief Estrutura para Edicao de Paciente
 */
typedef struct {
    GtkWidget *entry_nome;          // Campo de edição do nome
    GtkWidget *entry_email;         // Campo de edição do email
    GtkWidget *entry_cpf;           // Campo de edição do CPF
    GtkWidget *entry_data_nasc;     // Campo de edição da data de nascimento
    GtkWidget *entry_telefone;      // Campo de edição do telefone
} EdicaoPacienteCampos;

// ============================================================================
// VARIÁVEIS GLOBAIS DE ESTADO DO GUI
// ============================================================================
/** @brief Estado global da aplicacao */
extern AppState *g_app_state;
/** @brief Referencia global para a aplicacao GTK */
extern GtkApplication *g_gtk_app;
/** @brief Gerenciador de empilhamento de telas (GtkStack) */
extern GtkWidget *g_stack;
/** @brief Referencia do botao de cadastrar dentista na interface */
extern GtkWidget *g_btn_cadastrar_dentista;

/** @brief Armazena o ID do dentista logado no sistema */
extern uint64_t g_logged_dentist_id;
/** @brief Armazena o ID do paciente atualmente selecionado */
extern uint64_t g_selected_patient_id;
/** @brief Armazena a estrutura do paciente em memoria temporaria */
extern Patient g_current_patient;
/** @brief Armazena a estrutura do prontuario em edicao/visualizacao */
extern ClinicalRecord g_current_clinical_record;
/** @brief Indica se o laudo esta sendo criado isoladamente */
extern gboolean g_is_standalone_laudo;

/** @brief Variavel global para os campos de busca */
extern BuscaCampos g_campos_busca;
/** @brief Variavel global para os campos de cadastro */
extern CadastroCampos g_campos_cadastro;
/** @brief Variavel global para a visualizacao de prontuarios */
extern ProntuarioViewCampos g_pron_view;
/** @brief Variavel global para a visualizacao de laudos */
extern LaudoViewCampos g_laudo_view;
/** @brief Variavel global para os campos de cadastro de dentistas */
extern CadastroDentistaCampos g_campos_cad_dentista;
/** @brief Variavel global para os campos de edicao de paciente */
extern EdicaoPacienteCampos g_campos_edicao_pac;

// ============================================================================
// CONSTRUTORES DE TELA
// ============================================================================

/** @brief Constroi a tela de login. */
GtkWidget* criar_tela_login(LoginCampos *campos);
/** @brief Constroi a tela de cadastro de dentista. */
GtkWidget* criar_tela_cadastro_dentista(CadastroDentistaCampos *campos);
/** @brief Constroi a tela inicial (dashboard). */
GtkWidget* criar_tela_dashboard(void);
/** @brief Constroi a tela de listagem de prontuarios. */
GtkWidget* criar_tela_prontuarios(void);
/** @brief Constroi a tela de visualizacao de dados de um paciente. */
GtkWidget* criar_tela_prontuario_view(void);
/** @brief Constroi a tela de cadastro de novos pacientes. */
GtkWidget* criar_tela_cadastro_pacientes(CadastroCampos *campos);
/** @brief Constroi a tela de edicao de dados de pacientes. */
GtkWidget* criar_tela_edicao_paciente(EdicaoPacienteCampos *campos);
/** @brief Constroi a tela da ficha de pre-diagnostico. */
GtkWidget* criar_tela_pre_diagnostico(PreDiagCampos *campos);
/** @brief Constroi a tela de visualizacao isolada de um laudo. */
GtkWidget* criar_tela_laudo_view(void);

// ============================================================================
// FUNÇÕES DE UTILIDADE E CONTROLE DE TELAS
// ============================================================================

/**
 * @brief Inicializa a interface grafica GTK e salva o estado da aplicacao.
 */
bool gui_init(AppState *app_state);
/**
 * @brief Executa o laco principal da interface grafica GTK.
 */
void gui_run(void);

/**
 * @brief Carrega os dados de um paciente e atualiza a interface de visualizacao.
 */
void carregar_tela_prontuario_por_id(uint64_t patient_id);
/**
 * @brief Atualiza a listagem de pacientes baseada no filtro fornecido.
 */
void atualizar_lista_prontuarios(const char *filtro_nome);
/**
 * @brief Limpa os campos de entrada do formulario de cadastro basico.
 */
void limpar_campos_cadastro_basico(void);
/**
 * @brief Remove caracteres nao numericos de uma string de CPF.
 */
void clean_cpf(const char *input, char *output);

#endif // GUI_H
