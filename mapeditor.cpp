// Hyperbolic Rogue -- map editor and vector graphics editor
// Copyright (C) 2011-2019 Zeno Rogue, see 'hyper.cpp' for details

/** \file mapedit.cpp
 *  \brief HyperRogue editors (map and vector graphics)
 */

#include "hyper.h"
namespace hr {

EX namespace mapeditor {

  #if HDR
  enum eShapegroup { sgPlayer, sgMonster, sgItem, sgFloor, sgWall };
  static const int USERSHAPEGROUPS = 5;
  #endif

  hyperpoint lstart;
  cell *lstartcell;
  ld front_edit = 0.5;
  enum class eFront { sphere_camera, sphere_center, equidistants, const_x, const_y };
  eFront front_config;
  ld front_step = 0.1;

  #if HDR
  struct editwhat {
    double dist;
    int rotid, symid, pointid;
    bool side;
    cell *c;
    };
  #endif
  EX editwhat ew, ewsearch;
  EX bool autochoose = ISMOBILE;
  
  EX void scaleall(ld z) { 
     
     // (mx,my) = (xcb,ycb) + ss * (xpos,ypos) + (mrx,mry) * scale
     
     // (mrx,mry) * (scale-scale') =
     // ss * ((xpos',ypos')-(xpos,ypos))
     
     // mx = xb + ssiz*xpos + mrx * scale
     // mx = xb + ssiz*xpos' + mrx * scale' 
     
     ld mrx = (.0 + mousex - current_display->xcenter) / vid.scale;
     ld mry = (.0 + mousey - current_display->ycenter) / vid.scale;
     
     if(vid.xres > vid.yres) {      
       vid.xposition += (vid.scale - vid.scale*z) * mrx / current_display->scrsize;
       vid.yposition += (vid.scale - vid.scale*z) * mry / current_display->scrsize;
       }

     vid.scale *= z;
     // printf("scale = " LDF "\n", vid.scale);
     #if CAP_TEXTURE
     // display(texture::itt);
     texture::config.itt = xyscale(texture::config.itt, 1/z);
     if(false && texture::config.tstate) {
       calcparam();
       texture::config.perform_mapping();
       if(texture::config.tstate == texture::tsAdjusting) 
         texture::config.finish_mapping();
       }
     #endif
     }
  
#if CAP_EDIT
  EX map<int, cell*> modelcell;

  void handleKeyMap(int sym, int uni);

  EX void applyModelcell(cell *c) {
    if(patterns::whichPattern == 'H') return;
    auto si = patterns::getpatterninfo0(c);
    cell *c2 = modelcell[si.id];
    if(c2) {
      c->wall = c2->wall;
      c->land = c2->land;
      c->monst = c2->monst;
      c->item = c2->item;
      c->landparam = c2->landparam;
      c->wparam = c2->wparam;
      c->mondir = c2->mondir;
      c->stuntime = c2->stuntime;
      c->hitpoints = c2->hitpoints;
      if(c2->mondir != NODIR) {
        auto si2 = patterns::getpatterninfo0(c2);
        c->mondir = (c2->mondir - si2.dir + si.dir + MODFIXER) % c->type;
        // todo reflect
        }
      }
    }
#endif
  }

namespace mapstream {
#if CAP_EDIT

  std::map<cell*, int> cellids;
  vector<cell*> cellbyid;
  vector<char> relspin;
  
  
  void addToQueue(cell* c) {
    if(cellids.count(c)) return;
    int numcells = isize(cellbyid);
    cellbyid.push_back(c);
    cellids[c] = numcells;
    }
  
  int fixspin(int rspin, int dir, int t, int vernum) {
    if(vernum < 11018 && dir == 14)
      return NODIR;
    else if(vernum < 11018 && dir == 15)
      return NOBARRIERS;
    else if(dir >= 0 && dir < t)
      return (dir + rspin) % t;
    else
      return dir;
    }
  
  void save_only_map(fhstream& f) {
    f.write(patterns::whichPattern);
    f.write(geometry);
    char nbtype = char(variation);
    f.write(nbtype);
    #if CAP_GP
    if(GOLDBERG) {
      f.write(gp::param.first);
      f.write(gp::param.second);
      }
    #endif
    if(geometry == gTorus) {
      f.write(torusconfig::qty);
      f.write(torusconfig::dx);
      f.write(torusconfig::dy);
      f.write(torusconfig::sdx);
      f.write(torusconfig::sdy);
      f.write(torusconfig::torus_mode);
      }
    #if CAP_FIELD
    if(geometry == gFieldQuotient) {
      using namespace fieldpattern;
      f.write(quotient_field_changed);
      if(quotient_field_changed) {
        f.write(current_extra);
        f.write(fgeomextras[current_extra].current_prime_id);
        }
      }
    #endif
    #if CAP_CRYSTAL
    if(cryst) {
      f.write(ginf[gCrystal].sides);
      if(ginf[gCrystal].sides == 8)
        f.write(ginf[gCrystal].vertex);
      }
    #endif
    #if CAP_ARCM
    if(geometry == gArchimedean) f.write(arcm::current.symbol);
    #endif
    
    // game settings
    f.write(safety);
    f.write(autocheat);
    f.write(gen_wandering);
    f.write(reptilecheat);
    f.write(timerghost);
    f.write(patterns::canvasback);
    f.write(patterns::whichShape);
    f.write(patterns::subpattern_flags);
    f.write(patterns::whichCanvas);
    f.write(patterns::displaycodes);
    f.write(mapeditor::drawplayer);
    if(patterns::whichCanvas == 'f') f.write(patterns::color_formula);
    
    {
    int i = ittypes; f.write(i);
    for(int k=0; k<i; k++) f.write(items[k]);
    i = motypes; f.write(i);
    for(int k=0; k<i; k++) f.write(kills[k]); 
    }
    
    addToQueue((bounded || euclid) ? currentmap->gamestart() : cwt.at->master->c7);
    for(int i=0; i<isize(cellbyid); i++) {
      cell *c = cellbyid[i];
      if(i) {
        for(int j=0; j<c->type; j++) if(c->move(j) && cellids.count(c->move(j)) && 
          cellids[c->move(j)] < i) {
          int32_t i = cellids[c->move(j)];
          f.write(i);
          f.write_char(c->c.spin(j));
          f.write_char(j);
          break;
          }
        }
      f.write_char(c->land);
      f.write_char(c->mondir);
      f.write_char(c->monst);
      if(c->monst == moTortoise)
        f.write(tortoise::emap[c] = tortoise::getb(c));
      f.write_char(c->wall);
      // f.write_char(c->barleft);
      // f.write_char(c->barright);
      f.write_char(c->item);
      if(c->item == itBabyTortoise)
        f.write(tortoise::babymap[c]);
      f.write_char(c->mpdist);
      // f.write_char(c->bardir);
      f.write(c->wparam); f.write(c->landparam);
      f.write_char(c->stuntime); f.write_char(c->hitpoints);
      for(int j=0; j<c->type; j++) {
        cell *c2 = c->move(j);
        if(c2 && c2->land != laNone) addToQueue(c2);
        }
      }
    printf("cells saved = %d\n", isize(cellbyid));
    int32_t n = -1; f.write(n);
    int32_t id = cellids.count(cwt.at) ? cellids[cwt.at] : -1;
    f.write(id);

    f.write(vid.always3);
    f.write(mutantphase);
    f.write(rosewave);
    f.write(rosephase);
    f.write(turncount);
    int rms = isize(rosemap); f.write(rms);
    for(auto p: rosemap) f.write(cellids[p.first]), f.write(p.second);
    f.write(multi::players);
    if(multi::players > 1)
      for(int i=0; i<multi::players; i++)
        f.write(cellids[multi::player[i].at]);

    cellids.clear();
    cellbyid.clear();
    }
  
  void load_usershapes(fhstream& f) {
    if(f.vernum >= 7400) while(true) {
      int i = f.get<int>();
      if(i == -1) break;
      #if CAP_POLY
      int j = f.get<int>(), l = f.get<int>();
      if(i >= 4) i = 3;
      if(i<0 || i >= mapeditor::USERSHAPEGROUPS) break;
      if(l<0 || l >= USERLAYERS) break;

      initShape(i, j);
      usershapelayer& ds(usershapes[i][j]->d[l]);

      if(f.vernum >= 11008) f.read(ds.zlevel);
      
      f.read(ds.sym); f.read(ds.rots); f.read(ds.color);
      ds.list.clear();
      int siz = f.get<int>();
      
      ds.shift = f.get<hyperpoint>();
      ds.spin = f.get<hyperpoint>();
      
      for(int i=0; i<siz; i++)
        ds.list.push_back(f.get<hyperpoint>());
      #else
      printf("cannot read shapes\n"); exit(1);
      #endif
      }    
    }
  
