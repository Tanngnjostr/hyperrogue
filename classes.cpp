// Hyperbolic Rogue -- items, monsters, walls, lands, descriptions, etc.
// Copyright (C) 2011-2018 Zeno Rogue, see 'hyper.cpp' for details

/** \file classes.cpp
 *  \brief items, monsters, walls, lands, descriptions, etc.
 *
 *  See content.cpp for actual items, monsters, walls and lands.
 */

#include "hyper.h"
namespace hr {

// --- help ---

const char *wormdes = 
  "These huge monsters normally live below the sand, but your movements have "
  "disturbed them. They are too big to be slain with your "
  "weapons, but you can defeat them by making them unable to move. "
  "This also produces some Spice. They move two times slower than you.";

const char *cocytushelp =
   "This frozen lake is a hellish version of the Icy Land. Now, your body heat melts the floor, not the walls.";

const char *tentdes = 
  "The tentacles of Cthulhu are like sandworms, but longer. "
  "They also withdraw one cell at a time, instead of exploding instantly.";

const char *gameboardhelp = 
  "Ever wondered how some boardgame would look on the hyperbolic plane? "
  "I wondered about Go, so I have created this feature. Now you can try yourself!\n"
  "Enter = pick up an item (and score), space = clear an item\n"
  "Other keys place orbs and terrain features of various kinds\n"
  "In the periodic editor, press 0-4 to switch walls in different ways\n"
  ;

const char *ivydes = 
  "A huge plant growing in the Jungle. Each Ivy has many branches, "
  "and one branch grows per each of your moves. Branches grow in a clockwise "
  "order. The root itself is vulnerable.";

const char *slimehelp =
  "The Alchemists produce magical potions from pools of blue and red slime. You "
  "can go through these pools, but you cannot move from a blue pool to a red "
  "pool, or vice versa. Pools containing items count as colorless, and "
  "they change color to the PC's previous color when the item is picked up. "
  "Slime beasts also have to keep to their own color, "
  "but when they are killed, they explode, destroying items and changing "
  "the color of the slime and slime beasts around them.";

const char *gdemonhelp =
    "These creatures are slow, but very powerful... more powerful than you. "
    "You need some more experience in demon fighting before you will be able to defeat them. "
    "Even then, you will be able to slay this one, but more powerful demons will come...\n\n"
    "Each 10 lesser demons you kill, you become powerful enough to kill all the greater "
    "demons on the screen, effectively turning them into lesser demons.";    

const char *ldemonhelp =
    "These creatures are slow, but they often appear in large numbers.";

const char *trollhelp =
  "A big monster from the Living Caves. A dead Troll will be reunited "
  "with the rocks, causing some walls to grow around its body.";

EX const char *trollhelp2 =
  " Additionally, all items around the killed Troll will be destroyed.";

const char *trollhelpX =
  "There are several species of trolls living in the hyperbolic world. "
  "Some of them leave this wall behind them when they die.";

const char *camelothelp = 
  "The Knights of the Round Table are the greatest warriors of these lands. "
  "They are not very inventive with names though, as they call each of their "
  "castles Camelot. "
  "You are probably worthy of joining them, but they will surely give you "
  "some quest to prove yourself...\n\n"
  "Each castle contains a single treasure, the Holy Grail, in the center. "
  "The radius of the Round Table is usually 28, but after you find a Holy Grail "
  "successfully, each new castle (and each Round Table) you find will be bigger.";

const char *templehelp = 
  "The temple of Cthulhu consists of many concentric circles of columns. "
  "You will surely encounter many Cultists there, who believe that a pilgrimage "
  "to the inner circles will bring them closer to Cthulhu himself, and Grimoires "
  "which surely contain many interesting secrets.\n\n"
  "The circles in the temple of Cthulhu are actually horocycles. They are "
  "infinite, and there is an infinite number of them.";

const char *barrierhelp = 
  "Huge, impassable walls which separate various lands.";

const char *cavehelp =
  "This cave contains walls which are somehow living. After each turn, each cell "
  "counts the number of living wall and living floor cells around it, and if it is "
  "currently of a different type than the majority of cells around it, it switches. "
  "Items count as three floor cells, and dead Trolls count as five wall cells. "
  "Some foreign monsters also count as floor or wall cells.\n";

const char *vinehelp = 
  "The Vineyard is filled with vines. A very dense pattern of straight lines here...\n\n" 
  "Vine Beasts and Vine Spirits change vine cells to grass, and vice versa.";

const char *hvinehelp = 
  "A vine is growing here... but only on a half of the cell. How is that even possible?!"
  "Most monsters cannot move from this cell to the cell containing the other half. "
  "Vine spirits can move only to the adjacent cells which are also adjacent to the "
  "other half.";

const char *deadcavehelp = 
  "Somehow, this cave has not received the spark of Life yet.";

const char *foresthelp =
  "This forest is quite dry. Beware the bushfires!\n"
  "Trees catch fire on the next turn. The temperature of the grass cells "
  "rises once per turn for each fire nearby, and becomes fire itself "
  "when its temperature has risen 10 times.\n"
  "You can also chop down the trees. Big trees take two turns to chop down.";

const char *hivehelp =
  "The Hive is filled with Hyperbugs. They are huge insects which look a bit like "
  "ants, a bit like bees, and a bit like roaches. "
  "They live in circular nests, and an army of Hyperbugs will attack any intruder, "
  "including you and enemy Hyperbugs. Will you manage to get to the "
  "heart of such a nest, and get the precious Royal Jelly?";

const char *redrockhelp = 
  "This land contains high rock formations. Most of the valley is at level 0, "
  "while items are found at level 3. It is impossible to gain two or more levels, "
  "or to lose three levels, in a single move, (attacks are possible at any "
  "difference, though). Kill Red Trolls and Rock Snakes to make a cell higher.";

const char *caribbeanhelp = 
  "This dangerous sea contains nasty sharks and pirates. ";

const char *cislandhelp =
  "The islands of Caribbean are infinite and circular, and "
  "the pirates probably have hidden their treasures somewhere far from the coast.";

const char *redsnakedes = 
  "Rock snakes are similar to Sandworms, but they move each turn. However, they "
  "only move on the hexagonal cells. They are also somewhat longer than the Sandworms.";

const char *minedesc = 
  "This minefield has been created by the Bomberbirds to protect their valuable eggs. "
  "Mines explode when they are stepped on, creating fire all around them "
  "(this fire never expires in the Minefield). If you don't step on a mine, you "
  "know the number of mines in cells around you. These numbers are marked with "
  "color codes on the cells you have visited.";

const char *palacedesc =
  "So better just to concentrate on collecting treasure. "
  "Beware the traps, and the guards, who are hard to kill!";

const char *gatedesc =
  "Green pressure plates open all gates in radius 3 around them. "
  "Similarly, red pressure plates close all gates in this radius.";

const char *princessdesc =
  "A mouse squeaks at you. It seems that it wants you to go somewhere. "
  "That's interesting, what could you find here?\n\n"
  "Note: in the part of the Palace that you are exploring during this "
  "quest, the distribution of monsters, pressure plates, and items has been "
  "changed somewhat, to make it a bit more suitable for less "
  "experienced players. The number of monsters does not depend on the "
  "number of Hypersian Rugs you have collected, and there are more "
  "opening plates.\n\n"
  "However, even with the normal distribution, it appears that it should be "
  "always possible to reach your goal. If you want this challenge, "
  "select it from the special game modes menu.";
  
const char *elemdesc = 
  "The Elemental Planes are divided into four subzones: Planes of Fire, Water, Air, and Earth. "
  "You need to collect a Shard from each Plane to construct an Elemental Gem. "
  "It is dangerous to collect too many Shards of the same type without constructing a Gem.";

const char *wildwestdesc = 
  "Take a revolver, kill outlaws, collect bounties.\n\n"
  "Note: since this land is anachronistic, it is not available in normal game. "
  "It is only available in special modes.";

const char *elecdesc = 
  "Whenever after your move there is a connection between a charged and a "
  "grounded cell, there is a short circuit. All cells on any "
  "path connecting a charged and a grounded cell (without repeated cells, "
  "or two consecutive grounded/charged cells) become damaged.\n\n"
  
