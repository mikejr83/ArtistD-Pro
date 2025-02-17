/**
 * @file lcd_lvgl_configuration.c
 * @date    2020-02-21
 * */
#include "../../../../MarlinCore.h"
#if ENABLED(TFT_LITTLE_VGL_UI)
#include "tft_lvgl_configuration.h"
#include "lvgl.h"
#include "../../../../feature/touch/xpt2046.h"
#include "draw_ready_print.h"
#include "W25Qxx.h"
#include "pic_manager.h"
#include "mks_hardware_test.h"
#include "draw_ui.h"
#if ENABLED(POWER_LOSS_RECOVERY)
#include "../../../../feature/powerloss.h"
#endif
#include "../../../../gcode/queue.h"


extern void LCD_IO_WriteData(uint16_t RegValue);
extern void LCD_IO_WriteReg(uint16_t Reg);
extern void LCD_IO_WriteMultiple(uint16_t color, uint32_t count);
extern void init_gb2312_font();
static lv_disp_buf_t disp_buf;
//static lv_color_t buf[LV_HOR_RES_MAX * 18]; 
//static lv_color_t buf[10*5]; 
//extern lv_obj_t * scr;
#if ENABLED (SDSUPPORT)
extern void UpdatePic();
extern void UpdateFont();
#endif
uint16_t DeviceCode = 0x9488;
extern uint8_t sel_id;

#define SetCs  
#define ClrCs 

#define  HDP  799  //Horizontal Display Period     //**
#define  HT   1000 //Horizontal Total
#define  HPS  51  //LLINE Pulse Start Position
#define  LPS  3   //	Horizontal Display Period Start Position
#define  HPW  8   //	LLINE Pulse Width

#define  VDP  479	//Vertical Display Period
#define  VT   530	//Vertical Total
#define  VPS  24	//	LFRAME Pulse Start Position
#define  FPS  23	//Vertical Display Period Start Positio
#define  VPW  3 	// LFRAME Pulse Width     //**

#define MAX_HZ_POSX HDP+1
#define MAX_HZ_POSY VDP+1 

extern uint8_t gcode_preview_over;
extern uint8_t flash_preview_begin;
extern uint8_t default_preview_flg;

void SysTick_Callback() 
{
	lv_tick_inc(1);
	print_time_count();

	if(uiCfg.filament_loading_time_flg == 1)
	{
		uiCfg.filament_loading_time_cnt++;
		uiCfg.filament_rate = (uint32_t)(((uiCfg.filament_loading_time_cnt / (uiCfg.filament_loading_time * 1000.0)) * 100.0) + 0.5);
		if(uiCfg.filament_loading_time_cnt >= (uiCfg.filament_loading_time * 1000))
	  	{
			uiCfg.filament_loading_time_cnt  = 0;
			uiCfg.filament_loading_time_flg  = 0;
			uiCfg.filament_loading_completed = 1;
			uiCfg.filament_rate              = 100;
		}
	}
	if(uiCfg.filament_unloading_time_flg == 1)
	{
		uiCfg.filament_unloading_time_cnt++;
		uiCfg.filament_rate = (uint32_t)(((uiCfg.filament_unloading_time_cnt / (uiCfg.filament_unloading_time * 1000.0)) * 100.0) + 0.5);
		if(uiCfg.filament_unloading_time_cnt >= (uiCfg.filament_unloading_time * 1000))
		{
			uiCfg.filament_unloading_time_cnt  = 0;
			uiCfg.filament_unloading_time_flg  = 0;
			uiCfg.filament_unloading_completed = 1;
			uiCfg.filament_rate                = 100;
		}
	}
}


