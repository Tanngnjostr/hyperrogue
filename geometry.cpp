// Hyperbolic Rogue -- basic geometry
// Copyright (C) 2011-2019 Zeno Rogue, see 'hyper.cpp' for details

/** \file geometry.cpp
 *  \brief Calculation of basic, and less basic, constants in each geometry
 */

#include "hyper.h"
namespace hr {

#if HDR
struct usershapelayer {
  vector<hyperpoint> list;
  bool sym;
  int rots;
  color_t color;
  hyperpoint shift, spin;
  ld zlevel;
  int texture_offset;
  PPR prio;
  };

extern int usershape_changes;

static const int USERLAYERS = 32;

struct usershape { usershapelayer d[USERLAYERS]; };

struct hpcshape {
  int s, e;
  PPR prio;
  int flags;
  hyperpoint intester;
  struct basic_textureinfo *tinf;
  int texture_offset;
  int shs, she;
  void clear() { s = e = shs = she = texture_offset = 0; prio = PPR::ZERO; tinf = NULL; flags = 0; }
  };

#define SIDE_SLEV 0
#define SIDE_WTS3 3
#define SIDE_WALL 4
#define SIDE_LAKE 5
#define SIDE_LTOB 6
#define SIDE_BTOI 7
#define SIDE_SKY  8
#define SIDE_HIGH 9
#define SIDE_HIGH2 10
#define SIDEPARS  11

#define BADMODEL 0

static const int WINGS = (BADMODEL ? 1 : 4);

typedef array<hpcshape, WINGS+1> hpcshape_animated;

extern vector<hpcshape> shPlainWall3D, shWireframe3D, shWall3D, shMiniWall3D;

struct floorshape {
  bool is_plain;
  int shapeid;
  int id;
  int pstrength; // pattern strength in 3D
  int fstrength; // frame strength in 3D
  PPR prio;
  vector<hpcshape> b, shadow, side[SIDEPARS], gpside[SIDEPARS][MAX_EDGE], levels[SIDEPARS], cone[2];
  floorshape() { prio = PPR::FLOOR; pstrength = fstrength = 10; }
  };

struct plain_floorshape : floorshape {
  ld rad0, rad1;
  void configure(ld r0, ld r1) { rad0 = r0; rad1 = r1; }
  };

extern vector<ld> equal_weights;

// noftype: 0 (shapeid2 is heptagonal or just use shapeid1), 1 (shapeid2 is pure heptagonal), 2 (shapeid2 is Euclidean), 3 (shapeid2 is hexagonal)
struct escher_floorshape : floorshape {
  int shapeid0, shapeid1, noftype, shapeid2;
  ld scale;
  };

struct basic_textureinfo {
  int texture_id;
  vector<glvertex> tvertices; 
  };

/** basic geometry parameters */
struct geometry_information {

  /** distance from heptagon center to another heptagon center */
  ld tessf;

  /** distance from heptagon center to adjacent cell center (either hcrossf or tessf) */
  ld crossf;
  
  /** distance from heptagon center to small heptagon vertex */
  ld hexf;
  
  /** distance from heptagon center to big heptagon vertex */
  ld hcrossf;
  
  /** distance between adjacent hexagon vertices */
  ld hexhexdist;
  
  /** distance between hexagon vertex and hexagon center */
  ld hexvdist;
  
  /** distance between heptagon vertex and hexagon center (either hcrossf or something else) */
  ld hepvdist;

  /** distance from heptagon center to heptagon vertex (either hexf or hcrossf) */
  ld rhexf;

  transmatrix heptmove[MAX_EDGE], hexmove[MAX_EDGE];
  transmatrix invheptmove[MAX_EDGE], invhexmove[MAX_EDGE];

  int base_distlimit;

  /** size of the Sword (from Orb of the Sword), used in the shmup mode */
  ld sword_size;
  /** scale factor for the graphics of most things*/
  ld scalefactor;
  ld orbsize, floorrad0, floorrad1, zhexf;
  ld corner_bonus;
  ld hexshift;
  ld asteroid_size[8];
  ld wormscale;
  ld tentacle_length;
  /** level in product geometries */
  ld plevel;
  /** level for a z-step */
  int single_step;
  /** the number of levels in SL2 */
  int steps;
  
  /** various parameters related to the 3D view */
  ld INFDEEP, BOTTOM, HELLSPIKE, LAKE, WALL, FLOOR, STUFF,
    SLEV[4], FLATEYE,
    LEG0, LEG1, LEG, LEG3, GROIN, GROIN1, GHOST,
    BODY, BODY1, BODY2, BODY3,
    NECK1, NECK, NECK3, HEAD, HEAD1, HEAD2, HEAD3,
    ALEG0, ALEG, ABODY, AHEAD, BIRD, LOWSKY, SKY, HIGH, HIGH2;
  ld human_height, slev;

