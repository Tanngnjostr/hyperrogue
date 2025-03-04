// Hyperbolic Rogue -- minor modes
// Copyright (C) 2011-2019 Zeno Rogue, see 'hyper.cpp' for details

/** \file yendor.cpp
 *  \brief Yendor Quest/Challenge, Pure Tactics Mode, Peace Mode
 */

#include "hyper.h"
namespace hr {

namespace peace { extern bool on; }

EX int hiitemsMax(eItem it) {
  int mx = 0;
  for(auto& a: hiitems) if(a.second[it] > mx) mx = a.second[it];
  return mx;
  }

EX modecode_t modecode();

typedef vector<pair<int, string> > subscoreboard;

void displayScore(subscoreboard& s, int x) {
  int vf = min((vid.yres-64) / 70, vid.xres/80);

  if(get_sync_status() == 1) {
    displayfr(x, 56, 1, vf, "(syncing)", 0xC0C0C0, 0);
    }
  else {
    sort(s.begin(), s.end());
    for(int i=0; i<isize(s); i++) {
      int i0 = 56 + i * vf;
      displayfr(x, i0, 1, vf, its(-s[i].first), 0xC0C0C0, 16);
      displayfr(x+8, i0, 1, vf, s[i].second, 0xC0C0C0, 0);
      }
    }
  }

EX namespace yendor {

  EX bool on = false;
  EX bool generating = false;
  EX bool path = false;
  EX bool everwon = false;
  EX bool won = false;
  bool easy = false;
  
  EX int challenge; // id of the challenge
  EX int lastchallenge;
  
  #if HDR
  #define YF_DEAD 1
  #define YF_WALLS 2
  #define YF_END 4
  #define YF_DEAD5 8

  #define YF_NEAR_IVY   16
  #define YF_NEAR_ELEM  32
  #define YF_NEAR_OVER  64
  #define YF_NEAR_RED   128
  #define YF_REPEAT     512
  #define YF_NEAR_TENT  1024

  #define YF_START_AL   2048
  #define YF_START_CR   4096
  #define YF_CHAOS      8192
  #define YF_RECALL     16384
  #define YF_NEAR_FJORD 32768
  
  #define YF_START_ANY  (YF_START_AL|YF_START_CR)  

  struct yendorlevel {
    eLand l;
    int flags;
    };

  #define YENDORLEVELS 33
  #endif
  
  EX map<modecode_t, array<int, YENDORLEVELS>> bestscore;

  EX eLand nexttostart;

#define LAND_YENDOR_CHAOS 41

  yendorlevel levels[YENDORLEVELS] = {
    {laNone,      0},
    {laHell,      YF_DEAD}, // FORCE BARRIERS?
    {laGraveyard, YF_DEAD5}, 
    {laDesert,    YF_NEAR_IVY}, // IVY OR TENTACLE?
    {laMinefield, YF_END}, // NOT WON, SEEMS OKAY
    {laEmerald,   0}, // WON FINE
    {laOvergrown, 0}, // WON, TOO EASY?
    {laMotion,    YF_START_AL | YF_END}, // NOT WON, SEEMS OKAY
    {laAlchemist, 0}, // ALMOST WON
    {laIvoryTower,YF_START_CR | YF_NEAR_ELEM | YF_REPEAT}, // won cool
    {laMirrorOld, YF_NEAR_OVER}, // OK
    {laWhirlpool, 0}, // cool
    {laIce,       YF_NEAR_ELEM}, // OK
    {laHive,      YF_NEAR_RED}, // OK
    {laCaribbean, 0}, // seems OK
    {laOcean,     YF_WALLS}, // well... stupid, add Caribbean/Fjord
    {laPalace,    0}, // more trapdoors!
    {laZebra,     0}, // TOO HARD?
    {laWineyard,  0}, // hard-ish
    {laStorms,    0}, // ?
    {laLivefjord, 0},
    {laJungle,    0},
    {laPower,     YF_START_CR},
    {laWildWest,  0},
    {laWhirlwind, YF_NEAR_TENT},
    {laHell,      YF_CHAOS | YF_DEAD},
    {laDragon,    YF_DEAD},
    {laReptile,   0},
    {laTortoise,  YF_RECALL},
    {laCocytus,   YF_NEAR_FJORD},
    {laRuins,     YF_DEAD},
    {laCaves,     YF_DEAD5},
    {laWestWall,  YF_START_CR},
    // {laVariant,   YF_DEAD5}, (I do not think this works)
    };
  
  int tscorelast;

  void uploadScore() {
    int tscore = 0;
    for(int i=1; i<YENDORLEVELS; i++)
      if(bestscore[0][i]) tscore += 999 + bestscore[0][i];
    // printf("Yendor score = %d\n", tscore);

    if(tscore > tscorelast) {
      tscorelast = tscore;
      if(tscore >= 1000) achievement_gain("YENDC1", rg::global);
      if(tscore >= 5000) achievement_gain("YENDC2", rg::global);
      if(tscore >= 15000) achievement_gain("YENDC3", rg::global);
      }

    achievement_score(LB_YENDOR_CHALLENGE, tscore);
    }
    
  EX yendorlevel& clev() { return levels[challenge]; }
  
  eLand changeland(int i, eLand l) {
    if(l == laIvoryTower) return laNone;
    if((clev().flags & YF_START_ANY) && i < 20 && l != clev().l) return clev().l;
    if((clev().flags & YF_END) && i > 80 && l == clev().l) return laIce;
    return laNone;
    }
  
  string name;
  eLand first, second, last;

  #if HDR
  struct yendorinfo {
    cell *path[YDIST];
    cell *actualKey;
    bool found;
    bool foundOrb;
    int howfar;
    bignum age;
    yendorinfo() { actualKey = NULL; }
    cell* key() { return path[YDIST-1]; }
    cell *actual_key() { return actualKey ? actualKey : key(); }
    cell* orb() { return path[0]; }
    };
  #endif
  
  EX vector<yendorinfo> yi;

#if HDR
#define NOYENDOR 999999
#endif
  EX int yii = NOYENDOR;
  
  EX int hardness() {
    if(peace::on) return 15; // just to generate monsters
    if(!yendor::generating && !yendor::path && !yendor::on) return 0;
    int thf = 0;
    for(int i=0; i<isize(yi); i++) {
      yendorinfo& ye ( yi[i] );
      if(!ye.foundOrb && ye.howfar > 25)
        thf += (ye.howfar - 25);
      }
    thf -= 2 * (YDIST - 25); 
    if(thf<0) thf = 0;
    return items[itOrbYendor] * 5 + (thf * 5) / (YDIST-25);
    }

  #if HDR
  enum eState { ysUntouched, ysLocked, ysUnlocked };
  #endif
  
  EX eState state(cell *yendor) {
    for(int i=0; i<isize(yi); i++) if(yi[i].path[0] == yendor)
      return yi[i].found ? ysUnlocked : ysLocked;
    return ysUntouched;
    }
  
  EX bool check(cell *yendor) {
    int byi = isize(yi);
    for(int i=0; i<isize(yi); i++) if(yi[i].path[0] == yendor) byi = i;
    if(byi < isize(yi) && yi[byi].found) return false;
    if(byi == isize(yi)) {
      yendorinfo nyi;
      nyi.path[0] = yendor;
      nyi.howfar = 0;
      
      generating = true;
      
      int turns = 0;

      bignum full_id_0;
      
      int odir = 0;

      if(euclid && (penrose || archimedean)) {
        permanent_long_distances(yendor);
        auto at = random_in_distance(yendor, YDIST-1);
        permanent_long_distances(at);
        for(int a=YDIST-2; a>=0; a--) {
          nyi.path[a+1] = at;
          vector<cell*> prev;
          forCellCM(c2, at) if(celldistance(yendor, c2) == a) prev.push_back(c2);
          if(isize(prev))  at = prev[hrand(isize(prev))];
          }
        nyi.path[0] = yendor;
        }
      
      else if(hybri) {
        /* I am lazy */
        for(int i=1; i<=YDIST-1; i++) nyi.path[i] = nyi.path[i-1]->cmove(nyi.path[i-1]->type-1);
        }
      
      else {
        int t = -1;
        bignum full_id;
        bool onlychild = true;

        cellwalker ycw(yendor, odir = hrand(yendor->type));
        ycw--; if(S3 == 3) ycw--;
        setdist(nyi.path[0], 7, NULL);

        for(int i=0; i<YDIST-1; i++) {
          
          if(i > BARLEV-6) {
            setdist(nyi.path[i+7-BARLEV], 7, nyi.path[i+6-BARLEV]);
            if(challenge && !euclid && ycw.at->land != laIvoryTower) {
              eLand ycl = changeland(i, ycw.at->land);
              if(ycl) {
                if(weirdhyperbolic) {
                  buildBarrierNowall(ycw.at, ycl);
                  }
                else if(hyperbolic && ishept(ycw.at)) {
                  int bd = 2 + hrand(2) * 3;
                  buildBarrier(ycw.at, bd, ycl);
                  if(ycw.at->bardir != NODIR && ycw.at->bardir != NOBARRIERS) 
                    extendBarrier(ycw.at);
                  }
                }
              }  
            }

          if(ycw.at->land == laEndorian) {
            // follow the branch in Yendorian
            int bestval = -2000;
            int best = 0, qbest = 0;
            for(int d=0; d<ycw.at->type; d++) {
              setdist(ycw.at, 7, ycw.peek());
              cell *c1 = (ycw+d).cpeek();
              int val = d * (ycw.at->type - d);
              if(c1->wall == waTrunk) val += (i < YDIST-20 ? 1000 : -1000);
              if(val > bestval) qbest = 0, bestval = val;
              if(val == bestval) if(hrand(++qbest) == 0) best = d;
              }
            ycw += best;
            }
          
          else if(binarytiling) {
            // make it challenging
            vector<int> ds;
            for(int d=0; d<ycw.at->type; d++) {
              bool increase;
              if(sol)
                increase = i < YDIST / 4 || i > 3 * YDIST / 4;
              else
                increase = i < YDIST/2;
              if(increase) {
                if(celldistAlt((ycw+d).cpeek()) < celldistAlt(ycw.at))
                  ds.push_back(d);
                }
              else {
                if(celldistAlt((ycw+d).cpeek()) > celldistAlt(ycw.at) && (ycw+d).cpeek() != nyi.path[i-1])
                  ds.push_back(d);
                }
              }
            if(isize(ds)) ycw += ds[hrand(isize(ds))];
            }
          
          else if(trees_known()) { 
            if(i == 0) {
              t = type_in(expansion, yendor, [yendor] (cell *c) { return celldistance(yendor, c); });
              bignum b = expansion.get_descendants(YDIST-1, t);
              full_id_0 = full_id = hrand(b);
              }
            
            #if DEBUG_YENDORGEN
            printf("#%3d t%d %s / %s\n", i, t, full_id.get_str(100).c_str(), expansion.get_descendants(YDIST-1-i, t).get_str(100).c_str());
            for(int tch: expansion.children[t]) {
              printf("     t%d %s\n", tch, expansion.get_descendants(YDIST-2-i, t).get_str(100).c_str());
              }
            #endif

            if(i == 1) 
              onlychild = true;
            if(!onlychild) ycw++;
            if(VALENCE == 3) ycw++;

            onlychild = false;
            
            for(int tch: expansion.children[t]) {
              ycw++;
              if(i == 1)
                tch = type_in(expansion, ycw.cpeek(), [yendor] (cell *c) { return celldistance(yendor, c); });
              auto& sub_id = expansion.get_descendants(YDIST-i-2, tch);
              if(full_id < sub_id) { t = tch; break; }
              
              full_id.addmul(sub_id, -1);
              onlychild = true;
              }
            }
          
          else if(WDIM == 3) {
            int d = celldistance(nyi.path[0], ycw.at);
            vector<cell*> next;
            forCellCM(c, ycw.at) if(celldistance(nyi.path[0], c) > d) next.push_back(c);
            if(!isize(next)) {
              printf("error: no more cells"); 
              ycw.at = ycw.at->move(hrand(ycw.at->type)); 
              }
            else {
              ycw.at = next[hrand(isize(next))];
              }
            nyi.path[i+1] = ycw.at;
            }
          
          else {
            // stupid
            ycw += rev;
            // well, make it a bit more clever on bitruncated a4 grids
            if(a4 && BITRUNCATED && S7 <= 5) {
              if(ycw.at->type == 8 && ycw.cpeek()->type != 8)
                ycw++;
              if(hrand(100) < 10) {
                if(euclid ? (turns&1) : (hrand(100) < 50)) 
                  ycw+=2;
                else
                  ycw-=2;
                turns++;
                }
              }
            }

          if(inmirror(ycw)) ycw = mirror::reflect(ycw);
          ycw += wstep;
          nyi.path[i+1] = ycw.at;
          }
        }
      
      nyi.found = false;
      nyi.foundOrb = false;
  
      for(int i=1; i<YDIST; i++) {
        setdist(nyi.path[i], 7, nyi.path[i-1]);
        if(isEquidLand(nyi.path[i]->land)) {
          // println(hlog, i, ": ", coastvalEdge(nyi.path[i]));
          buildEquidistant(nyi.path[i]);
          }
        }

      setdist(nyi.path[YDIST-1], 7, nyi.path[YDIST-2]);
      cell *key = nyi.path[YDIST-1];
  
      generating = false;
        
      for(int b=10; b>=5; b--) setdist(key, b, nyi.path[YDIST-2]);

      for(int i=-1; i<key->type; i++) {
        cell *c2 = i >= 0 ? key->move(i) : key;
        checkTide(c2);
        c2->monst = moNone; c2->item = itNone;
        if(!passable(c2, NULL, P_MIRROR | P_MONSTER)) {
          if(c2->wall == waCavewall) c2->wall = waCavefloor;
          else if(c2->wall == waDeadwall) c2->wall = waDeadfloor2;
          else if(c2->wall == waLake) c2->wall = waFrozenLake;
          else if(c2->land == laCaribbean) c2->wall = waCIsland;
          else if(c2->land == laOcean) c2->wall = waCIsland;
          else if(c2->land == laRedRock) c2->wall = waRed3;
          else if(c2->land == laWhirlpool) 
            c2->wall = waBoat, c2->monst = moPirate, c2->item = itOrbWater;
          else c2->wall = waNone;
          }
        if(c2->wall == waReptile) c2->wall = waNone;
        if(c2->wall == waMineMine || c2->wall == waMineUnknown) 
          c2->wall = waMineOpen;
        if(c2->wall == waTrapdoor && i == -1)
          c2->wall = waGargoyleFloor;
        if(c2->land == laLivefjord) {
          c2->wall = waSea;
          for(int i=0; i<c2->type; i++) 
            c2->move(i)->wall = waSea;
          }
        if(isGravityLand(c2->land) && key->land == c2->land && c2->land != laWestWall &&
          c2->landparam < key->landparam && c2->wall != waTrunk)
            c2->wall = waPlatform;
        if(c2->land == laReptile && i >= 0)
          c2->wall = waChasm;
        if(c2->land == laMirrorWall && i == -1)
          c2->wall = waNone;
        }
      key->item = itKey;
      
      bool split_found = false;

      if(key->land == laWestWall && trees_known()) {

        int t = type_in(expansion, yendor, [yendor] (cell *c) { return celldistance(yendor, c); });
        int maxage = 10;
        for(int i=0; i<min(items[itOrbYendor], 8); i++)
          maxage *= 10;
        
        nyi.age = maxage - hrand(maxage/2);
        bignum full_id = full_id_0 - nyi.age;
        bool onlychild = true;

        cellwalker ycw(yendor, odir);
        ycw--; if(S3 == 3) ycw--;

        for(int i=0; i<YDIST-1; i++) {          
        
          if(i == 1) onlychild = true;
          if(!onlychild) ycw++;
          if(VALENCE == 3) ycw++;

          onlychild = false;
          
          for(int tch: expansion.children[t]) {
            ycw++;
            if(i == 1)
              tch = type_in(expansion, ycw.cpeek(), [yendor] (cell *c) { return celldistance(yendor, c); });
            auto& sub_id = expansion.get_descendants(YDIST-i-2, tch);
            if(full_id < sub_id) { 
              if(!split_found && !(full_id_0 < sub_id)) {
                ycw.at->item = itRuby;
                split_found = true;
                setdist(ycw.at, 6, NULL);
                auto tt = type_in(expansion, ycw.at, coastvalEdge);
                // println(hlog, "at split, ydist-type: ", t, " coast-type: ", tt);
                if(t != tt) {
                  // try again!
                  key->item = itNone;
                  return check(yendor);
                  }
                }    
              t = tch; 
              break; 
              }
            
            full_id.addmul(sub_id, -1);
            if(!split_found) full_id_0.addmul(sub_id, -1);
            onlychild = true;
            }
          
          if(inmirror(ycw)) ycw = mirror::reflect(ycw);
          ycw += wstep;
          setdist(ycw.at, 8, ycw.peek());
          buildEquidistant(ycw.at);
          // println(hlog, "actual key #", i, ": ", ycw.at->landparam);
          }

        nyi.actualKey = ycw.at;
        ycw.at->item = itKey;
        key->item = itNone;
        }
  
      yi.push_back(nyi);
      }
    addMessage(XLAT("You need to find the right Key to unlock this Orb of Yendor!"));
    if(yi[byi].actualKey)
      addMessage(XLAT("You feel that these directions are %1 turns old.", yi[byi].age.get_str(100)));
    if(yii != byi) {
      yii = byi;
      achievement_gain("YENDOR1");
      playSound(yendor, "pickup-yendor");
      return true;
      }
    return false;
    }
  
  EX void onpath() {
    path = false;
    if(yii < isize(yi)) {
      for(int i=0; i<YDIST; i++) if(yi[yii].path[i]->cpdist <= 7) {
        if(i > yi[yii].howfar) yi[yii].howfar = i;
        path = true;
        }
      }
    }
  
  EX void init(int phase) {
    if(!on) return;
    
    if(phase == 1) {
      won = false;
      if(!easy) items[itOrbYendor] = bestscore[modecode()][challenge];
      chaosmode = (clev().flags & YF_CHAOS) ? 1 : 0;
      specialland = clev().l;
      if(clev().flags & YF_START_AL) {
        specialland = laAlchemist;
        items[itElixir] = 50;
        items[itFeather] = 50;
        }
      if(specialland == laPower) 
        items[itOrbSpeed] = 60, items[itOrbWinter] = 60;
      if(clev().flags & YF_START_CR) {
        specialland = laCrossroads;
        }
      if(specialland == laGraveyard) items[itBone] = 10;
      if(specialland == laEmerald)   items[itEmerald] = 10;
      if(specialland == laCocytus)   items[itFjord] = 10;
      if(!euclid) {
        if(clev().flags & YF_DEAD)   items[itGreenStone] = 100;
        if(clev().flags & YF_DEAD5)  items[itGreenStone] = 5;
        }
      if(clev().flags & YF_RECALL) {
        int yq = items[itOrbYendor];
        items[itOrbRecall] = 60 - yq;
        items[itOrbTime] = 60 - yq;
        items[itOrbEnergy] = 60 - yq;
        items[itOrbTeleport] = 60 - yq;
        items[itOrbSpace] = 60 - yq;
        items[itOrbDash] = 60 - yq;
        items[itOrbFrog] = 60 - yq;
        }
      nexttostart = laNone;
      }
    
    if(phase == 2) {
      cell *c2 = cwt.at->move(0);
      c2->land = firstland;
      if(firstland == laRlyeh) c2->wall = waNone;
      yendor::check(c2);
      if(clev().flags & YF_NEAR_IVY)
        nexttostart = laJungle;
      if(clev().flags & YF_NEAR_FJORD)
        nexttostart = laLivefjord;
      if(clev().flags & YF_NEAR_TENT)
        nexttostart = laRlyeh;
      if(clev().flags & YF_NEAR_ELEM) {
        if(firstland == laIce) {
          nexttostart = laEWater;
          items[itWaterShard] = 10;
          }
        else nexttostart = laEAir;
        }
      if(clev().flags & YF_NEAR_OVER)
        nexttostart = laOvergrown;
      if(clev().flags & YF_NEAR_RED) {
        nexttostart = laRedRock;
        items[itRedGem] = 25;
        }
      if(clev().flags & YF_WALLS) {
        items[itPirate] += 25;
        items[itFjord] += 25;
        }
      if(clev().l == laWhirlpool) {
        items[itWhirlpool] += 10;
        items[itOrbWater] += 150;
        }
      }

    if(phase == 3) {
      cell *c2 = cwt.at->move(0);
      makeEmpty(c2);
      c2->item = itOrbYendor;
      nexttostart = laNone;
      if(clev().flags & YF_RECALL) recallCell = cwt.at;
      }
    }
  
  bool levelUnlocked(int i) {
    yendorlevel& ylev(levels[i]);

    eItem t = treasureType(ylev.l);
    if(ylev.l != laWildWest && hiitemsMax(t) < 10) return false;
    if((ylev.flags & YF_NEAR_ELEM) && hiitemsMax(itElemental) < 10) return false;
    if((ylev.flags & YF_NEAR_RED) && hiitemsMax(itRedGem) < 10) return false;
    if((ylev.flags & YF_NEAR_OVER) && hiitemsMax(itMutant) < 10) return false;
    if((ylev.flags & YF_NEAR_TENT) && hiitemsMax(itStatue) < 10) return false;
    if((ylev.flags & YF_NEAR_FJORD) && hiitemsMax(itFjord) < 10) return false;
    if((ylev.flags & YF_CHAOS) && !chaosUnlocked) return false;
    if((ylev.flags & (YF_DEAD|YF_DEAD5)) && hiitemsMax(itBone) < 10) return false;
    if((ylev.flags & YF_RECALL) && hiitemsMax(itSlime) < 10) return false;
    return true;
    }
  
  struct scoredata {
    string username;
    int scores[landtypes];
    };
  vector<scoredata> scoreboard;

  EX const char *chelp = 
    "There are many possible solutions to the Yendor Quest. In the Yendor "
    "Challenge, you will try many of them!\n\n"    
    "Each challenge takes part in a specific land, and you have to use what "
    "you have available.\n\n"
    
    "You need to obtain an Orb of Yendor in the normal game to activate "
    "this challenge, and (ever) collect 10 treasures in one or two lands "
    "to activate a specific level.\n\n"
    
    "After you complete each challenge, you can try it again, on a harder "
    "difficulty level.\n\n"
    
    "All the solutions showcased in the Yendor Challenge work in the normal "
    "play too. However, passages to other lands, and (sometimes) some land features "
    "are disabled in the Yendor "
    "Challenge, so that you have to use the expected method. Also, "
    "the generation rules are changed slightly for the Palace "
    "and Minefield while you are looking for the Orb of Yendor, "
    "to make the challenge more balanced "
    "(but these changes are also active during the normal Yendor Quest).\n\n"
    
    "You get 1000 points for each challenge won, and 1 extra point for "
    "each extra difficulty level.";
  
  int char_to_yendor(char c) {
    if(c >= 'a' && c <= 'z')
      return c - 'a' + 1;
    if(c >= 'A' && c <= 'Z')
      return c - 'A' + 1 + 26;
    return 0;
    }

  EX void showMenu() {
    set_priority_board(LB_YENDOR_CHALLENGE);
    int s = vid.fsize;
    vid.fsize = vid.fsize * 4/5;
    dialog::init(XLAT("Yendor Challenge"), iinf[itOrbYendor].color, 150, 100);

    for(int i=1; i<YENDORLEVELS; i++) {
      string s;
      yendorlevel& ylev(levels[i]);
      
      if(autocheat || levelUnlocked(i)) {
      
        s = XLATT1(ylev.l);
        
        if(!euclid) {  
          if(ylev.flags & YF_CHAOS) { s = "Chaos mode"; }
          if(ylev.flags & YF_NEAR_IVY) { s += "+"; s += XLATT1(laJungle); }
          if(ylev.flags & YF_NEAR_FJORD) { s += "+"; s += XLATT1(laLivefjord); }
          if(ylev.flags & YF_NEAR_TENT) { s += "+"; s += XLATT1(laRlyeh); }
          if(ylev.flags & YF_NEAR_ELEM) { s += "+"; s += XLATT1(laElementalWall); }
          if(ylev.flags & YF_NEAR_OVER) { s += "+"; s += XLATT1(laOvergrown); }
          if(ylev.flags & YF_NEAR_RED) { s += "+"; s += XLATT1(laRedRock); }
          if(ylev.flags & YF_START_AL) { s += "+"; s += XLATT1(laAlchemist); }
          if(ylev.flags & YF_DEAD) { s += "+"; s += XLATT1(itGreenStone); }
          if(ylev.flags & YF_RECALL) { s += "+"; s += XLATT1(itOrbRecall); }
          }
        }

      else {
        s = "(locked)";
        }
      
      string v;
      if(bestscore[modecode()][i] == 1)
        v = XLAT(" (won!)");
      else if(bestscore[modecode()][i])
        v = XLAT(" (won at level %1!)", its(bestscore[modecode()][i]));
      
      dialog::addSelItem(s, v, i > 26 ? 'A' + i - 27 : 'a' + i-1);
      }

    dialog::addBreak(60);
    if (yendor::on)
      dialog::addItem(XLAT("Return to the normal game"), '0');
    dialog::addSelItem(XLAT(
      easy ? "Challenges do not get harder" : "Each challenge gets harder after each victory"),
      " " + XLAT(easy ? "easy" : "challenge"), '1');
    
    dialog::addBack();
    dialog::addHelp();
    dialog::display();
    
    int yc = char_to_yendor(getcstat);

    if(yc > 0 && yc < YENDORLEVELS) {
      subscoreboard scorehere;

      for(int i=0; i<isize(scoreboard); i++) {
        int sc = scoreboard[i].scores[yc];
        if(sc > 0) 
          scorehere.push_back(
            make_pair(-sc, scoreboard[i].username));
        }
      
      displayScore(scorehere, vid.xres / 4);
      }

    yendor::uploadScore();
    vid.fsize = s;
    
    keyhandler = [] (int sym, int uni) {
      dialog::handleNavigation(sym, uni);
      int new_challenge = char_to_yendor(uni);
      if(new_challenge && new_challenge < YENDORLEVELS) {
        if(levelUnlocked(new_challenge) || autocheat) dialog::do_if_confirmed([new_challenge] {
          challenge = new_challenge;
          restart_game(yendor::on ? rg::nothing : rg::yendor);
          });
        else 
          addMessage("Collect 10 treasures in various lands to unlock the challenges there");
        }
      else if(uni == '0') {
        if(yendor::on) restart_game(rg::yendor);
        }
      else if(uni == '1') easy = !easy;
      else if(uni == '2' || sym == SDLK_F1) gotoHelp(chelp);
      else if(doexiton(sym, uni)) popScreen();
      };
    }
    
  EX void collected(cell* c2) {
    playSound(c2, "tada");
    items[itOrbShield] += 31;
    for(int i=0; i<isize(yendor::yi); i++)
      if(yendor::yi[i].path[0] == c2) 
        yendor::yi[i].foundOrb = true;
    // Shielding always, so that we know that it protects!
    for(int i=0; i<4; i++) switch(hrand(13)) {
      case 0: items[itOrbSpeed] += 31; break;
      case 1: items[itOrbLightning] += 78; break;
      case 2: items[itOrbFlash] += 78; break;
      case 3: items[itOrbTime] += 78; break;
      case 4: items[itOrbWinter] += 151; break;
      case 5: items[itOrbDigging] += 151; break;
      case 6: items[itOrbTeleport] += 151; break;
      case 7: items[itOrbThorns] += 151; break;
      case 8: items[itOrbInvis] += 151; break;
      case 9: items[itOrbPsi] += 151; break;
      case 10: items[itOrbAether] += 151; break;
      case 11: items[itOrbFire] += 151; break;
      case 12: items[itOrbSpace] += 78; break;
      }
    items[itOrbYendor]++; 
    items[itKey]--;
    yendor::everwon = true;
    if(yendor::on) {
      yendor::won = true;
      if(!cheater) {
        dynamicval<int> c(chaosmode, 0);
        yendor::bestscore[modecode()][yendor::challenge] = 
          max(yendor::bestscore[modecode()][yendor::challenge], items[itOrbYendor]);
        yendor::uploadScore();        
        }
      }
    addMessage(XLAT("CONGRATULATIONS!"));
    achievement_collection(itOrbYendor);
    achievement_victory(false);
    }
  
  auto hooks = addHook(clearmemory, 0, [] () {
    yendor::yii = NOYENDOR; yendor::yi.clear();
    }) + addHook(hooks_removecells, 0, [] () {
    eliminate_if(yendor::yi, [] (yendorinfo& i) {
      for(int j=0; j<YDIST; j++) if(is_cell_removed(i.path[j])) {
        DEBB(DF_MEMORY, ("removing a Yendor"));
        if(&yi[yii] == &i) yii = NOYENDOR;
        return true;
        }
      return false;
      });
    });
  EX }

#define MAXTAC 20
EX namespace tactic {

  EX bool trailer = false;
  EX bool on = false;
  EX int id;
  
  map<modecode_t, array<int, landtypes>> recordsum;
  map<modecode_t, array<array<int, MAXTAC>, landtypes> > lsc;
  
  eLand lasttactic;
  
  struct scoredata {
    string username;
    int scores[landtypes];
    };
  map<modecode_t, vector<scoredata>> scoreboard;
  
  int chances(eLand l) {
    if(modecode() != 0 && l != laCamelot) return 3;
    for(auto& ti: land_tac)
      if(ti.l == l) 
        return ti.tries;
    return 0;
    }
  
  int tacmultiplier(eLand l) {
    if(modecode() != 0 && l != laCamelot) return 1;
    if(modecode() != 0 && l == laCamelot) return 3;
    for(auto& ti: land_tac)
      if(ti.l == l)
        return ti.multiplier;
    return 0;
    }
  
  bool tacticUnlocked(eLand l) {
    if(autocheat) return true;
    if(l == laWildWest || l == laDual) return true;
    return hiitemsMax(treasureType(l)) * landMultiplier(l) >= 20;
    }

  EX void record(eLand land, int score, int xc IS(modecode())) {
    if(land >=0 && land < landtypes) {
      for(int i=MAXTAC-1; i; i--) lsc[xc][land][i] = lsc[xc][land][i-1];
      tactic::lsc[xc][land][0] = score;
      }
    int t = chances(land);
    int csum = 0;
    for(int i=0; i<t; i++) if(lsc[xc][land][i] > 0) csum += lsc[xc][land][i];
    if(csum > recordsum[xc][land]) recordsum[xc][land] = csum;
    }

  void record() {
    record(lasttactic, items[treasureType(lasttactic)]);
    }

  void unrecord(eLand land, int xc = modecode()) {
    if(land >=0 && land < landtypes) {
      for(int i=0; i<MAXTAC-1; i++) lsc[xc][land][i] = lsc[xc][land][i+1];
      lsc[xc][land][MAXTAC-1] = -1;
      }
    }

  void unrecord() {
    unrecord(lasttactic);
    }
  
  int tscorelast;

  void uploadScoreCode(int code, int lb) {
    int tscore = 0;
    for(int i=0; i<landtypes; i++) 
      tscore += recordsum[code][i] * tacmultiplier(eLand(i));
    // printf("PTM score = %d\n", tscore);
    
    if(code == 0 && tscore > tscorelast) {
      tscorelast = tscore;
      if(tscore >= 1000) achievement_gain("PTM1", 'x');
      if(tscore >= 5000) achievement_gain("PTM2", 'x');
      if(tscore >= 15000) achievement_gain("PTM3", 'x');
      }
    achievement_score(lb, tscore);
    }

  void uploadScore() {
    uploadScoreCode(0, LB_PURE_TACTICS);
    uploadScoreCode(2, LB_PURE_TACTICS_SHMUP);
    uploadScoreCode(4, LB_PURE_TACTICS_COOP);
    }
  
  EX void showMenu() {

    int xc = modecode();
    
    if(xc == 0) set_priority_board(LB_PURE_TACTICS);
    if(xc == 2) set_priority_board(LB_PURE_TACTICS_SHMUP);
    if(xc == 4) set_priority_board(LB_PURE_TACTICS_COOP);

    cmode = sm::ZOOMABLE;
    mouseovers = XLAT("pure tactics mode") + " - " + mouseovers;
    
    { 
    dynamicval<bool> t(tactic::on, true);
    generateLandList([] (eLand l) { return land_validity(l).flags & lv::appears_in_ptm; });
    }
    
    int nl = isize(landlist);

    int nlm;
    int ofs = dialog::handlePage(nl, nlm, (nl+1)/2);
        
    int vf = min((vid.yres-64-vid.fsize) / nlm, vid.xres/40);
    
    int xr = vid.xres / 64;
    
    if(on) record(specialland, items[treasureType(specialland)]);
    
    getcstat = SDLK_ESCAPE;

    for(int i=0; i<nl; i++) {
      int i1 = i + ofs;
      eLand l = landlist[i1];

      int i0 = 56 + i * vf;
      color_t col;
      
      int ch = chances(l);

      if(!ch) continue;
      
      bool unlocked = tacticUnlocked(l);
      
      if(unlocked) col = linf[l].color; else col = 0x202020;
      
      if(displayfrZH(xr*1, i0, 1, vf-4, XLAT1(linf[l].name), col, 0) && unlocked) {
        getcstat = 1000 + i1;
        }
        
      if(unlocked || autocheat) {
        for(int ii=0; ii<ch; ii++)
          if(displayfrZH(xr*(24+2*ii), i0, 1, (vf-4)*4/5, lsc[xc][l][ii] > 0 ? its(lsc[xc][l][ii]) : "-", col, 16)) 
            getcstat = 1000 + i1;

        if(displayfrZH(xr*(24+2*10), i0, 1, (vf-4)*4/5, 
          its(recordsum[xc][l]) + " x" + its(tacmultiplier(l)), col, 0)) 
            getcstat = 1000 + i1;
        }
      else {
        int m = landMultiplier(l);
        displayfrZ(xr*26, i0, 1, (vf-4)*4/5, 
          XLAT("Collect %1x %2 to unlock", its((20+m-1)/m), treasureType(l)), 
          col, 0);
        }
      }
    
    dialog::displayPageButtons(3, true);

    uploadScore();
    if(on) unrecord(specialland);
    
    if(getcstat >= 1000 && getcstat < 1000 + isize(landlist)) {
      int ld = landlist[getcstat-1000];
      subscoreboard scorehere;
      for(int i=0; i<isize(scoreboard[xc]); i++) {
        int sc = scoreboard[xc][i].scores[ld];
        if(sc > 0) 
          scorehere.push_back(
            make_pair(-sc, scoreboard[xc][i].username));
        }      
      displayScore(scorehere, xr * 50);
      }
     
    keyhandler = [] (int sym, int uni) {
      if(uni >= 1000 && uni < 1000 + isize(landlist)) dialog::do_if_confirmed([uni] {
        stop_game();
        specialland = landlist[uni - 1000];
        restart_game(tactic::on ? rg::nothing : rg::tactic);
        });
      else if(uni == '0') {
        if(tactic::on) {
          stop_game();          
          firstland = laIce;
          restart_game(rg::tactic);
          }
        else popScreen();
        }

      else if(sym == SDLK_F1) gotoHelp(
        "In the pure tactics mode, you concentrate on a specific land. "
        "Your goal to obtain as high score as possible, without using "
        "features of the other lands. You can then compare your score "
        "with your friends!\n\n"
        
        "You need to be somewhat proficient in the normal game to "
        "unlock the given land in this challenge "
        "(collect 20 treasure in the given land, or 2 in case of Camelot).\n\n"
        
        "Since getting high scores in some lands is somewhat luck dependent, "
        "you play each land N times, and your score is based on N consecutive "
        "plays. The value of N depends on how 'fast' the land is to play, and "
        "how random it is.\n\n"
        
        "In the Caribbean, you can access Orbs of Thorns, Aether, and "
        "Space if you have ever collected 25 treasure in their native lands.\n\n"
        
        "The rate of treasure spawn is static in this mode. It is not "
        "increased by killing monsters.\n\n"
        
        "Good luck, and have fun!"
        );
      else if(dialog::handlePageButtons(uni)) ;
      else if(doexiton(sym, uni)) popScreen();
      };
    }

EX }

// Identifiers for the current combinations of game modes
// These are recorded in the save file, so it is somewhat
// important that they do not change for already existing
// modes, as otherwise the scores would be lost. 
// Unfortunately, the codes assigned when HyperRogue had
// just a few special modes did not really follow a specific
// rule, so this system has grown rather ugly as the
// number of special modes kept growing...

// mode codes for 'old' modes, from 0 to 255

int modecodetable[42][6] = {
  {  0, 38, 39, 40, 41, 42}, // softcore hyperbolic
  {  7, 43, 44, 45, 46, 47}, // hardcore hyperbolic
  {  2,  4,  9, 11, 48, 49}, // shmup hyperbolic
  { 13, 50, 51, 52, 53, 54}, // softcore heptagonal hyperbolic
  { 16, 55, 56, 57, 58, 59}, // hardcore heptagonal hyperbolic
  { 14, 15, 17, 18, 60, 61}, // shmup heptagonal hyperbolic
  {  1, 62, 63, 64, 65, 66}, // softcore euclidean
  {  8, 67, 68, 69, 70, 71}, // hardcore euclidean
  {  3,  5, 10, 12, 72, 73}, // shmup euclidean
  {110,111,112,113,114,115}, // softcore spherical
  {116,117,118,119,120,121}, // hardcore spherical
  {122,123,124,125,126,127}, // shmup spherical
  {128,129,130,131,132,133}, // softcore heptagonal spherical
  {134,135,136,137,138,139}, // hardcore heptagonal spherical
  {140,141,142,143,144,145}, // shmup heptagonal spherical
  {146,147,148,149,150,151}, // softcore elliptic
  {152,153,154,155,156,157}, // hardcore elliptic
  {158,159,160,161,162,163}, // shmup elliptic
  {164,165,166,167,168,169}, // softcore heptagonal elliptic
  {170,171,172,173,174,175}, // hardcore heptagonal elliptic
  {176,177,178,179,180,181}, // shmup heptagonal elliptic
  { 19, 74, 75, 76, 77, 78}, // softcore hyperbolic chaosmode
  { 26, 79, 80, 81, 82, 83}, // hardcore hyperbolic chaosmode
  { 21, 23, 28, 30, 84, 85}, // shmup hyperbolic chaosmode
  { 32, 86, 87, 88, 89, 90}, // softcore heptagonal hyperbolic chaosmode
  { 35, 91, 92, 93, 94, 95}, // hardcore heptagonal hyperbolic chaosmode
  { 33, 34, 36, 37, 96, 97}, // shmup heptagonal hyperbolic chaosmode
  { 20, 98, 99,100,101,102}, // softcore euclidean chaosmode
  { 27,103,104,105,106,107}, // hardcore euclidean chaosmode
  { 22, 24, 29, 31,108,109}, // shmup euclidean chaosmode
  {182,183,184,185,186,187}, // softcore spherical chaosmode
  {188,189,190,191,192,193}, // hardcore spherical chaosmode
  {194,195,196,197,198,199}, // shmup spherical chaosmode
  {200,201,202,203,204,205}, // softcore heptagonal spherical chaosmode
  {206,207,208,209,210,211}, // hardcore heptagonal spherical chaosmode
  {212,213,214,215,216,217}, // shmup heptagonal spherical chaosmode
  {218,219,220,221,222,223}, // softcore elliptic chaosmode
  {224,225,226,227,228,229}, // hardcore elliptic chaosmode
  {230,231,232,233,234,235}, // shmup elliptic chaosmode
  {236,237,238,239,240,241}, // softcore heptagonal elliptic chaosmode
  {242,243,244,245,246,247}, // hardcore heptagonal elliptic chaosmode
  {248,249,250,251,252,253}, // shmup heptagonal elliptic chaosmode
  };
// unused codes: 6 (cheat/tampered), 25, 254, 255

modecode_t modecode() {
  // bit 61: product; 62: rotspace
  if(hybri) return PIU(modecode()) | (1ll << (prod ? 61 : 62));
#if CAP_SAVE
  if(anticheat::tampered || cheater || geometry >= gGUARD) return 6;
#endif

  // compute the old code
  int xcode = 0;

  if(shmup::on) xcode += 2;
  else if(pureHardcore()) xcode ++;
  
  if(euclid) xcode += 6;
  else if(!BITRUNCATED) xcode += 3;
  
  if(sphere) {
    xcode += 9;
    if(elliptic) xcode += 6;
    }
  
  if(chaosmode) xcode += 21;
  
  int np = numplayers()-1; if(np<0 || np>5) np=5;
  
  // bits: 0 to 7
  modecode_t mct = modecodetable[xcode][np];

  // bits: 9, 10, 15, 16, 17, 18
  mct += ginf[geometry].xcode;
  
#if CAP_INV
  if(inv::on) mct += (1<<11);
#endif
  if(peace::on) mct += (1<<12);
#if CAP_TOUR
  if(tour::on) mct += (1<<13);
#endif
  if(numplayers() == 7) mct += (1<<14);

  // daily/Yendor/Tactics/standard are saved separately, but are using the same codes (Daily uses no code)
  // randompattern never records its scores
  // no specifics of the advanced configuration of torus/fieldquotient currently recorded

  #if CAP_GP  
  if(GOLDBERG) { 
    mct += (1 << 19);
    auto loc = gp::human_representation(gp::param);
    mct += loc.first << 21; // 4 bits
    mct += loc.second << 25; // 4 bits
    }
  #endif
  
  #if CAP_IRR
  if(IRREGULAR) {
    mct += (1 << 20);
    mct += irr::density_code() << 21; // 8 bits
    }
  #endif
  
  if(DUAL) {
    mct += (1 << 19);
    mct += (1 << 20);
    }
  
  typedef long long ll;
  
  // 32 bits [29..61) for geometry specifics
  if(euwrap) {
    mct += ll(torusconfig::torus_mode) << 29;
    auto& mode = torusconfig::tmodes[torusconfig::torus_mode];
    bool single = (mode.flags & torusconfig::TF_SINGLE);
    if(single) {
      mct += ll(torusconfig::qty) << 37;
      mct += ll(torusconfig::dy) << 45;
      }
    else {
      mct += ll(torusconfig::sdx) << 37;
      mct += ll(torusconfig::sdy) << 45;
      }
    }
  
  #if CAP_FIELD
  if(geometry == gFieldQuotient) {
    using namespace fieldpattern;
    mct += ll(current_extra) << 29;
    mct += ll(fgeomextras[current_extra].current_prime_id) << 37;
    }
  #endif
  
  #if CAP_CRYSTAL
  if(cryst) {
    mct += ll(ginf[geometry].sides) << 29;
    mct += ll(ginf[geometry].vertex) << 37;
    }
  #endif
  
  #if CAP_ARCM
  if(geometry == gArchimedean) {
    unsigned res = 7;
    for(char c: arcm::current.symbol) res = res * 157 + c;
    mct += ll(res) << 29;
    if(PURE) mct ^= (1<<29);
    }
  #endif
  
  return mct;
  }

EX namespace peace {

  EX bool on = false;
  EX bool hint = false;
  
  EX bool otherpuzzles;
  
  eLand simonlevels[] = {
    laCrossroads, laCrossroads2, laDesert, laCaves, laAlchemist, laRlyeh, laEmerald,
    laWineyard, laDeadCaves, laRedRock, laPalace,
    laLivefjord, laDragon,
    laNone
    };

  eLand explorelevels[] = {
    laBurial, laTortoise, laCamelot, laPalace,
    laIce, laJungle, laMirror, laDryForest, laCaribbean, laOcean, laZebra,
    laOvergrown, laWhirlwind, laWarpCoast, laReptile,
    laElementalWall, laAlchemist,
    laNone
    };                       

  eLand *levellist;
  int qty;
  
  void listLevels() {
    levellist = otherpuzzles ? explorelevels : simonlevels;
 
    for(qty = 0; levellist[qty]; qty++);
    }
    
  eLand getNext(eLand last) {
    if(!peace::on) return laNone;
    if(!qty) listLevels();
    if(isElemental(last) && hrand(100) < 90)
      return laNone;
    else if(createOnSea(last))
      return getNewSealand(last);
    else if(isCrossroads(last)) {
      while(isCrossroads(last) || last == laCaribbean || last == laCamelot)
        last = levellist[hrand(qty)];
      if(last == laElementalWall) last = laEFire;
      return last;
      }
    else return pick(laCrossroads, laCrossroads2);
    }
    
  bool isAvailable(eLand l) {
    for(int i=0; explorelevels[i]; i++) if(explorelevels[i] == l) return true;
    return false;
    }
  
  const char *chelp = 
    "In the peaceful mode, you just explore the world, "
    "without any battles; there are also several "
    "navigational puzzles available. In the memory game, "
    "you have to collect as many Dodecahedra as you can, "
    "and return to the starting point -- hyperbolic geometry "
    "makes this extremely difficult! Other hyperbolic puzzles "
    "include the Burial Grounds (excavate the treasures " 
    "using your magical sword), Galápagos (try to find an adult "
    "tortoise matching the baby), Camelot (find the center of "
    "a large hyperbolic circle), and Palace (follow the mouse). "
    "Other places listed are for exploration.";
    
  EX namespace simon {

    vector<cell*> path;
    int tobuild;
    
    void build() {
      if(otherpuzzles || !on) return;
      while(isize(path) < tobuild) {
        cell *cp = path[isize(path)-1];
        cell *cp2 = path[isize(path)-2];
        vector<pair<cell*, cell*>> clister;
        clister.emplace_back(cp, cp);
        
        int id = 0;
        manual_celllister cl;
        while(id < isize(clister)) {
          cell *c = clister[id].first;
          cell *fr = clister[id].second;
          setdist(c, 5, NULL);
          
          forCellEx(c2,c)
            if(!cl.listed(c2) && passable(c2, c, 0) && (c2->land == specialland || c2->land == laTemple) && !c2->item) {
              if(!id) fr = c2;
              bool next;
              if(specialland == laRlyeh)
                next = c2->land == laTemple && (cp2->land == laRlyeh || celldistAlt(c2) < celldistAlt(cp2) - 8); 
              else 
                next = celldistance(c2, cp2) == 8;
              if(next) {
                path.push_back(fr);
                fr->item = itDodeca;
                goto again;
                }              
              clister.emplace_back(c2, fr); 
              cl.add(c2);
              }
          id++;
          }
        printf("Path broken, searched = %d\n", id);
        for(auto t: clister) t.first->item = itPirate;
        return;
        again: ;      
        }
      }
  
    EX void extend() {
      int i = 0;
      while(i<isize(path) && path[i]->item != itDodeca) i++;
      if(tobuild == i+9)
        addMessage("You must collect all the dodecahedra on the path!");
      tobuild = i + 9;
      build();
      }
    
    EX void init() {
      tobuild = 0;
      if(!on) return;
      if(otherpuzzles) { items[itGreenStone] = 500; return; }
      cell *c2 = cwt.at->move(0);
      makeEmpty(c2);
      c2->item = itOrbYendor;
      
      path.clear();
      path.push_back(cwt.at);
      path.push_back(c2);
      extend(); 
      }
    
    EX void restore() {
      for(int i=1; i<isize(path); i++)
        if(path[i]->item == itNone && items[itDodeca])
          path[i]->item = itDodeca, items[itDodeca]--;
      }
  EX }
  
  EX void showMenu() {
    listLevels();
    dialog::init(XLAT(otherpuzzles ? "puzzles and exploration" : "memory game"), 0x40A040, 150, 100);

    for(int i = 0; i<qty; i++) 
      dialog::addItem(XLAT1(linf[levellist[i]].name), 'a'+i);

    dialog::addBreak(100);
    dialog::addItem(XLAT(otherpuzzles ? "memory game" : "puzzles and exploration"), '1');
    dialog::addBoolItem(XLAT("display hints"), hint, '2');
    dialog::addItem(XLAT("Return to the normal game"), '0');

    dialog::addBreak(50);
    dialog::addHelp();
    dialog::addBack();    
    dialog::display();
    
    keyhandler = [] (int sym, int uni) {
      dialog::handleNavigation(sym, uni);
      
      if(uni == '1') otherpuzzles = !otherpuzzles;
      else if(uni >= 'a' && uni < 'a' + qty) dialog::do_if_confirmed([uni] {
        stop_game();
        specialland = levellist[uni - 'a'];
        restart_game(peace::on ? 0 : rg::peace);
        });
      else if(uni == '2') { hint = !hint; popScreen(); }
      else if(uni == '0') {
        firstland = laIce;
        if(peace::on) restart_game(rg::peace);
        }
      else if(uni == 'h' || sym == SDLK_F1) gotoHelp(chelp);
      else if(doexiton(sym, uni)) popScreen();
      };
    }
    
  auto aNext = addHook(hooks_nextland, 100, getNext);
  };

#if CAP_COMMANDLINE
int read_mode_args() {
  using namespace arg;
  if(argis("-Y")) { 
    yendor::on = true;
    shift(); yendor::challenge = argi();
    }
  else if(argis("-peace")) {
    peace::otherpuzzles = true;
    stop_game_and_switch_mode(peace::on ? 0 : rg::peace);
    }
  else if(argis("-pmem")) {
    peace::otherpuzzles = false;
    stop_game_and_switch_mode(peace::on ? 0 : rg::peace);
    }
  TOGGLE('T', tactic::on, stop_game_and_switch_mode(rg::tactic))

  else return 1;
  return 0;
  }

auto ah = addHook(hooks_args, 0, read_mode_args);
#endif
}
