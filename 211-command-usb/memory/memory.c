// memory.c

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "command.h"
#include "pico/stdlib.h"
#include "hardware/regs/addressmap.h"
#include "device.h"
int main(void);

uint32_t data_variable = 100;
uint32_t bss_variable;

// Символы определены в скрипте компоновщика.
// Используем только их адреса, значения по этим адресам не читаем.
extern char __flash_binary_start;
extern char __flash_binary_end;

extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;

extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;

extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name,
           (unsigned)start,
           (unsigned)end,
           (unsigned)(end - start));
}

void mem_info(void)
{
    // Физические области памяти RP2040.
    const uintptr_t flash_start = XIP_BASE;
    const uintptr_t flash_end = flash_start + PICO_FLASH_SIZE_BYTES;

    const uintptr_t sram_start = SRAM_BASE;
    const uintptr_t sram_end = sram_start + 264u * 1024u;

    const uintptr_t rom_start = ROM_BASE;
    const uintptr_t rom_end = rom_start + 16u * 1024u;

    // Границы областей программы.
    const uintptr_t image_start = (uintptr_t)&__flash_binary_start;
    const uintptr_t image_end = (uintptr_t)&__flash_binary_end;

    const uintptr_t boot2_start = (uintptr_t)&__boot2_start__;
    const uintptr_t boot2_end = (uintptr_t)&__boot2_end__;
    const uintptr_t text_end = (uintptr_t)&__etext;

    const uintptr_t data_start = (uintptr_t)&__data_start__;
    const uintptr_t data_end = (uintptr_t)&__data_end__;

    const uintptr_t bss_start = (uintptr_t)&__bss_start__;
    const uintptr_t bss_end = (uintptr_t)&__bss_end__;

    const uintptr_t heap_end = (uintptr_t)&__HeapLimit;
    const uintptr_t stack_start = (uintptr_t)&__StackBottom;
    const uintptr_t stack_end = (uintptr_t)&__StackTop;

    // Размеры областей.
    const uintptr_t image_size = image_end - image_start;
    const uintptr_t boot2_size = boot2_end - boot2_start;
    const uintptr_t text_size = text_end - boot2_end;
    const uintptr_t data_size = data_end - data_start;
    const uintptr_t bss_size = bss_end - bss_start;
    const uintptr_t heap_size = heap_end - bss_end;
    const uintptr_t stack_size = stack_end - stack_start;

    // Остальные части образа: дополнительные секции и выравнивание.
    const uintptr_t other_size =
        image_size - boot2_size - text_size - data_size;

    printf("%-10s %-10s %-10s %8s\n",
           "region", "start", "end", "bytes");

    row("flash", flash_start, flash_end);
    row("sram", sram_start, sram_end);
    row("rom", rom_start, rom_end);

    row("image", image_start, image_end);
    row("free", image_end, flash_end);
    row("boot2", boot2_start, boot2_end);
    row("text", boot2_end, text_end);

    row("data flash", text_end, text_end + data_size);
    row("data ram", data_start, data_end);
    row("bss", bss_start, bss_end);
    row("heap", bss_end, heap_end);
    row("stack", stack_start, stack_end);

    // Четыре итоговые строки.
    printf("\nflash image: %8u = boot2 %8u + text %8u"
           " + data %8u + other %8u\n",
           (unsigned)image_size,
           (unsigned)boot2_size,
           (unsigned)text_size,
           (unsigned)data_size,
           (unsigned)other_size);

    printf("\ntotal\n");

    printf("flash image %8u = boot2 %8u + text %8u"
           " + data %8u + other %8u\n",
           (unsigned)image_size,
           (unsigned)boot2_size,
           (unsigned)text_size,
           (unsigned)data_size,
           (unsigned)other_size);

    printf("flash free  %8u / %8u\n",
           (unsigned)(flash_end - image_end),
           (unsigned)(flash_end - flash_start));

    printf("ram used    %8u = data %8u + bss %8u\n",
           (unsigned)(data_size + bss_size),
           (unsigned)data_size,
           (unsigned)bss_size);

    printf("ram free    %8u heap + %8u stack = %8u total\n",
           (unsigned)heap_size,
           (unsigned)stack_size,
           (unsigned)(heap_size + stack_size));
}

void fw_info(void)
{
    // Увеличиваем при каждом вызове.
    ++data_variable;
    ++bss_variable;

    // Указатели на функции содержат признак Thumb.
    const uintptr_t main_address = (uintptr_t)main;
    const uintptr_t fw_info_address = (uintptr_t)fw_info;

    // Для чтения кода убираем признак Thumb.
    const uint16_t *main_code =
        (const uint16_t *)(main_address & ~(uintptr_t)1);

    const uint16_t *fw_info_code =
        (const uint16_t *)(fw_info_address & ~(uintptr_t)1);

    // Локальная переменная — на стеке.
    uint32_t stack_variable = 1946;

    // Выделяем блок в куче.
    uint32_t *heap_variable = malloc(sizeof(uint32_t));

    if (heap_variable != NULL)
    {
        *heap_variable = 1951;
    }

    printf("\n%-16s %-10s %s\n", "object", "address", "value");

    // Адрес функции печатаем с признаком Thumb.
    // Первые два байта читаем по адресу без этого признака.
    printf("%-16s 0x%08x 0x%04x\n",
           "main",
           (unsigned)main_address,
           (unsigned)main_code[0]);

    printf("%-16s 0x%08x 0x%04x\n",
           "fw_info",
           (unsigned)fw_info_address,
           (unsigned)fw_info_code[0]);

    // Здесь будут строки массива commands и его обработчиков.

    // Строки паспорта: адрес строки и её содержимое.
    printf("%-16s 0x%08x %s\n",
           "project",
           (unsigned)(uintptr_t)DEVICE_PROJECT,
           DEVICE_PROJECT);

    printf("%-16s 0x%08x %s\n",
           "repo",
           (unsigned)(uintptr_t)DEVICE_REPO,
           DEVICE_REPO);

    printf("%-16s 0x%08x %s\n",
           "board",
           (unsigned)(uintptr_t)DEVICE_BOARD,
           DEVICE_BOARD);

    // Глобальные переменные: адрес и текущее значение.
    printf("%-16s 0x%08x %u\n",
           "data_variable",
           (unsigned)(uintptr_t)&data_variable,
           (unsigned)data_variable);

    printf("%-16s 0x%08x %u\n",
           "bss_variable",
           (unsigned)(uintptr_t)&bss_variable,
           (unsigned)bss_variable);

    // Стек: адрес локальной переменной.
    printf("%-16s 0x%08x %u\n",
           "stack_variable",
           (unsigned)(uintptr_t)&stack_variable,
           (unsigned)stack_variable);

    // Куча: адрес выделенного блока, а не адрес указателя.
    if (heap_variable != NULL)
    {
        printf("%-16s 0x%08x %u\n",
               "heap_variable",
               (unsigned)(uintptr_t)heap_variable,
               (unsigned)*heap_variable);
    }
    else
    {
        printf("%-16s %-10s %s\n",
               "heap_variable", "NULL", "malloc failed");
    }

    // Возвращаем выделенную память. free(NULL) тоже допустим.
    free(heap_variable);
}