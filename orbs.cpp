// Hyperbolic Rogue -- orbs
// Copyright (C) 2011-2019 Zeno Rogue, see 'hyper.cpp' for details

/** \file orbs.cpp
 *  \brief Implementation of various Orb effects, and their properties such as default and maximum charges
 */

#include "hyper.h"
namespace hr {

EX bool markOrb(eItem it) {
  if(!items[it]) return false;
  orbused[it] = true;
  return true;
  }

EX bool markEmpathy(eItem it) {
  if(!items[itOrbEmpathy]) return false;
  if(!markOrb(it)) return false;
  markOrb(itOrbEmpathy);
  return true;
  }

EX bool markEmpathy2(eItem it) {
  if(items[itOrbEmpathy] < 2) return false;
  if(!markOrb2(it)) return false;
  markOrb2(itOrbEmpathy);
  return true;
  }

EX bool markOrb2(eItem it) {
  return markOrb(it);
  /* if(!items[it]) return false;
  orbused[it] = true;
  return items[it] > 1; */
  }

EX int fixpower(int qty) {
  if(markOrb(itOrbEnergy)) qty = (qty+1)/2;
  return qty;
  }

EX void useupOrb(eItem it, int qty) {
  items[it] -= fixpower(qty);
  if(items[it] < 0) items[it] = 0;
  }

EX void drainOrb(eItem it, int target IS(0)) {
  if(items[it] > target) useupOrb(it, items[it] - target);
  }

EX void empathyMove(cell *c, cell *cto, int dir) {
  if(!items[itOrbEmpathy]) return;

  if(items[itOrbFire]) {
    invismove = false;
    if(makeflame(c, 10, false)) markEmpathy(itOrbFire);
    }

  if(items[itOrbDigging]) {           
   if(dir != STRONGWIND && earthMove(c, dir)) 
     markEmpathy(itOrbDigging), invismove = false;
   }

  if(items[itOrbWinter] && isIcyLand(c) && c->wall == waNone) {
    invismove = false;
    c->wall = waIcewall;
    markEmpathy(itOrbWinter);
    }
  }

EX bool reduceOrbPower(eItem it, int cap) {
  if(items[it] && (lastorbused[it] || (it == itOrbShield && items[it]>3) || !markOrb(itOrbTime))) {
    items[it] -= multi::activePlayers();
    if(isHaunted(cwt.at->land)) survivalist = false;
    if(items[it] < 0) items[it] = 0;
    if(items[it] > cap && markOrb(itOrbIntensity)) cap = cap * 6 / 5;
    if(items[it] > cap && timerghost) items[it] = cap;
    if(items[it] == 0 && it == itOrbLove) 
      princess::bringBack();
    return true;
    }
  if(items[it] > cap && timerghost) items[it] = cap;
  auto_teleport_charges();
  return false;
  }

EX void reduceOrbPowerAlways(eItem it) {
  if(items[it]) {
    items[it] -= multi::activePlayers();
    if(items[it] < 0) items[it] = 0;
    }
  }

EX void reduceOrbPowers() {
  if(haveMount()) markOrb(itOrbDomination);
  for(int i=0; i<ittypes; i++) 
    lastorbused[i] = orbused[i], orbused[i] = false;
  if(items[itOrbShield]) orbused[itOrbShield] = lastorbused[itOrbShield];
  reduceOrbPower(itOrbTime, cwt.at->land == laCaribbean ? 777 : 150);
  if(invismove && !invisfish) markOrb(itOrbInvis);
  reduceOrbPower(itOrbLightning, 777);
  reduceOrbPower(itOrbSpeed, 67);
  reduceOrbPower(itOrbShield, 77);
  reduceOrbPower(itOrbShell, 150);
  reduceOrbPower(itOrbFlash, 777);
  reduceOrbPower(itOrbWinter, 77);
  reduceOrbPower(itOrbFire, 77);
  reduceOrbPower(itOrbIllusion, 111);
  reduceOrbPower(itOrbDragon, 111);
  reduceOrbPower(itOrbPsi, 111);
  reduceOrbPower(itOrbInvis, 77);
  reduceOrbPower(itOrbAether, 77);
  reduceOrbPower(itOrbDigging, 100);
  reduceOrbPower(itOrbTeleport, 200);
  reduceOrbPower(itOrbSpace, 150);
  reduceOrbPower(itOrbMagnetism, 150);
  reduceOrbPowerAlways(itOrbSafety);
  reduceOrbPower(itOrbThorns, 150);
  reduceOrbPower(itOrbWater, 150);
  reduceOrbPower(itOrbAir, 150);
  reduceOrbPower(itOrbFrog, 77);
  reduceOrbPower(itOrbDash, 77);
  reduceOrbPower(itOrbPhasing, 77);
  reduceOrbPower(itOrbDiscord, 67);
  reduceOrbPower(itOrbSummon, 333);
  reduceOrbPower(itOrbMatter, 333);
  reduceOrbPower(itOrbFish, 57 + 20 * multi::activePlayers());
  if(!items[itSavedPrincess]) items[itOrbLove] = 0;
  reduceOrbPower(itOrbLove, 777);
  reduceOrbPower(itOrbStunning, 100);
  reduceOrbPower(itOrbLuck, 333);
  reduceOrbPower(itOrbUndeath, 77);
  reduceOrbPower(itOrbFreedom, 77);
  reduceOrbPower(itOrbEmpathy, 77);
  markOrb(itOrb37); reduceOrbPower(itOrb37, 333);
  reduceOrbPower(itOrbBeauty, 77);
  reduceOrbPower(itOrbEnergy, 77);
  reduceOrbPower(itOrbDomination, 120);
  reduceOrbPower(itOrbSword, 100 + 20 * multi::activePlayers());
  reduceOrbPower(itOrbSword2, 100 + 20 * multi::activePlayers());
  reduceOrbPower(itOrbStone, 120);
  reduceOrbPower(itOrbNature, 120);
  reduceOrbPower(itOrbRecall, 77);
  reduceOrbPower(itOrbBull, 120);
  reduceOrbPower(itOrbHorns, 77);
  reduceOrbPower(itOrbLava, 80);
  reduceOrbPower(itOrbMorph, 80);
  reduceOrbPower(itOrbSlaying, 120);
  reduceOrbPower(itOrbGravity, 120);
  reduceOrbPower(itOrbChoice, 120);
  reduceOrbPower(itOrbIntensity, 120);

  reduceOrbPower(itOrbSide1, 120);
  reduceOrbPower(itOrbSide2, 120);
  reduceOrbPower(itOrbSide3, 120);
  if(cwt.at->land != laWildWest)
    reduceOrbPower(itRevolver, 6);
  whirlwind::calcdirs(cwt.at); 
  items[itStrongWind] = !items[itOrbAether] && whirlwind::qdirs == 1;
  items[itWarning] = 0;
  }

EX void flashAlchemist(cell *c) {
  if(isAlch(c)) {
    if(isAlch(cwt.at))
      c->wall = cwt.at->wall;
    else
      c->wall = eWall(c->wall ^ waFloorB ^ waFloorA);
    }
  }

EX void flashCell(cell *c, eMonster killer, flagtype flags) {
  eWall ow = c->wall;
  flashAlchemist(c);
  if((flags & AF_MSG) && c->monst && !isWorm(c) && c->monst != moShadow)
    addMessage(XLAT("%The1 is destroyed by the Flash.", c->monst));
  if(c->monst || isPlayerOn(c)) attackMonster(c, flags, killer);
  if(isIcyLand(c))
    HEAT(c) += 2;
  if(c->land == laDryForest)
    c->landparam += 2;
  if(c->wall == waCavewall)  c->wall = waCavefloor;
  if(c->wall == waDeadTroll) c->wall = waCavefloor;
  if(c->wall == waDeadTroll2) c->wall = waNone;
  if(c->wall == waPetrified) c->wall = waNone;
  if(c->wall == waDeadfloor2)  c->wall = waDeadfloor;
  if(c->wall == waGargoyleFloor)  c->wall = waChasm;
  if(c->wall == waGargoyleBridge)  placeWater(c, c);
  if(c->wall == waGargoyle)  c->wall = waNone;
  if(c->wall == waTerraWarrior)  c->wall = waNone, kills[moTerraWarrior]++;
  if(c->wall == waPlatform)  c->wall = waNone;
  if(c->wall == waStone)     c->wall = waNone, destroyTrapsAround(c);
  if(c->wall == waRubble)    c->wall = waNone;
  if(c->wall == waDeadwall)  c->wall = waDeadfloor2;
  if(c->wall == waGiantRug)  c->wall = waNone;
  if(c->wall == waMirror)    c->wall = waNone;
  if(c->wall == waCloud)     c->wall = waNone;
  if(c->wall == waExplosiveBarrel) explodeBarrel(c);
  if(c->wall == waRuinWall)  c->wall = waNone;
  if(c->wall == waDune)      c->wall = waNone;
  if(c->wall == waSaloon)    c->wall = waNone;
  if(c->wall == waSandstone) c->wall = waNone;
  if(c->wall == waAncientGrave) c->wall = waNone;
  if(c->wall == waFreshGrave) c->wall = waNone;
  if(c->wall == waColumn)    c->wall = waNone;
  if(c->wall == waGlass)     c->wall = waNone;
  if(c->wall == waBigTree || c->wall == waSmallTree)    c->wall = waNone;
  if(c->wall == waBigStatue) c->wall = waNone;
  if(c->wall == waCTree)     c->wall = waCIsland2;
  if(c->wall == waPalace)    c->wall = waRubble;
  if(c->wall == waRose)      c->wall = waNone;
  if(c->wall == waOpenGate || c->wall == waClosedGate) {
    eWall w = c->wall;
    c->wall = waNone;
    for(int i=0; i<c->type; i++) if(c->move(i) && c->move(i)->wall == w)
      flashCell(c->move(i), killer, flags);
    }
  if(c->wall == waRed1)      c->wall = waNone;
  else if(c->wall == waRed2)      c->wall = waRed1;
  else if(c->wall == waRed3)      c->wall = waRed2;
  
  if(c->wall == waBarrowWall) c->wall = waBarrowDig;
  else if(c->wall == waBarrowDig) c->wall = waNone;

  if(c->wall != ow && ow) drawParticles(c, winf[ow].color, 16);
  
  if(hasTimeout(c) && c->wparam < 77) c->wparam = 77;
  if(isActivable(c))
    activateActiv(c, false);
  }

EX void activateFlashFrom(cell *cf, eMonster who, flagtype flags) {
  drawFlash(cf);
  playSound(cf, "storm");
  for(int i=0; i<isize(dcal); i++) {
    cell *c = dcal[i];
    if(c == cf) continue;
    for(int t=0; t<c->type; t++)
    for(int u=0; u<cf->type; u++)
      if(c->move(t) == cf->move(u) && c->move(t) != NULL) {
        flashCell(c, who, flags);
        goto nexti;
        }
    nexti: ;
    }
  }

EX bool distanceBound(cell *c1, cell *c2, int d) {
  if(!c1 || !c2) return false;
  if(d == 0) return c1 == c2;
  for(int i=0; i<c2->type; i++)
    if(distanceBound(c1, c2->move(i), d-1)) return true;
  return false;
  }

EX void checkFreedom(cell *cf) {
  manual_celllister cl;
  cl.add(cf);
  for(int i=0; i<isize(cl.lst); i++) {
    cell *c = cl.lst[i];
    if(c->cpdist >= 5) return;
    for(int i=0; i<c->type; i++) {
      cell *c2 = c->move(i);
      // todo leader
      if(cl.listed(c2)) continue;
      if(!passable(c2, c, P_ISPLAYER | P_MIRROR | P_LEADER)) continue;
      if(againstRose(c, c2) && !scentResistant()) continue;
      if(c2->wall == waArrowTrap && c2->wparam == 2) continue;
      bool monsterhere = false;
      for(int j=0; j<c2->type; j++) {
        cell *c3 = c2->move(j);
        if(c3 && c3->monst && !isFriendly(c3)) 
          monsterhere = true;
        }
      if(!monsterhere) cl.add(c2);
      }
    }
  addMessage(XLAT("Your %1 activates!", itOrbFreedom));
  drainOrb(itOrbFreedom);
  for(int i=0; i<numplayers(); i++)
    drawBigFlash(playerpos(i));
  for(int i=0; i<isize(dcal); i++) {
    cell *c = dcal[i];
    if(c == cf && !shmup::on) continue;
    if(c->cpdist > 5) break;
    flashCell(c, moPlayer, AF_MAGIC);
    }
  }

EX void activateFlash() {
  int tk = tkills();

  for(int i=0; i<numplayers(); i++) 
    drawFlash(playerpos(i));

  addMessage(XLAT("You activate the Flash spell!"));
  playSound(cwt.at, "storm");
  drainOrb(itOrbFlash);
  for(int i=0; i<isize(dcal); i++) {
    cell *c = dcal[i];
    if(c->cpdist > 2) break;
    flashCell(c, moPlayer, AF_MAGIC);
    }
  achievement_count("FLASH", tkills(), tk);
  }

EX bool reflectingBarrierAt(cell *c) {
  return 
    c->wall == waBarrier || c->wall == waCamelot || 
    c->wall == waPalace || c->wall == waPlatform ||
    c->wall == waTempWall || c->wall == waWarpGate || c->wall == waBarrowDig || c->wall == waBarrowWall;
  }

EX bool reflectingBarrierAt(cellwalker& c, int d) {
  if(d >= 3) return true;
  if(d <= -3) return true;
  d = c.spin + d + MODFIXER;
  d%=c.at->type;
  if(!c.at->move(d)) return true;
  
  return reflectingBarrierAt(c.at->move(d));
  // WAS:
  // if(c.at->move(d)->wall == waBarrier) return true;
  // THEN:
  // if(c.at->move(d)->land == laBarrier || c.at->move(d)->land == laOceanWall ||
  //   c.at->move(d)->land == laHauntedWall || 
  //   c.at->move(d)->land == laElementalWall) ;
  // return false;
  }

EX void killAdjacentSharks(cell *c) {
  for(int i=0; i<c->type; i++) {
    cell *c2 = c->move(i);
    if(!c2) continue;
    if(isShark(c2->monst)) {
      c2->ligon = true;
      killMonster(c2, moLightningBolt);
      killAdjacentSharks(c2);
      }
    if(isKraken(c2->monst) && isWatery(c2)) {
      cell *c3 = kraken::head(c2);
      c3->ligon = true; 
      forCellEx(c4, c3) killMonster(c4, moLightningBolt); // kill-all
      forCellEx(c4, c3) if(isWatery(c4)) {
        c4->ligon = true;
        killAdjacentSharks(c4);
        }
      }
    }
  }

EX void castLightningBolt(cellwalker lig) {
  int bnc = 0;
  int counter = 1000;
  while(true) {
    counter--; if(counter < 0) break;
    // printf("at: %p i=%d d=%d\n", lig.c, i, lig.spin);

    killAdjacentSharks(lig.at);
    
    if(lig.peek() == NULL) break;

    lig += wstep;
    if(inmirror(lig)) lig = mirror::reflect(lig);
    
    cell *c = lig.at;

    eWall ow = c->wall;

    flashAlchemist(c);
    if(c->monst == moMetalBeast2 && !c->item) c->item = itFulgurite;
    if(c->monst) attackMonster(c, AF_MAGIC, moLightningBolt);
    if(isIcyLand(c)) HEAT(c) += 2;
    if(c->land == laDryForest) c->landparam += 2;
    bool first = !c->ligon;
    c->ligon = 1;
    
    bool brk = false, spin = false;
    
    if(c->wall == waGargoyle)  brk = true;
    if(c->wall == waExplosiveBarrel) explodeBarrel(c), brk = true;
    if(c->wall == waCavewall)  c->wall = waCavefloor, brk = true;
    if(c->wall == waDeadTroll) c->wall = waCavefloor, brk = true;
    if(c->wall == waDeadTroll2)c->wall = waNone, brk = true;
    if(c->wall == waPetrified) c->wall = waNone, brk = true;
    if(c->wall == waDeadfloor2)c->wall = waDeadfloor;
    if(c->wall == waRubble)    c->wall = waNone;
    if(c->wall == waDeadwall)  c->wall = waDeadfloor2, brk = true;
    if(c->wall == waGlass)     c->wall = waNone, spin = true;
    if(c->wall == waDune)      c->wall = waNone, brk = true;
    if(c->wall == waRuinWall)  c->wall = waNone;
    if(c->wall == waIcewall)   c->wall = waNone, brk = true;
    if(c->wall == waAncientGrave) c->wall = waNone, spin = true;
    if(c->wall == waFreshGrave) c->wall = waNone, spin = true;

    if(c->wall == waFreshGrave) c->wall = waNone, spin = true;

    if(c->wall == waBigStatue) c->wall = waNone, spin = true;
    if(c->wall == waColumn)    c->wall = waNone, spin = true;
    if(c->wall == waStone)     c->wall = waNone, brk = true, destroyTrapsAround(c);
    if(c->wall == waArrowTrap) activateArrowTrap(c);
    if(c->wall == waTerraWarrior)  c->wall = waNone, kills[moTerraWarrior]++;
    
    if(c->wall == waCanopy || c->wall == waTrunk || c->wall == waBigBush || c->wall == waSmallBush) {
      makeflame(c, 12, false); brk = true;
      }

    if(c->wall == waGrounded)  brk = true;
    if(c->wall == waFan)       spin = true;
    if(c->wall == waMetal)     c->wall = waCharged, brk = true;
    if(c->wall == waSandstone) c->wall = waNone, c->item = itFulgurite, brk = true;

    if(c->wall == waCharged && first) {
      for(int i=0; i<c->type; i++) 
        // do not do strange things in horocyclic spires
        if(c->move(i) && c->move(i)->wall != waCharged) {
          cellwalker lig2(c, i);
          castLightningBolt(lig2);
          }
      brk = true;
      }
                             
    if(c->wall == waBoat && c != cwt.at)    c->wall = waSea, spin = true;
    if(c->wall == waStrandedBoat && c !=cwt.at)    c->wall = waNone, spin = true;

    if((c->wall == waNone || c->wall == waSea) && c->land == laLivefjord)
      c->wall = eWall(c->wall ^ waSea ^ waNone);
    
    if(c->wall == waRed1)      c->wall = waNone;
    if(c->wall == waRed2)      c->wall = waRed1;
    if(c->wall == waRed3)      c->wall = waRed2, brk = true;
    
    if(isActivable(c))         activateActiv(c, false);
    if(c->wall == waBigTree || c->wall == waSmallTree || c->wall == waVinePlant ||
      c->wall == waSaloon)    {
      makeflame(c, 4, false);
      brk = true;
      }
    if(c->wall == waDock)  makeflame(c, 5, false);
    if(c->wall == waCTree) makeflame(c, 12, false);
    if(c->wall == waRose)  makeflame(c, 60, false);
    if(cellHalfvine(c) && c->wall == lig.peek()->wall) {
      destroyHalfvine(c, waPartialFire, 4);
      brk = true;
      }

    if(c->wall != ow && ow)  
      drawParticles(c, winf[ow].color, 16);

    if(c == cwt.at)             {bnc++; if(bnc > 10) break; }
    if(spin) lig += hrand(lig.at->type);
    
    if(brk) break;
    
    if(reflectingBarrierAt(c)) {
      int left = -1;
      int right = 1;
      while(!reflectingBarrierAt(lig, left)) left--;
      while(!reflectingBarrierAt(lig, right)) right++;
      lig += right + left;
      if(c->wall == waBarrowWall) c->wall = waBarrowDig;
      else if(c->wall == waBarrowDig) c->wall = waNone;
      bnc++; if(bnc > 10) break;
      }
    else 
      lig += rev;
    
    if(lig.at->wall == waCloud) {
      lig.at->wall = waNone;
      mirror::createMirages(lig, mirror::LIGHTNING);
      }
    if(lig.at->wall == waMirror) {
      lig.at->wall = waNone;
      mirror::createMirrors(lig, mirror::LIGHTNING);
      }
    }
  }

EX void castLightningBoltFrom(cell *c) {
  for(int i=0; i<c->type; i++) castLightningBolt(cellwalker(c, i));
  }

EX void activateLightning() {
  int tk = tkills();
  drawLightning();
  addMessage(XLAT("You activate the Lightning spell!"));

  for(int i=0; i<isize(dcal); i++) if(dcal[i]) dcal[i]->ligon = 0;

  drainOrb(itOrbLightning);

  for(int i=0; i<numplayers(); i++) 
    castLightningBoltFrom(playerpos(i));
    
  elec::afterOrb = true;
  elec::act();
  elec::afterOrb = false;
  
  achievement_count("LIGHTNING", tkills(), tk);
  playSound(cwt.at, "storm");
  }

// roCheck: return orb type if successful, 0 otherwise
// roMouse/roKeyboard: 
//    return orb type if successful, eItem(-1) if do nothing, 0 otherwise

EX bool haveRangedTarget() {
  if(!haveRangedOrb())
    return false;
  for(int i=0; i<isize(dcal); i++) {
    cell *c = dcal[i];
    if(targetRangedOrb(c, roCheck)) {
      return true;
      }
    }
  return false;
  }

void checkmoveO() {
  if(multi::players > 1 && multi::activePlayers() == 1)
    multi::checklastmove();
  if(multi::players == 1) checkmove();
  }

int teleportAction() {
  // normal teleport
  if(shmup::on || numplayers() == 1) return 1; 
  // multi-player, but all in -- do nothing
  else if(numplayers() == multi::activePlayers()) return 0;
  // otherwise teleport to the game
  else return 2;
  }

void teleportTo(cell *dest) {
  playSound(dest, "other-teleport");
  if(dest->monst) {
    cwt.at->monst = dest->monst;
    dest->monst = moNone;
    }
    
  if(teleportAction() == 2) {
    bool b = multiRevival(dest, NULL);
    if(b) {
      killFriendlyIvy();
      drainOrb(itOrbTeleport);
      movecost(cwt.at, dest, 3);
      playerMoveEffects(cwt.at, dest);
      afterplayermoved();
      bfs();
      }
    return;
    }
  
  addMessage(XLAT("You teleport to a new location!"));

  killFriendlyIvy();
  cell *from = cwt.at;
  movecost(from, dest, 1);
  playerMoveEffects(cwt.at, dest);
  cwt.at = dest; cwt.spin = hrand(dest->type); flipplayer = !!(hrand(2));
  drainOrb(itOrbTeleport);

  mirror::destroyAll();

  afterplayermoved();
  bfs();
  
  sword::reset();
  items[itOrbSword2] = 0;
  if(shmup::on)
    shmup::teleported();
  else
    checkmoveO();

  movecost(from, dest, 2);
  auto_teleport_charges();
  }

EX void jumpTo(cell *dest, eItem byWhat, int bonuskill IS(0), eMonster dashmon IS(moNone)) {
  if(byWhat != itStrongWind) playSound(dest, "orb-frog");
  cell *from = cwt.at;

  if(byWhat == itOrbFrog) {
    useupOrb(itOrbFrog, 5);
    addMessage(XLAT("You jump!"));
    }
  
  if(byWhat == itOrbDash) {
    useupOrb(itOrbDash, 5);
    addMessage(XLAT("You vault over %the1!", dashmon));
    }
  
  if(byWhat == itOrbPhasing) {
    useupOrb(itOrbPhasing, 5);
    addMessage(XLAT("You jump!"));
    }
  
  movecost(from, dest, 1);
  
  killFriendlyIvy();

  cell *c1 = cwt.at;
  animateMovement(cwt.at, dest, LAYER_SMALL, NOHINT);
  cwt.at = dest; 
  forCellIdEx(c2, i, dest) if(c2->cpdist < dest->cpdist) {
    cwt.spin = i;
    flipplayer = true;
    }
  countLocalTreasure();
  
  sword::reset();
  stabbingAttack(c1, dest, moPlayer, bonuskill);
  playerMoveEffects(c1, dest);

  // do not apply movecost later, when from no longer exists
  if(cwt.at->item == itOrbSafety) {
    movecost(from, dest, 2);
    from = NULL;
    }
  if(cwt.at->item != itOrbYendor && cwt.at->item != itHolyGrail)
    collectItem(cwt.at, true);

  mirror::destroyAll();

  for(int i=9; i>=0; i--)
    setdist(cwt.at, i, NULL);
  
  if(from) movecost(from, dest, 2);

  createNoise(1);

  if(shmup::on)
    shmup::teleported();
  else
    monstersTurn();
  }

void growIvyTo(cell *dest, cell *src, int direction_hint) {
  if(dest->monst) 
    attackMonster(dest, AF_NORMAL | AF_MSG, moFriendlyIvy);
  else {
    dest->monst = moFriendlyIvy;
    dest->mondir = neighborId(dest, src);
    moveEffect(dest, src, moFriendlyIvy, direction_hint);
    empathyMove(src, dest, neighborId(src, dest));
    }
  createNoise(1);
  monstersTurn();
  }

pair<int, bool> spacedrain(cell *c) {
  int d = c->cpdist;
  bool usemagnet = items[itOrbMagnetism] && d > 0;
  if(usemagnet) d--;
  return {d * d, usemagnet};
  }
 
void telekinesis(cell *dest) {
  
  auto cost = spacedrain(dest); 
  
  if(dest->land == laAlchemist && isAlchAny(dest) && isAlchAny(cwt.at))
    dest->wall = cwt.at->wall;

  if(dest->land == laPower && cwt.at->land != laPower && dest->item != itOrbFire && dest->item != itOrbLife) {
    if(itemclass(dest->item) != IC_ORB)
      items[dest->item] ++;
    else
      items[dest->item] += 2;
    addMessage(XLAT("The Orb loses its power as it leaves the Land of Power!"));
    dest->item = itNone;
    }

  if(dest->wall == waGlass) {
    drainOrb(itOrbSpace);
    addMessage(XLAT("Your power is drained by %the1!", dest->wall));
    }    

  moveItem(dest, cwt.at, true);
  collectItem(cwt.at, true);
  useupOrb(itOrbSpace, cost.first);
  if(cost.second) 
    markOrb(itOrbMagnetism);

  createNoise(3);
  checkSwitch();
  bfs();
  if(!shmup::on) checkmoveO();
  }

EX eMonster summonedAt(cell *dest) {
  if(dest->monst) return moNone;
  if(dest->wall == waVineHalfA || dest->wall == waVineHalfB || dest->wall == waVinePlant)
    return moVineSpirit;
  if(dest->wall == waCTree)
    return moParrot;
  if(dest->wall == waLake)
    return moGreaterShark;
  if(dest->wall == waAncientGrave || dest->wall == waFreshGrave)
    return moGhost;
  if(dest->wall == waClosePlate || dest->wall == waOpenPlate)
    return dest->land == laPalace ? moPalace : moBat;
  if(dest->wall == waFloorA || dest->wall == waFloorB)
    return moSlime;
  if(dest->wall == waCavefloor)
    return moTroll;
  if(dest->wall == waDeadfloor)
    return moEarthElemental;
  if(dest->wall == waDeadfloor2)
    return moMiner;
  if(dest->wall == waMineOpen || dest->wall == waMineMine || dest->wall == waMineUnknown)
    return moBomberbird;
  if(dest->wall == waTrapdoor)
    return dest->land == laPalace ? moFatGuard : moOrangeDog;
  if(dest->wall == waSea)
    return 
      isElemental(dest->land) ? moWaterElemental :
      dest->land == laLivefjord ? moViking : 
      dest->land == laKraken ? moViking : 
      dest->land == laWarpCoast ? moRatling : 
      dest->land == laDocks ? moWaterElemental :
      moPirate;
  if(isReptile(dest->wall))
    return moReptile;
  if(dest->wall == waChasm)
    return moAirElemental;
  if(isFire(dest) || dest->wall == waMagma)
    return moFireElemental;
  if(dest->wall == waCavewall || dest->wall == waDeadwall)
    return moSeep;
  if(dest->wall == waRed1 || dest->wall == waRed2 || dest->wall == waRed3)
    return moRedTroll;
  if(dest->wall == waFrozenLake)
    return moFireElemental;
  if(dest->wall == waCIsland || dest->wall == waCIsland2)
    return moWaterElemental;
  if(dest->wall == waRubble || dest->wall == waGargoyleFloor || dest->wall == waGargoyleBridge || dest->wall == waLadder)
    return moGargoyle;
  if(dest->wall == waStrandedBoat)
    return moWaterElemental;
  if(dest->wall == waBoat)
    return moAirElemental;
  if(dest->wall == waStone || dest->wall == waRuinWall)
    return moEarthElemental;
  if(dest->wall == waGiantRug)
    return moVizier;
  if(dest->wall == waNone) {
    if(dest->land == laHunting) return moAirElemental;
    if(dest->land == laBull) return moRagingBull;
    if(dest->land == laPrairie) return moAirElemental;
    if(dest->land == laZebra) return moAirElemental;
    if(dest->land == laMirror) return moAirElemental;
    if(dest->land == laMountain) return moAirElemental; // unfortunately Ivies are too large
    if(dest->land == laDungeon) return moBat;
    if(dest->land == laIce) return moFireElemental;
    if(dest->land == laDesert) return moEarthElemental;
    if(dest->land == laJungle) return moWaterElemental;
    if(dest->land == laGraveyard) return moZombie;
    if(dest->land == laRlyeh || dest->land == laTemple) return moPyroCultist;
    if(dest->land == laHell) return moWaterElemental;
    if(dest->land == laPower) return moWitchFire;
    if(dest->land == laWineyard) return moVineBeast;
    if(dest->land == laEmerald) return moMiner;
    if(dest->land == laHive) return dest->type == 7 ? moBug1 : moBug0;
    if(dest->land == laRedRock) return moRedTroll;
    if(dest->land == laOcean) return moEarthElemental;
    if(dest->land == laDryForest) return moFireFairy;
    if(dest->land == laLivefjord) return moFjordTroll;
    if(dest->land == laStorms) return moStormTroll;
    if(dest->land == laOvergrown) return moForestTroll;
    if(dest->land == laIvoryTower) return moAirElemental;
    if(dest->land == laEndorian) return moAirElemental;
    if(dest->land == laEAir) return moAirElemental;
    if(dest->land == laEWater) return moWaterElemental;
    if(dest->land == laEEarth) return moEarthElemental;
    if(dest->land == laEFire) return moFireElemental;
    if(dest->land == laMotion) return moRunDog;
    if(dest->land == laWildWest) return moOutlaw;
    if(dest->land == laClearing) return moForestTroll;
    if(dest->land == laWhirlwind) return moAirElemental;
    if(dest->land == laWarpCoast) return moRatling;
    if(dest->land == laRose) return moRoseLady;
    if(dest->land == laDragon) return moFireElemental;
    if(dest->land == laTortoise) return moTortoise;
    if(dest->land == laBurial) return moEarthElemental;
    if(dest->land == laVolcano) return moFireElemental;
    if(dest->land == laBlizzard) return moAirElemental;
    if(dest->land == laTerracotta) return moEarthElemental;
    if(dest->land == laRuins) return moEarthElemental;
    if(dest->land == laSwitch) return passive_switch;
    if(dest->land == laSnakeNest) return moEarthElemental;
    if(dest->land == laBrownian) return moAcidBird;
    if(dest->land == laVariant) return moFireElemental;
    if(dest->land == laWestWall) return moAirElemental;
    if(isHaunted(dest->land)) return moGhost;
    }
  return moNone;
  }

void summonAt(cell *dest) {
  playSound(dest, "orb-ranged");
  dest->monst = summonedAt(dest);
  dest->stuntime = 3;
  if(dest->monst == moPirate || dest->monst == moViking || (dest->monst == moRatling && dest->wall == waSea))
    dest->wall = waBoat, dest->item = itNone;
  if(dest->monst == moViking && dest->land == laKraken)
    dest->item = itOrbFish;
  if(dest->wall == waStrandedBoat)
    dest->wall = waBoat;
  else if(dest->monst == moWaterElemental)
    placeWater(dest, dest);
  if(dest->wall == waStone)
    dest->wall = waNone;
  if(dest->monst == moFireElemental && isFire(dest))
    dest->wall = waNone;
  if(dest->monst == moTortoise)
    tortoise::emap[dest] = getBits(dest), dest->hitpoints = 3;
  addMessage(XLAT("You summon %the1!", dest->monst));
  moveEffect(dest, dest, dest->monst, -1);
  if(dest->wall == waClosePlate || dest->wall == waOpenPlate)
    toggleGates(dest, dest->wall);

  if(hasHitpoints(dest->monst))
    dest->hitpoints = palaceHP();

  useupOrb(itOrbSummon, 20);
  createNoise(2);
  bfs();
  checkmoveO();
  }

bool tempWallPossibleAt(cell *dest) {
  if(dest->land == laWestWall) return false;
  if(dest->monst || (dest->item && !itemHidden(dest))) return false;
  return dest->wall == waChasm || isWatery(dest) || dest->wall == waNone ||
    dest->wall == waTempBridge;
  }

void tempWallAt(cell *dest) {
  if(dest->wall == waChasm)
    dest->wall = waTempFloor;
  else if(dest->wall == waNone)
    dest->wall = waTempWall;
  else if(dest->wall == waTempBridge) {
    dest->wall = waTempBridgeBlocked;
    return;
    }
  else if(isWatery(dest))
    dest->wall = waTempBridge;
  int len = (items[itOrbMatter]+1) / 2;
  dest->wparam = len;
  useupOrb(itOrbMatter, len);
  dest->item = itNone; // underwater items are destroyed by this
  createNoise(2);
  bfs();
  checkmoveO();
  }

void psi_attack(cell *dest) {
  playSound(dest, "other-mind");
  if(isNonliving(dest->monst))
    addMessage(XLAT("You destroy %the1 with a mental blast!", dest->monst));
  else if(isDragon(dest->monst) || isKraken(dest->monst))
    addMessage(XLAT("You damage %the1 with a mental blast!", dest->monst));
  else
    addMessage(XLAT("You kill %the1 with a mental blast!", dest->monst));
  // note: psi attack works with Petrify!
  attackMonster(dest, AF_MAGIC, moPlayer);
  useupOrb(itOrbPsi, 30);
  createNoise(2);
  bfs();
  checkmoveO();
  }

void gun_attack(cell *dest) {
  playSound(dest, "orb-ranged");
  addMessage(XLAT("You shoot %the1!", dest->monst));
  attackMonster(dest, AF_GUN, moNone);
  items[itRevolver] --;
  bfs();
  checkmoveO();
  createNoise(5);
  monstersTurn();
  }

EX void checkStunKill(cell *dest) {
  if(isBird(dest->monst)) {
    moveEffect(dest, dest, moDeadBird, NOHINT);
    doesFall(dest);
    if(isWatery(dest) || dest->wall == waChasm || isFire(dest)) {
      addMessage(XLAT("%The1 falls!", dest->monst));
      fallMonster(dest);
      return;
      }
    }
  /* if(!isPermanentFlying(dest->monst) && cellEdgeUnstable(dest)) {
    addMessage(XLAT("%The1 falls!", dest->monst));
    fallMonster(dest);
    } */
  }

void stun_attack(cell *dest) {
  playSound(dest, "orb-ranged");
  addMessage(XLAT("You stun %the1!", dest->monst));
  dest->stuntime += 5;
  checkStunKill(dest);
  useupOrb(itOrbStunning, 10);
  createNoise(3);
  bfs();
  checkmoveO();
  }

void poly_attack(cell *dest) {
  playSound(dest, "orb-ranged");
  eMonster orig = dest->monst;
  auto polymonsters = {
    moYeti, moRunDog, moHunterDog, moRanger,
    moDesertman, moMonkey, moZombie, moCultist,
    moFallingDog, moVariantWarrior, moFamiliar, moOrangeDog,
    moRedFox, moFalsePrincess, moResearcher,
    moNarciss, moJiangshi, 
    };
  int ssf = 0;
  eMonster target = *(polymonsters.begin() + hrand(isize(polymonsters)));
  for(eMonster m: polymonsters)
    if(kills[m] && m != dest->monst) {
      ssf += kills[m];
      if(hrand(ssf) < kills[m])
        target = m;
      }
  addMessage(XLAT("You polymorph %the1 into %the2!", dest->monst, target));
  dest->monst = target;
  if(!dest->stuntime) dest->stuntime = 1;

  if(orig == moPair) {
    cell *dest2 = dest->move(dest->mondir);
    if(dest2->monst == moPair) {
      dest2->monst = dest->monst;
      if(!dest2->stuntime) dest2->stuntime = 1;
      }
    }
  
  checkStunKill(dest);
  useupOrb(itOrbMorph, 3);
  createNoise(3);
  bfs();
  checkmoveO();
  }

void placeIllusion(cell *c) {
  c->monst = moIllusion;
  useupOrb(itOrbIllusion, 5);
  addMessage(XLAT("You create an Illusion!"));
  bfs();
  checkmoveO();
  }

void blowoff(cell *cf, cell *ct, int direction_hint) {
  playSound(ct, "orb-ranged");
  if(cf->monst)
  addMessage(XLAT("You blow %the1 away!", cf->monst));
  if(cf->wall == waThumperOff) activateActiv(cf, false);
  if(isPushable(cf->wall) || cf->wall == waBigStatue)
    pushThumper(cf, ct);
  else if(isBoat(cf) && !cf->monst) {
    bool was_stranded = cf->wall == waStrandedBoat;
    bool willbe_stranded = ct->wall == waNone;
    if(was_stranded) cf->wall = waBoat;
    if(willbe_stranded) ct->wall = waSea;
    moveBoat(ct, cf, direction_hint);  
    if(was_stranded) cf->wall = waNone;
    if(willbe_stranded) ct->wall = waStrandedBoat;
    }
  else
    pushMonster(ct, cf, direction_hint);
  if(cf->item == itBabyTortoise) {
    if(ct->item) ct->item = itNone;
    moveItem(cf, ct, true);
    }
  items[itOrbAir]--;
  createNoise(2);
  bfs();
  checkmoveO();
  }

void useOrbOfDragon(cell *c) {
  makeflame(c, 20, false);
  playSound(c, "fire");
  addMessage(XLAT("You throw fire!"));
  useupOrb(itOrbDragon, 5);
  createNoise(3);
  bfs();
  checkmoveO();
  }

bool monstersnearO(orbAction a, cell *c, cell *nocount, eMonster who, cell *pushto, cell *comefrom) {
  // printf("[a = %d] ", a);
  if(shmup::on) return false;
  if(a == roCheck && multi::players > 1) 
    return true;
  else if(a == roMultiCheck) return false;
  else return monstersnear(c, nocount, who, pushto, comefrom);
  }

EX bool isCheck(orbAction a) { return a == roCheck || a == roMultiCheck; }
EX bool isWeakCheck(orbAction a) { return a == roCheck || a == roMultiCheck || a == roMouse; }

EX cell *blowoff_destination(cell *c, int& di) {
  int d = 0;
  for(; d<c->type; d++) if(c->move(d) && c->move(d)->cpdist < c->cpdist) break;
  if(d<c->type) for(int e=d; e<d+c->type; e++) {
    int di = e % c->type;
    cell *c2 = c->move(di);
    if(c2 && c2->cpdist > c->cpdist && passable(c2, c, P_BLOW)) return c2;
    }
  return NULL;
  }

EX eItem targetRangedOrb(cell *c, orbAction a) {

  if(!haveRangedOrb()) {
    return itNone;
    }
  
  if(rosedist(cwt.at) == 1) {
    int r = rosemap[cwt.at];
    int r2 = rosemap[c];
    if(r2 <= r) {
      if(a == roKeyboard || a == roMouseForce ) 
        addMessage(XLAT("Those roses smell too nicely. You can only target cells closer to them!"));
      return itNone;
      }
    }

  // (-2) shmup variants
  eItem shmupEffect = shmup::targetRangedOrb(a);
  
  if(shmupEffect) return shmupEffect;
  
  // (-1) distance

  if(c == cwt.at || isNeighbor(cwt.at, c)) {
    if(!isWeakCheck(a))
      addMessage(XLAT("You cannot target that close!"));
    return itNone;
    }
  if(c->cpdist > 7) {
    if(!isWeakCheck(a))
      addMessage(XLAT("You cannot target that far away!"));
    return itNone;
    }
  
  // (0-) strong wind
  if(items[itStrongWind] && c->cpdist == 2 && cwt.at == whirlwind::jumpFromWhereTo(c, true) && !monstersnearO(a, c, NULL, moPlayer, NULL, cwt.at)) {
    if(!isCheck(a)) jumpTo(c, itStrongWind);
    return itStrongWind;
    }
  
  // (0x) control
  if(haveMount() && items[itOrbDomination] && dragon::whichturn != turncount) {
    if(!isCheck(a)) { 
      dragon::target = c;
      dragon::whichturn = turncount;
      addMessage(XLAT("Commanded %the1!", haveMount()));
      checkmoveO();
      }
    return itOrbDomination;
    }
  
  // (0) telekinesis
  if(c->item && !itemHiddenFromSight(c) && !cwt.at->item && items[itOrbSpace] >= fixpower(spacedrain(c).first) && !cantGetGrimoire(c, !isCheck(a))
    && c->item != itBarrow) {
    if(!isCheck(a)) telekinesis(c);
    return itOrbSpace;
    }
  
  // (0') air blow
  bool nowhereToBlow = false;
  if(items[itOrbAir] && (isBlowableMonster(c->monst) || isPushable(c->wall) || c->wall == waBigStatue || isBoat(c))) {
    int di = NODIR;
    cell *c2 = blowoff_destination(c, di);
    if(c2 && isBoat(c) && !isWatery(c2) && c2->wall != waNone) c2 = NULL;
    if(c2 && c->wall == waBigStatue && !canPushStatueOn(c2)) c2 = NULL;
    if(c2) {
      if(!isCheck(a)) blowoff(c, c2, di);
      return itOrbAir;
      }
    else nowhereToBlow = true;
    }
  
  // nature
  if(items[itOrbNature] && numplayers() == 1 && c->monst != moFriendlyIvy) {
    cell *sides[MAX_EDGE];
    int dirs[MAX_EDGE];
    int qsides = 0;
    forCellIdCM(cf, d, c)
      if(cf->monst == moFriendlyIvy) {

        if(c->monst) {
          if(!canAttack(cf, moFriendlyIvy, c, c->monst, 0)) continue;
          if(monstersnear(cwt.at, c, moPlayer, NULL, cwt.at)) continue;
          }
        else {
          if(!passable(c, cf, P_ISPLAYER | P_MONSTER)) continue;
          if(strictlyAgainstGravity(c, cf, false, MF_IVY)) continue;
          if(monstersnear(cwt.at, NULL, moPlayer, c, cwt.at)) continue;
          }
        dirs[qsides] = d;
        sides[qsides++] = cf;
        }
    
    if(qsides > 0) {
      int di = hrand(qsides);
      if(!isCheck(a)) growIvyTo(c, sides[di], revhint(c, dirs[di]));
      return itOrbNature;
      }
    }
    
  // (0'') jump
  int jumpstate = 0;
  cell *jumpthru = NULL;
  
  // orb of vaulting
  if(!shmup::on && items[itOrbDash] && c->cpdist == 2) {
    int i = items[itOrbAether];
    if(i) items[itOrbAether] = i-1;
    cell *c2 = NULL, *c3 = NULL;    
    for(int i=0; i<cwt.at->type; i++) {
      cell *cc = cwt.at->move(i);
      if(isNeighbor(cc, c)) c3 = c2, c2 = cc;
      }
    jumpthru = c2;
    jumpstate = 10;
    if(jumpstate == 10 && c2) jumpstate = 11;
    if(jumpstate == 11 && c2->monst) jumpstate = 12;
    if(jumpstate == 12 && !c3) jumpstate = 13;
    if(jumpstate == 13 && passable(c2, cwt.at, P_ISPLAYER | P_JUMP1 | P_MONSTER)) jumpstate = 14;
    if(jumpstate == 14 && passable(c, c2, P_ISPLAYER | P_JUMP2)) jumpstate = 15;
    if(jumpstate == 15 && canAttack(cwt.at, moPlayer, c2, c2->monst, 0)) jumpstate = 16;
    if(jumpstate == 16 && !monstersnearO(a, c, c2, moPlayer, NULL, cwt.at)) jumpstate = 17;
    items[itOrbAether] = i;
    
    if(jumpstate == 17) {
      if(!isCheck(a)) {
        int k = tkills();
        eMonster m = c2->monst;
        attackMonster(c2, AF_NORMAL | AF_MSG, moPlayer);
        k = tkills() - k;
        jumpTo(c, itOrbDash, k, m);
        }
      return itOrbDash;
      }
    }
  
  if(items[itOrbFrog] && c->cpdist == 2) {
    jumpstate = 1;
    int i = items[itOrbAether];
    if(i) items[itOrbAether] = i-1;
    for(int i=0; i<cwt.at->type; i++) {
      cell *c2 = cwt.at->move(i);
      if(isNeighbor(c2, c)) {
        jumpthru = c2;
        if(passable(c2, cwt.at, P_ISPLAYER | P_JUMP1)) {
          jumpstate = 2;
          if(passable(c, c2, P_ISPLAYER | P_JUMP2)) {
            jumpstate = 3;
            break;
            }
          }
        }
      }
    items[itOrbAether] = i;
    if(jumpstate == 3 && !monstersnearO(a, c, NULL, moPlayer, NULL, cwt.at)) {
      jumpstate = 4;
      if(!isCheck(a)) jumpTo(c, itOrbFrog);
      return itOrbFrog;
      }
    }
  
  if(items[itOrbPhasing] && c->cpdist == 2) {
    if(shmup::on) shmup::pushmonsters();
    jumpstate = 21;
    int i = items[itOrbAether];
    if(i) items[itOrbAether] = i-1;
    for(int i=0; i<cwt.at->type; i++) {
      cell *c2 = cwt.at->move(i);
      if(isNeighbor(c2, c) && !nonAdjacent(cwt.at, c2) && !nonAdjacent(c2, c)) {
        jumpthru = c2;
        if(passable(c, cwt.at, P_ISPLAYER | P_PHASE)) {
          jumpstate = 22;
          if(c2->monst || isWall(c2)) {
            jumpstate = 23;
            break;
            }
          }
        }
      }
    items[itOrbAether] = i;
    if(jumpstate == 23 && !monstersnearO(a, c, NULL, moPlayer, NULL, cwt.at)) {
      jumpstate = 24;
      if(!isCheck(a)) jumpTo(c, itOrbPhasing);
      }
    if(shmup::on) shmup::popmonsters();
    if(jumpstate == 24) return itOrbPhasing;
    }
  
  // (1) switch with an illusion
  if(items[itOrbTeleport] && c->monst == moIllusion && !cwt.at->monst && teleportAction() == 1) {
    if(!isCheck(a)) teleportTo(c);
    return itOrbTeleport;
    }

  // (2) place illusion
  if(!shmup::on && items[itOrbIllusion] && c->monst == moNone && c->item == itNone && passable(c, NULL, P_MIRROR)) {
    if(!isCheck(a)) placeIllusion(c);
    return itOrbIllusion;
    }
  
  // (3) teleport
  if(items[itOrbTeleport] && c->monst == moNone && (c->item == itNone || itemHidden(c)) && 
    passable(c, NULL, P_ISPLAYER | P_TELE) && teleportAction() && shmup::verifyTeleport()) {
    if(!isCheck(a)) teleportTo(c);
    return itOrbTeleport;
    }
    
  // (4) remove an illusion
  if(!shmup::on && items[itOrbIllusion] && c->monst == moIllusion) {
    if(!isCheck(a)) {
      addMessage(XLAT("You take the Illusion away."));
      items[itOrbIllusion] += 3; // 100% effective with the Orb of Energy!
      c->monst = moNone;
      }
    return itOrbIllusion;
    }

  // (4a) colt
  if(!shmup::on && items[itRevolver] && c->monst && canAttack(cwt.at, moPlayer, c, c->monst, AF_GUN)) {
    bool inrange = false;
    for(cell *c1: gun_targets(cwt.at)) if(c1 == c) inrange = true;
    if(inrange && !monstersnearO(a, cwt.at, c, moPlayer, NULL, cwt.at)) {
      if(!isCheck(a)) gun_attack(c);
      return itRevolver;
      }
    }
  
  // (5) psi blast (non-shmup variant)
  if(!shmup::on && items[itOrbPsi] && c->monst && (isDragon(c->monst) || !isWorm(c)) && c->monst != moShadow && c->monst != moKrakenH) {
    if(!isCheck(a)) psi_attack(c);
    return itOrbPsi;
    }
  
  // (5a) summoning
  if(items[itOrbSummon] && summonedAt(c)) {
    if(!isCheck(a)) summonAt(c);
    return itOrbSummon;
    }
  
  // (5b) matter
  if(items[itOrbMatter] && tempWallPossibleAt(c)) {
    if(!isCheck(a)) tempWallAt(c);
    return itOrbMatter;
    }

  // (5c) stun
  if(items[itOrbStunning] && c->monst && !isMultitile(c->monst) && c->stuntime < 3 && !shmup::on) {
    if(!isCheck(a)) stun_attack(c);
    return itOrbStunning;
    }
  
  // (5d) poly
  if(items[itOrbMorph] && c->monst && !isMultitile(c->monst) && !shmup::on) {
    if(!isCheck(a)) poly_attack(c);
    return itOrbMorph;
    }
  
  // (6) place fire (non-shmup variant)
  if(!shmup::on && items[itOrbDragon] && makeflame(c, 20, true)) {
    if(!isCheck(a)) useOrbOfDragon(c);
    return itOrbDragon;
    }
  
  if(isWeakCheck(a)) return itNone;
  
  if(nowhereToBlow) {
    addMessage(XLAT("Nowhere to blow %the1!", c->monst));
    }  
  else if(jumpstate == 1 && jumpthru && jumpthru->monst) {
    addMessage(XLAT("Cannot jump through %the1!", jumpthru->monst));
    }
  else if(jumpstate == 1 && jumpthru) {
    addMessage(XLAT("Cannot jump through %the1!", jumpthru->wall));
    }
  else if(jumpstate == 2 && c->monst) {
    addMessage(XLAT("Cannot jump on %the1!", c->monst));
    }
  else if(jumpstate == 2 && c->wall) {
    addMessage(XLAT("Cannot jump on %the1!", c->wall));
    }
  else if(jumpstate == 3) 
    wouldkill("%The1 would get you there!");
  else if(items[itOrbAir] && c->monst) {
    addMessage(XLAT("%The1 is immune to wind!", c->monst));
    }  
  else if(items[itOrbPsi] && c->monst) {
    addMessage(XLAT("%The1 is immune to mental blasts!", c->monst));
    }  
  else if(items[itOrbTeleport] && c->monst) {
    addMessage(XLAT("Cannot teleport on a monster!"));
    }
  else if(items[itOrbSpace] && c->item == itBarrow)
    addMessage(XLAT("%The1 is protected from this kind of magic!", c->item));
  else if(items[itOrbSpace] && saved_tortoise_on(c))
    addMessage(XLAT("No, that would be heartless!"));
  else if(c->item && items[itOrbSpace] && !itemHiddenFromSight(c)) {
    if(cwt.at->item)
      addMessage(XLAT("Cannot use %the1 here!", itOrbSpace));
    addMessage(XLAT("Not enough power for telekinesis!"));
    }
  else if(items[itOrbIllusion] && c->item)
    addMessage(XLAT("Cannot cast illusion on an item!"));
  else if(items[itOrbIllusion] && c->monst)
    addMessage(XLAT("Cannot cast illusion on a monster!"));
  else if(items[itOrbIllusion] && !passable(c, NULL, P_MIRROR))
    addMessage(XLAT("Cannot cast illusion here!"));
  else if(items[itOrbTeleport] && teleportAction() == 0) {
    addMessage(XLAT("All players are in the game!"));
    }
  else if(items[itOrbTeleport] && !passable(c, NULL, P_TELE | P_ISPLAYER)) {
    addMessage(XLAT("Cannot teleport here!"));
    }
  else if(items[itOrbMatter] && !tempWallPossibleAt(c)) {
    if(c->monst)
      addMessage(XLAT("Cannot create temporary matter on a monster!"));
    else if(c->item)
      addMessage(XLAT("Cannot create temporary matter on an item!"));
    else addMessage(XLAT("Cannot create temporary matter here!"));
    }
  else if(items[itOrbSummon] && !summonedAt(c)) {
    if(c->monst)
      addMessage(XLAT("Cannot summon on a monster!"));
    else
      addMessage(XLAT("No summoning possible here!"));
    }
  else if(items[itOrbTeleport] && c->item) {
    addMessage(XLAT("Cannot teleport on an item!"));
    }
  else if(items[itOrbDragon] && !makeflame(c, 20, true)) {
    addMessage(XLAT("Cannot throw fire there!"));
    }
  else return eItem(0);
  
  return eItem(-1);
  }

EX int orbcharges(eItem it) {
  switch(it) {
    case itRevolver: //pickup-key
      return 6 - items[itRevolver];
    case itOrbShield:
      return inv::on ? 30 : 20;
    case itOrbDiscord:
      return inv::on ? 46 : 23;
    case itOrbLove:
    case itOrbUndeath:
    case itOrbSpeed: //"pickup-speed");
    case itOrbInvis:
    case itOrbAether:
      return 30;
    case itOrbGravity:
      return 45;
    case itOrbChoice:
      return 60;
    case itOrbIntensity:
      return 50;
    case itOrbWinter: // "pickup-winter"
       return inv::on ? 45 : 30;
       break;
    case itOrbBeauty:
    case itOrbEmpathy:
    case itOrbFreedom:
      return 40;
    case itOrbFrog: 
    case itOrbDash:
    case itOrbPhasing:
      return 45;
    case itOrb37:
    case itOrbEnergy:
      return 50;
    case itOrbRecall:
    case itOrbNature:
    case itOrbStone:
    case itOrbStunning:
    case itOrbLuck:
      return 60;
    case itOrbWater:
    case itOrbMatter:
    case itOrbHorns:
    case itOrbBull:
    case itOrbShell:
    case itOrbAir:
    case itOrbSlaying:
      return 66;
    case itOrbTime:
    case itOrbSpace:
    case itOrbThorns:
    case itOrbLightning:
    case itOrbFlash:
    case itOrbIllusion:
    case itOrbPsi:
    case itOrbDigging:
    case itOrbTeleport:
    case itOrbMagnetism:
      return 77;
    case itOrbDomination:
      return 90;
    case itOrbSummon:
      return 120;     

    case itOrbSword:
      return 60 + 30 * multi::activePlayers();
    case itOrbSword2:
      return 40 + 20 * multi::activePlayers();
    case itOrbFish:
      return 20 + 10 * multi::activePlayers();
    case itOrbFire:
      return sphere ? 3 : 30;
    case itOrbDragon:
       return sphere ? 10 : 77;
    
    case itOrbMorph:
      return 60;
    
    case itOrbLava:
      return 50;
    
    case itOrbSide1:
    case itOrbSide2:
    case itOrbSide3:
      return 50;
       
    default:
      return 0;
    }
  }

EX bool isShmupLifeOrb(eItem it) {
  return 
    it == itOrbLife || it == itOrbFriend ||
    it == itOrbNature || it == itOrbEmpathy ||
    it == itOrbUndeath || it == itOrbLove ||
    it == itOrbDomination || it == itOrbGravity; 
  }

EX void makelava(cell *c, int i) {
  if(!pseudohept(c)) return;
  if(isPlayerOn(c)) return;
  if(c->wall == waFire && c->wparam < i)
    c->wparam = i, orbused[itOrbLava] = true;
  else if(makeflame(c, i, false)) orbused[itOrbLava] = true;
  }

EX void orboflava(int i) {
  if(items[itOrbLava]) 
    for(cell *c: dcal) {
      if(c->cpdist > 5) break;
      if(!c->monst || isFriendly(c) || survivesFire(c->monst)) continue;
      forCellEx(c2, c) makelava(c2, i);
      }
  }
}
