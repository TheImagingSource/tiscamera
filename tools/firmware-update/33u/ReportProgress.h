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

#pragma once

#include "IReportProgress.h"
#include "ForwardProgress.h"

#include <chrono>

namespace lib33u
{
namespace util
{
namespace progress
{
	class ReportItemProgress : public ForwardProgress
	{
		using TClock = std::chrono::high_resolution_clock;

	public:
		ReportItemProgress( IReportProgress& p, int num_items )
			: ForwardProgress( p )
			, num_items_ { num_items }
			, pos_ { 0 }
			, start_ { TClock::now() }
		{
		}

	public:
		void report_item();
		void report_items( int n, const std::string& speed_unit = "", int unit_divider = 1 );

		virtual void report_percentage( int pct ) override;

	private:
		int num_items_;
		int pos_;
		std::chrono::time_point<TClock> start_;
	};

	class MapPercentageProgress : public ForwardProgress
	{
	public:
		MapPercentageProgress( IReportProgress& p, int begin, int end )
			: ForwardProgress( p )
			, begin_ { begin }
			, end_ { end }
		{
		}

	public:
		virtual void report_percentage( int pct ) override;

	private:
		int begin_;
		int end_;
	};

	inline MapPercentageProgress MapPercentage( IReportProgress& p, int begin, int end )
	{
		return MapPercentageProgress( p, begin, end );
	}

	inline ReportItemProgress MapItemProgress( IReportProgress& p, int num_items )
	{
		return ReportItemProgress( p, num_items );
	}

	class SimpleConsole : public IReportProgress
	{
	public:
		virtual void report_percentage( int pct ) override;
		virtual void report_group( const std::string & msg ) override;
		virtual void report_step( const std::string & msg ) override;
		virtual void report_speed( float speed, const std::string & unit ) override;
	};

	static SimpleConsole Console = {};
} /* namespace progress */
} /* namespace util*/
} /* namespace lib33u */
