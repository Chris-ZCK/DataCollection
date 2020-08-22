#include "fattester.h"	 
#include "sdio_sdcard.h"
#include "usart.h"
#include "exfuns.h"
#include "malloc.h"		  
#include "ff.h"
#include "string.h"
#include "mqttApp.h"
#include "stmflash.h"  	// for pic id
#include "iwdg.h"		// watchdog
#include "M8266WIFI_ops.h"
#include "MyFunction_C.h"

//为磁盘注册工作区	 
//path:磁盘路径，比如"0:"、"1:"
//mt:0，不立即注册（稍后注册）；1，立即注册
//返回值:执行结果
u8 mf_mount(u8* path,u8 mt)
{		   
	return f_mount(fs[2],(const TCHAR*)path,mt); 
}
//打开路径下的文件
//path:路径+文件名
//mode:打开模式
//返回值:执行结果
u8 mf_open(u8*path,u8 mode)
{
	u8 res;	 
	res=f_open(file,(const TCHAR*)path,mode);//打开文件夹
	return res;
} 
//关闭文件
//返回值:执行结果
u8 mf_close(void)
{
	sd_ready_flag =0;  // 关闭标志位
	delay_ms(10);
	//OS_ERR err;
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER();	//进入临界区
	f_close(file);
	OS_CRITICAL_EXIT();	//退出临界区
	return 0;
}
//同步文件
//返回值:执行结果
u8 mf_sync(void)
{
	//OS_ERR err;
	printf("*******************syn log****************\r\n");
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER();	//进入临界区
	f_sync(file);
	OS_CRITICAL_EXIT();	//退出临界区
	return 0;
}


//读出数据
//len:读出的长度
//返回值:执行结果
u8 mf_read(u16 len)
{
	u16 i,t;
	u8 res=0;
	u16 tlen=0;
	printf("\r\nRead file data is:\r\n");
	for(i=0;i<len/512;i++)
	{
		res=f_read(file,fatbuf,512,&br);
		if(res)
		{
			printf("Read Error:%d\r\n",res);
			break;
		}else
		{
			tlen+=br;
			for(t=0;t<br;t++)printf("%c",fatbuf[t]); 
		}
	}
	if(len%512)
	{
		res=f_read(file,fatbuf,len%512,&br);
		if(res)	//读数据出错了
		{
			printf("\r\nRead Error:%d\r\n",res);   
		}else
		{
			tlen+=br;
			for(t=0;t<br;t++)printf("%c",fatbuf[t]); 
		}	 
	}
	if(tlen)printf("\r\nReaded data len:%d\r\n",tlen);//读到的数据长度
	printf("Read data over\r\n");	 
	return res;
}
//写入数据
//dat:数据缓存区
//len:写入长度
//返回值:执行结果
u8 mf_write(u8*dat,u16 len)
{			    
	u8 res;	   					   

	printf("\r\nBegin Write file...\r\n");
	printf("Write data len:%d\r\n",len);	 
	res=f_write(file,dat,len,&bw);
	if(res)
	{
		printf("Write Error:%d\r\n",res);   
	}else printf("Writed data len:%d\r\n",bw);
	printf("Write data over.\r\n");
	return res;
}

//打开目录
 //path:路径
//返回值:执行结果
u8 mf_opendir(u8* path)
{
	return f_opendir(&dir,(const TCHAR*)path);	
}


/**
 * @description: 检查并创建目录
 * @param {type} 
 * @return {type} 
 */
void mf_check_dir(u8* path)
{
	if(mf_opendir(path)!=FR_OK)
	{
		if(mf_mkdir(path)==FR_OK)
		{
			printf("[LOG]Create directory %s.\r\n", path);
		}
		else
		{
			printf("[ERROR]Fail to create directory. %s @!!!!!!!!!!!!!!!\r\n", path);
		}
	}
	else
	{
		//printf("Directory %s already exists.\r\n", path);
	}

}
//关闭目录 
//返回值:执行结果
u8 mf_closedir(void)
{
	return f_closedir(&dir);	
}
//打读取文件夹
//返回值:执行结果
u8 mf_readdir(void)
{
	u8 res;
	char *fn;			 
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mymalloc(SRAMIN,fileinfo.lfsize);
#endif		  
	res=f_readdir(&dir,&fileinfo);//读取一个文件的信息
	if(res!=FR_OK||fileinfo.fname[0]==0)
	{
		myfree(SRAMIN,fileinfo.lfname);
		return res;//读完了.
	}
#if _USE_LFN
	fn=*fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else
	fn=fileinfo.fname;;
#endif	
	printf("\r\n DIR info:\r\n");

	printf("dir.id:%d\r\n",dir.id);
	printf("dir.index:%d\r\n",dir.index);
	printf("dir.sclust:%d\r\n",dir.sclust);
	printf("dir.clust:%d\r\n",dir.clust);
	printf("dir.sect:%d\r\n",dir.sect);	  

	printf("\r\n");
	printf("File Name is:%s\r\n",fn);
	printf("File Size is:%d\r\n",fileinfo.fsize);
	printf("File data is:%d\r\n",fileinfo.fdate);
	printf("File time is:%d\r\n",fileinfo.ftime);
	printf("File Attr is:%d\r\n",fileinfo.fattrib);
	printf("\r\n");
	myfree(SRAMIN,fileinfo.lfname);
	return 0;
}			 

 //遍历文件
 //path:路径
 //返回值:执行结果
