#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

void twai_init(void);
void msg_2_can(uint8_t *MM_MSG);
//esp_err_t test_app(void);
// void FTC533_cycle(void);
// void FTC533_process(void);
#ifdef __cplusplus
}
#endif
