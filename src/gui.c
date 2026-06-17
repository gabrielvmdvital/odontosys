#include "gui.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include "logs.h"

// ============================================================================
// VARIÁVEIS GLOBAIS ESTÁTICAS (Escopo do Arquivo)
// ============================================================================
static AppState *g_app_state = NULL;       // Guarda o estado do core do programa
static GtkApplication *g_gtk_app = NULL;   // Controle do ciclo de vida do GTK
static GtkWidget *g_stack = NULL;          // O GtkStack global que controlará a troca de telas

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
struct CadastroCampos {
    GtkWidget *entry_nome;       // Ponteiro para o campo de texto do nome
    GtkWidget *entry_email;      // Ponteiro para o campo de texto do email (opcional)
    GtkWidget *entry_cpf;        // Ponteiro para o campo de texto do CPF
    GtkWidget *entry_data_nasc;  // Ponteiro para o campo de texto da data de nascimento
    GtkWidget *entry_telefone;   // Ponteiro para o campo de texto do telefone
};

// Adicionando o typedef da struct para o compilador reconhecer o tipo sem a palavra 'struct':
typedef struct CadastroCampos CadastroCampos;

// AGORA SIM, declare a variável global estática:
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
} PreDiagCampos;

/**
 * @brief Estrutura que agrupa os componentes do prontuário, agora como label, que diferente do entry, são só de exibição.
 */
typedef struct {
    GtkWidget *lbl_nome;
    GtkWidget *lbl_email;
    GtkWidget *lbl_cpf;
    GtkWidget *lbl_data_nasc;
    GtkWidget *lbl_telefone;
    GtkWidget *lbl_height;
    GtkWidget *lbl_weight;
    GtkWidget *lbl_age;
    GtkWidget *lbl_anb;
    GtkWidget *lbl_perfil;
    GtkWidget *lbl_cogn;
    GtkWidget *lbl_coa;
    GtkWidget *lbl_afai;
    GtkWidget *lbl_sngogn;
    GtkWidget *lbl_na1_dist;
    GtkWidget *lbl_na1_ang;
    GtkWidget *lbl_na2_dist;
    GtkWidget *lbl_na2_ang;
    GtkWidget *lbl_max_tipo;
    GtkWidget *lbl_max_desvio;
} ProntuarioViewCampos;
static ProntuarioViewCampos g_pron_view; // labels da tela de prontuario que serão preenchidas no criar_tela_prontuario_view (futuro)

// ============================================================================
// FUNÇÕES DE CALLBACK (EVENTOS)
// ============================================================================

/**
 * @brief Callback disparado quando o botão "Entrar" é clicado.
 * Esta função captura os textos e faz a transição de tela usando o GtkStack.
 */
static void on_btn_entrar_clicked(GtkButton *btn, gpointer user_data) {
    // 1. Recuperamos a struct com os campos de texto através do ponteiro genérico
    LoginCampos *campos = (LoginCampos *)user_data;

    // 2. Extraímos as strings que o usuário digitou
    const char *usuario = gtk_editable_get_text(GTK_EDITABLE(campos->entry_usuario));
    const char *senha = gtk_editable_get_text(GTK_EDITABLE(campos->entry_senha));

    // 3. Print de debug no terminal (útil para testes acadêmicos)
    log_message(LOG_INFO, "[GUI] Botão Entrar Clicado.");
    log_message(LOG_INFO, "[GUI] Usuário inserido: %s", usuario);
    
    // 4. TRANSIÇÃO DE TELA (O Segredo do GtkStack):
    // Como o 'g_stack' é global, nós dizemos para ele mudar a página visível 
    // para a página que batizamos de "dashboard_page".
    // Sem validação por enquanto, conforme o escopo do Trello.
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
}

/**
 * @brief Callback disparado quando o botão "Sair" da Dashboard é clicado.
 * Faz o GtkStack reverter a animação de volta para a tela de login.
 */