void tft_set_cursor(uint16_t x,uint16_t y)
{
    #if 0
	if(DeviceCode==0x8989)
	{
	 	LCD_WriteReg(0x004e,y);			//行
    	LCD_WriteReg(0x004f,x);			//列
	}
	else if((DeviceCode==0x9919))
	{
		LCD_WriteReg(0x004e,x); // 行
  		LCD_WriteReg(0x004f,y); // 列	
	}
    else if((DeviceCode==0x5761))      //SSD1963
	{
		LCD_WrtReg(0x002A);	
        LCD_WrtRAM(x>>8);	    
        LCD_WrtRAM(x&0x00ff);
        LCD_WrtRAM(HDP>>8);	    
        LCD_WrtRAM(HDP&0x00ff);
        LCD_WrtReg(0x002b);	
        LCD_WrtRAM(y>>8);	    
        LCD_WrtRAM(y&0x00ff);
        LCD_WrtRAM(VDP>>8);	    
        LCD_WrtRAM(VDP&0x00ff);	
	}
	else if(DeviceCode == 0x9488)
    {
        ILI9488_WriteCmd(0X002A); 
        ILI9488_WriteData(x>>8); 
        ILI9488_WriteData(x&0X00FF); 
        ILI9488_WriteData(x>>8); 
        ILI9488_WriteData(x&0X00FF);			
        //ILI9488_WriteData(0X01); 
        //ILI9488_WriteData(0XDF);			
        ILI9488_WriteCmd(0X002B); 
        ILI9488_WriteData(y>>8); 
        ILI9488_WriteData(y&0X00FF);
        ILI9488_WriteData(y>>8); 
        ILI9488_WriteData(y&0X00FF);			
        //ILI9488_WriteData(0X01); 
        //ILI9488_WriteData(0X3F);			
    }				
	else
	{
  		LCD_WriteReg(0x0020,y); // 行
  		LCD_WriteReg(0x0021,0x13f-x); // 列
	}  	
    #else
    LCD_IO_WriteReg(0X002A); 
    LCD_IO_WriteData(x>>8); 
    LCD_IO_WriteData(x&0X00FF); 
    LCD_IO_WriteData(x>>8); 
    LCD_IO_WriteData(x&0X00FF);			
    //ILI9488_WriteData(0X01); 
    //ILI9488_WriteData(0XDF);			
    LCD_IO_WriteReg(0X002B); 
    LCD_IO_WriteData(y>>8); 
    LCD_IO_WriteData(y&0X00FF);
    LCD_IO_WriteData(y>>8); 
    LCD_IO_WriteData(y&0X00FF);			
    //ILI9488_WriteData(0X01); 
    //ILI9488_WriteData(0X3F);
    #endif
}

void LCD_WriteRAM_Prepare(void)
{
    #if 0
    if((DeviceCode==0x9325)||(DeviceCode==0x9328)||(DeviceCode==0x8989))
	{
  	ClrCs
  	LCD->LCD_REG = R34;
  	SetCs
	}
	else
	{
  	LCD_WrtReg(0x002C);
	}
    #else
    LCD_IO_WriteReg(0x002C);
    #endif
}

void tft_set_point(uint16_t x,uint16_t y,uint16_t point)
{
	//if(DeviceCode == 0x9488)
	//{
		if ( (x>480)||(y>320) ) return;
	//}
  	//**if ( (x>320)||(y>240) ) return;
    tft_set_cursor(x,y);    /*设置光标位置*/

    LCD_WriteRAM_Prepare();     /* 开始写入GRAM*/
    //LCD_WriteRAM(point);
    LCD_IO_WriteData(point);
}

void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{
  /* Write 16-bit Index, then Write Reg */
  ClrCs
  LCD_IO_WriteReg(LCD_Reg);
  /* Write 16-bit Reg */
  LCD_IO_WriteData(LCD_RegValue);
  SetCs
}