  ld eyelevel_familiar, eyelevel_human, eyelevel_dog;

#if CAP_SHAPES
hpcshape 
  shSemiFloorSide[SIDEPARS],
  shBFloor[2],
  shWave[8][2],  
  shCircleFloor,
  shBarrel,
  shWall[2], shMineMark[2], shBigMineMark[2], shFan,
  shZebra[5],
  shSwitchDisk,
  shTower[11],
  shEmeraldFloor[6],
  shSemiFeatherFloor[2], 
  shSemiFloor[2], shSemiBFloor[2], shSemiFloorShadow,
  shMercuryBridge[2],
  shTriheptaSpecial[14], 
  shCross, shGiantStar[2], shLake, shMirror,
  shHalfFloor[6], shHalfMirror[3],
  shGem[2], shStar, shDisk, shDiskT, shDiskS, shDiskM, shDiskSq, shRing,   
  shTinyBird, shTinyShark,
  shEgg,
  shSpikedRing, shTargetRing, shSawRing, shGearRing, shPeaceRing, shHeptaRing,
  shSpearRing, shLoveRing,
  shDaisy, shTriangle, shNecro, shStatue, shKey, shWindArrow,
  shGun,
  shFigurine, shTreat,
  shElementalShard,
  // shBranch, 
  shIBranch, shTentacle, shTentacleX, shILeaf[3], 
  shMovestar,
  shWolf, shYeti, shDemon, shGDemon, shEagle, shGargoyleWings, shGargoyleBody,
  shFoxTail1, shFoxTail2,
  shDogBody, shDogHead, shDogFrontLeg, shDogRearLeg, shDogFrontPaw, shDogRearPaw,
  shDogTorso,
  shHawk,
  shCatBody, shCatLegs, shCatHead, shFamiliarHead, shFamiliarEye,
  shWolf1, shWolf2, shWolf3,
  shRatEye1, shRatEye2, shRatEye3,
  shDogStripes,
  shPBody, shPSword, shPKnife,
  shFerocityM, shFerocityF, 
  shHumanFoot, shHumanLeg, shHumanGroin, shHumanNeck, shSkeletalFoot, shYetiFoot,
  shMagicSword, shMagicShovel, shSeaTentacle, shKrakenHead, shKrakenEye, shKrakenEye2,
  shArrow,
  shPHead, shPFace, shGolemhead, shHood, shArmor, 
  shAztecHead, shAztecCap,
  shSabre, shTurban1, shTurban2, shVikingHelmet, shRaiderHelmet, shRaiderArmor, shRaiderBody, shRaiderShirt,
  shWestHat1, shWestHat2, shGunInHand,
  shKnightArmor, shKnightCloak, shWightCloak,
  shGhost, shEyes, shSlime, shJelly, shJoint, shWormHead, shTentHead, shShark, shWormSegment, shSmallWormSegment, shWormTail, shSmallWormTail,
  shSlimeEyes, shDragonEyes, shWormEyes, shGhostEyes,
  shMiniGhost, shMiniEyes,
  shHedgehogBlade, shHedgehogBladePlayer,
  shWolfBody, shWolfHead, shWolfLegs, shWolfEyes,
  shWolfFrontLeg, shWolfRearLeg, shWolfFrontPaw, shWolfRearPaw,
  shFemaleBody, shFemaleHair, shFemaleDress, shWitchDress,
  shWitchHair, shBeautyHair, shFlowerHair, shFlowerHand, shSuspenders, shTrophy,
  shBugBody, shBugArmor, shBugLeg, shBugAntenna,
  shPickAxe, shPike, shFlailBall, shFlailTrunk, shFlailChain, shHammerHead,
  shBook, shBookCover, shGrail,
  shBoatOuter, shBoatInner, shCompass1, shCompass2, shCompass3,
  shKnife, shTongue, shFlailMissile, shTrapArrow,
  shPirateHook, shPirateHood, shEyepatch, shPirateX,
  // shScratch, 
  shHeptaMarker, shSnowball, shSun, shNightStar, shEuclideanSky,
  shSkeletonBody, shSkull, shSkullEyes, shFatBody, shWaterElemental,
  shPalaceGate, shFishTail,
  shMouse, shMouseLegs, shMouseEyes,
  shPrincessDress, shPrinceDress,
  shWizardCape1, shWizardCape2,
  shBigCarpet1, shBigCarpet2, shBigCarpet3,
  shGoatHead, shRose, shRoseItem, shThorns,
  shRatHead, shRatTail, shRatEyes, shRatCape1, shRatCape2,
  shWizardHat1, shWizardHat2,
  shTortoise[13][6],
  shDragonLegs, shDragonTail, shDragonHead, shDragonSegment, shDragonNostril, 
  shDragonWings, 
  shSolidBranch, shWeakBranch, shBead0, shBead1,
  shBatWings, shBatBody, shBatMouth, shBatFang, shBatEye,
  shParticle[16], shAsteroid[8],
  shReptile[5][4],
  shReptileBody, shReptileHead, shReptileFrontFoot, shReptileRearFoot,
  shReptileFrontLeg, shReptileRearLeg, shReptileTail, shReptileEye,