  "Sandstone walls and most creatures are conductive. Great Walls are "
  "isolators, but lands beyond them count as grounded.\n\n"
  
  "Fulgurite, the treasure, is created when you manage to short circuit "
  "a sandstone wall, or a Rich Metal Beast.\n\n"
  
  "Trolls leave conductive rocks when killed, and Metal Beasts can only "
  "be killed by electricity -- your attacks only stun them, or push "
  "them away if already stunned.";

const char *overdesc = 
  "The Overgrown Woods are filled with mutant ivies! These plants "
  "grow very fast. Each leaf, after being grown, can grow itself "
  "on the next turn. However, each part is only able to grow "
  "once in 16 turns. Outside of the Overgrown Woods, the Mutant Ivy "
  "may grow only on hexagonal cells.\n\n"
  "Maybe such fast growing plants could help you solve the problem "
  "of hunger in your world? Kill the Mutant Ivies to collect Mutant Saplings.";

const char *cleardesc = 
  "A clearing in the Overgrown Woods. Obviously, this gives "
  "the Mutant Ivies an infinite space to grow...\n\n"
  "Mutant Fruits rot if they are not adjacent to a Mutant Ivy.";

const char *winddesc = 
  "Someone has put air fans in these plains, causing strong winds everywhere. "
  "You think that the purpose is to harness the magical power of Air Elementals, but "
  "you are not sure.\n\n"
  "All cells except fans are grouped into three colors according to a pattern. "
  "Wind blows counterclockwise around each group of cells of a single color. "
  "Cells which are blocked by walls, or at distance at most 2 from an Air Elemental, "
  "do not count for this.\n\n"
  "It is illegal to move in a direction which is closer to incoming wind than to "
  "outcoming wind. However, you can move two cells with the wind in a single turn, "
  "and so can the birds.";

const char *warningdesc = 
  "Warnings are issued when you try to do something that appears dangerous, "
  "like stepping on a known mine, or getting your boat destroyed by "
  "a Kraken without having Orb of the Fish. In some cases the action "
  "might actually be safe -- so you can ignore the warning and do it anyway, "
  "simply by repeating the action.";

const char *hauntdesc = 
  "A dark forest filled with ghosts and graves. But there is also treasure hidden "
  "deep within... But don't let greed make you stray from your path, as "
  "you can get lost!\n\n"
  "The Haunted Woods are bounded by a single equidistant curve. It is not a circle or horocycle.\n\n";

const char *bulldashdesc = 
  "Butterflies don't pursue you -- unless you get next to them, they just spin around the obstacles. "
  "They cannot be killed conventionally, but you get treasure when a Raging Bull crashes into a Butterfly. ";

const char *prairiedesc = 
  "You can find safety in some places in the Prairie, but if you want treasures, "
  "they can be found only on the other side of a giant herd of bulls.";
  
const char *cadesc = 
  "A land for people wanting to experiment with cellular automata in the HyperRogue grid. "
  "Rules can be given on the command line; the default rules are:\n"
  "-c07 00100000 -c06 0010000 -c17 00011000 -c16 0001100 -caprob 0.3\n"
  "(-c0 or -c1 can be given if the same rule is to be used for hexagonal "
  "and heptagonal cells).";

const char *huntingdesc = 
  "The Happy Hunting Ground is the place where hunting is easy; "
  "the spirits of hunters go here after their death, if their life was deemed worthy. "
  "You did not qualify though, so you will not ever be able to find your way to the places "
  "where the hunter spirits and game are... "
  "and their hunting dogs will hunt you instead!\n\n"
  "You hope to use this to learn some "
  "skills in fighting in open hyperbolic space, though.";

const char *terradesc = 
  "Spending 3-5 turns next to the Terracotta Warriors will cause them to awaken. "
  "They have 7 HP, and get stunned for longer and longer time with each attack.";

const char *terraldesc = 
  "The Emperor's mausoleum. The army "
  "of Terracotta Warriors is supposed to protect him in his "
  "afterlife, while the traps are to protect the treasures "
  "in the mausoleum from thieves.";

const char *arrowtrapdesc =
  "The arrow traps will automatically launch a deadly arrow when stepped on. They are quite old though, so the arrow will "
  "actually fly at the end of the next turn.";

const char *lavadesc =
  "The volcanic activity in this land is extremely unstable. "
  "Lava is too hot to walk on. It cools quickly, but another "
  "flow will come soon...";

const char *blizzarddesc = 
  "Once in the past there lived a tribe whose prophets warned about the great blizzard that was to come and freeze the world. "
  "They thought it was the wrath of the gods, so they put some altars, so that the gods could hear their pleas. "
  "The mighty golems guarded those altars, making sure nobody steals the jewels, because a robbed god is a angry god. "
  "Unfortunately winter has come (maybe because only the monks knew how to stop the golems, and they were said to had escaped earlier with some of the jewels). "
  "The tribe has frozen to death, altars got covered with snow and ice. Only ice golems still guard them waiting for the spring.";  

const char *crystaldesc = 
  "Yes, this is definitely a crystal. A very regular crystalline structure.\n\n"
  "This land has been designed as a tool to play with various geometries, and it does not appear during a normal gameplay.";

const char *NODESC = "No description yet.";
const char *NODESCYET = "No description yet.";
const char *GENDERSWITCH = NODESC;

// --- monsters ---

EX const char *rosedesc =
  "Each eight turns, each rosebush at distance at most 5 from you will "
  "release a wave of alluring scent. Creatures on the frontwave "
  "will move towards where the scent came from. Even if it causes them "
  "to attack their friends or beautiful creatures, or move into water, fire, chasm, or thorns of the rosebush. "
  "Ivies, Ghosts, Rock Snakes, Rose Ladies and Lords, and monsters restricted to a specific "
  "terrain are immune to scents.";

EX const char *warpdesc =
  "This part of the world is warped, restricting the movement somewhat. "
  "\"Diagonal\" movement and attacking between triangular cells is not allowed. "
  "Flash, Storms, and Freedom spells ignore this, and Ghosts can move, attack, and "
  "be attacked diagonally.";

const char *warplanddesc =
  "This land is warped. Ironically, the coast is completely straight...";

const char *roselanddesc =
  "This land is filled with beautiful, but dangerous, creatures and plants.";

const char *dragondesc = 
  "Dragons are powerful monsters. They are slow, but evil, "
  "and love to pick on creatures who are even slower than "
  "them. They must be stopped!\n\n"

