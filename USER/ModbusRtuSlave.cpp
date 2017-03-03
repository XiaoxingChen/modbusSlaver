#include "ModbusRtuSlave.h"
#include <stdint.h>
#include "CUsart.h"
#include "mbcrc.h"
#include "Console.h"

#define BATT_UART USART1
#define DEFAULT_SLAVE_ID	1

//Voltage, Current, SOC
//const uint8_t REG_NUM = 3;
//static uint16_t inputReg[REG_NUM] = {0x00aa,0x11aa,0x22aa};
const uint8_t RECV_BUFF_SIZE = 30;
static uint8_t recvBuf[RECV_BUFF_SIZE];

array<uint16_t, 10> CModbusRtuSlave::inputReg_;
array<uint16_t, 10> CModbusRtuSlave::holdingReg_;
array<uint8_t, 50> CModbusRtuSlave::workBuf_;

CUsart battUsart(BATT_UART, recvBuf, RECV_BUFF_SIZE);

CModbusCharDev::CModbusCharDev(uint32_t break_period)
	:CCharDev(break_period)
{
	
}

/**
	* @brief  open device
	* @param  None
	* @retval None
	*/
int CModbusCharDev::open()
{
	battUsart.InitSci();
	battUsart.InitSciGpio();
	return 1;
}
	
/**
	* @brief  write
	* @param  None
	* @retval None
	*/
int CModbusCharDev::write(const uint8_t* buff, uint32_t len)
{
	if(data_in_write_buf() > 0)
	{
		return -1;
	}
	battUsart.send_Array((uint8_t*)buff, len);
	{
		return 1;
	}
	
}

/**
	* @brief  read
	* @param  None
	* @retval None
	*/
int CModbusCharDev::read(uint8_t* buff, uint32_t len)
{
	if(!is_dataflow_break())
		return 0;
	
	return battUsart.read_RxFifo(buff);
}

/**
	* @brief  data_in_write_buf
	* @param  None
	* @retval None
	*/
uint32_t CModbusCharDev::data_in_write_buf()
{
	return battUsart.get_BytesInTxFifo();
}

/**
	* @brief  freesize_in_write_buf
	* @param  None
	* @retval None
	*/
uint32_t CModbusCharDev::freesize_in_write_buf()
{
	if(CUsart::TxDMA(BATT_UART)->CNDTR > 0)
		return 0;
	
	else
		return 0xFF;
}

/**
	* @brief  data_in_read_buf
	* @param  None
	* @retval None
	*/
uint32_t CModbusCharDev::data_in_read_buf()
{
	return battUsart.get_BytesInRxFifo();
}

/**
	* @brief  clear_read_buf
	* @param  None
	* @retval None
	*/
void CModbusCharDev::clear_read_buf()
{
	battUsart.clear_rxFifo();
}

CModbusRtuSlave::CModbusRtuSlave()
	:isFirstIn_(true),
	errCnt_(0),
	charDev_(5),
	slaverId_(DEFAULT_SLAVE_ID)
{
	inputReg_.at(1) = 0x5544;
}

int CModbusRtuSlave::decode(uint8_t& funCode, uint16_t& addr, uint16_t& data)
{
	if(workBuf_.at(0)!= slaverId_)
	{
		Console::Instance()->printf("slaver id%d unmatch", workBuf_.at(0));
		return -1;
	}
		
	
	if(workBufMsgLen_ < 3) 
	{
		Console::Instance()->printf("Message less than 3 bytes\r\n");
		return -1;
	}
	
	uint16_t crcResult = ((uint16_t)workBuf_.at(workBufMsgLen_ - 1) << 8) + workBuf_.at(workBufMsgLen_ - 2);
	if(usMBCRC16(workBuf_.begin(), workBufMsgLen_ - 2) != crcResult)
	{
		Console::Instance()->printf("CRC failed,should be 0x%X, get 0x%X\r\n", usMBCRC16(workBuf_.begin(), workBufMsgLen_ - 2), crcResult);
		return -1;
	}
	
	funCode = workBuf_.at(1);
	addr = ((uint16_t)workBuf_.at(2) << 8) + workBuf_.at(3);
	data = ((uint16_t)workBuf_.at(4) << 8) + workBuf_.at(5);
	return 0;
}

