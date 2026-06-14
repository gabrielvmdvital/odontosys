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
        if (1 <= record->anb <= 4)       // OBS.: Usar 'record->anb' é o mesmo que '(*record).anb'
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
        
        
        
        


// 3) Padrão de Crescimento Facial (afai)
        char str_cresc_fac[20];


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



// 7) Posição incisivo inf (na2_dist)



// 8) Inclinação incisivo inf (na2_ang)



// 9) Perfil facial (perf_tegument)




// No fim, juntar tudo em uma string para copiar em pre_diagnosis.
        sprintf(record->pre_diagnosis, "Paciente %s, ...", str_classe);

}

