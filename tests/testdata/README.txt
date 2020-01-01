Short summary of dirs used by apk:

 /etc/apk
 /etc/apk/keys
 /lib/apk
 /lib/apk/db
 /var/cache/apk
 /var/cache/misc

Reference used:

static int apk_db_create(struct apk_database *db)
{
	int fd;

	mkdirat(db->root_fd, "tmp", 01777);
	mkdirat(db->root_fd, "dev", 0755);
	mknodat(db->root_fd, "dev/null", S_IFCHR | 0666, makedev(1, 3));
	mkdirat(db->root_fd, "etc", 0755);
	mkdirat(db->root_fd, "etc/apk", 0755);
	mkdirat(db->root_fd, "lib", 0755);
	mkdirat(db->root_fd, "lib/apk", 0755);
	mkdirat(db->root_fd, "lib/apk/db", 0755);
	mkdirat(db->root_fd, "var", 0755);
	mkdirat(db->root_fd, "var/cache", 0755);
	mkdirat(db->root_fd, "var/cache/apk", 0755);
	mkdirat(db->root_fd, "var/cache/misc", 0755);

	fd = openat(db->root_fd, apk_world_file, O_CREAT|O_RDWR|O_TRUNC|O_CLOEXEC, 0644);
	if (fd < 0)
		return -errno;
	close(fd);

	return 0;
}
