#include <android/log.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>

/*
 * This is a port of NVIDIA's sensors-config binary-only tool. Produced by me,
 * dmitrygr@gmail.com from disassembly of that tool. Why? They kernel directly
 * opens and reads /data/lightsensor/AL3010_Config.ini, using file open apis
 * this is incredibly unsafe in so many ways! Why? Read this article and come
 * back here: http://www.linuxjournal.com/article/8110 . Ok, so how do we I
 * plan to fix this? I modified the kernel to instead add a sysfs node to which
 * one can write the calibration value. Only problem is, that file the kernel
 * reads is only written by sensors-config. Well, so off I went looking for its
 * source, only to find that none exist. I disassembled it, and it was riddled
 * with calls to system(), which further boiled my blood. Luckily it is a rather
 * small executable, so I decided to rewrite it from scratch. And that is what
 * you see here in front of you now. It is a faithful reproduction, including
 * code paths for sensors not present in grouper and some of the bad choices
 * (but not all), and some bad design (but nto all). This way if ever ever come
 * across any other device using this sensors-config binary, you can feel free
 * to trash it and replace it with this one.
 */

#define LOG_TAG "Sensor Daemon"
#define LOGI(...) 	__android_log_print(ANDROID_LOG_INFO , LOG_TAG, __VA_ARGS__)
#define LOGE(...) 	__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


static const char *calPartSensorsPath = "/data/calibration/sensors";
static uid_t uid_system = 1000; //in case we cnanot get this info, this is a backup
static gid_t gid_system = 1000; //in case we cnanot get this info, this is a backup

static bool file_copy(const char *from_file_path, const char *to_file_path)
{
	FILE *fi, *fo;
	bool ret = false;
	char buf[1024];
	
	fi = fopen(from_file_path, "r");
	if (fi) {
 		fo = fopen(to_file_path, "w");
		if (fo) {
			int c;
			
			ret = true;
			while(fgets(buf, sizeof(buf), fi)) {
				if (EOF == fputs(buf, fo)) {
					ret = false;
					break;
				}
			}
			if (ferror(fi) || ferror(fo))
				ret = false;
			fclose(fo);
		}
		fclose(fi);
	}
	
	return ret;
}

static void update_some_file(const char *from, const char *to, const char* nameForErr)
{
			
	if (access(from, R_OK))
		return;

	(void)mkdir(calPartSensorsPath, 0777);
	(void)file_copy(from, to);
	if (access(to, R_OK))
		LOGE("copy file to PER FAIL, path: %s", nameForErr);
}

static void update_ami_file(const char *name)
{
	static const char *amiDirPath = "/data/amit";
	char from[1024];
	char to[1024];
	
	snprintf(from, sizeof(from), "%s/%s", amiDirPath, name);
	snprintf(to, sizeof(to), "%s/%s", calPartSensorsPath, name);
	
	update_some_file(from, to, name);
}

static void update_generic_file(const char *name)
{
	char from[1024];
	char to[1024];
	
	snprintf(from, sizeof(from), "/data/%s", name);
	snprintf(to, sizeof(to), "/data/calibration/sensors/%s", name);
	
	update_some_file(from, to, name);
}

static void copy_file_from_per(const char *name)
{
	char from[1024];
	char to[1024];
	
	snprintf(from, sizeof(from), "/data/calibration/%s", name);
	snprintf(to, sizeof(to), "/data/%s", name);

	if (!file_copy(from, to))
		LOGE("Failed to copy '%s' -> '%s'", from, to);
}

static void set_perms_object(const char *path, mode_t mode)
{
	(void)chown(path, uid_system, gid_system);
	(void*)chmod(path, mode);
}

static void set_perms_file(const char *name, mode_t mode)
{
	char path[1024];
	
	snprintf(path, sizeof(path), "/data/sensors/%s", name);

	set_perms_object(path, mode);
}

int main(int argc, char** argv)
{
	static const char *calPartDev = "/dev/block/platform/sdhci-tegra.3/by-name/PER";
	static const char *calPartMountPt = "/data/calibration";
	struct passwd *p;
	struct group *g;
	
	LOGI("Sensors-load-config: Service started, Version: 1.2.dmitrygr");
	(void)argc;
	(void)argv;
	
	//get uids and gids if we can
	if ((p = getpwnam("system")) != NULL)
		uid_system = p->pw_uid;
	if ((g = getgrnam("system")) != NULL)
		gid_system = g->gr_gid;
	
	//do the weird thigns this thing must do
	(void)mkdir("/data/calibration" ,0777);
	(void)mkdir("/data/sensors", 0777);
	(void)mkdir("/data/lightsensor", 0777);
	(void)mkdir(calPartMountPt, 0777);
	
	//their code mounted and unmounted the FS many times - but there is no point - just do it once here and live with it
	if (mount(calPartDev, calPartMountPt, "vfat", 0, NULL))
		LOGE("Can't mount PER file system !");
	else {
		update_ami_file("AMI304_Config.ini");
		update_ami_file("AMI306_Config.ini");
		update_generic_file("KXTF9_Calibration.ini");
		update_generic_file("Accel_Config.ini");
		update_generic_file("MPU6050_Config.ini");
		
		copy_file_from_per("/sensors/KXTF9_Calibration.ini");
		copy_file_from_per("/sensors/MPU6050_Config.ini");
		copy_file_from_per("/sensors/Accel_Config.ini");
		copy_file_from_per("/sensors/AMI304_Config.ini");
		copy_file_from_per("/sensors/AMI306_Config.ini");
		copy_file_from_per("/lightsensor/AL3010_Config.ini");

		(void)umount(calPartMountPt);
	}
	(void)rmdir(calPartMountPt);
	
	//set permissions, etc
	set_perms_object("/data/sensors", 0751);
	set_perms_object("/data/lightsensor", 0751);
	set_perms_file("/sensors/KXTF9_Calibration.ini", 0751);
	set_perms_file("/sensors/MPU6050_Config.ini", 0751);
	set_perms_file("/sensors/Accel_Config.ini", 0751);
	set_perms_file("/sensors/AMI304_Config.ini", 0751);
	set_perms_file("/sensors/AMI306_Config.ini", 0751);
	set_perms_file("/lightsensor/AL3010_Config.ini", 0644);
	
	(void)chown("/data/sensors/KXTF9_Calibration.ini", uid_system, gid_system);
	
	//now load the lightsensor calibration value into the kernel the proper way (the whole reason we're here)
	file_copy("/data/lightsensor/AL3010_Config.ini", "/sys/devices/platform/tegra-i2c.2/i2c-2/2-001c/calibration");

	return 0;
}
