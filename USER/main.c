#include "stm32f10x.h"
#include "CSysTick.h"
#include "CUartConsole.h"
#include "CommonConfig.h"
#include "CLed.h"
#include "CCan.h"
#include "CUartCanProbe.h"
CanTxMsg tempMsg;
int main()
{
	CommonConfig();
	
	CanRouter250k.InitCan();
	CanRouter250k.InitCanGpio(CCanRouter::GROUP_B8);
	
	tempMsg.StdId = 0x00;
	tempMsg.Data[0] = 0x01;
	tempMsg.IDE = CAN_Id_Standard;
	tempMsg.RTR = CAN_RTR_Data;
	tempMsg.DLC = 8;
	
	Console::Instance()->printf("Start..\r\n");
	uint16_t i = 0;
	while(1)
	{
		// if(i++ == 0)
		// {
		// 	CanRouter250k.putMsg(tempMsg);
		// }
		
		CanRouter250k.runTransmitter();
		CanRouter250k.runReceiver();
		uartCanProbe::Instance()->run();
		Console::Instance()->run();
	}
	//return 0;
}