static void on_btn_sair_clicked(GtkButton *btn, gpointer user_data) {
    log_message(LOG_INFO, "[GUI] Usuário deslogou da sessão. Retornando ao Login...");
    
    // Faz o Stack exibir a tela de login novamente com animação reversa automática
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "login_page");
}

/**
 * @brief Callback temporário para os botões do menu interno.
 */
static void on_menu_button_clicked(GtkButton *btn, gpointer user_data) {
    const char *label = gtk_button_get_label(btn);
    log_message(LOG_INFO, "[GUI] Botão do menu clicado: %s", label);

    if (g_str_has_prefix(label, "📁")) {
        gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuarios_page");
    }
    // Se clicar no botão de Pacientes (👥), abre o cadastro
    else if (g_str_has_prefix(label, "👥")) {
        gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "cadastro_page");
    }
}

/**
 * @brief Callback para o botão "Voltar" da Tela de Prontuários.
 * Faz o GtkStack deslizar de volta para a Dashboard.
 */
static void on_btn_voltar_prontuarios_clicked(GtkButton *btn, gpointer user_data) {
    log_message(LOG_INFO, "[GUI] Retornando dos Prontuários para a Dashboard...");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "dashboard_page");
}
/**
 * @brief Lista onde vai ser procurado o paciente buscado.
 * Armazena os pacientes fictícios.
 */
// lista que recebe os endereços dos pacientes ficticios (precisa ser mudado dps para receber todos cadastrados)
static const char *pacientes_mock[] = {
    "  📝 João Silva (Invisalign - Manutenção)",
    "  📝 Maria Souza (Canal Pendor)",
    "  📝 Carlos Eduardo (Avaliação Inicial)",
    NULL // fim da liksta 
};
/**
 * @brief Callback do botão de buscar.
 */
static void on_btn_buscar_clicked(GtkButton *btn, gpointer user_data) { 
    //Muda o user_data pata BuscaCampos agr levando ao ponteiro da busca
    BuscaCampos *busca = (BuscaCampos *)user_data;
    const char *texto = gtk_editable_get_text(GTK_EDITABLE(busca->entry_busca));

    printf("[GUI] Buscando por: %s\n", texto);

    // Limpa os itens anteriores do listbox
    GtkWidget *paciente;
    while ((paciente = gtk_widget_get_first_child(busca->listbox)) != NULL) {
        gtk_list_box_remove(GTK_LIST_BOX(busca->listbox), paciente);
    }

    // Aqui usamos o for para testar se o digitado(busca) faz parte da listbox e tratamos o texto para que aceite maiusculas e minusculas.
    gchar *busca_lower = g_utf8_strdown(texto, -1);
    int encontrou = 0;

    for (int i = 0; pacientes_mock[i] != NULL; i++) {
        gchar *item_lower = g_utf8_strdown(pacientes_mock[i], -1);
        if (g_strstr_len(item_lower, -1, busca_lower) != NULL) {
            gtk_list_box_append(GTK_LIST_BOX(busca->listbox),
                                gtk_label_new(pacientes_mock[i]));
            encontrou++;
        }
        g_free(item_lower); //Libera a memória
    }

    // Caso não tiver ninguém com o texto digitado
    if (encontrou == 0) {
        gtk_list_box_append(GTK_LIST_BOX(busca->listbox),
                            gtk_label_new("  ❌ Nenhum paciente encontrado."));
    }

    g_free(busca_lower); 
}

/**
 * @brief Callback disparado quando o botão "Salvar Ficha" da tela de Cadastro é clicado.
 * Captura os dados textuais inseridos na interface visual e redireciona para a tela de pré-diagnóstico.
 */