  void load_only_map(fhstream& f) {
    stop_game();
    if(f.vernum >= 10420 && f.vernum < 10503) {
      int i;
      f.read(i);
      patterns::whichPattern = patterns::ePattern(i);
      }
    else if(f.vernum >= 7400) f.read(patterns::whichPattern);
    
    if(f.vernum >= 10203) {
      f.read(geometry);
      char nbtype;
      f.read(nbtype);
      variation = eVariation(nbtype);
      #if CAP_GP
      if(GOLDBERG) {
        f.read(gp::param.first);
        f.read(gp::param.second);
        }
      #endif
      if(geometry == gTorus) {
        f.read(torusconfig::qty);
        f.read(torusconfig::dx);
        f.read(torusconfig::dy);
        if(f.vernum >= 10504) {
          f.read(torusconfig::sdx);
          f.read(torusconfig::sdy);
          f.read(torusconfig::torus_mode);
          }
        torusconfig::activate();
        }
      #if CAP_CRYSTAL
      if(cryst && f.vernum >= 10504) {
        int sides;
        f.read(sides);
        #if CAP_CRYSTAL
        crystal::set_crystal(sides);
        #endif
        if(sides == 8) {
          int vertices;
          eVariation v = variation;
          f.read(vertices);          
          if(vertices == 3) {
            set_variation(eVariation::bitruncated);
            set_variation(v);
            }
          }
        }
      #endif
      #if CAP_FIELD
      if(geometry == gFieldQuotient) {
        using namespace fieldpattern;
        f.read(quotient_field_changed);
        if(quotient_field_changed) {
          f.read(current_extra);
          f.read(fgeomextras[current_extra].current_prime_id);
          enableFieldChange();
          }
        }
      #endif
      #if CAP_ARCM
      if(geometry == gArchimedean) {
        string& symbol = arcm::current.symbol;
        symbol = f.get<string>();
        arcm::current.parse();
        if(arcm::current.errors > 0) {
          printf("Errors! %s\n", arcm::current.errormsg.c_str());
          }
        }
      #endif
      }
      
    check_cgi();
    cgi.require_basics();

    usershape_changes++;

    initcells();
    if(shmup::on) shmup::init();
    
    if(f.vernum >= 10505) {
      // game settings
      f.read(safety);
      bool b;
      f.read(b); if(b) autocheat = true;
      f.read(gen_wandering);
      f.read(reptilecheat);
      f.read(timerghost);
      f.read(patterns::canvasback);
      f.read(patterns::whichShape);
      f.read(patterns::subpattern_flags);
      f.read(patterns::whichCanvas);
      f.read(patterns::displaycodes);
      f.read(mapeditor::drawplayer);
      if(patterns::whichCanvas == 'f') f.read(patterns::color_formula);
      
      int i;
      f.read(i); if(i > ittypes || i < 0) throw hstream_exception();
      for(int k=0; k<i; k++) f.read(items[k]);
      f.read(i); if(i > motypes || i < 0) throw hstream_exception();
      for(int k=0; k<i; k++) f.read(kills[k]);    
      }

    while(true) {
      cell *c;
      int rspin;
      
      if(isize(cellbyid) == 0) {
        c = currentmap->gamestart();
        rspin = 0;
        }
      else {
        int32_t parent = f.get<int>();
        
        if(parent<0 || parent >= isize(cellbyid)) break;
        int dir = f.read_char();
        cell *c2 = cellbyid[parent];
        dir  = fixspin(dir, relspin[parent], c2->type, f.vernum);
        c = createMov(c2, dir);
        // printf("%p:%d,%d -> %p\n", c2, relspin[parent], dir, c);
        
        // spinval becomes xspinval
        rspin = (c2->c.spin(dir) - f.read_char() + MODFIXER) % c->type;
        if(GDIM == 3 && rspin) {
          println(hlog, "rspin in 3D");
          throw hstream_exception();
          }
        }
      
      cellbyid.push_back(c);
      relspin.push_back(rspin);
      c->land = (eLand) f.read_char();
      c->mondir = fixspin(rspin, f.read_char(), c->type, f.vernum);
      c->monst = (eMonster) f.read_char();
      if(c->monst == moTortoise && f.vernum >= 11001)
        f.read(tortoise::emap[c]);
      c->wall = (eWall) f.read_char();
      // c->barleft = (eLand) f.read_char();
      // c->barright = (eLand) f.read_char();
      c->item = (eItem) f.read_char();
      if(c->item == itBabyTortoise && f.vernum >= 11001)
        f.read(tortoise::babymap[c]);
      c->mpdist = f.read_char();
      c->bardir = NOBARRIERS;
      // fixspin(rspin, f.read_char(), c->type);
      if(f.vernum < 7400) {
        short z;
        f.read(z);
        c->wparam = z;
        }
      else f.read(c->wparam);
      f.read(c->landparam);
      // backward compatibility
      if(f.vernum < 7400 && !isIcyLand(c->land)) c->landparam = HEAT(c);
      c->stuntime = f.read_char();
      c->hitpoints = f.read_char();

      if(patterns::whichPattern)
        mapeditor::modelcell[patterns::getpatterninfo0(c).id] = c;
      }
    
    int32_t whereami = f.get<int>();
    if(whereami >= 0 && whereami < isize(cellbyid))
      cwt.at = cellbyid[whereami];
    else cwt.at = currentmap->gamestart();

    for(int i=0; i<isize(cellbyid); i++) {
      cell *c = cellbyid[i];
      if(c->bardir != NODIR && c->bardir != NOBARRIERS) 
        extendBarrier(c);
      }

    for(int d=BARLEV-1; d>=0; d--)
    for(int i=0; i<isize(cellbyid); i++) {
      cell *c = cellbyid[i];
      if(c->mpdist <= d) 
        for(int j=0; j<c->type; j++) {
          cell *c2 = createMov(c, j);
          setdist(c2, d+1, c);
          }
      }

    relspin.clear();

    if(shmup::on) shmup::init();

    timerstart = time(NULL); turncount = 0; 
    sagephase = 0; hardcoreAt = 0;
    timerstopped = false;
    savecount = 0; savetime = 0;
    cheater = 1;
    
    dynamicval<bool> a3(vid.always3, vid.always3);
    if(f.vernum >= 0xA616) { f.read(vid.always3); geom3::apply_always3(); }
    
    if(f.vernum < 0xA61A) load_usershapes(f);

    if(f.vernum >= 11005) {
      f.read(mutantphase);
      f.read(rosewave);
      f.read(rosephase);
      f.read(turncount);
      int i; f.read(i);
      if(i) havewhat |= HF_ROSE;
      while(i--) { 
        int cid; int val; f.read(cid); f.read(val); 
        if(cid >= 0 && cid < isize(cellbyid)) rosemap[cellbyid[cid]] = val; 
        }
      f.read(multi::players);
      if(multi::players > 1)
        for(int i=0; i<multi::players; i++) {
          auto& mp = multi::player[i];
          int whereami = f.get<int>();
          if(whereami >= 0 && whereami < isize(cellbyid))
            mp.at = cellbyid[whereami];
          else
            mp.at = currentmap->gamestart();
          mp.spin = 0,
          mp.mirrored = false;
          }
      }

    cellbyid.clear();
    restartGraph();
    bfs();
    game_active = true;
    }
  
  void save_usershapes(fhstream& f) {
    int32_t n;
    #if CAP_POLY    
    for(int i=0; i<mapeditor::USERSHAPEGROUPS; i++) for(auto usp: usershapes[i]) {
      usershape *us = usp.second;
      if(!us) continue;
      
      for(int l=0; l<USERLAYERS; l++) if(isize(us->d[l].list)) {
        usershapelayer& ds(us->d[l]);
        f.write(i); f.write(usp.first); f.write(l); 
        f.write(ds.zlevel);      
        f.write(ds.sym); f.write(ds.rots); f.write(ds.color);
        n = isize(ds.list); f.write(n);
        f.write(ds.shift);
        f.write(ds.spin);
        for(int i=0; i<isize(ds.list); i++) f.write(ds.list[i]);
        }
      }
    #endif
    n = -1; f.write(n);
    }
  
  bool saveMap(const char *fname) {
    fhstream f(fname, "wb");
    if(!f.f) return false;
    f.write(f.vernum);
    f.write(dual::state);
    // make sure we save in correct order
    if(dual::state) dual::switch_to(1);
    dual::split_or_do([&] { save_only_map(f); });
    save_usershapes(f);
    return true;
    }
  
  bool loadMap(const string& fname) {
    fhstream f(fname, "rb");
    if(!f.f) return false;
    f.read(f.vernum);
    if(f.vernum > 10505 && f.vernum < 11000) 
      f.vernum = 11005;
    auto ds = dual::state;
    if(f.vernum >= 0xA61A)
      f.read(ds);
    else
      ds = 0;
    if(ds == 1 && dual::state == 0) dual::enable();
    if(ds == 0 && dual::state == 1) dual::disable();
    dual::split_or_do([&] { load_only_map(f); });
    if(dual::state) dual::assign_landsides();
    if(f.vernum >= 0xA61A) 
      load_usershapes(f);
    return true;
    }
  
#endif
  }

namespace mapeditor {

  EX bool drawplayer = true;

  EX cell *drawcell;

#if CAP_EDIT
  int paintwhat = 0;
  int painttype = 0;
  int paintstatueid = 0;
  int radius = 0;
  string paintwhat_str = "clear monster";
  
  cellwalker copysource;
  
  int whichpart;
    
  const char *mapeditorhelp = 
    "This mode allows you to edit the map.\n\n"
    "NOTE: Use at your own risk. Combinations which never "
    "appear in the real game may work in an undefined way "
    "(do not work, look strangely, give strange messages, or crash the game).\n\n"
    "To get the most of this editor, "
    "some knowledge of inner workings of HyperRogue is required. "
    "Each cell has four main fields: land type, wall type, monster type, item type. "
    "The same wall type (especially \"none\", \"sea\", or \"bonfire\") may look or "
    "work a bit differently, based on the land it is in. Sometimes an object may "
    "appear twice on the list due to subtle differences (for example, Demons could "
    "move next turn or not).\n\n"
    "Press w, i, l, or m to choose which aspect of cells to change, "
    "then just click on the cells and they will change. Press 'c' while "
    "hovering over a cell to copy that cell, this copies all information about it. "
    "When copying large areas or placing multi-tile monsters, it might be important where "
    "on the cell you are clicking.\n\n"
    "You can also press 0-9 to apply your changes to a greater radius. "
    "This also affects the copy/paste feature, allowing to copy a larger area.\n\n"
    "Press F2 to save the current map (and F3 to load it). If you try this after "
    "a long game of HyperRogue (without using Orbs of Safety), the filesize will "
    "be very large! "
    "Note however that large structures, such as "
    "Great Walls, large circles and horocycles, are destroyed by this.\n\n"
    "Press 'b' to mark cells as boundaries. Such cells, and cells beyond "
    "them, are not copied by the copy/paste feature, nor saved by the "
    "save feature.\n\n";

  const char* patthelp = 
   "Press 'r' to choose a regular pattern. When a pattern is on, "
   "editing a cell automatically edits all cells which are "
   "equivalent according to this pattern. You can choose from "
   "several patterns, and choose which symmetries matter "
   "for equivalence. Also, you can press Space to switch between "
   "the map and graphics editor quickly -- note that editing floors "
   "with the graphics editor also adheres to the pattern.";
  
  string mehelptext() {
    return XLAT(mapeditorhelp) + XLAT(patthelp);
    }

  struct undo_info {
    cell *c;
    eWall w;
    eItem i;
    eMonster m;
    eLand l;
    char wparam;
    int32_t lparam;
    char dir;
    };
  
  vector<undo_info> undo;
  
  bool checkEq(undo_info& u) {
    return u.w == u.c->wall && u.i == u.c->item && u.m == u.c->monst && u.l == u.c->land &&
      u.dir == u.c->mondir && u.lparam == u.c->landparam && u.wparam == u.c->wparam;
    }
  
  void saveUndo(cell *c) {
    undo_info u;
    u.c=c; u.w = c->wall; u.i = c->item; u.m = c->monst; u.l = c->land; u.dir = c->mondir;
    u.wparam = c->wparam; u.lparam = c->landparam;
    undo.push_back(u);
    }

  undo_info& lastUndo() { return undo[isize(undo)-1]; }
  
  void undoLock() {
    if(!isize(undo) || lastUndo().c) {
      undo_info i; i.c = NULL; undo.push_back(i);
      }
    }

  void applyUndo() {
    while(isize(undo) && !lastUndo().c) undo.pop_back();
    while(isize(undo)) {
      undo_info& i(lastUndo());
      if(!i.c) break;
      i.c->wall = i.w;
      i.c->item = i.i;
      i.c->monst = i.m;
      i.c->land = i.l;
      i.c->mondir = i.dir;
      i.c->wparam = i.wparam; 
      i.c->landparam = i.lparam;
      undo.pop_back();
      }
    }
  
