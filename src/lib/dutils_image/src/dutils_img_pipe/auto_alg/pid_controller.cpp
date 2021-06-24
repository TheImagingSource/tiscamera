
#include "pid_controller.h"

using namespace auto_alg::detail;

pid_controller::pid_controller( float p, float i, float d, float e_sum_limit )
    : _P( p ),
    _I( i ),
    _D( d ),
    _e_sum_limit( e_sum_limit ),
    _e_sum( 0 ),
    _e_prev( 0 ),
    _e_prev_valid( false )
{
}

void	pid_controller::reset()
{
    _e_sum = 0;
    _e_prev_valid = false;
}

float	pid_controller::step( float e, float fps )
{
    _e_sum += e;

    if( fps == 0.0f ) {
        fps = 1.0f;
    }

    float p = _P * e;
    float i = _I * _e_sum / fps;
    float d = 0;

    if( _e_prev_valid ) {
        d = _D * (e - _e_prev) / fps;
    }

    if( _e_sum > _e_sum_limit ) _e_sum = _e_sum_limit;
    if( _e_sum < -_e_sum_limit ) _e_sum = -_e_sum_limit;

    return p + i + d;
}