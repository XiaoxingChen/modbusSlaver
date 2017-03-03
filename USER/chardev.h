#ifndef DEVICE_H
#define DEVICE_H
#include <stdint.h>
#include <Timer.h>
#include <string.h>
class CCharDev
{
	public:
		virtual int open() = 0;
		virtual int close() = 0;
	
		virtual int write(const uint8_t*, uint32_t) = 0;
		virtual int read(uint8_t*, uint32_t) = 0;
	
		virtual uint32_t data_in_write_buf() = 0;
		virtual uint32_t freesize_in_write_buf() = 0;
		virtual uint32_t data_in_read_buf() = 0;
		virtual void clear_read_buf(){}
		
		virtual void runTransmitter() = 0;
		virtual void runReceiver() = 0;
		
		CCharDev(uint32_t break_period = 0)
			:prev_data_to_read_(0),
			is_dataflow_break_(false),
			flow_break_timer_(NULL)
		{
			if(break_period > 0)
				flow_break_timer_ = new Timer(break_period, break_period);
		}
		
	/**
		* @brief  update_data_break_flag
		* @param  None
		* @retval None
		*/
		bool is_dataflow_break()
		{
			if(NULL == flow_break_timer_ || data_in_read_buf() == 0)
			{
				is_dataflow_break_ = false;
			}
			else if(flow_break_timer_->isAbsoluteTimeUp())
			{
				is_dataflow_break_ = true;
			}
			return is_dataflow_break_;
		}
		
	/**
		* @brief  update_data_break_flag
		* @param  None
		* @retval None
		*/
		void update_data_break_flag()
		{
			if(NULL == flow_break_timer_)
				return;
			
			if(data_in_read_buf() > prev_data_to_read_ || data_in_read_buf() == 0)
				flow_break_timer_->reset();
			
			prev_data_to_read_ = data_in_read_buf();
		}
	
		void run()
		{
			runTransmitter();
			runReceiver();
		}
	
	private:
		uint32_t prev_data_to_read_;
		bool is_dataflow_break_;
		Timer* flow_break_timer_;
		
};
#endif
//end of file
