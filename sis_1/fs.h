#ifndef FS_H
#define FS_H

// Sabitler
#define DISK_NAME "disk.sim"
#define DISK_SIZE (1024 * 1024)           // 1 MB
#define METADATA_SIZE 4096                // 4 KB metadata alanı
#define BLOCK_SIZE 512                    // Blok boyutu
#define DATA_START METADATA_SIZE          // Veri alanı başlangıcı
#define MAX_FILES 68                      // Maksimum dosya sayısı
#define FILENAME_LEN 32                   // Dosya adı uzunluğu

// Bitmap için
#define TOTAL_BLOCKS ((DISK_SIZE - METADATA_SIZE) / BLOCK_SIZE)  // 2048
#define BITMAP_SIZE (TOTAL_BLOCKS / 8)                            // 256 byte

// Yapılar
typedef struct {
    char name[FILENAME_LEN];   // Dosya adı
    int size;                  // Dosya boyutu (byte)
    int start;                 // Veri alanındaki başlangıç adresi
    char created[20];          // Oluşturulma tarihi (YYYY-MM-DD HH:MM)
} FileEntry;

//  Fonksiyonlar
int fs_format();
int fs_create(const char* filename);
int fs_list();
int fs_write(const char* filename, const char* data, int size);
int fs_read(const char* filename);
int fs_delete(const char* filename);
int fs_rename(const char* old_name, const char* new_name);
int fs_exists(const char* filename);
int fs_size(const char* filename);
int fs_append(const char* filename, const char* data, int size);
int fs_truncate(const char* filename, int new_size);
int fs_copy(const char* src_filename, const char* dest_filename);
int fs_mv(const char* old_name, const char* new_name);
int fs_defragment();
int fs_check_integrity();
int fs_backup(const char* backup_filename);
int fs_restore(const char* backup_filename);
int fs_cat(const char* filename);
int fs_diff(const char* filename1, const char* filename2);
int fs_log(const char* message);

#endif
