#include <avr/io.h>
#include <avr/delay.h>

#include "rf73_spi.h"

#define INVBIT(port, bit) port = port ^ (1<<bit);
#define UPBIT(port, bit) port = port | (1<<bit);
#define DOWNBIT(port, bit) port = port & (~(1<<bit));

#define CE 1
#define SS 2
#define CSN 2
#define MOSI 3
#define MISO 4
#define SCK 5
#define SPIPORT PORTB
#define SPIPIN PINB



//*************** SPI PART

void Init_Spi()
{


// SPI initialization
// SPI Type: Master
// SPI Clock Rate: 250,000 kHz
// SPI Clock Phase: Cycle Half
// SPI Clock Polarity: Low
// SPI Data Order: MSB First
PORTB=0x00;
DDRB=0x3E;

SPCR=0x50;
SPSR=0x00;

//PORTB=0x00;
//DDRB=0xB8;
//SPCR=0x51;
SPSR=0x00;
}

UINT8 ReadWrite_Spi(UINT8 value)
{

	SPDR = value;
	while(!(SPSR & (1<<SPIF)));
//	if (SPDR)
//		PORTC = PORTC ^ (1<<7);
	return SPDR;

/*
	UINT8 bit_ctr;
	for(bit_ctr=0;bit_ctr<8;bit_ctr++)   // output 8-bit
	{
		if(value & 0x80)
		{
			UPBIT(SPIPORT,MOSI);//MOSI=1
		}
		else
		{
			DOWNBIT(SPIPORT,MOSI);//MOSI=0;		
		}

		value = (value << 1);           // shift next bit into MSB..
		UPBIT(SPIPORT,SCK);//SCK = 1;						// Set SCK high..
		value |= ((SPIPIN >> MISO) & 0x01);//value |= MISO;       		  // capture current MISO bit
		DOWNBIT(SPIPORT,SCK);//SCK = 0;            		  // ..then set SCK low again
	}
	return(value);           		  // return read UINT8
*/

}

//*************** RF73 PART

UINT8 op_status;

//Bank1 register initialization value

//In the array RegArrFSKAnalog,all the register value is the byte reversed!!!!!!!!!!!!!!!!!!!!!
const unsigned long Bank1_Reg0_13[]={       //latest config txt
0xE2014B40,
0x00004BC0,
0x028CFCD0,
0x41390099,
0x1B8296d9,
0xA67F0224,
0x00000000,
0x00000000,
0x00000000,
0x00000000,
0x00000000,
0x00000000,
0x00127300,
0x46B48000,
};

const UINT8 Bank1_Reg14[]=
{
	0x41,0x20,0x08,0x04,0x81,0x20,0xCF,0xF7,0xFE,0xFF,0xFF
};

//Bank0 register initialization value
const UINT8 Bank0_Reg[][2]={
{0,0x0F},//reflect RX_DR\TX_DS\MAX_RT,Enable CRC ,2byte,POWER UP,PRX
{1,0x3F},//Enable auto acknowledgement data pipe5\4\3\2\1\0
{2,0x3F},//Enable RX Addresses pipe5\4\3\2\1\0
{3,0x03},//RX/TX address field width 5byte
//{4,0xff},//auto retransmission dalay (4000us),auto retransmission count(15)
{4,0x00},//no retransmission dalay (),auto retransmission count(0)
//{5,0x17},//23 channel
{5,0x33},//11 channel
//{6,0x07},//air data rate-1M,out power 0dbm,setup LNA gain \bit4 must set up to 0
{6,0x27},//air data rate-250kb,out power 0dbm,setup LNA gain \bit4 must set up to 0
{7,0x07},//
{8,0x00},//
{9,0x00},
{12,0xc3},//only LSB Receive address data pipe 2, MSB bytes is equal to RX_ADDR_P1[39:8]
{13,0xc4},//only LSB Receive address data pipe 3, MSB bytes is equal to RX_ADDR_P1[39:8]
{14,0xc5},//only LSB Receive address data pipe 4, MSB bytes is equal to RX_ADDR_P1[39:8]
{15,0xc6},//only LSB Receive address data pipe 5, MSB bytes is equal to RX_ADDR_P1[39:8]
{17,0x20},//Number of bytes in RX payload in data pipe0(32 byte) 
{18,0x20},//Number of bytes in RX payload in data pipe1(32 byte)
{19,0x20},//Number of bytes in RX payload in data pipe2(32 byte)
{20,0x20},//Number of bytes in RX payload in data pipe3(32 byte)
{21,0x20},//Number of bytes in RX payload in data pipe4(32 byte)
{22,0x20},//Number of bytes in RX payload in data pipe5(32 byte)
{23,0x00},//fifo status
{28,0x3F},//Enable dynamic payload length data pipe5\4\3\2\1\0
{29,0x07}//Enables Dynamic Payload Length,Enables Payload with ACK,Enables the W_TX_PAYLOAD_NOACK command 
};


