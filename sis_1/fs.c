#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fs.h"

static FileEntry files[MAX_FILES];
static int file_count = 0;

void get_time(char* buffer) {
    time_t t = time(NULL);
    strftime(buffer, 20, "%Y-%m-%d %H:%M", localtime(&t));
}

static unsigned char block_bitmap[BITMAP_SIZE];

void load_metadata(FILE* fp) {
    fseek(fp, 0, SEEK_SET);
    fread(&file_count, sizeof(int), 1, fp);
    fread(files, sizeof(FileEntry), file_count, fp);
    fread(block_bitmap, sizeof(unsigned char), BITMAP_SIZE, fp); // Bitmapi oku
}

void save_metadata(FILE* fp) {
    fseek(fp, 0, SEEK_SET);
    fwrite(&file_count, sizeof(int), 1, fp);
    fwrite(files, sizeof(FileEntry), file_count, fp);
    fwrite(block_bitmap, sizeof(unsigned char), BITMAP_SIZE, fp); // Bitmapi yaz
}

int get_block(int index) {
    int byte = index / 8;
    int bit = index % 8;
    return (block_bitmap[byte] >> bit) & 1;
}





int find_free_blocks(int count) {
    for (int i = 0; i <= TOTAL_BLOCKS - count; i++) {
        int found = 1;
        for (int j = 0; j < count; j++) {
            if (get_block(i + j)) {
                found = 0;
                break;
            }
        }
        if (found) return i;
    }
    return -1;  // Yeterli boş blok yok
}

void set_block(int index, int value) {
    int byte = index / 8;
    int bit = index % 8;
    if (value)
        block_bitmap[byte] |= (1 << bit);  // set bit to 1 (used)
    else
        block_bitmap[byte] &= ~(1 << bit); // clear bit to 0 (free)
}

void save_bitmap(FILE* fp) {
    fseek(fp, METADATA_SIZE - BITMAP_SIZE, SEEK_SET);  // Metadata sonuna yaz
    fwrite(block_bitmap, 1, BITMAP_SIZE, fp);
}

void load_bitmap(FILE* fp) {
    fseek(fp, METADATA_SIZE - BITMAP_SIZE, SEEK_SET);
    fread(block_bitmap, 1, BITMAP_SIZE, fp);
}
int allocate_blocks_for_file(int index, int size) {
    int blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int start_block = find_free_blocks(blocks_needed);
    if (start_block == -1) return -1;

    for (int i = 0; i < blocks_needed; i++) {
        set_block(start_block + i, 1);
    }

    files[index].start = DATA_START + start_block * BLOCK_SIZE;
    files[index].size = size;
    return 0;
}
void free_blocks_of_file(int index) {
    int blocks = (files[index].size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int start_block = (files[index].start - DATA_START) / BLOCK_SIZE;
    for (int i = 0; i < blocks; i++) {
        set_block(start_block + i, 0);
    }
}



int fs_format() {
    FILE* fp = fopen(DISK_NAME, "wb");
    if (!fp) return -1;
    char zero = 0;
    for (int i = 0; i < DISK_SIZE; i++) fwrite(&zero, 1, 1, fp);
    fclose(fp);
    fp = fopen(DISK_NAME, "rb+");
    file_count = 0;
    save_metadata(fp);
    fclose(fp);
    return 0;
}

int fs_create(const char* filename) {
    FILE* fp = fopen(DISK_NAME, "rb+");
    if (!fp) return -1;
    load_metadata(fp);

    if (file_count >= MAX_FILES) {
        fclose(fp);
        return -1;
    }

    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, filename) == 0) {
            fclose(fp);
            return -1;
        }
    }

    FileEntry fe;
    strncpy(fe.name, filename, FILENAME_LEN);
    fe.size = 0;
    fe.start = DATA_START + file_count * BLOCK_SIZE;
    get_time(fe.created);
    files[file_count++] = fe;
    save_metadata(fp);
    fclose(fp);
    return 0;
}

int fs_write(const char* filename, const char* data, int size) {
    FILE* fp = fopen(DISK_NAME, "rb+");
    if (!fp) return -1;
    load_metadata(fp);
    load_bitmap(fp);  // bitmap'i oku

    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, filename) == 0) {
            // Önce varsa önceki blokları boşalt
            if (files[i].size > 0) {
                free_blocks_of_file(i);
            }

            if (allocate_blocks_for_file(i, size) == -1) {
                fclose(fp);
                return -1; // Yeterli boş yer yok
            }

            // Veriyi yaz
            fseek(fp, files[i].start, SEEK_SET);
            fwrite(data, 1, size, fp);

            save_metadata(fp);
            save_bitmap(fp); // bitmap'i güncelle
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return -1;
}


