#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <linux/uinput.h>

#include <libevdev-1.0/libevdev/libevdev.h>
#include <libevdev-1.0/libevdev/libevdev-uinput.h>

#include <fcntl.h>
#include <unistd.h>

static const char* pwrkey = "/dev/input/event0";
static const char* joypad = "/dev/input/event2";

void brightness(int direction) {
	FILE* fp = fopen("/sys/devices/platform/backlight/backlight/backlight/brightness", "r+");
	int brightness = 0;

	fscanf(fp, "%i", &brightness);

	if (brightness > 0 && brightness < 255 && direction != 0) {
		fprintf(fp, "%d", (brightness + direction));
	}
	else {
		if (brightness > 0 && direction == 0) {
			fprintf(fp, "%d", 0);
		}
		else {
			fprintf(fp, "%d", 30);
		}
	}

	fclose(fp);
}

int r3 = 0;

void handle_event(int type, int code, int value) {
	if (type == 1) {
		// power key
		if (code == 116 && value == 1) {
			if (r3) {
				r3 = 0;
				system("/bin/systemctl poweroff || true");
			}
			else {
				system("/bin/systemctl suspend || true");
			}
		}
		else if (code == 311 && value == 1) { // select
			if (r3) {
				r3 = 0;
				brightness(0);
			}
		}
		else if (code == 313 && value == 2) { // r3 - hold down
			r3 = 1;
		}
	}

	// dpad
	if (type == 3) {
		if (r3) {
			r3 = 0;

			if (code == 17 && value == -1) { // up
				brightness(1);
			}
			else if (code == 17 && value == 1) { // down
				brightness(-1);
			}
		}
	}

	//printf("type:%d code: %d value: %d\n", type, code, value);
}

int main () {
	struct libevdev* dev_pwrkey;
	int fd_ev_pwrkey;
	int rc_pwrkey;

	struct libevdev* dev_joypad;
	int fd_ev_joypad;
	int rc_joypad;

	fd_ev_pwrkey = open(pwrkey, O_RDONLY|O_NONBLOCK);
	rc_pwrkey = libevdev_new_from_fd(fd_ev_pwrkey, &dev_pwrkey);

	fd_ev_joypad = open(joypad, O_RDONLY|O_NONBLOCK);
	rc_joypad = libevdev_new_from_fd(fd_ev_joypad, &dev_joypad);

	struct input_event ev_pwrkey;
	struct input_event ev_joypad;

	int quit = 0;
	while (!quit) {
		rc_pwrkey = libevdev_next_event(dev_pwrkey, LIBEVDEV_READ_FLAG_NORMAL, &ev_pwrkey);
		rc_joypad = libevdev_next_event(dev_joypad, LIBEVDEV_READ_FLAG_NORMAL, &ev_joypad);

        if (rc_pwrkey == LIBEVDEV_READ_STATUS_SUCCESS) {
        	handle_event(ev_pwrkey.type, ev_pwrkey.code, ev_pwrkey.value);
        }

        if (rc_joypad == LIBEVDEV_READ_STATUS_SUCCESS) {
        	handle_event(ev_joypad.type, ev_joypad.code, ev_joypad.value);
        }
    }

    libevdev_free(dev_pwrkey);
    libevdev_free(dev_joypad);
    close(fd_ev_pwrkey);
    close(fd_ev_joypad);
}