  void checkUndo() {
    if(checkEq(undo[isize(undo)-1])) undo.pop_back();
    }
  
  int itc(int k) {
    if(k == 0) return 0;
    if(k == 1) return 0x40;
    if(k == 2) return 0x80;
    if(k == 3) return 0xFF;
    return 0x20;
    }

  bool choosefile = false;
  
  int editor_fsize() {
    return min(vid.fsize + 5, (vid.yres - 16) /32 );
    }

  void displayFunctionKeys() {
    int fs = editor_fsize();
    displayButton(8, vid.yres-8-fs*11, XLAT("F1 = help"), SDLK_F1, 0);
    displayButton(8, vid.yres-8-fs*10, XLAT("F2 = save"), SDLK_F2, 0);
    displayButton(8, vid.yres-8-fs*9, XLAT("F3 = load"), SDLK_F3, 0);
    displayButton(8, vid.yres-8-fs*7, XLAT("F5 = restart"), SDLK_F5, 0);
    #if CAP_SHOT
    displayButton(8, vid.yres-8-fs*6, XLAT("F6 = HQ shot"), SDLK_F6, 0);
    #endif
    displayButton(8, vid.yres-8-fs*5, XLAT("F7 = player on/off"), SDLK_F7, 0);
    displayButton(8, vid.yres-8-fs*3, XLAT("SPACE = map/graphics"), ' ', 0);
    displayButton(8, vid.yres-8-fs*2, XLAT("ESC = return to the game"), SDLK_ESCAPE, 0);
    }

  EX void showMapEditor() {
    cmode = sm::MAP;
    gamescreen(0);
  
    int fs = editor_fsize();
    
    getcstat = '-';

    displayfr(8, 8 + fs, 2, vid.fsize, paintwhat_str, forecolor, 0);
    displayfr(8, 8+fs*2, 2, vid.fsize, XLAT("use at your own risk!"), 0x800000, 0);

    displayButton(8, 8+fs*4, XLAT("0-9 = radius (%1)", its(radius)), ('0' + (radius+1)%10), 0);
    displayButton(8, 8+fs*5, XLAT("b = boundary"), 'b', 0);
    displayButton(8, 8+fs*6, XLAT("m = monsters"), 'm', 0);
    displayButton(8, 8+fs*7, XLAT("w = walls"), 'w', 0);
    displayButton(8, 8+fs*8, XLAT("i = items"), 'i', 0);
    displayButton(8, 8+fs*9, XLAT("l = lands"), 'l', 0);
    displayfr(8, 8+fs*10, 2, vid.fsize, XLAT("c = copy"), 0xC0C0C0, 0);
    displayButton(8, 8+fs*11, XLAT("u = undo"), 'u', 0);
    if(painttype == 4)
      displayButton(8, 8+fs*12, XLAT("f = flip %1", ONOFF(copysource.mirrored)), 'u', 0);
    displayButton(8, 8+fs*13, XLAT("r = regular"), 'r', 0);
    displayButton(8, 8+fs*14, XLAT("p = paint"), 'p', 0);

    displayFunctionKeys();
    displayButton(8, vid.yres-8-fs*4, XLAT("F8 = settings"), SDLK_F8, 0);
    
    keyhandler = handleKeyMap;
    }
  
  int spillinc() {
    if(radius>=2) return 0;
    if(anyshiftclick) return -1;
    return 1;
    }
  
  EX eShapegroup drawcellShapeGroup() {
    if(drawcell == cwt.at && drawplayer) return sgPlayer;
    if(drawcell->wall == waEditStatue) return sgWall;
    if(drawcell->monst) return sgMonster;
    if(drawcell->item) return sgItem;
    return sgFloor;
    }
  
  EX int drawcellShapeID() {
    if(drawcell == cwt.at && drawplayer) return vid.cs.charid;
    if(drawcell->wall == waEditStatue) return drawcell->wparam;
    if(drawcell->monst) return drawcell->monst;
    if(drawcell->item) return drawcell->item;
    auto si = patterns::getpatterninfo0(drawcell);
    return si.id;
    }

  bool editingShape(eShapegroup group, int id) {
    if(group != mapeditor::drawcellShapeGroup()) return false;
    return id == drawcellShapeID();
    }

  void editCell(const pair<cellwalker, cellwalker>& where) {
    cell *c = where.first.at;
    int cdir = where.first.spin;
    saveUndo(c);
    switch(painttype) {
      case 0: {
        eMonster last = c->monst;
        c->monst = eMonster(paintwhat);
        c->hitpoints = 3;
        c->stuntime = 0;
        c->mondir = cdir;
        
        if((isWorm(c) || isIvy(c) || isMutantIvy(c)) && c->move(cdir) && 
          !isWorm(c->move(cdir)) && !isIvy(c->move(cdir)))
          c->mondir = NODIR;
        
        if(c->monst == moMimic) {
          c->monst = moNone;
          mirror::createMirror(cellwalker(c, cdir, true), 0);
          c->monst = moMimic;
          }

        if(c->monst ==moTortoise && last == moTortoise) {
          cell *c1 = c;
          for(int i=0; i<100; i++) c1 = c1->cmove(hrand(c1->type));
          tortoise::emap[c] = tortoise::getRandomBits();
          }
        break;
        }
      case 1: {
        eItem last = c->item;
        c->item = eItem(paintwhat);
        if(c->item == itBabyTortoise)
          tortoise::babymap[c] = getBits(c) ^ (last == itBabyTortoise ? tortoise::getRandomBits() : 0);
        break;
        }
      case 2: {
        eLand last = c->land;
        c->land = eLand(paintwhat);
        if(isIcyLand(c) && isIcyLand(last))
           HEAT(c) += spillinc() / 100.;
        else if(last == laDryForest && c->land == laDryForest)
          c->landparam += spillinc();
        else if(last == laOcean && c->land == laOcean)
          c->landparam += spillinc();
        else if(last == laHive && c->land == laHive)
          c->landparam += spillinc();
        else
          c->landparam = 0;
        break;
        }
      case 3: {
        eWall last = c->wall;
        c->wall = eWall(paintwhat);
        
        if(last != c->wall) {
          if(hasTimeout(c))
            c->wparam = 10;
          else if(c->wall == waWaxWall)
            c->landparam = hrand(0xFFFFFF + 1);
          }
        else if(hasTimeout(c))
          c->wparam += spillinc();
        
        if(c->wall == waEditStatue) {
          c->wparam = paintstatueid;
          c->mondir = cdir;
          }
        break;
        }
      case 5:
        c->land = laNone;
        c->wall = waNone;
        c->item = itNone;
        c->monst = moNone;
        c->landparam = 0;
        // c->tmp = -1;
        break;
      case 6:
        c->land = laCanvas;
        c->wall = GDIM == 3 ? waWaxWall : waNone;
        c->landparam = paintwhat >> 8;
        break;
      case 4:
        cell *copywhat = where.second.at;
        c->wall = copywhat->wall;
        c->item = copywhat->item;
        c->land = copywhat->land;
        c->monst = copywhat->monst;
        c->landparam = copywhat->landparam;
        c->wparam = copywhat->wparam;
        c->hitpoints = copywhat->hitpoints;
        c->stuntime = copywhat->stuntime; 
        if(copywhat->mondir == NODIR) c->mondir = NODIR;
        else c->mondir = ((where.first.mirrored == where.second.mirrored ? 1 : -1) * (copywhat->mondir - where.second.spin) + cdir + MODFIXER) % c->type;
        break;
      }
    checkUndo();
    }
  
  vector<pair<cellwalker, cellwalker> > spill_list;
  
  void list_spill(cellwalker tgt, cellwalker src, manual_celllister& cl) {
    spill_list.clear(); 
    spill_list.emplace_back(tgt, src);
    int crad = 0, nextstepat = 0;
    for(int i=0; i<isize(spill_list); i++) {
      if(i == nextstepat) {
        crad++; nextstepat = isize(spill_list);
        if(crad > radius) break;
        }
      auto sd = spill_list[i];
      for(int i=0; i<sd.first.at->type; i++) {
        auto sd2 = sd;
        sd2.first = sd2.first + i + wstep;
        if(!cl.add(sd2.first.at)) continue;
        if(sd2.second.at) {
          sd2.second = sd2.second + i + wstep;
          if(sd2.second.at->land == laNone) continue;
          }
        spill_list.push_back(sd2);
        }
      }
    }

  void editAt(cellwalker where, manual_celllister& cl) {

    if(painttype == 4 && radius) {
      if(where.at->type != copysource.at->type) return;
      if(where.spin<0) where.spin=0;
      if(BITRUNCATED && !ctof(mouseover) && ((where.spin&1) != (copysource.spin&1)))
        where += 1;
      }
    if(painttype != 4) copysource.at = NULL;
    list_spill(where, copysource, cl);
    
    for(auto& st: spill_list)
      editCell(st);
    }
  
  void allInPattern(cellwalker where) {

    manual_celllister cl;
    if(!patterns::whichPattern) {
      editAt(where, cl);
      return;
      }

    cl.add(where.at);
    
    int at = 0;
    while(at < isize(cl.lst)) {
      cell *c2 = cl.lst[at];
      at++;
      
      forCellEx(c3, c2) cl.add(c3);
      }
    
    auto si = patterns::getpatterninfo0(where.at);
    int cdir = where.spin;
    if(cdir >= 0) cdir = cdir - si.dir;
    
    for(cell* c2: cl.lst) {
      auto si2 = patterns::getpatterninfo0(c2);
      if(si2.id == si.id) {
        editAt(cellwalker(c2, cdir>=0 ? cdir + si2.dir : -1), cl);
        modelcell[si2.id] = c2;
        }
      }
    }
  
  cellwalker mouseover_cw(bool fix) {
    int d = neighborId(mouseover, mouseover2);
    if(d == -1 && fix) d = hrand(mouseover->type);
    return cellwalker(mouseover, d);
    }
  