void ili9320_SetWindows(uint16_t StartX,uint16_t StartY,uint16_t width,uint16_t heigh)
{
	uint16_t s_h,s_l, e_h, e_l;
	
	uint16_t xEnd, yEnd;
	xEnd = StartX + width;
  yEnd = StartY + heigh-1;
  if(DeviceCode==0x8989)
  {
  	
	/*LCD_WriteReg(0x0044, (StartX & 0xff) | (xEnd << 8));
	 LCD_WriteReg(0x0045, StartY);
	 LCD_WriteReg(0x0046, yEnd);*/
	 LCD_WriteReg(0x0044, (StartY& 0xff) | (yEnd << 8));
	 LCD_WriteReg(0x0045, StartX);
	 LCD_WriteReg(0x0046, xEnd);
    
  }
	else if(DeviceCode == 0X9488)
	{
	 	s_h = (StartX >> 8) & 0X00ff;
		s_l = StartX & 0X00ff;
		e_h = ((StartX + width - 1) >> 8) & 0X00ff;
		e_l = (StartX + width - 1) & 0X00ff;
		
		LCD_IO_WriteReg(0x002A);
		LCD_IO_WriteData(s_h);
		LCD_IO_WriteData(s_l);
		LCD_IO_WriteData(e_h);
		LCD_IO_WriteData(e_l);

		s_h = (StartY >> 8) & 0X00ff;
		s_l = StartY & 0X00ff;
		e_h = ((StartY + heigh - 1) >> 8) & 0X00ff;
		e_l = (StartY + heigh - 1) & 0X00ff;
		
		LCD_IO_WriteReg(0x002B);
		LCD_IO_WriteData(s_h);
		LCD_IO_WriteData(s_l);
		LCD_IO_WriteData(e_h);
		LCD_IO_WriteData(e_l);		
	}	
  else if((DeviceCode==0x9325)||(DeviceCode==0x9328)||(DeviceCode==0x1505))
  {
	 /* LCD_WriteReg(0x0050, StartX);
	  LCD_WriteReg(0x0052, StartY);
	  LCD_WriteReg(0x0051, xEnd);
	  LCD_WriteReg(0x0053, yEnd);*/
	  LCD_WriteReg(0x0050,StartY);        //Specify the start/end positions of the window address in the horizontal direction by an address unit
		LCD_WriteReg(0x0051,yEnd);        //Specify the start positions of the window address in the vertical direction by an address unit 
		LCD_WriteReg(0x0052,320 - xEnd); 
		LCD_WriteReg(0x0053,320 - StartX - 1);        //Specify the end positions of the window address in the vertical direction by an address unit
	
  }	
	else
	 {
	 	s_h = (StartX >> 8) & 0Xff;
		s_l = StartX & 0Xff;
		e_h = ((StartX + width - 1) >> 8) & 0Xff;
		e_l = (StartX + width - 1) & 0Xff;
		
		LCD_IO_WriteReg(0x2A);
		LCD_IO_WriteData(s_h);
		LCD_IO_WriteData(s_l);
		LCD_IO_WriteData(e_h);
		LCD_IO_WriteData(e_l);

		s_h = (StartY >> 8) & 0Xff;
		s_l = StartY & 0Xff;
		e_h = ((StartY + heigh - 1) >> 8) & 0Xff;
		e_l = (StartY + heigh - 1) & 0Xff;
		
		LCD_IO_WriteReg(0x2B);
		LCD_IO_WriteData(s_h);
		LCD_IO_WriteData(s_l);
		LCD_IO_WriteData(e_h);
		LCD_IO_WriteData(e_l);
	 }
}

void LCD_Clear(uint16_t  Color)
{
  uint32_t index=0;

  
  unsigned int count; 
	
	if(DeviceCode ==0x9488)
	{
		tft_set_cursor(0,0);
    	ili9320_SetWindows(0,0,480,320);
		LCD_WriteRAM_Prepare();
		//index = (160*480);
    /*for(index=0;index<320*480;index++)
    {
        LCD_IO_WriteData(Color);
    }*/
    LCD_IO_WriteMultiple(Color, (480*320));
    //while(index --)
		//LCD_IO_WriteData(Color);
	}
	else if(DeviceCode == 0x5761)
	{
	    LCD_IO_WriteReg(0x002a);	
	    LCD_IO_WriteData(0);	    
	    LCD_IO_WriteData(0);
	    LCD_IO_WriteData(HDP>>8);	    
	    LCD_IO_WriteData(HDP&0x00ff);
	    LCD_IO_WriteReg(0x002b);	
	    LCD_IO_WriteData(0);	    
	    LCD_IO_WriteData(0);
	    LCD_IO_WriteData(VDP>>8);	    
	    LCD_IO_WriteData(VDP&0x00ff);
	    LCD_IO_WriteReg(0x002c);	
	    LCD_IO_WriteReg(0x002c);
	    for(count=0;count<(HDP+1)*(VDP+1);count++)
			{
	       LCD_IO_WriteData(Color);
			}
	}
	else
	{
		  tft_set_cursor(0,0); 
		  LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
		  for(index=0;index<76800;index++)
		  {
		     LCD_IO_WriteData(Color);
		  }		
	}
}

