#include <glib.h>
#include <libssh/libssh.h>
#include <stdlib.h>

#include "ssh.h"

extern GQuark WSH_SSH_ERROR;

static const gchar* username = "worr";
static const gchar* password = "pass";
//static const gchar* remote = "127.0.0.1";
static const gint 	port = 22;
static const gchar* unreachable_remote = "xxx.yyy.zzz";

static void host_not_reachable(void) {
	set_ssh_connect_res(SSH_ERROR);

	wsh_ssh_session_t* session = g_slice_new0(wsh_ssh_session_t);
	session->hostname = unreachable_remote;
	session->username = username;
	session->password = password;
	session->port = port;
	GError* err;
	gint ret = wsh_ssh_host(session, &err);

	g_assert(ret == -1);
	g_assert(session->session == NULL);
	g_assert_error(err, WSH_SSH_ERROR, 4);

	g_slice_free(wsh_ssh_session_t, session);
}

static void change_host_key(void) {
	set_ssh_connect_res(SSH_OK);
	set_ssh_is_server_known_res(SSH_SERVER_KNOWN_CHANGED);

	wsh_ssh_session_t* session = g_slice_new0(wsh_ssh_session_t);
	session->hostname = unreachable_remote;
	session->username = username;
	session->password = password;
	session->port = port;
	GError* err;
	gint ret = wsh_verify_host_key(session, FALSE, FALSE, &err);

	g_assert(ret == WSH_SSH_NEED_ADD_HOST_KEY);
	g_assert(session->session == NULL);
	g_assert_error(err, WSH_SSH_ERROR, 2);

	g_slice_free(wsh_ssh_session_t, session);
}

static void fail_add_host_key(void) {
	set_ssh_is_server_known_res(SSH_SERVER_NOT_KNOWN);
	set_ssh_write_knownhost_res(SSH_ERROR);

	wsh_ssh_session_t* session = g_slice_new0(wsh_ssh_session_t);
	session->hostname = unreachable_remote;
	session->username = username;
	session->password = password;
	session->port = port;
	GError *err;
	gint ret = wsh_verify_host_key(session, TRUE, FALSE, &err);

	g_assert(ret == WSH_SSH_HOST_KEY_ERROR);
	g_assert(session->session == NULL);
	g_assert_error(err, WSH_SSH_ERROR, 1);

	g_slice_free(wsh_ssh_session_t, session);
}

static void add_host_key(void) {
	set_ssh_connect_res(SSH_OK);
	set_ssh_is_server_known_res(SSH_SERVER_KNOWN_OK);
	set_ssh_write_knownhost_res(SSH_OK);

	wsh_ssh_session_t* session = g_slice_new0(wsh_ssh_session_t);
	session->hostname = unreachable_remote;
	session->username = username;
	session->password = password;
	session->port = port;
	GError *err;
	gint ret = wsh_verify_host_key(session, TRUE, FALSE, &err);

	g_assert(ret == 0);
	g_assert_no_error(err);

	g_slice_free(wsh_ssh_session_t, session);
}

int main(int argc, char** argv) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/Library/SSH/UnreachableHost", host_not_reachable);
	g_test_add_func("/Library/SSH/ChangedHostKey", change_host_key);
	g_test_add_func("/Library/SSH/FailToAddHostKey", fail_add_host_key);
	g_test_add_func("/Library/SSH/AddHostKey", add_host_key);

	return g_test_run();
}
