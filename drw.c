#include <u.h>
#include <libc.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"

Point EP = {-1,-1};

int scale = 1;
Point pan;
void (*mapdrawfn)(void);
QLock drwlock;
int repaint;

enum{
	Cbg,
	Ctext,
	Cend,
};
static Image *cols[Cend];

Rectangle scrwin;
static Rectangle fbr, hudr;
static Point scrofs, tlwin, fbbufr;
static Image *fb;
static u32int *fbbuf;

static Image *
eallocimage(Rectangle r, ulong chan, int repl, ulong col)
{
	Image *i;

	if((i = allocimage(display, r, chan, repl, col)) == nil)
		sysfatal("allocimage: %r");
	return i;
}

Point
s2p(Point p)
{
	if(!ptinrect(p, scrwin))
		return EP;
	return divpt(subpt(p, scrwin.min), scale);
}

Tile *
p2t(Point p)
{
	p = divpt(p, Tilesz);
	assert(p.x >= 0 && p.x < mapwidth && p.y >= 0 && p.y < mapheight);
	return map + p.y * mapwidth + p.x;
}

Tile *
s2t(Point p)
{
	if(eqpt(p, EP))
		return nil;
	return p2t(p);
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

void
composeat(Tile *m, u32int c)
{
	int k, n, x, w;
	u32int v, *pp;

	w = fbr.max.x * fbr.max.y / scale;
	pp = fbbuf + (m-map) / mapwidth * w
		+ (m-map) % mapwidth * Tilesz * scale;
	n = Tilesz;
	w = (fbr.max.x / scale - Tilesz) * scale;
	while(n-- > 0){
		x = Tilesz;
		while(x-- > 0){
			v = *pp;
			v = (v & 0xff0000) + (c & 0xff0000) >> 1 & 0xff0000
				| (v & 0xff00) + (c & 0xff00) >> 1 & 0xff00
				| (v & 0xff) + (c & 0xff) >> 1 & 0xff;
			k = scale;
			while(k-- > 0)
				*pp++ = v;
		}
		pp += w;
	}
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
drawmenuselected(void)
{
	int i, n;
	char s[256], *sp;
	Point p;
	Building *b;

	if(selected == nil)
		return;
	assert(selected >= map && selected < map + mapwidth * mapheight);
	b = buildings + (selected - map);
	if(b >= buildings + nelem(buildings)){
		fprint(2, "nope\n");
		return;
	}
	p = addpt(hudr.min, Pt(0, font->height));
	sp = s;
	sp = seprint(sp, s+sizeof s, "%s time %d cost ",
		b->name, b->buildtime);
	for(i=0, n=0; i<nelem(b->product); i++)
		if(b->buildcost[i] > 0){
			sp = seprint(sp, s+sizeof s, "%s%d %s",
				n > 0 ? "," : "", b->buildcost[i], goods[i].name);
			n++;
		}
	sp = seprint(sp, s+sizeof s, " product ");
	for(i=0, n=0; i<nelem(b->product); i++)
		if(b->product[i] > 0){
			sp = seprint(sp, s+sizeof s, "%s%d %s",
				n > 0 ? "," : "", b->product[i], goods[i].name);
			n++;
		}
	string(screen, p, cols[Ctext], ZP, font, s);
}

static void
drawtile(Tile *m)
{
	Pic *pic;

	pic = m->b != nil ? &m->b->Pic : &m->t->Pic;
	drawpic(tile2xy(m), pic);
}

void
drawbuildings(void)
{
	int x, y;
	Tile *m;
	Building *b;

	for(y=0, m=map, b=buildings; y<tlwin.y; y++){
		for(x=0; x<tlwin.x; x++, m++, b++){
			if(b >= buildings + nelem(buildings))
				return;
			drawpic(tile2xy(m), &b->Pic);
			if(!couldbuild(b))
				composeat(m, 0x555555);
			if(m->stale){
				m->stale = 0;
				repaint = 1;
			}
		}
		m += mapwidth - tlwin.x;
	}
}

static void
drawmap(void)
{
	int x, y;
	Tile *m;

	for(y=0, m=map; y<tlwin.y; y++){
		for(x=0; x<tlwin.x; x++, m++)
			if(m->stale){
				drawtile(m);
				m->stale = 0;
				repaint = 1;
			}
		m += mapwidth - tlwin.x;
	}
}

void
drawbuildmenu(void)
{
	drawbuildings();
	drawmenuselected();
}

static void
redrawcanvas(void)
{
	int x, y;
	Tile *m;

	memset(fbbuf, 0, fbbufr.x * fbbufr.y * sizeof *fbbuf);
	for(y=0, m=map; y<tlwin.y; y++){
		for(x=0; x<tlwin.x; x++, m++)
			m->stale = 1;
		m += mapwidth - tlwin.x;
	}
}

void
updatedraw(int all)
{
	qlock(&drwlock);
	if(all || repaint)
		redrawcanvas();
	(mapdrawfn != nil ? mapdrawfn : drawmap)();
	qunlock(&drwlock);
	flushfb();
	repaint = 0;
}

void
flushfb(void)
{
	uchar *p;
	Rectangle r, r2;

	lockdisplay(display);
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
	unlockdisplay(display);
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
	updatedraw(1);
}

void
initdrw(void)
{
	if(initdraw(nil, nil, "city") < 0)
		sysfatal("initdraw: %r");
	display->locking = 1;
	unlockdisplay(display);
	cols[Cbg] = eallocimage(Rect(0,0,1,1), screen->chan, 1, 0x777777FF);
	cols[Ctext] = display->black;
	fmtinstall('P', Pfmt);
	fmtinstall('R', Rfmt);
}