  void showList() {
    dialog::v.clear();
    if(painttype == 4) painttype = 0;
    switch(painttype) {
      case 0: 
        for(int i=0; i<motypes; i++) {
          eMonster m = eMonster(i);
          if(
            m == moTongue || m == moPlayer || m == moFireball || m == moBullet ||
            m == moFlailBullet || m == moShadow || m == moAirball ||
            m == moWolfMoved || m == moGolemMoved ||
            m == moTameBomberbirdMoved || m == moKnightMoved ||
            m == moDeadBug || m == moLightningBolt || m == moDeadBird ||
            m == moMouseMoved || m == moPrincessMoved || m == moPrincessArmedMoved) ;
          else if(m == moDragonHead) dialog::vpush(i, "Dragon Head");
          else dialog::vpush(i, minf[i].name);
          }
        break;
      case 1:
        for(int i=0; i<ittypes; i++) dialog::vpush(i, iinf[i].name);
        break;
      case 2:
        for(int i=0; i<landtypes; i++) dialog::vpush(i, linf[i].name);
        break;
      case 3:
        for(int i=0; i<walltypes; i++) if(i != waChasmD) dialog::vpush(i, winf[i].name);
        break;
      }
    // sort(v.begin(), v.end());
    
    if(dialog::infix != "") mouseovers = dialog::infix;
    
    int q = dialog::v.size();
    int percolumn = vid.yres / (vid.fsize+5) - 4;
    int columns = 1 + (q-1) / percolumn;
    
    for(int i=0; i<q; i++) {
      int x = 16 + (vid.xres * (i/percolumn)) / columns;
      int y = (vid.fsize+5) * (i % percolumn) + vid.fsize*2;
      
      int actkey = 1000 + i;
      string vv = dialog::v[i].first;
      if(i < 9) { vv += ": "; vv += ('1' + i); }
      
      displayButton(x, y, vv, actkey, 0);
      }
    keyhandler = [] (int sym, int uni) {
      if(uni >= '1' && uni <= '9') uni = 1000 + uni - '1';
      if(sym == SDLK_RETURN || sym == SDLK_KP_ENTER || sym == '-' || sym == SDLK_KP_MINUS) uni = 1000;
      for(int z=0; z<isize(dialog::v); z++) if(1000 + z == uni) {
        paintwhat = dialog::v[z].second;
        paintwhat_str = dialog::v[z].first;

        mousepressed = false;
        popScreen();

        if(painttype == 3 && paintwhat == waEditStatue)
          dialog::editNumber(paintstatueid, 0, 127, 1, 1, XLAT1("editable statue"), 
            XLAT("These statues are designed to have their graphics edited in the Vector Graphics Editor. Each number has its own, separate graphics.")
            );
        return;
        }
      if(dialog::editInfix(uni)) ;
      else if(doexiton(sym, uni)) popScreen();
      };    
    }

  void handleKeyMap(int sym, int uni) {
    handlePanning(sym, uni);

    // left-clicks are coded with '-', and right-clicks are coded with sym F1
    if(uni == '-' && !holdmouse) undoLock();
    if(uni == '-' && mouseover) {
      allInPattern(mouseover_cw(false));
      holdmouse = true;
      }
    
    if(mouseover) for(int i=0; i<mouseover->type; i++) createMov(mouseover, i);
    if(uni == 'u') applyUndo();
    else if(uni == 'v' || sym == SDLK_F10 || sym == SDLK_ESCAPE) popScreen();
    else if(uni >= '0' && uni <= '9') radius = uni - '0';
    else if(uni == 'm') pushScreen(showList), painttype = 0, dialog::infix = "";
    else if(uni == 'i') pushScreen(showList), painttype = 1, dialog::infix = "";
    else if(uni == 'l') pushScreen(showList), painttype = 2, dialog::infix = "";
    else if(uni == 'w') pushScreen(showList), painttype = 3, dialog::infix = "";
    else if(uni == 'r') pushScreen(patterns::showPattern);
    else if(uni == 't' && mouseover) {
      playermoved = true;
      cwt = mouseover_cw(true);
      }
    else if(uni == 'b') painttype = 5, paintwhat_str = XLAT("boundary");
    else if(uni == 'p') {
      painttype = 6;
      paintwhat_str = "paint";
      dialog::openColorDialog((unsigned&)(paintwhat = (painttype ==6 ? paintwhat : 0x808080)));
      }
    else if(uni == 'G') 
      push_debug_screen();
    else if(sym == SDLK_F5) {
      restart_game();
      }
    else if(sym == SDLK_F2) {
      dialog::openFileDialog(levelfile, XLAT("level to save:"), ".lev", [] () {
        if(mapstream::saveMap(levelfile.c_str())) {
          addMessage(XLAT("Map saved to %1", levelfile));
          return true;
          }
        else {
          addMessage(XLAT("Failed to save map to %1", levelfile));
          return false;
          }
        });
      }
    else if(sym == SDLK_F3)
      dialog::openFileDialog(levelfile, XLAT("level to load:"), ".lev", [] () {
        if(mapstream::loadMap(levelfile.c_str())) {
          addMessage(XLAT("Map loaded from %1", levelfile));
          return true;
          }
        else {
          addMessage(XLAT("Failed to load map from %1", levelfile));
          return false;
          }
        });
#if CAP_SHOT
    else if(sym == SDLK_F6) {
      pushScreen(shot::menu);
      }
#endif
    else if(sym == SDLK_F7) {
      drawplayer = !drawplayer;
      }
    else if(sym == SDLK_F8) {
      pushScreen(map_settings);
      }
    else if(uni == 'c' && mouseover) {
      copysource = mouseover_cw(true);
      painttype = 4;
      paintwhat_str = XLAT("copying");
      }
    else if(uni == 'f') {
      copysource.mirrored = !copysource.mirrored;
      }
    else if(uni == 'h' || sym == SDLK_F1) 
      gotoHelp(mehelptext());
    else if(uni == ' ') {
      popScreen();
      pushScreen(showDrawEditor);
      initdraw(mouseover ? mouseover : cwt.at);
      }
    }

// VECTOR GRAPHICS EDITOR

  const char* drawhelp = 
   "In this mode you can draw your own player characters, "
   "floors, monsters, and items. Press 'e' while hovering over "
   "an object to edit it. Start drawing shapes with 'n', and "
   "add extra vertices with 'a'. Press 0-9 to draw symmetric "
   "pictures easily. More complex pictures can "
   "be created by using several layers ('l'). See the edges of "
   "the screen for more keys.";

  string drawhelptext() { 
    return XLAT(drawhelp);
    }
      
  int dslayer;
  bool coloring;
  color_t colortouse = 0xC0C0C0FFu;
  // fake key sent to change the color
  static const int COLORKEY = (-10000); 

  EX transmatrix drawtrans, drawtransnew;

  #if CAP_POLY
  void loadShape(int sg, int id, hpcshape& sh, int d, int layer) {
    usershapelayer *dsCur = &usershapes[sg][id]->d[layer];
    dsCur->list.clear();
    dsCur->sym = d==2;
    for(int i=sh.s; i < sh.s + (sh.e-sh.s)/d; i++)
      dsCur->list.push_back(cgi.hpc[i]);
    }
  #endif

  EX void drawGhosts(cell *c, const transmatrix& V, int ct) {
    if((cmode & sm::MAP) && lmouseover && darken == 0 &&
      (GDIM == 3 || !mouseout()) && 
      (patterns::whichPattern ? patterns::getpatterninfo0(c).id == patterns::getpatterninfo0(lmouseover).id : c == lmouseover)) {
      queuecircleat(c, .78, 0x00FFFFFF);
      }
    }

  hyperpoint ccenter = C0;
  hyperpoint coldcenter = C0;
  
  unsigned gridcolor = 0xC0C0C040;
  
  hyperpoint in_front_dist(ld d) {
    return direct_exp(lp_iapply(ztangent(d)), 100);
    }
  
  hyperpoint find_mouseh3() {
    if(front_config == eFront::sphere_camera)
      return in_front_dist(front_edit);
    ld step = 0.01;
    ld cdist = 0;
    
    auto idt = inverse(drawtrans);

    auto qu = [&] (ld d) { 
      ld d1 = front_edit;
      hyperpoint h1 = in_front_dist(d);
      if(front_config == eFront::sphere_center) 
        d1 = geo_dist(drawtrans * C0, h1, iTable);
      if(front_config == eFront::equidistants) {
        hyperpoint h = idt * in_front_dist(d);
        d1 = asin_auto(h[2]);
        }
      if(front_config == eFront::const_x) {
        hyperpoint h = idt * in_front_dist(d);
        d1 = asin_auto(h[0]);
        }
      if(front_config == eFront::const_y) {
        hyperpoint h = idt * in_front_dist(d);
        d1 = asin_auto(h[1]);
        }
      return pow(d1 - front_edit, 2);
      };
    
    ld bq = qu(cdist);
    while(abs(step) > 1e-10) {
      ld cq = qu(cdist + step);
      if(cq < bq) cdist += step, bq = cq;
      else step *= -.5;
      }
    return in_front_dist(cdist);
    }
  
  int parallels = 12, meridians = 6;
  ld equi_range = 1;
    
  EX void drawGrid() {
    color_t lightgrid = gridcolor;
    lightgrid -= (lightgrid & 0xFF) / 2;
    transmatrix d2 = drawtrans * rgpushxto0(ccenter) * rspintox(gpushxto0(ccenter) * coldcenter);

    if(GDIM == 3) {
      queuecircleat(mapeditor::drawcell, 1, 0x80D080FF);
      color_t cols[4] = { 0x80D080FF, 0x80D080FF, 0xFFFFFF40, 0x00000040 };
      if(true) {
        transmatrix t = rgpushxto0(find_mouseh3());
        for(int i=0; i<4; i++)
          queueline(t * cpush0(i&1, 0.1), t * cpush0(i&1, -0.1), cols[i], -1, i < 2 ? PPR::LINE : PPR::SUPERLINE);
        }
      if(front_config == eFront::sphere_center) for(int i=0; i<4; i+=2) {
        auto pt = [&] (ld a, ld b) {
          return d2 * direct_exp(spin(a*degree) * cspin(0, 2, b*degree) * xtangent(front_edit), 100);
          };
        for(int ai=0; ai<parallels; ai++) {
          ld a = ai * 360 / parallels;
          for(int b=-90; b<90; b+=5) curvepoint(pt(a,b));
          queuecurve(cols[i + ((ai*4) % parallels != 0)], 0, i < 2 ? PPR::LINE : PPR::SUPERLINE);
          }
        for(int bi=1-meridians; bi<meridians; bi++) {
          ld b = 90 * bi / meridians;
          for(int a=0; a<=360; a+=5) curvepoint(pt(a, b));
          queuecurve(cols[i + (bi != 0)], 0, i < 2 ? PPR::LINE : PPR::SUPERLINE);
          }
        }
      transmatrix T;
      if(front_config == eFront::equidistants) T = Id;
      else if(front_config == eFront::const_x) T = cspin(2, 0, M_PI/2);
      else if(front_config == eFront::const_y) T = cspin(2, 1, M_PI/2);
      else return;
      for(int i=0; i<4; i+=2) {
        for(int u=2; u<=20; u++) {
          PRING(d) {
            curvepoint(d2 * T * xspinpush(M_PI*d/cgi.S42, u/20.) * zpush0(front_edit));
            }
          queuecurve(cols[i + (u%5 != 0)], 0, i < 2 ? PPR::LINE : PPR::SUPERLINE);
          }
        for(int d=0; d<cgi.S84; d++) {
          for(int u=0; u<=20; u++) curvepoint(d2 * T * xspinpush(M_PI*d/cgi.S42, u/20.) * zpush(front_edit) * C0);
          queuecurve(cols[i + (d % (cgi.S84/drawcell->type) != 0)], 0, i < 2 ? PPR::LINE : PPR::SUPERLINE);
          }
        }
      return;
      }

    for(int d=0; d<cgi.S84; d++) {
      unsigned col = (d % (cgi.S84/drawcell->type) == 0) ? gridcolor : lightgrid;
      queueline(d2 * C0, d2 * xspinpush0(M_PI*d/cgi.S42, 1), col, 4 + vid.linequality);
      }
    for(int u=2; u<=20; u++) {
      PRING(d) {
        curvepoint(d2 * xspinpush0(M_PI*d/cgi.S42, u/20.));
        }
      queuecurve((u%5==0) ? gridcolor : lightgrid, 0, PPR::LINE);
      }
    queueline(drawtrans*ccenter, drawtrans*coldcenter, gridcolor, 4 + vid.linequality);
    }

