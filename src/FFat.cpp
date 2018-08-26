// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "vfs_api.h"

extern "C" {
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "esp_vfs_fat.h"
#include "diskio.h"
#include "diskio_wl.h"
#include "vfs_fat_internal.h"
}
#include "FFat.h"

using namespace fs;

F_Fat::F_Fat(FSImplPtr impl)
    : FS(impl)
{}

bool F_Fat::begin(bool formatOnFail, const char * basePath, uint8_t maxOpenFiles, const char * partitionLabel)
{
    if(_wl_handle){
        log_w("Already Mounted!");
        return true;
    }     
    esp_vfs_fat_mount_config_t conf = {
      .format_if_mount_failed = formatOnFail,
      .max_files = maxOpenFiles
    };

    esp_err_t err = esp_vfs_fat_spiflash_mount(basePath, partitionLabel, &conf, &_wl_handle);
    if(err){
        log_e("Mounting FatFlash failed! Error: %d", err);
        return false;
    }
    _impl->mountpoint(basePath);
    return true;
}

void F_Fat::end()
{
    if(_wl_handle){
        esp_err_t err = esp_vfs_fat_spiflash_unmount(_impl->mountpoint(), _wl_handle);
        if(err){
            log_e("Unmounting FatFlash failed! Error: %d", err);
            return;
        }
        _wl_handle = NULL;
        _impl->mountpoint(NULL);
    }
}

bool F_Fat::format()
{
    esp_err_t result = ESP_OK;
    BYTE pdrv = 0xFF;
    if (ff_diskio_get_drive(&pdrv) != ESP_OK) {
        ESP_LOGD(TAG, "the maximum count of volumes is already mounted");
        return ESP_ERR_NO_MEM;
    }
    ESP_LOGD(TAG, "using pdrv=%i", pdrv);
    char drv[3] = {(char)('0' + pdrv), ':', 0};

    const size_t workbuf_size = 4096;
    void *workbuf = NULL;
    workbuf = malloc(workbuf_size);
    if (workbuf == NULL) {
        result = ESP_ERR_NO_MEM;
        log_e("Memory allocation failure");
        return false;
    }
    size_t alloc_unit_size = CONFIG_WL_SECTOR_SIZE;
    log_i("Formatting FatFlash partition, allocation unit size=%d", alloc_unit_size);
/*
    FRESULT fresult = f_mkfs(drv, FM_ANY | FM_SFD, alloc_unit_size, workbuf, workbuf_size);
    if (fresult != FR_OK) {
        result = ESP_FAIL;
        ESP_LOGE(TAG, "f_mkfs failed (%d)", fresult);
        return false;
    }
*/
    free(workbuf);
    workbuf = NULL;
    return true;
}

size_t F_Fat::totalBytes()
{
    FATFS *fs;
    DWORD free_clust, tot_sect, sect_size;

    BYTE pdrv = ff_diskio_get_pdrv_wl(_wl_handle);
    char drv[3] = {(char)'0' + pdrv, ':', 0};
    FRESULT res = f_getfree(drv, &free_clust, &fs);
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    sect_size=CONFIG_WL_SECTOR_SIZE;
    return tot_sect * sect_size;
}

size_t F_Fat::freeBytes()
{

    FATFS *fs;
    DWORD free_clust, free_sect, sect_size;

    BYTE pdrv = ff_diskio_get_pdrv_wl(_wl_handle);
    char drv[3] = {(char)'0' + pdrv, ':', 0};
    FRESULT res = f_getfree(drv, &free_clust, &fs);
    free_sect = free_clust * fs->csize;
    sect_size=CONFIG_WL_SECTOR_SIZE;
    return free_sect * sect_size;
}

bool F_Fat::exists(const char* path)
{
    File f = open(path, "r");
    return (f == true) && !f.isDirectory();
}

bool F_Fat::exists(const String& path)
{
    return exists(path.c_str());
}


F_Fat FFat = F_Fat(FSImplPtr(new VFSImpl()));
