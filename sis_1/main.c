#include <stdio.h>
#include <string.h>
#include "fs.h"

int main() {
    int choice;
    char filename[100], newname[100], filename2[100];
    char data[1024];
    int size;

    while (1) {
        printf("\n--- SimpleFS Menü ---\n");
        printf("1. Disk Formatla\n");
        printf("2. Dosya Oluştur\n");
        printf("3. Dosyaya Yaz\n");
        printf("4. Dosyadan Oku\n");
        printf("5. Dosya Sil\n");
        printf("6. Dosyaları Listele\n");
        printf("7. Dosya Yeniden Adlandır (rename)\n");
        printf("8. Dosya Var mı (exists)\n");
        printf("9. Dosya Boyutu (size)\n");
        printf("10. Dosyaya Veri Ekle (append)\n");
        printf("11. Dosyayı Kısalt (truncate)\n");
        printf("12. Dosyayı Kopyala (copy)\n");
        printf("13. Dosyayı Taşı (mv)\n");
        printf("14. Diski Defragment Et\n");
        printf("15. Bütünlük Kontrolü (integrity)\n");
        printf("16. Disk Yedekle (backup)\n");
        printf("17. Yedeği Geri Yükle (restore)\n");
        printf("18. Dosya İçeriğini Göster (cat)\n");
        printf("19. İki Dosyayı Karşılaştır (diff)\n");
        printf("20. İşlem Günlüğü Kaydet (log)\n");
        printf("0. Çıkış\n");
        printf("Seçiminiz: ");
        scanf("%d", &choice);
        getchar(); // \n yakala

        switch (choice) {
            case 1:
                fs_format();
                printf("Disk formatlandı.\n");
                break;
            case 2:
                printf("Dosya adı: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                if (fs_create(filename) == 0)
                    printf("Dosya oluşturuldu.\n");
                else
                    printf("Dosya oluşturulamadı.\n");
                break;
            case 3:
                printf("Dosya adı: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                printf("Yazılacak veri: ");
                fgets(data, sizeof(data), stdin);
                data[strcspn(data, "\n")] = 0;
                size = strlen(data);
                if (fs_write(filename, data, size) == 0)
                    printf("Yazıldı.\n");
                else
                    printf("Yazma hatası.\n");
                break;
            case 4:
                printf("Dosya adı: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                fs_read(filename);
                break;
            case 5:
                printf("Dosya silinecek adı: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                fs_delete(filename);
                break;
            case 6:
                fs_list();
                break;
            case 7:
                printf("Eski ad: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                printf("Yeni ad: ");
                fgets(newname, sizeof(newname), stdin);
                newname[strcspn(newname, "\n")] = 0;
                fs_rename(filename, newname);
                break;
            case 8:
                printf("Dosya adı: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                printf(fs_exists(filename) ? "Dosya var.\n" : "Dosya yok.\n");
                break;
            case 9:
                printf("Dosya adı: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                printf("Boyut: %d byte\n", fs_size(filename));
                break;
            case 10:
                printf("Dosya adı: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                printf("Eklenecek veri: ");
                fgets(data, sizeof(data), stdin);
                data[strcspn(data, "\n")] = 0;
                fs_append(filename, data, strlen(data));
                break;
            case 11:
                printf("Dosya adı: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                printf("Yeni boyut: ");
                scanf("%d", &size); getchar();
                fs_truncate(filename, size);
                break;
            case 12:
                printf("Kaynak dosya: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                printf("Hedef dosya: ");
                fgets(filename2, sizeof(filename2), stdin);
                filename2[strcspn(filename2, "\n")] = 0;
                fs_copy(filename, filename2);
                break;
            case 13:
                printf("Taşınacak dosya adı: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                printf("Yeni ad: ");
                fgets(newname, sizeof(newname), stdin);
                newname[strcspn(newname, "\n")] = 0;
                fs_mv(filename, newname);
                break;
            case 14:
                fs_defragment();
                printf("Defragment işlemi tamamlandı.\n");
                break;
            case 15:
                if (fs_check_integrity() == 0)
                    printf("Disk bütünlüğü sağlıklı.\n");
                else
                    printf("Diskte tutarsızlık var!\n");
                break;
            case 16:
                printf("Yedek dosya adı (örn: yedek.sim): ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                fs_backup(filename);
                break;
            case 17:
                printf("Geri yüklenecek yedek dosya: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                fs_restore(filename);
                break;
            case 18:
                printf("Görüntülenecek dosya: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                fs_cat(filename);
                break;
            case 19:
                printf("1. Dosya: ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = 0;
                printf("2. Dosya: ");
                fgets(filename2, sizeof(filename2), stdin);
                filename2[strcspn(filename2, "\n")] = 0;
                fs_diff(filename, filename2);
                break;
            case 20:
                printf("Log mesajı: ");
                fgets(data, sizeof(data), stdin);
                data[strcspn(data, "\n")] = 0;
                fs_log(data);
                break;
            case 0:
                printf("Çıkılıyor...\n");
                return 0;
            default:
                printf("Geçersiz seçim.\n");
        }
    }

    return 0;
}