u8 mf_scan_files(u8 * path)
{
	FRESULT res;	  
    char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mymalloc(SRAMIN,fileinfo.lfsize);
#endif		  
	printf("[INFO]scan files path:<%s>, the details are as follows:\r\n",path);
    res = f_opendir(&dir,(const TCHAR*)path); //打开一个目录
    if (res == FR_OK) 
	{	
		// printf("\r\n"); 
		while(1)
		{
	        res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
	        if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
	        //if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
#if _USE_LFN
        	fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else							   
        	fn = fileinfo.fname;
#endif	                                              /* It is a file. */
			printf("%s/", path);  // 打印路径	
			printf("%s\r\n",  fn);  // 打印文件名	  
		} 
		printf("[LOG]Above are all the directories.\r\n");
    }	  
	myfree(SRAMIN,fileinfo.lfname);
    return res;	  
}
//显示剩余容量
//drv:盘符
//返回值:剩余容量(字节)
u32 mf_showfree(u8 *drv)
{
	FATFS *fs1;
	u8 res;
    u32 fre_clust=0, fre_sect=0, tot_sect=0;
    //得到磁盘信息及空闲簇数量
    res = f_getfree((const TCHAR*)drv,(DWORD*)&fre_clust, &fs1);
    if(res==0)
	{											   
	    tot_sect = (fs1->n_fatent - 2) * fs1->csize;//得到总扇区数
	    fre_sect = fre_clust * fs1->csize;			//得到空闲扇区数	   
#if _MAX_SS!=512
		tot_sect*=fs1->ssize/512;
		fre_sect*=fs1->ssize/512;
#endif	  
		if(tot_sect<20480)//总容量小于10M
		{
		    /* Print free space in unit of KB (assuming 512 bytes/sector) */
		    printf("\r\n磁盘总容量:%d KB\r\n"
		           "可用空间:%d KB\r\n",
		           tot_sect>>1,fre_sect>>1);
		}else
		{
		    /* Print free space in unit of KB (assuming 512 bytes/sector) */
		    printf("\r\n磁盘总容量:%d MB\r\n"
		           "可用空间:%d MB\r\n",
		           tot_sect>>11,fre_sect>>11);
		}
	}
	return fre_sect;
}		    
//文件读写指针偏移
//offset:相对首地址的偏移量
//返回值:执行结果.
u8 mf_lseek(u32 offset)
{
	return f_lseek(file,offset);
}
//读取文件当前读写指针的位置.
//返回值:位置
u32 mf_tell(void)
{
	return f_tell(file);
}
//读取文件大小
//返回值:文件大小
u32 mf_size(void)
{
	return f_size(file);
} 
//创建目录
//pname:目录路径+名字
//返回值:执行结果
u8 mf_mkdir(u8*pname)
{
	return f_mkdir((const TCHAR *)pname);
}
//格式化
//path:磁盘路径，比如"0:"、"1:"
//mode:模式
//au:簇大小
//返回值:执行结果
u8 mf_fmkfs(u8* path,u8 mode,u16 au)
{
	return f_mkfs((const TCHAR*)path,mode,au);//格式化,drv:盘符;mode:模式;au:簇大小
} 
//删除文件/目录
//pname:文件/目录路径+名字
//返回值:执行结果
u8 mf_unlink(u8 *pname)
{
	return  f_unlink((const TCHAR *)pname);
}

