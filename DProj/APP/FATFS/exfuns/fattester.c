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

//Ϊ����ע�Ṥ����	 
//path:����·��������"0:"��"1:"
//mt:0��������ע�ᣨ�Ժ�ע�ᣩ��1������ע��
//����ֵ:ִ�н��
u8 mf_mount(u8* path,u8 mt)
{		   
	return f_mount(fs[2],(const TCHAR*)path,mt); 
}
//��·���µ��ļ�
//path:·��+�ļ���
//mode:��ģʽ
//����ֵ:ִ�н��
u8 mf_open(u8*path,u8 mode)
{
	u8 res;	 
	res=f_open(file,(const TCHAR*)path,mode);//���ļ���
	return res;
} 
//�ر��ļ�
//����ֵ:ִ�н��
u8 mf_close(void)
{
	sd_ready_flag =0;  // �رձ�־λ
	delay_ms(10);
	//OS_ERR err;
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER();	//�����ٽ���
	f_close(file);
	OS_CRITICAL_EXIT();	//�˳��ٽ���
	return 0;
}
//ͬ���ļ�
//����ֵ:ִ�н��
u8 mf_sync(void)
{
	//OS_ERR err;
	printf("*******************syn log****************\r\n");
	CPU_SR_ALLOC();
	OS_CRITICAL_ENTER();	//�����ٽ���
	f_sync(file);
	OS_CRITICAL_EXIT();	//�˳��ٽ���
	return 0;
}


//��������
//len:�����ĳ���
//����ֵ:ִ�н��
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
		if(res)	//�����ݳ�����
		{
			printf("\r\nRead Error:%d\r\n",res);   
		}else
		{
			tlen+=br;
			for(t=0;t<br;t++)printf("%c",fatbuf[t]); 
		}	 
	}
	if(tlen)printf("\r\nReaded data len:%d\r\n",tlen);//���������ݳ���
	printf("Read data over\r\n");	 
	return res;
}
//д������
//dat:���ݻ�����
//len:д�볤��
//����ֵ:ִ�н��
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

//��Ŀ¼
 //path:·��
//����ֵ:ִ�н��
u8 mf_opendir(u8* path)
{
	return f_opendir(&dir,(const TCHAR*)path);	
}


