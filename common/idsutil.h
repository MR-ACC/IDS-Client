#ifndef IDSUTIL_H
#define IDSUTIL_H
#include <glib.h>
G_BEGIN_DECLS

gint base64_encode(guchar *data, guint data_len,
        guchar *encData, guint encdata_len);
gint base64_decode(guchar *data, guint data_len,
        guchar *dec_data, guint dec_data_len);
G_END_DECLS
#endif // IDSUTIL_H

