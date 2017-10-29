// Usage:
// * compile hyper.cpp with CU == 0
// * compile init.cpp with CU == 1
// * link them.
// Only the parts defined in #if IN_CU(1) will be included in the second compiling.


#ifndef CU
#define IN_CU(x) 1
#else
#define IN_CU(x) (CU == x)
#endif

#include "classes.h"
#include "hyper.h"

#define CU_INIT IN_CU(0)
#define CU_HYPER IN_CU(0)

#if IN_CU(0)
#include "classes.cpp"
#endif

#if IN_CU(0)
#include "util.cpp"
#include "hyperpoint.cpp"
#include "patterns.cpp"
#include "fieldpattern.cpp"
#include "heptagon.cpp"
#include "language.cpp"
#include "cell.cpp"
#include "flags.cpp"
#include "yendor.cpp"
#include "complex.cpp"
#include "game.cpp"
#include "orbgen.cpp"
#include "monstergen.cpp"
#include "landlock.cpp"
#include "landgen.cpp"
#include "orbs.cpp"
#if CAP_INV
#include "inventory.cpp"
#else
bool inv::on;
#endif
#include "system.cpp"
#include "debug.cpp"
#include "geometry.cpp"
#include "polygons.cpp"
#include "mapeditor.cpp"
#if CAP_MODEL
#include "netgen.cpp"
#endif
#if CAP_TABFONT || CAP_CREATEFONT
#include "nofont.cpp"
#endif
#include "basegraph.cpp"
#include "help.cpp"
#include "config.cpp"
#include "scores.cpp"
#include "menus.cpp"
#include "quit.cpp"
#include "shmup.cpp"
#if CAP_ROGUEVIZ
#include "rogueviz.cpp"
#endif
#include "conformal.cpp"
#include "rug.cpp"
#include "control.cpp"
#include "hud.cpp"
#include "hypgraph.cpp"
#include "graph.cpp"
#include "sound.cpp"
#include "achievement.cpp"
#include "barriers.cpp"
#include "bigstuff.cpp"
#if CAP_TOUR
#include "tour.cpp"
#endif
#if ISMOBILE==0
#include <unistd.h>
#endif
#endif

#if IN_CU(1)
#endif

