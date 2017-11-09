#! /usr/bin/env lua

-- Copyright 2017 The Imaging Source Europe GmbH
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
-- http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

--
-- This example will show you how to list information about the available devices
--

local lgi = require 'lgi'
local Gst = lgi.Gst
local Tcam = lgi.Tcam


function list_devices()
    local source = Gst.ElementFactory.make("tcambin")

    local serials = source:get_device_serials()

    for i,single_serial in ipairs(serials) do

        -- This returns someting like:
        -- (True, name='DFK Z12GP031',
        -- identifier='The Imaging Source Europe GmbH-11410533', connection_type='aravis')
        -- The identifier is the name given by the backend
        -- The connection_type identifies the backend that is used.
        model, identifier, connection_type = source:get_device_info(single_serial)

        print(i, "Model: ", model, "Serial: ", single_serial, "Identifier: ", identifier)
    end
end

Gst.init(arg)  -- init gstreamer

list_devices()