static void on_btn_salvar_cadastro_clicked(GtkButton *btn, gpointer user_data) {
    // 1. Recuperamos a struct com os campos de entrada através do ponteiro genérico
    CadastroCampos *campos = (CadastroCampos *)user_data;

    // 2. Extraímos as strings contidas em cada GtkEntry mapeado
    const char *nome = gtk_editable_get_text(GTK_EDITABLE(campos->entry_nome));
    const char *email = gtk_editable_get_text(GTK_EDITABLE(campos->entry_email));
    const char *cpf = gtk_editable_get_text(GTK_EDITABLE(campos->entry_cpf));
    const char *data_nasc = gtk_editable_get_text(GTK_EDITABLE(campos->entry_data_nasc));
    const char *telefone = gtk_editable_get_text(GTK_EDITABLE(campos->entry_telefone));

    // 3. Print de debug estruturado usando o logger unificado do sistema
    log_message(LOG_INFO, "[GUI] Processando novo cadastro de paciente...");
    log_message(LOG_INFO, " |- Nome: %s", nome);
    log_message(LOG_INFO, " |- Email: %s", (strlen(email) == 0) ? "(Não Informado)" : email);
    log_message(LOG_INFO, " |- CPF: %s", cpf);
    log_message(LOG_INFO, " |- Data de Nasc: %s", data_nasc);
    log_message(LOG_INFO, " |- Telefone: %s", telefone);

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
 * @brief Função auxiliar interna para limpar a tela de dados cadastrais básicos.
 */
static void limpar_campos_cadastro_basico(void) {
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_nome), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_email), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_cpf), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_data_nasc), "");
    gtk_editable_set_text(GTK_EDITABLE(g_campos_cadastro.entry_telefone), "");
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
    
    log_message(LOG_INFO, "[GUI] Fluxo encerrado pelo operador. Reiniciando interface para novo paciente...");
    
    // 1. Limpa os dados de ambas as etapas visuais para evitar vazamento de memória visual
    limpar_campos_pre_diagnostico(campos);
    limpar_campos_cadastro_basico();
    
    // 2. Restaura a semântica e funcionalidade do botão original do formulário
    gtk_button_set_label(btn, "🧠 Gerar Pré-Diagnóstico");
    g_signal_handlers_disconnect_by_func(btn, G_CALLBACK(on_btn_concluir_fluxo_clicked), campos);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_btn_gerar_diagnostico_clicked), campos);
    
    // 3. Joga o dentista de volta na primeira tela de dados cadastrais (Nome, CPF, fone)
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "cadastro_page");
}

/**
 * @brief Callback para o botão "Voltar" da Ficha de Pré-Diagnóstico.
 * Retorna o usuário para a tela de cadastro básico do paciente.
 */
