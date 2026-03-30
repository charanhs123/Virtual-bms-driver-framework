#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../kernel/vbms_ioctl.h"

static void print_status(const struct vbms_status *status)
{
    printf("control=%u\n", status->control);
    printf("status=%u\n", status->status);
    printf("fault=%u\n", status->fault);
    printf("voltage_mv=%u\n", status->voltage_mv);
    printf("temperature_cx10=%u\n", status->temperature_cx10);
    printf("ov_threshold_mv=%u\n", status->overvoltage_mv);
    printf("ot_threshold_cx10=%u\n", status->overtemperature_cx10);
}

static void usage(const char *prog)
{
    printf("Usage:\n");
    printf("  %s enable_meas <0|1>\n", prog);
    printf("  %s enable_faults <0|1>\n", prog);
    printf("  %s set_thresholds <ov_mv> <ot_cx10>\n", prog);
    printf("  %s push <voltage_mv> <temp_cx10>\n", prog);
    printf("  %s get_status\n", prog);
    printf("  %s get_thresholds\n", prog);
    printf("  %s clear_faults\n", prog);
}

int main(int argc, char *argv[])
{
    int fd;
    const char *dev = "/dev/vbms0";

    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    fd = open(dev, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (strcmp(argv[1], "enable_meas") == 0) {
        uint32_t value;

        if (argc != 3) {
            usage(argv[0]);
            close(fd);
            return 1;
        }

        value = (uint32_t)atoi(argv[2]);

        if (ioctl(fd, VBMS_IOCTL_ENABLE_MEAS, &value) < 0) {
            perror("ioctl enable_meas");
        }

    } else if (strcmp(argv[1], "enable_faults") == 0) {
        uint32_t value;

        if (argc != 3) {
            usage(argv[0]);
            close(fd);
            return 1;
        }

        value = (uint32_t)atoi(argv[2]);

        if (ioctl(fd, VBMS_IOCTL_ENABLE_FAULTS, &value) < 0) {
            perror("ioctl enable_faults");
        }

    } else if (strcmp(argv[1], "set_thresholds") == 0) {
        struct vbms_thresholds t;

        if (argc != 4) {
            usage(argv[0]);
            close(fd);
            return 1;
        }

        t.overvoltage_mv = (uint32_t)atoi(argv[2]);
        t.overtemperature_cx10 = (uint32_t)atoi(argv[3]);

        if (ioctl(fd, VBMS_IOCTL_SET_THRESHOLDS, &t) < 0) {
            perror("ioctl set_thresholds");
        }

    } else if (strcmp(argv[1], "push") == 0) {
        struct vbms_measurement m;

        if (argc != 4) {
            usage(argv[0]);
            close(fd);
            return 1;
        }

        m.voltage_mv = (uint32_t)atoi(argv[2]);
        m.temperature_cx10 = (uint32_t)atoi(argv[3]);

        if (ioctl(fd, VBMS_IOCTL_PUSH_MEASUREMENT, &m) < 0) {
            perror("ioctl push");
        }

    } else if (strcmp(argv[1], "get_status") == 0) {
        struct vbms_status s;

        if (ioctl(fd, VBMS_IOCTL_GET_STATUS, &s) < 0) {
            perror("ioctl get_status");
        } else {
            print_status(&s);
        }

    } else if (strcmp(argv[1], "get_thresholds") == 0) {
        struct vbms_thresholds t;

        if (ioctl(fd, VBMS_IOCTL_GET_THRESHOLDS, &t) < 0) {
            perror("ioctl get_thresholds");
        } else {
            printf("overvoltage_mv=%u\n", t.overvoltage_mv);
            printf("overtemperature_cx10=%u\n", t.overtemperature_cx10);
        }

    } else if (strcmp(argv[1], "clear_faults") == 0) {
        if (ioctl(fd, VBMS_IOCTL_CLEAR_FAULTS) < 0) {
            perror("ioctl clear_faults");
        }

    } else {
        usage(argv[0]);
    }

    close(fd);
    return 0;
}