//修改文件/目录名字(如果目录不同,还可以移动文件哦!)
//oldname:之前的名字
//newname:新名字
//返回值:执行结果
u8 mf_rename(u8 *oldname,u8* newname)
{
	return  f_rename((const TCHAR *)oldname,(const TCHAR *)newname);
}
//获取盘符（磁盘名字）
//path:磁盘路径，比如"0:"、"1:"  
void mf_getlabel(u8 *path)
{
	u8 buf[20];
	u32 sn=0;
	u8 res;
	res=f_getlabel ((const TCHAR *)path,(TCHAR *)buf,(DWORD*)&sn);
	if(res==FR_OK)
	{
		printf("\r\n磁盘%s 的盘符为:%s\r\n",path,buf);
		printf("磁盘%s 的序列号:%X\r\n\r\n",path,sn); 
	}else printf("\r\n获取失败，错误码:%X\r\n",res);
}
//设置盘符（磁盘名字），最长11个字符！！，支持数字和大写字母组合以及汉字等
//path:磁盘号+名字，比如"0:ALIENTEK"、"1:OPENEDV"  
void mf_setlabel(u8 *path)
{
	u8 res;
	res=f_setlabel ((const TCHAR *)path);
	if(res==FR_OK)
	{
		printf("\r\n磁盘盘符设置成功:%s\r\n",path);
	}else printf("\r\n磁盘盘符设置失败，错误码:%X\r\n",res);
} 

//从文件里面读取一段字符串
//size:要读取的长度
void mf_gets(u16 size)
{
 	TCHAR* rbuf;
	rbuf=f_gets((TCHAR*)fatbuf,size,file);
	if(*rbuf==0)return  ;//没有数据读到
	else
	{
		printf("\r\nThe String Readed Is:%s\r\n",rbuf);  	  
	}			    	
}
//需要_USE_STRFUNC>=1
//写一个字符到文件
//c:要写入的字符
//返回值:执行结果
u8 mf_putc(u8 c)
{
	return f_putc((TCHAR)c,file);
}
//写字符串到文件
//c:要写入的字符串
//返回值:写入的字符串长度
u8 mf_puts(u8*c)
{
	return f_puts((TCHAR*)c,file);
}

// 能够自动提取字符串中的数字
u32 srt2integer_pro(u8 *str)
{
    u8 len = 0;
    u8 i = 0;
    u8 buf[20] = {0};
    u32 wi = 1;
    u32 sum = 0;

    while (str[i])
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            buf[len] = str[i];
            //printf("%c",buf[len]);
            len++;
        }
        i++;
    }

    for (i = 0; i < len; i++)
    {
        sum = sum + (buf[len - 1 - i] - '0') * wi;
        wi *= 10;
    }
    return sum;
}



/** 
 * @brief send pictures of assigned path(folder)
 * 这里来看，文件不存在转存，而是直接删除了。
 * 因为文件被删除，因此也就不存在check发送失败的内容了。
 * @param psrc	源文件夹
 * @param pdst	归档文件夹
 * @param fwmode	
 *		<>1<> 覆盖模式
 *		<>0<> 非覆盖模式
 *
 * @return num of files
 * @note  max 20
 */
