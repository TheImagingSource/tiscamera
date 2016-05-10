/*
 * Copyright 2016 The Imaging Source Europe GmbH
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

#include "FirmwareUpgrade.h"

namespace FirmwareUpdate
{

namespace GigE3
{

inline tReportProgressFunc mapProgress (tReportProgressFunc progressFunc,
                                        int beginPct,
                                        int endPct)
{
    return [=](int pct, const std::string& msg)
    {
        progressFunc(beginPct + pct * (endPct - beginPct) / 100, msg);
    };
}

struct mapItemProgress
{
    tReportProgressFunc progressFunc_;
    int numItems_;
    int pos_;

    mapItemProgress (tReportProgressFunc progressFunc,
                     int beginPct, int endPct,
                     int numItems, const std::string& msg)
        : progressFunc_{mapProgress(progressFunc, beginPct, endPct)},
          numItems_{numItems},
          pos_{0}
    {
        progressFunc(beginPct, msg);
    }

    void NextItem ()
    {
        pos_ += 1;

        this->operator()(0, std::string());
    }

    void operator () (int pct, const std::string& msg)
    {
        int itemStepPct = pos_ * 100 / numItems_;

        int subStepPct = pct / numItems_;

        progressFunc_(itemStepPct + subStepPct, msg);
    }
}; /* struct mapItemProgress */

} /* namespace GigE3 */

} /* namespace FirmwareUpdate */
