#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct point {
    double x;
    double y;
    int num;
} POINT;

POINT p[100000];
int hull[100000];
int hull_count = 0;

// Функция сравнения для qsort
int compare_points(const void *a, const void *b) {
    POINT *pa = (POINT *)a;
    POINT *pb = (POINT *)b;
    if (pa->x < pb->x) return -1;
    if (pa->x > pb->x) return 1;
    if (pa->y < pb->y) return -1;
    if (pa->y > pb->y) return 1;
    return 0;
}

// Проверка положения точки относительно вектора
double cross(double ax, double ay, double bx, double by, double cx, double cy) {
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

// Нахождение самой удаленной точки
int find_farthest(POINT *points, int n, POINT p1, POINT p2) {
    int farthest_idx = -1;
    double max_dist = -1;
    
    for (int i = 0; i < n; i++) {
        double dist = fabs(cross(p1.x, p1.y, p2.x, p2.y, points[i].x, points[i].y));
        if (dist > max_dist) {
            max_dist = dist;
            farthest_idx = i;
        }
    }
    return farthest_idx;
}

// Разделение точек
void partition_points(POINT *points, int n, POINT p1, POINT p2, 
                       POINT *left, int *left_count) {
    *left_count = 0;
    
    for (int i = 0; i < n; i++) {
        double cr = cross(p1.x, p1.y, p2.x, p2.y, points[i].x, points[i].y);
        if (cr > 0) {
            left[(*left_count)++] = points[i];
        }
    }
}

// Рекурсивное построение выпуклой оболочки
void quickhull(POINT *points, int n, POINT p1, POINT p2) {
    if (n == 0) return;
    
    int farthest_idx = find_farthest(points, n, p1, p2);
    if (farthest_idx == -1) return;
    
    POINT farthest = points[farthest_idx];
    hull[hull_count++] = farthest.num;
    
    POINT *left1 = malloc(n * sizeof(POINT));
    POINT *left2 = malloc(n * sizeof(POINT));
    int count1 = 0, count2 = 0;
    
    partition_points(points, n, p1, farthest, left1, &count1);
    partition_points(points, n, farthest, p2, left2, &count2);
    
    quickhull(left1, count1, p1, farthest);
    quickhull(left2, count2, farthest, p2);
    
    free(left1);
    free(left2);
}

// Основная функция построения выпуклой оболочки
void build_convex_hull(POINT *p, int n) {
    if (n < 3) {
        for (int i = 0; i < n; i++) {
            hull[hull_count++] = p[i].num;
        }
        return;
    }
    
    qsort(p, n, sizeof(POINT), compare_points);
    
    POINT leftmost = p[0];
    POINT rightmost = p[n-1];
    
    hull[hull_count++] = leftmost.num;
    hull[hull_count++] = rightmost.num;
    
    POINT *upper = malloc(n * sizeof(POINT));
    POINT *lower = malloc(n * sizeof(POINT));
    int upper_count = 0, lower_count = 0;
    
    for (int i = 0; i < n; i++) {
        double cr = cross(leftmost.x, leftmost.y, rightmost.x, rightmost.y, p[i].x, p[i].y);
        if (cr > 0) {
            upper[upper_count++] = p[i];
        } else if (cr < 0) {
            lower[lower_count++] = p[i];
        }
    }
    
    quickhull(upper, upper_count, leftmost, rightmost);
    quickhull(lower, lower_count, rightmost, leftmost);
    
    free(upper);
    free(lower);
}

// Удаление дубликатов
void clean_hull() {
    int unique[100000] = {0};
    int temp[100000];
    int temp_count = 0;
    
    for (int i = 0; i < hull_count; i++) {
        if (!unique[hull[i]]) {
            unique[hull[i]] = 1;
            temp[temp_count++] = hull[i];
        }
    }
    
    hull_count = temp_count;
    for (int i = 0; i < hull_count; i++) {
        hull[i] = temp[i];
    }
}

int main(int argc, char *argv[]) {
    FILE *In, *Out;
    int i = 0, num;
    double x, y;
    char filename[100];
    
    // Если файл не указан в аргументах, запрашиваем имя
    if (argc < 2) {
        printf("Введите имя входного файла: ");
        scanf("%s", filename);
    } else {
        sprintf(filename, "%s", argv[1]);
    }
    
    // Открываем входной файл
    In = fopen(filename, "r");
    if (In == NULL) {
        printf("Ошибка: не удалось открыть файл '%s'\n", filename);
        printf("Убедитесь, что файл существует в текущей директории\n");
        printf("Текущая директория: ");
        system("pwd");  // Для Linux/Mac
        // system("cd"); // Для Windows
        return 1;
    }
    
    // Чтение точек
    while (fscanf(In, "%d%lf%lf", &num, &x, &y) == 3) {
        p[i].num = num;
        p[i].x = x;
        p[i].y = y;
        i++;
    }
    int n = i;
    fclose(In);
    
    printf("Прочитано %d точек из файла '%s'\n", n, filename);
    
    // Создаем выходной файл
    Out = fopen("OUT.txt", "w");
    if (Out == NULL) {
        printf("Ошибка: не удалось создать файл OUT.txt\n");
        return 1;
    }
    
    if (n == 0) {
        fprintf(Out, "Файл пуст или имеет неверный формат\n");
        printf("Файл пуст или имеет неверный формат\n");
        fclose(Out);
        return 0;
    }
    
    if (n < 3) {
        printf("Предупреждение: точек меньше 3, выпуклая оболочка не определена\n");
        fprintf(Out, "Точек меньше 3, выпуклая оболочка не определена\n");
        fprintf(Out, "Номера точек: ");
        for (int j = 0; j < n; j++) {
            fprintf(Out, "%d ", p[j].num);
        }
        fprintf(Out, "\n");
        fclose(Out);
        printf("Результат сохранен в OUT.txt\n");
        return 0;
    }
    
    // Построение выпуклой оболочки
    build_convex_hull(p, n);
    clean_hull();
    
    // Вывод результатов
    printf("Построена выпуклая оболочка из %d точек\n", hull_count);
    
    fprintf(Out, "Выпуклая оболочка (номера точек):\n");
    for (int j = 0; j < hull_count; j++) {
        fprintf(Out, "%d ", hull[j]);
    }
    fprintf(Out, "\n\n");
    
    fprintf(Out, "Координаты точек оболочки:\n");
    for (int j = 0; j < n; j++) {
        for (int k = 0; k < hull_count; k++) {
            if (p[j].num == hull[k]) {
                fprintf(Out, "%d\t(%.2f, %.2f)\n", p[j].num, p[j].x, p[j].y);
                break;
            }
        }
    }
    
    fclose(Out);
    printf("Результат успешно сохранен в файл 'OUT.txt'\n");
    
    return 0;
}