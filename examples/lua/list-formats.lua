#! /usr/bin/env lua

-- Copyright 2016 The Imaging Source Europe GmbH
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
-- This example will show you how to retrieve the available gstreamer
-- caps from tcamsrc and how to display print them to stdout.
--

local lgi = require 'lgi'
local Gst = lgi.Gst
local Tcam = lgi.Tcam

function list_formats(serial)
    source = Gst.ElementFactory.make("tcamsrc")

    if serial then
        source.serial = serial
    end

    -- ensure that the real caps are available
    source:set_state(Gst.State.PAUSED)

    caps = source.pads[1]:query_caps()
    for i = 0,caps:get_size()-1 do
        print(caps:get_structure(i):to_string())
    end
    source:set_state(Gst.State.NULL)
end

Gst.init(arg)
serial = arg[1]
list_formats(serial)
