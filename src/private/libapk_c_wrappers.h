// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

/**
 * Since libapk stock headers can not be compiled by
 * C++ compiler, create wrapped interface contained in
 * C files so that we can compile them using C compiler
 **/

// Copied from libapk's apk_database.h
#define APK_OPENF_READ              0x0001
#define APK_OPENF_WRITE             0x0002
#define APK_OPENF_CREATE            0x0004
#define APK_OPENF_NO_INSTALLED      0x0010
#define APK_OPENF_NO_SCRIPTS        0x0020
#define APK_OPENF_NO_WORLD          0x0040
#define APK_OPENF_NO_SYS_REPOS      0x0100
#define APK_OPENF_NO_INSTALLED_REPO 0x0200
#define APK_OPENF_CACHE_WRITE       0x0400
#define APK_OPENF_NO_AUTOUPDATE     0x0800
#define APK_OPENF_NO_REPOS  (APK_OPENF_NO_SYS_REPOS | APK_OPENF_NO_INSTALLED_REPO)
#define APK_OPENF_NO_STATE  (APK_OPENF_NO_INSTALLED | APK_OPENF_NO_SCRIPTS | APK_OPENF_NO_WORLD)

#define APK_REPOSITORY_CACHED           0
#define APK_REPOSITORY_FIRST_CONFIGURED 1

#define APK_DEFAULT_REPOSITORY_TAG  0
#define APK_DEFAULT_PINNING_MASK    BIT(APK_DEFAULT_REPOSITORY_TAG)

// Copied from apk_solver.h
#define APK_SOLVERF_UPGRADE         0x0001
#define APK_SOLVERF_AVAILABLE       0x0002
#define APK_SOLVERF_REINSTALL       0x0004
#define APK_SOLVERF_LATEST          0x0008
#define APK_SOLVERF_IGNORE_CONFLICT 0x0010
#define APK_SOLVERF_INSTALLED       0x0020

#ifdef __cplusplus
extern "C" {
#endif

const char *w_apk_error_str(int e);

// wraps setting ::apk_progress_fd from apk_print.h
void w_set_apk_progress_fd(int fd);
int w_get_apk_progress_fd();

struct apk_database;
struct apk_atom_pool;
struct apk_repository;
struct apk_dependency_array;
struct apk_changeset;
struct apk_package;

struct w_apk_database; // forward-decl

struct w_apk_database
{
    struct apk_database *db;
};

struct w_apk_database *w_db_open(unsigned long open_flags, const char *fakeRootPath);
void w_db_close(struct w_apk_database *wdb);

// wraps apk_database->open_complete
bool w_db_is_open_complete(const struct apk_database *db);
// wraps apk_database->open_complete->num_repos
unsigned int w_db_get_num_repos(const struct apk_database *db);
// wraps apk_database->repos[i]
struct apk_repository *w_db_get_repo(struct apk_database *db, int i);
// wraps db->repo_update_counter
unsigned int w_db_get_repo_update_counter(const struct apk_database *db);
// wraps db->repo_update_errors
unsigned int w_db_get_repo_update_errors(const struct apk_database *db);
// wraps db->available.packages.num_items
int w_db_get_get_available_packages_count(const struct apk_database *db);
// wraps apk_db_check_world()
int w_db_check_world(struct apk_database *db);

bool w_db_has_installed(const struct apk_database *db);

typedef void (*ENUMERATE_INSTALLED_CB)(struct apk_package *, void *);
typedef int (*ENUMERATE_AVAILABLE_CB)(void *, void *);
void w_db_enumerate_installed(const struct apk_database *db, ENUMERATE_INSTALLED_CB cb, void *cb_param);
int w_db_enumerate_available(struct apk_database *db, ENUMERATE_AVAILABLE_CB cb, void *cb_param);

// apk_repository_update() // is not public?? why, libapk??
// return 0 on success
int w_db_repository_update(struct apk_database *db, int iRepo, bool allow_untrusted);
const char *w_db_get_repo_url(const struct apk_database *db, int iRepo);
const char *w_db_get_repo_desc(const struct apk_database *db, int iRepo);


struct apk_changeset *w_create_apk_changeset();
void w_delete_apk_changeset(struct apk_changeset *cs);
// wraps changeset.num_install
int w_apk_changeset_get_num_install(const struct apk_changeset *cs);
// wraps changeset.num_remove
int w_apk_changeset_get_num_remove(const struct apk_changeset *cs);
// wraps changeset.num_adjust
int w_apk_changeset_get_num_adjust(const struct apk_changeset *cs);
// wraps changeset.changes->num
unsigned int w_apk_changeset_get_num_changes(const struct apk_changeset *cs);
// wraps &changeset.changes->item[iChange];
struct apk_change *w_apk_changeset_get_change(struct apk_changeset *cs, int iChange);

// wraps achange->reinstall
bool w_apk_change_is_reinstall(const struct apk_change *c);
struct apk_package *w_apk_change_get_old_pkg(struct apk_change *c);
struct apk_package *w_apk_change_get_new_pkg(struct apk_change *c);

const char *w_apk_package_get_pkg_name(const struct apk_package *pkg);
const char *w_apk_package_get_version(const struct apk_package *pkg);
const char *w_apk_package_get_arch(const struct apk_package *pkg);
const char *w_apk_package_get_license(const struct apk_package *pkg);
const char *w_apk_package_get_origin(const struct apk_package *pkg);
const char *w_apk_package_get_maintainer(const struct apk_package *pkg);
const char *w_apk_package_get_url(const struct apk_package *pkg);
const char *w_apk_package_get_description(const struct apk_package *pkg);
const char *w_apk_package_get_commit(const struct apk_package *pkg);
const char *w_apk_package_get_filename(const struct apk_package *pkg);
time_t w_apk_package_get_buildTime(const struct apk_package *pkg);
size_t w_apk_package_get_size(const struct apk_package *pkg);
size_t w_apk_package_get_installedSize(const struct apk_package *pkg);


// wraps apk_solver_solve
int w_apk_solver_solve(struct apk_database *db, unsigned short solver_flags, struct apk_changeset *cs);
// wraps apk_solver_commit_changeset
int w_apk_solver_commit_changeset(struct apk_database *db, struct apk_changeset *cs);

// more generic high level wrappers

struct w_resolved_apk_dependency
{
    char *name;
    char *version;
};

void w_resolved_dep_free_strings(struct w_resolved_apk_dependency *dep);

// returns 0 on success
int w_apk_add(struct apk_database *db,
              const char *pkgNameSpec,
              unsigned short solver_flags,
              struct w_resolved_apk_dependency *resolved_dep);

// returns 0 on success
int w_apk_del(struct apk_database *db,
              const char *pkgname,
              bool recursive_delete);

#ifdef __cplusplus
} // extern "C"
#endif