static void on_btn_voltar_pre_diag_clicked(GtkButton *btn, gpointer user_data) {
    log_message(LOG_INFO, "[GUI] Retornando do Pré-Diagnóstico para a tela de Cadastro...");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "cadastro_page");
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

    // 2. Impressão estruturada de diagnóstico nos logs do terminal
    log_message(LOG_INFO, "[GUI] Coletando métricas estruturadas de ClinicalRecord para processamento:");
    log_message(LOG_INFO, " |- Métricas Básicas: Altura: %sm | Peso: %skg | Idade: %s anos",
                gtk_editable_get_text(GTK_EDITABLE(campos->entry_height)),
                gtk_editable_get_text(GTK_EDITABLE(campos->entry_weight)),
                gtk_editable_get_text(GTK_EDITABLE(campos->entry_age)));
    log_message(LOG_INFO, " |- Cefalometria: ANB: %s° | COA: %smm | CO-GN: %smm",
                gtk_editable_get_text(GTK_EDITABLE(campos->entry_anb)),
                gtk_editable_get_text(GTK_EDITABLE(campos->entry_coa)),
                gtk_editable_get_text(GTK_EDITABLE(campos->entry_cogn)));
    log_message(LOG_INFO, " |- Maxila: Tipo Codificado: %d | Desvio Mapeado: %smm", valor_maxila_tipo, gtk_editable_get_text(GTK_EDITABLE(campos->entry_max_desvio)));
    log_message(LOG_INFO, " |- Estrutura Facial: AFAI: %smm | SN.GO.GN: %s°", gtk_editable_get_text(GTK_EDITABLE(campos->entry_afai)), gtk_editable_get_text(GTK_EDITABLE(campos->entry_sngogn)));
    log_message(LOG_INFO, " |- Incisivos Superiores: NA_Dist: %smm | NA_Ang: %s°", gtk_editable_get_text(GTK_EDITABLE(campos->entry_na1_dist)), gtk_editable_get_text(GTK_EDITABLE(campos->entry_na1_ang)));
    log_message(LOG_INFO, " |- Incisivos Inferiores: NB_Dist: %smm | NB_Ang: %s°", gtk_editable_get_text(GTK_EDITABLE(campos->entry_na2_dist)), gtk_editable_get_text(GTK_EDITABLE(campos->entry_na2_ang)));
    log_message(LOG_INFO, " |- Tecido Mole: Perfil Tegumentar: %s", gtk_editable_get_text(GTK_EDITABLE(campos->entry_perf_tegu)));
    log_message(LOG_INFO, "[GUI] Registro clínico consolidado com sucesso!");

    // --- GRÁFICO DO CARD DE RESULTADO RENDERIZADO COM ESTILO PANGO ---
    gtk_label_set_markup(GTK_LABEL(campos->lbl_resultado), 
        "<span size='large' weight='bold' foreground='#1b5e20'>Classe II Esquelética</span>\n"
        "<span size='small' foreground='#424242'>Região Anatômica Afetada: Protrução Maxilar Aplicada.</span>\n\n"
        "<i>Ficha consolidada com sucesso e pronta para persistência.</i>");

    // Torna visível a estrutura gráfica do card na tela
    gtk_widget_set_visible(campos->frame_resultado, TRUE);

    // Configura o botão para avançar em loop de cadastro
    gtk_button_set_label(btn, "✨ Concluir e Novo Cadastro");

    // Desconecta e chaveia o fluxo para a função de reset e loop infinito
    g_signal_handlers_disconnect_by_func(btn, G_CALLBACK(on_btn_gerar_diagnostico_clicked), campos);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_btn_concluir_fluxo_clicked), campos);
}
/**
 * @brief Callback para quando é clicado em um paciente da lista de prontuários.
 * Por enquanto, apenas navega para a tela de visualização — os labels mantêm o
 * placeholder "—" até a integração real com o backend/banco de dados.
 */
static void on_prontuario_row_activated(GtkListBox *listbox, GtkListBoxRow *row, gpointer user_data) {
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuario_view_page");
}
/**
 * @brief callback do botão voltar da tela de prontuarios 
 */
static void on_btn_voltar_pron_view_clicked(GtkButton *btn, gpointer user_data) {
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "prontuarios_page");
}

/**
 * @brief callback pro botão editar (para o futuro)
 */
static void on_btn_editar_pron_clicked(GtkButton *btn, gpointer user_data) {
    log_message(LOG_INFO, "[GUI] Editar prontuário — funcionalidade futura.");
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "cadastro_page"); //leva para uma nova tela de cadastro, pois a edição será implementada quando o gui for integrado ao banco de dados
}

/**
 * @brief callback do botão excluir  (ainda somente visual)
 */

static void on_btn_excluir_pron_clicked(GtkButton *btn, gpointer user_data) {
    log_message(LOG_INFO, "[GUI] Excluir prontuário — funcionalidade futura.");
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
    // Criamos uma caixa vertical com 10 pixels de espaçamento entre os componentes
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER); 
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER); 

    // Componente Título
    GtkWidget *lbl_titulo = gtk_label_new("ODONTOSYS");
    gtk_widget_set_margin_bottom(lbl_titulo, 15); 
    gtk_box_append(GTK_BOX(vbox), lbl_titulo);

    // Componente Campo Usuário
    GtkWidget *lbl_usuario = gtk_label_new("Usuário:");
    gtk_widget_set_halign(lbl_usuario, GTK_ALIGN_START); 
    gtk_box_append(GTK_BOX(vbox), lbl_usuario);

    campos->entry_usuario = gtk_entry_new();
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
 * @brief Constrói e organiza a interface real da Tela 2 (Dashboard / Painel de Controle).
 * @return Retorna um GtkWidget (GtkBox) com o menu principal ajustado sem agendamentos.
 */
