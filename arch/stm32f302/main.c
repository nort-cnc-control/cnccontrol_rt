#include "config.h"

#ifdef CONFIG_LIBCORE
#include <control/moves/moves.h>
#include <control/planner/planner.h>
#include <control/control.h>
#include <output/output.h>
#include "steppers.h"
#endif

#include <shell.h>

#include "platform.h"
#include "net.h"

#include <FreeRTOS.h>
#include <task.h>

#ifdef CONFIG_LIBCORE
static void init_steppers(void)
{
    gpio_definition gd;

    steppers_definition sd = {};
    steppers_config(&sd, &gd);
    init_control(&sd, &gd);
}

void preCalculateTask(void *args)
{
    while (true)
        planner_pre_calculate();
}
#endif

void heartBeatTask(void *args)
{
    while (true)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        led_off();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        led_on();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        led_off();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        led_on();
    }
}

void networkTask(void *args)
{
    while (true)
        network_loop();
}

int main(void)
{
    hardware_setup();

#ifdef CONFIG_LIBCORE
    init_steppers();
    planner_lock();
    moves_reset();
#endif

    while (!net_ready())
    {
        net_receive();
    }


    shell_add_message("Hello", -1);

    xTaskCreate(networkTask, "network", 4096, NULL, 0, NULL);
#ifdef CONFIG_LIBCORE 
    xTaskCreate(preCalculateTask, "precalc", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
#endif
    
    xTaskCreate(heartBeatTask, "hb", configMINIMAL_STACK_SIZE, NULL, 0, NULL);

    vTaskStartScheduler();

    while (true)
        ;

    return 0;
}

void vApplicationTickHook(void)
{}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    for (;;)
        ;
}

void vApplicationMallocFailedHook(void)
{
    for (;;)
        ;
}

void vApplicationIdleHook(void)
{}