  "A Dragon moves each two turns. It may attack with all its segments, "
  "or move its whole body forwards or "
  "backwards, it may also move a frontal part backwards. To kill a Dragon, "
  "you need to hit each of its segments. "
  "The head will regenerate on the "
  "turns the Dragon is not moving, so you will usually have to hit it with "
  "your last attack; otherwise, if the head is healthy, it may breathe "
  "fire (at range 3), losing the hitpoint. Killing the Dragon "
  "while still in the Dragon Chasms gives you treasure.";

const char *tortoisedesc = 
  "Galápagos is the land of Tortoises. "
  "They are very slow, which allows the Dragons to pick on them by "
  "stealing and eating their young. Bring the Baby Tortoises back, "
  "but, there is a catch: the Tortoises come in many varieties, depending "
  "on the part of Galápagos they live in -- there are 21 binary environmental "
  "factors, and thus "
  "2097152 varieties. You'll have to find a "
  "Tortoise which matches the baby exactly!\n\n"
  "Tortoises move each 3 turns, and attacks only stun them.\n\n"
  "Bringing back a Baby Tortoise counts as 5 $$$. The more factors agree in "
  "the given location of Galápagos, the brighter it is shown on your screen."; 

const char *krakendesc =
  "There are Krakens in your homeland, too... huge sea monsters which "
  "could easily destroy ships. The geometry of this strange world "
  "prevents quick movement of huge objects, "
  "so there are no large ships, only small boats, and "
  "hyperbolic Krakens are relatively small too. Still, you suppose they might be "
  "the widest creatures which could still move at considerable speed...\n\n"
  
