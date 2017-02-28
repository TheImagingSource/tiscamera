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

#include "ForwardProgress.h"

namespace lib33u
{
namespace util
{
namespace progress
{
	void ForwardProgress::report_percentage (int pct)
	{
		fwd_.report_percentage(pct);
	}

	void ForwardProgress::report_group (const std::string& msg)
	{
		fwd_.report_group(msg);
	}

	void ForwardProgress::report_step (const std::string& msg)
	{
		fwd_.report_step(msg);
	}

	void ForwardProgress::report_speed (float speed, const std::string& unit)
	{
		fwd_.report_speed(speed, unit);
	}
} /* namespace progress */
} /* namespace util */
} /* namespace lib33u */
