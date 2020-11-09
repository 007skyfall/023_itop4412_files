#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/irq.h>
#include <linux/gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio.h>
#include <mach/gpio-exynos4.h>

#define DS18B20_DEBUG
#ifdef DS18B20_DEBUG
#define DPRINTK(x...) printk("DS18B20_CTL DEBUG:" x)
#else
#define DPRINTK(x...)
#endif

#define DRIVER_NAME "ds18b20"
#define DEVICE_NAME "ds18b20"
#define DS18B20_DQ			EXYNOS4_GPA0(7)  //J38	�ӿ�13������
#define OUTPUT               S3C_GPIO_OUTPUT
#define INPUT                S3C_GPIO_INPUT

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("TOPEET");

unsigned char init_ds(void)//ds18b20��λ������0�ɹ�������1ʧ��
{
        unsigned char ret = 1;
		int i =0;
        s3c_gpio_cfgpin(DS18B20_DQ,OUTPUT);
        s3c_gpio_setpull(DS18B20_DQ, S3C_GPIO_PULL_DOWN);
        gpio_direction_output(DS18B20_DQ,0);
	    udelay(250);
        gpio_direction_output(DS18B20_DQ,0);//���͸�λ����
        udelay(500);//��ʱ( >480us )
        gpio_direction_output(DS18B20_DQ,1);//����������  
	    s3c_gpio_cfgpin(DS18B20_DQ, INPUT); //�÷��ص�ֵ���жϳ�ʼ����û�гɹ���18B20���ڵĻ�=0������=1 
		
	while((ret==1)&&(i<10)){
		ret = gpio_get_value(DS18B20_DQ);
		udelay(10);
		i++;
	}
	if(ret==0){
		return 0;
	}
	else{
               return -1;
	}			
}




void write_byte(char data)//��18b20дһ���ֽ�
{                          //�����ߴӸߵ�ƽ�����͵�ƽ������д��ʼ�źš�15us֮�ڽ�����д��λ�͵��������ϣ�
    int i = 0;
    s3c_gpio_cfgpin(DS18B20_DQ,OUTPUT);
	s3c_gpio_setpull(DS18B20_DQ, S3C_GPIO_PULL_UP);
	for(i=0;i<8;i++) 
        {
                gpio_direction_output(DS18B20_DQ,0);
	        udelay(10);
                gpio_direction_output(DS18B20_DQ,1);
                gpio_direction_output(DS18B20_DQ, data&0x01);  
                udelay(40);
                gpio_direction_output(DS18B20_DQ,1); 
                udelay(2);
                data >>= 1;
	}		
}
unsigned char read_byte(void)//��18b20��һ���ֽ�
{                                     //�����������ȴӸ������͵�ƽ1us���ϣ���ʹ��������Ϊ�ߵ�ƽ���Ӷ��������ź�
        unsigned char i;
        unsigned char data=0;
        s3c_gpio_cfgpin(DS18B20_DQ,OUTPUT); 
        for(i=0;i<8;i++)  
	{
                data >>= 1;
	        gpio_direction_output(DS18B20_DQ,0);
	        udelay(1);
	        gpio_direction_output(DS18B20_DQ,1);
	        s3c_gpio_cfgpin(DS18B20_DQ,INPUT);
                udelay(10);		   
	        if(gpio_get_value(DS18B20_DQ))
	        data |= 0x80;
	        udelay(50);           
                s3c_gpio_cfgpin(DS18B20_DQ,OUTPUT);
	        gpio_direction_output(DS18B20_DQ,0);
	        gpio_direction_output(DS18B20_DQ,1);		   
	}               		  
        return data; 
}

static ssize_t ds18b20_ctl_read(struct file *files, unsigned int *buffer, size_t count, loff_t *ppos)
{
         unsigned int tmp;
	     unsigned int ret;
	     unsigned int th,tl;
         th=tl=0;   
	     init_ds( );
	     udelay(500);
	     write_byte(0xcc);//����������кŵĲ���
         write_byte(0x44); //�����¶�ת��
         init_ds( );
	     udelay(500);	  	     	
	     write_byte(0xcc);   //����������кŵĲ���
         write_byte(0xbe);   //׼�����¶�
         tl= read_byte( );   //�����¶ȵĵ�λLSB  
         th= read_byte( );   //�����¶ȵĸ�λMSB	
	     th<<=8;             
         tl|=th;         	 //��ȡ�¶�	  
	     tmp=tl;
         ret=copy_to_user(buffer, &tmp, sizeof(tmp));
		 
              if(ret>0)  
             {  
                    return 0;  
             }
                return -1;  	     
}