  void drawHandleKey(int sym, int uni);
  
#if CAP_TEXTURE
  static ld brush_sizes[10] = {
    0.001, 0.002, 0.005, 0.0075, 0.01, 0.015, 0.02, 0.05, 0.075, 0.1};
  
  static unsigned texture_colors[] = {
    11,
    0x000000FF,
    0xFFFFFFFF,
    0xFF0000FF,
    0xFFFF00FF,
    0x00FF00FF,
    0x00FFFFFF,
    0x0000FFFF,
    0xFF00FFFF,
    0xC0C0C0FF,
    0x808080FF,
    0x404040FF,
    0x804000FF
    };
#endif

  bool area_in_pi = false;

  ld compute_area(hpcshape& sh) {
    ld area = 0;
    for(int i=sh.s; i<sh.e-1; i++) {
      hyperpoint h1 = cgi.hpc[i];
      hyperpoint h2 = cgi.hpc[i+1];
      if(euclid)
        area += (h2[1] + h1[1]) * (h2[0] - h1[0]) / 2;
      else {
        hyperpoint rh2 = gpushxto0(h1) * h2;
        hyperpoint rh1 = gpushxto0(h2) * h1;
        // ld a1 = atan2(h1[1], h1[0]);
        // ld a2 = atan2(h2[1], h2[0]);
        ld b1 = atan2(rh1[1], rh1[0]);
        ld b2 = atan2(rh2[1], rh2[0]);
        // C0 -> H1 -> H2 -> C0
        // at C0: (a1-a2)
        // at H1: (rh2 - a1 - M_PI)
        // at H2: (a2+M_PI - rh1)
        // total: rh2 - rh1
        // ld z = degree;
        ld x = b2 - b1 + M_PI;
        while(x > M_PI) x -= 2 * M_PI;
        while(x < -M_PI) x += 2 * M_PI;
        area += x;
        }
      }
    return area;
    }

#define EDITING_TRIANGLES (GDIM == 3)

  EX void showDrawEditor() {
#if CAP_POLY
    cmode = sm::DRAW;
    gamescreen(0);
    drawGrid();
    if(callhandlers(false, hooks_prestats)) return;

    if(!mouseout()) getcstat = '-';
    
    int sg;
    string line1, line2;
  
    usershape *us = NULL;
    
    bool intexture = false;

#if CAP_TEXTURE        
    if(texture::config.tstate == texture::tsActive) {
      sg = 16;
      line1 = "texture";
      line2 = "";
      texture::config.data.update();
      intexture = true;
      }
#else
    if(0);
#endif
    
    else {

      sg = drawcellShapeGroup();
      
      switch(sg) {
        case sgPlayer:
          line1 = XLAT("character");
          line2 = csname(vid.cs);
          break;
        
        case sgMonster:
          line1 = XLAT("monster");
          line2 = XLAT1(minf[drawcell->monst].name);
          break;
          
        case sgItem:
          line1 = XLAT("item");
          line2 = XLAT1(iinf[drawcell->item].name);
          break;
        
        case sgFloor:
          line1 = GDIM == 3 ? XLAT("pick something") : XLAT("floor");
          line2 = "#" + its(drawcellShapeID());
          /* line2 = XLAT(ishept(drawcell) ? "heptagonal" : 
            ishex1(drawcell) ? "hexagonal #1" : "hexagonal"); */
          break;        

        case sgWall:
          line1 = XLAT("statue");
          line2 = "#" + its(drawcellShapeID());
          /* line2 = XLAT(ishept(drawcell) ? "heptagonal" : 
            ishex1(drawcell) ? "hexagonal #1" : "hexagonal"); */
          break;        
        }
      
      us =usershapes[drawcellShapeGroup()][drawcellShapeID()];
      }
    
    int fs = editor_fsize();

    // displayButton(8, 8+fs*9, XLAT("l = lands"), 'l', 0);
    displayfr(8, 8+fs, 2, vid.fsize, line1, 0xC0C0C0, 0);
    
    if(!intexture) {
      if(sg == sgFloor)
        displayButton(8, 8+fs*2, line2 + XLAT(" (r = complex tesselations)"), 'r', 0);
      else
        displayfr(8, 8+fs*2, 2, vid.fsize, line2, 0xC0C0C0, 0);
      displayButton(8, 8+fs*3, XLAT(GDIM == 3 ? "l = color group: %1" : "l = layers: %1", its(dslayer)), 'l', 0);
      }

    if(us && isize(us->d[dslayer].list)) {
      usershapelayer& ds(us->d[dslayer]);
      if(!EDITING_TRIANGLES) {
        displayButton(8, 8+fs*4, XLAT("1-9 = rotations: %1", its(ds.rots)), '1' + (ds.rots % 9), 0);
        displayButton(8, 8+fs*5, XLAT(ds.sym ? "0 = symmetry" : "0 = asymmetry"), '0', 0);
        }

      displayfr(8, 8+fs*7, 2, vid.fsize, XLAT("%1 vertices", its(isize(ds.list))), 0xC0C0C0, 0);
      displaymm('a', 8, 8+fs*8, 2, vid.fsize, XLAT("a = add v"), 0);
      if(!EDITING_TRIANGLES) {
        if(autochoose) {
          displaymm('m', 8, 8+fs*9, 2, vid.fsize, XLAT("m = move v"), 0);
          displaymm('d', 8, 8+fs*10, 2, vid.fsize, XLAT("d = delete v"), 0);
          }
        else {
          displayButton(8, 8+fs*9, XLAT("m = move v"), 'm', 0);
          displayButton(8, 8+fs*10, XLAT("d = delete v"), 'd', 0);
          }
        displaymm('c', 8, 8+fs*11, 2, vid.fsize, XLAT(autochoose ? "autochoose" : "c = choose"), 0);
        displayButton(8, 8+fs*12, XLAT("b = switch auto"), 'b', 0);
        }
      else {
        displayfr(8, 8+fs*9, 2, vid.fsize, XLAT("c = reuse"), 0xC0C0C0, 0);
        displayfr(8, 8+fs*10, 2, vid.fsize, XLAT("d = delete"), 0xC0C0C0, 0);
        }

      if(GDIM == 2) {
        displayfr(8, 8+fs*14, 2, vid.fsize, XLAT("t = shift"), 0xC0C0C0, 0);
        displayfr(8, 8+fs*15, 2, vid.fsize, XLAT("y = spin"), 0xC0C0C0, 0);
        }
      if(mousekey == 'g')
        displayButton(8, 8+fs*16, XLAT("p = grid color"), 'p', 0);
      else
        displayButton(8, 8+fs*16, XLAT("p = paint"), 'p', 0);
      if(GDIM == 2) 
        displayfr(8, 8+fs*17, 2, vid.fsize, XLAT("z = z-level"), 0xC0C0C0, 0);

      }
#if CAP_TEXTURE
    else if(texture::config.tstate == texture::tsActive) {
      displayButton(8, 8+fs*2, XLAT(texture::texturesym ? "0 = symmetry" : "0 = asymmetry"), '0', 0);
      if(mousekey == 'g')
        displayButton(8, 8+fs*16, XLAT("p = grid color"), 'p', 0);
      else
        displayButton(8, 8+fs*16, XLAT("p = color"), 'p', 0);
      displayButton(8, 8+fs*4, XLAT("b = brush size: %1", fts(texture::penwidth)), 'b', 0);
      displayButton(8, 8+fs*5, XLAT("u = undo"), 'u', 0);
      displaymm('d', 8, 8+fs*7, 2, vid.fsize, XLAT("d = draw"), 0);
      displaymm('l', 8, 8+fs*8, 2, vid.fsize, XLAT("l = line"), 0);
      displaymm('c', 8, 8+fs*9, 2, vid.fsize, XLAT("c = circle"), 0);
      int s = isize(texture::config.data.pixels_to_draw);
      if(s) displaymm(0, 8, 8+fs*11, 2, vid.fsize, its(s), 0);
      }
#endif
    else {
      displaymm('n', 8, 8+fs*5, 2, vid.fsize, XLAT("'n' to start"), 0);
      displaymm('u', 8, 8+fs*6, 2, vid.fsize, XLAT("'u' to load current"), 0);
      if(mousekey == 'a' || mousekey == 'd' || mousekey == 'd' ||
        mousekey == 'c') mousekey = 'n';
      }
    
    if(GDIM == 3)
      displayfr(8, 8+fs*19, 2, vid.fsize, XLAT(front_config == eFront::sphere_camera ? "z = camera" : front_config == eFront::sphere_center ? "z = spheres" : 
        nonisotropic && front_config == eFront::equidistants ? "Z =" :
        nonisotropic && front_config == eFront::const_x ? "X =" :
        nonisotropic && front_config == eFront::const_y ? "Y =" :
        "z = equi") + " " + fts(front_edit), 0xC0C0C0, 0);

    displaymm('g', vid.xres-8, 8+fs*4, 2, vid.fsize, XLAT("g = grid"), 16);

#if CAP_TEXTURE    
    if(intexture) for(int i=0; i<10; i++) {
      if(8 + fs * (6+i) < vid.yres - 8 - fs * 7)
        displayColorButton(vid.xres-8, 8+fs*(6+i), "###", 1000 + i, 16, 1, dialog::displaycolor(texture_colors[i+1]));

      if(displayfr(vid.xres-8 - fs * 3, 8+fs*(6+i), 0, vid.fsize, its(i+1), texture::penwidth == brush_sizes[i] ? 0xFF8000 : 0xC0C0C0, 16))
        getcstat = 2000+i;
      }

    if(texture::config.tstate != texture::tsActive)
      displaymm('e', vid.xres-8, 8+fs, 2, vid.fsize, XLAT("e = edit this"), 16);
#endif

    if(!mouseout()) {
      hyperpoint mh;
      if(GDIM == 2) {
        transmatrix T = inverse(drawtrans * rgpushxto0(ccenter));
        mh = spintox(gpushxto0(ccenter) * coldcenter) * T * mouseh;
        }
      else
        mh = inverse(drawtrans) * find_mouseh3();
        
      displayfr(vid.xres-8, vid.yres-8-fs*7, 2, vid.fsize, XLAT("x: %1", fts(mh[0],4)), 0xC0C0C0, 16);
      displayfr(vid.xres-8, vid.yres-8-fs*6, 2, vid.fsize, XLAT("y: %1", fts(mh[1],4)), 0xC0C0C0, 16);
      displayfr(vid.xres-8, vid.yres-8-fs*5, 2, vid.fsize, XLAT("z: %1", fts(mh[2],4)), 0xC0C0C0, 16);
      if(MDIM == 4)
        displayfr(vid.xres-8, vid.yres-8-fs*4, 2, vid.fsize, XLAT("w: %1", fts(mh[3],4)), 0xC0C0C0, 16);
      mh = inverse_exp(mh, iTable, false);
      displayfr(vid.xres-8, vid.yres-8-fs*3, 2, vid.fsize, XLAT("r: %1", fts(hypot_d(3, mh),4)), 0xC0C0C0, 16);
      if(GDIM == 3) {
        displayfr(vid.xres-8, vid.yres-8-fs, 2, vid.fsize, XLAT("ϕ: %1°", fts(-atan2(mh[2], hypot_d(2, mh)) / degree,4)), 0xC0C0C0, 16);
        displayfr(vid.xres-8, vid.yres-8-fs*2, 2, vid.fsize, XLAT("λ: %1°", fts(-atan2(mh[1], mh[0]) / degree,4)), 0xC0C0C0, 16);
        }
      else {
        displayfr(vid.xres-8, vid.yres-8-fs*2, 2, vid.fsize, XLAT("ϕ: %1°", fts(-atan2(mh[1], mh[0]) / degree,4)), 0xC0C0C0, 16);
        }
      }

    if(us) {
      cgi.require_usershapes();
      auto& sh = cgi.ushr[&us->d[dslayer]];
      if(sh.e >= sh.s + 3)
        displayButton(vid.xres-8, vid.yres-8-fs*8, XLAT("area: %1", area_in_pi ? fts(compute_area(sh) / M_PI, 4) + "π" : fts(compute_area(sh), 4)), 'w', 16);
      }

    
    displayFunctionKeys();
    
    keyhandler = drawHandleKey;
#else
    popScreen();
#endif
    }
  
#if CAP_POLY
  void loadShapes(int sg, int id) {
    delete usershapes[sg][id];
    usershapes[sg][id] = NULL;

    initquickqueue();
    
    dynamicval<bool> ws(mmspatial, false);
    
    if(sg == 0) {
      multi::cpid = id, drawMonsterType(moPlayer, drawcell, Id, 0xC0C0C0, 0, 0xC0C0C0);
      }
    else if(sg == 1) {
      drawMonsterType(eMonster(id), drawcell, Id, minf[id].color, 0, minf[id].color);
      }
    else if(sg == 2) {
      drawItemType(eItem(id), drawcell, Id, iinf[id].color, 0, false);
      }
    else {
      draw_qfi(drawcell, Id, 0, PPR::FLOOR);
      }

    sortquickqueue();
      
    int layer = 0;
    
    initShape(sg, id);
    
    for(int i=0; i<isize(ptds); i++) { 
      auto pp = dynamic_cast<dqi_poly*> (&*ptds[i]);
      if(!pp) continue;
      auto& ptd = *pp;
      
      int cnt = ptd.cnt;
      
      usershapelayer *dsCur = &usershapes[sg][id]->d[layer];
      dsCur->list.clear();
      dsCur->color = ptd.color;
      dsCur->sym = false;
      dsCur->rots = 1;
      dsCur->zlevel = 0;
      
      for(auto& v: cgi.symmetriesAt)
        if(v[0] == ptd.offset) {
          dsCur->rots = v[1];
          dsCur->sym = v[2] == 2;
          }
        
      int d = dsCur->rots * (dsCur->sym ? 2 : 1);
      
      for(int i=0; i < cnt/d; i++)
        dsCur->list.push_back(ptd.V * glhr::gltopoint((*ptd.tab)[i+ptd.offset]));
      
      layer++;      
      if(layer == USERLAYERS) break;
      }
    usershape_changes++;
    }