  shTrylobite, shTrylobiteHead, shTrylobiteBody,
  shTrylobiteFrontLeg, shTrylobiteRearLeg, shTrylobiteFrontClaw, shTrylobiteRearClaw,
  
  shBullBody, shBullHead, shBullHorn, shBullRearHoof, shBullFrontHoof,
  
  shButterflyBody, shButterflyWing, shGadflyBody, shGadflyWing, shGadflyEye,

  shTerraArmor1, shTerraArmor2, shTerraArmor3, shTerraHead, shTerraFace, 
  shJiangShi, shJiangShiDress, shJiangShiCap1, shJiangShiCap2,
  
  shAsymmetric,
  
  shPBodyOnly, shPBodyArm, shPBodyHand, shPHeadOnly,
  
  shDodeca;
  
  hpcshape_animated 
    shAnimatedEagle, shAnimatedTinyEagle, shAnimatedGadfly, shAnimatedHawk, shAnimatedButterfly, 
    shAnimatedGargoyle, shAnimatedGargoyle2, shAnimatedBat, shAnimatedBat2;  

  vector<hpcshape> shPlainWall3D, shWireframe3D, shWall3D, shMiniWall3D;
  vector<hyperpoint> walltester;
  
  vector<int> wallstart;
  vector<transmatrix> raywall;

  vector<struct plain_floorshape*> all_plain_floorshapes;
  vector<struct escher_floorshape*> all_escher_floorshapes;

  plain_floorshape
    shFloor, 
    shMFloor, shMFloor2, shMFloor3, shMFloor4, shFullFloor,
    shBigTriangle, shTriheptaFloor, shBigHepta;
  
  escher_floorshape    
    shStarFloor, shCloudFloor, shCrossFloor, shChargedFloor,
    shSStarFloor, shOverFloor, shTriFloor, shFeatherFloor,
    shBarrowFloor, shNewFloor, shTrollFloor, shButterflyFloor,
    shLavaFloor, shLavaSeabed, shSeabed, shCloudSeabed,
    shCaveSeabed, shPalaceFloor, shDemonFloor, shCaveFloor,
    shDesertFloor, shPowerFloor, shRoseFloor, shSwitchFloor,
    shTurtleFloor, shRedRockFloor[3], shDragonFloor;

  ld dlow_table[SIDEPARS], dhi_table[SIDEPARS], dfloor_table[SIDEPARS];

  int prehpc;
  vector<hyperpoint> hpc;
  bool first;

  bool validsidepar[SIDEPARS];

  vector<glvertex> ourshape;
#endif

  hpcshape shFullCross[2];
  hpcshape *last;

  int SD3, SD6, SD7, S12, S14, S21, S28, S42, S36, S84;
  
  vector<int> walloffsets;
  
  vector<array<int, 3>> symmetriesAt;
  
  vector<pair<transmatrix, vector<int> > > cellrotations;  
  
  #ifndef SCALETUNER
  static constexpr
  #endif
  double bscale7 = 1, brot7 = 0, bscale6 = 1, brot6 = 0;
  
  vector<hpcshape*> allshapes;
  
  transmatrix shadowmulmatrix;
  
  map<usershapelayer*, hpcshape> ushr;
  
  void prepare_basics();
  void prepare_compute3();
  void prepare_shapes();
  void prepare_usershapes();

  void hpcpush(hyperpoint h);
  void hpcsquare(hyperpoint h1, hyperpoint h2, hyperpoint h3, hyperpoint h4);
  void chasmifyPoly(double fac, double fac2, int k);
  void shift(hpcshape& sh, double dx, double dy, double dz);
  void initPolyForGL();
  void extra_vertices();
  transmatrix ddi(int a, ld x);
  void drawTentacle(hpcshape &h, ld rad, ld var, ld divby);
  hyperpoint hpxyzsc(double x, double y, double z);
  hyperpoint turtlevertex(int u, double x, double y, double z);
  
  void bshape(hpcshape& sh, PPR prio);
  void finishshape();
  void bshape(hpcshape& sh, PPR prio, double shzoom, int shapeid, double bonus = 0, flagtype flags = 0);
  
  void copyshape(hpcshape& sh, hpcshape& orig, PPR prio);
  void zoomShape(hpcshape& old, hpcshape& newsh, double factor, PPR prio);
  void pushShape(usershapelayer& ds);
  void make_sidewalls();
  void procedural_shapes();
  void make_wall(int id, const vector<hyperpoint> vertices, vector<ld> weights = equal_weights);
  
  void reserve_wall3d(int i);
  void compute_cornerbonus();
  void create_wall3d();
  void configure_floorshapes();
  
  void init_floorshapes();
  void bshape2(hpcshape& sh, PPR prio, int shapeid, struct matrixlist& m);
  void bshape_regular(floorshape &fsh, int id, int sides, int shift, ld size, cell *model);
  void generate_floorshapes_for(int id, cell *c, int siid, int sidir);
  void generate_floorshapes();
  void make_floor_textures_here();

