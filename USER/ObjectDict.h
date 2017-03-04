#ifndef OBJECT_DICT_H
#define OBJECT_DICT_H

/**
	* @brief  Object dictionary of input register.
	*					Items blow are read-only variables to the hoster
	*/
enum inputRegEnum
{
	BATT_VOLT = 0,
	BATT_CURT,
	BATT_SOC,
	INPUT_REG_NUM //do not add element after this line
};

/**
	* @brief  object dictionary of holding register.
	*					Items blow are writable variables to the hoster
	*/
enum holdingRegEnum
{
	RW_REG1 = 0,
	RW_REG2,
	RW_REG3,
	RW_REG4,
	RW_REG5,
	HOLDING_REG_NUM //do not add element after this line
};

/**
	* @brief  ID of modbus RTU slaver.
	*					Just change the number whithout adding items into enum structure.
	*/
enum slaveIdEnum
{
	SLAVE_ID = 1
};
#endif
//end of file
