#include <stdio.h>
#include "pico/stdlib.h"

// FatFS includes
#include "f_util.h"
#include "ff.h"
#include "hw_config.h"

int main() {
    // Initialize stdio
    stdio_init_all();
    
    // Wait for USB serial connection
    sleep_ms(2000);
    
    printf("\n--- SD Card FatFS Demo ---\n");
    printf("Build: %s %s\n\n", __DATE__, __TIME__);
    
    // Get SD card instance
    printf("Initializing SD card...\n");
    sd_card_t *pSD = sd_get_by_num(0);
    if (!pSD) {
        printf("ERROR: Failed to get SD card instance\n");
        while (1) sleep_ms(1000);
    }
    
    // Mount filesystem
    printf("Mounting filesystem...\n");
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (FR_OK != fr) {
        printf("ERROR: f_mount failed: %s (%d)\n", FRESULT_str(fr), fr);
        printf("\nCheck: SD card inserted? Formatted as FAT32? Wiring correct?\n");
        while (1) sleep_ms(1000);
    }
    printf("Filesystem mounted ok\n\n");
    
    // Test 1: Write a file
    printf("Test 1: Writing to file\n");
    
    FIL fil;
    const char* filename = "test.txt";
    
    printf("Opening %s...\n", filename);
    fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if (FR_OK != fr) {
        printf("ERROR: f_open failed: %s (%d)\n", FRESULT_str(fr), fr);
    } else {
        printf("File opened\n");
        
        printf("Writing data...\n");
        int ret = f_printf(&fil, "Hello from RP2040!\n");
        ret += f_printf(&fil, "SD card is working!\n");
        ret += f_printf(&fil, "Medical Clinic Check-in System\n");
        
        if (ret < 0) {
            printf("ERROR: f_printf failed\n");
        } else {
            printf("Wrote %d bytes\n", ret);
        }
        
        fr = f_close(&fil);
        if (FR_OK != fr) {
            printf("ERROR: f_close failed: %s (%d)\n", FRESULT_str(fr), fr);
        } else {
            printf("File closed\n");
        }
    }
    printf("\n");
    
    // Test 2: Read the file
    printf("Test 2: Reading from file\n");
    
    printf("Opening %s...\n", filename);
    fr = f_open(&fil, filename, FA_READ);
    if (FR_OK != fr) {
        printf("ERROR: f_open failed: %s (%d)\n", FRESULT_str(fr), fr);
    } else {
        printf("File opened\n");
        printf("\n--- File content:\n");
        
        char line[128];
        while (f_gets(line, sizeof(line), &fil)) {
            printf("%s", line);
        }
        
        printf("--- End\n\n");
        
        f_close(&fil);
        printf("File closed\n");
    }
    printf("\n");
    
    // Test 3: Append to file
    printf("Test 3: Appending to file\n");
    
    const char* logfile = "log.txt";
    printf("Opening %s...\n", logfile);
    fr = f_open(&fil, logfile, FA_WRITE | FA_OPEN_APPEND);
    if (FR_OK != fr) {
        printf("ERROR: f_open failed: %s (%d)\n", FRESULT_str(fr), fr);
    } else {
        printf("File opened for append\n");
        
        printf("Appending log entry...\n");
        if (f_printf(&fil, "[%lu] System started\n", to_ms_since_boot(get_absolute_time())) < 0) {
            printf("ERROR: f_printf failed\n");
        } else {
            printf("Log entry added\n");
        }
        
        f_close(&fil);
        printf("File closed\n");
    }
    printf("\n");
    
    // Test 4: File information
    printf("Test 4: File information\n");
    
    FILINFO fno;
    fr = f_stat(filename, &fno);
    if (FR_OK == fr) {
        printf("File: %s\n", filename);
        printf("  Size: %lu bytes\n", (unsigned long)fno.fsize);
        printf("  Date: %04d-%02d-%02d\n", 
               1980 + ((fno.fdate >> 9) & 0x7F),
               (fno.fdate >> 5) & 0x0F,
               fno.fdate & 0x1F);
        printf("  Time: %02d:%02d:%02d\n",
               (fno.ftime >> 11) & 0x1F,
               (fno.ftime >> 5) & 0x3F,
               (fno.ftime & 0x1F) * 2);
    } else {
        printf("ERROR: f_stat failed: %s (%d)\n", FRESULT_str(fr), fr);
    }
    printf("\n");
    
    // Test 5: Storage information
    printf("Test 5: Storage information\n");
    
    FATFS *fs;
    DWORD fre_clust;
    
    fr = f_getfree("0:", &fre_clust, &fs);
    if (FR_OK == fr) {
        DWORD tot_sect = (fs->n_fatent - 2) * fs->csize;
        DWORD fre_sect = fre_clust * fs->csize;
        
        printf("SD Card:\n");
        printf("  Total: %lu MB (%lu sectors)\n", tot_sect / 2048, tot_sect);
        printf("  Free:  %lu MB (%lu sectors)\n", fre_sect / 2048, fre_sect);
        printf("  Used:  %lu MB\n", (tot_sect - fre_sect) / 2048);
        
        float usage = ((tot_sect - fre_sect) * 100.0f) / tot_sect;
        printf("  Usage: %.1f%%\n", usage);
    } else {
        printf("ERROR: f_getfree failed: %s (%d)\n", FRESULT_str(fr), fr);
    }
    printf("\n");
    
    // Test 6: Directory listing
    printf("Test 6: Directory listing\n");
    
    DIR dir;
    fr = f_opendir(&dir, "/");
    if (FR_OK == fr) {
        printf("Files in root:\n");
        
        int file_count = 0;
        while (true) {
            fr = f_readdir(&dir, &fno);
            if (fr != FR_OK || fno.fname[0] == 0) break;
            
            if (fno.fattrib & AM_DIR) {
                printf("  [DIR]  %s\n", fno.fname);
            } else {
                printf("  [FILE] %-20s %8lu bytes\n", fno.fname, (unsigned long)fno.fsize);
                file_count++;
            }
        }
        
        f_closedir(&dir);
        printf("\nTotal: %d files\n", file_count);
    } else {
        printf("ERROR: f_opendir failed: %s (%d)\n", FRESULT_str(fr), fr);
    }
    printf("\n");
    
    // Unmount
    printf("Unmounting filesystem...\n");
    f_unmount(pSD->pcName);
    printf("Done\n\n");
    
    printf("Demo complete. Files created:\n");
    printf("  - test.txt\n");
    printf("  - log.txt\n");
    
    // Loop forever
    while (1) {
        sleep_ms(1000);
    }
    
    return 0;
}
