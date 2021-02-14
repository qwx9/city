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
	int time;
	int buildtime;
	int costs[Gtot];
	int terrain;
	int goods[Gtot];
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
	Swait,
	Sbuild,
	Sproduce,
};
struct Tile{
	Terrain *t;
	Building *b;
	int stock;
	int state;
	int clock;
};
extern Tile *map;
extern int mapwidth, mapheight;

enum{
	Te9 = 1000000000,
	Te6 = 1000000,
	THz = 5,
	Travelticks = 10,
};
extern int tdiv;
extern vlong clock;
