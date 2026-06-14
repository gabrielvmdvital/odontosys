#include <stdio.h>
#include <string.h>
#include "clinical.h"

// REGRAS DE CLASSIFICAÇÃO ORTODÔNTICA
// 1) ANB (Classe esquelética): 1-4 = Classe I | >4 = Classe II | <1 = Classe III
// 2) CoA + CoGn: Comparação com tabela de McNamara -> Mandíbula reduzida / normal / aumentada
// 3) AFAI (Padrão crescimento facial): aumentada = vertical | normal = equilibrado | diminuída = horizontal
// 4) SNGoGn (Padrão crescimento): aumentado = vertical | normal = equilibrado | diminuído = horizontal (!! Verificar como utilizar esse !!)
// 5) 1-NA (Posição incisivo sup): 3-5 = normal | >5 = protruído | <3 = retruído
// 6) 1.NA (Inclinação incisivo sup): 24-25 = normal | >25 = inclinado | <23 = verticalizado
// 7) 1-NB (Posição incisivo inf): 3-5 = normal | >5 = protruído | <3 = retruído
// 8) 1.NB (Inclinação incisivo inf): 24-26 = normal | >26 = inclinado | <24 = verticalizado
// 9) Perf_tegument (Perfil facial): reto | convexo | concavo | normal

/* Exemplo de Pré-Diagnóstico:
        Paciente Classe I esquelética, com mandíbula reduzida, padrão de crescimento facial vertical,
        incisivos superiores retruídos e verticalizados, incisivos inferiores protruídos e inclinados,
        e perfil facial côncavo.
*/

void clinical_formular_diag(ClinicalRecord *record) {

// 1) Classe esquelética (variável utilizada: anb)
        char str_classe[15];
        if (1 <= record->anb && record->anb <= 4)   // OBS.: Usar 'record->anb' é o mesmo que '(*record).anb'
                strcpy(str_classe, "Classe I");

        else if (record->anb > 4)
                strcpy(str_classe, "Classe II");

        else
                strcpy(str_classe, "Classe III");

// 2) Tamanho da Mandíbula (coa, maxila_tipo, maxila_desvio + cogn + Tabela de McNamara)
        char str_tam_mand[20];
        // Receber Coa e corrigir proporcinalmente ao desvio da mandibula
        int coa_corrigido;

        // Maxila bem posicionada/normal (Desvio 0)
        coa_corrigido = (int)record->coa;

        // Maxila Protuida (Diminuir os mm extra):
        if (record->maxila_tipo == 1)
                coa_corrigido = coa_corrigido - record->maxila_desvio;

        // Maxila Retruida (Acrescentar os mm extra):
        else if (record->maxila_tipo == -1)
                coa_corrigido = coa_corrigido + record->maxila_desvio;

        // Tabela de McNamara
        int mcNam[29][5] = {
//               0          1    2          3   4
                {80,        97,  100,       57, 58},    // Coluna de id 0: Comprimento da maxila(mm) -> baseado no 'coa_corrigido'
                {81,        99,  102,       57, 58},    // Coluna de id 1: Comprimento mandibular(mm) [limite inferior]
                {82,        101, 104,       58, 59},    // Coluna de id 2: Comprimento mandibular(mm) [limite superior]
                {83,        103, 106,       58, 59},    // Coluna de id 3: Alt Facial inferior(mm) [limite inferior]
                {84,        104, 107,       59, 60},    // Coluna de id 4: Alt Facial inferior(mm) [limite superior]
                {85,        105, 108,       60, 62},
                {86,        107, 110,       60, 62},
                {87,        109, 112,       61, 63},
                {88,        111, 114,       61, 63},
                {89,        112, 115,       62, 64},
                {90,        113, 116,       63, 64},
                {91,        115, 118,       63, 64},
                {92,        117, 120,       64, 65},
                {93,        119, 122,       65, 66},
                {94,        121, 124,       66, 67},
                {95,        122, 125,       67, 69},
                {96,        124, 127,       67, 69},
                {97,        126, 129,       68, 70},
                {98,        128, 131,       68, 70},
                {99,        129, 132,       69, 71},
                {100,       130, 133,       70, 74},
                {101,       132, 135,       71, 75},
                {102,       134, 137,       72, 76},
                {103,       136, 139,       73, 77},
                {104,       137, 140,       74, 78},
                {105,       138, 141,       75, 79},
                {106,       139, 142,       76, 80},
                {107,       140, 143,       77, 81},
                {108,       141, 144,       78, 82}
        };

        // Iterar por cada linha para comparar os valores
        for (int i = 0; i<29; i++) {
                if (coa_corrigido == mcNam[i][0]) {
                        // Mandíbula Normal
                        if (mcNam[i][1] <= record->cogn && record->cogn <= mcNam[i][2]) // cogn do paciente entre os valores ideais
                                strcpy(str_tam_mand, "normal");

                        // Mandíbula Aumentada
                        else if (mcNam[i][2] < record->cogn) // cogn do paciente acima dos valores ideais
                                strcpy(str_tam_mand, "aumentada");
                        
                        // Mandíbula Reduzida
                        else if (record->cogn < mcNam[i][1]) // cogn do paciente abaixo dos valores ideais
                                strcpy(str_tam_mand, "reduzida");
                }
        }

// 3) Padrão de Crescimento Facial (afai, coa_corrigido + Tabela de McNamamra)
        char str_cresc_fac[20];

        // Repetir a busca pela tabela de McNamara
        for (int i = 0; i<29; i++) {
                if (coa_corrigido == mcNam[i][0]) {
                        // Crescimento Facial Equilibrado
                        if (mcNam[i][3] <= record->afai && record->afai <= mcNam[i][4]) // afai do paciente entre os valores ideais
                                strcpy(str_cresc_fac, "equilibrado");

                        // Crescimento Facial Vertical
                        else if (mcNam[i][4] < record->afai) // afai do paciente acima dos valores ideais
                                strcpy(str_cresc_fac, "vertical");
                        
                        // Crescimento Facial Horizontal
                        else if (record->afai < mcNam[i][3]) // afai do paciente abaixo dos valores ideais
                                strcpy(str_cresc_fac, "horizontal");
                }
        }


// 4) Padrão crescimento (sngocn) | OBS: Possivelmente será um só com o item 3)


// 5) Posição incisivo sup (variável: na1_dist)
        char str_pos_incsup[20];
        if (3 <= record->na1_dist <= 5)
                strcpy(str_pos_incsup, "normal");

        else if (record->na1_dist > 5)
                strcpy(str_pos_incsup, "protruido");

        else 
                strcpy(str_pos_incsup, "retruido");

// 6) Inclinação incisivo sup (na1_ang)
        char str_inc_incsup[20];


// 7) Posição incisivo inf (na2_dist)
        char str_pos_incinf[20];


// 8) Inclinação incisivo inf (na2_ang)
        char str_inc_incinf[20];


// 9) Perfil facial (perf_tegument)
        char str_perf_fac[20];




// No fim, juntar tudo em uma string para copiar em pre_diagnosis.
        sprintf(record->pre_diagnosis, "Paciente %s, ...", str_classe);

}

