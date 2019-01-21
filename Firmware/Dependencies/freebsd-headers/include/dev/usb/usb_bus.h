/* $FreeBSD: release/9.0.0/sys/dev/usb/usb_bus.h 215802 2010-11-24 19:11:32Z weongyo $ */
/*-
 * Copyright (c) 2008 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _USB_BUS_H_
#define	_USB_BUS_H_

/*
 * The following structure defines the USB explore message sent to the USB
 * explore process.
 */

struct usb_bus_msg {
	struct usb_proc_msg hdr;
	struct usb_bus *bus;
};

/*
 * The following structure defines the USB statistics structure.
 */
struct usb_bus_stat {
	uint32_t uds_requests[4];
};

/*
 * The following structure defines an USB BUS. There is one USB BUS
 * for every Host or Device controller.
 */
struct usb_bus {
	struct usb_bus_stat stats_err;
	struct usb_bus_stat stats_ok;
	struct root_hold_token *bus_roothold;
	/*
	 * There are two callback processes. One for Giant locked
	 * callbacks. One for non-Giant locked callbacks. This should
	 * avoid congestion and reduce response time in most cases.
	 */
	struct usb_process giant_callback_proc;
	struct usb_process non_giant_callback_proc;

	/* Explore process */
	struct usb_process explore_proc;

	/* Control request process */
	struct usb_process control_xfer_proc;

	struct usb_bus_msg explore_msg[2];
	struct usb_bus_msg detach_msg[2];
	struct usb_bus_msg attach_msg[2];
	/*
	 * This mutex protects the USB hardware:
	 */
	struct mtx bus_mtx;
	struct usb_xfer_queue intr_q;
	struct usb_callout power_wdog;	/* power management */

	device_t parent;
	device_t bdev;			/* filled by HC driver */

#if USB_HAVE_BUSDMA
	struct usb_dma_parent_tag dma_parent_tag[1];
	struct usb_dma_tag dma_tags[USB_BUS_DMA_TAG_MAX];
#endif
	struct usb_bus_methods *methods;	/* filled by HC driver */
	struct usb_device **devices;

	struct ifnet *ifp;	/* only for USB Packet Filter */

	usb_power_mask_t hw_power_state;	/* see USB_HW_POWER_XXX */
	usb_size_t uframe_usage[USB_HS_MICRO_FRAMES_MAX];

	uint16_t isoc_time_last;	/* in milliseconds */

	uint8_t	alloc_failed;		/* Set if memory allocation failed. */
	uint8_t	driver_added_refcount;	/* Current driver generation count */
	enum usb_revision usbrev;	/* USB revision. See "USB_REV_XXX". */

	uint8_t	devices_max;		/* maximum number of USB devices */
	uint8_t	do_probe;		/* set if USB BUS should be re-probed */

	/* 
	 * The scratch area can only be used inside the explore thread
	 * belonging to the give serial bus.
	 */
	union {
		struct usb_hw_ep_scratch hw_ep_scratch[1];
		struct usb_temp_setup temp_setup[1];
		uint8_t	data[255];
	}	scratch[1];
};

#endif					/* _USB_BUS_H_ */