  "Kraken heads can move only on hexagons. You need to attack all the tentacles to "
  "kill the Kraken. A tentacle cannot attack if it has been attacked on the "
  "same turn. When a Kraken attacks you while you are in a boat, it "
  "destroys the boat, but does not kill you.";

const char *halloweendesc = 
  "Halloween is a special land, that is available only in the spherical "
  "or elliptic geometry (press 'o' to switch). You play on the surface of "
  "a jack-o'-lantern, "
  "and have to collect as many Treats as possible. Each Treat you collect "
  "brings new monsters to fight, and new magical powers for you. You "
  "have to fight the monsters while effectively managing your limited "
  "resources.";

const char *reptiledesc =
  "These reptiles are quite strange creatures. They "
  "spend most of their lives sleeping as floors "
  "that other creatures can walk on. "
  "Sometimes they wake up to hunt their prey, "
  "but they will happily go back to sleep if they "
  "happen to move into a hole on their way. "
  "Your attacks do not kill the Reptiles, but "
  "you can push and stun them.";

const char *naturedesc = 
  "This Orb allows you to grow like an Ivy. "
  "The Ivy is always rooted in your current location; "
  "moving among the Ivy cells will move the root. "
  "Moving to a new location will cause the Ivy to grow "
  ", if an Ivy could make that movement "
  "(otherwise it breaks). "
  "You can also target one of the cells adjacent to your ivy "
  "(not to you) to grow or attack there.";

const char *mirroreddesc =
  "A perfect mirror wall. It is unbreakable "
  "and impassable "
  "even for aethereal beings, and everything "
  "you see inside is just an image of "
  "the real world; you can swing your sword "
  "at them, but that will not destroy them "
  "in the real world. "
  "Mirror walls reflect Mimics, lightning bolts, and "
  "missiles perfectly.";

const char* tamebomberdesc = 
  "This bomberbird will follow you at some distance, and attack your enemies. "
  "You can kill it to place a mine.";

const char *gargdesc = 
    "A being made of stone, who likes high buildings. It becomes normal stone when "
    "killed, but only if next to something stable -- otherwise it falls.";
  
const char *lakeDesc = "Hell has these lakes everywhere... They are shaped like evil stars, and filled with burning sulphur.";

const char *thumpdesc = "A device that attracts sandworms and other enemies. You need to activate it.";

const char *twdesc = "This structure will disappear after some time.";

const char *jellydesc = 
  "Some of the Slime Beasts have decided to revolt against the color rules in the "
  "Alchemist Lab. They have changed their shape and consistency, declared independence, and established their own Kingdom.\n\n"
  "Jellies switch between being a wall and being a monster after every treasure you pick.";

const char *ruindesc = 
  "Once a beautiful city... but now overrun by the mighty Raiders of unknown origin.\n\n"
  "Raiders cannot be harmed with mundane weapons, but each color has its movement restrictions.";

const char *rock_description = 
  "Shoot the Space Rocks for score. Large Rocks will split into two smaller rocks.";
  
monstertype minf[motypes] = {
  #define MONSTER(a,b,c,d,e,f,g,h) {a,b,c,e,g,h},
  #include "content.cpp"
  };

EX genderswitch_t genderswitch[NUM_GS] = {
  { GEN_F, moFalsePrincess, "False Princess", 
     "Don't be fooled by this red-haired girl, or you will be stabbed if you come too close!"},
  { GEN_M, moFalsePrincess, "False Prince",   
    "Don't be fooled by this red-haired boy, or you will be stabbed if you come too close!"},
  { GEN_F, moRoseLady,      "Rose Lady",
    "This false princess is immune to the alluring scent of roses."},
  { GEN_M, moRoseLady,      "Rose Lord",
    "This false prince is immune to the alluring scent of roses."},
  { GEN_F, moRoseBeauty,    "Rose Beauty",
    "She has flowers in her long fair hair. You could not bring yourself to attack such a beautiful woman."},
  { GEN_M, moRoseBeauty,    "Handsome Gardener",
    "Tall, strong, and holding a flower in his hand. You could "
    "not bring yourself to attack such a handsome man."}
  };

// --- items ---

itemtype iinf[ittypes] = {
  #define ITEM(a,b,c,d,e,f,g,h,i) {a,b,c,e,f,h,i},
  #include "content.cpp"
  };

// --- wall types ---

walltype winf[walltypes] = {
  #define WALL(a,b,c,d,e,f,g,h,i) {a,b,c,e,g,h,i},
  #include "content.cpp"
  };

const landtype linf[landtypes] = {
  #define LAND(a,b,c,d,e,f,g) {a,b,d,e,g},
  #include "content.cpp"
  };

EX vector<landtacinfo> land_tac = {
  {laIce, 10, 1}, {laDesert, 10, 1}, 
  {laHunting, 5, 2},
  {laMotion, 10, 1}, {laCaves, 10, 1}, {laAlchemist, 10, 1},
  {laJungle, 10, 1}, {laMirror, 10, 1}, {laZebra, 10, 1}, {laPalace, 10, 1}, 
  {laOcean, 10, 1}, {laLivefjord, 10, 1}, {laWarpCoast, 10, 1}, {laRlyeh, 10, 1}, {laHell, 10, 1}, 
  {laDryForest, 10, 1}, {laWineyard, 10, 1}, {laSwitch, 10, 1}, {laReptile, 10, 1},
  {laDeadCaves, 10, 1}, {laGraveyard, 10, 1}, 
  {laHaunted, 10, 1}, 
  {laIvoryTower, 10, 1}, {laEndorian, 10, 1}, {laMountain, 5, 2}, {laDungeon, 5, 2},
  {laRuins, 10, 1}, {laEmerald, 10, 1}, 
  {laCocytus, 10, 1}, 
  
  {laCaribbean, 5, 2}, {laWhirlpool, 5, 2}, {laKraken, 5, 2}, 
  {laTemple, 5, 2}, {laMinefield, 5, 2},
  {laPower, 5, 2}, {laHive, 5, 2}, {laRedRock, 5, 2}, {laStorms, 5, 2}, {laOvergrown, 5, 2},
  {laClearing, 5, 2},
  {laWhirlwind, 5, 2}, 
  {laBlizzard, 10, 1},
  {laRose, 5, 2}, {laVolcano, 10, 1}, {laDragon, 2, 5}, {laTortoise, 1, 10},
  {laBurial, 5, 2}, 
  {laElementalWall, 10, 1}, {laTrollheim, 5, 2},
  {laPrairie, 5, 2}, {laBull, 5, 2}, {laTerracotta, 10, 1}, 
  
  {laCrossroads, 10, 1}, {laCrossroads2, 10, 1}, {laCrossroads3, 10, 1}, {laCrossroads4, 10, 1}, 
  
  {laCamelot, 1, 100},
  {laWildWest, 10, 1},
  {laDual, 10, 1},
  {laSnakeNest, 10, 1},
  {laDocks, 10, 1},
  
  {laBrownian, 10, 1},
  {laVariant, 10, 1},
  {laWestWall, 10, 1},
  };

EX vector<eLand> randlands = {
  laIce, laDesert, laCaves, laAlchemist, laGraveyard, laPower, laLivefjord, laZebra,
  laRlyeh, laDryForest, laEmerald, laWineyard, laDeadCaves, laRedRock,
  laOvergrown, laWildWest, laWarpCoast, laRuins, laBull, laDragon, laReptile, laDocks
  };

static const flagtype qsNONOR           = qANYQ | qSMALL | qBOUNDED | qNONORIENTABLE;
static const flagtype qsNONORE          = qsNONOR | qELLIPTIC;
static const flagtype qsBQ              = qANYQ | qSMALL | qBOUNDED;
static const flagtype qsSMALL           = qANYQ | qSMALL | qBOUNDED;
static const flagtype qsSMALLN          = qANYQ | qSMALL | qBOUNDED | qNONORIENTABLE;
static const flagtype qsZEBRA           = qANYQ | qSMALL | qBOUNDED | qZEBRA;
static const flagtype qsFIELD           = qANYQ | qFIELD | qBOUNDED;
static const flagtype qsDOCKS           = qANYQ | qSMALL | qBOUNDED | qDOCKS;
static const flagtype qsSMALLB          = qSMALL | qBOUNDED;
static const flagtype qsSMALLBF         = qsSMALLB | qsFIELD | qANYQ;
static const flagtype qsSMALLBE         = qsSMALLB | qELLIPTIC | qANYQ;
static const flagtype qsBP              = qBINARY | qPENROSE;

EX geometryinfo1 giEuclid2 = { gcEuclid,     2, 2, 3, {1,1, 0,0 } };
EX geometryinfo1 giHyperb2 = { gcHyperbolic, 2, 2, 3, {1,1,-1,0 } };
EX geometryinfo1 giSphere2 = { gcSphere,     2, 2, 3, {1,1,+1,0 } };

EX geometryinfo1 giEuclid3 = { gcEuclid,     3, 3, 4, {1,1, 1,0 } };
EX geometryinfo1 giHyperb3 = { gcHyperbolic, 3, 3, 4, {1,1, 1,-1} };
EX geometryinfo1 giSphere3 = { gcSphere,     3, 3, 4, {1,1, 1,+1} };

EX geometryinfo1 giSolNIH  = { gcSolNIH,     3, 3, 4, {1,1, 1,0 } };
EX geometryinfo1 giNil     = { gcNil,        3, 3, 4, {1,1, 1,0 } };
EX geometryinfo1 giProduct = { gcSL2,        3, 3, 4, {1,1, 1,0 } /* will be filled in product::configure() */ };
EX geometryinfo1 giSL2     = { gcSL2,        3, 3, 4, {1,1,-1,-1} };

/** list of available geometries */
vector<geometryinfo> ginf = {
  {"{7,3}", "none",     "{7,3} (standard HyperRogue map)",            "HR",       7, 3, 0,         giHyperb2,       0, {{7, 5}}, eVariation::bitruncated},
  {"{6,3}", "none",     "{6,3} (euclidean Hex grid)",                 "euclid",   6, 3, 0,         giEuclid2,       0, {{7, FORBIDDEN}}, eVariation::bitruncated},
  {"{5,3}", "none",     "{5,3} (dodecahedron)",                       "sphere",   5, 3, qsSMALLB,  giSphere2,       0, {{SEE_ALL, SEE_ALL}}, eVariation::bitruncated},
  {"{5,3}", "elliptic", "elliptic geometry in {5,3}",                 "elliptic", 5, 3, qsNONORE,  giSphere2,       0, {{SEE_ALL, SEE_ALL}}, eVariation::bitruncated},
  {"{7,3}", "Zebra",    "Zebra quotient",                             "Zebra",    7, 3, qsZEBRA,   giHyperb2, 0x00400, {{7, 5}}, eVariation::bitruncated},
  {"{7,3}", "field",    "field quotient",                             "field",    7, 3, qsFIELD,   giHyperb2, 0x00200, {{7, 5}}, eVariation::bitruncated},
  {"{6,3}", "torus",    "torus/Klein bottle/...",                     "torus",    6, 3, qsBQ,      giEuclid2, 0x00600, {{7, 7}}, eVariation::bitruncated},
  {"{8,3}", "none",     "{8,3} (like standard but with octagons)",    "oct",      8, 3, 0,         giHyperb2, 0x08000, {{6, 4}}, eVariation::bitruncated},
  {"{5,4}", "none",     "{5,4} (four pentagons)",                     "4x5",      5, 4, 0,         giHyperb2, 0x08200, {{6, 4}}, eVariation::bitruncated},
  {"{6,4}", "none",     "{6,4} (four hexagons)",                      "4x6",      6, 4, 0,         giHyperb2, 0x08400, {{5, 3}}, eVariation::bitruncated},
  {"{7,4}", "none",     "{7,4} (four heptagons)",                     "4x7",      7, 4, 0,         giHyperb2, 0x08600, {{4, 3}}, eVariation::bitruncated},
  {"{4,3}", "none",     "{4,3} (cube)",                               "3x4",      4, 3, qsSMALLB,  giSphere2, 0x10000, {{SEE_ALL, SEE_ALL}}, eVariation::bitruncated},
  {"{3,3}", "none",     "{3,3} (tetrahedron)",                        "3x3",      3, 3, qsSMALLB,  giSphere2, 0x10200, {{SEE_ALL, SEE_ALL}}, eVariation::bitruncated},
  {"{4,4}", "none",     "{4,4} (Euclidean square grid)",              "4x4",      4, 4, 0,         giEuclid2, 0x10400, {{7, 7}}, eVariation::bitruncated},
  {"{4,3}", "elliptic", "elliptic geometry in {4,3}",                 "e3x4",     4, 3, qsNONORE,  giSphere2, 0x10600, {{SEE_ALL, SEE_ALL}}, eVariation::bitruncated},
  {"{7,3}", "Klein",    "Klein Quartic",                              "Klein",    7, 3, qsSMALL,   giHyperb2, 0x18000, {{7, 5}}, eVariation::bitruncated},
  {"{8,3}", "Bolza",    "Bolza Surface",                              "Bolza",    8, 3, qsDOCKS,   giHyperb2, 0x18200, {{6, 4}}, eVariation::bitruncated},
  {"{8,3}", "Bolza2",   "Bolza Surface x2",                           "Bolza2",   8, 3, qsDOCKS,   giHyperb2, 0x18400, {{6, 4}}, eVariation::bitruncated},
  {"{7,3}", "minimal",  "minimal quotient",                           "minimal",  7, 3, qsSMALLN,  giHyperb2, 0x18600, {{7, 5}}, eVariation::bitruncated},
  {"binary","none",     "variant of the binary tiling",               "binary",   7, 3, qBINARY,   giHyperb2, 0x48400, {{7, 5}}, eVariation::pure},
  {"Arch",  "none",     "Archimedean",                                "A",        7, 3, qARCHI,    giHyperb2,       0, {{7, 5}}, eVariation::pure},
  {"{7,3}", "Macbeath", "Macbeath Surface",                           "Macbeath", 7, 3, qsSMALL,   giHyperb2, 0x20000, {{7, 5}}, eVariation::bitruncated},
  {"{5,4}", "Bring",    "Bring's Surface",                            "Bring",    5, 4, qsSMALL,   giHyperb2, 0x20200, {{6, 4}}, eVariation::bitruncated},
  {"{12,3}","M3",       "Schmutz's M(3)",                             "M3",      12, 3, qsSMALL,   giHyperb2, 0x20400, {{4, 2}}, eVariation::bitruncated},
  {"{12,3}","M4",       "Schmutz's M(4)",                             "M4",      12, 3, qsSMALL,   giHyperb2, 0x20600, {{4, 2}}, eVariation::bitruncated},
  {"{6,4}", "Crystal",  "dimensional crystal",                        "Crystal",  6, 4, qANYQ|qCRYSTAL, giHyperb2, 0x28000, {{5, 3}}, eVariation::pure},
  {"{3,4}", "none",     "{3,4} (octahedron)",                         "4x3",      3, 4, qsSMALLB,  giSphere2, 0x28200, {{SEE_ALL, SEE_ALL}}, eVariation::bitruncated},
  {"bin{4,4}",  "none", "{4,4} on horospheres",                       "bin44",    9, 3, qBINARY,   giHyperb3, 0x30000, {{7, 3}}, eVariation::pure},
  {"{4,3,4}","none",    "{4,3,4} cube tiling",                        "434",      6, 4, 0,         giEuclid3, 0x30200, {{7, 5}}, eVariation::pure},
  {"{5,3,3}","none",    "{5,3,3} 120-cell",                           "533",     12, 3, qsSMALLB,  giSphere3, 0x30400, {{SEE_ALL, SEE_ALL}}, eVariation::pure},
  {"{5,3,3}", "elliptic","{5,3,3} 120-cell (elliptic space)",         "e533",    12, 3, qsSMALLBE, giSphere3, 0x30600, {{SEE_ALL, SEE_ALL}}, eVariation::pure},
  {"rh{4,3,4}","none",  "rhombic dodecahedral honeycomb",             "rh434",   12, 3, 0,         giEuclid3, 0x31000, {{7, 5}}, eVariation::pure},
  {"2t{4,3,4}","none",  "bitruncated cubic honeycomb",                "2t434",   14, 3, 0,         giEuclid3, 0x31200, {{7, 5}}, eVariation::pure},
  {"{5,3,4}","none",    "{5,3,4} hyperbolic honeycomb",               "534",     12, 4, 0,         giHyperb3, 0x31400, {{7, 2}}, eVariation::pure},
  {"{4,3,5}","none",    "{4,3,5} hyperbolic honeycomb",               "435",      6, 5, 0,         giHyperb3, 0x31600, {{7, 2}}, eVariation::pure},
  {"{3,3,3}","none",    "{3,3,3} 5-cell",                             "333",      4, 3, qsSMALLB,  giSphere3, 0x38000, {{SEE_ALL, SEE_ALL}}, eVariation::pure},
  {"{3,3,4}","none",    "{3,3,4} 16-cell",                            "334",      4, 4, qsSMALLB,  giSphere3, 0x38200, {{SEE_ALL, SEE_ALL}}, eVariation::pure},
  {"{3,3,4}","elliptic","{3,3,4} 16-cell (elliptic)",                 "e334",     4, 4, qsSMALLBE, giSphere3, 0x39200, {{SEE_ALL, SEE_ALL}}, eVariation::pure},
  {"{4,3,3}","none",    "{4,3,3} 8-cell",                             "433",      6, 4, qsSMALLB,  giSphere3, 0x38400, {{SEE_ALL, SEE_ALL}}, eVariation::pure},
  {"{4,3,3}","elliptic","{4,3,3} 8-cell (elliptic)",                  "e433",     6, 4, qsSMALLBE, giSphere3, 0x39400, {{SEE_ALL, SEE_ALL}}, eVariation::pure},
  {"{3,4,3}","none",    "{3,4,3} 24-cell",                            "343",      8, 3, qsSMALLB,  giSphere3, 0x38600, {{SEE_ALL, SEE_ALL}}, eVariation::pure},
  {"{3,4,3}","elliptic","{3,4,3} 24-cell (elliptic)",                 "e343",     8, 3, qsSMALLBE, giSphere3, 0x39600, {{SEE_ALL, SEE_ALL}}, eVariation::pure},
  {"{3,3,5}","none",    "{3,3,5} 600-cell",                           "335",      4, 3, qsSMALLB,  giSphere3, 0x41000, {{SEE_ALL, SEE_ALL}}, eVariation::pure},
  {"{3,3,5}","elliptic","{3,3,5} 600-cell (elliptic)",                "e335",     4, 3, qsSMALLBE, giSphere3, 0x41200, {{SEE_ALL, SEE_ALL}}, eVariation::pure},
  {"bin{3,6}", "none",  "{3,6} on horospheres",                       "bin36",    8, 3, qBINARY,   giHyperb3, 0x40000, {{7, 3}}, eVariation::pure},
  {"bin-rect", "none",  "rectangles on horospheres",                  "bin44/2",  7, 3, qBINARY,   giHyperb3, 0x40200, {{7, 3}}, eVariation::pure},
  {"bin{6,3}", "none",  "{6,3} on horospheres",                       "bin63",   14, 3, qBINARY,   giHyperb3, 0x40400, {{7, 3}}, eVariation::pure},
  {"{4,3,5}","field",   "{4,3,5} field quotient space",               "f435",     6, 5, qsSMALLBF, giHyperb3, 0x40600, {{SEE_ALL, SEE_ALL}}, eVariation::pure},
  {"{5,3,4}","field",   "{5,3,4} field quotient space",               "f435",    12, 4, qsSMALLBF, giHyperb3, 0x40800, {{SEE_ALL, SEE_ALL}}, eVariation::pure},
  {"binary4","none",    "standard binary tiling",                     "binary4",  5, 3, qBINARY,   giHyperb2, 0x41400, {{7, 5}}, eVariation::pure},
  {"sol",    "none",    "Solv geometry",                              "sol",      8, 3, qBINARY|qSOL,   giSolNIH,  0x41600, {{7, 5}}, eVariation::pure},
  {"kd2",    "none",    "kite-and-dart",                              "kd2",      4, 3, qPENROSE,  giEuclid2, 0x48000, {{7, 7}}, eVariation::pure},
  {"kd3",    "none",    "kite-and-dart on horospheres",               "kd3",     12, 3, qsBP,      giHyperb3, 0x48200, {{7, 3}}, eVariation::pure},
  {"nil",    "none",    "Nil geometry",                               "nil",      6, 3, 0,         giNil,     0x48600, {{7, 5}}, eVariation::pure},
  {"product","none",    "product space",                              "product",  7, 3, qHYBRID,   giProduct, 0x00000, {{7, 3}}, eVariation::pure},
  {"twisted","none",    "rotation space",                             "twisted",  7, 3, qHYBRID,   giSL2,     0x00000, {{6, 4}}, eVariation::pure},
  {"ternary","none",    "standard ternary tiling",                    "ternary",  6, 3, qBINARY,   giHyperb2, 0x48400, {{6, 4}}, eVariation::pure},
  {"3x2",    "none",    "stretched hyperbolic",                       "3:2",     11, 3, qBINARY|qNIH,   giSolNIH,  0x49000, {{6, 3}}, eVariation::pure},
  {"3x1/2",  "none",    "stretched Solv",                             "3:1/2",    9, 3, (qBINARY|qSOL|qNIH),   giSolNIH,  0x49200, {{7, 3}}, eVariation::pure},
  {"{3,oo}", "none",    "{3,∞} (infinite triangles)",                 "oox3",     3, 100, qIDEAL,  giHyperb2, 0x49400, {{7, 7}}, eVariation::pure},
  {"{3,3,6}","none",    "{3,3,6} hyperbolic honeycomb",               "336",      4, 6, qIDEAL,    giHyperb3, 0x49600, {{7, 2}}, eVariation::pure},
  {"{3,4,4}","none",    "{3,4,4} hyperbolic honeycomb",               "344",      8, 4, qIDEAL,    giHyperb3, 0x50000, {{7, 2}}, eVariation::pure},
  {"{3,4,4}","Crystal", "4D crystal in H3",                           "Cryst3" ,  8, 4, qIDEAL | qANYQ | qCRYSTAL, giHyperb3, 0x52000, {{7, 3}}, eVariation::pure},
  };
  // bits: 9, 10, 15, 16, (reserved for later) 17, 18

// remember to match the following mask when specifying codes for extra geometries: 0x78600
// (other bits are used for other information)

#define X3(x) x, x, x

/** list of available models (i.e., projections) */
vector<modelinfo> mdinf = {
  {"disk/Gans", "general perspective", "general perspective", mf::azimuthal | mf::conformal},
  {"half-plane", "inversion", "half-plane", mf::conformal},
  {"band", "band", "Mercator", mf::band | mf::conformal},
  {X3("polygonal"), mf::conformal},
  {X3("formula"), 0},
  {X3("azimuthal equidistant"), mf::azimuthal | mf::equidistant | mf::euc_boring},  
  {X3("azimuthal equi-area"), mf::azimuthal | mf::equiarea | mf::euc_boring},
  {X3("ball model"), mf::conformal | mf::azimuthal | mf::space},
  {"Minkowski hyperboloid", "plane", "sphere", mf::conformal | mf::space | mf::euc_boring},
  {"hemisphere", "sphere", "sphere", mf::conformal | mf::space},
  {X3("band equidistant"), mf::band | mf::equidistant | mf::euc_boring},
  {X3("band equi-area"), mf::band | mf::equiarea | mf::euc_boring},
  {X3("sinusoidal"), mf::pseudoband | mf::equiarea | mf::euc_boring},
  {X3("two-point equidistant"), mf::equidistant | mf::euc_boring | mf::twopoint},
  {X3("fisheye"), 0},
  {X3("Joukowsky transform"), mf::hyper_only | mf::conformal},
  {X3("Joukowsky+inversion"), mf::hyper_only | mf::conformal},
  {X3("rotated hyperboles"), mf::hyper_only},  
  {X3("spiral/ring"), mf::hyper_or_torus | mf::uses_bandshift},
  {X3("native perspective"), 0},
  {X3("azimuthal equi-volume"), mf::azimuthal | mf::equivolume | mf::euc_boring},
  {X3("central inversion"), mf::azimuthal | mf::conformal},
  {X3("two-point azimuthal"), mf::euc_boring | mf::twopoint},
  {X3("two-point hybrid"), mf::euc_boring | mf::twopoint},
  {X3("geodesic"), 0},
  {X3("Mollweide"), mf::euc_boring | mf::pseudoband | mf::equiarea },
  {X3("central cylindrical"), mf::euc_boring | mf::band },
  {X3("Collignon"), mf::pseudoband | mf::equiarea },
  {X3("guard"), 0},
  {X3("polynomial"), mf::conformal}
  };

#undef X3

}
