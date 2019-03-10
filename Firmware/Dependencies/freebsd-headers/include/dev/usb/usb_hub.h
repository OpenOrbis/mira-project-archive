/* $FreeBSD: release/9.0.0/sys/dev/usb/usb_hub.h 213435 2010-10-04 23:18:05Z hselasky $ */
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

#ifndef _USB_HUB_H_
#define	_USB_HUB_H_

/*
 * The following structure defines an USB port. 
 */
struct usb_port {
	uint8_t	restartcnt;
#define	USB_RESTART_MAX 5
	uint8_t	device_index;		/* zero means not valid */
	enum usb_hc_mode usb_mode;	/* host or device mode */
};

/*
 * The following structure defines how many bytes are
 * left in an 1ms USB time slot.
 */
struct usb_fs_isoc_schedule {
	uint16_t total_bytes;
	uint8_t	frame_bytes;
	uint8_t	frame_slot;
};

/*
 * The following structure defines an USB HUB.
 */
struct usb_hub {
#if USB_HAVE_TT_SUPPORT
	struct usb_fs_isoc_schedule fs_isoc_schedule[USB_ISOC_TIME_MAX];
#endif
	struct usb_device *hubudev;	/* the HUB device */
	usb_error_t (*explore) (struct usb_device *hub);
	void   *hubsoftc;
	usb_size_t uframe_usage[USB_HS_MICRO_FRAMES_MAX];
	uint16_t portpower;		/* mA per USB port */
	uint8_t	isoc_last_time;
	uint8_t	nports;
	struct usb_port ports[0];
};

/* function prototypes */

void	usb_hs_bandwidth_alloc(struct usb_xfer *xfer);
void	usb_hs_bandwidth_free(struct usb_xfer *xfer);
void	usbd_fs_isoc_schedule_init_all(struct usb_fs_isoc_schedule *fss);
void	usb_bus_port_set_device(struct usb_bus *bus, struct usb_port *up,
	    struct usb_device *udev, uint8_t device_index);
struct usb_device *usb_bus_port_get_device(struct usb_bus *bus,
	    struct usb_port *up);
void	usb_needs_explore(struct usb_bus *bus, uint8_t do_probe);
void	usb_needs_explore_all(void);
void	usb_bus_power_update(struct usb_bus *bus);
void	usb_bus_powerd(struct usb_bus *bus);
void	uhub_root_intr(struct usb_bus *, const uint8_t *, uint8_t);
usb_error_t uhub_query_info(struct usb_device *, uint8_t *, uint8_t *);

#endif					/* _USB_HUB_H_ */