const UINT8 RX0_Address[]={0x34,0x43,0x10,0x10,0x01};//Receive address data pipe 0
const UINT8 RX1_Address[]={0x39,0x38,0x37,0x36,0xc2};////Receive address data pipe 1

extern UINT8 test_data;
extern UINT8  channel;
extern UINT8  power;
extern UINT8  data_rate;
extern UINT8 rx_buf[MAX_PACKET_LEN];

extern void delay_200ms(void);
extern void delay_50ms(void);
///////////////////////////////////////////////////////////////////////////////
//                  SPI access                                               //
///////////////////////////////////////////////////////////////////////////////


                                                              
/**************************************************         
Function: SPI_Write_Reg();                                  
                                                            
Description:                                                
	Writes value 'value' to register 'reg'              
**************************************************/        
void SPI_Write_Reg(UINT8 reg, UINT8 value)                 
{
	DOWNBIT(PORTB, CSN);                   // CSN low, init SPI transaction
	op_status = ReadWrite_Spi(reg);      // select register
	ReadWrite_Spi(value);             // ..and write value to it..
	UPBIT(PORTB, CSN);                   // CSN high again
}                                                         
/**************************************************/        
                                                            
/**************************************************         
Function: SPI_Read_Reg();                                   
                                                            
Description:                                                
	Read one UINT8 from BK2421 register, 'reg'           
**************************************************/        
UINT8 SPI_Read_Reg(UINT8 reg)                               
{                                                           
	UINT8 value;
	DOWNBIT(PORTB, CSN);               // CSN low, initialize SPI communication...
	op_status=ReadWrite_Spi(reg);            // Select register to read from..
	value = ReadWrite_Spi(0);    // ..then read register value
	UPBIT(PORTB, CSN);                // CSN high, terminate SPI communication

	return(value);        // return register value
}                                                           
/**************************************************/        
                                                            
/**************************************************         
Function: SPI_Read_Buf();                                   
                                                            
Description:                                                
	Reads 'length' #of length from register 'reg'         
/**************************************************/        
void SPI_Read_Buf(UINT8 reg, UINT8 *pBuf, UINT8 length)     
{                                                           
	UINT8 status,byte_ctr;                              
                                                            
	DOWNBIT(SPIPORT,CSN);//CSN = 0;                    		// Set CSN l
	status = ReadWrite_Spi(reg);       		// Select register to write, and read status UINT8
                                                            
	for(byte_ctr=0;byte_ctr<length;byte_ctr++)           
		pBuf[byte_ctr] = ReadWrite_Spi(0);    // Perform ReadWrite_Spi to read UINT8 from RFM73 
                                                            
	UPBIT(SPIPORT,CSN);//CSN = 1;                           // Set CSN high again
               
}                                                           
/**************************************************/        
                                                            
/**************************************************         
Function: SPI_Write_Buf();                                  
                                                            
Description:                                                
	Writes contents of buffer '*pBuf' to RFM73         
/**************************************************/        
void SPI_Write_Buf(UINT8 reg, UINT8 *pBuf, UINT8 length)    
{                                                           
	UINT8 byte_ctr;                              
                                                            
	DOWNBIT(SPIPORT,CSN);//CSN = 0;                   // Set CSN low, init SPI tranaction
	op_status = ReadWrite_Spi(reg);    // Select register to write to and read status UINT8
	for(byte_ctr=0; byte_ctr<length; byte_ctr++) // then write all UINT8 in buffer(*pBuf) 
		ReadWrite_Spi(*pBuf++);                                    
	UPBIT(SPIPORT,CSN);//CSN = 1;                 // Set CSN high again      

}
/**************************************************
Function: SwitchToRxMode();
Description:
	switch to Rx mode
/**************************************************/
void SwitchToRxMode()
{
	UINT8 value;

	SPI_Write_Reg(FLUSH_RX,0);//flush Rx

	value=SPI_Read_Reg(STATUS);	// read register STATUS's value
	SPI_Write_Reg(WRITE_REG|STATUS,value);// clear RX_DR or TX_DS or MAX_RT interrupt flag

	DOWNBIT(SPIPORT,CE);//CE=0;

	value=SPI_Read_Reg(CONFIG);	// read register CONFIG's value
	
//PRX
	value=value|0x01;//set bit 1
  	SPI_Write_Reg(WRITE_REG | CONFIG, value); // Set PWR_UP bit, enable CRC(2 length) & Prim:RX. RX_DR enabled..
	UPBIT(SPIPORT,CE);//CE=1;
}

