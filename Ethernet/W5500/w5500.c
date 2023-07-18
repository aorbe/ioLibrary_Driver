//*****************************************************************************
//
//! \file w5500.c
//! \brief W5500 HAL Interface.
//! \version 1.0.2
//! \date 2013/10/21
//! \par  Revision history
//!       <2015/02/05> Notice
//!        The version history is not updated after this point.
//!        Download the latest version directly from GitHub. Please visit the our GitHub repository for ioLibrary.
//!        >> https://github.com/Wiznet/ioLibrary_Driver
//!       <2014/05/01> V1.0.2
//!         1. Implicit type casting -> Explicit type casting. Refer to M20140501
//!            Fixed the problem on porting into under 32bit MCU
//!            Issued by Mathias ClauBen, wizwiki forum ID Think01 and bobh
//!            Thank for your interesting and serious advices.
//!       <2013/12/20> V1.0.1
//!         1. Remove warning
//!         2. WIZCHIP_READ_BUF WIZCHIP_WRITE_BUF in case _WIZCHIP_IO_MODE_SPI_FDM_
//!            for loop optimized(removed). refer to M20131220
//!       <2013/10/21> 1st Release
//! \author MidnightCow
//! \copyright
//!
//! Copyright (c)  2013, WIZnet Co., LTD.
//! All rights reserved.
//! 
//! Redistribution and use in source and binary forms, with or without 
//! modification, are permitted provided that the following conditions 
//! are met: 
//! 
//!     * Redistributions of source code must retain the above copyright 
//! notice, this list of conditions and the following disclaimer. 
//!     * Redistributions in binary form must reproduce the above copyright
//! notice, this list of conditions and the following disclaimer in the
//! documentation and/or other materials provided with the distribution. 
//!     * Neither the name of the <ORGANIZATION> nor the names of its 
//! contributors may be used to endorse or promote products derived 
//! from this software without specific prior written permission. 
//! 
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//! AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
//! IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//! ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
//! LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
//! CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
//! SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//! INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
//! CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
//! ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
//! THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************
//#include <stdio.h>
#include "w5500.h"
#include "../../inc/stm32f4_discovery.h"
#include "../../inc/main.h"

#define _W5500_SPI_VDM_OP_          0x00
#define _W5500_SPI_FDM_OP_LEN1_     0x01
#define _W5500_SPI_FDM_OP_LEN2_     0x02
#define _W5500_SPI_FDM_OP_LEN4_     0x03


#if   (_WIZCHIP_ == 5500)
////////////////////////////////////////////////////

uint8_t  WIZCHIP_READ(uint32_t AddrSel)
{
   uint8_t ret;
   uint8_t spi_data[4];
   uint32_t delay = 100;


   WIZCHIP_CRITICAL_ENTER();
   SS_SET

   AddrSel |= (_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_);

   spi_data[0] = (AddrSel & 0x00FF0000) >> 16;
   spi_data[1] = (AddrSel & 0x0000FF00) >> 8;
   spi_data[2] = (AddrSel & 0x000000FF) >> 0;
   spi_data[3] = 0x00;
   
   ret = *(volatile uint8_t *)&SPI_ETH->DR;
   for(int x=0; x<4; x++) {
      while (!(SPI_ETH->SR & SPI_I2S_FLAG_TXE));
      *(volatile uint8_t *)&SPI_ETH->DR = spi_data[x];
      while (!(SPI_ETH->SR & SPI_I2S_FLAG_RXNE));
      ret = *(volatile uint8_t *)&SPI_ETH->DR;
   } 

   while (SPI_ETH->SR & SPI_I2S_FLAG_BSY);
   SS_RESET
   WIZCHIP_CRITICAL_EXIT();
   

   return ret;
}

void WIZCHIP_WRITE(uint32_t AddrSel, uint8_t wb )
{
   uint8_t spi_data[4];

   WIZCHIP_CRITICAL_ENTER();
   SS_SET


   AddrSel |= (_W5500_SPI_WRITE_ | _W5500_SPI_VDM_OP_);

   spi_data[0] = (AddrSel & 0x00FF0000) >> 16;
   spi_data[1] = (AddrSel & 0x0000FF00) >> 8;
   spi_data[2] = (AddrSel & 0x000000FF) >> 0;
   spi_data[3] = wb;

   for(int x=0; x<4; x++) {
      while (!(SPI_ETH->SR & SPI_I2S_FLAG_TXE));
      SPI_ETH->DR = spi_data[x];
   } 

   while (SPI_ETH->SR & SPI_I2S_FLAG_BSY);
   SS_RESET
   WIZCHIP_CRITICAL_EXIT();
}
         