#define MAX_TRANSFORM_SINGLE 6
u8 mf_send_pics(u8 *psrc, u8 *pdst, u8 fwmode, u8 device)
{
	#define MAX_PATHNAME_DEPTH 100 + 1 //最大目标文件路径+文件名深度
	u8 cnt = 0;
	
	u8 res = 0;
    DIR *srcdir = 0;    // 源目录
    DIR *dstdir = 0;    // 源目录
    FILINFO *finfo = 0; // 文件信息
    u8 *fn = 0;         // 长文件名

    u8 *dstpathname = 0; // 目标文件夹路径+文件名
    u8 *srcpathname = 0; // 源文件夹路径+文件名

    u16 dstpathlen = 0; // 目标路径长度
    u16 srcpathlen = 0; // 源路径长度
	u32 id_pic;

    srcdir = (DIR *)mymalloc(SRAMIN, sizeof(DIR)); // 申请内存
    dstdir = (DIR *)mymalloc(SRAMIN, sizeof(DIR));
    finfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO));

    if (srcdir == NULL || dstdir == NULL || finfo == NULL)
    {
        res = 100;
    }
    if (res == 0)
    {
        finfo->lfsize = _MAX_LFN * 2 + 1;
        finfo->lfname = mymalloc(SRAMIN, finfo->lfsize); // 申请内存
        dstpathname = mymalloc(SRAMIN, MAX_PATHNAME_DEPTH);
        srcpathname = mymalloc(SRAMIN, MAX_PATHNAME_DEPTH);
        if (finfo->lfname == NULL || dstpathname == NULL || srcpathname == NULL)
        {
            res = 101;
        }
        if (res == 0)
        {
            dstpathname[0] = 0;
            srcpathname[0] = 0;
            strcat((char *)srcpathname, (const char *)psrc); // 复制原始源文件路径
            strcat((char *)dstpathname, (const char *)pdst); // 复制原始目标文件路径
            res = f_opendir(srcdir, (const TCHAR *)psrc);    // 打开源目录
            if (res == 0)                                    // 打开目录成功
            {
                res = f_mkdir((const TCHAR *)dstpathname);
                // 如果文件夹已经存在,就不创建.如果不存在就创建新的文件夹.
                if (res == FR_EXIST)
                {
                    res = 0;
                }
                while (res == 0)   // 开始复制文件夹里面的东东
                {
                    res = f_readdir(srcdir, finfo);  // 读取目录下的一个文件
                    if (res != FR_OK || finfo->fname[0] == 0 || cnt > MAX_TRANSFORM_SINGLE)
                    {
                        break;
                    } // 错误了/到末尾了,退出
                    if (finfo->fname[0] == '.')
                    {
                        continue;
                    } // 忽略上级目录
                    
					// 这里的名字是唯一的，要通过其写入时进行改变
					fn = (u8 *)(*finfo->lfname ? finfo->lfname : finfo->fname);
					id_pic=srt2integer_pro(fn);
                    // 得到文件名
                    dstpathlen = strlen((const char *)dstpathname); // 得到当前目标路径的长度
                    srcpathlen = strlen((const char *)srcpathname); // 得到源路径长度

                    strcat((char *)srcpathname, (const char *)"/"); // 源路径加斜杠
                    if (finfo->fattrib & 0X10)                      // 是子目录   文件属性,0X20,归档文件;0X10,子目录;
                    {
                        continue;
                    }    // 忽略子目录
                    else // 非目录
                    {
						u8 sta;
                        strcat((char *)dstpathname, (const char *)"/"); // 目标路径加斜杠
                        strcat((char *)dstpathname, (const char *)fn);  // 目标路径加文件名
                        strcat((char *)srcpathname, (const char *)fn);  // 源路径加文件名
                        printf("[INFO]]open file %s(src) and %s(dst)\r\n", srcpathname, dstpathname);
						//CPU_SR_ALLOC();
						//OS_CRITICAL_ENTER(); //进入临界区
						IWDG_Feed();//喂狗
						if(device==1)  // wifi
							sta = WiFiSendFile(srcpathname,id_pic);	   // 发送文件
						else  // 4G
							sta = mysend_picture(srcpathname, id_pic);	   // 发送文件
						//OS_CRITICAL_EXIT();  
                        //mf_copy(srcpathname, dstpathname, fwmode); // 复制文件
						mf_unlink(srcpathname);					     // 删除文件
						printf("[INFO]rm %s\r\n",srcpathname);
						cnt++;
						if(sta != 0) {
							printf("[WARNING]ERROR mysend_picture:%s,%d\r\n",srcpathname,id_pic);
							break;
						}
                    }
                    srcpathname[srcpathlen] = 0; // 加入结束符
                    dstpathname[dstpathlen] = 0; // 加入结束符
					
					// break;
                }
            }
            myfree(SRAMIN, dstpathname);
            myfree(SRAMIN, srcpathname);
            myfree(SRAMIN, finfo->lfname);
        }
    }
    myfree(SRAMIN, srcdir);
    myfree(SRAMIN, dstdir);
    myfree(SRAMIN, finfo);
    return res;
}


// 文件复制
// 将psrc文件,copy到pdst.
// psrc,pdst:源文件和目标文件
// fwmode:文件写入模式
// 0:不覆盖原有的文件
// 1:覆盖原有的文件
u8 mf_copy(u8 *psrc, u8 *pdst, u8 fwmode)
{
    u8 res;
    u16 br = 0;
    u16 bw = 0;
    FIL *fsrc = 0;
    FIL *fdst = 0;
    u8 *fbuf = 0;
    fsrc = (FIL *)mymalloc(SRAMIN, sizeof(FIL)); // 申请内存
    fdst = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    fbuf = (u8 *)mymalloc(SRAMIN, 512);
    
	//喂狗
	IWDG_Feed();//喂狗
	//
	
	if (fsrc == NULL || fdst == NULL || fbuf == NULL)
    {
        res = 100;
    } // 前面的值留给fatfs
    else
    {
        if (fwmode == 0)
        {
            fwmode = FA_CREATE_NEW;
        } // 不覆盖
        else
        {
            fwmode = FA_CREATE_ALWAYS;
        } // 覆盖存在的文件

        res = f_open(fsrc, (const TCHAR *)psrc, FA_READ | FA_OPEN_EXISTING);
        // 打开只读文件
        if (res == 0)
        {
            res = f_open(fdst, (const TCHAR *)pdst, FA_WRITE | fwmode);
        }             // 第一个打开成功,才开始打开第二个
        if (res == 0) // 两个都打开成功了
        {
            while (res == 0) // 开始复制
            {
                res = f_read(fsrc, fbuf, 512, (UINT *)&br); // 源头读出512字节
                if (res || br == 0)
                {
                    break;
                }
                res = f_write(fdst, fbuf, (UINT)br, (UINT *)&bw); // 写入目的文件
                if (res || bw < br)
                {
                    break;
                }
            }
            f_close(fsrc);
            f_close(fdst);
        }
    }
    myfree(SRAMIN, fsrc); // 释放内存
    myfree(SRAMIN, fdst);
    myfree(SRAMIN, fbuf);
    return res;
}

