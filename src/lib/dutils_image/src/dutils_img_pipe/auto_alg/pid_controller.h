
#pragma once


namespace auto_alg {
namespace detail {
	class pid_controller
	{
	private:
		float	_P, _I, _D;
		float	_e_sum_limit;

		float	_e_sum;

		float	_e_prev;
		bool	_e_prev_valid;

		
	public:
		pid_controller( float p, float i, float d, float e_sum_limit );

		float	step( float e, float fps );

		void	reset( void );
	};
}
}