  vector<hyperpoint> get_shape(hpcshape sh);
  void add_cone(ld z0, const vector<hyperpoint>& vh, ld z1);
  void add_prism_sync(ld z0, vector<hyperpoint> vh0, ld z1, vector<hyperpoint> vh1);
  void add_prism(ld z0, vector<hyperpoint> vh0, ld z1, vector<hyperpoint> vh1);
  void shift_last(ld z);
  void shift_shape(hpcshape& sh, ld z);
  void shift_shape_orthogonally(hpcshape& sh, ld z);
  void add_texture(hpcshape& sh);
  void make_ha_3d(hpcshape& sh, bool isarmor, ld scale);
  void make_humanoid_3d(hpcshape& sh);
  void addtri(array<hyperpoint, 3> hs, int kind);
  void make_armor_3d(hpcshape& sh, int kind = 1); 
  void make_foot_3d(hpcshape& sh);
  void make_head_only();
  void make_head_3d(hpcshape& sh);
  void make_paw_3d(hpcshape& sh, hpcshape& legsh);
  void make_abody_3d(hpcshape& sh, ld tail);
  void make_ahead_3d(hpcshape& sh);
  void make_skeletal(hpcshape& sh, ld push = 0);
  void make_revolution(hpcshape& sh, int mx = 180, ld push = 0);
  void make_revolution_cut(hpcshape &sh, int each = 180, ld push = 0, ld width = 99);
  void clone_shape(hpcshape& sh, hpcshape& target);
  void animate_bird(hpcshape& orig, hpcshape_animated& animated, ld body);
  void slimetriangle(hyperpoint a, hyperpoint b, hyperpoint c, ld rad, int lev);
  void balltriangle(hyperpoint a, hyperpoint b, hyperpoint c, ld rad, int lev);
  void make_ball(hpcshape& sh, ld rad, int lev);
  void make_star(hpcshape& sh, ld rad);
  void make_euclidean_sky();
  void adjust_eye(hpcshape& eye, hpcshape head, ld shift_eye, ld shift_head, int q, ld zoom=1);
  void shift_last_straight(ld z);
  void queueball(const transmatrix& V, ld rad, color_t col, eItem what);
  void make_shadow(hpcshape& sh);
  void make_3d_models();
  
  /* Goldberg parameters */
  #if CAP_GP
  struct gpdata_t {
    transmatrix Tf[MAX_EDGE][32][32][6];
    transmatrix corners;
    ld alpha;
    int area;
    };
  shared_ptr<gpdata_t> gpdata;
  #endif
  
  int state;
  int usershape_state;

  /** contains the texture point coordinates for 3D models */
  basic_textureinfo models_texture;
  
  geometry_information() { last = NULL; state = usershape_state = 0; gpdata = NULL; }
  
