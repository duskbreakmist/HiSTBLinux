/*
 * Rootfs boot argument selector.
 */

#include <common.h>
#include <command.h>

#define BOOTROOT_ENV_SELECT "rootfs_sel"
#ifndef CONFIG_BOOTROOT_AUTOSELECT_TIMEOUT
#define CONFIG_BOOTROOT_AUTOSELECT_TIMEOUT 5
#endif

struct bootroot_option {
	const char *id;
	const char *name;
	const char *bootargs;
};

static const struct bootroot_option bootroot_options[] = {
	{
		"1",
		"SATA/USB: /dev/sda4 ext4",
		"console=ttyAMA0,115200 root=/dev/sda4 rootfstype=ext4 rootwait "
		"blkdevparts=mmcblk0:1M(boot),1M(bootargs),18M(kernel),3800M(rootfs),-(others) "
		"video=HDMI-A-1:1920x1080@60",
	},
	{
		"2",
		"eMMC: /dev/mmcblk0p4 ext4",
		"console=ttyAMA0,115200 root=/dev/mmcblk0p4 rootfstype=ext4 rootwait "
		"blkdevparts=mmcblk0:1M(boot),1M(bootargs),18M(kernel),3800M(rootfs),-(others) "
		"video=HDMI-A-1:1920x1080@60",
	},
	{
		"3",
		"SD card: /dev/mmcblk1p1 ext4",
		"console=ttyAMA0,115200 root=/dev/mmcblk1p1 rootfstype=ext4 rootwait "
		"blkdevparts=mmcblk0:1M(boot),1M(bootargs),18M(kernel),3800M(rootfs),-(others) "
		"video=HDMI-A-1:1920x1080@60",
	},
};

static const struct bootroot_option *bootroot_find(const char *id)
{
	int i;

	if (!id)
		return NULL;

	for (i = 0; i < ARRAY_SIZE(bootroot_options); i++) {
		if (strcmp(id, bootroot_options[i].id) == 0)
			return &bootroot_options[i];
	}

	if (strcmp(id, "sda4") == 0 || strcmp(id, "sata") == 0 ||
	    strcmp(id, "usb") == 0)
		return &bootroot_options[0];
	if (strcmp(id, "mmc0") == 0 || strcmp(id, "emmc") == 0 ||
	    strcmp(id, "btrfs") == 0)
		return &bootroot_options[1];
	if (strcmp(id, "mmc1") == 0 || strcmp(id, "sd") == 0 ||
	    strcmp(id, "debug") == 0)
		return &bootroot_options[2];

	return NULL;
}

static const struct bootroot_option *bootroot_current(void)
{
	char *sel;
	char *bootargs;

	bootargs = getenv("bootargs");
	if (bootargs) {
		if (strstr(bootargs, "root=/dev/sda4"))
			return &bootroot_options[0];
		if (strstr(bootargs, "root=/dev/mmcblk0p4"))
			return &bootroot_options[1];
		if (strstr(bootargs, "root=/dev/mmcblk1p1"))
			return &bootroot_options[2];
	}

	sel = getenv(BOOTROOT_ENV_SELECT);
	return bootroot_find(sel);
}

static void bootroot_print_list(void)
{
	const struct bootroot_option *cur;
	char *bootargs;
	int i;

	cur = bootroot_current();

	puts("Rootfs boot options:\n");
	for (i = 0; i < ARRAY_SIZE(bootroot_options); i++) {
		printf("  %s) %s%s\n", bootroot_options[i].id,
		       bootroot_options[i].name,
		       cur == &bootroot_options[i] ? " [current]" : "");
	}

	bootargs = getenv("bootargs");
	if (bootargs)
		printf("\nbootargs=%s\n", bootargs);
}

static int bootroot_apply(const struct bootroot_option *opt, int save)
{
	if (setenv(BOOTROOT_ENV_SELECT, (char *)opt->id))
		return 1;

	if (setenv("bootargs", (char *)opt->bootargs))
		return 1;

	printf("Selected rootfs %s: %s\n", opt->id, opt->name);
	printf("bootargs=%s\n", opt->bootargs);

	if (!save)
		return 0;

#if defined(CONFIG_CMD_SAVEENV) && !defined(CONFIG_ENV_IS_NOWHERE)
	if (run_command("saveenv", 0) < 0)
		return 1;
#else
	puts("Environment save is not available; selection is temporary.\n");
#endif

	return 0;
}

static int bootroot_prompt(void)
{
	const struct bootroot_option *cur;
	const struct bootroot_option *opt;
	char select[16];
	int len;

	cur = bootroot_current();
	bootroot_print_list();

	if (cur)
		printf("\nPress ENTER to keep %s.\n", cur->id);

	len = readline_into_buffer("Select rootfs: ", select);
	if (len < 0)
		return 1;

	if (select[0] == '\0') {
		if (cur)
			printf("Keep rootfs %s: %s\n", cur->id, cur->name);
		return 0;
	}

	opt = bootroot_find(select);
	if (!opt) {
		printf("Invalid rootfs selection: %s\n", select);
		return 1;
	}

	return bootroot_apply(opt, 1);
}

int bootroot_autoselect(int timeout)
{
	const struct bootroot_option *cur;
	const struct bootroot_option *opt;
	int remain;

	if (timeout <= 0)
		timeout = CONFIG_BOOTROOT_AUTOSELECT_TIMEOUT;

	cur = bootroot_current();
	bootroot_print_list();

	if (cur)
		printf("\nDefault rootfs %s: %s\n", cur->id, cur->name);

	puts("Press 1/2/3 to select rootfs, ENTER to keep current, any other key to stop autoboot.\n");

	for (remain = timeout; remain > 0; remain--) {
		int i;

		printf("Autoboot in %d seconds: ", remain);

		for (i = 0; i < 100; i++) {
			if (tstc()) {
				char c = getc();

				putc('\n');
				if (c == '\r' || c == '\n') {
					if (cur)
						printf("Keep rootfs %s: %s\n",
						       cur->id, cur->name);
					return 0;
				}

				if (c >= '1' && c <= '3') {
					char id[2];

					id[0] = c;
					id[1] = '\0';
					opt = bootroot_find(id);
					if (opt)
						return bootroot_apply(opt, 1);
				}

				puts("Stop autoboot.\n");
				return 1;
			}
			udelay(10000);
		}

		puts("\r");
	}

	putc('\n');
	return 0;
}

int do_bootroot(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const struct bootroot_option *opt;
	int save;

	if (argc == 1)
		return bootroot_prompt();

	if (argc == 2 &&
	    (strcmp(argv[1], "show") == 0 || strcmp(argv[1], "list") == 0)) {
		bootroot_print_list();
		return 0;
	}

	if (argc < 2 || argc > 3) {
		cmd_usage(cmdtp);
		return 1;
	}

	save = 1;
	if (argc == 3) {
		if (strcmp(argv[2], "nosave") != 0) {
			cmd_usage(cmdtp);
			return 1;
		}
		save = 0;
	}

	opt = bootroot_find(argv[1]);
	if (!opt) {
		printf("Invalid rootfs selection: %s\n", argv[1]);
		cmd_usage(cmdtp);
		return 1;
	}

	return bootroot_apply(opt, save);
}

U_BOOT_CMD(
	bootroot, 3, 0, do_bootroot,
	"select and save rootfs bootargs",
	"[1|2|3|sda4|emmc|sd] [nosave]\n"
	"    - set bootargs for a rootfs option and save it\n"
	"bootroot\n"
	"    - show an interactive rootfs menu\n"
	"bootroot show\n"
	"    - show rootfs options and current bootargs"
);
