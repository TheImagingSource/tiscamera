
#ifndef AUTO_FOCUS_H_INC_
#define AUTO_FOCUS_H_INC_

#pragma once

#include <dutils_img_pipe/auto_alg_params.h>

namespace {
    struct RegionInfo;
}

namespace auto_alg::impl
{
    bool    supports_auto_focus( const img::img_type& img ) noexcept;

    class auto_focus
    {
    public:
        auto_focus();

        bool    is_auto_alg_run_needed( const auto_alg::auto_focus_params& params ) const noexcept;

        bool    auto_alg_run( uint64_t time_point, const img::img_descriptor& img, const auto_alg::auto_focus_params& state, img::point offsets, img::dim pixel_dim, int& new_focus_vale );

        bool	is_running() const;

        void    reset() { end(); }
    private:
        /* 
         * Beware that this function currently only works for bayer-8 images and RGB32
         *	@return	true when a new focus value was evaluated and should be submitted to the focus control
         *	@param new_focus_vale	When true was returned, this contains the new focus value to set
         */
        bool	analyze_frame( uint64_t now, const img::img_descriptor& img, int& new_focus_vale );

        void	setup_run( int focus_val, const auto_alg::auto_focus_params::run_cmd_param_struct& params, img::rect roi );
        void	end();
        bool	analyze_frame_( const img::img_descriptor& img, int& new_focus_vale );

        void	restart_roi( const RegionInfo& info );
        void	find_region( const img::img_descriptor& image, img::rect roi, RegionInfo& region );

        int		calc_next_focus();

        int		get_sharpness( const img::img_descriptor& img );

        bool	check_wait_condition( uint64_t now );
        void	arm_focus_timer( uint64_t now, int diff );

        void	update_focus( int focus_val );
        
        struct data_holder
        {
            int	x, y, width, height;

            int	    stepCount;
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

        img::rect	    user_roi_;

        img::dim	    init_dim_;
        img::point		init_offset_;

        int				focus_min_ = 0;
        int				focus_max_ = 0;
        // time in ms for moving between min and max
        int				max_time_to_wait_for_focus_change_ = 0;
        // minimum speed for moving between any focus values
        int				min_time_to_wait_for_focus_change_ = 0;

        int				auto_step_divisor_ = 0;

        bool			sweep_suggested_ = false;

        uint64_t        img_wait_endtime_ = 0;  // in us
        int				img_wait_cnt_ = 0;
    };
}


#endif // AUTO_FOCUS_H_INC_