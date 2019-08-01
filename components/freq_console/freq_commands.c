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

#include <stdio.h>

// -----------------------------------
// Expressif SDK-IDF standard includes
// -----------------------------------

#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"

// --------------
// Local includes
// --------------

#include "freq_generator.h"

/* ************************************************************************* */
/*                      DEFINES AND ENUMERATIONS SECTION                     */
/* ************************************************************************* */

#define CMD_TAG "FGen"  // logging tag

/* ************************************************************************* */
/*                               DATATYPES SECTION                           */
/* ************************************************************************* */



/* ************************************************************************* */
/*                          GLOBAL VARIABLES SECTION                         */
/* ************************************************************************* */


fgen_resources_t* FGEN[RMT_CHANNEL_MAX] = { 0, 0, 0, 0, 0, 0, 0, 0 };


/* ************************************************************************* */
/*                        AUXILIAR FUNCTIONS SECTION                         */
/* ************************************************************************* */

static void register_fgen(fgen_resources_t* fgen)
{
    FGEN[fgen->channel] = fgen; 
}

static void unregister_fgen(fgen_resources_t* fgen)
{
    FGEN[fgen->channel] = 0; 
}

static fgen_resources_t* search_fgen(rmt_channel_t channel)
{
    for (rmt_channel_t i = 0; i<RMT_CHANNEL_MAX; i++) {
        if ( (FGEN[i] != NULL) && (FGEN[i]->channel == channel) ) {
            return FGEN[i];
        }
    }
    return NULL;
}

static const char* state_msg(fgen_resources_t* fgen)
{
    return fgen->started ? "started": "stopped" ;
}

/* ************************************************************************* */
/*                     COMMAND IMPLEMENTATION SECTION                        */
/* ************************************************************************* */

// ============================================================================
// 'params' command arguments static variable
static 
struct {
    struct arg_dbl *frequency;
    struct arg_dbl *duty_cycle;
    struct arg_end *end;
} params_args;

// forward declaration
static int exec_params(int argc, char **argv);

