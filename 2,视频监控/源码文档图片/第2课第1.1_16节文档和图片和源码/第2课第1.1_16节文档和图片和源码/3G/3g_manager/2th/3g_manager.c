
/* 进行usb模式切换: 调用usb_modeswitch
 */
int do_switch(int argc, char **argv)
{
    /* 在使用mdev的嵌入式系统中, 接上一个USB设备后, 
     * 在/dev/目录下将出现/dev/usbdevM.N设备 
     * M表示总线, N表示设备地址    
     */

    /* 根据usbdevM.N找出它的VID,PID */

    /* 根据VID,PID在/usr/share/usb_modeswitch找到名为"VID:PID"的文件 */

    /* 调用"usb_modeswitch -V VID -P PID -f "文件内容"" */
    
}

/* 
 * 创建/dev/gsmmodem链接, 指向某个/dev/ttyUSBX
 */
int do_link(int argc, char **argv)
{
}

/* 进行usb模式切换: 调用usb_modeswitch
 * 创建/dev/gsmmodem链接, 指向某个/dev/ttyUSBX
 */

/* 用法:
 * 3g_manager switch usbdev1.17
 * 3g_manager link   ttyUSB1
 */
int main(int argc, char **argv)
{
    if ((argc != 3) || \
        ((strcmp(argv[1], "switch") && strcmp(argv[1], "link"))))
    {
        printf("Usage:\n");
        printf("%s switch <usbdevM.N> : switch 3G Modem\n", argv[0]);
        printf("%s link   <ttyUSBX>   : Create Link to /dev/ttyUSBX if it has interrupt endpoint\n", argv[0]);
        return -1;
    }

    if (strcmp(argv[1], "switch") == 0)
        return do_switch(argc, argv);
    else if (strcmp(argv[1], "link") == 0)
        return do_link(argc, argv);
    return -1;
}

