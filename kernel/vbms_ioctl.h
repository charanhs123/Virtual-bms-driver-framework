 #ifndef VBMS_IOCTL_H
#define VBMS_IOCTL_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define VBMS_IOCTL_MAGIC 'V'

struct vbms_measurement {
	    __u32 voltage_mv;
	        __u32 temperature_cx10;
};

struct vbms_thresholds {
	    __u32 overvoltage_mv;
	        __u32 overtemperature_cx10;
};

struct vbms_status {
	    __u32 control;
	        __u32 status;
		    __u32 fault;
		        __u32 voltage_mv;
			    __u32 temperature_cx10;
			        __u32 overvoltage_mv;
				    __u32 overtemperature_cx10;
};

#define VBMS_IOCTL_SET_THRESHOLDS _IOW(VBMS_IOCTL_MAGIC, 1, struct vbms_thresholds)
#define VBMS_IOCTL_GET_THRESHOLDS _IOR(VBMS_IOCTL_MAGIC, 2, struct vbms_thresholds)
#define VBMS_IOCTL_PUSH_MEASUREMENT _IOW(VBMS_IOCTL_MAGIC, 3, struct vbms_measurement)
#define VBMS_IOCTL_GET_STATUS _IOR(VBMS_IOCTL_MAGIC, 4, struct vbms_status)
#define VBMS_IOCTL_CLEAR_FAULTS _IO(VBMS_IOCTL_MAGIC, 5)
#define VBMS_IOCTL_ENABLE_MEAS _IOW(VBMS_IOCTL_MAGIC, 6, __u32)
#define VBMS_IOCTL_ENABLE_FAULTS _IOW(VBMS_IOCTL_MAGIC, 7, __u32)

#endif