static GtkWidget* criar_tela_dashboard(void) {
    // 1. Criamos a caixa vertical principal com 15 pixels de espaçamento
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

    // 4. Criando o Botão "Sair" com destaque inferior
    GtkWidget *btn_sair = gtk_button_new_with_label("🚪 Sair do Sistema");
    gtk_widget_set_size_request(btn_sair, 250, 40);
    gtk_widget_set_margin_top(btn_sair, 25); 
    
    // Conecta o evento de clique para reverter para a tela de login
    g_signal_connect(btn_sair, "clicked", G_CALLBACK(on_btn_sair_clicked), NULL);
    
    gtk_box_append(GTK_BOX(vbox), btn_sair);

    return vbox;
}

/**
 * @brief Constrói o Passo 1 (Visual) da Tela 3 - Gerenciar Prontuários.
 */
static GtkWidget* criar_tela_prontuarios(void) {
    static BuscaCampos busca;
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
    
    busca.entry_busca = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(busca.entry_busca), "Buscar paciente pelo nome...");
    gtk_widget_set_hexpand(busca.entry_busca, TRUE);
    gtk_box_append(GTK_BOX(hbox_busca), busca.entry_busca);

    GtkWidget *btn_buscar = gtk_button_new_with_label("🔍 Buscar");
    gtk_box_append(GTK_BOX(hbox_busca), btn_buscar);
    gtk_box_append(GTK_BOX(vbox), hbox_busca);

    // --- LISTA DE PRONTUÁRIOS (Dados simulados / Mocados) ---
    GtkWidget *lbl_lista = gtk_label_new("Prontuários Recentes:");
    gtk_widget_set_halign(lbl_lista, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), lbl_lista);

    busca.listbox = gtk_list_box_new(); // Salva na struct 
    g_signal_connect(busca.listbox, "row-activated", G_CALLBACK(on_prontuario_row_activated), NULL);
    gtk_widget_set_vexpand(busca.listbox, TRUE); // Faz a lista ocupar o espaço restante
    //
    for (int i = 0; pacientes_mock[i] != NULL; i++) {
        gtk_list_box_append(GTK_LIST_BOX(busca.listbox), gtk_label_new(pacientes_mock[i]));
    }

    // conecta o botão ao callback
    g_signal_connect(btn_buscar, "clicked", G_CALLBACK(on_btn_buscar_clicked), &busca);

    gtk_box_append(GTK_BOX(vbox), busca.listbox);

    return vbox;
}
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
    gtk_box_append(GTK_BOX(hbox_topo), lbl_titulo);
    gtk_box_append(GTK_BOX(vbox), hbox_topo);

    // Dados pessoais não relacionados ainda com a cefalometria
    // titulo
    GtkWidget *lbl_sec1 = gtk_label_new("── Dados Pessoais ──");
    gtk_widget_set_halign(lbl_sec1, GTK_ALIGN_START);
    gtk_widget_set_margin_top(lbl_sec1, 10);
    gtk_box_append(GTK_BOX(vbox), lbl_sec1);

    // nome
    g_pron_view.lbl_nome = gtk_label_new("Nome: —");
    gtk_widget_set_halign(g_pron_view.lbl_nome, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_nome);

    //email
    g_pron_view.lbl_email = gtk_label_new("E-mail: —");
    gtk_widget_set_halign(g_pron_view.lbl_email, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_email);

    //cpf
    g_pron_view.lbl_cpf = gtk_label_new("CPF: —");
    gtk_widget_set_halign(g_pron_view.lbl_cpf, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_cpf);

    //data de nasc
    g_pron_view.lbl_data_nasc = gtk_label_new("Data de Nasc.: —");
    gtk_widget_set_halign(g_pron_view.lbl_data_nasc, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_data_nasc);

    //telefone
    g_pron_view.lbl_telefone = gtk_label_new("Telefone: —");
    gtk_widget_set_halign(g_pron_view.lbl_telefone, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_telefone);

    // dados do exame 
    GtkWidget *lbl_sec2 = gtk_label_new("── Dados da Cefalómetria──");
    gtk_widget_set_halign(lbl_sec2, GTK_ALIGN_START);
    gtk_widget_set_margin_top(lbl_sec2, 10);
    gtk_box_append(GTK_BOX(vbox), lbl_sec2);

    g_pron_view.lbl_height = gtk_label_new("Altura: —");
    gtk_widget_set_halign(g_pron_view.lbl_height, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_height);

    g_pron_view.lbl_weight = gtk_label_new("Peso: —");
    gtk_widget_set_halign(g_pron_view.lbl_weight, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_weight);

    g_pron_view.lbl_age = gtk_label_new("Idade: —");
    gtk_widget_set_halign(g_pron_view.lbl_age, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_age);

    g_pron_view.lbl_anb = gtk_label_new("Ângulo ANB: —");
    gtk_widget_set_halign(g_pron_view.lbl_anb, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_anb);

    g_pron_view.lbl_coa = gtk_label_new("COA: —");
    gtk_widget_set_halign(g_pron_view.lbl_coa, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_coa);

    g_pron_view.lbl_cogn = gtk_label_new("CO-GN: —");
    gtk_widget_set_halign(g_pron_view.lbl_cogn, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_cogn);

    g_pron_view.lbl_afai = gtk_label_new("AFAI: —");
    gtk_widget_set_halign(g_pron_view.lbl_afai, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_afai);

    g_pron_view.lbl_sngogn = gtk_label_new("SN.GO.GN: —");
    gtk_widget_set_halign(g_pron_view.lbl_sngogn, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_sngogn);

    g_pron_view.lbl_na1_dist = gtk_label_new("NA Sup. Dist: —");
    gtk_widget_set_halign(g_pron_view.lbl_na1_dist, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_na1_dist);

    g_pron_view.lbl_na1_ang = gtk_label_new("NA Sup. Âng: —");
    gtk_widget_set_halign(g_pron_view.lbl_na1_ang, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_na1_ang);

    g_pron_view.lbl_na2_dist = gtk_label_new("NB Inf. Dist: —");
    gtk_widget_set_halign(g_pron_view.lbl_na2_dist, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_na2_dist);

    g_pron_view.lbl_na2_ang = gtk_label_new("NB Inf. Âng: —");
    gtk_widget_set_halign(g_pron_view.lbl_na2_ang, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_na2_ang);

    g_pron_view.lbl_max_tipo = gtk_label_new("Tipo da Maxila: —");
    gtk_widget_set_halign(g_pron_view.lbl_max_tipo, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_max_tipo);

    g_pron_view.lbl_max_desvio = gtk_label_new("Desvio Maxila: —");
    gtk_widget_set_halign(g_pron_view.lbl_max_desvio, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_max_desvio);

    g_pron_view.lbl_perfil = gtk_label_new("Perfil Tegumentar: —");
    gtk_widget_set_halign(g_pron_view.lbl_perfil, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(vbox), g_pron_view.lbl_perfil);

    // botões de excluir e editar 
    GtkWidget *hbox_acoes = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_top(hbox_acoes, 20);

    GtkWidget *btn_editar = gtk_button_new_with_label("✏️ Editar");
    gtk_widget_set_hexpand(btn_editar, TRUE);
    g_signal_connect(btn_editar, "clicked", G_CALLBACK(on_btn_editar_pron_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_acoes), btn_editar);

    GtkWidget *btn_excluir = gtk_button_new_with_label("🗑️ Excluir");
    gtk_widget_set_hexpand(btn_excluir, TRUE);
    g_signal_connect(btn_excluir, "clicked", G_CALLBACK(on_btn_excluir_pron_clicked), NULL);
    gtk_box_append(GTK_BOX(hbox_acoes), btn_excluir);

    gtk_box_append(GTK_BOX(vbox), hbox_acoes);
    return vbox;
}