  void require_basics() { if(state & 1) return; state |= 1; prepare_basics(); }
  void require_shapes() { if(state & 2) return; state |= 2; prepare_shapes(); }
  void require_usershapes() { if(usershape_state == usershape_changes) return; usershape_state = usershape_changes; prepare_usershapes(); }
  int timestamp;
  };
#endif

/** values of hcrossf and hexf for the standard geometry. Since polygons are 
 *  usually drawn in this geometry, the scale in other geometries is usually
 *  based on comparing these values to the values in the other geometry.
 */

#if HDR
static const ld hcrossf7 = 0.620672, hexf7 = 0.378077, tessf7 = 1.090550, hexhexdist7 = 0.566256;
#endif

EX bool scale_used() { return (shmup::on && geometry == gNormal && BITRUNCATED) ? (cheater || autocheat) : true; }

void geometry_information::prepare_basics() {

  DEBBI(DF_INIT | DF_POLY | DF_GEOM, ("prepare_basics"));
  
  hexshift = 0;

  ld fmin, fmax;  

  if(archimedean) 
    ginf[gArchimedean].cclass = gcHyperbolic;

  if(euclid) {
    // dynamicval<eGeometry> g(geometry, gNormal);
    // for(int i=0; i<S84; i++) spinmatrix[i] = spin(i * M_PI / S42);
    if(a4 && !BITRUNCATED) {
      crossf = .5;
      hexf = .5;
      hcrossf = crossf * sqrt(2) / 2;
      hexhexdist = crossf;
      hexvdist = hexf;
      hepvdist = hexf;
      rhexf = crossf * sqrt(2) / 2;
      tessf = crossf;
      }
    else if(a4 && BITRUNCATED) {
      ld s2 = sqrt(2);
      ld xx = 1 - s2 / 2;
      crossf = .5;
      tessf = crossf * s2;
      hexf = .5 * xx * s2;
      hcrossf = crossf;
      hexhexdist = crossf * s2;
      hexvdist = crossf * hypot(1-xx, xx);
      hepvdist = crossf;
      rhexf = hexf;
      tessf = crossf;
      }
    else {
      crossf = .5;
      tessf = crossf * sqrt(3);
      hexf = tessf/3;
      hcrossf = crossf;
      hexhexdist = crossf;
      hexvdist = hexf;
      hepvdist = crossf;
      rhexf = hexf;
      }
    goto finish;
    }
  
  if(hybri) {
    auto t = this;
    ld d = prod ? 1 : 2;
    hybrid::in_underlying_geometry([&] {
      t->rhexf = cgi.rhexf / d;
      t->hexf = cgi.hexf / d;
      t->crossf = cgi.crossf / d;
      t->hcrossf = cgi.crossf / d;
      t->tessf = cgi.tessf / d;
      t->hexvdist = cgi.hexvdist / d;
      t->hexhexdist = hdist(xpush0(cgi.hcrossf), xspinpush0(M_PI*2/S7, cgi.hcrossf)) / d;
      t->base_distlimit = cgi.base_distlimit-1;
      });
    goto hybrid_finish;
    }

  if((sphere || hyperbolic) && WDIM == 3 && !binarytiling) {
    rhexf = hexf = 0.378077;
    crossf = hcrossf = 0.620672;
    tessf = 1.090550;
    hexhexdist = 0.566256;
    goto finish;
    }

  tessf = edge_of_triangle_with_angles(S3 >= OINF ? 0 : 2*M_PI/S3, M_PI/S7, M_PI/S7);
  
  if(elliptic && S7 == 4) tessf = M_PI/2;
  
  hcrossf = edge_of_triangle_with_angles(M_PI/2, M_PI/S7, M_PI/S3);

  crossf = BITRUNCATED ? hcrossf : tessf;
  
  fmin = 0, fmax = tessf;
  for(int p=0; p<100; p++) {
    ld f =  (fmin+fmax) / 2;
    hyperpoint H = xpush0(f);
    hyperpoint H1 = spin(2*M_PI/S7) * H;
    hyperpoint H2 = xpush0(tessf-f);
    ld v1 = intval(H, H1), v2 = intval(H, H2);
    if(v1 < v2) fmin = f; else fmax = f;
    }
  hexf = fmin;
  
  rhexf = BITRUNCATED ? hexf : hcrossf;
  
  if(!euclid && BITRUNCATED && !(S7&1))
    hexshift = ALPHA/2 + ALPHA * ((S7-1)/2) + M_PI;

  finish:
  
  for(int d=0; d<S7; d++)
    heptmove[d] = spin(-d * ALPHA) * xpush(tessf) * spin(M_PI);
    
  for(int d=0; d<S7; d++) 
    hexmove[d] = spin(hexshift-d * ALPHA) * xpush(-crossf)* spin(M_PI);  

  for(int d=0; d<S7; d++) invheptmove[d] = inverse(heptmove[d]);
  for(int d=0; d<S7; d++) invhexmove[d] = inverse(hexmove[d]);

  hexhexdist = hdist(xpush0(crossf), xspinpush0(M_PI*2/S7, crossf));
  
  hexvdist = hdist(xpush0(hexf), xspinpush0(ALPHA/2, hcrossf));

  DEBB(DF_GEOM | DF_POLY,
    (format("S7=%d S6=%d hexf = " LDF" hcross = " LDF" tessf = " LDF" hexshift = " LDF " hexhex = " LDF " hexv = " LDF "\n", S7, S6, hexf, hcrossf, tessf, hexshift, 
    hexhexdist, hexvdist)));  
  
  base_distlimit = ginf[geometry].distlimit[!BITRUNCATED];

  #if CAP_GP
  gp::compute_geometry();  
  #endif
  #if CAP_IRR
  irr::compute_geometry();
  #endif
  #if CAP_ARCM
  if(archimedean) {
    arcm::current.compute_geometry();
    crossf = hcrossf7 * arcm::current.scale();
    hexvdist = arcm::current.scale() * .5;
    rhexf = arcm::current.scale() * .5;
    }
  #endif
  #if CAP_BT
  if(binarytiling) hexvdist = rhexf = 1, tessf = 1, scalefactor = 1, crossf = hcrossf7;
  if(geometry == gHoroRec || penrose || sol || nil || nih) hexvdist = rhexf = .5, tessf = .5, scalefactor = .5, crossf = hcrossf7/2;
  #endif
  #if CAP_BT && MAXMDIM >= 4
  if(binarytiling) binary::build_tmatrix();
  #endif
  
  hybrid_finish:
  
  scalefactor = crossf / hcrossf7;
  orbsize = crossf;

  if(scale_used()) scalefactor *= vid.creature_scale;

  zhexf = BITRUNCATED ? hexf : crossf* .55;
  if(scale_used()) zhexf *= vid.creature_scale;
  if(WDIM == 2 && GDIM == 3) zhexf *= 1.5, orbsize *= 1.2;

  floorrad0 = hexvdist* (GDIM == 3 ? 1 : 0.92);
  floorrad1 = rhexf * (GDIM == 3 ? 1 : 0.94);
  
  if(euclid4) {
    if(!BITRUNCATED)
      floorrad0 = floorrad1 = rhexf * (GDIM == 3 ? 1 : .94);
    else
      floorrad0 = hexvdist * (GDIM == 3 ? 1 : .9),
      floorrad1 = rhexf * (GDIM == 3 ? 1 : .8);
    }
  
  plevel = vid.plevel_factor * scalefactor;
  steps = 0;
  single_step = 1;
  if(hybri && !prod) {
    if(hybrid::underlying == gArchimedean) 
      arcm::current.get_step_values(steps, single_step);
    else {
      single_step = S3 * S7 - 2 * S7 - 2 * S3;
      steps = 2 * S7;    
      if(BITRUNCATED) steps *= S3;
      if(single_step < 0) single_step = -single_step;
      }
    DEBB(DF_GEOM | DF_POLY, ("steps = ", steps, " / ", single_step));
    plevel = M_PI * single_step / steps;
    }
  
  if(hybri) {
    /* we do not want too short creatures, better make the scale factor smaller */
    scalefactor = min(scalefactor, cgi.plevel * 1.8 / vid.height_width);
    }
  
  set_sibling_limit();
  
  prepare_compute3();
  if(hyperbolic && &currfp != &fieldpattern::fp_invalid)
    currfp.analyze(); 
  }

EX transmatrix xspinpush(ld dir, ld dist) {
  if(euclid)
    return eupush(cos(dir) * dist, -sin(dir) * dist);
  else
    return spin(dir) * xpush(dist) * spin(-dir);
  }

EX purehookset hooks_swapdim;

EX namespace geom3 {
  
  // Here we convert between the following parameters:
  
  // abslev: level below the plane
  // lev: level above the world (abslev = depth-lev)
  // projection: projection parameter
  // factor: zoom factor
  
  ld abslev_to_projection(ld abslev) {
    if(sphere || euclid) return vid.camera+abslev;
    return tanh(abslev) / tanh(vid.camera);
    }
  
  ld projection_to_abslev(ld proj) {
    if(sphere || euclid) return proj-vid.camera;
    // tanh(abslev) / tanh(camera) = proj
    return atanh(proj * tanh(vid.camera));
    }
  
  ld lev_to_projection(ld lev) {
    return abslev_to_projection(vid.depth - lev);
    }
  
  ld projection_to_factor(ld proj) {
    return lev_to_projection(0) / proj;
    }
  
  EX ld factor_to_projection(ld fac) {
    return lev_to_projection(0) / fac;
    }
  
  EX ld lev_to_factor(ld lev) { 
    if(prod) return -lev;
    if(WDIM == 3) return lev;
    if(GDIM == 3) return vid.depth - lev;
    return projection_to_factor(lev_to_projection(lev)); 
    }
  EX ld factor_to_lev(ld fac) { 
    if(prod) return -fac;
    if(GDIM == 3) return fac;
    return vid.depth - projection_to_abslev(factor_to_projection(fac)); 
    }
  
  EX void do_auto_eye() {
    if(!vid.auto_eye) return;
    auto& cs = getcs();
    if(cs.charid < 4)
      vid.eye = cgi.eyelevel_human;
    else if(cs.charid < 8)
      vid.eye = cgi.eyelevel_dog;
    else if(cs.charid == 8)
      vid.eye = cgi.eyelevel_familiar;
    }
  
  // how should we scale at level lev
  EX ld scale_at_lev(ld lev) { 
    if(sphere || euclid) return 1;
    return cosh(vid.depth - lev); 
    }
  
  EX string invalid;
  
  EX ld actual_wall_height() {
      if(hybri) return cgi.plevel;
      #if CAP_GP
      if(GOLDBERG && vid.gp_autoscale_heights) 
        return vid.wall_height * min<ld>(4 / hypot_d(2, gp::next), 1);
      #endif
      return vid.wall_height;
      }
  EX }
  