// 得到路径下的文件夹
// 返回值:0,路径就是个卷标号.
// 其他,文件夹名字首地址
u8* get_src_dname(u8* dpfn)
{
	u16 temp=0;
 	while(*dpfn!=0)
	{
		dpfn++;
		temp++;	
	}
	printf("temp=%d\r\n",temp);
	if(temp<4)return 0; 
	while((*dpfn!=0x5c)&&(*dpfn!=0x2f)&&(*dpfn!=':')) {	
		dpfn--;
		temp--;	//追述到倒数第一个"\"或者"/"处 
		printf("dpfn[%d]=%c\r\n",temp,*dpfn);
	}
	printf("dpfn=%d\r\n",++temp);
	return ++dpfn;
}


#define MAX_COYP_SINGLE 6
// 文件夹复制
// 将psrc文件夹,copy到pdst文件夹.
// pdst:必须形如"X:"/"X:XX"/"X:XX/XX"之类的.而且要实现确认上一级文件夹存在
// psrc,pdst:源文件夹和目标文件夹
// fwmode:文件写入模式
// 0:不覆盖原有的文件
// 1:覆盖原有的文件
u8 mf_dcopy(u8 *psrc, u8 *pdst, u8 fwmode, u8 if_save)
{
#define MAX_PATHNAME_DEPTH 100 + 1 //最大目标文件路径+文件名深度
    u8 cnt=0;
	
	u8 res = 0;
    DIR *srcdir = 0;    // 源目录
    DIR *dstdir = 0;    // 源目录
    FILINFO *finfo = 0; // 文件信息
    u8 *fn = 0;         // 长文件名

    u8 *dstpathname = 0; // 目标文件夹路径+文件名
    u8 *srcpathname = 0; // 源文件夹路径+文件名
	
	u8 *fn_t = 0; // #speical目标文件夹路径+文件名

    u16 dstpathlen = 0; // 目标路径长度
    u16 srcpathlen = 0; // 源路径长度

	//u16 id_in_flash; // #special
	STMFLASH_Read(FLASH_SAVE_ADDR,(u32 *)&eerom,sizeof(eerom)/4);
	//printf("[INFO]STMFLASH_Read|pic_id=%d,buf={%s}\r\n",eerom.id_in_flash,eerom.buf);
	printf("\r\n[INFO]STMFLASH_Read|pic_id=%d\r\n",eerom.id_in_flash);
	//eerom.id_in_flash = STMFLASH_Read_Num(FLASH_SAVE_ADDR); // ##special
	//id_in_flash = STMFLASH_Read_Num(FLASH_SAVE_ADDR); // ##special
	srcdir = (DIR *)mymalloc(SRAMIN, sizeof(DIR)); // 申请内存
    dstdir = (DIR *)mymalloc(SRAMIN, sizeof(DIR));
    finfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO));

    if (srcdir == NULL || dstdir == NULL || finfo == NULL)
    {
        res = 100;
    }
    if (res == 0)
    {
        finfo->lfsize = _MAX_LFN * 2 + 1;
        finfo->lfname = mymalloc(SRAMIN, finfo->lfsize); // 申请内存
        dstpathname = mymalloc(SRAMIN, MAX_PATHNAME_DEPTH);
        fn_t = mymalloc(SRAMIN, 30); // #speical
		srcpathname = mymalloc(SRAMIN, MAX_PATHNAME_DEPTH);
        if (finfo->lfname == NULL || dstpathname == NULL || srcpathname == NULL)
        {
            res = 101;
        }
        if (res == 0)
        {
            dstpathname[0] = 0;
            srcpathname[0] = 0;
            strcat((char *)srcpathname, (const char *)psrc); // 复制原始源文件路径
            strcat((char *)dstpathname, (const char *)pdst); // 复制原始目标文件路径
            res = f_opendir(srcdir, (const TCHAR *)psrc); // 打开源目录
            if (res == 0)                                 // 打开目录成功
            {
                res = f_mkdir((const TCHAR *)dstpathname);
                // 如果文件夹已经存在,就不创建.如果不存在就创建新的文件夹.
                if (res == FR_EXIST)
                {
                    res = 0;
                }
                while (res == 0) // 开始复制文件夹里面的东东
                {
					IWDG_Feed();
                    res = f_readdir(srcdir, finfo); // 读取目录下的一个文件
                    if (res != FR_OK || finfo->fname[0] == 0 || cnt>MAX_COYP_SINGLE)
                    {
                        break;
                    } // 错误了/到末尾了,退出
                    if (finfo->fname[0] == '.')
                    {
                        continue;
                    } // 忽略上级目录
                    fn = (u8 *)(*finfo->lfname ? finfo->lfname : finfo->fname);
                    // 得到文件名
                    dstpathlen = strlen((const char *)dstpathname); // 得到当前目标路径的长度
                    srcpathlen = strlen((const char *)srcpathname); // 得到源路径长度
                    strcat((char *)srcpathname, (const char *)"/"); // 源路径加斜杠
                    if (finfo->fattrib & 0X10)                      // 是子目录   文件属性,0X20,归档文件;0X10,子目录;
                    {
                        continue;
                    } // 忽略子目录
                    else // 非目录
                    {
                        strcat((char *)dstpathname, (const char *)"/"); // 目标路径加斜杠
                        strcat((char *)srcpathname, (const char *)fn);  // 源路径加文件名
                        //sprintf((char *)fn_t, "IMAG%05d.JPG", id_in_flash); 
						sprintf((char *)fn_t, "IMAG%04d.JPG", eerom.id_in_flash); 
						strcat((char *)dstpathname, (const char *)fn_t);  // 目标路径加文件名

                        printf("[INFO]copy file \"%s\" to \"%s\"\r\n", srcpathname,
                               dstpathname);                       // 拷贝文件
                        mf_copy(srcpathname, dstpathname, fwmode); // 复制文件
						
						//id_in_flash++;
						eerom.id_in_flash++;						
						//STMFLASH_Write_Num(FLASH_SAVE_ADDR, eerom.id_in_flash);
						STMFLASH_Write(FLASH_SAVE_ADDR,(u32 *)&eerom,sizeof(eerom)/4);
						//printf("[INFO]STMFLASH_Write|pic_id=%d,buf={%s}\r\n",eerom.id_in_flash,eerom.buf);
						printf("[INFO]STMFLASH_Write|pic_id=%d\r\n",eerom.id_in_flash);
						if(if_save==0)
						{
							mf_unlink(srcpathname);					   // 删除文件
							printf("[INFO]rm \"%s\",cnt=%d\r\n",srcpathname,cnt);
						}
						cnt++;
						
                    }
                    srcpathname[srcpathlen] = 0; // 加入结束符
                    dstpathname[dstpathlen] = 0; // 加入结束符
                }
            }
            myfree(SRAMIN, dstpathname);
            myfree(SRAMIN, srcpathname);
            myfree(SRAMIN, finfo->lfname);
			myfree(SRAMIN, fn_t);
        }
    }
    myfree(SRAMIN, srcdir);
    myfree(SRAMIN, dstdir);
    myfree(SRAMIN, finfo);
    return res;
}
/** 
 * @brief write data to assign dir
 * @param pdst	dst dir
 * @param data	data

 * @return 0 succeed
 */
