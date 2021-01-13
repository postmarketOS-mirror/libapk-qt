// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>

#include "libapk_c_wrappers.h"

#include <apk_atom.h>
#include <apk_blob.h>
#include <apk_database.h>
#include <apk_defines.h>
#include <apk_version.h>
#include <apk_solver.h>
#include <apk_package.h>
#include <apk_print.h>

const char *w_apk_error_str(int e)
{
    return apk_error_str(e);
}

// wraps setting ::apk_progress_fd from apk_print.h
void w_set_apk_progress_fd(int fd)
{
    apk_progress_fd = fd;
}

int w_get_apk_progress_fd()
{
    return apk_progress_fd;
}

struct w_apk_database *w_db_open(unsigned long open_flags, const char *fakeRootPath)
{
    struct apk_db_options db_opts;
    struct apk_database *db;
    struct w_apk_database *wdb;
    int r;

    memset(&db_opts, 0, sizeof(db_opts));
    db_opts.open_flags = open_flags;

    // apply "fake" root, if set
    if (fakeRootPath && (strlen(fakeRootPath) > 0)) {
        db_opts.root = strdup(fakeRootPath);
        // Do not load scripts when db is opened for writing
        // in fake root mode. Scripts don't like it and always fail
        if (open_flags & APK_OPENF_WRITE) {
            db_opts.open_flags |= APK_OPENF_NO_SCRIPTS;
        }
    }
    list_init(&db_opts.repository_list);

    wdb = (struct w_apk_database *)malloc(sizeof(struct w_apk_database));
    if (!wdb) {
        return NULL;
    }

    db = (struct apk_database *)malloc(sizeof(struct apk_database));
    memset(db, 0, sizeof(struct apk_database));
    apk_db_init(db);
    r = apk_db_open(db, &db_opts);

    if (r == 0) {
        wdb->db = db;
    } else {
        free(db); // failed to open :(
        free(wdb);
        wdb = NULL;
    }

    return wdb;
}

void w_db_close(struct w_apk_database *wdb)
{
    if (!wdb) return;
    if (wdb->db) {
        if (wdb->db->open_complete) {
            apk_db_close(wdb->db);
        }
    }
    free(wdb->db);
    free(wdb);
}

// wraps struct apk_database->open_complete
bool w_db_is_open_complete(const struct apk_database *db)
{
    return (db->open_complete != 0);
}

// wraps struct apk_database->open_complete->num_repos
unsigned int w_db_get_num_repos(const struct apk_database *db)
{
    return db->num_repos;
}

// wraps apk_database->repos[i]
struct apk_repository *w_db_get_repo(struct apk_database *db, int iRepo)
{
    return &db->repos[iRepo];
}

// wraps db->repo_update_counter
unsigned int w_db_get_repo_update_counter(const struct apk_database *db)
{
    return db->repo_update_counter;
}

// wraps db->repo_update_errors
unsigned int w_db_get_repo_update_errors(const struct apk_database *db)
{
    return db->repo_update_errors;
}

// wraps db->available.packages.num_items
int w_db_get_get_available_packages_count(const struct apk_database *db)
{
    return db->available.packages.num_items;
}

// wraps apk_db_check_world()
int w_db_check_world(struct apk_database *db)
{
    return apk_db_check_world(db, db->world);
}

bool w_db_has_installed(const struct apk_database *db)
{
    struct apk_installed_package *ipkg;
    ipkg = list_entry((&db->installed.packages)->next,
                      struct apk_installed_package,
                      installed_pkgs_list);
    if (!ipkg) {
        return false;
    }
    if (ipkg->installed_pkgs_list.next == &db->installed.packages) {
        return false;
    }
    return true;
}

void w_db_enumerate_installed(const struct apk_database *db, ENUMERATE_INSTALLED_CB cb, void *cb_param)
{
    struct apk_installed_package *ipkg;
    ipkg = list_entry((&db->installed.packages)->next,
                      struct apk_installed_package,
                      installed_pkgs_list);
    while (&ipkg->installed_pkgs_list != &db->installed.packages) {
        cb(ipkg->pkg, cb_param); // call callback
        ipkg = list_entry(ipkg->installed_pkgs_list.next,
                          typeof(*ipkg), installed_pkgs_list);
    }
}

