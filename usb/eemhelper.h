/*
    USB descriptor macros for CDC-EEM

    Copyright (C) 2015,2016,2018,2020 Peter Lawrence

    Permission is hereby granted, free of charge, to any person obtaining a 
    copy of this software and associated documentation files (the "Software"), 
    to deal in the Software without restriction, including without limitation 
    the rights to use, copy, modify, merge, publish, distribute, sublicense, 
    and/or sell copies of the Software, and to permit persons to whom the 
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in 
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.
*/

#ifndef __EEM_HELPER_H
#define __EEM_HELPER_H

#include <stdint.h>
#include "usbhelper.h"

#define USB_FS_MAX_PACKET_SIZE   64

#define EEM_MAX_USB_DATA_PACKET_SIZE   USB_FS_MAX_PACKET_SIZE
#define EEM_MAX_USB_NOTIFY_PACKET_SIZE USB_FS_MAX_PACKET_SIZE
#define EEM_MAX_SEGMENT_SIZE           1514

struct eem_interface
{
  struct interface_descriptor             dat_interface;
  struct endpoint_descriptor              ep_in;
  struct endpoint_descriptor              ep_out;
};

/* macro to help generate CDC EEM USB descriptors */

#define EEM_DESCRIPTOR(DATA_ITF, DATAOUT_EP, DATAIN_EP) \
    { \
      .dat_interface = { \
        /*Data class interface descriptor*/ \
        .bLength            = sizeof(struct interface_descriptor),             /* Endpoint Descriptor size */ \
        .bDescriptorType    = USB_INTERFACE_DESCRIPTOR,                        \
        .bInterfaceNumber   = DATA_ITF,                                        /* Number of Interface */ \
        .bAlternateSetting  = 0x00,                                            /* Alternate setting */ \
        .bNumEndpoints      = 0x02,                                            /* Two endpoints used */ \
        .bInterfaceClass    = 0x02,                                            /* Communications */ \
        .bInterfaceSubclass = 0x0C,                                            /* EEM */ \
        .bInterfaceProtocol = 0x07,                                            /* EEM */ \
        .iInterface         = 0x00,                                            \
      }, \
 \
      .ep_in = { \
        /* Data Endpoint OUT Descriptor */ \
        .bLength            = sizeof(struct endpoint_descriptor),              /* Endpoint Descriptor size */ \
        .bDescriptorType    = USB_ENDPOINT_DESCRIPTOR,                         /* Endpoint */ \
        .bEndpointAddress   = DATAOUT_EP,                                      \
        .bmAttributes       = 0x02,                                            /* Bulk */ \
        .wMaxPacketSize     = USB_UINT16(EEM_MAX_USB_DATA_PACKET_SIZE),        \
        .bInterval          = 0x00,                                            /* ignore for Bulk transfer */ \
      }, \
 \
      .ep_out = { \
        /* Data Endpoint IN Descriptor*/ \
        .bLength            = sizeof(struct endpoint_descriptor),              /* Endpoint Descriptor size */ \
        .bDescriptorType    = USB_ENDPOINT_DESCRIPTOR,                         /* Endpoint */ \
        .bEndpointAddress   = DATAIN_EP,                                       \
        .bmAttributes       = 0x02,                                            /* Bulk */ \
        .wMaxPacketSize     = USB_UINT16(EEM_MAX_USB_DATA_PACKET_SIZE),        \
        .bInterval          = 0x00                                             /* ignore for Bulk transfer */ \
      }, \
    },

#endif /* __EEM_HELPER_H */