u8 sensordata_write(u8 *pdst, u8 *data)
{
    u8 res;
	u16 br = 0;
    u16 bw = 0;
    FIL *fdst = 0;
    fdst = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    
	IWDG_Feed();
	
	if (fdst == NULL)
    {
        res = 100;
		printf("[WARNING]warming:sensordata_write|fail to mymalloc memery!\r\n");
    }
    else
    {
		res = f_open(fdst, (const TCHAR *)pdst, FA_WRITE | FA_OPEN_ALWAYS); 
        if (res == 0)
        {
			res = f_lseek(fdst, fdst ->fsize);
			if (res == 0)
			{
				// put the point to the end
				br = strlen((const char *)data);
				bw = f_puts((const char *)data,fdst);
				if (br!=bw)  // check if write ok
				{
					printf("[WARNING]sensordata_write|fail to write sensor data to SD card!\r\n");
					return 200;
				}
				else
				{
					printf("[LOG]sensordata_write|success write in %s\r\n",pdst);
				}
			}
            f_close(fdst);
        }
		else
		{
			printf("[WARNING]sensordata_write|fail to f_open %s\r\n",pdst);
		}
    }
    myfree(SRAMIN, fdst);
    return res;
}

/**
 * @description: 发送传感器数据
 * 不进出队列
 * @param {type} 
 * @return {type} 
 */