void init_tft()
{
	uint16_t i;
	//************* Start Initial Sequence **********//
	LCD_IO_WriteReg(0x00E0); 
	LCD_IO_WriteData(0x0000); 
	LCD_IO_WriteData(0x0007); 
	LCD_IO_WriteData(0x000f); 
	LCD_IO_WriteData(0x000D); 
	LCD_IO_WriteData(0x001B); 
	LCD_IO_WriteData(0x000A); 
	LCD_IO_WriteData(0x003c); 
	LCD_IO_WriteData(0x0078); 
	LCD_IO_WriteData(0x004A); 
	LCD_IO_WriteData(0x0007); 
	LCD_IO_WriteData(0x000E); 
	LCD_IO_WriteData(0x0009); 
	LCD_IO_WriteData(0x001B); 
	LCD_IO_WriteData(0x001e); 
	LCD_IO_WriteData(0x000f);  

	LCD_IO_WriteReg(0x00E1); 
	LCD_IO_WriteData(0x0000); 
	LCD_IO_WriteData(0x0022); 
	LCD_IO_WriteData(0x0024); 
	LCD_IO_WriteData(0x0006); 
	LCD_IO_WriteData(0x0012); 
	LCD_IO_WriteData(0x0007); 
	LCD_IO_WriteData(0x0036); 
	LCD_IO_WriteData(0x0047); 
	LCD_IO_WriteData(0x0047); 
	LCD_IO_WriteData(0x0006); 
	LCD_IO_WriteData(0x000a); 
	LCD_IO_WriteData(0x0007); 
	LCD_IO_WriteData(0x0030); 
	LCD_IO_WriteData(0x0037); 
	LCD_IO_WriteData(0x000f); 

	LCD_IO_WriteReg(0x00C0); 
	LCD_IO_WriteData(0x0010); 
	LCD_IO_WriteData(0x0010); 

	LCD_IO_WriteReg(0x00C1); 
	LCD_IO_WriteData(0x0041); 

	LCD_IO_WriteReg(0x00C5); 
	LCD_IO_WriteData(0x0000); 
	LCD_IO_WriteData(0x0022); 
	LCD_IO_WriteData(0x0080); 

	LCD_IO_WriteReg(0x0036); 
	//ILI9488_WriteData(0x0068);
	//if(gCfgItems.overturn_180 != 0xEE)
	//{
		LCD_IO_WriteData(0x0068); 
	//}
	//else
	//{
		//ILI9488_WriteData(0x00A8);
	//}

	LCD_IO_WriteReg(0x003A); //Interface Mode Control
	LCD_IO_WriteData(0x0055);

	LCD_IO_WriteReg(0X00B0);  //Interface Mode Control  
	LCD_IO_WriteData(0x0000); 
	LCD_IO_WriteReg(0x00B1);   //Frame rate 70HZ  
	LCD_IO_WriteData(0x00B0); 
	LCD_IO_WriteData(0x0011); 
	LCD_IO_WriteReg(0x00B4); 
	LCD_IO_WriteData(0x0002);   
	LCD_IO_WriteReg(0x00B6); //RGB/MCU Interface Control
	LCD_IO_WriteData(0x0002); 
	LCD_IO_WriteData(0x0042); 

	LCD_IO_WriteReg(0x00B7); 
	LCD_IO_WriteData(0x00C6); 

	//WriteComm(0XBE);
	//WriteData(0x00);
	//WriteData(0x04);

	LCD_IO_WriteReg(0x00E9); 
	LCD_IO_WriteData(0x0000);

	LCD_IO_WriteReg(0X00F7);    
	LCD_IO_WriteData(0x00A9); 
	LCD_IO_WriteData(0x0051); 
	LCD_IO_WriteData(0x002C); 
	LCD_IO_WriteData(0x0082);

	LCD_IO_WriteReg(0x0011); 
	for(i=0;i<65535;i++);
	LCD_IO_WriteReg(0x0029); 	

	ili9320_SetWindows(0,0,480,320);
	LCD_Clear(0x0000);

    //ili9320_Clear(0x0000);
}