/**
 * @description: ��鲢����Ŀ¼
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
//�ر�Ŀ¼ 
//����ֵ:ִ�н��
u8 mf_closedir(void)
{
	return f_closedir(&dir);	
}
//���ȡ�ļ���
//����ֵ:ִ�н��
u8 mf_readdir(void)
{
	u8 res;
	char *fn;			 
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mymalloc(SRAMIN,fileinfo.lfsize);
#endif		  
	res=f_readdir(&dir,&fileinfo);//��ȡһ���ļ�����Ϣ
	if(res!=FR_OK||fileinfo.fname[0]==0)
	{
		myfree(SRAMIN,fileinfo.lfname);
		return res;//������.
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

 //�����ļ�
 //path:·��
 //����ֵ:ִ�н��
u8 mf_scan_files(u8 * path)
{
	FRESULT res;	  
    char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mymalloc(SRAMIN,fileinfo.lfsize);
#endif		  
	printf("[INFO]scan files path:<%s>, the details are as follows:\r\n",path);
    res = f_opendir(&dir,(const TCHAR*)path); //��һ��Ŀ¼
    if (res == FR_OK) 
	{	
		// printf("\r\n"); 
		while(1)
		{
	        res = f_readdir(&dir, &fileinfo);                   //��ȡĿ¼�µ�һ���ļ�
	        if (res != FR_OK || fileinfo.fname[0] == 0) break;  //������/��ĩβ��,�˳�
	        //if (fileinfo.fname[0] == '.') continue;             //�����ϼ�Ŀ¼
#if _USE_LFN
        	fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else							   
        	fn = fileinfo.fname;
#endif	                                              /* It is a file. */
			printf("%s/", path);  // ��ӡ·��	
			printf("%s\r\n",  fn);  // ��ӡ�ļ���	  
		} 
		printf("[LOG]Above are all the directories.\r\n");
    }	  
	myfree(SRAMIN,fileinfo.lfname);
    return res;	  
}
//��ʾʣ������
//drv:�̷�
//����ֵ:ʣ������(�ֽ�)
u32 mf_showfree(u8 *drv)
{
	FATFS *fs1;
	u8 res;
    u32 fre_clust=0, fre_sect=0, tot_sect=0;
    //�õ�������Ϣ�����д�����
    res = f_getfree((const TCHAR*)drv,(DWORD*)&fre_clust, &fs1);
    if(res==0)
	{											   
	    tot_sect = (fs1->n_fatent - 2) * fs1->csize;//�õ���������
	    fre_sect = fre_clust * fs1->csize;			//�õ�����������	   
#if _MAX_SS!=512
		tot_sect*=fs1->ssize/512;
		fre_sect*=fs1->ssize/512;
#endif	  
		if(tot_sect<20480)//������С��10M
		{
		    /* Print free space in unit of KB (assuming 512 bytes/sector) */
		    printf("\r\n����������:%d KB\r\n"
		           "���ÿռ�:%d KB\r\n",
		           tot_sect>>1,fre_sect>>1);
		}else
		{
		    /* Print free space in unit of KB (assuming 512 bytes/sector) */
		    printf("\r\n����������:%d MB\r\n"
		           "���ÿռ�:%d MB\r\n",
		           tot_sect>>11,fre_sect>>11);
		}
	}
	return fre_sect;
}		    
//�ļ���дָ��ƫ��
//offset:����׵�ַ��ƫ����
//����ֵ:ִ�н��.
u8 mf_lseek(u32 offset)
{
	return f_lseek(file,offset);
}
//��ȡ�ļ���ǰ��дָ���λ��.
//����ֵ:λ��
u32 mf_tell(void)
{
	return f_tell(file);
}
//��ȡ�ļ���С
//����ֵ:�ļ���С
u32 mf_size(void)
{
	return f_size(file);
} 
//����Ŀ¼
//pname:Ŀ¼·��+����
//����ֵ:ִ�н��
u8 mf_mkdir(u8*pname)
{
	return f_mkdir((const TCHAR *)pname);
}
//��ʽ��
//path:����·��������"0:"��"1:"
//mode:ģʽ
//au:�ش�С
//����ֵ:ִ�н��
u8 mf_fmkfs(u8* path,u8 mode,u16 au)
{
	return f_mkfs((const TCHAR*)path,mode,au);//��ʽ��,drv:�̷�;mode:ģʽ;au:�ش�С
} 
//ɾ���ļ�/Ŀ¼
//pname:�ļ�/Ŀ¼·��+����
//����ֵ:ִ�н��
u8 mf_unlink(u8 *pname)
{
	return  f_unlink((const TCHAR *)pname);
}

//�޸��ļ�/Ŀ¼����(���Ŀ¼��ͬ,�������ƶ��ļ�Ŷ!)
//oldname:֮ǰ������
//newname:������
//����ֵ:ִ�н��
u8 mf_rename(u8 *oldname,u8* newname)
{
	return  f_rename((const TCHAR *)oldname,(const TCHAR *)newname);
}
//��ȡ�̷����������֣�
//path:����·��������"0:"��"1:"  
void mf_getlabel(u8 *path)
{
	u8 buf[20];
	u32 sn=0;
	u8 res;
	res=f_getlabel ((const TCHAR *)path,(TCHAR *)buf,(DWORD*)&sn);
	if(res==FR_OK)
	{
		printf("\r\n����%s ���̷�Ϊ:%s\r\n",path,buf);
		printf("����%s �����к�:%X\r\n\r\n",path,sn); 
	}else printf("\r\n��ȡʧ�ܣ�������:%X\r\n",res);
}
//�����̷����������֣����11���ַ�������֧�����ֺʹ�д��ĸ����Լ����ֵ�
//path:���̺�+���֣�����"0:ALIENTEK"��"1:OPENEDV"  
void mf_setlabel(u8 *path)
{
	u8 res;
	res=f_setlabel ((const TCHAR *)path);
	if(res==FR_OK)
	{
		printf("\r\n�����̷����óɹ�:%s\r\n",path);
	}else printf("\r\n�����̷�����ʧ�ܣ�������:%X\r\n",res);
} 