/**************************************************
Function: SwitchToTxMode();
Description:
	switch to Tx mode
/**************************************************/
void SwitchToTxMode()
{
	UINT8 value;
	SPI_Write_Reg(FLUSH_TX,0);//flush Tx

	DOWNBIT(SPIPORT,CE);//CE=0;
	value=SPI_Read_Reg(CONFIG);	// read register CONFIG's value
//PTX
	value=value&0xfe;//set bit 0
  	SPI_Write_Reg(WRITE_REG | CONFIG, value); // Set PWR_UP bit, enable CRC(2 length) & Prim:RX. RX_DR enabled.
	
	UPBIT(SPIPORT,CE);//CE=1;
}

/**************************************************
Function: SwitchCFG();
                                                            
Description:
	 access switch between Bank1 and Bank0 

Parameter:
	_cfg      1:register bank1
	          0:register bank0
Return:
     None
/**************************************************/
void SwitchCFG(char _cfg)//1:Bank1 0:Bank0
{
	UINT8 Tmp;

	Tmp=SPI_Read_Reg(7);
	Tmp=Tmp&0x80;

	if( ( (Tmp)&&(_cfg==0) )
	||( ((Tmp)==0)&&(_cfg) ) )
	{
		SPI_Write_Reg(ACTIVATE_CMD,0x53);
	}
}

/**************************************************
Function: SetChannelNum();
Description:
	set channel number

/**************************************************/
void SetChannelNum(UINT8 ch)
{
	SPI_Write_Reg((UINT8)(WRITE_REG|5),(UINT8)(ch));
}



///////////////////////////////////////////////////////////////////////////////
//                  RFM73 initialization                                    //
///////////////////////////////////////////////////////////////////////////////
/**************************************************         
Function: RFM73_Initialize();                                  

Description:                                                
	register initialization
/**************************************************/   
void RFM73_Initialize()
{
	UINT8 i,j,temp;
 	UINT8 WriteArr[12];

	//DelayMs(100);//delay more than 50ms.
	_delay_ms(200);
	
	SwitchCFG(0);

	for(i=0;i<20;i++)
	{
		SPI_Write_Reg((WRITE_REG|Bank0_Reg[i][0]),Bank0_Reg[i][1]);
	}
	
/*//reg 10 - Rx0 addr
	SPI_Write_Buf((WRITE_REG|10),RX0_Address,5);
	
//REG 11 - Rx1 addr
	SPI_Write_Buf((WRITE_REG|11),RX1_Address,5);

//REG 16 - TX addr
	SPI_Write_Buf((WRITE_REG|16),RX0_Address,5);*/

//reg 10 - Rx0 addr
	for(j=0;j<5;j++)
	{
		WriteArr[j]=RX0_Address[j];
	}
	SPI_Write_Buf((WRITE_REG|10),&(WriteArr[0]),5);
	
//REG 11 - Rx1 addr
	for(j=0;j<5;j++)
	{
		WriteArr[j]=RX1_Address[j];
	}
	SPI_Write_Buf((WRITE_REG|11),&(WriteArr[0]),5);
//REG 16 - TX addr
	for(j=0;j<5;j++)
	{
		WriteArr[j]=RX0_Address[j];
	}
	SPI_Write_Buf((WRITE_REG|16),&(WriteArr[0]),5);
	
//	printf("\nEnd Load Reg");

	i=SPI_Read_Reg(29);//read Feature Register ???????§??¶??¬?¤¶?»??? Payload With ACK?¬???????????¬·??? ACTIVATE????????????0x73),?»??????¶??¬?¤¶?»??? Payload With ACK (REG28,REG29).
	if(i==0) // i!=0 showed that chip has been actived.so do not active again.
		SPI_Write_Reg(ACTIVATE_CMD,0x73);// Active
	for(i=22;i>=21;i--)
	{
		SPI_Write_Reg((WRITE_REG|Bank0_Reg[i][0]),Bank0_Reg[i][1]);
		//SPI_Write_Reg_Bank0(Bank0_Reg[i][0],Bank0_Reg[i][1]);
	}
	
//********************Write Bank1 register******************
	SwitchCFG(1);
	
	for(i=0;i<=8;i++)//reverse
	{
		for(j=0;j<4;j++)
			WriteArr[j]=(Bank1_Reg0_13[i]>>(8*(j) ) )&0xff;

		SPI_Write_Buf((WRITE_REG|i),&(WriteArr[0]),4);
	}

	for(i=9;i<=13;i++)
	{
		for(j=0;j<4;j++)
			WriteArr[j]=(Bank1_Reg0_13[i]>>(8*(3-j) ) )&0xff;

		SPI_Write_Buf((WRITE_REG|i),&(WriteArr[0]),4);
	}

	//SPI_Write_Buf((WRITE_REG|14),&(Bank1_Reg14[0]),11);
	for(j=0;j<11;j++)
	{
		WriteArr[j]=Bank1_Reg14[j];
	}
	SPI_Write_Buf((WRITE_REG|14),&(WriteArr[0]),11);

//toggle REG4<25,26>
	for(j=0;j<4;j++)
		//WriteArr[j]=(RegArrFSKAnalog[4]>>(8*(j) ) )&0xff;
		WriteArr[j]=(Bank1_Reg0_13[4]>>(8*(j) ) )&0xff;

	WriteArr[0]=WriteArr[0]|0x06;
	SPI_Write_Buf((WRITE_REG|4),&(WriteArr[0]),4);

	WriteArr[0]=WriteArr[0]&0xf9;
	SPI_Write_Buf((WRITE_REG|4),&(WriteArr[0]),4);

	//**************************Test spi*****************************//
	//SPI_Write_Reg((WRITE_REG|Bank0_Reg[2][0]),0x0f);
	//test_data = SPI_Read_Reg(0x02);

	
	//DelayMs(10);
	_delay_ms(50);
	
//********************switch back to Bank0 register access******************
	SwitchCFG(0);
	SwitchToRxMode();//switch to RX mode
}