extern uint8_t bmp_public_buf[17 * 1024];
void tft_lvgl_init()
{
	//uint16_t test_id=0;
    W25QXX.init(SPI_QUARTER_SPEED);
    //test_id=W25QXX.W25QXX_ReadID();
    #if ENABLED (SDSUPPORT)
    UpdatePic();
    UpdateFont();
    #endif
    gCfgItems_init();
    ui_cfg_init();
    disp_language_init();
    //spi_flash_read_test();
    WRITE(LCD_BACKLIGHT_PIN, LOW);
    LCD_Clear(0x0000);	
    draw_logo();
	
    WRITE(LCD_BACKLIGHT_PIN, HIGH);
    delay(4000);

    lv_init();	

    lv_disp_buf_init(&disp_buf, bmp_public_buf, NULL, LV_HOR_RES_MAX * 18);		/*Initialize the display buffer*/

    lv_disp_drv_t disp_drv;						/*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);				/*Basic initialization*/
    disp_drv.flush_cb = my_disp_flush;			/*Set your driver function*/
    disp_drv.buffer = &disp_buf;				/*Assign the buffer to the display*/
    lv_disp_drv_register(&disp_drv);			/*Finally register the driver*/

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);			 	/*Descriptor of a input device driver*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;		/*Touch pad is a pointer-like device*/
    indev_drv.read_cb = my_touchpad_read; 	 	/*Set your driver function*/
    lv_indev_drv_register(&indev_drv);		 	/*Finally register the driver*/

	systick_attach_callback(SysTick_Callback);

	init_gb2312_font();

    tft_style_init();

    #if ENABLED(POWER_LOSS_RECOVERY)
    if((recovery.info.valid_head != 0) && 
	(recovery.info.valid_head == recovery.info.valid_foot))
    {
    	if(gCfgItems.from_flash_pic == 1)
		flash_preview_begin = 1;
	else
		default_preview_flg = 1;
	    uiCfg.print_state = REPRINTING;

		memset(public_buf_m,0,sizeof(public_buf_m));
		strncpy(public_buf_m,recovery.info.sd_filename,sizeof(public_buf_m));
		card.printLongPath(public_buf_m);
		
		strncpy(list_file.long_name[sel_id],card.longFilename,sizeof(list_file.long_name[sel_id]));
		
		lv_draw_printing();
		#if 0
		#if ENABLED(DUAL_X_CARRIAGE)
		switch (gCfgItems.dual_x_mode) {
	        case REPRINT_FULL_CONTROL:
			//gcode.process_subcommands_now_P(PSTR("M605 S0"));
			queue.inject_P(PSTR("M605 S0\nG28 X"));
			break;
	        case REPRINT_AUTO_PARK:
			//gcode.process_subcommands_now_P(PSTR("M605 S1"));
			queue.inject_P(PSTR("M605 S1\nG28 X\nG1 X100 F2000"));
	          	break;
	        case REPRINT_DUPLICATION:
			//gcode.process_subcommands_now_P(PSTR("M605 S2"));
			queue.inject_P(PSTR("M605 S1\nT0\nM605 S2 X200\nG28 X\nG1 X100 F2000"));
			break;
		case REPRINT_MIRROR:
			//gcode.process_subcommands_now_P(PSTR("M605 S2"));
			//gcode.process_subcommands_now_P(PSTR("M605 S3"));
			queue.inject_P(PSTR("M605 S1\nT0\nM605 S2 X200\nG28 X\nG1 X100 F2000\nM605 S3 X200"));
	          	break;
	        default:
	          	break;
	      }
		#endif
		#endif

    }
    else
    #endif
    {
    WRITE(LCD_BACKLIGHT_PIN, LOW);
    lv_draw_ready_print(0);
	lv_task_handler();
	lv_clear_ready_print();
	lv_task_handler();
	WRITE(LCD_BACKLIGHT_PIN, HIGH);
	lv_draw_ready_print(1);
    }
	#if ENABLED(MKS_TEST)
	Test_GPIO();
	#endif
	//pinMode(PS_ON_PIN, OUTPUT);
	//digitalWrite(PS_ON_PIN, HIGH);
}



#if 0
void LCD_WriteRAM(uint16_t RGB_Code)					 
{
    #if 0
    ClrCs
    /* Write 16-bit GRAM Reg */
    LCD->LCD_RAM = RGB_Code;
    SetCs
    #else
    LCD_IO_WriteData(RGB_Code);
    #endif
}
#endif

void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{
    #if 1
    uint16_t i,width,height;
    uint16_t clr_temp;
    #if 0
    int32_t x,y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            //set_pixel(x, y, *color_p);  /* Put a pixel to the display.*/
			clr_temp = (uint16_t)(((uint16_t)color_p->ch.red<<11)
						|((uint16_t)color_p->ch.green<<5)
						|((uint16_t)color_p->ch.blue));
			tft_set_point(x, y,clr_temp);            
            color_p++;
        }
    }
    #else
    width = area->x2 - area->x1 + 1;
    height = area->y2 - area->y1 +1;
    //tft_set_cursor((uint16_t)area->x1,(uint16_t)area->y1);
    ili9320_SetWindows((uint16_t)area->x1,(uint16_t)area->y1,width,height);
    LCD_WriteRAM_Prepare();
    for(i=0;i<(width*height);i++)
    {
    	  clr_temp = (uint16_t)(((uint16_t)color_p->ch.red<<11)
						|((uint16_t)color_p->ch.green<<5)
						|((uint16_t)color_p->ch.blue));
        LCD_IO_WriteData(clr_temp);
	 color_p++;
    }
    #endif
	
    lv_disp_flush_ready(disp);         /* Indicate you are ready with the flushing*/
    #endif
}

