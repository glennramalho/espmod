#include <systemc.h>
#include "freertos/portmacro.h"
#include "espintr.h"
#include "rom/ets_sys.h"

void intr_matrix_set(int cpu_no, uint32_t model_num, uint32_t intr_num) {
   espintrptr->alloc(cpu_no, model_num, intr_num);
}

void ets_isr_unmask(unsigned int mask) {
    int cpu=xPortGetCoreID();
    espintrptr->set_mask(mask, cpu);
}

void ets_isr_mask(unsigned int mask) {
    int cpu=xPortGetCoreID();
    espintrptr->clr_mask(mask, cpu);
}
