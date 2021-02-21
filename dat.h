typedef struct Resource Resource;
typedef struct Good Good;
typedef struct Building Building;
typedef struct Terrain Terrain;
typedef struct Tile Tile;

enum{
	Gfish,
	Gcattle,
	Gwheat,
	Gwood,
	Glumber,
	Gstone,
	Giron,
	Gtools,
	Ggold,
	Gjewelry,
	Gtot,
};
struct Good{
	char *name;
	int terrain;
	int resource;
};
extern Good goods[Gtot];
extern int initialstock[Gtot];

enum{
	Rfood,
	Rwood,
	Rlumber,
	Rstone,
	Riron,
	Rtools,
	Rgold,
	Rjewelry,
	Rtot,
};
struct Resource{
	char *name;
	int goods[Gtot];
};
extern Resource resources[Rtot];

enum{
	Btownhall,
	Bmarket,
	Bfishyard,
	Bcarpentry,
	Bsawmill,
	Bmill,
	Bfarm,
	Bquarry,
	Bsmeltery,
	Bforge,
	Blapidary,
	Btot,
};
struct Building{
	char *name;
	int buildtime;
	int buildcost[Gtot];
	int prodtime;
	int product[Gtot];
	int prodcost[Gtot];
	int terrain;
	int upkeep[Rtot];
};
extern Building buildings[Btot];

enum{
	Tplain,
	Twoods,
	Tpond,
	Tcrop,
	Tmountain,
	Tironvein,
	Tgoldvein,
	Ttot,
};
struct Terrain{
	char *name;
	int good;
	int initialstock;
	double yield;
};
extern Terrain terrains[Ttot];

enum{
	Snull,
	Svoid,
	Swaitbuild,
	Sbuild,
	Sstarved,
	Swaitsupply,
	Sproduce,
};
struct Tile{
	Terrain *t;
	int distance;
	int stock;
	Building *b;
	int state;
	int gotsupply;
	int prodstock[Gtot];
	vlong clock;
	vlong prodΔt;
	vlong supplyΔt;
	vlong pickupΔt;
	int stale;
};
extern Tile *map;
extern int mapwidth, mapheight;

enum{
	Te9 = 1000000000,
	Te6 = 1000000,
	SimHz = 5,
	AnimHz = 1,
	Travelticks = 10,
};
extern vlong clock;
extern int paused;
