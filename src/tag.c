/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#include <X11/Xutil.h> /* IconicState / NormalState */

#include "tag.h"
#include "util.h"
#include "infobar.h"
#include "client.h"
#include "config.h"
#include "barwin.h"
#include "ewmh.h"
#include "layout.h"

struct tag*
tag_new(struct screen *s, char *name)
{
     struct tag *t, *l;

     t = xcalloc(1, sizeof(struct tag));

     t->screen = s;
     t->name   = xstrdup(name);
     t->flags  = 0;
     t->id     = 0;
     t->sel    = NULL;

     if((l = TAILQ_LAST(&s->tags, tsub)))
          t->id = l->id + 1;

     SLIST_INIT(&t->clients);
     TAILQ_INIT(&t->sets);

     TAILQ_INSERT_TAIL(&s->tags, t, next);

     return t;
}

void
tag_screen(struct screen *s, struct tag *t)
{
     struct client *c;

     /* Unmap previous tag's frame */
     SLIST_FOREACH(c, &s->seltag->clients, tnext)
          client_unmap(c);

     /*
      * Map selected tag's frame, only if there is
      * clients in t
      */
     if(!SLIST_EMPTY(&t->clients))
     {
          SLIST_FOREACH(c, &t->clients, tnext)
               if(!(c->flags & CLIENT_TABBED))
                    client_map(c);

          client_focus( client_tab_next(t->sel));
     }

     s->seltag = t;

     infobar_elem_screen_update(s, ElemTag);

     ewmh_update_wmfs_props();
}

/* Set t to NULL to untag c from c->tag */
void
tag_client(struct tag *t, struct client *c)
{
     struct tag *ot = c->tag;

     /* Remove client from its previous tag */
     if(c->tag && !(c->flags & CLIENT_RULED))
     {
          if(c->tag == t || c->flags & CLIENT_TABBED)
               return;

          if(!(c->flags & CLIENT_IGNORE_LAYOUT))
               layout_split_arrange_closed(c);


          SLIST_REMOVE(&c->tag->clients, c, client, tnext);

          if(c->tag->sel == c || W->client == c)
               client_focus( client_tab_next( client_next(c)));
     }

     c->flags &= ~CLIENT_RULED;

     /* Client remove */
     if(!t)
          return;

     c->tag = t;

     client_update_props(c, CPROP_LOC);

     /*
      * Insert in new tag list before
      * layout_split_integrate, because of set historic.
      */
     SLIST_INSERT_HEAD(&t->clients, c, tnext);

     if(c->flags & CLIENT_IGNORE_LAYOUT)
          c->flags ^= CLIENT_IGNORE_LAYOUT;
     else
          layout_split_integrate(c, t->sel);

     if(c->flags & CLIENT_TABMASTER)
     {
          struct client *cc;

          SLIST_FOREACH(cc, &ot->clients, tnext)
               if(cc->tabmaster == c)
                    cc->tag = t;
     }
}

void
uicb_tag_set(Uicb cmd)
{
     int i = 0, n = ATOI(cmd);
     struct tag *t;

     TAILQ_FOREACH(t, &W->screen->tags, next)
          if(++i == n)
          {
               tag_screen(W->screen, t);
               return;
          }
}

void
uicb_tag_set_with_name(Uicb cmd)
{
     struct tag *t;

     TAILQ_FOREACH(t, &W->screen->tags, next)
          if(!strcmp(cmd, t->name))
          {
               tag_screen(W->screen, t);
               return;
          }
}

void
uicb_tag_next(Uicb cmd)
{
     (void)cmd;
     struct tag *t;

     if((t = TAILQ_NEXT(W->screen->seltag, next)))
          tag_screen(W->screen, t);
     else if( /* CIRCULAR OPTION */ 1)
          tag_screen(W->screen, TAILQ_FIRST(&W->screen->tags));
}

void
uicb_tag_prev(Uicb cmd)
{
     (void)cmd;
     struct tag *t;

     if((t = TAILQ_PREV(W->screen->seltag, tsub, next)))
          tag_screen(W->screen, t);
     else if( /* CIRCULAR OPTION */ 1)
          tag_screen(W->screen, TAILQ_LAST(&W->screen->tags, tsub));
}

static void
tag_remove(struct tag *t)
{
     free(t->name);

     layout_free_set(t);

     free(t);
}

void
tag_free(struct screen *s)
{
     struct tag *t;

     TAILQ_FOREACH(t, &s->tags, next)
     {
          TAILQ_REMOVE(&s->tags, t, next);
          tag_remove(t);
     }
}