/**
 * @brief Callback para o botão "Voltar" da Tela de Cadastro de Pacientes.
 */
static void on_btn_voltar_cadastro_clicked(GtkButton *btn, gpointer user_data) {
    log_message(LOG_INFO, "[GUI] Retornando do Cadastro para a Dashboard...");
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

    GtkWidget *btn_diagnostico = gtk_button_new_with_label("🧠 Gerar Pré-Diagnóstico");
    gtk_widget_set_margin_top(btn_diagnostico, 15);
    gtk_widget_set_size_request(btn_diagnostico, -1, 42);
    g_signal_connect(btn_diagnostico, "clicked", G_CALLBACK(on_btn_gerar_diagnostico_clicked), campos);
    gtk_box_append(GTK_BOX(vbox), btn_diagnostico);

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
    GtkWidget *btn_salvar = gtk_button_new_with_label("💾 Salvar Ficha");
    gtk_widget_set_margin_top(btn_salvar, 15);
    gtk_widget_set_size_request(btn_salvar, -1, 40);
    
    // Conectando o clique do botão salvar ao callback mapeado com a struct de campos
    g_signal_connect(btn_salvar, "clicked", G_CALLBACK(on_btn_salvar_cadastro_clicked), campos);
    
    gtk_box_append(GTK_BOX(vbox), btn_salvar);

    return vbox;
}


