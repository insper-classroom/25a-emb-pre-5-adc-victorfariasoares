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
    static int samples[5] = {0}; // Buffer circular para 5 amostras
    static int count = 0;        // Número de amostras já recebidas (até 5)
    static int index = 0;        // Índice para atualização circular
    static int sum = 0;          // Soma das 5 amostras atuais

    while (true) {
        if (xQueueReceive(xQueueData, &data, 100)) {
        // Realiza o processamento diretamente dentro deste if
            if (count < 5) {
                // Ainda não tem 5 amostras: preenche o buffer
                samples[count] = data;
                sum += data;
                count++;
                // Quando atingir 5 amostras, calcula e imprime a média filtrada
                if (count == 5) {
                    double avg = sum / 5.0;
                    int filtered = (int)(avg + 0.5) - 127; // Arredonda a média e subtrai o DC offset
                    printf("%d\n", filtered);
                }
            } else {
                // Buffer já possui 5 amostras, atualiza o buffer circular:
                sum = sum - samples[index] + data;  // Remove a amostra mais antiga e adiciona a nova
                samples[index] = data;
                index = (index + 1) % 5;              // Atualiza o índice de forma circular

                double avg = sum / 5.0;
                int filtered = (int)(avg) - 127; // Filtra (subtraindo 127) e arredonda a média
                printf("%d\n", filtered);
                }

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
