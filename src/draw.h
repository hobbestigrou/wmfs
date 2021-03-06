/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */


#ifndef DRAW_H
#define DRAW_H

#include <string.h>
#include <X11/Xlib.h>

#ifdef HAVE_IMLIB2
#include <Imlib2.h>
#endif /* HAVE_IMLIB2 */

#include "wmfs.h"
#include "config.h"
#include "screen.h"

#define TEXTY(t, w) ((t->font.height - t->font.de) + ((w - t->font.height) >> 1))
#define PAD (8)

static inline void
draw_text(Drawable d, struct theme *t, int x, int y, Color fg, const char *str)
{
     XSetForeground(W->dpy, W->gc, fg);
     XmbDrawString(W->dpy, d, t->font.fontset, W->gc, x, y, str, strlen(str));
}

static inline void
draw_rect(Drawable d, struct geo *g, Color bg)
{
     XSetForeground(W->dpy, W->gc, bg);
     XFillRectangle(W->dpy, d, W->gc, g->x, g->y, g->w, g->h);
}

#ifdef HAVE_IMLIB2

/*
 * Draw image on drawable with g geo
 * Require that the image was loaded with draw_image_load()
 */
static inline void
draw_image(Drawable d, struct geo *g)
{
     imlib_context_set_drawable(d);
     imlib_render_image_on_drawable_at_size(g->x, g->y, g->w, g->h);
     imlib_free_image();
}

/*
 * Load image, set it in imlib context, and return
 * width & height as argument 2 & 3
 */
static inline void
draw_image_load(char *path, int *w, int *h)
{
     Imlib_Image image = imlib_load_image(path);

     imlib_context_set_image(image);

     *w = imlib_image_get_width();
     *h = imlib_image_get_height();
}

#endif /* HAVE_IMLIB2 */

/*
 * For client use
 */
static inline void
draw_reversed_rect(Drawable dr, struct client *c, bool t)
{
     struct geo *g = (t ? &c->tgeo : &c->geo);
     struct geo *ug = &c->screen->ugeo;
     int i = c->theme->client_border_width;

     XDrawRectangle(W->dpy, dr, W->rgc,
                    ug->x + g->x + i,
                    ug->y + g->y + i,
                    g->w - (i << 1),
                    g->h - (i << 1));
}

static inline void
draw_line(Drawable d, int x1, int y1, int x2, int y2)
{
     XDrawLine(W->dpy, d, W->gc, x1, y1, x2, y2);
}

static inline unsigned short
draw_textw(struct theme *t, const char *str)
{
     XRectangle r;

     XmbTextExtents(t->font.fontset, str, strlen(str), NULL, &r);

     return r.width;
}

#endif /* DRAW_H */