u8 sensordata_send(u8 *psrc)
{
    u8 res;
    //u16 br = 0;
    //u16 bw = 0;
    FIL *fsrc = 0;
    u8 *fbuf = 0;
    fsrc = (FIL *)mymalloc(SRAMIN, sizeof(FIL)); 
    fbuf = (u8 *)mymalloc(SRAMIN, 512);
    
	IWDG_Feed();
	
	if (fsrc == NULL || fbuf == NULL)
    {
        res = 100;
		printf("[WARNING]sensordata_send|fail to mymalloc memery!\r\n");
    }
    else
    {
        res = f_open(fsrc, (const TCHAR *)psrc, FA_READ | FA_OPEN_EXISTING);
        if (res == 0)
        {
			// 按行获取所有的待读数据
			while(f_gets((TCHAR*)fbuf, 512, fsrc))
			{
				printf("[INFO]sensordata_read:\"%s\"\r\n",(char*)fbuf);
				mysend_data((char*)fbuf);  // send sensor data
			}

            f_close(fsrc);
			f_unlink((const char*)psrc);  // delet sent data
        }
		else
		{
			printf("[WARNING]sensordata_send|fail to f_open %s\r\n",psrc);
		}
    }
    myfree(SRAMIN, fsrc); 
    myfree(SRAMIN, fbuf);
	printf("[LOG]sensordata_send res=%d\r\n",res);
    return res;
}


#define	 SENSOR_DATA_PATH 	"0:sensor.dat"
#define	 SENSOR_DATA_WIFI_PATH 	"0:sensor_wifi.dat"
u8 mf_sensordata_write(u8 *data, u8 device)
{
	if(device == 1)  //存到WiFi
		return sensordata_write((u8*)SENSOR_DATA_PATH, data);
	else  // 存到sd卡
		return sensordata_write((u8*)SENSOR_DATA_PATH, data);
}
u8 mf_sensordata_send(void)
{
	return sensordata_send((u8*)SENSOR_DATA_PATH);
}