int w_db_enumerate_available(struct apk_database *db, ENUMERATE_AVAILABLE_CB cb, void *cb_param)
{
    return apk_hash_foreach(&db->available.packages, cb, cb_param);
}

static void w_internal_repoupdate_progress_cb(void *cb_ctx, size_t p)
{
    (void)cb_ctx;
    (void)p;
    // useless, numbers are too unpredictable
    /*
     * OK: DB was opened!
     * qtapk: Updating: [ http://mirror.yandex.ru/mirrors/alpine/v3.11/main ] v3.11.3-31-g2e6d6d513d
     * fetch http://mirror.yandex.ru/mirrors/alpine/v3.11/main/x86_64/APKINDEX.tar.gz
     * qtapk: repoupdate_progress_cb:  0
     * qtapk: repoupdate_progress_cb:  0
     * qtapk: repoupdate_progress_cb:  262144
     * qtapk: repoupdate_progress_cb:  524288
     * qtapk: repoupdate_progress_cb:  719075
     * qtapk: Updating: [ http://mirror.yandex.ru/mirrors/alpine/v3.11/community ] v3.11.3-30-g2d9c1a116c
     * fetch http://mirror.yandex.ru/mirrors/alpine/v3.11/community/x86_64/APKINDEX.tar.gz
     * qtapk: repoupdate_progress_cb:  0
     * qtapk: repoupdate_progress_cb:  0
     * qtapk: repoupdate_progress_cb:  262144
     * qtapk: repoupdate_progress_cb:  524288
     * qtapk: repoupdate_progress_cb:  786432
     * qtapk: repoupdate_progress_cb:  855159
     * qtapk: Updated:  2 ; Update errors:  0
     * qtapk: 11276  distinct packages available
     * OK: DB was updated!
     * qtapk: To install: 0 ; To remove: 0 ; To adjust: 18
     * 18  packages can be updated.
     * */
}

// return 0 on success
int w_db_repository_update(struct apk_database *db, int iRepo, bool allow_untrusted)
{
    struct apk_repository *repo = &db->repos[iRepo];

    int r = 0;
    int verify_flag = allow_untrusted ? APK_SIGN_NONE : APK_SIGN_VERIFY;
    int autoupdate = 1;

    r = apk_cache_download(db, repo, NULL, verify_flag, autoupdate,
                           w_internal_repoupdate_progress_cb, (void *)NULL);
    if (r == -EALREADY) {
        return 0;
    }
    if (r != 0) {
        db->repo_update_errors++;
    } else {
        db->repo_update_counter++;
    }
    return r;
}

const char *w_db_get_repo_url(const struct apk_database *db, int iRepo)
{
    const struct apk_repository *repo = &db->repos[iRepo];
    return repo->url;
}

const char *w_db_get_repo_desc(const struct apk_database *db, int iRepo)
{
    const struct apk_repository *repo = &db->repos[iRepo];
    return apk_blob_cstr(repo->description);
}

struct apk_changeset *w_create_apk_changeset()
{
    struct apk_changeset *ret = (struct apk_changeset *)malloc(sizeof(struct apk_changeset));
    memset(ret, 0, sizeof(struct apk_changeset));
    return ret;
}

void w_delete_apk_changeset(struct apk_changeset *cs)
{
    apk_change_array_free(&cs->changes);
    free(cs);
}

// wraps changeset.num_install
int w_apk_changeset_get_num_install(const struct apk_changeset *cs)
{
    return cs->num_install;
}
// wraps changeset.num_remove
int w_apk_changeset_get_num_remove(const struct apk_changeset *cs)
{
    return cs->num_remove;
}
// wraps changeset.num_adjust
int w_apk_changeset_get_num_adjust(const struct apk_changeset *cs)
{
    return cs->num_adjust;
}
// wraps changeset.changes->num
unsigned int w_apk_changeset_get_num_changes(const struct apk_changeset *cs)
{
    return (unsigned int)cs->changes->num;
}

// wraps &changeset.changes->item[iChange];
struct apk_change *w_apk_changeset_get_change(struct apk_changeset *cs, int iChange)
{
    return &cs->changes->item[iChange];
}

