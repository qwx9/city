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
		.abbr "H",
		.buildtime 1000,
		.buildcost {0},
		.terrain Tplain,
		.prodtime 0,
		.product {0},
		.prodcost {0},
		.upkeep {
			[Rfood] 1,
		},
	},[Bfishyard]{
		.name "fishyard",
		.abbr "F",
		.buildtime 50,
		.buildcost {
			[Ggold] 5,
			[Gwood] 5,
		},
		.terrain Tpond,
		.prodtime 25,
		.product {
			[Gfish] 1,
		},
		.prodcost {0},
		.upkeep {
			[Rfood] 1,
		},
	},[Bcarpentry]{
		.name "carpentry",
		.abbr "C",
		.buildtime 50,
		.buildcost {
			[Ggold] 3,
			[Gwood] 3,
		},
		.terrain Tplain,
		.prodtime 25,
		.product {
			[Gwood] 1,
		},
		.prodcost {0},
		.upkeep {
			[Rfood] 1,
		},
	},[Bsawmill]{
		.name "sawmill",
		.abbr "L",
		.buildtime 50,
		.buildcost {
			[Ggold] 10,
			[Gwood] 10,
		},
		.terrain Tplain,
		.prodtime 50,
		.product {
			[Glumber] 1,
		},
		.prodcost {
			[Gwood] 1,
		},
		.upkeep {
			[Rfood] 2,
		},
	},[Bmill]{
		.name "mill",
		.abbr "M",
		.buildtime 50,
		.buildcost {
			[Ggold] 8,
			[Glumber] 8,
		},
		.terrain Tplain,
		.prodtime 25,
		.product {
			[Gwheat] 2,
		},
		.prodcost {0},
		.upkeep {
			[Rfood] 1,
		},
	},[Bfarm]{
		.name "farm",
		.abbr "B",
		.buildtime 50,
		.buildcost {
			[Ggold] 8,
			[Glumber] 4,
			[Gwheat] 4,
		},
		.terrain Tplain,
		.prodtime 25,
		.product {
			[Gcattle] 2,
		},
		.prodcost {
			[Gwheat] 1,
		},
		.upkeep {
			[Rfood] 1,
			[Rlumber] 1,
		},
	},[Bquarry]{
		.name "quarry",
		.abbr "Q",
		.buildtime 50,
		.buildcost {
			[Ggold] 5,
			[Glumber] 2,
			[Gwheat] 3,
		},
		.terrain Tplain,
		.prodtime 25,
		.product {
			[Gstone] 1,
		},
		.prodcost {0},
		.upkeep {
			[Rfood] 1,
		},
	},[Bsmeltery]{
		.name "smeltery",
		.abbr "S",
		.buildtime 50,
		.buildcost {
			[Ggold] 12,
			[Glumber] 6,
			[Gstone] 4,
			[Gwood] 2,
		},
		.terrain Tplain,
		.prodtime 50,
		.product {
			[Giron] 1,
		},
		.prodcost {
			[Gwood] 2,
		},
		.upkeep {
			[Rfood] 2,
		},
	},[Bforge]{
		.name "forge",
		.abbr "F",
		.buildtime 50,
		.buildcost {
			[Ggold] 10,
			[Glumber] 4,
			[Gstone] 4,
			[Giron] 2,
		},
		.terrain Tplain,
		.prodtime 25,
		.product {
			[Gtools] 1,
		},
		.prodcost {
			[Gwood] 1,
			[Giron] 1,
		},
		.upkeep {
			[Rfood] 2,
		},
	},[Blapidary]{
		.name "lapidary",
		.abbr "L",
		.buildtime 50,
		.buildcost {
			[Ggold] 26,
			[Gstone] 16,
			[Giron] 14,
			[Gtools] 14,
		},
		.terrain Tplain,
		.prodtime 100,
		.product {
			[Gjewelry] 1,
		},
		.prodcost {
			[Gwood] 2,
			[Giron] 2,
			[Gtools] 2,
		},
		.upkeep {
			[Rfood] 2,
		},
	},[Bmarket]{
		.name "market",
		.abbr "G",
		.buildtime 50,
		.buildcost {
			[Ggold] 10,
			[Glumber] 10,
		},
		.terrain Tplain,
		.prodtime 0,
		.product {0},
		.prodcost {0},
		.upkeep {
			[Rgold] 1,
			[Rfood] 5,
		},
	}
};
Terrain terrains[] = {
	[Tplain]{
		.name "plain",
		.abbr "p",
		.good -1,
		.initialstock 0,
		.yield 0,
	},[Twoods]{
		.name "woods",
		.abbr "w",
		.good Gwood,
		.initialstock 9001,
		.yield 1,
	},[Tpond]{
		.name "pond",
		.abbr "p",
		.good Gfish,
		.initialstock 9001,
		.yield 1,
	},[Tcrop]{
		.name "crop",
		.abbr "c",
		.good Gwheat,
		.initialstock 800,
		.yield 0.8,
	},[Tmountain]{
		.name "mountain",
		.abbr "m",
		.good Gstone,
		.initialstock 9001,
		.yield 0.8,
	},[Tironvein]{
		.name "ironvein",
		.abbr "i",
		.good Giron,
		.initialstock 9001,
		.yield 0.5,
	},[Tgoldvein]{
		.name "goldvein",
		.abbr "g",
		.good Ggold,
		.initialstock 9001,
		.yield 0.3,
	},
};
