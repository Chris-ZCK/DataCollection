/*
 * @Author: your name
 * @Date: 2020-08-10 17:34:15
 * @LastEditTime: 2020-08-12 06:47:10
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: \USERd:\Documents\GitProject\DataCollection\DProj\APP\FATFS\exfuns\fattester.h
 */
#ifndef __FATTESTER_H
#define __FATTESTER_H 			   
#include <stm32f4xx.h>	   
#include "ff.h"
 
u8 mf_mount(u8* path,u8 mt);
u8 mf_open(u8*path,u8 mode);
u8 mf_close(void);
u8 mf_sync(void);
u8 mf_read(u16 len);
u8 mf_write(u8*dat,u16 len);
u8 mf_opendir(u8* path);
u8 mf_closedir(void);
u8 mf_readdir(void);
u8 mf_scan_files(u8 * path);
u32 mf_showfree(u8 *drv);
u8 mf_lseek(u32 offset);
u32 mf_tell(void);
u32 mf_size(void);
u8 mf_mkdir(u8*pname);
u8 mf_fmkfs(u8* path,u8 mode,u16 au);
u8 mf_unlink(u8 *pname);
u8 mf_rename(u8 *oldname,u8* newname);
void mf_getlabel(u8 *path);
void mf_setlabel(u8 *path); 
void mf_gets(u16 size);
u8 mf_putc(u8 c);
u8 mf_puts(u8*c);
 

u8 mf_log_init(void);
u8 * get_src_dname ( u8 * dpfn );					/*得到路径下的文件夹*/
u8 mf_copy ( u8 * psrc, u8 * pdst, u8 fwmode );		/*file   copy*/
u8 mf_dcopy ( u8 * psrc, u8 * pdst, u8 fwmode );	/*folder copy*/
u8 mf_send_pics(u8 *psrc, u8 *pdst, u8 fwmode);		/*picture send*/
u8 mf_sensordata_write(u8 *data);
u8 mf_sensordata_send(void);
//u8 mf_config_data_write(u8 *data);
//u8 mf_config_data_read(u8 *fbuf);
void mf_config_data_write_flash(u8 *data);
void mf_config_data_read_flash(u8 *fbuf);
void mf_send_log(void);
extern u8 sd_ready_flag;
void mf_check_dir(u8* path);
#endif





























