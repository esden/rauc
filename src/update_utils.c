#include <errno.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "update_utils.h"
#include "context.h"

gboolean r_copy_stream_with_progress(GInputStream *in_stream, GOutputStream *out_stream,
		goffset size, GError **error)
{
	GError *ierror = NULL;
	gsize out_size = 0;
	goffset sum_size = 0;
	gint last_percent = -1, percent;
	gchar buffer[8192];
	gssize in_size;

	g_return_val_if_fail(in_stream, FALSE);
	g_return_val_if_fail(out_stream, FALSE);
	g_return_val_if_fail(size >= 0, FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	/* no-op for zero-sized images */
	if (size == 0)
		return TRUE;

	do {
		gboolean ret;

		in_size = g_input_stream_read(in_stream,
				buffer, 8192, NULL, &ierror);
		if (in_size == -1) {
			g_propagate_error(error, ierror);
			return FALSE;
		}
		ret = g_output_stream_write_all(out_stream, buffer,
				in_size, &out_size, NULL, &ierror);
		if (!ret) {
			g_propagate_error(error, ierror);
			return FALSE;
		}

		sum_size += out_size;

		percent = sum_size * 100 / size;
		/* emit progress info (but only when in progress context) */
		if (r_context()->progress && percent != last_percent) {
			last_percent = percent;
			r_context_set_step_percentage("copy_image", percent);
		}
	} while (out_size);

	return TRUE;
}