static int ds18b20_ctl_close(struct inode *inode, struct file *file){
	printk(" %s  ! ! !\n",__FUNCTION__);
	DPRINTK("Device Closed Success!\n");
	return 0;
}

static int ds18b20_ctl_open(struct inode *inode, struct file *file){
	printk(" %s  ! ! !\n",__FUNCTION__);
	DPRINTK("Device Opened Success!\n");
	return nonseekable_open(inode,file);		
}

int ds18b20_pm(bool enable)
{
	int ret = 0;
	printk(" %s  ! ! !\n",__FUNCTION__);
	printk("debug: DS18B20 PM return %d\r\n" , ret);
	return ret;
};

static struct file_operations ds18b20_ctl_ops = {
	.owner = THIS_MODULE,
	.open = ds18b20_ctl_open,
	.release = ds18b20_ctl_close,
	.read 	 = ds18b20_ctl_read,		
};

static  struct miscdevice ds18b20_ctl_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &ds18b20_ctl_ops,
};


static int ds18b20_ctl_probe(struct platform_device *pdv){
	int ret;
	char *banner = "ds18b20 Initialize\n";
	printk(banner);
	printk(" %s  ! ! !\n",__FUNCTION__);
	ret = gpio_request(DS18B20_DQ,"GPA0_7");
	if(ret){
		DPRINTK( "gpio_request DS18B20_DQ failed!\n");
		return ret;
	}
	ret=init_ds();
	if(ret==0){
		DPRINTK( "DS18B20 initialized ok!\n");
	}
	else 
	        DPRINTK( "DS18B20 initialized fail!\n");  
		
	s3c_gpio_cfgpin(DS18B20_DQ,OUTPUT);
	gpio_direction_output(DS18B20_DQ, 1);
	ret = misc_register(&ds18b20_ctl_dev);
	if(ret<0)
	{
		printk("leds:register device failed!\n");
		goto exit;
	}
	return 0;

exit:
	misc_deregister(&ds18b20_ctl_dev);
	return ret;	
}

static int ds18b20_ctl_remove(struct platform_device *pdv){
	printk(" %s  ! ! !\n",__FUNCTION__);
	printk(KERN_EMERG "\tremove\n");
	gpio_free(DS18B20_DQ);
	misc_deregister(&ds18b20_ctl_dev);
	return 0;
}

static void ds18b20_ctl_shutdown(struct platform_device *pdv){
	printk(" %s  ! ! !\n",__FUNCTION__);
	
}

static int ds18b20_ctl_suspend(struct platform_device *pdv,pm_message_t pmt){
	printk(" %s  ! ! !\n",__FUNCTION__);
	DPRINTK("ds18b20 suspend:power off!\n");
	return 0;
}

static int ds18b20_ctl_resume(struct platform_device *pdv){
	printk(" %s  ! ! !\n",__FUNCTION__);
	DPRINTK("ds18b20 resume:power on!\n");
	return 0;
}

struct platform_driver ds18b20_ctl_driver = {
	.probe = ds18b20_ctl_probe,
	.remove = ds18b20_ctl_remove,
	.shutdown = ds18b20_ctl_shutdown,
	.suspend = ds18b20_ctl_suspend,
	.resume = ds18b20_ctl_resume,
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
	}
};

static int ds18b20_ctl_init(void)
{
	int DriverState;
	printk(" %s  ! ! !\n",__FUNCTION__);
	DriverState = platform_driver_register(&ds18b20_ctl_driver);
	printk(KERN_EMERG "\tDriverState is %d\n",DriverState);
	return 0;
}


static void ds18b20_ctl_exit(void)
{
	printk(" %s  ! ! !\n",__FUNCTION__);
	platform_driver_unregister(&ds18b20_ctl_driver);	
}

module_init(ds18b20_ctl_init);
module_exit(ds18b20_ctl_exit);
