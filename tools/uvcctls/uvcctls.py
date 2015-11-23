#
# Copyright 2015 The Imaging Source Europe GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#


import os
import glob
import struct
import fcntl
import IOCTL
from xml.dom import minidom
import sys
import traceback


VIDIOC_QUERYCAP=struct.unpack('i', struct.pack('I', 0x80685600))[0]
#UVCIOC_CTRL_ADD=struct.unpack('i', struct.pack('I', 0x40185501))[0]
UVCIOC_CTRL_MAP=struct.unpack('i', struct.pack('I', 0xc0607520))[0]

class Control:

    pass

class Mapping:

    pass


class ConfigFile:
    __requests={"SET_CUR": (1 << 0),
                "GET_CUR": (1 << 1),
                "GET_MIN": (1 << 2),
                "GET_MAX": (1 << 3),
                "GET_RES": (1 << 4),
                "GET_LEN":  0,
                "GET_INFO": 0,
                "GET_DEF": (1 << 5) }

    UVC_CTRL_DATA_TYPE_RAW       = 0
    UVC_CTRL_DATA_TYPE_SIGNED    = 1
    UVC_CTRL_DATA_TYPE_UNSIGNED  = 2
    UVC_CTRL_DATA_TYPE_BOOLEAN   = 3
    UVC_CTRL_DATA_TYPE_ENUM      = 4
    UVC_CTRL_DATA_TYPE_BITMASK   = 5

    V4L2_CTRL_TYPE_INTEGER   = 1
    V4L2_CTRL_TYPE_BOOLEAN   = 2
    V4L2_CTRL_TYPE_MENU          = 3
    V4L2_CTRL_TYPE_BUTTON    = 4
    V4L2_CTRL_TYPE_INTEGER64     = 5
    V4L2_CTRL_TYPE_CTRL_CLASS    = 6



    def __init__(self, filename ):
        self.xmldoc = minidom.parse(filename).documentElement
        self.__constants={}
        self.__controls={}
        self.__mappings=[]
        self.__define_constants()
        self.__define_controls()
        self.__define_mappings()

    def __define_constants(self):
        for el in self.xmldoc.getElementsByTagName('constant'):
            name = el.getElementsByTagName('id')[0].firstChild.data
            value = el.getElementsByTagName('value')[0].firstChild.data
            self.__constants[name] = value

    def __define_controls(self):
        for device in self.xmldoc.getElementsByTagName('device'):
            vendors=[]
            products=[]
            for vendor in device.getElementsByTagName('vendor_id'):
                vendors.append( eval( vendor.firstChild.data, None ) )
            for product in device.getElementsByTagName('product_id'):
                products.append( eval( product.firstChild.data, None ) )

            for el in self.xmldoc.getElementsByTagName('control'):
                ident = el.getAttribute( 'id' )
                entity = self.__constants[el.getElementsByTagName('entity')[0].firstChild.data]
                selector = eval(self.__constants[el.getElementsByTagName('selector')[0].firstChild.data], None)
                index = eval(el.getElementsByTagName('index')[0].firstChild.data, None)
                size = eval(el.getElementsByTagName('size')[0].firstChild.data, None)
                requests = 0
                for r in el.getElementsByTagName('request'):
                    requests = requests | self.__requests[r.firstChild.data]
                self.__controls[ident] = (entity,selector,index,size,requests,vendors,products)

    def __define_mappings(self):
        for el in self.xmldoc.getElementsByTagName('mapping'):
            name = el.getElementsByTagName('name')[0].firstChild.data
            control = self.__controls[el.getElementsByTagName('control_ref')[0].getAttribute('idref')]
            size = eval(el.getElementsByTagName('size')[0].firstChild.data, None)
            offset = eval(el.getElementsByTagName('offset')[0].firstChild.data, None)
            uvc_type = getattr( self, el.getElementsByTagName('uvc_type')[0].firstChild.data )
            v4l2_id = eval( self.__constants[ el.getElementsByTagName('id')[0].firstChild.data ], None )
            v4l2_type = getattr( self, el.getElementsByTagName('v4l2_type')[0].firstChild.data )
            self.__mappings.append( (name, control, size, offset, uvc_type, v4l2_id, v4l2_type) )

    def __parse_guid(self,guid):
        ret=''
        s = guid.split('-')
        i = s[0]
        while i:
            ret += struct.pack( 'B', int(i[-2:],16) )
            i = i[:-2]
        i = s[1]
        while i:
            ret += struct.pack( 'B', int(i[-2:],16) )
            i = i[:-2]
        i = s[2]
        while i:
            ret += struct.pack( 'B', int(i[-2:],16) )
            i = i[:-2]
        i = s[3]
        while i:
            ret += struct.pack( 'B', int(i[:2],16) )
            i = i[2:]
        i = s[4]
        while i:
            ret += struct.pack( 'B', int(i[:2],16) )
            i = i[2:]

        return ret

    def get_control_structs(self,driver):
        ids = get_usb_id(driver)
        for ident in self.__controls:
            ctrl = self.__controls[ident]
            if len( ctrl[5] ) and not ids[0] in ctrl[5]:
                continue
            if len( ctrl[6] ) and not ids[1] in ctrl[6]:
                continue
            s = self.__parse_guid( ctrl[0] )
            s += struct.pack( 'BBHL', ctrl[2], ctrl[1], ctrl[3], ctrl[4] )
            yield s,ident

    def get_mapping_structs(self):
        for mapping in self.__mappings:
            s = struct.pack( 'I', mapping[5] ) # id
            print len(s),
            s += str(mapping[0]) + (32 - len( mapping[0] )) * struct.pack( 'B', 0 ) # name ( 32 bytes )
            print len(s),
            s += self.__parse_guid( mapping[1][0] ) # guid
            print len(s),
            s += struct.pack( 'B', mapping[1][1] ) # selector
            print len(s),
            s += struct.pack( 'B', mapping[2] ) # size
            print len(s),
            s += struct.pack( 'B', mapping[3] ) # offset
            s += struct.pack( 'B', 0 ) # padding
            print "offset:",len(s),
            s += struct.pack( 'I', mapping[6] ) # v4l2_type
            print len(s),
            s += struct.pack( 'I', mapping[4] ) # data_type
            print len(s),
            s += struct.pack( 'Q', 0 ) # menu_info.value
            print len(s),
            s += struct.pack ('I', 0) # menu_info.count
            print len(s),
            s += 4 * struct.pack ('I', 0) # reserved
            print len(s)
            yield s