//���ļ������ȡһ���ַ���
//size:Ҫ��ȡ�ĳ���
void mf_gets(u16 size)
{
 	TCHAR* rbuf;
	rbuf=f_gets((TCHAR*)fatbuf,size,file);
	if(*rbuf==0)return  ;//û�����ݶ���
	else
	{
		printf("\r\nThe String Readed Is:%s\r\n",rbuf);  	  
	}			    	
}
//��Ҫ_USE_STRFUNC>=1
//дһ���ַ����ļ�
//c:Ҫд����ַ�
//����ֵ:ִ�н��
u8 mf_putc(u8 c)
{
	return f_putc((TCHAR)c,file);
}
//д�ַ������ļ�
//c:Ҫд����ַ���
//����ֵ:д����ַ�������
u8 mf_puts(u8*c)
{
	return f_puts((TCHAR*)c,file);
}

// �ܹ��Զ���ȡ�ַ����е�����
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
 * �����������ļ�������ת�棬����ֱ��ɾ���ˡ�
 * ��Ϊ�ļ���ɾ�������Ҳ�Ͳ�����check����ʧ�ܵ������ˡ�
 * @param psrc	Դ�ļ���
 * @param pdst	�鵵�ļ���
 * @param fwmode	
 *		<>1<> ����ģʽ
 *		<>0<> �Ǹ���ģʽ
 *
 * @return num of files
 * @note  max 20
 */
