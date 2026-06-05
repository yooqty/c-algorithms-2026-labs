#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 2-битовая кодировка (00, 01, 10, 11)
// Используем код 11 как escape для N
#define ENC_A  0b00
#define ENC_C  0b01
#define ENC_G  0b10
#define ENC_T  0b11
#define ESCAPE 0b11  // escape-код (совпадает с T, нужно различать контекстом)

// Глобальные счётчики
long long count_A = 0, count_C = 0, count_G = 0, count_T = 0, count_N = 0;

// Функция получения кода символа
int char_to_code(char c) {
    switch (c) {
        case 'A': count_A++; return ENC_A;
        case 'C': count_C++; return ENC_C;
        case 'G': count_G++; return ENC_G;
        case 'T': count_T++; return ENC_T;
        case 'N': count_N++; return ESCAPE;
        default: return -1;
    }
}

int main(int argc, char *argv[]) {
    const char *input_filename;
    const char *output_filename = "compressed.txt";
    
    // Определяем имя входного файла
    if (argc >= 2) {
        input_filename = argv[1];
    } else {
        input_filename = "01hgp10a.txt";
    }
    
    // Если передан второй аргумент - имя выходного файла
    if (argc >= 3) {
        output_filename = argv[2];
    }
    
    FILE *in = fopen(input_filename, "r");
    if (!in) {
        fprintf(stderr, "Ошибка: не удалось открыть файл %s\n", input_filename);
        return 1;
    }
    
    // Открываем выходной текстовый файл
    FILE *out = fopen(output_filename, "w");
    if (!out) {
        fprintf(stderr, "Ошибка: не удалось создать %s\n", output_filename);
        fclose(in);
        return 1;
    }
    
    // Буфер для накопления битов
    unsigned long long buffer = 0;
    int bit_count = 0;
    
    char line[1024];
    int escape_mode = 0;
    int first_number = 1;  // для форматирования (без лишнего пробела в начале)
    
    // Читаем файл построчно
    while (fgets(line, sizeof(line), in)) {
        for (char *p = line; *p && *p != '\n'; p++) {
            char c = *p;
            int code = char_to_code(c);
            if (code == -1) continue;
            
            if (code == ESCAPE) {
                buffer = (buffer << 2) | ESCAPE;
                bit_count += 2;
                escape_mode = 1;
                continue;
            }
            
            if (escape_mode) {
                buffer = (buffer << 2) | code;
                bit_count += 2;
                escape_mode = 0;
            } else {
                buffer = (buffer << 2) | code;
                bit_count += 2;
            }
            
            // Если набрали 64 бита - записываем число в текстовый файл
            if (bit_count >= 64) {
                unsigned long long out_val = buffer >> (bit_count - 64);
                
                if (!first_number) {
                    fprintf(out, " ");
                }
                fprintf(out, "%llu", out_val);
                first_number = 0;
                
                buffer = buffer & ((1ULL << (bit_count - 64)) - 1);
                bit_count -= 64;
            }
        }
    }
    
    // Записываем оставшиеся биты (добиваем нулями до 64 бит)
    if (bit_count > 0) {
        buffer <<= (64 - bit_count);
        
        if (!first_number) {
            fprintf(out, " ");
        }
        fprintf(out, "%llu", buffer);
    }
    
    fclose(in);
    fclose(out);
    
    // Вывод статистики
    printf("Результаты подсчёта азотистых оснований:\n");
    printf("A (аденин):   %lld\n", count_A);
    printf("C (цитозин):  %lld\n", count_C);
    printf("G (гуанин):   %lld\n", count_G);
    printf("T (тимин):    %lld\n", count_T);
    printf("N (неопред.): %lld\n", count_N);
    
    long long total = count_A + count_C + count_G + count_T + count_N;
    printf("\nВсего символов: %lld\n", total);
    printf("Сжатые данные записаны в файл %s (в десятичном виде)\n", output_filename);
    
    return 0;
}