  void applyToShape(int sg, int id, int uni, hyperpoint mh) {
    bool haveshape = usershapes[sg][id];
    bool xnew = false;
    
    if(uni == '-') uni = mousekey;
    
    if(!haveshape) {
      if(uni == 'n')
        initShape(sg, id);
      else if(uni == 'u') ;
      else if(uni >= '0' && uni <= '9') {
        initShape(sg, id);
        xnew = true;
        }
      else 
        return;
      }

    usershapelayer *dsCur = &usershapes[sg][id]->d[dslayer];

    if(uni == 'n' || xnew) {
      dsCur->list.clear();
      dsCur->list.push_back(mh);
      usershape_changes++;
      }

    if(uni == 'u') 
      loadShapes(sg, id);
    
    if(uni == 'z' && haveshape && GDIM == 2)
      dialog::editNumber(dsCur->zlevel, -10, +10, 0.1, 0, XLAT("z-level"),
        XLAT("Changing the z-level will make this layer affected by the parallax effect."));

    if(EDITING_TRIANGLES) {
      if(uni == 'a') {
        dsCur->list.push_back(mh);
        uni = 0;
        usershape_changes++;
        }
      else if(uni == 'c' || uni == 'd' || uni == 'm') {
        hyperpoint best = mh;
        hyperpoint onscr;
        applymodel(drawtrans * mh, onscr);
        println(hlog, "onscr = ", onscr);
        ld dist = HUGE_VAL;
        for(auto& layer: usershapes[sg][id]->d)
        for(const hyperpoint& h: layer.list) {
          hyperpoint h1;
          applymodel(drawtrans * h, h1);
          ld d = sqhypot_d(2, h1 - onscr);
          if(d < dist) dist = d, best = h;
          }
        if(uni == 'c') dsCur->list.push_back(best);
        else if(uni == 'd') {
          vector<hyperpoint> oldlist = move(dsCur->list);
          dsCur->list.clear();
          int i;
          for(i=0; i<isize(oldlist); i+=3)
            if(oldlist[i] != best && oldlist[i+1] != best && oldlist[i+2] != best)
              dsCur->list.push_back(oldlist[i]),
              dsCur->list.push_back(oldlist[i+1]),
              dsCur->list.push_back(oldlist[i+2]);
          for(; i<isize(oldlist); i++)
            if(oldlist[i] != best)
              dsCur->list.push_back(oldlist[i]);
          }
        usershape_changes++;
        uni = 0;
        }
      else if(uni == COLORKEY) dsCur->color = colortouse;
      else if(uni != 'D') uni = 0;
      }
    
    if(uni == 'a' && haveshape) {
      mh = spin(2*M_PI*-ew.rotid/dsCur->rots) * mh;
      if(ew.symid) mh = Mirror * mh;
    
      if(ew.pointid < 0 || ew.pointid >= isize(dsCur->list)) 
        ew.pointid = isize(dsCur->list)-1, ew.side = 1;

      dsCur->list.insert(dsCur->list.begin()+ew.pointid+(ew.side?1:0), mh);
      if(ew.side) ew.pointid++;
      usershape_changes++;
      }
    
    if(uni == 'D') {
      delete usershapes[sg][id];
      usershapes[sg][id] = NULL;
      }

    if(uni == 'm' || uni == 'd') {

      int i = ew.pointid;

      if(i < 0 || i >= isize(dsCur->list)) return;

      mh = spin(2*M_PI*-ew.rotid/dsCur->rots) * mh;
      if(ew.symid) mh = Mirror * mh;

      if(uni == 'm' || uni == 'M') 
        dsCur->list[i] = mh;
      if(uni == 'd' || uni == 'b') {
        dsCur->list.erase(dsCur->list.begin() + i);
        if(ew.side == 1 && ew.pointid >= i) ew.pointid--;
        if(ew.side == 0 && ew.pointid > i) ew.pointid--;
        }
      usershape_changes++;
      }
    
    if(uni == 'K') {
      if(vid.cs.charid >= 4) {
        loadShape(sg, id, cgi.shCatBody, 2, 0);
        loadShape(sg, id, cgi.shCatHead, 2, 1);
        }
      else {
        if(!(vid.cs.charid&1)) loadShape(sg, id, cgi.shPBody, 2, 0);
        else loadShape(sg, id, cgi.shFemaleBody, 2, 0);
  
        loadShape(sg, id, cgi.shPSword, 1, 1);
  
        if(vid.cs.charid&1)
          loadShape(sg, id, cgi.shFemaleDress, 2, 2);

        /* if(vid.cs.charid&1)
          loadShape(sg, id, cgi.shPrincessDress, 1, 3);
        else
          loadShape(sg, id, cgi.shPrinceDress, 2, 3); */
        
        loadShape(sg, id, cgi.shRatCape2, 1, 3);
      
        if(vid.cs.charid&1)
          loadShape(sg, id, cgi.shFemaleHair, 2, 4);
        else
          loadShape(sg, id, cgi.shPHead, 2, 4);
        
        loadShape(sg, id, cgi.shPFace, 2, 5); 
        }
      
      // loadShape(sg, id, cgi.shWolf, 2, dslayer);
      
      usershape_changes++;
      }

    if(uni == '+') dsCur->rots++;

    if(uni >= '1' && uni <= '9') {
      dsCur->rots = uni - '0';
      if(dsCur->rots == 9) dsCur->rots = 21;
      usershape_changes++;
      }
    if(uni == '0') {
      dsCur->sym = !dsCur->sym;
      usershape_changes++;
      }

    if(uni == 't') {
      dsCur->shift = mh;
      usershape_changes++;
      }
    if(uni == 'y') {
      dsCur->spin = mh;
      usershape_changes++;
      }

    if(uni == COLORKEY) dsCur->color = colortouse;
    }

  void writeHyperpoint(hstream& f, hyperpoint h) {
    println(f, spaced_of(&h[0], MDIM));
    }
  
  hyperpoint readHyperpoint(fhstream& f) {
    hyperpoint h;
    for(int i=0; i<MDIM; i++) scan(f, h[i]);
    return h;
    }
  
  string drawHelpLine() {
    return XLAT("vector graphics editor");
    }
  
  bool onelayeronly;
  