int fs_read(const char* filename) {
    FILE* fp = fopen(DISK_NAME, "rb");
    if (!fp) return -1;
    load_metadata(fp);

    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, filename) == 0) {
            char* buffer = malloc(files[i].size + 1);
            fseek(fp, files[i].start, SEEK_SET);
            fread(buffer, 1, files[i].size, fp);
            buffer[files[i].size] = '\0';
            printf("İçerik: %s\n", buffer);
            free(buffer);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return -1;
}

int fs_list() {
    FILE* fp = fopen(DISK_NAME, "rb");
    if (!fp) return -1;
    load_metadata(fp);
    printf("Toplam dosya: %d\n", file_count);
    for (int i = 0; i < file_count; i++) {
        printf("Ad: %s | Boyut: %d | Başlangıç: %d | Tarih: %s\n",
            files[i].name, files[i].size, files[i].start, files[i].created);
    }
    fclose(fp);
    return 0;
}

int fs_rename(const char* old_name, const char* new_name) {
    FILE* fp = fopen(DISK_NAME, "rb+");
    if (!fp) return -1;
    load_metadata(fp);

    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, old_name) == 0) {
            strncpy(files[i].name, new_name, FILENAME_LEN);
            save_metadata(fp);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return -1;
}

int fs_exists(const char* filename) {
    FILE* fp = fopen(DISK_NAME, "rb");
    if (!fp) return 0;
    load_metadata(fp);

    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, filename) == 0) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

int fs_size(const char* filename) {
    FILE* fp = fopen(DISK_NAME, "rb");
    if (!fp) return -1;
    load_metadata(fp);

    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, filename) == 0) {
            fclose(fp);
            return files[i].size;
        }
    }

    fclose(fp);
    return -1;
}

int fs_append(const char* filename, const char* data, int size) {
    FILE* fp = fopen(DISK_NAME, "rb+");
    if (!fp) return -1;
    load_metadata(fp);
    load_bitmap(fp);

    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, filename) == 0) {
            int old_size = files[i].size;
            int new_size = old_size + size;
            // Blokları yeniden tahsis et
            free_blocks_of_file(i);
            if (allocate_blocks_for_file(i, new_size) == -1) {
                fclose(fp);
                return -1;
            }

            // Eski veri + yeni veri yaz
            fseek(fp, files[i].start, SEEK_SET);
            char* old_data = malloc(old_size);
            fread(old_data, 1, old_size, fp);
            fseek(fp, files[i].start, SEEK_SET);
            fwrite(old_data, 1, old_size, fp);
            fwrite(data, 1, size, fp);
            free(old_data);

            save_metadata(fp);
            save_bitmap(fp);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return -1;
}

int fs_truncate(const char* filename, int new_size) {
    FILE* fp = fopen(DISK_NAME, "rb+");
    if (!fp) return -1;
    load_metadata(fp);
    load_bitmap(fp);

    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, filename) == 0) {
            // Küçültmek serbest, büyütmek için yeniden blok gerekir
            if (new_size < files[i].size) {
                files[i].size = new_size;
                save_metadata(fp);
                save_bitmap(fp);
                fclose(fp);
                return 0;
            } else {
                // Yeniden ayır
                free_blocks_of_file(i);
                if (allocate_blocks_for_file(i, new_size) == -1) {
                    fclose(fp);
                    return -1;
                }
                save_metadata(fp);
                save_bitmap(fp);
                fclose(fp);
                return 0;
            }
        }
    }

    fclose(fp);
    return -1;
}

int fs_copy(const char* src, const char* dest) {
    FILE* fp = fopen(DISK_NAME, "rb+");
    if (!fp) return -1;
    load_metadata(fp);
    load_bitmap(fp);

    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, src) == 0) {
            char* buffer = malloc(files[i].size);
            fseek(fp, files[i].start, SEEK_SET);
            fread(buffer, 1, files[i].size, fp);
            fs_create(dest);
            fs_write(dest, buffer, files[i].size);
            free(buffer);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return -1;
}

int fs_mv(const char* old_name, const char* new_name) {
    return fs_rename(old_name, new_name);
}

