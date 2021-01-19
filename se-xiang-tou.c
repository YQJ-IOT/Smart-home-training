#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>           
#include <fcntl.h>            
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>        
#include <linux/videodev2.h>

/*此片代码从csdn上copy的,替换掉dev_name后直接编译就能运行*/


struct buffer {  //用户层映射地址结构体
        void *                  start;
        size_t                  length;
};

struct buffer *buffers; //用户映射地址
unsigned long  n_buffers;
unsigned long file_length;
 
int file_fd; //存放摄像头文件描述符
char *dev_name = "/dev/v4l/by-id/usb-Guillemot_Corporation_USB_Camera-video-index0";   //此处需要根据自己的摄像头节点名填写
int fd; //存放照片文件描述符
 
//读取一幅图像
static int read_frame (void)
{
     struct v4l2_buffer buf; 
     
     /*帧出列*/
     buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     buf.memory = V4L2_MEMORY_MMAP; 
	 /*把数据放入缓冲队列,取出帧缓冲*/
     ioctl (fd, VIDIOC_DQBUF, &buf);
	  
	 /*将获取到的数据存储在照片文件,文件大小 = 4*buffers[buf.index]字节*/
 
  if(buffers[buf.index].length ==  write(file_fd,buffers[buf.index].start,buffers[buf.index].length))
			

	 /*buf入列,帧缓冲入队列，重新入队*/
     ioctl(fd, VIDIOC_QBUF, &buf);

     return 1;
}
 
int main (int argc,char ** argv)
{
     //printf("进入摄像头\n"); 
     struct v4l2_capability cap;
     struct v4l2_format fmt;
     struct v4l2_requestbuffers req;
     struct v4l2_buffer buf; 
     unsigned int i;
     enum v4l2_buf_type type;
     
     
     /*1:打开摄像头文件*/
     fd = open (dev_name, O_RDWR | O_NONBLOCK, 0);
 
     /*2:获取驱动信息*/
      ioctl (fd, VIDIOC_QUERYCAP, &cap);   //驱动信息保存在cap结构体
      printf("Driver Name:%s\n Card Name:%s\n Bus info:%s\n\n",cap.driver,cap.card,cap.bus_info);
          
     /*3:设置图像格式*/
     fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;   //这是设置传输流类型
     fmt.fmt.pix.width       = 320;   //设置分辨率
     fmt.fmt.pix.height      = 240;
     fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
     fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;   //图像格式，此处是jpg
 
     ioctl (fd, VIDIOC_S_FMT, &fmt) ;
      
     /*4:申请图像缓冲区*/
     req.count               = 4;
     req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     req.memory              = V4L2_MEMORY_MMAP;
     ioctl (fd, VIDIOC_REQBUFS, &req);
     
	 /*5:分配的映射地址*/     
	 buffers = calloc (req.count, sizeof (*buffers));
    
	 /**/
     for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
     { 
           /*获取图像缓冲区的信息*/
           buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
           buf.memory      = V4L2_MEMORY_MMAP;
           buf.index       = n_buffers;
			
		   /*把VIDIOC_REQBUFS中分配的数据缓存转换成物理地址*/
           ioctl (fd, VIDIOC_QUERYBUF, &buf); 
           buffers[n_buffers].length = buf.length; 
           
           /*把内核空间中的图像缓冲区映射到用户空间*/
          buffers[n_buffers].start = mmap (NULL ,    //通过mmap建立映射关系
                                        buf.length,
                                        PROT_READ | PROT_WRITE ,
                                        MAP_SHARED ,
                                        fd,
                                        buf.m.offset);
     }

     /*图像缓冲入队*/       
       for (i = 0; i < n_buffers; ++i)
       {
               buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
               buf.memory      = V4L2_MEMORY_MMAP;
               buf.index       = i; 
			   /*将数据从缓存读取出来*/
               ioctl (fd, VIDIOC_QBUF, &buf);
       }
    //开始采集图像数据  
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl (fd, VIDIOC_STREAMON, &type);
 
	//io复用
   fd_set fds;
   FD_ZERO (&fds);
   FD_SET (fd, &fds);
	//监听读文件描述符
   select(fd + 1, &fds, NULL, NULL, NULL);
   /*创建照片文件*/
     file_fd = open("text.jpg", O_RDWR|O_CREAT, 0777); //用来保存图片

   /*读取一幅图像*/
   read_frame();
   /*如果遇到拍出的照片不能打开就把下面这行代码取消注释*/
   //read_frame();
 
	
   //取消映射
   for (i = 0; i < n_buffers; ++i)
      munmap (buffers[i].start, buffers[i].length);   
 
   //关闭文件描述符
   close (fd);
   close (file_fd);
   printf("Camera Done.\n");
   return 0;
}