static u8 send_log_flag=0;
#define	 LOG_DATA_PATH 	"0:log.dat"
u8 sd_ready_flag=0;
u8 mf_log_init(void)
{
	u8 res;	 
	res = mf_open((u8*)LOG_DATA_PATH,FA_WRITE | FA_OPEN_ALWAYS);
	if(res==0)
	{
		res = f_lseek(file, file ->fsize);
		if (res == 0)
		{
			sd_ready_flag =0xAA;
			if(send_log_flag==0)  // 第一次启动才需要把系统参数打印出来
			{
				printf("\r\n##################sd log data###############\r\n");
				//printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\r\n");
				printf("[LOG]Start logging the log data and write to file log.dat\r\n");
				calendar_get_time(&calendar);
				calendar_get_date(&calendar);
				printf("[TIME]%d/%d/%d %d:%d:%d\r\n",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
				send_log_flag++;
			}
			else
			{
				printf("******************sd log data***************\r\n");
				//printf("*****************************************\r\n");
				printf("[INFO]mf_log_init,time:%d\r\n" ,send_log_flag);
			}
		}
		
	}
	else
	{
		sd_ready_flag =0;
		printf("[WARNING]mf_log_init|res=%d\r\n",res);
	}
	return res;
}
void mf_send_log(void)
{
	MY_MQTT_ERR res;
	F407USART1_SendString("[INST]act:mf_send_log...\r\n");
	delay_ms(1000);
	delay_ms(1000);
	printf("[LOG]^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\r\n");
	mf_close();
	res = mysend_picture((u8*)LOG_DATA_PATH,0);
	if(res == MY_MQTT_ERR_NONE)
	{
		mf_unlink((u8*)LOG_DATA_PATH);	
		printf("[LOG]Succeed send log file\r\n");
	}
	else
	{
		printf("[WARNING]Fail send log file\r\n");
	}
	mf_log_init(); // 重新启动log记录
}


/**
 * @description: Send files by WiFi.
 * @param {type} 
 * @return {type} 
 */
u8 WiFiSendFileRaw(u8 *psrc)
{
	u8 res;

	FIL *fsrc = 0;
	u8 *fbuf = 0;
	u32 total = 0;
	u32 count = 0;

	//PrintProgressBarInit();

	fsrc = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
	fbuf = (u8 *)mymalloc(SRAMIN, 1024);

	if (fsrc == NULL || fbuf == NULL)
	{
		res = 100;
		printf("[WARNING]sensordata_send|fail to mymalloc memery!\r\n");
	}
	else
	{
		res = f_open(fsrc, (const TCHAR *)psrc, FA_READ | FA_OPEN_EXISTING);

		if (res == 0)
		{
			total = f_size(fsrc);
			printf("[INFO]WiFiSendFile:%s\r\n",psrc);
			printf("[INFO]File size:%d\r\n",total);
			while (res == 0)
			{
				res = f_read(fsrc, fbuf, SEND_DATA_MAX_SIZE, (UINT *)&br);
				printf("[INFO]send=%d,packet=%d,", res, br);
				if (res || br == 0)
				{
					break;
				}
				WiFiSendPacketBuffer(fbuf, br);
				count += br;
				
				printf("count=%d/total=%d[%2.2f%%]\r\n", total, count, (float)count*100/total);
				//PrintProgressBar(count, total);
			}
			f_close(fsrc);
		}
		else
		{
			printf("[WARNING]sensordata_send|fail to f_open %s\r\n", psrc);
		}
	}
	myfree(SRAMIN, fsrc);
	myfree(SRAMIN, fbuf);
	
	//PrintProgressBarEnd(count,total);
	printf("[LOG]WiFiSendFile count=%d/total=%d[%2.2f%%]\r\n", total, count, (float)count*100/total);
	return res;
}

#define	 TEST_FILE_NAME 	"0:pic1.jpg"
u8 WiFiSendPic(u8 *psrc, u32 myid)
{
	u8 res;
	char buf[50];

	res = M8266TransportOpen();	//建立链接
	if(res == M8266_ERROR)
	{
		printf("[WARNING]Fail M8266TransportOpen\r\n");
		return M8266_ERROR;
	}
	sprintf(buf,"%d.jpg",myid);
	printf("[INFO]WiFi send picture name id:%s\r\n",buf);
	WiFiSendPacketBuffer((u8*)buf,SEND_DATA_MAX_SIZE);  // 发送名字
	#if TEST_WIFI_SENDING_ON
	WiFiSendFileRaw(TEST_FILE_NAME);  // 发送图片
	#else
	WiFiSendFileRaw(psrc);  // 发送图片TEST_FILE_NAME
	#endif
	res = M8266TransportCLose();
	if(res == M8266_ERROR)
	{
		printf("[WARNING]Fail M8266TransportOpen\r\n");
		return M8266_ERROR;
	}
	
	return M8266_SUCCESS;
}

u8 WiFiSendFile(u8 *psrc, u32 myid)
{
	u8 res;
	char buf[50];

	res = M8266TransportOpen();	//建立链接
	if(res == M8266_ERROR)
	{
		printf("[WARNING]Fail M8266TransportOpen\r\n");
		return M8266_ERROR;
	}
	sprintf(buf,"%d.dat",myid);
	WiFiSendPacketBuffer((u8*)buf,SEND_DATA_MAX_SIZE);  // 发送名字
	WiFiSendFileRaw(psrc);  // 发送图片
	res = M8266TransportCLose();
	if(res == M8266_ERROR)
	{
		printf("[WARNING]Fail M8266TransportOpen\r\n");
		return M8266_ERROR;
	}
	
	return M8266_SUCCESS;
}

void mf_config_data_write_flash(u8 *data)
{
	printf("[LOG]mf_config_data_write_flash\r\n");
	STMFLASH_Read(FLASH_SAVE_ADDR,(u32 *)&eerom,sizeof(eerom)/4);  // 先读出原来存的内容，实际上是保存pic id
	printf("[INFO]STMFLASH_Read|pic_id=%d,buf={%s}\r\n",eerom.id_in_flash,eerom.buf);
	strcpy((char*)eerom.buf,(const char *)data);									 // 再将有效的数据写入到buf
	STMFLASH_Write(FLASH_SAVE_ADDR,(u32 *)&eerom,sizeof(eerom)/4); // 最后保存
	printf("[INFO]STMFLASH_Write|pic_id=%d,buf={%s}\r\n",eerom.id_in_flash,eerom.buf);
}


/**
 * @description: updata sys parameters
 *	打开SD开
 *	读取数据
 *	解析数据buf,len,flash
 * @param {type} 
 * @return {type} 
 */
void mf_config_data_read_flash(u8 *fbuf)
{
	STMFLASH_Read(FLASH_SAVE_ADDR,(u32 *)&eerom,sizeof(eerom)/4);
	//printf("*info:STMFLASH_Read|pic_id=%d,buf={%s}\r\n",eerom.id_in_flash,eerom.buf);
	printf("*info:STMFLASH_Read|pic_id=%d\r\n",eerom.id_in_flash);
	strcpy((char*)fbuf,(const char *)eerom.buf);
}
