/* This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <cutils/log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/reboot.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h> 
#include <sys/types.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <hardware_legacy/power.h>
#include <sys/statfs.h>
#define BUFF_SIZE 1048576 //1M
#define WRITE2NAND	1
#ifdef	WRITE2NAND
#define BUFFER_LEN		(128)
#define REQHEADINFOCOUNT	(3)
static int req_head_info_count = 0;
char gch;
FILE *gfp;
char gbuffer[BUFFER_LEN];
int ghour, gminute, gsecond;
time_t  gtimep;
struct tm   *gp;
#endif
#ifdef WRITE2NAND
#define LOCAL_LOG(format,args...) do{\
			struct timeval now_time = {0};\
			gettimeofday(&now_time,(struct timezone*)0);\
			memset(gbuffer,0,BUFFER_LEN);\
			sprintf(gbuffer,"fuction: %s, line: %d, "format", %lu(us), %s", \
			__func__,__LINE__,##args,now_time.tv_usec,ctime(&now_time.tv_sec));\
			fputs(gbuffer,gfp);\
			fflush(gfp);\
		}while(0);
#else
#define LOCAL_LOG(format,args...) 
#endif

#ifdef	WRITE2NAND
void write_time(void)
{
	time(&gtimep);
  	gp = localtime(&gtimep);
	memset(gbuffer, 0, BUFFER_LEN);
	ghour = gp->tm_hour;
	gminute = gp->tm_min;
	gsecond = gp->tm_sec;
	sprintf(gbuffer, "\nH:%02d  M:%02d  S:%02d\n", ghour, gminute, gsecond);
	fputs(gbuffer, gfp);
	fflush(gfp);
}

char select_log_file(void)
{
	FILE *fp;
	char ch;

	fp = fopen("/data/index_emmc.txt", "r+");
	if (fp == NULL) {
		printf("/data/index_emmc.txt is not exist, create it\n");
		fp = fopen("/data/index_emmc.txt", "w+");
		if (fp == NULL) {
			printf("can not creat /data/index_emmc.txt\n");
			return 0;
		} else {
			fputc('1', fp);
			fclose(fp);
			return '1';
		}
	}
	printf("/data/index_emmc.txt is exist\n");
	fseek(fp, 0, SEEK_SET);
	ch = fgetc(fp);
	ch ++;
	//if (ch == '6')
	//	ch = '1';
	fseek(fp, 0, SEEK_SET);
	fputc(ch, fp);
	fclose(fp);

	return ch;
}
#endif

#ifdef WRITE2NAND
static int open_logfile(void){
	int ret = 0;
	gch = select_log_file();
	gfp = NULL;
	if (gch > 100){
		remove("/data/mem_test.txt");
		gfp = fopen("/data/mem_test.txt", "a+");
	}
	else if(gch>=1 && gch <=100)
		gfp = fopen("/data/mem_test.txt", "a+");
	else
		printf("nothing is selected\n");
	
	if (gfp == NULL)
		ret = 1;

	return ret;
}
#endif
char* get_free_file(void)
{
   	int	fd, fileSize;
    char*  	buf;


	fd = open("/data/free.txt", O_RDONLY);
	if (fd < 0) {
		LOCAL_LOG("can not open /data/mtd.txt\n");
    	return 0;
	}

	fileSize = lseek(fd, 0, SEEK_END);
	if (fileSize < 0) {
		LOCAL_LOG("fileSize is error\n");
		return 0;
	}
	//LOGE("file size %d \n", fileSize);

	buf = (char*) malloc((fileSize) + 1);
	if (buf == 0) {
		LOCAL_LOG("Malloc buffer failed\n");
		return 0;
	}

	if (lseek(fd, 0, SEEK_SET) < 0) {
		LOCAL_LOG("lseek header error\n");
		return 0;
	}

	if (read(fd, buf, fileSize) != fileSize) {
    	free(buf);
		buf = 0;
	} else
		buf[fileSize] = 0;

	printf("buf = [%s]\n", buf);
	return buf;

}
static void set_random_data(unsigned char *buf ,size_t len) {
	static int num = 0;
	if(0 == num) {
		memset(buf,0x5a,len) ;
		num = 1;
	} else {
		memset(buf,0xa5,len) ;
		num = 0;
	}
}
long long get_free_size(char *s)
{
	struct statfs st;
	long long nandsize;
	if (statfs(s, &st) < 0) {
		printf("fileSize is error\n");
    } else {
        if (st.f_blocks == 0 )
            return 0;
       nandsize = (long long)st.f_bfree * (long long)st.f_bsize;
    } 
	return nandsize ;
}
int main(int argc , char * argv[])
{

	long long loop,fd,fd2,size,i,j,k,l;
	long long freesize ;
	long long cycle    = atoi(argv[1]);
	long long FileNum  = atoi(argv[2]);
	long long filesize = atoi(argv[3]);
	long long TotalSize = 0;
	struct stat stat_size;
    int write_flag = 0 ;
	long long cyc_num  =0;
	ssize_t ret = 0; 
	unsigned char * buf;
	unsigned char *verify;
   	buf    = malloc(BUFF_SIZE);
	verify = malloc(BUFF_SIZE);
	if(!buf || !verify){
	    LOCAL_LOG("not enough memory for splash image\n");
	    exit(1);
	}

#ifdef WRITE2NAND
	if(open_logfile())
		return 0;
#endif

	if(access("/data/test",0)==-1)//access函数是查看是不是存在
	{
	   if (mkdir("/data/test",0777))//如果不存在就用mkdir函数来创建
		{
		   LOCAL_LOG("creat file test failed!");
		   exit(1);
		}
	}
	else//如果存在不进行处理
	{
		system("rm /data/test/*.txt");
		system("rm -r /data/test/");
	   if (mkdir("/data/test",0777))//如果不存在就用mkdir函数来创建
		{
		   LOCAL_LOG("creat file test failed!");
			exit(1);
		}
	} 
	freesize = get_free_size("/data"); //get data part Free size

	TotalSize = filesize * FileNum ;

   	
	/* 
	 * It's nend n cycle writed if file size  very very big 
	 */
	freesize = freesize - freesize/2 ;


	if(TotalSize > freesize) {
			filesize = freesize / FileNum;
	}

	if(filesize > BUFF_SIZE)
	{
		    cyc_num  = filesize % BUFF_SIZE ;
		if(0 != cyc_num )
			cyc_num = filesize / BUFF_SIZE + 1;
		else
			cyc_num = filesize / BUFF_SIZE ;
	}
	//memset (buf, 0x5a, BUFF_SIZE);
	size = filesize;
	loop = cycle ;

