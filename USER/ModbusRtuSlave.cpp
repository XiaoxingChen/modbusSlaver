#include "ModbusRtuSlave.h"
#include <stdint.h>
#include "CUsart.h"
#include "mbcrc.h"
#include "Console.h"

#define BATT_UART USART1
#define DEFAULT_SLAVE_ID	1

const uint8_t RECV_BUFF_SIZE = 30;
static uint8_t recvBuf[RECV_BUFF_SIZE];

CModbusRtuSlave::inputRegTyp CModbusRtuSlave::inputReg_;
CModbusRtuSlave::holdingRegTyp CModbusRtuSlave::holdingReg_;
array<uint8_t, 50> CModbusRtuSlave::workBuf_;

CUsart battUsart(BATT_UART, recvBuf, RECV_BUFF_SIZE);

CModbusCharDev::CModbusCharDev(uint32_t break_period)
	:CCharDev(break_period)
{
	
}

/**
	* @brief  open device
	* @param  None
	* @retval 1: success
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
	* @param  buff:
	* @param  len:
	* @retval bytes that actually get
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

/**
	* @brief  Constructor of ModbusSlave
	* @param  None
	*/
CModbusRtuSlave::CModbusRtuSlave()
	:isFirstIn_(true),
	errCnt_(0),
	charDev_(5)
{
	inputReg_.at(1) = 0x5544;
}

/**
	* @brief  decode modbus rtu message to function code, address and data
	* @param  funCode:	function code of rtu message
	* @param  addr: access address of message
	* @param  data: write data or read data number
	* @retval 0: success
	* @retval -1: faild
	*/
int CModbusRtuSlave::decode(uint8_t& funCode, uint16_t& addr, uint16_t& data)
{
	if(workBuf_.at(0)!=  CModbusRtuSlave::SLAVE_ID)
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

/**
	* @brief  execute command from modbus host
	* @param  funCode: function code of rtu message
	* @param  addr: access address of message
	* @param  data: write data or read data number
	* @retval 0: success
	* @retval -1: faild
	*/
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
		
		workBuf_.at(0) = CModbusRtuSlave::SLAVE_ID;
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

/**
	* @brief  reply message to modbus host
	* @param  None
	* @retval None
	*/
void CModbusRtuSlave::reply()
{
	charDev_.write(workBuf_.begin(), workBufMsgLen_);
}


/**
	* @brief  run modbus slaver for 1 time
	* @param  None
	* @retval None
	*/
void CModbusRtuSlave::run()
{
	if(isFirstIn_)
	{
		charDev_.open();
		isFirstIn_ = false;
	}
	
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
}

//end of file
