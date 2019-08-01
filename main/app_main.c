/* RMT transmit example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/



/* ************************************************************************* */
/*                         INCLUDE HEADER SECTION                            */
/* ************************************************************************* */

// -------------------
// C standard includes
// -------------------

// -----------------------------------
// Expressif SDK-IDF standard includes
// -----------------------------------

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// --------------
// Local includes
// --------------

#include "freq_console.h"


/* ************************************************************************* */
/*                      DEFINES AND ENUMERATIONS SECTION                     */
/* ************************************************************************* */

#define MAIN_TAG "main"

/* ************************************************************************* */
/*                               DATATYPES SECTION                           */
/* ************************************************************************* */



/* ************************************************************************* */
/*                          GLOBAL VARIABLES SECTION                         */
/* ************************************************************************* */


/* ************************************************************************* */
/*                        AUXILIAR FUNCTIONS SECTION                         */
/* ************************************************************************* */


/* ************************************************************************* */
/*                             MAIN ENTRY POINT                              */
/* ************************************************************************* */

void app_main(void *ignore)
{
    ESP_LOGD(MAIN_TAG, "Starting interactive program");
    freq_console_init();
    freq_console_loop();
    vTaskDelete(NULL);
}