// wraps achange->reinstall
bool w_apk_change_is_reinstall(const struct apk_change *c)
{
    return c->reinstall;
}
struct apk_package *w_apk_change_get_old_pkg(struct apk_change *c)
{
    return c->old_pkg;
}
struct apk_package *w_apk_change_get_new_pkg(struct apk_change *c)
{
    return c->new_pkg;
}

const char *w_apk_package_get_pkg_name(const struct apk_package *pkg)
{
    return pkg->name->name;
}
static const char s_w_emptyRet[2] = {0, 0};
const char *w_apk_package_get_version(const struct apk_package *pkg)
{
    if (pkg->version) {
        return apk_blob_cstr(*pkg->version);
    }
    return s_w_emptyRet;
}
const char *w_apk_package_get_arch(const struct apk_package *pkg)
{
    if (pkg->arch) {
        return apk_blob_cstr(*pkg->arch);
    }
    return s_w_emptyRet;
}
const char *w_apk_package_get_license(const struct apk_package *pkg)
{
    if (pkg->license) {
        return apk_blob_cstr(*pkg->license);
    }
    return s_w_emptyRet;
}
const char *w_apk_package_get_origin(const struct apk_package *pkg)
{
    if (pkg->origin) {
        return apk_blob_cstr(*pkg->origin);
    }
    return s_w_emptyRet;
}
const char *w_apk_package_get_maintainer(const struct apk_package *pkg)
{
    if (pkg->maintainer) {
        return apk_blob_cstr(*pkg->maintainer);
    }
    return s_w_emptyRet;
}
const char *w_apk_package_get_url(const struct apk_package *pkg)
{
    if (pkg->url) {
        return pkg->url;
    }
    return s_w_emptyRet;
}
const char *w_apk_package_get_description(const struct apk_package *pkg)
{
    if (pkg->description) {
        return pkg->description;
    }
    return s_w_emptyRet;
}
const char *w_apk_package_get_commit(const struct apk_package *pkg)
{
    if (pkg->commit) {
        return pkg->commit;
    }
    return s_w_emptyRet;
}
const char *w_apk_package_get_filename(const struct apk_package *pkg)
{
    if (pkg->filename) {
        return pkg->filename;
    }
    return s_w_emptyRet;
}
time_t w_apk_package_get_buildTime(const struct apk_package *pkg)
{
    return pkg->build_time;
}
size_t w_apk_package_get_size(const struct apk_package *pkg)
{
    return pkg->size;
}
size_t w_apk_package_get_installedSize(const struct apk_package *pkg)
{
    return pkg->installed_size;
}

int w_apk_solver_solve(struct apk_database *db, unsigned short solver_flags, struct apk_changeset *cs)
{
    return apk_solver_solve(db, solver_flags, db->world, cs);
}

int w_apk_solver_commit_changeset(struct apk_database *db, struct apk_changeset *cs)
{
    return apk_solver_commit_changeset(db, cs, db->world);
}

static bool w_internal_non_repository_check(struct apk_database *db)
{
    // copied from libapk's add.c
    if (apk_force & APK_FORCE_NON_REPOSITORY)
        return true;
    if (apk_db_cache_active(db))
        return true;
    if (apk_db_permanent(db))
        return true;

    return false;
}

static bool w_internal_package_name_to_apk_dependency(
    struct apk_database *db,
    const char *pkgname,
    struct apk_dependency *pdep)
{
    // copied from libapk's add.c, but virtual packages support was removed
    int r;
    if (strstr(pkgname, ".apk") != NULL) {
        struct apk_package *pkg = NULL;
        struct apk_sign_ctx sctx;

        if (!w_internal_non_repository_check(db)) {
            return false;
        }

        apk_sign_ctx_init(&sctx, APK_SIGN_VERIFY_AND_GENERATE,
                          NULL, db->keys_fd);
        r = apk_pkg_read(db, pkgname, &sctx, &pkg);
        apk_sign_ctx_free(&sctx);
        if (r != 0) {
            return false;
        }
        apk_dep_from_pkg(pdep, db, pkg);
    } else {
        apk_blob_t b = APK_BLOB_STR(pkgname);

        apk_blob_pull_dep(&b, db, pdep);
        if (APK_BLOB_IS_NULL(b) || b.len > 0) {
            return false;
        }
    }
    return true;
}

