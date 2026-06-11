#include <stdio.h>
#include <string.h>
#include "clinical.h"

void clinical_formular_diag(ClinicalRecord *record) {

// receber medida coa e corrigir se tiver desvio
int coa_corrigido;
//maxila normal:
coa_corrigido = (int)record->coa;

//maxilaprotuida:
if (record->maxila_tipo == 1)
        coa_corrigido = coa_corrigido - record->maxila_desvio;

//maxila retraida:
else if (record->maxila_tipo == -1)
        coa_corrigido = coa_corrigido + record->maxila_desvio;

}