//*****************************************************************************

void RFM73_SetPower(char power)
{
	UINT8 power_mask = (((power << 1) & 0b00000110) | 0b11111001);	

	SPI_Write_Reg((WRITE_REG|Bank0_Reg[6][0]),( Bank0_Reg[6][1] & power_mask));
	SwitchToRxMode();

}

/**************************************************
Function: Send_Packet
Description:
	fill FIFO to send a packet
Parameter:
	type: WR_TX_PLOAD or  W_TX_PAYLOAD_NOACK_CMD
	pbuf: a buffer pointer
	len: packet length
Return:
	None
**************************************************/
char Send_Packet(UINT8 type,UINT8* pbuf,UINT8 len)
{
	UINT8 fifo_sta;
	
	SwitchToTxMode();  //switch to tx mode

	fifo_sta=SPI_Read_Reg(FIFO_STATUS);	// read register FIFO_STATUS's value
	if((fifo_sta&FIFO_STATUS_TX_FULL)==0)//if not full, send data (write buff)
	{ 
	  	//RED_LED = 1;
		
		SPI_Write_Buf(type, pbuf, len); // Writes data to buffer
		
		//delay_50ms();
		//RED_LED = 0;
		//delay_50ms();
		_delay_ms(100);
	}	  
	
	return 0;	 	
}
/**************************************************
Function: Receive_Packet
Description:
	read FIFO to read a packet
Parameter:
	None
Return:
	None
**************************************************/
char Receive_Packet(char * buf, char buf_len)
{
	UINT8 len,i,sta,fifo_sta,value,chksum,aa, res = 0;
	UINT8 rx_buf[MAX_PACKET_LEN];

	sta=SPI_Read_Reg(STATUS);	// read register STATUS's value

	if((STATUS_RX_DR&sta) == 0x40)				// if receive data ready (RX_DR) interrupt
	{
		do
		{
			len=SPI_Read_Reg(R_RX_PL_WID_CMD);	// read len

			if(len<=MAX_PACKET_LEN)
			{
				SPI_Read_Buf(RD_RX_PLOAD,buf,len);// read receive payload from RX_FIFO buffer
			}
			else
			{
				SPI_Write_Reg(FLUSH_RX,0);//flush Rx
			}

			fifo_sta=SPI_Read_Reg(FIFO_STATUS);	// read register FIFO_STATUS's value
							
		}while((fifo_sta&FIFO_STATUS_RX_EMPTY)==0); //while not empty

/*		
		chksum = 0;
		for(i=0;i<16;i++)
		{
			chksum +=rx_buf[i]; 
		}
		if(chksum==rx_buf[16]&&rx_buf[0]==0x30)
		{

//			GREEN_LED = 1;
//			delay_50ms();
//			delay_50ms();
//			GREEN_LED = 0;
			_delay_ms(100);

			//Send_Packet(W_TX_PAYLOAD_NOACK_CMD,rx_buf,17);
			SwitchToRxMode();//switch to RX mode	

			res = 1;
		}
		
*/
		res = len;		
	}
	SPI_Write_Reg(WRITE_REG|STATUS,sta);// clear RX_DR or TX_DS or MAX_RT interrupt flag
	
	return res;	
}

//******************************************************************************************

