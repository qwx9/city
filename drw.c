#include <u.h>
#include <libc.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"

int scale = 1;
Point pan;
QLock drwlock;

static Tile *selected;

enum{
	Cbg,
	Ctext,
	Cend,
};
static Image *cols[Cend];

static Rectangle fbr, scrwin, hudr;
static Point scrofs, tlwin, fbbufr;
static Image *fb;
static u32int *fbbuf;
static int doupdate;

static Image *
eallocimage(Rectangle r, ulong chan, int repl, ulong col)
{
	Image *i;

	if((i = allocimage(display, r, chan, repl, col)) == nil)
		sysfatal("allocimage: %r");
	return i;
}

static int
clippic(Point pp, Pic *pic, Point *scpp, Point *ofs, Point *sz)
{
	int fbbufunsx;
	Point minp, maxp;

	fbbufunsx = fbbufr.x / scale;
	minp = subpt(pp, pan);
	maxp = addpt(minp, pic->picsz);
	if(maxp.x < 0 || maxp.y < 0 || minp.x > fbbufunsx || minp.y > fbbufr.y)
		return -1;
	ofs->x = minp.x < 0 ? -minp.x : 0;
	ofs->y = minp.y < 0 ? -minp.y : 0;
	*sz = subpt(pic->picsz, *ofs);
	if(maxp.x > fbbufunsx)
		sz->x -= maxp.x - fbbufunsx;
	if(maxp.y > fbbufr.y)
		sz->y -= maxp.y - fbbufr.y;
	if(sz->x <= 0 || sz->y <= 0)
		return -1;
	scpp->x = minp.x + ofs->x;
	scpp->y = minp.y + ofs->y;
	return 0;
}

static void
drawpic(Point pp, Pic *pic)
{
	int Δpx, Δx;
	Point scpp, ofs, sz;
	u32int v, *p, *fbp, *fbe;

	if(pic->pic == nil)
		sysfatal("drawpic: empty pic");
	if(clippic(pp, pic, &scpp, &ofs, &sz) < 0)
		return;
	p = pic->pic + ofs.y * pic->picsz.x + ofs.x;
	assert(p < pic->pic + pic->picsz.y * pic->picsz.x);
	Δpx = pic->picsz.x - sz.x;
	sz.x *= scale;
	fbp = fbbuf + scpp.y * fbbufr.x + scpp.x * scale;
	assert(fbp < fbbuf + fbbufr.x * fbbufr.y);
	Δx = fbbufr.x - sz.x;
	while(sz.y-- > 0){
		fbe = fbp + sz.x;
		while(fbp < fbe){
			v = *p++;
			switch(scale){
			case 16: fbp[15] = v;
			case 15: fbp[14] = v;
			case 14: fbp[13] = v;
			case 13: fbp[12] = v;
			case 12: fbp[11] = v;
			case 11: fbp[10] = v;
			case 10: fbp[9] = v;
			case 9: fbp[8] = v;
			case 8: fbp[7] = v;
			case 7: fbp[6] = v;
			case 6: fbp[5] = v;
			case 5: fbp[4] = v;
			case 4: fbp[3] = v;
			case 3: fbp[2] = v;
			case 2: fbp[1] = v;
			default: fbp[0] = v;
			}
			fbp += scale;
		}
		p += Δpx;
		fbp += Δx;
	}
}

static Point
tile2xy(Tile *m)
{
	Point p;

	p.x = ((m - map) % mapwidth) * Tilesz;
	p.y = ((m - map) / mapwidth) * Tilesz;
	return p;
}

static Point
scr2tilexy(Point p)
{
	p = divpt(subpt(p, scrwin.min), scale * Tilesz);
	assert(p.x < tlwin.x && p.x >= 0 && p.y < tlwin.y && p.y >= 0);
	return p;
}

static Tile *
scr2tile(Point p)
{
	assert(ptinrect(p, scrwin));
	p = scr2tilexy(p);
	return map + p.y * mapwidth + p.x;
}

static void
drawhud(void)
{
	char *name;

	draw(screen, hudr, cols[Cbg], nil, ZP);
	if(selected == nil)
		return;
	name = selected->b != nil ? selected->b->name : selected->t->name;
	string(screen, hudr.min, cols[Ctext], ZP, font, name);
}