  void geometry_information::prepare_compute3() {
    using namespace geom3;
    DEBBI(DF_INIT | DF_POLY | DF_GEOM, ("geom3::compute"));
    // tanh(depth) / tanh(camera) == vid.alpha
    invalid = "";
    
    if(GDIM == 3) ;
    else if(vid.tc_alpha < vid.tc_depth && vid.tc_alpha < vid.tc_camera)
      vid.alpha = tan_auto(vid.depth) / tan_auto(vid.camera);
    else if(vid.tc_depth < vid.tc_alpha && vid.tc_depth < vid.tc_camera) {
      ld v = vid.alpha * tan_auto(vid.camera);
      if(hyperbolic && (v<1e-6-12 || v>1-1e-12)) invalid = "cannot adjust depth", vid.depth = vid.camera;
      else vid.depth = atan_auto(v);
      }
    else {
      ld v = tan_auto(vid.depth) / vid.alpha;
      if(hyperbolic && (v<1e-12-1 || v>1-1e-12)) invalid = "cannot adjust camera", vid.camera = vid.depth;
      else vid.camera = atan_auto(v);
      }
    
    if(fabs(vid.alpha) < 1e-6) invalid = "does not work with perfect Klein";
  
    if(invalid != "") {
      INFDEEP = .7;
      BOTTOM = .8;
      HELLSPIKE = .85;
      LAKE = .9;
      FLOOR = 1;
      WALL = 1.25;
      SLEV[0] = 1;
      SLEV[1] = 1.08;
      SLEV[2] = 1.16;
      SLEV[3] = 1.24;
      FLATEYE = 1.03;
      LEG1 = 1.025;
      LEG = 1.05;
      LEG3 = 1.075;
      GROIN = 1.09;
      GROIN1 = 1.105;
      GHOST = 1.1;
      BODY = 1.15;
      BODY1 = 1.151;
      BODY2 = 1.152;
      BODY3 = 1.153;
      NECK1 = 1.16;
      NECK = 1.17;
      NECK3 = 1.18;
      HEAD = 1.188;
      HEAD1= 1.189;
      HEAD2= 1.190;
      HEAD3= 1.191;
      ABODY = 1.08;
      AHEAD = 1.12;
      BIRD = 1.20;
      }
    else {
      INFDEEP = GDIM == 3 ? (sphere ? M_PI/2 : +5) : (euclid || sphere) ? 0.01 : lev_to_projection(0) * tanh(vid.camera);
      ld wh = actual_wall_height();
      WALL = lev_to_factor(wh);
      FLOOR = lev_to_factor(0);
      
      human_height = vid.human_wall_ratio * wh;
      if(WDIM == 3) human_height = scalefactor * vid.height_width / 2;
      if(hybri) human_height = min(human_height, cgi.plevel * .9);
      
      ld reduce = (WDIM == 3 ? human_height / 2 : 0);
      
      LEG0  = lev_to_factor(human_height * .0 - reduce);
      LEG1  = lev_to_factor(human_height * .1 - reduce);
      LEG   = lev_to_factor(human_height * .2 - reduce);
      LEG3  = lev_to_factor(human_height * .3 - reduce);
      GROIN = lev_to_factor(human_height * .4 - reduce);
      GROIN1= lev_to_factor(human_height * .5 - reduce);
      BODY  = lev_to_factor(human_height * .6 - reduce);
      BODY1 = lev_to_factor(human_height * .61 - reduce);
      BODY2 = lev_to_factor(human_height * .62 - reduce);
      BODY3 = lev_to_factor(human_height * .63 - reduce);
      NECK1 = lev_to_factor(human_height * .7 - reduce);
      NECK  = lev_to_factor(human_height * .8 - reduce);
      NECK3 = lev_to_factor(human_height * .9 - reduce);
      HEAD  = lev_to_factor(human_height * .97 - reduce);
      HEAD1 = lev_to_factor(human_height * .98 - reduce);
      HEAD2 = lev_to_factor(human_height * .99 - reduce);
      HEAD3 = lev_to_factor(human_height - reduce);
      
      reduce = (GDIM == 3 ? human_height * .3 : 0);
      
      STUFF = lev_to_factor(0) - max(orbsize * 0.3, zhexf * .6);
      
      ABODY = lev_to_factor(human_height * .4 - reduce);
      ALEG0 = lev_to_factor(human_height * .0 - reduce);
      ALEG  = lev_to_factor(human_height * .2 - reduce);
      AHEAD = lev_to_factor(human_height * .6 - reduce);
      BIRD = lev_to_factor(WDIM == 3 ? 0 : (vid.human_wall_ratio+1)/2 * wh * .8);
      GHOST = lev_to_factor(WDIM == 3 ? 0 : human_height * .5);
      FLATEYE = lev_to_factor(human_height * .15);
      
      slev = vid.rock_wall_ratio * wh / 3;
      for(int s=0; s<=3; s++)
        SLEV[s] = lev_to_factor(vid.rock_wall_ratio * wh * s/3);
      LAKE = lev_to_factor(-vid.lake_top);
      HELLSPIKE = lev_to_factor(-(vid.lake_top+vid.lake_bottom)/2);
      BOTTOM = lev_to_factor(-vid.lake_bottom);
      LOWSKY = lev_to_factor(2 * wh);
      HIGH = LOWSKY;
      HIGH2 = lev_to_factor(3 * wh);
      SKY = LOWSKY - 5;
      }
    }    

EX namespace geom3 {
  
EX void apply_always3() {
    for(geometryinfo& gi: ginf) {
      auto &g = gi.g;
      if(vid.always3 && g.gameplay_dimension == 2 && g.graphical_dimension == 2) {
        g.graphical_dimension++;
        g.homogeneous_dimension++;
        g.sig[3] = g.sig[2];
        g.sig[2] = g.sig[1];
        }
      if(!vid.always3 && g.gameplay_dimension == 2 && g.graphical_dimension == 3) {
        g.graphical_dimension--;
        g.homogeneous_dimension--;
        g.sig[1] = g.sig[2];
        g.sig[2] = g.sig[3];
        }
      }
    }
    
  #if MAXMDIM >= 4
EX void switch_always3() {
    if(dual::split(switch_always3)) return;
    if(rug::rugged) rug::close();
    vid.always3 = !vid.always3;
    apply_always3();
    swapmatrix(View);
    callhooks(hooks_swapdim);
    }
#endif

  EX void switch_tpp() {
    if(dual::split(switch_fpp)) return;
    if(pmodel == mdDisk && vid.camera_angle) {
      vid.yshift = 0;
      vid.camera_angle = 0;
      vid.xposition = 0;
      vid.yposition = 0;
      vid.scale = 1;      
      vid.fixed_facing = false;
      }
    else {
      vid.yshift = -0.3;
      vid.camera_angle = -45;
      vid.scale = 18/16. * vid.xres / vid.yres / multi::players;
      vid.xposition = 0;
      vid.yposition = -0.9;
      vid.fixed_facing = true;
      vid.fixed_facing_dir = 90;
      }
    }
    
  EX void switch_fpp() {
#if MAXMDIM >= 4
    if(rug::rugged) rug::close();
    if(dual::split(switch_fpp)) return;
    check_cgi(); cgi.require_basics();
    View = inverse(models::rotmatrix()) * View;
    if(!vid.always3) {
      vid.always3 = true;
      apply_always3();
      ld ms = min<ld>(cgi.scalefactor, 1);
      vid.wall_height = 1.5 * ms;
      if(sphere) {
        vid.depth = M_PI / 6;
        vid.wall_height = M_PI / 3;
        }
      vid.human_wall_ratio = 0.8;
      if(euclid && allowIncreasedSight() && vid.use_smart_range == 0) {
        genrange_bonus = gamerange_bonus = sightrange_bonus = cgi.base_distlimit * 3/2;
        }
      vid.camera = 0;
      vid.depth = ms;
      if(pmodel == mdDisk) pmodel = mdPerspective;
      swapmatrix(View);
      callhooks(hooks_swapdim);
#if CAP_RACING
      racing::player_relative = true;
#endif
      }
    else {
      vid.always3 = false;
      apply_always3();
      vid.wall_height = .3;
      vid.human_wall_ratio = .7;
      vid.camera = 1;
      vid.depth = 1;
      if(pmodel == mdPerspective) pmodel = mdDisk;
      swapmatrix(View);
      callhooks(hooks_swapdim);
      }
    View = models::rotmatrix() * View;
#endif
    }

  EX }

EX geometry_information *cgip;
EX map<string, geometry_information> cgis;

#if HDR
#define cgi (*cgip)
#endif

EX int last_texture_step;

int ntimestamp;

EX void check_cgi() {
  string s;
  auto V = [&] (string a, string b) { s += a; s += ": "; s += b; s += "; "; };
  V("GEO", its(int(geometry)));
  V("VAR", its(int(variation)));
  
  if(GOLDBERG) V("GP", its(gp::param.first) + "," + its(gp::param.second));
  if(IRREGULAR) V("IRR", its(irr::irrid));

  if(archimedean) V("ARCM", arcm::current.symbol);
  
  if(cryst) V("CRYSTAL", its(ginf[gCrystal].sides) + its(ginf[gCrystal].vertex));
  
  if(binarytiling || GDIM == 3) V("WQ", its(vid.texture_step));
  
  if(hybri) V("U", its(int(hybrid::underlying)));
  
  if(prod) V("PL", fts(vid.plevel_factor));
  
  if(binarytiling) V("BT", fts(vid.binary_width));
  
  if(GDIM == 2) { 
    V("CAMERA", fts(vid.camera));
    }
  
  if(WDIM == 2) {
    V("WH", fts(vid.wall_height));
    V("HW", fts(vid.human_wall_ratio));
    V("RW", fts(vid.rock_wall_ratio));
    V("DEPTH", fts(vid.depth));
    V("ASH", ONOFF(vid.gp_autoscale_heights));
    V("LT", fts(vid.lake_top));
    V("LB", fts(vid.lake_bottom));
    }

  V("3D", ONOFF(vid.always3));
  
  if(scale_used()) V("CS", fts(vid.creature_scale));
  
  if(WDIM == 3) V("HTW", fts(vid.height_width));

  V("LQ", its(vid.linequality));
  
  cgip = &cgis[s];
  cgi.timestamp = ++ntimestamp;
  if(hybri) hybrid::underlying_cgip->timestamp = ntimestamp;
  
  if(isize(cgis) > 4) {
    vector<pair<int, string>> timestamps;
    for(auto& t: cgis) timestamps.emplace_back(-t.second.timestamp, t.first);
    sort(timestamps.begin(), timestamps.end());
    while(isize(timestamps) > 4) {
      println(hlog, "erasing geometry ", timestamps.back().second);
      cgis.erase(timestamps.back().second);
      timestamps.pop_back();
      }
    }
  
  if(floor_textures && last_texture_step != vid.texture_step) {
    println(hlog, "changed ", last_texture_step, " to ", vid.texture_step);
    delete floor_textures;
    floor_textures = NULL;
    }
  
  #if MAXMDIM >= 4
  if(!floor_textures && GDIM == 3 && (cgi.state & 2)) 
    make_floor_textures();
  #endif

  }

void clear_cgis() {
  printf("clear_cgis\n");
  for(auto& p: cgis) if(&p.second != &cgi) { cgis.erase(p.first); return; }
  }

auto ah_clear_geo = addHook(hooks_clear_cache, 0, clear_cgis);

}