/**
 * @brief Evento principal de ativação do GTK. Constrói a janela e a pilha de telas.
 */
static void on_app_activate(GtkApplication *app, gpointer user_data) {
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
    GtkWidget *layout_dashboard = criar_tela_dashboard();
    GtkWidget *layout_prontuarios = criar_tela_prontuarios();
    GtkWidget *layout_pron_view = criar_tela_prontuario_view();
    GtkWidget *layout_cadastro = criar_tela_cadastro_pacientes(&g_campos_cadastro); 
    GtkWidget *layout_pre_diag = criar_tela_pre_diagnostico(&campos_pre_diag); // <-- NOVA TELA INSTANCIADA

    // 4. Adicionando as telas dentro do Stack e dando um "nome" de identificação para cada uma
    gtk_stack_add_named(GTK_STACK(g_stack), layout_login, "login_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_dashboard, "dashboard_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_prontuarios, "prontuarios_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_pron_view, "prontuario_view_page");
    gtk_stack_add_named(GTK_STACK(g_stack), layout_cadastro, "cadastro_page"); 
    gtk_stack_add_named(GTK_STACK(g_stack), layout_pre_diag, "pre_diagnostico_page"); 

    // 5. Define qual página o Stack deve exibir assim que o programa abrir
    gtk_stack_set_visible_child_name(GTK_STACK(g_stack), "login_page");

    // 6. Define o GtkStack como o filho principal (conteúdo) da nossa janela
    gtk_window_set_child(GTK_WINDOW(window), g_stack);

    // 7. Renderiza a janela na tela do usuário
    gtk_window_present(GTK_WINDOW(window));
}

bool gui_init(AppState *app_state) {
    if (app_state == NULL) return false;
    g_app_state = app_state;

    g_gtk_app = gtk_application_new("com.odontosys.app", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(g_gtk_app, "activate", G_CALLBACK(on_app_activate), NULL);

    return true;
}

void gui_run(void) {
    if (g_gtk_app != NULL) {
        g_application_run(G_APPLICATION(g_gtk_app), 0, NULL);
        g_object_unref(g_gtk_app);
    }
}