void w_resolved_dep_free_strings(struct w_resolved_apk_dependency *dep)
{
    free(dep->name);
    free(dep->version);
}

// returns 0 on success
int w_apk_add(struct apk_database *db,
              const char *pkgNameSpec,
              unsigned short solver_flags,
              struct w_resolved_apk_dependency *resolved_dep)
{
    struct apk_dependency dep;
    if (!w_internal_package_name_to_apk_dependency(db, pkgNameSpec, &dep)) {
        return 1;
    }

    if (resolved_dep) {
        resolved_dep->name = strdup(dep.name->name);
        resolved_dep->version = strdup(apk_blob_cstr(*dep.version));
    }

    int r;
    struct apk_dependency_array *world_copy = NULL;
    apk_dependency_array_copy(&world_copy, db->world);
    apk_deps_add(&world_copy, &dep);
    apk_solver_set_name_flags(dep.name, solver_flags, solver_flags);
    r = apk_solver_commit(db, 0, world_copy);
    // ^^ this may be split into:
    //    - apk_solver_solve
    //    - apk_solver_commit_changeset
    // if more detailed information is required about
    //    what is happening behind the scenes
    apk_dependency_array_free(&world_copy);
    return r;
}

/**
 * Internal struct passed as void* context in delete
 * package callbacks
 **/
struct w_internal_del_ctx {
    int recursive_delete;   //! delete rdepends?
    struct apk_dependency_array *world; //! world copy
    int errors;             //! errors counter
};

/**
 * @brief w_internal_cb_delete_pkg
 * Used as callback and can call itself recursively.
 * Performs an actual deletion.
 * @param pkg0 - actual package to delete
 * @param dep0 - unused
 * @param pkg - unused
 * @param ctx - deletion context, stores information
 *              about current deletion procedure. Points
 *              to @see struct w_internal_del_ctx.
 */
static void w_internal_cb_delete_pkg(struct apk_package *pkg0,
                                     struct apk_dependency *dep0,
                                     struct apk_package *pkg,
                                     void *ctx)
{
    (void)dep0;
    (void)pkg;
    struct w_internal_del_ctx *pctx = (struct w_internal_del_ctx *)ctx;
    apk_deps_del(&pctx->world, pkg0->name);
    if (pctx->recursive_delete) {
        // call self for each reverse dep
        apk_pkg_foreach_reverse_dependency(
            pkg0, APK_FOREACH_INSTALLED | APK_DEP_SATISFIES,
            w_internal_cb_delete_pkg, ctx);
    }
}

// returns 0 on success
int w_apk_del(struct apk_database *db,
              const char *pkgname,
              bool recursive_delete)
{
    const apk_blob_t pkgname_blob = APK_BLOB_STR(pkgname);
    struct apk_dependency_array *world_copy = NULL;
    struct apk_changeset changeset = {};
    struct apk_name *name = NULL;
    struct apk_package *pkg = NULL;

    apk_dependency_array_copy(&world_copy, db->world);

    // fill in deletion context
    struct w_internal_del_ctx dctx = {
        .recursive_delete = recursive_delete ? 1 : 0,
        .world = world_copy,
        .errors = 0
    };

    // find package name as apk_name in installed packages
    name = (struct apk_name *)apk_hash_get(&db->available.names, pkgname_blob);

    if (!name) {
        dctx.errors++;
        return 1;
    }

    // This time find package (not only name!)
    // This function finds first provider of the name
    //      that is actually installed
    pkg = apk_pkg_get_installed(name);
    if (pkg != NULL) {
        // cb_delete_pkg() can call itself recursively to
        //  delete reverse depends, if requested
        w_internal_cb_delete_pkg(pkg, NULL, NULL, &dctx);
    } else {
        apk_deps_del(&dctx.world, name);
    }

    // solve world
    int r = apk_solver_solve(db, 0, world_copy, &changeset);
    if (r == 0) {
        r = apk_solver_commit_changeset(db, &changeset, world_copy);
    }

    // cleanup
    apk_change_array_free(&changeset.changes);
    apk_dependency_array_free(&world_copy);
    return r;
}

