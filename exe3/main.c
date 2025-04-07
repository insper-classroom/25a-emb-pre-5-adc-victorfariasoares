#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

#include "data.h"
QueueHandle_t xQueueData;

// não mexer! Alimenta a fila com os dados do sinal
void data_task(void *p) {
    vTaskDelay(pdMS_TO_TICKS(400));

    int data_len = sizeof(sine_wave_four_cycles) / sizeof(sine_wave_four_cycles[0]);
    for (int i = 0; i < data_len; i++) {
        xQueueSend(xQueueData, &sine_wave_four_cycles[i], 1000000);
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void process_task(void *p) {
    int data = 0;

    // Variáveis estáticas para manter o estado do filtro
    static int samples[5] = {0, 0, 0, 0, 0}; // Buffer circular para 5 amostras
    static int count = 0;        // Número de amostras já recebidas (até 5)
    static int mean = 0;          // Soma das 5 amostras atuais

    while (true) {
        if (xQueueReceive(xQueueData, &data, 100)) {
            if (count < 4) {
                count ++;
            } else{
                count = 0;
            }
            samples[count] = data;
            mean = (samples[0] + samples[1] + samples[2] + samples[3] + samples[4]) / 5;
            printf("%d\n", mean);

            // deixar esse delay!
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

int main() {
    stdio_init_all();

    xQueueData = xQueueCreate(64, sizeof(int));

    xTaskCreate(data_task, "Data task ", 4096, NULL, 1, NULL);
    xTaskCreate(process_task, "Process task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
