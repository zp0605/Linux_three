//******************************************************************************
// File: head.S
// 功能: 设置SDRAM，将程序复制到SDRAM，然后跳到SDRAM继续执行
//******************************************************************************       

#define S3C2440_MPLL_200MHZ     ((0x5c<<12)|(0x01<<4)|(0x02))
#define S3C2440_MPLL_400MHZ     ((0x5c<<12)|(0x01<<4)|(0x01))
#define MEM_CTL_BASE    0x48000000
   
.extern     main
.text 
.global _start 
_start:
//******************************************************************************       
// 中断向量，本程序中，除Reset和HandleIRQ外，其它异常都没有使用
//******************************************************************************       
    b   Reset

// 0x04: 未定义指令中止模式的向量地址
HandleUndef:
    b   HandleUndef 
 
// 0x08: 管理模式的向量地址，通过SWI指令进入此模式
HandleSWI:
    b   HandleSWI

// 0x0c: 指令预取终止导致的异常的向量地址
HandlePrefetchAbort:
    b   HandlePrefetchAbort

// 0x10: 数据访问终止导致的异常的向量地址
HandleDataAbort:
    b   HandleDataAbort

// 0x14: 保留
HandleNotUsed:
    b   HandleNotUsed

// 0x18: 中断模式的向量地址
    b   HandleIRQ

// 0x1c: 快中断模式的向量地址
HandleFIQ:
    b   HandleFIQ

Reset:                  
/* 1. 关看门狗 */
	ldr r0, =0x53000000
	mov r1, #0
	str r1, [r0]

/* 2. 设置时钟 */
	ldr r0, =0x4c000014
	//	mov r1, #0x03;			  // FCLK:HCLK:PCLK=1:2:4, HDIVN=1,PDIVN=1
	mov r1, #0x05;			  // FCLK:HCLK:PCLK=1:4:8
	str r1, [r0]

	/* 如果HDIVN非0，CPU的总线模式应该从“fast bus mode”变为“asynchronous bus mode” */
	mrc	p15, 0, r1, c1, c0, 0		/* 读出控制寄存器 */ 
	orr	r1, r1, #0xc0000000			/* 设置为“asynchronous bus mode” */
	mcr	p15, 0, r1, c1, c0, 0		/* 写入控制寄存器 */

	/* MPLLCON = S3C2440_MPLL_200MHZ */
	ldr r0, =0x4c000004
	ldr r1, =S3C2440_MPLL_400MHZ
	str r1, [r0]

	/* 启动ICACHE */
	mrc p15, 0, r0, c1, c0, 0	@ read control reg
	orr r0, r0, #(1<<12)
	mcr	p15, 0, r0, c1, c0, 0   @ write it back


/* 3. 初始化SDRAM */
	ldr r0, =MEM_CTL_BASE
	adr r1, sdram_config     /* sdram_config的当前地址 */
	add r3, r0, #(13*4)
1:
	ldr r2, [r1], #4
	str r2, [r0], #4
	cmp r0, r3
	bne 1b

/* 4. 重定位 : 把bootloader本身的代码从flash复制到它的链接地址去 */
	ldr sp, =0x34000000

	bl nand_init

	mov r0, #0
	ldr r1, =_start
	ldr r2, =__bss_start
	sub r2, r2, r1
	
	bl copy_code_to_sdram
	bl clear_bss
	
    msr cpsr_c, #0xd2       // 进入中断模式
    ldr sp, =0x33000000     // 设置中断模式栈指针

    msr cpsr_c, #0xdf       // 进入系统模式
    ldr sp, =0x34000000     // 设置系统模式栈指针，

    msr cpsr_c, #0x5f       // 设置I-bit=0，开IRQ中断

    ldr lr, =halt_loop      // 设置返回地址
    ldr pc, =main           // 调用main函数
halt_loop:
    b   halt_loop

HandleIRQ:
    sub lr, lr, #4                  // 计算返回地址
    stmdb   sp!,    { r0-r12,lr }   // 保存使用到的寄存器
                                    // 注意，此时的sp是中断模式的sp
                                    // 初始值是上面设置的4096
    
    ldr lr, =int_return             // 设置调用IRQ_Handle函数后的返回地址  
    ldr pc, =IRQ_Handle             // 调用中断分发函数，在interrupt.c中
int_return:
    ldmia   sp!,    { r0-r12,pc }^  // 中断返回, ^表示将spsr的值复制到cpsr

sdram_config:
	.long 0x22011110	 //BWSCON
	.long 0x00000700	 //BANKCON0
	.long 0x00000700	 //BANKCON1
	.long 0x00000700	 //BANKCON2
	.long 0x00000700	 //BANKCON3  
	.long 0x00000700	 //BANKCON4
	.long 0x00000700	 //BANKCON5
	.long 0x00018005	 //BANKCON6
	.long 0x00018005	 //BANKCON7
	.long 0x008C04F4	 // REFRESH
	.long 0x000000B1	 //BANKSIZE
	.long 0x00000030	 //MRSRB6
	.long 0x00000030	 //MRSRB7
    
