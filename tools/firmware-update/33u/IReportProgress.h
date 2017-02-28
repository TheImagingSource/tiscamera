#pragma once

#include <string>
#include <cstdarg>

namespace lib33u
{
namespace util
{
namespace progress
{
	struct IReportProgress
	{
		virtual ~IReportProgress()
		{
		}

		virtual void report_percentage( int pct ) = 0;
		virtual void report_group( const std::string& msg ) = 0;
		virtual void report_step( const std::string& msg ) = 0;
		virtual void report_speed( float speed, const std::string& unit ) = 0;


		void report_group_format( const char* format, ... )
		{
			va_list ap;
			va_start( ap, format );

			char buffer[256];
			vsnprintf( buffer, 256, format, ap );

			report_group( std::string( buffer ) );

			va_end( ap );
		}

		void report_step_format( const char* format, ... )
		{
			va_list ap;
			va_start( ap, format );

			char buffer[256];
			vsnprintf( buffer, 256, format, ap );

			report_step( std::string( buffer ) );

			va_end( ap );
		}
	};
} /* namespace progress */
} /* namespace util */
} /* namespace lib33u */
