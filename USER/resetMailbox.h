#ifndef RESET_MAILBOX_H
#define RESET_MAILBOX_H

#include "CCan.h"
#include "Console.h"
#include "Timer.h"
#include "stm32f10x.h"

class CResetMailbox
	:public CCanRxMailbox
{
public:
	CResetMailbox()
	:CCanRxMailbox(&rstMailboxBuf_, 1),
		canBaseRouter_(NULL)
	{
		setExtId(0x5005);
	}
	~CResetMailbox(){}
	
		
	bool attachToRouter(CCanRouter& refRouter)
	{
		canBaseRouter_ = &refRouter;
		return CCanRxMailbox::attachToRouter(refRouter);
	}		
		
	virtual void pushMsg(const CanRxMsg& rstMsg)
	{
		uint8_t rstCmd[8] = {0x09, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff};
		if(0 == memcmp(rstMsg.Data, rstCmd, 8))
		{
			if(canBaseRouter_ != NULL)
			{
				CanTxMsg rstReplyMsg;
				rstReplyMsg.ExtId = 0x5004;
				rstReplyMsg.Data[0] = 0x09;
				rstReplyMsg.Data[0] = 0x00;
				rstReplyMsg.Data[0] = 0x00;
				rstReplyMsg.Data[0] = 0x00;
				rstReplyMsg.IDE = CAN_Id_Extended;
				rstReplyMsg.RTR = CAN_RTR_Data;
				rstReplyMsg.DLC = 4;
				canBaseRouter_->putMsg(rstReplyMsg);
				Console::Instance()->printf("get reset command\r\n");
	
				while(canBaseRouter_->getMsgsInTxQue() > 0)
					canBaseRouter_->runTransmitter();
				while(!Console::Instance()->isIdel());
				BaseTimer::Instance()->delay_ms(20);
				NVIC_SystemReset();
			}	
			Console::Instance()->printf("reset failed\r\n");			
		}
	}

private:
	CCanRouter* canBaseRouter_;
	CanRxMsg rstMailboxBuf_;
};

typedef NormalSingleton<CResetMailbox> resetMailbox;

#endif
//end of file