  bool loadPicFile(const string& s) {
    fhstream f(picfile, "rt");
    if(!f.f) {
      addMessage(XLAT("Failed to load pictures from %1", picfile));
      return false;
      }
    scanline(f);
    scan(f, f.vernum);
    printf("vernum = %x\n", f.vernum);
    if(f.vernum == 0) {
      addMessage(XLAT("Failed to load pictures from %1", picfile));
      return false;
      }

    if(f.vernum >= 0xA0A0) {
      int tg, wp;
      int nt;
      scan(f, tg, nt, wp, patterns::subpattern_flags);
      patterns::whichPattern = patterns::ePattern(wp);
      set_geometry(eGeometry(tg));
      set_variation(eVariation(nt));
      start_game();
      }

    while(true) {
      int i, j, l, sym, rots, siz;
      color_t color;
      if(!scan(f, i, j, l, sym, rots, color, siz)) break;
      if(i == -1) break;
      if(siz < 0 || siz > 1000) break;
      
      if(i >= 4) {
        if(i < 8) patterns::whichPattern = patterns::ePattern("xxxxfpzH"[i]);
        patterns::subpattern_flags = 0;
        i = 3;
        }

      initShape(i, j);
      usershapelayer& ds(usershapes[i][j]->d[l]);
      if(f.vernum >= 0xA608) scan(f, ds.zlevel);
      ds.shift = readHyperpoint(f);
      ds.spin = readHyperpoint(f);
      ds.list.clear();
      for(int i=0; i<siz; i++) {
        ds.list.push_back(readHyperpoint(f));
        writeHyperpoint(hlog, ds.list[i]);
        }
      ds.sym = sym;
      ds.rots = rots;
      ds.color = color;
      }
    addMessage(XLAT("Pictures loaded from %1", picfile));
    
    usershape_changes++;
    return true;
    }
  
  bool savePicFile(const string& s) {
    fhstream f(picfile, "wt");
    if(!f.f) {
      addMessage(XLAT("Failed to save pictures to %1", picfile));
      return false;
      }
    println(f, "HyperRogue saved picture");
    println(f, f.vernum);
    if(f.vernum >= 0xA0A0)
      println(f, spaced(geometry, int(variation), patterns::whichPattern, patterns::subpattern_flags));
    for(int i=0; i<USERSHAPEGROUPS; i++) for(auto usp: usershapes[i]) {
      usershape *us = usp.second;
      if(!us) continue;
      
      for(int l=0; l<USERLAYERS; l++) if(isize(us->d[l].list)) {
        usershapelayer& ds(us->d[l]);
        println(f, spaced(i, usp.first, l, ds.sym, ds.rots, ds.color, int(isize(ds.list))));
        print(f, spaced(ds.zlevel), " ");
        writeHyperpoint(f, ds.shift);
        writeHyperpoint(f, ds.spin);
        println(f);
        for(int i=0; i<isize(ds.list); i++)
          writeHyperpoint(f, ds.list[i]);
        }
      }
    println(f, "-1");
    addMessage(XLAT("Pictures saved to %1", picfile));
    return true;
    }

  void drawHandleKey(int sym, int uni) {

    if(uni == PSEUDOKEY_WHEELUP && GDIM == 3 && front_step) {
      front_edit += front_step * shiftmul; return;
      }

    if(uni == PSEUDOKEY_WHEELDOWN && GDIM == 3 && front_step) {
      front_edit -= front_step * shiftmul; return;
      }

    handlePanning(sym, uni);
  
    if(uni == SETMOUSEKEY) {
       if(mousekey == newmousekey)
         mousekey = '-';
       else
         mousekey = newmousekey;
       }
    
    if(uni == 'w') area_in_pi = !area_in_pi;

    if(uni == 'r') {
      pushScreen(patterns::showPattern);
      if(drawplayer) 
        addMessage(XLAT("Hint: use F7 to edit floor under the player"));
      }
    
    hyperpoint mh = GDIM == 2 ? mouseh : find_mouseh3();
    mh = inverse(drawtrans) * mh;

    bool clickused = false;
    
    if((uni == 'p' && mousekey == 'g') || (uni == 'g' && coldcenter == ccenter && ccenter == mh)) {
      static unsigned grid_colors[] = {
        8,
        0x00000040,
        0xFFFFFF40,
        0xFF000040,
        0x0000F040,
        0x00000080,
        0xFFFFFF80,
        0xFF000080,
        0x0000F080,
        };
      dialog::openColorDialog(gridcolor, grid_colors);
      clickused = true;
      }
    
    char mkuni = uni == '-' ? mousekey : uni;
    
    if(mkuni == 'g') 
      coldcenter = ccenter, ccenter = mh, clickused = true;
    
    if(uni == 'd' || uni == 'l' || uni == 'c')
      mousekey = uni;

    if(uni == ' ' && (cheater || autocheat)) {
      popScreen();
      pushScreen(showMapEditor);
      }

    if(uni == 'z' && GDIM == 3) {
      dialog::editNumber(front_edit, 0, 5, 0.1, 0.5, XLAT("z-level"), "");
      dialog::extra_options = [] () {
        dialog::addBoolItem(XLAT("The distance from the camera to added points."), front_config == eFront::sphere_camera, 'A');
        dialog::add_action([] { front_config = eFront::sphere_camera; });
        dialog::addBoolItem(XLAT("place points at fixed radius"), front_config == eFront::sphere_center, 'B');
        dialog::add_action([] { front_config = eFront::sphere_center; });
        dialog::addBoolItem(XLAT(nonisotropic ? "place points on surfaces of const Z" : "place points on equidistant surfaces"), front_config == eFront::equidistants, 'C');
        dialog::add_action([] { front_config = eFront::equidistants; });
        if(nonisotropic) {
          dialog::addBoolItem(XLAT("place points on surfaces of const X"), front_config == eFront::const_x, 'D');
          dialog::add_action([] { front_config = eFront::const_x; });
          dialog::addBoolItem(XLAT("place points on surfaces of const Y"), front_config == eFront::const_y, 'E');
          dialog::add_action([] { front_config = eFront::const_y; });
          }
        dialog::addSelItem(XLAT("mousewheel step"), fts(front_step), 'S');
        dialog::add_action([] {
          popScreen();
          dialog::editNumber(front_step, -10, 10, 0.1, 0.1, XLAT("mousewheel step"), "hint: shift for finer steps");
          });
        if(front_config == eFront::sphere_center) {
          dialog::addSelItem(XLAT("parallels to draw"), its(parallels), 'P');
          dialog::add_action([] {
            popScreen();
            dialog::editNumber(parallels, 0, 72, 1, 12, XLAT("parallels to draw"), "");
            });
          dialog::addSelItem(XLAT("meridians to draw"), its(meridians), 'M');
          dialog::add_action([] {
            popScreen();
            dialog::editNumber(meridians, 0, 72, 1, 12, XLAT("meridians to draw"), "");
            });
          }
        else if(front_config != eFront::sphere_camera) {
          dialog::addSelItem(XLAT("range of grid to draw"), fts(equi_range), 'R');
          dialog::add_action([] {
            popScreen();
            dialog::editNumber(equi_range, 0, 5, 0.1, 1, XLAT("range of grid to draw"), "");
            });
          }
        };
      }
    
    if(sym == SDLK_F7) {
      drawplayer = !drawplayer;
      }

#if CAP_SHOT
    else if(sym == SDLK_F6) {
      pushScreen(shot::menu);
      }
#endif

    if(sym == SDLK_ESCAPE) popScreen();

    if(sym == SDLK_F1) {
      gotoHelp(drawhelptext());
      }

    if(sym == SDLK_F10) popScreen();

#if CAP_TEXTURE
    if(texture::config.tstate == texture::tsActive) {
    
      int tcolor = (texture::config.paint_color >> 8) | ((texture::config.paint_color & 0xFF) << 24);
      
      if(uni == '-' && !clickused) {
        if(mousekey == 'l' || mousekey == 'c') {
          if(!holdmouse) lstart = mouseh, lstartcell = mouseover, holdmouse = true;
          }
        else {
          if(!holdmouse) texture::config.data.undoLock();
          texture::drawPixel(mouseover, mouseh, tcolor);
          holdmouse = true; lstartcell = NULL;
          }
        }
      
      if(sym == PSEUDOKEY_RELEASE) {
        printf("release\n");
        if(mousekey == 'l') { 
          texture::config.data.undoLock();
          texture::where = mouseover;
          texture::drawPixel(mouseover, mouseh, tcolor);
          texture::drawLine(mouseh, lstart, tcolor);
          lstartcell = NULL;
          }
        if(mousekey == 'c') { 
          texture::config.data.undoLock();
          ld rad = hdist(lstart, mouseh);
          int circp = int(1 + 3 * (circlelength(rad) / texture::penwidth));
          if(circp > 1000) circp = 1000;
          transmatrix T = rgpushxto0(lstart);
          texture::where = lstartcell;
          for(int i=0; i<circp; i++)
            texture::drawPixel(T * xspinpush0(2 * M_PI * i / circp, rad), tcolor);
          lstartcell = NULL;
          }
        }
      
      if(uni >= 1000 && uni < 1010)
        texture::config.paint_color = texture_colors[uni - 1000 + 1];

      if(uni >= 2000 && uni < 2010)
        texture::penwidth = brush_sizes[uni - 2000];

      if(uni == '0')
        texture::texturesym = !texture::texturesym;

      if(uni == 'u') {
        texture::config.data.undo();
        }        
      
      if(uni == 'p') {
        if(!clickused)
          dialog::openColorDialog(texture::config.paint_color, texture_colors);
        }

      if(uni == 'b') 
        dialog::editNumber(texture::penwidth, 0, 0.1, 0.005, 0.02, XLAT("brush size"), XLAT("brush size"));
      }
#else
    (void)clickused;
    if(0);
#endif    

    else {
      dslayer %= USERLAYERS;

      applyToShape(drawcellShapeGroup(), drawcellShapeID(), uni, mh);

      if(uni == 'e' || (uni == '-' && mousekey == 'e')) {
        initdraw(mouseover ? mouseover : cwt.at);
        }
      if(uni == 'l') { dslayer++; dslayer %= USERLAYERS; }
      if(uni == 'L') { dslayer--; if(dslayer < 0) dslayer += USERLAYERS; }
      if(uni == 'l' - 96) onelayeronly = !onelayeronly;
    
      if(uni == 'c') ew = ewsearch;
      if(uni == 'b') autochoose = !autochoose;
      
      if(uni == 'S') {
        for(int i=0; i<USERSHAPEGROUPS; i++) for(auto usp: usershapes[i]) {
          auto us = usp.second;
          if(!us) continue;
          
          for(int l=0; l<USERLAYERS; l++) if(isize(us->d[l].list)) {
            usershapelayer& ds(us->d[l]);
            println(hlog, spaced("//", i, usp.first, l, "[", ds.color, double(ds.zlevel), "]"));
            print(hlog, " ID, ", us->d[l].rots, ", ", us->d[l].sym?2:1, ", "); 
            for(int i=0; i<isize(us->d[l].list); i++) {
              for(int d=0; d<GDIM; d++) print(hlog, fts(us->d[l].list[i][d]), ", ");
              print(hlog, " ");
              }
            println(hlog);
            }
          }
        }
  
      if(uni == 'p') {
        dialog::openColorDialog(colortouse);
        dialog::reaction = [] () {
          drawHandleKey(COLORKEY, COLORKEY);
          };
        }

      if(sym == SDLK_F2) 
        dialog::openFileDialog(picfile, XLAT("pics to save:"), ".pic", 
          [] () {
            return savePicFile(picfile);
            });
      
      if(sym == SDLK_F3) 
        dialog::openFileDialog(picfile, XLAT("pics to load:"), ".pic", 
          [] () {
            return loadPicFile(picfile);
            });

      if(sym == SDLK_F5) {
        for(int i=0; i<USERSHAPEGROUPS; i++) {
          for(auto us: usershapes[i])
            if(us.second) delete us.second;
          usershapes[i].clear();
          }
        }
      }
    }
#endif    

