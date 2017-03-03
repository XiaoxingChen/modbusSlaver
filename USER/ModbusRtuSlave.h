#ifndef MODBUS_RTU_SLV_H
#define MODBUS_RTU_SLV_H

#include "mbproto.h"
#include <stdint.h>
#include "CharDev.h"
#include "array.h"
#include "mbcrc.h"
#include "Console.h"

class CModbusCharDev
	:public CCharDev
{
	public:
		CModbusCharDev(uint32_t break_period);
		virtual int open();
		virtual int close() {return 0;}
	
		virtual int write(const uint8_t*, uint32_t);
		virtual int read(uint8_t*, uint32_t);
	
		virtual uint32_t data_in_write_buf();
		virtual uint32_t freesize_in_write_buf();
		virtual uint32_t data_in_read_buf();
		virtual void clear_read_buf();
		
		virtual void runTransmitter(){};
		virtual void runReceiver(){};
		
};

class CModbusRtuSlave
{
	public:
		CModbusRtuSlave();
		void run();
	
	
	private:
		bool isFirstIn_;
		uint8_t errCnt_;
		CModbusCharDev charDev_;
		
		int decode(uint8_t& func, uint16_t& addr, uint16_t& data);
		int execute(uint8_t func, uint16_t addr, uint16_t data);
		void reply();
	
		void printWorkBuf()
		{
			Console::Instance()->printf("workbuf:");
			for(int i = 0; i < workBufMsgLen_; i++)
			{
				Console::Instance()->printf(" 0x%02X", workBuf_.at(i));
			}
			Console::Instance()->printf("\r\n");
		}
	
	
	private:
		uint16_t slaverId_;
		static array<uint16_t, 10> inputReg_;
		static array<uint16_t, 10> holdingReg_;
		static array<uint8_t, 50> workBuf_;
		uint16_t workBufMsgLen_;
};	

typedef NormalSingleton<CModbusRtuSlave>	ModbusSlave;

#endif
//end of files