#define TICK_CYCLE 1

static int32_t touch_time1 = 0;

unsigned int  getTickDiff(unsigned int curTick, unsigned int  lastTick)
{
	if(lastTick <= curTick)
	{
		return (curTick - lastTick) * TICK_CYCLE;
	}
	else
	{
		return (0xffffffff - lastTick + curTick) * TICK_CYCLE;
	}
}

static void xpt2046_corr(uint16_t * x, uint16_t * y)
{
#if XPT2046_XY_SWAP     
	int16_t swap_tmp;    
	swap_tmp = *x;    
	*x = *y;    
	*y = swap_tmp;
#endif    
	if((*x) > XPT2046_X_MIN)
		(*x) -= XPT2046_X_MIN;    
	else
		(*x) = 0;    
	if((*y) > XPT2046_Y_MIN)
		(*y) -= XPT2046_Y_MIN;    
	else
		(*y) = 0;    
	(*x) = (uint32_t)((uint32_t)(*x) * XPT2046_HOR_RES)/(XPT2046_X_MAX - XPT2046_X_MIN);    
	(*y) = (uint32_t)((uint32_t)(*y) * XPT2046_VER_RES)/(XPT2046_Y_MAX - XPT2046_Y_MIN);
#if XPT2046_X_INV     
	(*x) =  XPT2046_HOR_RES - (*x);
#endif
#if XPT2046_Y_INV     
	(*y) =  XPT2046_VER_RES - (*y);
#endif
}

#define  times  4
#define	CHX 	0x90//0x90 
#define	CHY 	0xD0//0xd0

int SPI2_ReadWrite2Bytes(void)  
{
	uint16_t temp = 0;
	volatile uint16_t ans=0;

	temp=W25QXX.spi_flash_read_write_byte(0xff);
	ans=temp<<8;
	temp=W25QXX.spi_flash_read_write_byte(0xff);
	ans|=temp;
	ans>>=3;
	return ans&0x0fff;
}
uint16_t		x_addata[times],y_addata[times];
void ADS7843_Rd_Addata(uint16_t *X_Addata,uint16_t *Y_Addata)
{

	uint16_t		i,j,k;
    //int result;

       #if ENABLED(TOUCH_BUTTONS)
	W25QXX.init(SPI_SPEED_6);
	for(i=0;i<times;i++)					
	{
		OUT_WRITE(TOUCH_CS_PIN, LOW);
		W25QXX.spi_flash_read_write_byte(CHX);
		y_addata[i] = SPI2_ReadWrite2Bytes();
		WRITE(TOUCH_CS_PIN, HIGH);
		
		OUT_WRITE(TOUCH_CS_PIN, LOW);
		W25QXX.spi_flash_read_write_byte(CHY);
		x_addata[i] = SPI2_ReadWrite2Bytes(); 
		WRITE(TOUCH_CS_PIN, HIGH);
	}
	W25QXX.init(SPI_QUARTER_SPEED);
	#endif
	//result = x_addata[0];
	for(i=0;i<times;i++)					
	{
		for(j = i + 1; j < times; j++)
		{
			if(x_addata[j] > x_addata[i])
			{
				k = x_addata[j];
				x_addata[j] = x_addata[i];
				x_addata[i] = k;
			}
		}
	}
	if(x_addata[times / 2 -1] - x_addata[times / 2 ] > 50)
	{
            *X_Addata = 0;
            *Y_Addata = 0;
            return ;
        }

	*X_Addata = (x_addata[times / 2 -1] + x_addata[times / 2 ]) /2;

	
	//result = y_addata[0];
	for(i=0;i<times;i++)					
	{
		for(j = i + 1; j < times; j++)
		{
			if(y_addata[j] > y_addata[i])
			{
				k = y_addata[j];
				y_addata[j] = y_addata[i];
				y_addata[i] = k;
			}
		}
	}

	
	if(y_addata[times / 2 -1] - y_addata[times / 2 ] > 50)
	{
            *X_Addata = 0;
            *Y_Addata = 0;
            return ;
        }

	*Y_Addata = (y_addata[times / 2 -1] + y_addata[times / 2 ]) /2;
	
	
}