  auto hooks = addHook(clearmemory, 0, [] () {
    if(mapeditor::painttype == 4) 
      mapeditor::painttype = 0, mapeditor::paintwhat = 0,
      mapeditor::paintwhat_str = "clear monster";
    mapeditor::copysource.at = NULL;
    mapeditor::undo.clear();
    if(!cheater) patterns::displaycodes = false;
    if(!cheater) patterns::whichShape = 0;
    modelcell.clear();
    }) + 
  addHook(hooks_removecells, 0, [] () {
    modelcell.clear();
    set_if_removed(mapeditor::copysource.at, NULL);
    });;;  
#endif

  EX void initdraw(cell *c) {
    #if CAP_EDIT
    mapeditor::drawcell = c;
    ew.c = c;
    ew.rotid = 0;
    ew.symid = 0;
    ew.pointid = -1;
    ew.side = 0;
    ewsearch = ew;
    ccenter = coldcenter = C0;
    #endif
    }
  
  transmatrix textrans;

#if CAP_TEXTURE
  void queue_hcircle(transmatrix Ctr, ld radius) {
    vector<hyperpoint> pts;
    int circp = int(6 * pow(2, vid.linequality));
    if(radius > 0.04) circp *= 2;
    if(radius > .1) circp *= 2; 
    
    for(int j=0; j<circp; j++)
      pts.push_back(Ctr * xspinpush0(M_PI*j*2/circp, radius));
    for(int j=0; j<circp; j++) curvepoint(pts[j]);
    curvepoint(pts[0]);
    queuecurve(texture::config.paint_color, 0, PPR::LINE);
    }
#endif

#if CAP_POLY
  EX bool haveUserShape(eShapegroup group, int id) {
  #if !CAP_EDIT
    return false;
  #else
    return usershapes[group].count(id) && usershapes[group][id];
  #endif
    }
#endif
  
#if CAP_TEXTURE      
  EX void draw_texture_ghosts(cell *c, const transmatrix& V) {
    if(!c) return;
    if(holdmouse && !lstartcell) return;
    cell *ls = lstartcell ? lstartcell : lmouseover;     
    if(!ls) return;
     
    auto sio = patterns::getpatterninfo0(ls);
    auto sih = patterns::getpatterninfo0(c);
    
    if(sio.id == sih.id) {
      if(c == ls)
        textrans = inverse(V * applyPatterndir(ls, sio));
      
      transmatrix mh = textrans * rgpushxto0(mouseh);
      transmatrix ml = textrans * rgpushxto0(lstart);

      for(int j=0; j<=texture::texturesym; j++)
      for(int i=0; i<c->type; i += sih.symmetries) {
        transmatrix M2 = V * applyPatterndir(c, sih) * spin(2*M_PI*i/c->type);
        if(j) M2 = M2 * Mirror;
        switch(holdmouse ? mousekey : 'd') {
          case 'c':
            queue_hcircle(M2 * ml, hdist(lstart, mouseh));
            break;
          case 'l':
            queueline(M2 * mh * C0, M2 * ml * C0, texture::config.paint_color, 4 + vid.linequality, PPR::LINE);
            break;
          default:
            queue_hcircle(M2 * mh, texture::penwidth);
          }                
        }
      }
    }
#endif

#if CAP_POLY
  EX bool drawUserShape(const transmatrix& V, eShapegroup group, int id, color_t color, cell *c, PPR prio IS(PPR::DEFAULT)) {
  #if !CAP_EDIT
    return false;
  #else
  
    // floors handled separately
    if(c && c == drawcell && editingShape(group, id) && group != sgFloor)
      drawtrans = V;

    usershape *us = usershapes[group][id];
    if(us) {  
      cgi.require_usershapes();
      for(int i=0; i<USERLAYERS; i++) {
        if(i != dslayer && onelayeronly) continue;
        usershapelayer& ds(us->d[i]);
        hpcshape& sh(cgi.ushr[&ds]);
    
        if(sh.s != sh.e) {
          auto& last = queuepolyat(mmscale(V, GDIM == 3 ? 0 : geom3::lev_to_factor(ds.zlevel)), sh, ds.color ? ds.color : color, prio);
          if(GDIM == 3) {
            last.tinf = &user_triangles_texture;
            last.offset_texture = ds.texture_offset;
            }
          }
        }
      }
  
    if(cmode & sm::DRAW) {

      if(c == drawcell && EDITING_TRIANGLES && mapeditor::editingShape(group, id)) {
        if(!us) return false;
        usershapelayer &ds(us->d[mapeditor::dslayer]);
        for(int i=0; i<isize(ds.list); i++) {
          int j = (i%3 == 2 ? i-2 : i+1);
          if(j < isize(ds.list))
            queueline(V * ds.list[i], V * ds.list[j], 0xFF00FFFF, -1, PPR::SUPERLINE);
          queuechr(V * ds.list[i], 10, 'x', 0xFF00FF);
          }
        }

      if(mapeditor::editingShape(group, id) && !EDITING_TRIANGLES) {
  
        /* for(int a=0; a<isize(ds.list); a++) {
          hyperpoint P2 = V * ds.list[a];
    
          int xc, yc, sc;
          getcoord(P2, xc, yc, sc);
          queuechr(xc, yc, sc, 10, 'x', 
            a == 0 ? 0x00FF00 : 
            a == isize(ds.list)-1 ? 0xFF0000 :
            0xFFFF00);
          } */
        
        if(!us) return false;
     
        usershapelayer &ds(us->d[mapeditor::dslayer]);
        
        hyperpoint mh = inverse(mapeditor::drawtrans) * mouseh;
    
        for(int a=0; a<ds.rots; a++) 
        for(int b=0; b<(ds.sym?2:1); b++) {
    
          if(mouseout()) break;
    
          hyperpoint P2 = V * spin(2*M_PI*a/ds.rots) * (b?Mirror*mh:mh);
        
          queuechr(P2, 10, 'x', 0xFF00FF);
          }
        
        if(isize(ds.list) == 0) return us;
        
        hyperpoint Plast = V * spin(-2*M_PI/ds.rots) * (ds.sym?Mirror*ds.list[0]:ds.list[isize(ds.list)-1]);
        int state = 0;
        int gstate = 0;
        double dist2 = 0;
        hyperpoint lpsm;
        
        for(int a=0; a<ds.rots; a++) 
        for(int b=0; b<(ds.sym?2:1); b++) {
        
          hyperpoint mh2 = spin(2*M_PI*-ew.rotid/ds.rots) * mh;
          if(ew.symid) mh2 = Mirror * mh2;
          hyperpoint pseudomouse = V * spin(2*M_PI*a/ds.rots) * mirrorif(mh2, b);      
        
          for(int t=0; t<isize(ds.list); t++) {
            int ti = b ? isize(ds.list)-1-t : t;
  
            hyperpoint P2 = V * spin(2*M_PI*a/ds.rots) * mirrorif(ds.list[ti], b);
            
            if(!mouseout()) {
              double d = hdist(mouseh, P2);
              if(d < ewsearch.dist)
                ewsearch.dist = d,
                ewsearch.rotid = a,
                ewsearch.symid = b,
                ewsearch.pointid = ti,
                ewsearch.c = c,
                ewsearch.side = b,
                state = 1,
                dist2 = d + hdist(mouseh, Plast) - hdist(P2, Plast);
            
              else if(state == 1) {
                double dist3 = d + hdist(mouseh, Plast) - hdist(P2, Plast);
                if(dist3 < dist2) 
                  ewsearch.side = !ewsearch.side;
                state = 2;
                }
              }
            
            queuechr(P2, 10, 'o', 
              0xC000C0);
            
            if(!mouseout()) {
              if(gstate == 1) queueline(lpsm, P2, 0x90000080), gstate = 0;
              if(ti == ew.pointid) {
                queueline(pseudomouse, P2, 0xF0000080);
                if(ew.side == (b==1)) queueline(pseudomouse, Plast, 0x90000080);
                else gstate = 1, lpsm = pseudomouse;
                }
              }
    
            Plast = P2;           
            }
    
          }
                
        if(gstate == 1) queueline(lpsm, V * ds.list[0], 0x90000080), gstate = 0;
        if(state == 1) {
          hyperpoint P2 = V * ds.list[0];
          if(hdist(mouseh, P2) + hdist(mouseh, Plast) - hdist(P2, Plast) < dist2) 
            ewsearch.side = 1;
          }
        }
      
      }
  
    return us;
  #endif
    }
#endif

  EX void map_settings() {
    cmode = sm::SIDE | sm::MAYDARK;
    gamescreen(1);
  
    dialog::init(XLAT("Map settings"));
  
    dialog::addBoolItem_action_neg(XLAT("disable wandering monsters"), gen_wandering, 'w');

    if(gen_wandering) {
      dialog::addBoolItem_action_neg(XLAT("disable ghost timer"), timerghost, 'g');
      }
    else dialog::addBreak(100);

    dialog::addBoolItem_action(XLAT("simple pattern generation"), reptilecheat, 'p');
    dialog::addInfo(XLAT("(e.g. pure Reptile pattern)"));

    dialog::addBoolItem_action(XLAT("safety generation"), safety, 's');
    dialog::addInfo(XLAT("(no treasure, no dangers)"));

    dialog::addBoolItem(XLAT("god mode"), autocheat, 'G');
    dialog::add_action([] () { autocheat = true; });
    dialog::addInfo(XLAT("(unlock all, allow cheats, normal character display, cannot be turned off!)"));
    
    dialog::addItem(XLAT("change the pattern/color of new Canvas cells"), 'c');
    dialog::add_action_push(patterns::showPrePatternNoninstant);
    
    dialog::addBack();
    dialog::display();
    }
EX }

#if CAP_EDIT
EX string levelfile = "hyperrogue.lev";
EX const char *loadlevel = NULL;
EX string picfile = "hyperrogue.pic";

#if CAP_COMMANDLINE

int read_editor_args() {
  using namespace arg;
  if(argis("-lev")) { shift(); levelfile = args(); }
  else if(argis("-pic")) { shift(); picfile = args(); }
  else if(argis("-load")) { PHASE(3); shift(); mapstream::loadMap(args()); }
  #if CAP_POLY
  else if(argis("-picload")) { PHASE(3); shift(); mapeditor::loadPicFile(args()); }
  #endif
  else return 1;
  return 0;
  }

auto ah_editor = addHook(hooks_args, 0, read_editor_args);
#endif
#endif
}
