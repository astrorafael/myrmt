

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************************************* */
/*                         INCLUDE HEADER SECTION                            */
/* ************************************************************************* */

// -----------------------------------
// Expressif SDK-IDF standard includes
// -----------------------------------

#include "driver/gpio.h"
#include "driver/rmt.h"

/* ************************************************************************* */
/*                      DEFINES AND ENUMERATIONS SECTION                     */
/* ************************************************************************* */

#define GPIO_5   5
#define GPIO_18 18
#define GPIO_19 19
#define GPIO_21 21


/* ************************************************************************* */
/*                               DATATYPES SECTION                           */
/* ************************************************************************* */

typedef struct {
    double        freq;       // real frequency after adjustment (Hz)
    double        duty_cycle; // duty cycle after adjustments (0 < x < 1)
    size_t        onitems;    // original items sequence length without duplication nor  EoTx
    size_t        nitems;     // number of RMT items including duplication and EoTx mark
    uint8_t       nrep;       // how many times the items sequence is being repeated (1 < nrep)
    uint8_t       mem_blocks; // number of memory blocks consumed (1 block = 64 RMT items)
    uint8_t       prescaler;  // RMT prescaler value
    uint32_t      N;          // Big divisor to decompose in items (internal value)
    uint32_t      NH;         // The high level part of N (N = NH + NL)
    uint32_t      NL;         // The low level part of N  (N = NH + NL)
} fgen_info_t;


typedef struct {
    rmt_item32_t* items;      // Array of RMT items including EoTx
    gpio_num_t    gpio_num;   // Allocated GPIO pin for this frequency generator
    rmt_channel_t channel;    // Allocated RMT channel
    fgen_info_t   info;       // detailed info about the frequency generator
    bool          started;    // state flag (true if started)
} fgen_resources_t;



/* ************************************************************************* */
/*                          GLOBAL VARIABLES SECTION                         */
/* ************************************************************************* */



/* ************************************************************************* */
/*                               API FUNCTIONS                               */
/* ************************************************************************* */

esp_err_t fgen_info(double freq, double duty_cycle, fgen_info_t* info);

fgen_resources_t* fgen_alloc(const fgen_info_t* info, gpio_num_t gpio_num);

void fgen_free(fgen_resources_t* res);

esp_err_t fgen_start(fgen_resources_t* res);

esp_err_t fgen_stop(fgen_resources_t* res);




#ifdef __cplusplus
}
#endif