while(loop){
	for(i=FileNum ; i > 0 ; i--) {
	    char src[125];
		//seting file name
	    sprintf(src, "/data/test/%d.txt" , i);
		set_random_data(buf,BUFF_SIZE); //set temp buf
	if(cyc_num) {
		size = BUFF_SIZE ; //size need modify when filesize % BUFF_SIZE != 0
		for(l = 0; l < cyc_num ; l++ ){ //write filesize / BUFF_SIZE BUFF_SIZE data
		    fd=open(src,O_RDWR | O_CREAT | O_APPEND,00777);
		    if(fd == -1){
		        LOCAL_LOG("open src fail !!!\n");
		        exit(1) ;
		    }
		    off_t pos = lseek(fd, 0, SEEK_END);
		    if ((off_t) pos == (off_t) -1) {
	                goto exit ;
	            }
		    if (write(fd, buf, size) != size) {
                write_flag ++;
		        LOCAL_LOG(" write error ; write_flag=%d!!!!! \n",write_flag);
		    }
		    fsync(fd);
		    close(fd);
		}			
	} else {
        //open write file
	    fd=open(src,O_RDWR | O_CREAT | O_APPEND,00777);
	    if(fd == -1){
	        LOCAL_LOG("open src fail !!!\n");
	        exit(1) ;
	    }
	    off_t pos = lseek(fd, 0, SEEK_END);
	    if ((off_t) pos == (off_t) -1) {
                goto exit ;
            }
	    if (write(fd, buf, size) != size) {
                write_flag ++;
	        LOCAL_LOG(" write error ; write_flag=%d!!!!! \n",write_flag);
	    }
	    fsync(fd);
	    //close(fd);

		size = filesize;
		ret = 0;
		pos = lseek(fd , 0 , SEEK_SET);
		if ((off_t) pos == (off_t) -1) {
              goto   exit;
        }

		while(size != 0 && (ret = read(fd, verify ,size)) != size) {
	    	if(ret == -1) {
				if (errno == EINTR)
		    	continue ;
		    	perror("write");
		    	break ;
			}
		   	size    -= ret ;
	  	 	verify  += ret ;
		}
		close(fd);
		if (memcmp(buf, verify, filesize) != 0) {
			LOCAL_LOG(" verification compare error");
		}

	}
        if( write_flag > 0 ) {
			LOCAL_LOG("write  error ...");
			free(verify);
			free(buf);
			sync();
			reboot(LINUX_REBOOT_CMD_POWER_OFF);
        }
		if(get_free_size("/data") < 20000000){
			break ;			
		}
}
	sync();
	for(j=0; j < 0x10000 ; j++);

    system("rm /data/test/*.txt");
    sync();

	//printf("test sucessfull loop=%lld\n",loop);
	if (stat("/data/mem_test.txt",&stat_size) < 0) {
        LOCAL_LOG("Mesg: %s\n", strerror(errno));
            exit(-1);
  	}
	if(stat_size.st_size >( 1024*1000)){
	    if(remove("/data/mem_test.txt"))
		LOCAL_LOG("Could not delete the file");
	    if(open_logfile()){
		LOCAL_LOG("reopen file !\n");
		return 0;
	    }
	}
	/*if(loop == 1) //做无限循环
			loop = cycle ;
		else
			loop-- ;
	*/
}

	if(loop <= 0) {
		printf("test sucessfull loop=%lld",loop);
		LOCAL_LOG("test sucessfull loop=%lld",loop);
	}else{
		LOCAL_LOG("test error loop=%d",loop);
	}
exit:
    close(fd);
	fclose(gfp);
	free(buf);
	free(verify);
	exit(0);
}
