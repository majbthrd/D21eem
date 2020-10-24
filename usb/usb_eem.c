/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Peter Lawrence
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdbool.h>
#include <stdalign.h>
#include <string.h>
#include <stddef.h>
#include "utils.h"
#include "usb.h"
#include "usb_std.h"
#include "usb_eem.h"

static alignas(4) uint8_t received[EEM_MAX_SEGMENT_SIZE + 4 /* CRC-32 */];
static alignas(4) uint8_t transmitted[EEM_MAX_SEGMENT_SIZE + 4 /* CRC-32 */];

static bool can_xmit;

static void usb_eem_ep_send_callback(int size);
static void usb_eem_ep_recv_callback(int size);

void usb_eem_init(void)
{
  usb_set_callback(USB_EEM_EP_SEND, usb_eem_ep_send_callback);
  usb_set_callback(USB_EEM_EP_RECV, usb_eem_ep_recv_callback);
}

static void usb_eem_send(uint8_t *data, int size)
{
  usb_send(USB_EEM_EP_SEND, data, size);
}

void usb_eem_recv_renew(void)
{
  usb_recv(USB_EEM_EP_RECV, received, sizeof(received));
}

void usb_configuration_callback(int config)
{
  (void)config;

  usb_eem_recv_renew();
  can_xmit = true;
}


static void usb_eem_ep_send_callback(int size)
{
  (void)size;

  can_xmit = true;
}

struct cdc_eem_packet_header
{
  uint16_t length:14;
  uint16_t bmCRC:1;
  uint16_t bmType:1;
};

static void usb_eem_ep_recv_callback(int size)
{
  struct cdc_eem_packet_header *hdr = (struct cdc_eem_packet_header *)received;

  (void)size;

  if (hdr->bmType)
  {
    /* EEM Control Packet: discard it */
    usb_eem_recv_renew();
    return;
  }

  usb_eem_recv_callback(received + sizeof(struct cdc_eem_packet_header), size - sizeof(struct cdc_eem_packet_header) - 4 /* CRC-32 */);
}

bool usb_class_handle_request(usb_request_t *request)
{
  int length = request->wLength;

  return false;
}

bool usb_eem_can_xmit(void)
{
  return can_xmit;
}

void usb_eem_xmit_packet(struct pbuf *p)
{
  struct pbuf *q;
  uint8_t *data;
  int packet_size;

  if (!can_xmit)
    return;

  data = transmitted + sizeof(struct cdc_eem_packet_header);
  packet_size = sizeof(struct cdc_eem_packet_header);
  for(q = p; q != NULL; q = q->next)
  {
    memcpy(data, (char *)q->payload, q->len);
    data += q->len;
    packet_size += q->len;
    if (q->tot_len == q->len) break;
  }

  struct cdc_eem_packet_header *hdr = (struct cdc_eem_packet_header *)transmitted;
  /* append a fake CRC-32; the standard allows 0xDEADBEEF, which takes less CPU time */
  data[0] = 0xDE; data[1] = 0xAD; data[2] = 0xBE; data[3] = 0xEF;
  /* adjust length to reflect added fake CRC-32 */
  packet_size += 4;
  hdr->bmType = 0; /* EEM Data Packet */
  hdr->length = packet_size - sizeof(struct cdc_eem_packet_header);
  hdr->bmCRC = 0; /* Ethernet Frame CRC-32 set to 0xDEADBEEF */

  can_xmit = false;
  usb_eem_send(transmitted, packet_size + sizeof(struct cdc_eem_packet_header));
}