// 'params' command registration
static void register_params()
{
    params_args.frequency =
        arg_dbl1("f", "freq", "<frequency>", "Frequency, Hz");
    params_args.duty_cycle =
        arg_dbl0("d", "duty", "<duty cycle>",
                 "Defaults to 0.5 (50%) if not given");
    params_args.duty_cycle->dval[0] = 0.5; // Give it a default value
  
    params_args.end = arg_end(3);

    const esp_console_cmd_t cmd = {
        .command = "params",
        .help = "Computes the frequency generator parameters as well as the needed resouces. "
                "Does not create a frequency generator. ",
        .hint = NULL,
        .func = exec_params,
        .argtable = &params_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

// 'params' command implementation
static int exec_params(int argc, char **argv)
{
    fgen_info_t    info;

    int nerrors = arg_parse(argc, argv, (void **) &params_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, params_args.end, argv[0]);
        return 1;
    }

    fgen_info( params_args.frequency->dval[0], 
               params_args.duty_cycle->dval[0], 
               &info);

    printf("------------------------------------------------------------------\n");
    printf("                 FREQUENCY GENERATOR PARAMETERS                   \n");
    printf("Final Frequency:\t%0.4f Hz\n", info.freq);
    printf("Final Duty Cycle:\t%0.2f%%\n", info.duty_cycle*100);
    printf("Prescaler:\t\t%d\n", info.prescaler);
    printf("N:\t\t\t%d (%d high + %d low)\n", info.N, info.NH, info.NL);
    printf("Nitems:\t\t\t%d, repeated x%d\n", info.onitems, info.nrep);
    printf("Blocks:\t\t\t%d (64 items each)\n", info.mem_blocks);
    printf("------------------------------------------------------------------\n");
    return 0;
}


// ============================================================================

// 'create' command arguments static variable
static 
struct {
    struct arg_dbl *frequency;
    struct arg_dbl *duty_cycle;
    struct arg_int *gpio_num;
    struct arg_end *end;
} create_args;

// forward declaration
static int exec_create(int argc, char **argv);

// 'create' command registration
static void register_create()
{
    create_args.frequency =
        arg_dbl1("f", "freq", "<frequency>", "Frequency, Hz");
    create_args.duty_cycle =
        arg_dbl0("d", "duty", "<duty cycle>",
                 "Defaults to 0.5 (50%) if not given");
    create_args.duty_cycle->dval[0] = 0.5; // Give it a default value
    create_args.gpio_num =
        arg_int0("g", "gpio", "<GPIO num>",
                 "Defaults to -1 if not given");
    create_args.gpio_num->ival[0] = GPIO_NUM_NC; // Give it a default value
    create_args.end = arg_end(3);

    const esp_console_cmd_t cmd = {
        .command = "create",
        .help = "Creates a frequency generator and binds it to a GPIO pin. "
                "Returns a channel identifier (1-8)"
                "Does not start it.",
        .hint = NULL,
        .func = exec_create,
        .argtable = &create_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

// 'create' command implementation
static int exec_create(int argc, char **argv)
{
    fgen_info_t       info;
    fgen_resources_t* fgen;

    int nerrors = arg_parse(argc, argv, (void **) &create_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, create_args.end, argv[0]);
        return 1;
    }

    fgen_info( create_args.frequency->dval[0], 
               create_args.duty_cycle->dval[0], 
               &info);

    fgen = fgen_alloc(&info, create_args.gpio_num->ival[0] );
    if (fgen != NULL) {
        register_fgen(fgen); 

        printf("------------------------------------------------------------------\n");
        printf("                   FREQUENCY GENERATOR CREATED                    \n");
        printf("Channel:\t\t%d\n", fgen->channel);
        printf("GPIO:\t\t\t%d\n", fgen->gpio_num);
        printf("Frequency:\t%0.4f Hz\t\tDuty Cycle:\t%0.2f%%\n", fgen->info.freq, fgen->info.duty_cycle*100);
        printf("------------------------------------------------------------------\n");
    } else {
        printf("NO RESORECES AVAILABLE TO CREATE A NEW FREQUENCY GENERATOR\n");
    }
    return 0;
}

// ============================================================================

// 'delete' command arguments static variable
static 
struct {
    struct arg_int *channel;
    struct arg_end *end;
} delete_args;

// forward declaration
static int exec_delete(int argc, char **argv);

// 'delete' command registration
static void register_delete()
{
    delete_args.channel =
        arg_int1("c", "channel", "<channel num>",
                 "RMT channel number returned by the create command");
    delete_args.end = arg_end(3);

    const esp_console_cmd_t cmd = {
        .command = "delete",
        .help = "Deletes frequency generator and frees its GPIO pin. "
                "Must stop it first.",
        .hint = NULL,
        .func = exec_delete,
        .argtable = &delete_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

// 'delete' command implementation
static int exec_delete(int argc, char **argv)
{
    fgen_resources_t* fgen;

    int nerrors = arg_parse(argc, argv, (void **) &delete_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, delete_args.end, argv[0]);
        return 1;
    }

    fgen = search_fgen(delete_args.channel->ival[0]);
    if (fgen != NULL) {
        unregister_fgen(fgen);
        fgen_free(fgen);
    }  

    printf("------------------------------------------------------------------\n");
    printf("                   FREQUENCY GENERATOR DELETED                    \n");
    printf("Channel:\t\t%d\n", delete_args.channel->ival[0]);
    printf("------------------------------------------------------------------\n");
    return 0;
}

// ============================================================================

// 'list' command arguments static variable
static 
struct {
    struct arg_lit *extended;
    struct arg_end *end;
} list_args;

// forward declaration
static int exec_list(int argc, char **argv);

// 'list' command registration
static void register_list()
{
    list_args.extended =
        arg_lit0("x", "extended", "Extended listing");
    list_args.end = arg_end(3);

    const esp_console_cmd_t cmd = {
        .command = "list",
        .help = "List all created frequency generators.",
        .hint = NULL,
        .func = exec_list,
       
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

// 'list' command implementation
static int exec_list(int argc, char **argv)
{
    fgen_resources_t* fgen;

    int nerrors = arg_parse(argc, argv, (void **) &list_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, list_args.end, argv[0]);
        return 1;
    }

    printf("------------------------------------------------------------------\n");
    for (rmt_channel_t channel = 0; channel<RMT_CHANNEL_MAX ; channel++) {
         fgen = search_fgen(channel);
        if (fgen != NULL) {
            printf("Channel: %02d [%s]\tGPIO: %02d\tFreq.: %0.2f Hz\tBlocks: %d\n", 
                fgen->channel, state_msg(fgen), fgen->gpio_num, fgen->info.freq, fgen->info.mem_blocks);
            if (list_args.extended->count) {
                printf("\tPrescaler: %03d, N: %d (%d + %d)\n", 
                fgen->info.prescaler, fgen->info.N, fgen->info.NH, fgen->info.NL);
            }
        }
    }
    printf("------------------------------------------------------------------\n");   
    return 0;
}

// ============================================================================

// 'start' command arguments static variable
static 
struct {
    struct arg_int *channel;
    struct arg_end *end;
} start_args;

// forward declaration
static int exec_start(int argc, char **argv);

// 'start' command registration
static void register_start()
{
    start_args.channel =
        arg_int1("c", "channel", "<channel num>",
                 "RMT channel number returned by the create command");
    start_args.end = arg_end(3);

    const esp_console_cmd_t cmd = {
        .command = "start",
        .help = "Start Frequency geenrator given by channel id",
        .hint = NULL,
        .func = exec_start,
        .argtable = &start_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

// 'start' command implementation
static int exec_start(int argc, char **argv)
{
    fgen_resources_t* fgen;

    int nerrors = arg_parse(argc, argv, (void **) &start_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, start_args.end, argv[0]);
        return 1;
    }

    fgen = search_fgen(start_args.channel->ival[0]);
    if (fgen != NULL) {
       fgen_start(fgen);
    }  

    printf("------------------------------------------------------------------\n");
    printf("                   FREQUENCY GENERATOR STARTED                    \n");
    printf("Channel: %02d\tGPIO: %02d\tFreq.: %0.2f Hz\n", fgen->channel, fgen->gpio_num, fgen->info.freq);
    printf("------------------------------------------------------------------\n");
    return 0;
}

// ============================================================================

// 'stop' command arguments static variable
static 
struct {
    struct arg_int *channel;
    struct arg_end *end;
} stop_args;

// forward declaration
static int exec_stop(int argc, char **argv);

// 'stop' command registration
static void register_stop()
{
    stop_args.channel =
        arg_int1("c", "channel", "<channel num>",
                 "RMT channel number returned by the create command");
    stop_args.end = arg_end(3);

    const esp_console_cmd_t cmd = {
        .command = "start",
        .help = "Stopt Frequency generator given by channel id",
        .hint = NULL,
        .func = exec_stop,
        .argtable = &stop_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

// 'stop' command implementation
static int exec_stop(int argc, char **argv)
{
    fgen_resources_t* fgen;

    int nerrors = arg_parse(argc, argv, (void **) &stop_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, stop_args.end, argv[0]);
        return 1;
    }

    fgen = search_fgen(stop_args.channel->ival[0]);
    if (fgen != NULL) {
       fgen_stop(fgen);
    }  

    printf("------------------------------------------------------------------\n");
    printf("                   FREQUENCY GENERATOR STOPPED                    \n");
    printf("Channel: %02d\tGPIO: %02d\tFreq.: %0.2f Hz\n", fgen->channel, fgen->gpio_num, fgen->info.freq);
    printf("------------------------------------------------------------------\n");
    return 0;
}


/* ************************************************************************* */
/*                               API FUNCTIONS                               */
/* ************************************************************************* */

void freq_cmds_register()
{
    register_params();
    register_create();
    register_start();
    register_stop();
    register_delete();
    register_list();
}

