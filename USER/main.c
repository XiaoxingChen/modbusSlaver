#include "stm32f10x.h"
#include "Console.h"
#include "CommonConfig.h"
#include "Timer.h"
#include "CCan.h"
#include "ModbusRtuSlave.h"
#include "CUsart.h"
#include "resetMailbox.h"
#include "array.h"
CanTxMsg tempMsg;

array<uint8_t, 3> testA = {1,2,3};
int main()
{
	SCB->VTOR = 0x08008000;
	CommonConfig();
	BaseTimer::Instance()->initialize();
	BaseTimer::Instance()->start();
	
	CanRouter1.InitCan();
	CanRouter1.InitCanGpio(CCanRouter::GROUP_B8);

	Console::Instance()->printf("\r\nApp start..\r\n");
	ModbusSlave::Instance()->run();
	resetMailbox::Instance()->attachToRouter(CanRouter1);
	while(1)
	{
		CanRouter1.runTransmitter();
		CanRouter1.runReceiver();
		Console::Instance()->runTransmitter();
		ModbusSlave::Instance()->run();
	}
	//return 0;
}
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	Timer loopTimer(1000,1000);
	
  /* Infinite loop */
  while (1)
  {
		Console::Instance()->runTransmitter();
		if(loopTimer.isAbsoluteTimeUp())
		{
			Console::Instance()->printf("Wrong parameters value: file %s on line %d\r\n", file, line);
		}
  }
}