void WIZCHIP_READ_BUF (uint32_t AddrSel, uint8_t* pBuf, uint16_t len)
{
   uint8_t spi_data[3];
   uint16_t i;

   WIZCHIP_CRITICAL_ENTER();
   SS_SET



   AddrSel |= (_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_);

   spi_data[0] = (AddrSel & 0x00FF0000) >> 16;
   spi_data[1] = (AddrSel & 0x0000FF00) >> 8;
   spi_data[2] = (AddrSel & 0x000000FF) >> 0;

   pBuf[0] = *(volatile uint8_t *)&SPI_ETH->DR;
   for(int x=0; x<3; x++) {
      while (!(SPI_ETH->SR & SPI_I2S_FLAG_TXE));
      *(volatile uint8_t *)&SPI_ETH->DR = spi_data[x];
      while (!(SPI_ETH->SR & SPI_I2S_FLAG_RXNE));
      pBuf[0] = *(volatile uint8_t *)&SPI_ETH->DR;
   }

   for(i = 0; i < len; i++) {
      while (!(SPI_ETH->SR & SPI_I2S_FLAG_TXE));
      *(volatile uint8_t *)&SPI_ETH->DR = 0x00;
      while (!(SPI_ETH->SR & SPI_I2S_FLAG_RXNE));
      pBuf[i] = *(volatile uint8_t *)&SPI_ETH->DR;
   }

   while (SPI_ETH->SR & SPI_I2S_FLAG_BSY);
   SS_RESET
   WIZCHIP_CRITICAL_EXIT();
}

void WIZCHIP_WRITE_BUF(uint32_t AddrSel, uint8_t* pBuf, uint16_t len)
{
   uint8_t spi_data[3];
   uint16_t i;

   WIZCHIP_CRITICAL_ENTER();
   SS_SET

   AddrSel |= (_W5500_SPI_WRITE_ | _W5500_SPI_VDM_OP_);

   spi_data[0] = (AddrSel & 0x00FF0000) >> 16;
   spi_data[1] = (AddrSel & 0x0000FF00) >> 8;
   spi_data[2] = (AddrSel & 0x000000FF) >> 0;

   for(int x=0; x<3; x++) {
      while (!(SPI_ETH->SR & SPI_I2S_FLAG_TXE));
      SPI_ETH->DR = spi_data[x];
   } 

   // while (!(SPI_ETH->SR & SPI_I2S_FLAG_TXE));
   // DMA2->LISR = DMA_LIFCR_CTCIF3;
   // DMA2_Stream3->M0AR = &pBuf[0];
   // DMA2_Stream3->NDTR = len;
   // DMA2_Stream3->CR  |= (uint32_t)DMA_SxCR_EN;
	// // GPIOD->ODR 			^= GPIO_Pin_12;
   // while( !(DMA2_Stream3->CR & (uint32_t)DMA_SxCR_EN));
   // while (!(DMA2->LISR & DMA_LISR_TCIF3));
   // DMA2->HIFCR =  DMA_LIFCR_CTCIF3;

   for(int x=0; x<len; x++) {
      while (!(SPI_ETH->SR & SPI_I2S_FLAG_TXE));
      SPI_ETH->DR = pBuf[x];
   } 

   while (SPI_ETH->SR & SPI_I2S_FLAG_BSY);
   SS_RESET
   WIZCHIP_CRITICAL_EXIT();
}


uint16_t getSn_TX_FSR(uint8_t sn)
{
   uint16_t val=0,val1=0;

   do
   {
      val1 = WIZCHIP_READ(Sn_TX_FSR(sn));
      val1 = (val1 << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_TX_FSR(sn),1));
      if (val1 != 0)
      {
        val = WIZCHIP_READ(Sn_TX_FSR(sn));
        val = (val << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_TX_FSR(sn),1));
      }
   }while (val != val1);
   return val;
}


uint16_t getSn_RX_RSR(uint8_t sn)
{
   uint16_t val=0,val1=0;

   do
   {
      val1 = WIZCHIP_READ(Sn_RX_RSR(sn));
      val1 = (val1 << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_RSR(sn),1));
      if (val1 != 0)
      {
        val = WIZCHIP_READ(Sn_RX_RSR(sn));
        val = (val << 8) + WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_RSR(sn),1));
      }
   }while (val != val1);
   return val;
}

void wiz_send_data(uint8_t sn, uint8_t *wizdata, uint16_t len)
{
   uint16_t ptr = 0;
   uint32_t addrsel = 0;

   if(len == 0)  return;
   ptr = getSn_TX_WR(sn);
   //M20140501 : implict type casting -> explict type casting
   //addrsel = (ptr << 8) + (WIZCHIP_TXBUF_BLOCK(sn) << 3);
   addrsel = ((uint32_t)ptr << 8) + (WIZCHIP_TXBUF_BLOCK(sn) << 3);
   //
   WIZCHIP_WRITE_BUF(addrsel,wizdata, len);
   
   ptr += len;
   setSn_TX_WR(sn,ptr);
}

void wiz_recv_data(uint8_t sn, uint8_t *wizdata, uint16_t len)
{
   uint16_t ptr = 0;
   uint32_t addrsel = 0;
   
   if(len == 0) return;
   ptr = getSn_RX_RD(sn);
   //M20140501 : implict type casting -> explict type casting
   //addrsel = ((ptr << 8) + (WIZCHIP_RXBUF_BLOCK(sn) << 3);
   addrsel = ((uint32_t)ptr << 8) + (WIZCHIP_RXBUF_BLOCK(sn) << 3);
   //
   WIZCHIP_READ_BUF(addrsel, wizdata, len);
   ptr += len;
   
   setSn_RX_RD(sn,ptr);
}


void wiz_recv_ignore(uint8_t sn, uint16_t len)
{
   uint16_t ptr = 0;

   ptr = getSn_RX_RD(sn);
   ptr += len;
   setSn_RX_RD(sn,ptr);
}

#endif
