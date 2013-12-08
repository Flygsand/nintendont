#ifndef __IPC_H__
#define __IPC_H__	1

#define IPC_EINVAL -1

struct ioctl_vector {
	void *data;
	unsigned int len;
}  __attribute__((packed)) ;


/* IOCTL vector */
typedef struct iovec {
	void *data;
	u32   len;
} ioctlv;

#define IOS_OPEN				0x01
#define IOS_CLOSE				0x02
#define IOS_READ				0x03
#define IOS_WRITE				0x04
#define IOS_SEEK				0x05
#define IOS_IOCTL				0x06
#define IOS_IOCTLV				0x07

#endif
