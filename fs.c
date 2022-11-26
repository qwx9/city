#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"

char *prefix = "/sys/games/lib/city";

static void
loadpic(char *name, int dim, Pic *pic)
{
	int fd, n, w;
	char *f;
	uchar *buf, *s;
	u32int v, *p;
	Image *i;

	if((f = smprint("%s.bit", name)) == nil)
		sysfatal("loadpic:smprint: %r");
	if((fd = open(f, OREAD)) < 0)
		sysfatal("loadpic:open: %r");
	if((i = readimage(display, fd, 0)) == nil)
		sysfatal("loadpic:readimage: %r");
	close(fd);
	if(i->chan != RGB24 || Dx(i->r) != dim || Dy(i->r) != dim)
		sysfatal("loadpic %s: inappropriate image format", f);
	free(f);
	n = dim * dim;
	pic->picsz = Pt(dim, dim);
	pic->pic = emalloc(n * sizeof *pic->pic);
	n *= i->depth / 8;
	buf = emalloc(n);
	unloadimage(i, i->r, buf, n);
	w = i->depth / 8;
	n = dim * dim;
	s = buf;
	p = pic->pic;
	while(n-- > 0){
		v = 0xff << 24 | s[2] << 16 | s[1] << 8 | s[0];
		*p++ = v;
		s += w;
	}
	freeimage(i);
	free(buf);
}

static void
readimg(void)
{
	Terrain *t;
	Building *b;

	for(t=terrains; t<terrains+nelem(terrains); t++)
		loadpic(t->name, Tilesz, &t->Pic);
	for(b=buildings; b<buildings+nelem(buildings); b++)
		loadpic(b->name, Tilesz, &b->Pic);
}

void
readfs(void)
{
	rfork(RFNAMEG);
	if(bind(".", prefix, MBEFORE|MCREATE) == -1 || chdir(prefix) < 0)
		fprint(2, "initfs: %r\n");
	readimg();
}