int CModbusRtuSlave::execute(uint8_t funCode, uint16_t addr, uint16_t data)
{
	if(MB_FUNC_READ_INPUT_REGISTER == funCode  || MB_FUNC_READ_HOLDING_REGISTER == funCode)
	{
		uint16_t* pReg;
		uint16_t dataNum = data;
		(MB_FUNC_READ_INPUT_REGISTER == funCode) ? (pReg = (uint16_t*)&inputReg_) : (pReg = (uint16_t*)&holdingReg_);
		if(addr + dataNum > inputReg_.size())
		{
			Console::Instance()->printf("invalid address\r\n");
			return -1;
		}
		
		
		
		workBuf_.at(0) = slaverId_;
		workBuf_.at(1) = funCode;
		workBuf_.at(2) = 2 * dataNum;//2bytes per register
		
		workBufMsgLen_ = 3 + 2 * dataNum + 2;
	
		for(int i = 0; i < dataNum; i++)
		{
			workBuf_.at(3 + 2*i) = pReg[i] >> 8;
			workBuf_.at(3 + 2*i + 1) = pReg[i] & 0xFF;
		}
		uint16_t crcResult = usMBCRC16(workBuf_.begin(), workBufMsgLen_ - 2);

		workBuf_.at(workBufMsgLen_ - 2) = crcResult & 0xFF;
		workBuf_.at(workBufMsgLen_ - 1) = crcResult >> 8;
		
		return 0;
	}
	else if(MB_FUNC_WRITE_REGISTER == funCode)
	{
		if(addr >= holdingReg_.size())
		{
			Console::Instance()->printf("invalid address\r\n");
			return -1;
		}
		
		holdingReg_.at(addr) = data;
		return 0;
	}
	
	return -1;
}

void CModbusRtuSlave::reply()
{
	charDev_.write(workBuf_.begin(), workBufMsgLen_);
//	Console::Instance()->printf("send:");
//	for(int i = 0; i < workBufMsgLen_; i++)
//	{
//		Console::Instance()->printf(" 0x%02X", workBuf_.begin()[i]);
//	}
//	Console::Instance()->printf("\r\n");
}



void CModbusRtuSlave::run()
{
	if(isFirstIn_)
	{
		charDev_.open();
		isFirstIn_ = false;
	}
	
	//charDev_.write((uint8_t*)testString, strlen(testString));
	charDev_.update_data_break_flag();
	
	if(charDev_.is_dataflow_break())
	{
		uint8_t funCode;
		uint16_t addr;
		uint16_t data;
		workBufMsgLen_ = charDev_.read(workBuf_.begin(), charDev_.data_in_read_buf());
		if(decode(funCode, addr, data) != 0)
		{
			printWorkBuf();
			return;
		}
		
		if(execute(funCode, addr, data) != 0)
		{
			printWorkBuf();
			return;
		}
		charDev_.clear_read_buf();
		reply();
	}
/*if(charDev_.is_dataflow_break() 
		&& (readData = charDev_.read(workBuf, charDev_.data_in_read_buf())) > 2)
	{
		for(int i = 0; i < readData; i++)
			{
				Console::Instance()->printf("0x%02X ", workBuf[i]);
			}
			Console::Instance()->printf("\r\n");	
		if(workBuf[0] != SLAVE_NUM)
		{
//			for(int i = 0; i < readData; i++)
//			{
//				Console::Instance()->printf("0x%02X ", workBuf[i]);
//			}
			Console::Instance()->printf("Message not for this node\r\n");
			
			return;
		}
		//Console::Instance()->printf("get pack\r\n");
		CRCresult = workBuf[readData - 2] + (workBuf[readData - 1] << 8);
		if(usMBCRC16(workBuf, readData - 2) != CRCresult)
		{
//			for(int i = 0; i < readData; i++)
//			{
//				Console::Instance()->printf("0x%02X ", workBuf[i]);
//			}
			Console::Instance()->printf("CRC failed,should be 0x%X, get 0x%X\r\n", usMBCRC16(workBuf, readData - 2), CRCresult);
			return;
		}
			
		uint8_t fun_code = workBuf[1];
		if(fun_code != MB_FUNC_READ_INPUT_REGISTER)
			return;
		
		uint16_t data_addr = (workBuf[2] << 8) + workBuf[3];
		uint16_t data_num = (workBuf[4] << 8) + workBuf[5];
		if(data_addr + data_num > REG_NUM)
		{
			Console::Instance()->printf("Access out of range\r\n");
			return;
		}
		
		Console::Instance()->printf("Send %d data since addr = %d\r\n", data_num, data_addr);	
		uint8_t byte_num = data_num * sizeof(uint16_t);
		workBuf[2] = byte_num;
		for(int i = 0; i < data_num ; i++)
		{
			*(&workBuf[3] + 2*i) = (inputReg[i] >> 8);
			*(&workBuf[3] + 2*i + 1) = (inputReg[i] & 0xFF);
		}
		CRCresult = usMBCRC16(workBuf,3 + byte_num);
		workBuf[3 + byte_num] = (CRCresult & 0xFF);
		workBuf[3 + byte_num + 1] = (CRCresult >> 8);
		
		battUsart.send_Array(workBuf, 2 + 1 + byte_num + 2);
	}	*/
}












//end of file
