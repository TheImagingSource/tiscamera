/*
 * Copyright 2017 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ReportProgress.h"

namespace lib33u
{
namespace util
{
namespace progress
{

	void ReportItemProgress::report_item()
	{
		report_items( 1 );
	}

	void ReportItemProgress::report_items( int n, const std::string & speed_unit, int unit_divider )
	{
		pos_ += n;

		report_percentage( 0 );

		if( !speed_unit.empty() )
		{
			auto t = TClock::now();
			auto dt_ms = std::chrono::duration_cast<std::chrono::microseconds>(t - start_).count();
			auto speed = pos_ / (dt_ms / 1e6f);
			auto speed_by_unit = speed / unit_divider;

			report_speed( speed_by_unit, speed_unit );
		}
	}

	void ReportItemProgress::report_percentage( int pct )
	{
		int item_step_pct = pos_ * 100 / num_items_;
		int sub_step_pct = pct / num_items_;

		ForwardProgress::report_percentage( item_step_pct + sub_step_pct );
	}

	void MapPercentageProgress::report_percentage( int pct )
	{
		int fwd_pct = begin_ + pct * (end_ - begin_) / 100;
		ForwardProgress::report_percentage( fwd_pct );
	}

	void SimpleConsole::report_percentage( int pct )
	{
		printf( "[%%] %d\n", pct );
	}
	void SimpleConsole::report_group( const std::string & msg )
	{
		printf( "[GROUP] %s\n", msg.c_str() );
	}
	void SimpleConsole::report_step( const std::string & msg )
	{
		printf( "[STEP] %s\n", msg.c_str() );
	}
	void SimpleConsole::report_speed( float speed, const std::string & unit )
	{
		printf( "[S] %.2f %s\n", speed, unit.c_str() );
	}

} /* namespace progress */
} /* namespace util */
} /* namespace lib33u */
