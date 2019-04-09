/*
 * Copyright 2019 The Imaging Source Europe GmbH
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


#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "tcam-network.h"

#include <iostream>

TEST_CASE("Verify a IP address string is a valid address", "[isValidIpAddress]")
{

    REQUIRE(tis::isValidIpAddress("100.0.0.2") == true);
    REQUIRE(tis::isValidIpAddress("") == false);
    REQUIRE(tis::isValidIpAddress("192.168..0") == false);
    REQUIRE(tis::isValidIpAddress("AAA") == false);
}


TEST_CASE("Verify IP settings are legal", "[verification]")
{
    std::string err_reason;
    REQUIRE(tis::verifySettings("255.255.255.0", "255.255.0.0", "168.254.1.0",
                                err_reason) == false);
    // test that err_reason is set to someting since test failed.
    REQUIRE(!err_reason.empty());

    err_reason.clear();
    REQUIRE(tis::verifySettings("127.0.0.1", "255.255.255.0", "192.168.0.1",
                                err_reason) == false);
    REQUIRE(!err_reason.empty());

    err_reason.clear();
    REQUIRE(tis::verifySettings("0.0.0.0", "255.255.255.0", "192.168.0.1",
                                err_reason) == false);
    REQUIRE(!err_reason.empty());


    err_reason.clear();
    REQUIRE(tis::verifySettings("0.0.0.0", "255.255.255.0", "192.168.0.1",
                                err_reason) == false);
    REQUIRE(!err_reason.empty());

    // std::cout << err_reason << "\n";
}