#define ADC_VALID_OFFSET	30

uint8_t	TOUCH_PressValid(uint16_t _usX, uint16_t _usY)
{
	if ((_usX <= ADC_VALID_OFFSET) || (_usY <= ADC_VALID_OFFSET)
		|| (_usX >= 4095 - ADC_VALID_OFFSET)
		|| (_usY >= 4095 - ADC_VALID_OFFSET))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

static lv_coord_t last_x = 0;
   static lv_coord_t last_y = 0;
bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
    #if 1
	uint32_t tmpTime, diffTime = 0;

	tmpTime = millis();
	diffTime = getTickDiff(tmpTime, touch_time1);
    /*Save the state and save the pressed coordinate*/
    //data->state = TOUCH_PressValid(last_x, last_y) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL; 
    //if(data->state == LV_INDEV_STATE_PR)  ADS7843_Rd_Addata((u16 *)&last_x, (u16 *)&last_y);
		//touchpad_get_xy(&last_x, &last_y);
    /*Save the pressed coordinates and the state*/
	if(diffTime > 4)
	{
		ADS7843_Rd_Addata((uint16_t *)&last_x, (uint16_t *)&last_y);
	    if(TOUCH_PressValid(last_x, last_y)) {
	        
	        data->state = LV_INDEV_STATE_PR;
			
		    /*Set the coordinates (if released use the last pressed coordinates)*/
	
			xpt2046_corr((uint16_t *)&last_x, (uint16_t *)&last_y);
		    data->point.x = last_x;
		    data->point.y = last_y;			
			
	    } else {
	        data->state = LV_INDEV_STATE_REL;
	    }  
	    touch_time1 = tmpTime;
	}

    return false; /*Return `false` because we are not buffering and no more data to read*/
    #endif
}


uint8_t logo_n[13] = "bmp_logo.bin";
void draw_logo()
{
	#if 1    
	int index; 
	int x_off = 0, y_off = 0;
	int _y;
	uint16_t *p_index;
	int i;
	#if 0
	for(index = 0; index < 10; index ++)
	{
	    Pic_Logo_Read(logo_n, bmp_public_buf, 15360);
	  
	    i = 0;
	    //GUI_DrawBitmap(&bmp_struct, x_off, y_off);
	    
	    LCD_setWindowArea(0, y_off * 24, 320, 24);
	    ili9320_SetCursor(0, y_off * 24);
	    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */ 
	    for(_y = y_off * 24; _y < (y_off + 1) * 24; _y++)
	    {
	        for (x_off = 0; x_off < 320; x_off++) 
	        {
	            p_index = (uint16_t *)(&bmp_public_buf[i]);                     
	            LCD_WriteRAM(*p_index);
	            i += 2;
	            
	        }
	        if(i >= 15360)
	                break;
	    }
	    y_off++;        

	    
	}
	LCD_setWindowArea(0, 0, 319, 239);
	#else
	for(index = 0; index < 40; index ++)
	{
		Pic_Logo_Read(logo_n, bmp_public_buf, 7680);

		
		i = 0;
		//GUI_DrawBitmap(&bmp_struct, x_off, y_off);
		
		ili9320_SetWindows(0, y_off * 8, 480, 8);
		//ili9320_SetCursor(0, y_off * 8);
		LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */ 
		for(_y = y_off * 8; _y < (y_off + 1) * 8; _y++)
		{
			for (x_off = 0; x_off < 480; x_off++) 
			{
				p_index = (uint16_t *)(&bmp_public_buf[i]); 					
				LCD_IO_WriteData(*p_index);
				i += 2;
				
			}
			if(i >= 7680)
					break;
		}
		y_off++;		

		
	}
	ili9320_SetWindows(0, 0, 479, 319);

	#endif
	#endif  

}


void LV_TASK_HANDLER()
{
	//lv_tick_inc(1);
	lv_task_handler();
	#if ENABLED(MKS_TEST)
	mks_test();
	#endif
	disp_pre_gcode(2,36);
	GUI_RefreshPage();
	printer_action_polling();
	//sd_detection();
}
#endif