#define MAX_TRANSFORM_SINGLE 6
u8 mf_send_pics(u8 *psrc, u8 *pdst, u8 fwmode, u8 device)
{
	#define MAX_PATHNAME_DEPTH 100 + 1 //���Ŀ���ļ�·��+�ļ������
	u8 cnt = 0;
	
	u8 res = 0;
    DIR *srcdir = 0;    // ԴĿ¼
    DIR *dstdir = 0;    // ԴĿ¼
    FILINFO *finfo = 0; // �ļ���Ϣ
    u8 *fn = 0;         // ���ļ���

    u8 *dstpathname = 0; // Ŀ���ļ���·��+�ļ���
    u8 *srcpathname = 0; // Դ�ļ���·��+�ļ���

    u16 dstpathlen = 0; // Ŀ��·������
    u16 srcpathlen = 0; // Դ·������
	u32 id_pic;

    srcdir = (DIR *)mymalloc(SRAMIN, sizeof(DIR)); // �����ڴ�
    dstdir = (DIR *)mymalloc(SRAMIN, sizeof(DIR));
    finfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO));

    if (srcdir == NULL || dstdir == NULL || finfo == NULL)
    {
        res = 100;
    }
    if (res == 0)
    {
        finfo->lfsize = _MAX_LFN * 2 + 1;
        finfo->lfname = mymalloc(SRAMIN, finfo->lfsize); // �����ڴ�
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
            strcat((char *)srcpathname, (const char *)psrc); // ����ԭʼԴ�ļ�·��
            strcat((char *)dstpathname, (const char *)pdst); // ����ԭʼĿ���ļ�·��
            res = f_opendir(srcdir, (const TCHAR *)psrc);    // ��ԴĿ¼
            if (res == 0)                                    // ��Ŀ¼�ɹ�
            {
                res = f_mkdir((const TCHAR *)dstpathname);
                // ����ļ����Ѿ�����,�Ͳ�����.��������ھʹ����µ��ļ���.
                if (res == FR_EXIST)
                {
                    res = 0;
                }
                while (res == 0)   // ��ʼ�����ļ�������Ķ���
                {
                    res = f_readdir(srcdir, finfo);  // ��ȡĿ¼�µ�һ���ļ�
                    if (res != FR_OK || finfo->fname[0] == 0 || cnt > MAX_TRANSFORM_SINGLE)
                    {
                        break;
                    } // ������/��ĩβ��,�˳�
                    if (finfo->fname[0] == '.')
                    {
                        continue;
                    } // �����ϼ�Ŀ¼
                    
					// �����������Ψһ�ģ�Ҫͨ����д��ʱ���иı�
					fn = (u8 *)(*finfo->lfname ? finfo->lfname : finfo->fname);
					id_pic=srt2integer_pro(fn);
                    // �õ��ļ���
                    dstpathlen = strlen((const char *)dstpathname); // �õ���ǰĿ��·���ĳ���
                    srcpathlen = strlen((const char *)srcpathname); // �õ�Դ·������

                    strcat((char *)srcpathname, (const char *)"/"); // Դ·����б��
                    if (finfo->fattrib & 0X10)                      // ����Ŀ¼   �ļ�����,0X20,�鵵�ļ�;0X10,��Ŀ¼;
                    {
                        continue;
                    }    // ������Ŀ¼
                    else // ��Ŀ¼
                    {
						u8 sta;
                        strcat((char *)dstpathname, (const char *)"/"); // Ŀ��·����б��
                        strcat((char *)dstpathname, (const char *)fn);  // Ŀ��·�����ļ���
                        strcat((char *)srcpathname, (const char *)fn);  // Դ·�����ļ���
                        printf("[INFO]]open file %s(src) and %s(dst)\r\n", srcpathname, dstpathname);
						//CPU_SR_ALLOC();
						//OS_CRITICAL_ENTER(); //�����ٽ���
						IWDG_Feed();//ι��
						if(device==1)  // wifi
							sta = WiFiSendFile(srcpathname,id_pic);	   // �����ļ�
						else  // 4G
							sta = mysend_picture(srcpathname, id_pic);	   // �����ļ�
						//OS_CRITICAL_EXIT();  
                        //mf_copy(srcpathname, dstpathname, fwmode); // �����ļ�
						mf_unlink(srcpathname);					     // ɾ���ļ�
						printf("[INFO]rm %s\r\n",srcpathname);
						cnt++;
						if(sta != 0) {
							printf("[WARNING]ERROR mysend_picture:%s,%d\r\n",srcpathname,id_pic);
							break;
						}
                    }
                    srcpathname[srcpathlen] = 0; // ���������
                    dstpathname[dstpathlen] = 0; // ���������
					
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


// �ļ�����
// ��psrc�ļ�,copy��pdst.
// psrc,pdst:Դ�ļ���Ŀ���ļ�
// fwmode:�ļ�д��ģʽ
// 0:������ԭ�е��ļ�
// 1:����ԭ�е��ļ�
u8 mf_copy(u8 *psrc, u8 *pdst, u8 fwmode)
{
    u8 res;
    u16 br = 0;
    u16 bw = 0;
    FIL *fsrc = 0;
    FIL *fdst = 0;
    u8 *fbuf = 0;
    fsrc = (FIL *)mymalloc(SRAMIN, sizeof(FIL)); // �����ڴ�
    fdst = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    fbuf = (u8 *)mymalloc(SRAMIN, 512);
    
	//ι��
	IWDG_Feed();//ι��
	//
	
	if (fsrc == NULL || fdst == NULL || fbuf == NULL)
    {
        res = 100;
    } // ǰ���ֵ����fatfs
    else
    {
        if (fwmode == 0)
        {
            fwmode = FA_CREATE_NEW;
        } // ������
        else
        {
            fwmode = FA_CREATE_ALWAYS;
        } // ���Ǵ��ڵ��ļ�

        res = f_open(fsrc, (const TCHAR *)psrc, FA_READ | FA_OPEN_EXISTING);
        // ��ֻ���ļ�
        if (res == 0)
        {
            res = f_open(fdst, (const TCHAR *)pdst, FA_WRITE | fwmode);
        }             // ��һ���򿪳ɹ�,�ſ�ʼ�򿪵ڶ���
        if (res == 0) // �������򿪳ɹ���
        {
            while (res == 0) // ��ʼ����
            {
                res = f_read(fsrc, fbuf, 512, (UINT *)&br); // Դͷ����512�ֽ�
                if (res || br == 0)
                {
                    break;
                }
                res = f_write(fdst, fbuf, (UINT)br, (UINT *)&bw); // д��Ŀ���ļ�
                if (res || bw < br)
                {
                    break;
                }
            }
            f_close(fsrc);
            f_close(fdst);
        }
    }
    myfree(SRAMIN, fsrc); // �ͷ��ڴ�
    myfree(SRAMIN, fdst);
    myfree(SRAMIN, fbuf);
    return res;
}

// �õ�·���µ��ļ���
// ����ֵ:0,·�����Ǹ�����.
// ����,�ļ��������׵�ַ
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
		temp--;	//׷����������һ��"\"����"/"�� 
		printf("dpfn[%d]=%c\r\n",temp,*dpfn);
	}
	printf("dpfn=%d\r\n",++temp);
	return ++dpfn;
}


#define MAX_COYP_SINGLE 6
// �ļ��и���
// ��psrc�ļ���,copy��pdst�ļ���.
// pdst:��������"X:"/"X:XX"/"X:XX/XX"֮���.����Ҫʵ��ȷ����һ���ļ��д���
// psrc,pdst:Դ�ļ��к�Ŀ���ļ���
// fwmode:�ļ�д��ģʽ
// 0:������ԭ�е��ļ�
// 1:����ԭ�е��ļ�
u8 mf_dcopy(u8 *psrc, u8 *pdst, u8 fwmode, u8 if_save)
{
#define MAX_PATHNAME_DEPTH 100 + 1 //���Ŀ���ļ�·��+�ļ������
    u8 cnt=0;
	
	u8 res = 0;
    DIR *srcdir = 0;    // ԴĿ¼
    DIR *dstdir = 0;    // ԴĿ¼
    FILINFO *finfo = 0; // �ļ���Ϣ
    u8 *fn = 0;         // ���ļ���

    u8 *dstpathname = 0; // Ŀ���ļ���·��+�ļ���
    u8 *srcpathname = 0; // Դ�ļ���·��+�ļ���
	
	u8 *fn_t = 0; // #speicalĿ���ļ���·��+�ļ���

    u16 dstpathlen = 0; // Ŀ��·������
    u16 srcpathlen = 0; // Դ·������

	//u16 id_in_flash; // #special
	STMFLASH_Read(FLASH_SAVE_ADDR,(u32 *)&eerom,sizeof(eerom)/4);
	//printf("[INFO]STMFLASH_Read|pic_id=%d,buf={%s}\r\n",eerom.id_in_flash,eerom.buf);
	printf("\r\n[INFO]STMFLASH_Read|pic_id=%d\r\n",eerom.id_in_flash);
	//eerom.id_in_flash = STMFLASH_Read_Num(FLASH_SAVE_ADDR); // ##special
	//id_in_flash = STMFLASH_Read_Num(FLASH_SAVE_ADDR); // ##special
	srcdir = (DIR *)mymalloc(SRAMIN, sizeof(DIR)); // �����ڴ�
    dstdir = (DIR *)mymalloc(SRAMIN, sizeof(DIR));
    finfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO));

    if (srcdir == NULL || dstdir == NULL || finfo == NULL)
    {
        res = 100;
    }
    if (res == 0)
    {
        finfo->lfsize = _MAX_LFN * 2 + 1;
        finfo->lfname = mymalloc(SRAMIN, finfo->lfsize); // �����ڴ�
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
            strcat((char *)srcpathname, (const char *)psrc); // ����ԭʼԴ�ļ�·��
            strcat((char *)dstpathname, (const char *)pdst); // ����ԭʼĿ���ļ�·��
            res = f_opendir(srcdir, (const TCHAR *)psrc); // ��ԴĿ¼
            if (res == 0)                                 // ��Ŀ¼�ɹ�
            {
                res = f_mkdir((const TCHAR *)dstpathname);
                // ����ļ����Ѿ�����,�Ͳ�����.��������ھʹ����µ��ļ���.
                if (res == FR_EXIST)
                {
                    res = 0;
                }
                while (res == 0) // ��ʼ�����ļ�������Ķ���
                {
					IWDG_Feed();
                    res = f_readdir(srcdir, finfo); // ��ȡĿ¼�µ�һ���ļ�
                    if (res != FR_OK || finfo->fname[0] == 0 || cnt>MAX_COYP_SINGLE)
                    {
                        break;
                    } // ������/��ĩβ��,�˳�
                    if (finfo->fname[0] == '.')
                    {
                        continue;
                    } // �����ϼ�Ŀ¼
                    fn = (u8 *)(*finfo->lfname ? finfo->lfname : finfo->fname);
                    // �õ��ļ���
                    dstpathlen = strlen((const char *)dstpathname); // �õ���ǰĿ��·���ĳ���
                    srcpathlen = strlen((const char *)srcpathname); // �õ�Դ·������
                    strcat((char *)srcpathname, (const char *)"/"); // Դ·����б��
                    if (finfo->fattrib & 0X10)                      // ����Ŀ¼   �ļ�����,0X20,�鵵�ļ�;0X10,��Ŀ¼;
                    {
                        continue;
                    } // ������Ŀ¼
                    else // ��Ŀ¼
                    {
                        strcat((char *)dstpathname, (const char *)"/"); // Ŀ��·����б��
                        strcat((char *)srcpathname, (const char *)fn);  // Դ·�����ļ���
                        //sprintf((char *)fn_t, "IMAG%05d.JPG", id_in_flash); 
						sprintf((char *)fn_t, "IMAG%04d.JPG", eerom.id_in_flash); 
						strcat((char *)dstpathname, (const char *)fn_t);  // Ŀ��·�����ļ���

                        printf("[INFO]copy file \"%s\" to \"%s\"\r\n", srcpathname,
                               dstpathname);                       // �����ļ�
                        mf_copy(srcpathname, dstpathname, fwmode); // �����ļ�
						
						//id_in_flash++;
						eerom.id_in_flash++;						
						//STMFLASH_Write_Num(FLASH_SAVE_ADDR, eerom.id_in_flash);
						STMFLASH_Write(FLASH_SAVE_ADDR,(u32 *)&eerom,sizeof(eerom)/4);
						//printf("[INFO]STMFLASH_Write|pic_id=%d,buf={%s}\r\n",eerom.id_in_flash,eerom.buf);
						printf("[INFO]STMFLASH_Write|pic_id=%d\r\n",eerom.id_in_flash);
						if(if_save==0)
						{
							mf_unlink(srcpathname);					   // ɾ���ļ�
							printf("[INFO]rm \"%s\",cnt=%d\r\n",srcpathname,cnt);
						}
						cnt++;
						
                    }
                    srcpathname[srcpathlen] = 0; // ���������
                    dstpathname[dstpathlen] = 0; // ���������
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
 * @description: ���ʹ���������
 * ����������
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
			// ���л�ȡ���еĴ�������
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
	if(device == 1)  //�浽WiFi
		return sensordata_write((u8*)SENSOR_DATA_PATH, data);
	else  // �浽sd��
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
			if(send_log_flag==0)  // ��һ����������Ҫ��ϵͳ������ӡ����
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
	mf_log_init(); // ��������log��¼
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

	res = M8266TransportOpen();	//��������
	if(res == M8266_ERROR)
	{
		printf("[WARNING]Fail M8266TransportOpen\r\n");
		return M8266_ERROR;
	}
	sprintf(buf,"%d.jpg",myid);
	printf("[INFO]WiFi send picture name id:%s\r\n",buf);
	WiFiSendPacketBuffer((u8*)buf,SEND_DATA_MAX_SIZE);  // ��������
	#if TEST_WIFI_SENDING_ON
	WiFiSendFileRaw(TEST_FILE_NAME);  // ����ͼƬ
	#else
	WiFiSendFileRaw(psrc);  // ����ͼƬTEST_FILE_NAME
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

	res = M8266TransportOpen();	//��������
	if(res == M8266_ERROR)
	{
		printf("[WARNING]Fail M8266TransportOpen\r\n");
		return M8266_ERROR;
	}
	sprintf(buf,"%d.dat",myid);
	WiFiSendPacketBuffer((u8*)buf,SEND_DATA_MAX_SIZE);  // ��������
	WiFiSendFileRaw(psrc);  // ����ͼƬ
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
	STMFLASH_Read(FLASH_SAVE_ADDR,(u32 *)&eerom,sizeof(eerom)/4);  // �ȶ���ԭ��������ݣ�ʵ�����Ǳ���pic id
	printf("[INFO]STMFLASH_Read|pic_id=%d,buf={%s}\r\n",eerom.id_in_flash,eerom.buf);
	strcpy((char*)eerom.buf,(const char *)data);									 // �ٽ���Ч������д�뵽buf
	STMFLASH_Write(FLASH_SAVE_ADDR,(u32 *)&eerom,sizeof(eerom)/4); // ��󱣴�
	printf("[INFO]STMFLASH_Write|pic_id=%d,buf={%s}\r\n",eerom.id_in_flash,eerom.buf);
}


/**
 * @description: updata sys parameters
 *	��SD��
 *	��ȡ����
 *	��������buf,len,flash
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