int fs_defragment() {
    FILE* fp = fopen(DISK_NAME, "rb+");
    if (!fp) return -1;

    load_metadata(fp);
    load_bitmap(fp);

    int current_block = 0;

    for (int i = 0; i < file_count; i++) {
        int needed_blocks = (files[i].size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        int new_start = DATA_START + current_block * BLOCK_SIZE;

        if (files[i].start != new_start) {
            char* buffer = malloc(files[i].size);
            fseek(fp, files[i].start, SEEK_SET);
            fread(buffer, 1, files[i].size, fp);

            fseek(fp, new_start, SEEK_SET);
            fwrite(buffer, 1, files[i].size, fp);

            files[i].start = new_start;
            free(buffer);
        }

        for (int b = 0; b < needed_blocks; b++) {
            set_block(current_block + b, 1);
        }

        current_block += needed_blocks;
    }

    // Kalan blokları boşalt
    for (int i = current_block; i < TOTAL_BLOCKS; i++) {
        set_block(i, 0);
    }

    save_metadata(fp);
    save_bitmap(fp);
    fclose(fp);
    return 0;
}

int fs_check_integrity() {
    FILE* fp = fopen(DISK_NAME, "rb");
    if (!fp) return -1;

    load_metadata(fp);
    load_bitmap(fp);

    int valid = 1;
    unsigned char temp_bitmap[BITMAP_SIZE] = {0};

    for (int i = 0; i < file_count; i++) {
        int start_block = (files[i].start - DATA_START) / BLOCK_SIZE;
        int blocks = (files[i].size + BLOCK_SIZE - 1) / BLOCK_SIZE;

        for (int b = 0; b < blocks; b++) {
            int blk = start_block + b;
            int byte = blk / 8;
            int bit = blk % 8;

            if ((temp_bitmap[byte] >> bit) & 1) {
                printf("  Çakışan blok: %d (dosya: %s)\n", blk, files[i].name);
                valid = 0;
            } else {
                temp_bitmap[byte] |= (1 << bit);
            }

            if (!get_block(blk)) {
                printf(" Metadata blok ayırmış ama bitmap boş: %d (dosya: %s)\n", blk, files[i].name);
                valid = 0;
            }
        }
    }

    fclose(fp);
    return valid ? 0 : -1;
}

int fs_backup(const char* backup_filename) {
    FILE* src = fopen(DISK_NAME, "rb");
    FILE* dst = fopen(backup_filename, "wb");
    if (!src || !dst) return -1;

    char buffer[BLOCK_SIZE];
    size_t bytes;
    while ((bytes = fread(buffer, 1, BLOCK_SIZE, src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }

    fclose(src);
    fclose(dst);
    return 0;
}

int fs_restore(const char* backup_filename) {
    FILE* src = fopen(backup_filename, "rb");
    FILE* dst = fopen(DISK_NAME, "wb");
    if (!src || !dst) return -1;

    char buffer[BLOCK_SIZE];
    size_t bytes;
    while ((bytes = fread(buffer, 1, BLOCK_SIZE, src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }

    fclose(src);
    fclose(dst);
    return 0;
}

int fs_cat(const char* filename) {
    return fs_read(filename);
}

int fs_diff(const char* f1, const char* f2) {
    FILE* fp = fopen(DISK_NAME, "rb");
    if (!fp) return -1;
    load_metadata(fp);

    FileEntry* file1 = NULL;
    FileEntry* file2 = NULL;

    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, f1) == 0) file1 = &files[i];
        if (strcmp(files[i].name, f2) == 0) file2 = &files[i];
    }

    if (!file1 || !file2) {
        fclose(fp);
        return -1;
    }

    if (file1->size != file2->size) {
        printf("Boyutlar farklı: %d vs %d\n", file1->size, file2->size);
        fclose(fp);
        return 1;
    }

    char* buf1 = malloc(file1->size);
    char* buf2 = malloc(file2->size);

    fseek(fp, file1->start, SEEK_SET);
    fread(buf1, 1, file1->size, fp);
    fseek(fp, file2->start, SEEK_SET);
    fread(buf2, 1, file2->size, fp);

    int diff = memcmp(buf1, buf2, file1->size);

    if (diff == 0) printf("Dosyalar aynı.\n");
    else printf("Dosyalar farklı.\n");

    free(buf1);
    free(buf2);
    fclose(fp);
    return diff ? 1 : 0;
}

int fs_log(const char* message) {
    FILE* log = fopen("fs.log", "a");
    if (!log) return -1;

    time_t now = time(NULL);
    char* ts = ctime(&now);
    ts[strcspn(ts, "\n")] = 0;

    fprintf(log, "[%s] %s\n", ts, message);
    fclose(log);
    return 0;
}


int fs_delete(const char* filename) {
    FILE* fp = fopen(DISK_NAME, "rb+");
    if (!fp) return -1;
    load_metadata(fp);
    load_bitmap(fp);

    int found = 0;
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].name, filename) == 0) {
            // Blokları boşalt
            if (files[i].size > 0) {
                free_blocks_of_file(i);
            }

            // Dosya girişini kaydır
            for (int j = i; j < file_count - 1; j++) {
                files[j] = files[j + 1];
            }
            file_count--;
            found = 1;
            break;
        }
    }

    if (found) {
        save_metadata(fp);
        save_bitmap(fp);
    }

    fclose(fp);
    return found ? 0 : -1;
}

