#include <u.h>
#include <libc.h>
#include "dat.h"

Good goods[] = {
	[Gfish]{
		.name "fish",
		.terrain Tpond,
		.resource Rfood,
	},[Gcattle]{
		.name "cattle",
		.terrain -1,
		.resource Rfood,
	},[Gwheat]{
		.name "wheat",
		.terrain -1,
		.resource Rfood,
	},[Gwood]{
		.name "wood",
		.terrain Twoods,
		.resource Rwood,
	},[Glumber]{
		.name "lumber",
		.terrain -1,
		.resource Rlumber,
	},[Gstone]{
		.name "stone",
		.terrain Tmountain,
		.resource Rstone,
	},[Giron]{
		.name "iron",
		.terrain -1,
		.resource Riron,
	},[Gtools]{
		.name "tools",
		.terrain -1,
		.resource Rtools,
	},[Ggold]{
		.name "gold",
		.terrain Tgoldvein,
		.resource Rgold,
	},[Gjewelry]{
		.name "jewelry",
		.terrain -1,
		.resource Rjewelry,
	}
};
int initialstock[Gtot] = {
	[Gwood] 10,
	[Gwheat] 5,
	[Gfish] 5,
	[Gcattle] 5,
	[Ggold] 100,
};
Resource resources[] = {
	[Rfood]{
		.name "food",
		.goods {
			[Gfish] 1,
			[Gcattle] 1,
			[Gwheat] 1,
		},
	},[Rwood]{
		.name "wood",
		.goods {
			[Gwood] 1,
		},
	},[Rlumber]{
		.name "lumber",
		.goods {
			[Glumber] 1,
		},
	},[Rstone]{
		.name "stone",
		.goods {
			[Gstone] 1,
		},
	},[Riron]{
		.name "iron",
		.goods {
			[Giron] 1,
		},
	},[Rtools]{
		.name "tools",
		.goods {
			[Gtools] 1,
		},
	},[Rgold]{
		.name "gold",
		.goods {
			[Ggold] 1,
		},
	},[Rjewelry]{
		.name "jewelry",
		.goods {
			[Gjewelry] 1,
		},
	}
};
Building buildings[] = {
	[Btownhall]{
		.name "townhall",
		.time 0,
		.buildtime 1000,
		.costs {0},
		.terrain Tplain,
		.goods {0},
		.upkeep {
			[Rfood] 1,
		},
	},[Bcarpentry]{
		.name "carpentry",
		.time 25,
		.buildtime 50,
		.costs {
			[Ggold] 3,
			[Gwood] 3,
		},
		.terrain Tplain,
		.goods {
			[Gwood] 1,
		},
		.upkeep {
			[Rfood] 1,
		},
	},[Bsawmill]{
		.name "sawmill",
		.time 25,
		.buildtime 50,
		.costs {
			[Ggold] 10,
			[Gwood] 10,
		},
		.terrain Tplain,
		.goods {
			[Glumber] 1,
		},
		.upkeep {
			[Rfood] 2,
		},
	},[Bfishyard]{
		.name "fishyard",
		.time 25,
		.buildtime 50,
		.costs {
			[Ggold] 5,
			[Gwood] 5,
		},
		.terrain Tpond,
		.goods {
			[Gfish] 1,
		},
		.upkeep {
			[Rfood] 1,
		},
	},[Bmill]{
		.name "mill",
		.time 25,
		.buildtime 50,
		.costs {
			[Ggold] 8,
			[Glumber] 8,
		},
		.terrain Tplain,
		.goods {
			[Gwheat] 2,
		},
		.upkeep {
			[Rfood] 1,
		},
	},[Bfarm]{
		.name "farm",
		.time 25,
		.buildtime 50,
		.costs {
			[Ggold] 8,
			[Glumber] 4,
			[Gwheat] 4,
		},
		.terrain Tplain,
		.goods {
			[Gcattle] 2,
		},
		.upkeep {
			[Rfood] 1,
			[Rlumber] 1,
		},
	},[Bquarry]{
		.name "quarry",
		.time 25,
		.buildtime 50,
		.costs {
			[Ggold] 5,
			[Glumber] 2,
			[Gwheat] 3,
		},
		.terrain Tplain,
		.goods {
			[Gstone] 1,
		},
		.upkeep {
			[Rfood] 1,
		},
	},[Bsmeltery]{
		.name "smeltery",
		.time 25,
		.buildtime 50,
		.costs {
			[Ggold] 12,
			[Glumber] 6,
			[Gstone] 4,
			[Gwood] 2,
		},
		.terrain Tplain,
		.goods {
			[Giron] 1,
		},
		.upkeep {
			[Rfood] 2,
		},
	},[Bforge]{
		.name "forge",
		.time 25,
		.buildtime 50,
		.costs {
			[Ggold] 10,
			[Glumber] 4,
			[Gstone] 4,
			[Giron] 2,
		},
		.terrain Tplain,
		.goods {
			[Gtools] 1,
		},
		.upkeep {
			[Rfood] 2,
		},
	},[Blapidary]{
		.name "lapidary",
		.time 25,
		.buildtime 50,
		.costs {
			[Ggold] 26,
			[Gstone] 16,
			[Giron] 14,
			[Gtools] 14,
		},
		.terrain Tplain,
		.goods {
			[Gjewelry] 1,
		},
		.upkeep {
			[Rfood] 2,
		},
	},[Bmarket]{
		.name "market",
		.time 25,
		.buildtime 50,
		.costs {
			[Ggold] 10,
			[Glumber] 10,
		},
		.terrain Tplain,
		.goods {0},
		.upkeep {
			[Rgold] 1,
			[Rfood] 5,
		},
	}
};
Terrain terrains[] = {
	[Tplain]{
		.name "plain",
		.good -1,
		.initialstock 0,
		.yield 0,
	},[Twoods]{
		.name "woods",
		.good Gwood,
		.initialstock 9001,
		.yield 1,
	},[Tpond]{
		.name "pond",
		.good Gfish,
		.initialstock 9001,
		.yield 1,
	},[Tcrop]{
		.name "crop",
		.good Gwheat,
		.initialstock 800,
		.yield 0.8,
	},[Tmountain]{
		.name "mountain",
		.good Gstone,
		.initialstock 9001,
		.yield 0.8,
	},[Tironvein]{
		.name "ironvein",
		.good Giron,
		.initialstock 9001,
		.yield 0.5,
	},[Tgoldvein]{
		.name "goldvein",
		.good Ggold,
		.initialstock 9001,
		.yield 0.3,
	},
};