static void
drawtile(Tile *m)
{
	Pic *pic;

	pic = m->b != nil ? &m->b->Pic : &m->t->Pic;
	drawpic(tile2xy(m), pic);
}

void
updatedraw(void)
{
	int x, y;
	Tile *m;

	qlock(&drwlock);
	for(y=0, m=map; y<tlwin.y; y++){
		for(x=0; x<tlwin.x; x++, m++)
			if(m->stale){
				drawtile(m);
				m->stale = 0;
				doupdate = 1;
			}
		m += mapwidth - tlwin.x;
	}
	qunlock(&drwlock);
	if(!doupdate)
		return;
	flushfb();
	doupdate = 0;
}

void
mouseselect(Point p)
{
	doupdate = 1;
	if(!ptinrect(p, scrwin)){
		selected = nil;
		return;
	}
	selected = scr2tile(p);
	doupdate = 1;
	updatedraw();
}

static void
redraw(void)
{
	int x, y;
	Tile *m;

	draw(fb, fb->r, display->black, nil, ZP);
	for(y=0, m=map; y<tlwin.y; y++){
		for(x=0; x<tlwin.x; x++, m++)
			m->stale = 1;
		m += mapwidth - tlwin.x;
	}
	updatedraw();
}

void
flushfb(void)
{
	uchar *p;
	Rectangle r, r2;

	p = (uchar *)fbbuf;
	if(scale == 1){
		loadimage(fb, fb->r, p, fbbufr.x * fbbufr.y * sizeof *fbbuf);
		draw(screen, screen->r, fb, nil, scrofs);
	}else{
		r = rectaddpt(fbr, subpt(screen->r.min, scrofs));
		r2 = r;
		while(r.min.y < r2.max.y){
			r.max.y = r.min.y + scale;
			p += loadimage(fb, fb->r, p, fbbufr.x * sizeof *fbbuf);
			draw(screen, r, fb, nil, ZP);
			r.min.y = r.max.y;
		}
	}
	drawhud();
	flushimage(display, 1);
}

void
resetdraw(void)
{
	int sw, sh;

	sw = Dx(screen->r);
	sh = Dy(screen->r);
	/* fb rect in pixels cropped to fit window */
	fbr.min = ZP;
	fbr.max.x = min(sw, mapwidth * Tilesz * scale);
	fbr.max.y = min(sh, mapheight * Tilesz * scale);
	/* actual drawing fb size, scaled horizontally only */
	fbbufr.x = fbr.max.x;
	fbbufr.y = fbr.max.y / scale;
	/* negative offset to translate the framebuffer
	 * towards the lower right corner (see draw(2)) */
	scrofs.x = -(sw > fbr.max.x ? (sw - fbr.max.x) / 2 : 0);
	scrofs.y = -(sh > fbr.max.y ? (sh - fbr.max.y) / 2 : 0);
	/* tile window in tiles cropped to fit window */
	tlwin.x = min(mapwidth, fbr.max.x / scale / Tilesz + 1);
	tlwin.y = min(mapheight, fbr.max.y / scale / Tilesz + 1);
	/* absolute tile window rect in pixels */
	scrwin.min = subpt(screen->r.min, scrofs);
	scrwin.max = addpt(scrwin.min, fbr.max);
	/* absolute hud rect in pixels */
	hudr.min = addpt(scrwin.max, Pt(-fbr.max.x, font->height));
	hudr.max = addpt(hudr.min, Pt(fbr.max.x, screen->r.max.y - hudr.min.y));
	freeimage(fb);
	fb = eallocimage(Rect(0,0,fbr.max.x,scale==1 ? fbr.max.y : 1),
		XRGB32, scale>1, DNofill);
	free(fbbuf);
	fbbuf = emalloc(fbbufr.x * fbbufr.y * sizeof *fbbuf);
	if(!eqpt(scrofs, ZP))
		draw(screen, screen->r, cols[Cbg], nil, ZP);
	doupdate = 1;
	redraw();
}

void
initdrw(void)
{
	if(initdraw(nil, nil, "city") < 0)
		sysfatal("initdraw: %r");
	cols[Cbg] = eallocimage(Rect(0,0,1,1), screen->chan, 1, 0x777777FF);
	cols[Ctext] = display->black;
	fmtinstall('P', Pfmt);
	fmtinstall('R', Rfmt);
}