def get_usb_id(driver):
    vendorid = None
    productid = None
    try:
        devpath = os.path.normpath( os.path.realpath( '/sys/class/video4linux/%s/device/' % ( os.path.basename(driver[1]) ) ) + '/../' )
        f = open( devpath + '/idVendor' )
        vendorid = int(f.readline().strip(),16)
        f.close()
        f = open( devpath + '/idProduct' )
        productid = int(f.readline().strip(),16)
        f.close()
    except:
        traceback.print_exc()
        pass
    return ( vendorid, productid )

def get_uvcvideo_drivers():
    drivers = []
    for dev in glob.glob('/dev/video[0-9]'):
        try:
            fd = open(dev)
            if fcntl.ioctl( fd, VIDIOC_QUERYCAP, 'uvcvidex' ) == 'uvcvideo' :
                drivers.append( (fd,dev) )
            else:
                fd.close()
        except:
            print "Failed to open:",dev

    return drivers


if __name__ == '__main__':
    try:
        cf = ConfigFile(sys.argv[1])
    except Exception, e:
        print( "Failed to load config file: %s" % (e.message, ) )
        traceback.print_exc()
        exit( 1 )

    drivers = get_uvcvideo_drivers()
    if len(drivers) == 0:
        print( "uvcvideo driver not loaded or missing permissions" )
        exit(1)

    # add controls
    #for d in drivers:
    #	print "Driver :", str(d)
    #    for s,ident in cf.get_control_structs(d):
    #        try:
    #            fcntl.ioctl( drivers[0][0], UVCIOC_CTRL_ADD, s )
    #        except Exception, e:
    #            print( "Failed to add control '%s': %s" % (ident, e.message, ))
    #            #traceback.print_exc()
    #            pass

    # add mappings
    for s in cf.get_mapping_structs():
        try:
        #for c in s:
        #	print hex(ord(c)),
        #print s
        #UVCIOC_CTRL_MAP = IOCTL._IOWR (ord('u'), 0x20, len(s)*"B")
            fcntl.ioctl( drivers[0][0], UVCIOC_CTRL_MAP, s )
            print( "Mapped control '%s'" % ( s[4:36] ) )
        except IOError, (errno, strerror):
            print( "Failed to map control '%s': %s" % (s[4:36],strerror) )
        except:
            print "Failed to map controls"
            traceback.print_exc()


    for d in drivers:
        d[0].close()
