
#ifndef AUTO_FOCUS_H_INC_
#define AUTO_FOCUS_H_INC_

#pragma once

#if defined KERNEL_DRIVER_
#include "../../dutils/base/timer.h"
#include "../../dutils/base/threading.h"
#else
#include <chrono>
#include <mutex>
#endif

#include "../image_transform_base.h"

#include "auto_alg_pass.h"

namespace {
	struct RegionInfo;
};

namespace img
{
	class auto_focus
	{
	public:
		auto_focus();

        bool    auto_alg_run( const img::img_descriptor& img, const auto_alg::auto_focus_params& state, POINT offsets, SIZE pixel_dim, int& new_focus_vale );

		bool	is_running() const;

        void    reset()
        {
            end();
        }

		/*
		 *	This must be called each time when the focus value was changed for the device.
		 */
		void	update_focus( int focus_val );
	private:
        /* 
		 * Beware that this function currently only works for bayer-8 images and RGB32
		 *	@return	true when a new focus value was evaluated and should be submitted to the focus control
		 *	@param new_focus_vale	When true was returned, this contains the new focus value to set
		 */
		bool	analyze_frame( const img::img_descriptor& img, POINT offsets, SIZE pixel_dim, int& new_focus_vale );

		void	run( int focus_val, int min, int max, const RECT& roi, int speed, int auto_step_divisor, bool suggest_sweep );
		void	end();
		bool	analyze_frame_( const img::img_descriptor& img, int& new_focus_vale );

		void	set_focus( int newval );

		void	restart_roi( const RegionInfo& info );
		void	find_region( const img::img_descriptor& image, RECT roi, RegionInfo& region );

		int		calc_next_focus();

		unsigned int		get_sharpness( const img::img_descriptor& img );

		bool	check_wait_condition();
		void	arm_focus_timer( int diff );

		struct data_holder
		{
			unsigned int	x, y, width, height;

			unsigned int	stepCount;
			int		focus_val;

			int		left, right;

			int		prev_sharpness;
			int		prev_focus;

			int		sweep_step;

			enum
			{
				ended = 0,
				init,
				sweep_1,
				sweep_2,
				binary_search,
			} state;
		} data;

		RECT			user_roi_;

		unsigned int	init_width_, init_height_;
        int             init_pitch_;
		SIZE        	init_pixel_dim_;
		POINT			init_offset_;

		volatile long	focus_applied_;

		int				focus_min_;
		int				focus_max_;
		// time in ms for moving between min and max
		int				max_time_to_wait_for_focus_change_;
		// minimum speed for moving between any focus values
		int				min_time_to_wait_for_focus_change_;

		int				auto_step_divisor_;

		bool			sweep_suggested_;

#if defined KERNEL_DRIVER_
		dutil::system_time::time	img_wait_endtime;
#else
		std::chrono::time_point<std::chrono::high_resolution_clock>	img_wait_endtime;
#endif
		int							img_wait_cnt;
	};
};


#endif // AUTO_FOCUS_H_INC_