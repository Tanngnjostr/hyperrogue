// Hyperbolic Rogue -- Archimedean Tilings
// Copyright (C) 2011-2019 Zeno Rogue, see 'hyper.cpp' for details

/** \file archimedean.cpp 
 *  \brief Archimedean tilings
 *
 *  These are tilings available in the 'Archimedean' option in Geometry Experiments; simpler Archimedean tilings are defined in other files.
 */

#include "hyper.h"
namespace hr {

EX namespace arcm {

#if HDR
struct archimedean_tiling {

  int coloring;

  string symbol;
  
  vector<int> faces;
  vector<int> adj;
  vector<bool> invert;
  vector<int> nflags;

  bool have_ph, have_line, have_symmetry;
  int real_faces;
  int real_face_type;

  int repetition;
  int N;

  ld euclidean_angle_sum;

  vector<int> flags;

  vector<vector<pair<int, int>>> adjacent;
  vector<vector<pair<ld, ld>>> triangles;
  
  void make_match(int a, int i, int b, int j);
  void prepare();
  void compute_geometry();
  
  void parse();
  void parse(string s) { symbol = s; parse(); }

  ld edgelength;
  
  vector<ld> inradius, circumradius, alphas;
  
  int matches[30][30];
  int periods[30];
  int tilegroup[30], groupoffset[30], tilegroups;

  int errors;
  string errormsg;

  pair<int, int>& get_adj(heptagon *h, int cid);
  pair<int, int>& get_adj(heptspin hs) { return get_adj(hs.at, hs.spin); }
  pair<ld, ld>& get_triangle(heptagon *h, int cid);
  pair<ld, ld>& get_triangle(heptspin hs) { return get_triangle(hs.at, hs.spin); }
  pair<ld, ld>& get_triangle(const pair<int, int>& p, int delta = 0);
  pair<int, int>& get_adj(const pair<int, int>& p, int delta = 0);

  int support_threecolor();
  int support_threecolor_bitruncated();
  int support_football();
  bool support_chessboard();
  void regroup();
  string world_size();
  void get_nom_denom(int& anom, int& adenom);
  
  geometryinfo1& get_geometry();
  eGeometryClass get_class() { return get_geometry().kind; }

  bool get_step_values(int& steps, int& single_step);
  
  ld scale();
  };
#endif

#if CAP_ARCM

static const int sfPH = 1;
static const int sfLINE = 2;
static const int sfCHESS = 4;
static const int sfTHREE = 8;
static const int sfSEMILINE = 16;

EX archimedean_tiling current;

/** id of vertex in the archimedean tiling
 *  odd numbers = reflected tiles
 *  0, 2, ..., 2(N-1) = as in the symbol
 *  2N = bitruncated tile
 */

EX short& id_of(heptagon *h) {
  return h->zebraval;
  }

/** which index in id_of's neighbor list does h->move(0) have */

EX short& parent_index_of(heptagon *h) {
  return h->emeraldval;
  }

/** total number of neighbors */

EX int neighbors_of(heptagon *h) {
  return isize(current.triangles[id_of(h)]);
  }

EX int gcd(int x, int y) { return x ? gcd(y%x, x) : y < 0 ? -y : y; }

void archimedean_tiling::make_match(int a, int i, int b, int j) {
  if(isize(adjacent[a]) != isize(adjacent[b])) {
    DEBB(DF_GEOM, ("(error here)"));
    errormsg = XLAT("polygons match incorrectly");
    errors++;
    }
  if(matches[a][b] == -1)
    matches[a][b] = j - i, matches[b][a] = i - j;
  else
    periods[a] = periods[b] = gcd(matches[a][b] - (j-i), periods[a]);
  }

void archimedean_tiling::prepare() {

  euclidean_angle_sum = 0;
  for(int f: faces) euclidean_angle_sum += (f-2.) / f;

  for(int i: faces) if(i > MAX_EDGE) {
    errormsg = XLAT("currently no more than %1 edges", its(MAX_EDGE));
    errors++;
    return;
    }
  if(isize(faces) > MAX_EDGE/2) {
    errormsg = XLAT("currently no more than %1 faces in vertex", its(MAX_EDGE/2));
    errors++;
    return;
    }
  if(isize(faces) < 2) {
    errormsg = XLAT("not enough faces");
    errors++;
    return;
    }
  for(int i: faces) if(i < 2) {
    errormsg = XLAT("not enough edges");
    errors++;
    return;
    }

  real_faces = 0, real_face_type = 0;
  for(int i=0; i<N; i++) if(faces[i] > 2) real_faces++, real_face_type += faces[i];
  real_face_type /= 2;

  if(real_faces) {
    for(int i=1; i<isize(faces); i++) if(faces[i] == 2 && faces[i-1] == 2) {
      errormsg = XLAT("Not implemented.");
      errors++;
      return;
      }
    if(faces[0] == 2 && faces[isize(faces)-1] == 2) {
      errormsg = XLAT("Not implemented.");
      errors++;
      return;
      }
    }

  if(real_faces == 2) {
    for(int i: faces) if(i != real_face_type) {
      errormsg = XLAT("polygons match incorrectly");
      errors++;
      }
    }

  errors = 0;

  /* build the 'adjacent' table */

  N = isize(faces);
  int M = 2 * N + 2;
  adjacent.clear();
  adjacent.resize(M);

  have_symmetry = false;
  for(int i=0; i<N; i++) if(invert[i]) have_symmetry = true;  
  
  for(int i=0; i<M; i++) for(int j=0; j<M; j++) matches[i][j] = i==j ? 0 : -1;

  for(int i=0; i<M; i++) periods[i] = i<2*N ? faces[i/2] : N;
  
  for(int i=0; i<N; i++) {
    for(int oi=0; oi<1; oi++) {
      int at = (i+oi)%N;
      int inv = oi;
      DEBB0(DF_GEOM, ("vertex "));
      for(int z=0; z<faces[i]; z++) {
        DEBB0(DF_GEOM, (format("[%d %d] " , at, inv)));
        adjacent[2*i+oi].emplace_back(2*N+int(inv), inv ? (2*at+2*N-2) % (2*N) : 2*at);
        if(invert[at]) inv ^= 1;
        at = adj[at];
        if(inv) at = (at+1) % N;
        else at = (at+N-1) % N;
        }
      if(!inv) make_match(2*i, 0, inv ? (2*at+2*N-1) % 2*N : 2*at, 0);
      DEBB(DF_GEOM, (format("-> [%d %d]\n", at, inv)));
      }
    }
  for(int i=0; i<N; i++) {
    adjacent[2*N].emplace_back(2*i, 0);
    int ai = (i+1) % N;
    adjacent[2*N].emplace_back(2*N+int(invert[ai]), (2*adj[ai]+2*N-1) % (2*N));
    }
  
  for(int d=0; d<=2*N; d+=2) {
    int s = isize(adjacent[d]);
    for(int i=0; i<s; i++) {
      auto& orig = adjacent[d][s-1-i];
      adjacent[d+1].emplace_back(orig.first ^ 1, orig.second);
      }
    }
  for(int d=0; d<M; d++) {
    int s = isize(adjacent[d]);
    for(int i=0; i<s; i++) {
      auto& orig = adjacent[d][i];
      if(orig.first & 1) orig.second = isize(adjacent[orig.first]) - 1 - orig.second;
      }
    }
  
  if(debugflags & DF_GEOM) {
    for(int i=0; i<M; i++) {
      DEBB0(DF_GEOM, ("adjacent ", i, ":"));
      for(int j=0; j<isize(adjacent[i]); j++) {
        auto p = adjacent[i][j];
        DEBB0(DF_GEOM, (" ", p));
        }
      DEBB(DF_GEOM, ("\n"));
      }
    }

  for(int i=0; i<M; i++) {
    for(int j=0; j<isize(adjacent[i]); j++) {
      auto p = adjacent[i][j];
      auto q = adjacent[p.first][p.second];
      make_match(i, j, q.first, q.second);
      }
    }
    
  /* verify all the triangles */
  for(int i=0; i<M; i++) {
    for(int j=0; j<isize(adjacent[i]); j++) {
      int ai = i, aj = j;
      DEBB0(DF_GEOM, ("triangle "));
      for(int s=0; s<3; s++) {
        DEBB0(DF_GEOM, (format("[%d %d] ", ai, aj)));
        tie(ai, aj) = adjacent[ai][aj];
        aj++; if(aj >= isize(adjacent[ai])) aj = 0;
        }
      DEBB(DF_GEOM, (format("-> [%d %d]\n", ai, aj)));
      make_match(i, j, ai, aj);
      }
    }

  for(int i=0; i<2*N; i++) {
    for(int j=0; j<isize(adjacent[i]); j++) {
      auto aa = make_pair(i, j);
      aa = get_adj(aa, 1);
      aa = get_adj(aa, 2);
      aa = get_adj(aa, 1);
      aa = get_adj(aa, 2);
      make_match(i, j, aa.first, aa.second);
      }
    }
  
  regroup();
  }

void archimedean_tiling::regroup() {
  int M = 2 * N + 2;
  for(int i=0; i<M; i++) for(int j=0; j<M; j++) if(matches[i][j] != -1)
  for(int l=0; l<M; l++) for(int k=0; k<M; k++) if(matches[j][k] != -1) {
    make_match(i, 0, k, matches[i][j] + matches[j][k]);
    make_match(i, 0, k, matches[i][j] + matches[j][k] + gcd(periods[i], periods[j]));
    }
  for(int i=0; i<M; i++) tilegroup[i] = -1;
  tilegroups = 0;
  for(int i=0; i<M; i+=(have_symmetry?1:2)) if(tilegroup[i] == -1) {
    if(periods[i] < 0) periods[i] = -periods[i];
    for(int j=0; j<M; j++) if(matches[i][j] != -1)
      tilegroup[j] = tilegroups, groupoffset[j] = matches[i][j] % periods[i];
    tilegroups++;
    }

  flags.clear();
  flags.resize(M);
  for(int i=0; i<M; i++)
  for(int j=0; j<M; j++) {
    if(tilegroup[i] == tilegroup[j]) {
      flags[i] |= nflags[j/2];
      if(j%2 == 1 && (nflags[j/2] & sfSEMILINE))
        flags[i] |= sfLINE;
      }
    }
  
  if(!have_ph) {
    for(int i=0; i<M; i++) if(tilegroup[i] == 0) flags[i] |= sfPH;
    }
  
  if(debugflags & DF_GEOM) {
    for(int i=0; i<M; i+=(have_symmetry?1:2)) {
      DEBB(DF_GEOM, (format("tiling group of %2d: [%2d]%2d+Z%2d\n", i, tilegroup[i], groupoffset[i], periods[i])));
      }
    }
  }

geometryinfo1& archimedean_tiling::get_geometry() {
  if(euclidean_angle_sum < 1.999999) return ginf[gSphere].g;
  else if(euclidean_angle_sum > 2.000001) return ginf[gNormal].g;
  else return ginf[gEuclid].g;
  }

void archimedean_tiling::compute_geometry() {
  ginf[gArchimedean].g = get_geometry();
  set_flag(ginf[gArchimedean].flags, qBOUNDED, get_class() == gcSphere);
  
  DEBB(DF_GEOM, (format("euclidean_angle_sum = %f\n", float(euclidean_angle_sum))));
  
  dynamicval<eGeometry> dv(geometry, gArchimedean);
  
  /* compute the geometry */
  inradius.resize(N);
  circumradius.resize(N);
  alphas.resize(N);
  ld elmin = 0, elmax = hyperbolic ? 10 : sphere ? M_PI : 1;
  
  if(real_faces == 2) {
    /* standard methods fail for dihedra, but the answer is easy */
    edgelength = 2 * M_PI / faces[0];
    for(int i=0; i<N; i++)
      if(faces[i] == 2)
        alphas[i] = 0,
        circumradius[i] = M_PI / real_face_type,
        inradius[i] = 0;
      else
        alphas[i] = M_PI/2,
        circumradius[i] = inradius[i] = M_PI/2;
    }
  else if(real_faces == 0) {
    // these are called hosohedra
    edgelength = M_PI;
    for(int i=0; i<N; i++)
      alphas[i] = M_PI / N,
      circumradius[i] = M_PI/2,
      inradius[i] = 0;
    }
  else for(int p=0; p<100; p++) {
    edgelength = (elmin + elmax) / 2;
    
    ld alpha_total = 0;

    for(int i=0; i<N; i++) {
      ld crmin = 0, crmax = sphere ? M_PI/2 : 10;
      ld el = 0;
      for(int q=0; q<100; q++) {
        circumradius[i] = (crmin + crmax) / 2;
        hyperpoint p1 = xpush0(circumradius[i]);
        hyperpoint p2 = spin(2 * M_PI / faces[i]) * p1;
        inradius[i] = hdist0(mid(p1, p2));
        el = hdist(p1, p2);
        if(el > edgelength) crmax = circumradius[i];
        else crmin = circumradius[i];
        }
      if(el < edgelength - 1e-3) alpha_total += 100; // could not make an edge that long
      hyperpoint h = xpush(edgelength/2) * xspinpush0(M_PI/2, inradius[i]);
      ld a = atan2(-h[1], h[0]);
      if(a < 0) a += 2 * M_PI;
      alphas[i] = a;
      // printf("  H = %s alp = %f\n", display(h), (float) atan2(-h[1], h[0]));
      alpha_total += alphas[i];
      }
    
    // printf("el = %f alpha = %f\n", float(edgelength), float(alpha_total));

    if(sphere ^ (alpha_total > M_PI)) elmin = edgelength;
    else elmax = edgelength;
    if(euclid) break;
    }
  
  DEBB(DF_GEOM, (format("computed edgelength = %f\n", float(edgelength))));
  
  triangles.clear();
  triangles.resize(2*N+2);
  for(int i=0; i<N; i++) for(int j=0; j<2; j++) 
    for(int k=0; k<faces[i]; k++) 
      triangles[2*i+j].emplace_back(2*M_PI/faces[i], circumradius[i]);
  
  for(int k=0; k<N; k++) {
    triangles[2*N].emplace_back(alphas[k], circumradius[k]);
    triangles[2*N].emplace_back(alphas[(k+1)%N], edgelength);
    triangles[2*N+1].emplace_back(alphas[N-1-k], edgelength);
    triangles[2*N+1].emplace_back(alphas[N-1-k], circumradius[N-1-k]);
    }
  
  for(auto& ts: triangles) {
    ld total = 0;
    for(auto& t: ts) tie(t.first, total) = make_pair(total, total + t.first);
    }

  if(debugflags & DF_GEOM) for(auto& ts: triangles) {
    DEBB0(DF_GEOM, ("T"));
    for(auto& t: ts) DEBB0(DF_GEOM, (format(" %f@%f", float(t.first), float(t.second))));
    DEBB(DF_GEOM, ());
    }
  
  }

ld archimedean_tiling::scale() {
  if(real_faces == 0 && N == 2) return M_PI / 2;
  if(real_faces == 2) return M_PI / 2;
  if(real_faces == 0) return 2 * M_PI / N;
  return edgelength;
  }

map<heptagon*, vector<pair<heptagon*, transmatrix> > > altmap;

EX map<heptagon*, pair<heptagon*, transmatrix>> archimedean_gmatrix;

EX hrmap *current_altmap;

heptagon *build_child(heptspin p, pair<int, int> adj);

bool skip_digons(heptspin hs, int step);
void connect_digons_too(heptspin h1, heptspin h2);
void fixup_matrix(transmatrix& T, const transmatrix& X, ld step);
void connectHeptagons(heptspin hi, heptspin hs);
transmatrix adjcell_matrix(heptagon *h, int d);

struct hrmap_archimedean : hrmap {
  map<int, struct cdata> eucdata;
  heptagon *origin;
  heptagon *getOrigin() { return origin; }

  hrmap_archimedean() {
    dynamicval<hrmap*> curmap(currentmap, this);
    int id = DUAL ? current.N * 2 : 0;;
    int N0 = isize(current.adjacent[id]);
    origin = tailored_alloc<heptagon> (N0);
    origin->s = hsOrigin;
    origin->emeraldval = 0;
    origin->zebraval = 0;
    origin->fiftyval = 0;
    origin->fieldval = 0;
    origin->rval0 = origin->rval1 = 0;
    origin->cdata = NULL;
    origin->alt = NULL;
    origin->c7 = NULL;
    origin->distance = 0;

    parent_index_of(origin) = DUAL ? 1 : 0;
    id_of(origin) = id;
    origin->c7 = newCell(N0/DUALMUL, origin);
    
    heptagon *alt = NULL;
    
    if(hyperbolic) {
      dynamicval<eGeometry> g(geometry, gNormal); 
      alt = tailored_alloc<heptagon> (S7);
      alt->s = hsOrigin;
      alt->emeraldval = 0;
      alt->zebraval = 0;
      alt->distance = 0;
      alt->c7 = NULL;
      alt->alt = alt;
      alt->cdata = NULL;
      current_altmap = newAltMap(alt); 
      }
  
    transmatrix T = xpush(.01241) * spin(1.4117) * xpush(0.1241) * Id;
    archimedean_gmatrix[origin] = make_pair(alt, T);
    altmap[alt].emplace_back(origin, T);
  
    if(current.real_faces == 0 && DUAL) {
      heptspin hs(origin, 0);
      heptagon *hnew = build_child(hs, current.get_adj(hs));
      for(int s=1; s<2*current.N; s++)
        origin->c.connect(s, hnew, s, false);
      }
    else if(current.real_faces == 0) {
      may_create_step(origin, 0); 
      heptagon *o0 = origin->move(0);
      may_create_step(origin, 1);
      heptagon *o1 = origin->move(1);
      for(int s=1; s<2*current.N; s+=2)
        o0->c.connect(s, o1, 2*current.N-s, false);
      for(int s=2; s<2*current.N; s+=2) {
        heptspin hs(o0, s);
        heptagon *hnew = build_child(hs, current.get_adj(hs));
        // no need to specify archimedean_gmatrix and altmap
        hnew->c.connect(1, heptspin(o1, 2*current.N-s));
        }
      o1->c.connect(1, o0, 2*current.N-1, false);
      }
    else if(origin->degree() == 2) {
      may_create_step(origin, 0);
      may_create_step(origin, 1);
      origin->move(0)->c.connect(1, origin->move(1), 2*current.N-1, false);
      origin->move(1)->c.connect(1, origin->move(0), 2*current.N-1, false);
      }
    
    cgi.base_distlimit = 0;
    celllister cl(origin->c7, 1000, 200, NULL);
    ginf[geometry].distlimit[!BITRUNCATED] = cgi.base_distlimit = cl.dists.back();
    if(sphere) cgi.base_distlimit = SEE_ALL;
    }

  ~hrmap_archimedean() {
    if(hyperbolic) for(auto& p: archimedean_gmatrix) if(p.second.first->cdata) {
      delete p.second.first->cdata;
      p.second.first->cdata = NULL;
      }
    clearfrom(origin);
    altmap.clear();
    archimedean_gmatrix.clear();
    if(current_altmap) {
      dynamicval<eGeometry> g(geometry, gNormal);       
      delete current_altmap;
      current_altmap = NULL;
      }
    }
  void verify() { }

  heptagon *create_step(heptagon *h, int d) {
  
    DEBB(DF_GEOM, (format("%p.%d ~ ?\n", h, d)));
  
    heptspin hi(h, d);
    
    while(skip_digons(hi, 1)) hi++;
    
    auto& t1 = current.get_triangle(hi);
  
    // * spin(-tri[id][pi+i].first) * xpush(t.second) * pispin * spin(tri[id'][p'+d'].first)
    
    auto& p1 = archimedean_gmatrix[h];
    
    heptagon *alt = p1.first;
  
    transmatrix T = p1.second * spin(-t1.first) * xpush(t1.second);
    transmatrix U = Id;
    
    if(hyperbolic) {
      dynamicval<eGeometry> g(geometry, gNormal); 
      dynamicval<hrmap*> cm(currentmap, current_altmap);
      U = T;
      virtualRebaseSimple(alt, T);
      U = U * inverse(T);
      }
    
    if(euclid) 
      alt = encodeId(pair_to_vec(int(T[0][LDIM]), int(T[1][LDIM])));
      
    DEBB(DF_GEOM, ("look for: ", alt, " / ", T * C0));
  
    for(auto& p2: altmap[alt]) if(intval(p2.second * C0, T * C0) < 1e-4) {
      DEBB(DF_GEOM, ("cell found: ", p2.first));
      for(int d2=0; d2<p2.first->degree(); d2++) {
        heptspin hs(p2.first, d2);
        auto& t2 = current.get_triangle(p2.first, d2);
        transmatrix T1 = T * spin(M_PI + t2.first);
        DEBB(DF_GEOM, ("compare: ", T1 * xpush0(1), ":: ", p2.second * xpush0(1)));
        if(intval(T1 * xpush0(1), p2.second * xpush0(1)) < 1e-4) {
        
          // T1 = p2.second
          // T * spin(pi+t2.first) == p2.second
          // p1.second * spinm(-t1.first) * xpush(t1.second) * spin(pi+t2.first) == p2.second
          
          // bring p1 and p2 closer, to prevent floating point errors
          if(hyperbolic) {
            fixup_matrix(p1.second, U * p2.second * spin(-M_PI - t2.first) * xpush(-t1.second) * spin(t1.first), 0.25);
            fixup_matrix(p2.second, T1, 0.25);
            }
  
          while(skip_digons(hs, -1)) hs--;
          connectHeptagons(hi, hs);
          connect_digons_too(hi, hs);
          return h->move(d);
          }
        }
      DEBB(DF_GEOM, ("but rotation not found"));
      }
    
    auto& t2 = current.get_triangle(current.get_adj(hi));
    transmatrix T1 = T * spin(M_PI + t2.first);
    fixmatrix(T1);
  
    heptagon *hnew = build_child(hi, current.get_adj(hi));
    altmap[alt].emplace_back(hnew, T1);
    archimedean_gmatrix[hnew] = make_pair(alt, T1);
    connect_digons_too(hi, heptspin(hnew, 0));
    
    return hnew;
    }
  
  void draw() {
    dq::visited.clear();
    dq::enqueue(viewctr.at, cview());
    
    while(!dq::drawqueue.empty()) {
      auto& p = dq::drawqueue.front();
      heptagon *h = get<0>(p);
      transmatrix V = get<1>(p);
      dynamicval<ld> b(band_shift, get<2>(p));
      dq::drawqueue.pop();
  
      int id = id_of(h);
      int S = isize(current.triangles[id]);
  
      if(id < 2*current.N ? !DUAL : !PURE) {
        if(!do_draw(h->c7, V)) continue;
        drawcell(h->c7, V);
        }
  
      for(int i=0; i<S; i++) {
        if(DUAL && (i&1)) continue;
        h->cmove(i);
        if(PURE && id >= 2*current.N && h->move(i) && id_of(h->move(i)) >= 2*current.N) continue;
        transmatrix V1 = V * adjcell_matrix(h, i);
        bandfixer bf(V1);
        dq::enqueue(h->move(i), V1);
        }
      }
    }
  
  transmatrix relative_matrix(heptagon *h2, heptagon *h1) {
    if(gmatrix0.count(h2->c7) && gmatrix0.count(h1->c7))
      return inverse(gmatrix0[h1->c7]) * gmatrix0[h2->c7];
    transmatrix gm = Id, where = Id;
    while(h1 != h2) {
      for(int i=0; i<neighbors_of(h1); i++) {
        if(h1->move(i) == h2) {
          return gm * adjcell_matrix(h1, i) * where;
          }
        }
      if(h1->distance > h2->distance) {
        gm = gm * adjcell_matrix(h1, 0);
        h1 = h1->move(0);
        }
      else {
        where = inverse(adjcell_matrix(h2, 0)) * where;
        h2 = h2->move(0);
        }
      }
    return gm * where;
    }
  
  };

EX hrmap *new_map() { return new hrmap_archimedean; }

heptagon *build_child(heptspin p, pair<int, int> adj) {
  indenter ind;
  auto h = buildHeptagon1(tailored_alloc<heptagon> (isize(current.adjacent[adj.first])), p.at, p.spin, hstate(1), 0);
  DEBB(DF_GEOM, (format("NEW %p.%d ~ %p.0\n", p.at, p.spin, h)));
  id_of(h) = adj.first;
  parent_index_of(h) = adj.second;
  int nei = neighbors_of(h);
  h->c7 = newCell(nei/DUALMUL, h);
  h->distance = p.at->distance + 1;
  if(adj.first < 2*current.N && !DUAL) {
    int s = 0;
    heptspin hs(p);
    while(id_of(hs.at->move(0)) >= 2 * current.N) {
      s += hs.spin / 2 - 1;
      hs = hs - hs.spin + wstep - 1;
      }
    h->fieldval = hs.at->move(0)->fieldval + s + hs.spin/2;
    }
  else
    h->fieldval = -100;
  h->fiftyval = isize(archimedean_gmatrix);
  if(p.at->s == hsOrigin)
    h->rval1 = 1 + (p.spin % 2);
  else {
    if(p.spin % 2 == 0)
      h->rval1 = p.at->move(0)->rval1;
    else
      h->rval1 = 3 - p.at->move(0)->rval1 - p.at->rval1;
    }
  h->rval0 = hrand(256);
  heptspin hs(h, 0);
  return h;
  }

bool skip_digons(heptspin hs, int step) {
  return
    isize(current.adjacent[current.get_adj(hs).first]) == 2 ||
    isize(current.adjacent[current.get_adj(hs+step).first]) == 2;
  }

void connect_digons_too(heptspin h1, heptspin h2) {
  if(skip_digons(h1, -1)) {
    h1--, h2++;
    heptagon *hnew = build_child(h1, current.get_adj(h1));
    // no need to specify archimedean_gmatrix and altmap
    hnew->c.connect(1, h2);
    h1--, h2++;
    DEBB(DF_GEOM, (format("OL2 %p.%d ~ %p.%d\n", h1.at, h1.spin, h2.at, h2.spin)));
    h1.at->c.connect(h1.spin, h2);
    }
  }

void connectHeptagons(heptspin hi, heptspin hs) {
  DEBB(DF_GEOM, (format("OLD %p.%d ~ %p.%d\n", hi.at, hi.spin, hs.at, hs.spin)));
  if(hi.at->move(hi.spin) == hs.at && hi.at->c.spin(hi.spin) == hs.spin) {
    DEBB(DF_GEOM, (format("WARNING: already connected\n")));
    return;
    }
  if(hi.peek()) {
    DEBB(DF_GEOM, (format("ERROR: already connected left\n")));
    exit(1);
    }
  if(hs.peek()) {
    DEBB(DF_GEOM, (format("ERROR: already connected right\n")));
    exit(1);
    }
  hi.at->c.connect(hi.spin, hs);

  auto p = current.get_adj(hi);
  if(current.tilegroup[p.first] != current.tilegroup[id_of(hs.at)]) {
    printf("should merge %d %d\n", p.first, id_of(hs.at));
    current.make_match(p.first, p.second, id_of(hs.at), hs.spin + parent_index_of(hs.at));
    current.regroup();
    }
  // heptagon *hnew = build_child(h, d, get_adj(h, d).first, get_adj(h, d).second);
  }

/** T and X are supposed to be equal -- move T so that it is closer to X */
void fixup_matrix(transmatrix& T, const transmatrix& X, ld step) {
  for(int i=0; i<MDIM; i++)
  for(int j=0; j<MDIM; j++)
    T[i][j] = (T[i][j] * (1-step) + X[i][j] * step);

  /*
  for(int i=0; i<3; i++)
  for(int j=0; j<3; j++)
    if(T[i][j] - X[i][j] > 1e-3) exit(1);
  */
  fixmatrix(T);
  }

pair<ld, ld>& archimedean_tiling::get_triangle(heptagon *h, int cid) {
  return triangles[id_of(h)][(parent_index_of(h) + cid + MODFIXER) % neighbors_of(h)];
  }

pair<int, int>& archimedean_tiling::get_adj(heptagon *h, int cid) {
  return adjacent[id_of(h)][(parent_index_of(h) + cid + MODFIXER) % neighbors_of(h)];
  }

pair<int, int>& archimedean_tiling::get_adj(const pair<int, int>& p, int delta) {
  return adjacent[p.first][(p.second + delta + MODFIXER) % isize(adjacent[p.first])];
  }

pair<ld, ld>& archimedean_tiling::get_triangle(const pair<int, int>& p, int delta) {
  return triangles[p.first][(p.second + delta + MODFIXER) % isize(adjacent[p.first])];
  }

transmatrix adjcell_matrix(heptagon *h, int d) {
  auto& t1 = current.get_triangle(h, d);

  heptagon *h2 = h->move(d);

  int d2 = h->c.spin(d);
  auto& t2 = current.get_triangle(h2, d2);
  
  return spin(-t1.first) * xpush(t1.second) * spin(M_PI + t2.first);
  }

EX int fix(heptagon *h, int spin) {
  int type = isize(current.adjacent[id_of(h)]);
  spin %= type;
  if(spin < 0) spin += type;
  return spin;
  }

void archimedean_tiling::parse() {
  int at = 0;
  
  auto peek = [&] () { if(at == isize(symbol)) return char(0); else return symbol[at]; };
  auto is_number = [&] () { char p = peek(); return p >= '0' && p <= '9'; };
  auto read_number = [&] () { int result = 0; while(is_number()) result = 10 * result + peek() - '0', at++; return result; };
  
  faces.clear(); nflags.clear();
  have_line = false;
  have_ph = false;
  int nflags0;
  auto nfback = [this, &nflags0] () -> int& { if(nflags.empty()) return nflags0; else return nflags.back(); };
  while(true) {
    if(peek() == ')' || (peek() == '(' && isize(faces)) || peek() == 0) break;
    else if((peek() == 'L') && faces.size()) {
      if(!nflags.empty()) nfback() |= sfLINE;
      have_line = true, at++;
      }
    else if((peek() == 'l') && faces.size()) {
      if(!nflags.empty()) nfback() |= sfSEMILINE;
      have_line = true, at++;
      }
    else if((peek() == 'H' || peek() == 'h') && faces.size()) {
      if(!nflags.empty()) nfback() |= sfPH;
      have_ph = true, at++;
      }
    else if(is_number()) faces.push_back(read_number()), nflags.push_back(0);
    else if(peek() == '^' && !faces.empty()) {
      at++;
      int rep = read_number();
      if(rep == 0) nflags.pop_back(), faces.pop_back();
      for(int i=1; i<rep; i++) nflags.push_back(nfback()), faces.push_back(faces.back());
      }
    else at++;
    }
  nflags.push_back(nflags0);
  repetition = 1;
  N = isize(faces);
  invert.clear(); invert.resize(N, true);
  adj.clear(); adj.resize(N, 0); for(int i=0; i<N; i++) adj[i] = i;
  while(peek() != 0) {
    if(peek() == '^') at++, repetition = read_number();
    else if(peek() == '(') { 
      at++; int a = read_number(); while(!is_number() && !among(peek(), '(', '[', ')',']', 0)) at++;
      if(is_number()) { int b = read_number(); adj[a] = b; adj[b] = a; invert[a] = invert[b] = false; }
      else { invert[a] = false; }
      }
    else if(peek() == '[') { 
      at++; int a = read_number(); while(!is_number() && !among(peek(), '(', '[', ')',']', 0)) at++;
      if(is_number()) { int b = read_number(); adj[a] = b; adj[b] = a; invert[a] = invert[b] = true; }
      else { invert[a] = true; }
      }
    else at++;
    }
  for(int i=0; i<N * (repetition-1); i++)
    faces.push_back(faces[i]),
    nflags.push_back(nflags[i]),
    invert.push_back(invert[i]),
    adj.push_back(adj[i] + N);
  N *= repetition;
  prepare();  
  }

#if CAP_COMMANDLINE  
int readArgs() {
  using namespace arg;
           
  if(0) ;
  else if(argis("-symbol")) {
    PHASEFROM(2);
    archimedean_tiling at;
    shift(); at.parse(args());
    if(at.errors) {
      DEBB(DF_ERROR | DF_GEOM, ("error: ", at.errormsg));
      }
    else {
      set_geometry(gArchimedean);
      current = at;
      showstartmenu = false;
      }
    }
  else if(argis("-dual")) { PHASEFROM(2); set_variation(eVariation::dual); }
  else if(argis("-d:arcm")) 
    launch_dialog(show);
  else return 1;
  return 0;
  }
#endif

#if CAP_COMMANDLINE
auto hook = 
  addHook(hooks_args, 100, readArgs)
+ addHook(hooks_gamedata, 0, [] (gamedata* gd) { gd->store(altmap); gd->store(archimedean_gmatrix); gd->store(current_altmap); });

#endif

#if MAXMDIM >= 4
auto hooksw = addHook(hooks_swapdim, 100, [] {
  for(auto& p: altmap) for(auto& pp: p.second) swapmatrix(pp.second);
  for(auto& p: archimedean_gmatrix) swapmatrix(p.second.second);
  });
#endif

int archimedean_tiling::support_threecolor() {
  return (isize(faces) == 3 && invert[0] && invert[1] && invert[2] && faces[0] % 2 == 0 && faces[1] % 2 == 0 && faces[2] % 2 == 0) ? 2 :
    tilegroup[N*2] > 1 ? 1 :
    0;
  return 2;
  }

int archimedean_tiling::support_threecolor_bitruncated() {
  for(int i: faces) if(i % 2) return 0;
  return 2;
  }

int archimedean_tiling::support_football() {
  return 
    have_ph ? 1 :
    (isize(faces) == 3 && invert[0] && invert[1] && invert[2] && faces[1] % 2 == 0 && faces[2] % 2 == 0) ? 2 :
    0;
  }

bool archimedean_tiling::support_chessboard() {
  return N % 2 == 0;
  }

EX bool pseudohept(cell *c) {
  if(DUAL)
    return !(c->master->rval0 & 3);
  int id = id_of(c->master);
  if(PURE) 
    return current.flags[id] & arcm::sfPH;
  if(BITRUNCATED)
    return id < current.N * 2;
  return false;
  }

EX bool chessvalue(cell *c) {
  if(DUAL)
    return c->master->rval1 - 1;
  return c->master->fieldval & 1;
  }

EX bool linespattern(cell *c) {
  return current.flags[id_of(c->master)] & arcm::sfLINE;
  }

EX int threecolor(cell *c) {
  if(current.have_ph)
    return !arcm::pseudohept(c);
  else if(PURE)
    return current.tilegroup[id_of(c->master)];
  else {
    int id = id_of(c->master);
    if(current.support_threecolor() == 2) return id < current.N * 2 ? (id&1) : 2;
    return current.tilegroup[id];
    }
  }

int cEucRegular = 0x008000;
int cEucSemiregular = 0x40C040;
int cPlatonic = 0x000080;
int cArchimedean = 0x4040C0;
int cPrism = 0x40A0A0;
int cAntiPrism = 0x80A0A0;
int cHyperRegular = 0x800000;
int cHyperSemi = 0xC04040;

int cWeird = 0xA000A0;

vector<pair<string, int> > samples = {
  /* Euclidean */
  {"(3,3,3,3,3,3)", cEucRegular},
  {"(4,4,4,4)", cEucRegular},
  {"(6,6,6)", cEucRegular},
  {"(8,8,4)", cEucSemiregular},
  {"(4,6,12)", cEucSemiregular},
  {"(6,4,3,4)", cEucSemiregular}, 
  {"(3,6,3,6)", cEucSemiregular}, 
  {"(3,12,12)", cEucSemiregular}, 
  {"(4,4,3L,3L,3L) [3,4]", cEucSemiregular},
  {"(3,3,3,3,6) (1,2)(0,4)(3)", cEucSemiregular},
  {"(3,3,4,3,4) (0,4)(1)(2,3)", cEucSemiregular},
  
  /* Platonic */
  {"(3,3,3)", cPlatonic},
  {"(3,3,3,3)", cPlatonic},
  {"(3,3,3,3,3)", cPlatonic},
  {"(4,4,4)", cPlatonic},
  {"(5,5,5)", cPlatonic},
  
  /* Archimedean solids */
  {"(3,6,6)", cArchimedean},
  {"(3,4,3,4)", cArchimedean},
  {"(3,8,8)", cArchimedean},
  {"(4,6,6)", cArchimedean},
  {"(3,4,4,4)", cArchimedean},
  {"(4,6,8)", cArchimedean}, 
  {"(3,3,3,3,4) (1,2)(0,4)(3)", cArchimedean},
  {"(3,5,3,5)", cArchimedean},
  {"(3,10,10)", cArchimedean},
  {"(5,6,6)", cArchimedean},
  {"(3,4,5,4)", cArchimedean},
  {"(4,6,10)", cArchimedean},
  {"(3,3,3,3,5) (1,2)(0,4)(3)", cArchimedean},
  
  /* prisms */
  {"(3,4,4)", cPrism},
  {"(5,4,4)", cPrism},
  {"(6,4,4)", cPrism},
  {"(7,4,4)", cPrism},
  
  /* sample antiprisms */
  {"(3,3,3,4)(1)(2)", cAntiPrism},
  {"(3,3,3,5)(1)(2)", cAntiPrism},
  {"(3,3,3,6)(1)(2)", cAntiPrism},
  {"(3,3,3,7)(1)(2)", cAntiPrism}, 
  
  /* hyperbolic ones */
  {"(3)^7", cHyperRegular},
  {"(4)^5", cHyperRegular}, 
  {"(4)^6", cHyperRegular}, 
  {"(5,5,5,5)", cHyperRegular},
  {"(7,7,7)", cHyperRegular},
  {"(8,8,8)", cHyperRegular},
  {"(7,6^2)", cHyperSemi},
  {"(4,6,14)", cHyperSemi},
  {"(3,4,7,4)", cHyperSemi},
  {"(6,6,4L,4L)", cHyperSemi},
  {"(8,8,4L,4L)", cHyperSemi},
  {"(3,3,3,3,7) (1,2)(0,4)(3)", cHyperSemi}, 
  {"(3H,6,6,6) (1,0)[2](3)", cHyperSemi},
  {"(3,6,6,6) (0 1)(2)(3)", cHyperSemi},
  {"(3,4,4L,4L,4)", cHyperSemi}, // buggy
  {"(3l,4l,4,4,4) (0 1)[2 3](4)", cHyperSemi},
  {"(3,4,4,4,4) (0 1)(2)(3)(4)", cHyperSemi},
  {"(3,4,4L,4L,4L,4)", cHyperSemi},
  {"(6,6,3L,3L,3L) (0 2)(1)(3)(4)", cHyperSemi},
  {"(5,3,5,3,3) (0 1)(2 3)(4)", cHyperSemi}, 
  {"(4,3,3,3,3,3) (0 1)(2 3)(4 5)", cHyperSemi},
  {"(3l,5l,5,5,5,5) (0 1)[2 3](4)(5)", cHyperSemi},
  {"(3,5,5,5,5,5) (0 1)(2 4)(3 5)", cHyperSemi}, 
  {"(3l,5l,5,5,5,5) (0 1)(2 4)[3 5]", cHyperSemi},
  {"(3l,5l,5,5,5,5) (0 1)[2 4](3)(5)", cHyperSemi}, 
  {"(3,5,5,5,5,5) (0 1)(2)(3)(4)(5)", cHyperSemi}, 
  
  /* interesting symmetry variants */
  {"(3,3,3,3,3,3) (0,1)(2,3)(4,5)", cEucRegular},
  {"(3,3H,3,3,3L,3L,3L) (0 4)(1 2)(3)(5)(6)", cHyperRegular},
  {"(3,3H,3,3,3L,3L,3L) (0 4)(1 2)(3)[5 6]", cHyperRegular},
  {"(3,3H,3,3L,3,3L,3L) [0 4](1 2)[3 5](6)", cHyperRegular},

  /* with digons */
  {"(2,3,3,3,3,3) (2,3)(4,5)", cWeird},
  {"(6,6)", cWeird},
  {"(2,2)", cWeird},
  {"(2,2,2,2,2,2)", cWeird},
  {"(6,6,2)", cWeird},
  {"(6,2,6,2)", cWeird},
  };

int lastsample = 0;

vector<archimedean_tiling> tilings;

int spos = 0;

archimedean_tiling edited;

bool symbol_editing;

EX void next_variation() {
  set_variation(
    PURE ? eVariation::dual :
    DUAL ? eVariation::bitruncated : 
    eVariation::pure);
  start_game();
  }

EX void enable(archimedean_tiling& arct) {
  stop_game();
  if(!archimedean) set_variation(eVariation::pure);
  set_geometry(gArchimedean);
  patterns::whichPattern = patterns::PAT_NONE;
  current = arct;
#if CAP_TEXTURE
  if(texture::config.tstate == texture::tsActive && texture::cgroup == cpThree) {
    patterns::whichPattern = patterns::PAT_COLORING;
    if(geosupport_threecolor() < 2) {
      if(arct.support_threecolor() == 2) set_variation(eVariation::pure);
      else if(arct.support_threecolor_bitruncated() == 2) set_variation(eVariation::bitruncated);
      }
    }
  if(texture::config.tstate == texture::tsActive && texture::cgroup == cpFootball) {
    patterns::whichPattern = patterns::PAT_TYPES, patterns::subpattern_flags = patterns::SPF_FOOTBALL;
    if(geosupport_football() < 2) set_variation(eVariation::bitruncated);
    }
  if(texture::config.tstate == texture::tsActive && texture::cgroup == cpChess) {
    patterns::whichPattern = patterns::PAT_CHESS;
    if(!geosupport_chessboard()) {
      if(arct.support_chessboard()) set_variation(eVariation::pure);
      else if(arct.support_threecolor_bitruncated() == 2) set_variation(eVariation::dual);
      }
    }
#endif
  start_game();
  }

function<void()> setcanvas(char c) {
  return [c] () {
    stop_game();
    firstland = specialland = laCanvas;
    patterns::whichCanvas = c;
    start_game();
    };
  };

EX void show() {
  if(lastsample < isize(samples)) {
    string s = samples[lastsample].first;
    int col = samples[lastsample].second;
    lastsample++;
    archimedean_tiling tested;
    tested.parse(s);
    if(tested.errors) {
      DEBB(DF_GEOM | DF_WARN, ("WARNING: ", tested.errors, " errors on ", s, " '", tested.errormsg, "'"));
      }
    else {
      tested.coloring = col;
      tilings.push_back(move(tested));
      /* sort(tilings.begin(), tilings.end(), [] (archimedean_tiling& s1, archimedean_tiling& s2) {
        if(s1.euclidean_angle_sum < s2.euclidean_angle_sum - 1e-6) return true;
        if(s2.euclidean_angle_sum < s1.euclidean_angle_sum - 1e-6) return false;
        return s1.symbol < s2.symbol;
        }); */
      }
    }
  cmode = sm::SIDE | sm::MAYDARK;
  gamescreen(0);  
  dialog::init(XLAT("Archimedean tilings"));
  
  if(symbol_editing) {
    dialog::addSelItem("edit", dialog::view_edited_string(), '/');
    dialog::add_action([] () { 
      symbol_editing = false;
      if(!edited.errors) enable(edited);
      });
    dialog::addBreak(100);
    if(edited.errors)
      dialog::addInfo(edited.errormsg, 0xFF0000);
    else
      dialog::addInfo(XLAT("OK"), 0x00FF00);
    
    dialog::addBreak(100);
    dialog::addSelItem(XLAT("full angle"), fts(edited.euclidean_angle_sum * 180) + "°", 0);
    dialog::addSelItem(XLAT("size of the world"), edited.world_size(), 0);

    edited.compute_geometry();
    dialog::addSelItem(XLAT("edge length"), fts(edited.edgelength) + (edited.get_class() == gcEuclid ? XLAT(" (arbitrary)") : ""), 0);
    current.compute_geometry();

    dialog::addBreak(100);
    dialog::addKeyboardItem("1234567890");
    dialog::addKeyboardItem("()[]lLhH,");
    dialog::addKeyboardItem(" \t\b\x1\x2\n");
    dialog::addBreak(100);
    }
  else {
    string cs = archimedean ? current.symbol : XLAT("OFF");
    dialog::addSelItem("edit", cs, '/');  
    dialog::add_action([] () { 
      symbol_editing = true;
      edited = current;
      dialog::start_editing(edited.symbol);
      edited.parse();
      });
    dialog::addBreak(100);
    int nextpos = spos;
    int shown = 0;
    while(nextpos < isize(tilings) && shown < 10) {
      auto &ps = tilings[nextpos++];
      bool valid = true;
      string suffix = "";
#if CAP_TEXTURE
      if(texture::config.tstate == texture::tsActive && texture::cgroup == cpThree) {
        valid = false;
        if(ps.support_threecolor() == 2) valid = true, suffix += bitruncnames[int(eVariation::pure)];
        if(ps.support_threecolor_bitruncated() == 2) valid = true, suffix += bitruncnames[int(eVariation::bitruncated)];
        }
      if(texture::config.tstate == texture::tsActive && texture::cgroup == cpFootball) {
        if(ps.support_football() == 2) suffix += bitruncnames[int(eVariation::pure)];
        suffix += bitruncnames[int(eVariation::bitruncated)];
        }
      if(texture::config.tstate == texture::tsActive && texture::cgroup == cpChess && !ps.support_chessboard()) {
        valid = false;
        if(ps.support_chessboard()) valid = true, suffix += bitruncnames[int(eVariation::pure)];
        if(ps.support_threecolor_bitruncated() == 2) valid = true, suffix += bitruncnames[int(eVariation::dual)];
        }
#endif
      if(!valid) continue;
      dialog::addSelItem(ps.symbol, fts(ps.euclidean_angle_sum * 180) + "°" + suffix, 'a' + shown);
      dialog::lastItem().color = ps.coloring;
      dialog::add_action([&] () { enable(ps); });
      shown++;
      }
    dialog::addItem(XLAT("next page"), '-');
    if(shown == 0) nextpos = 0;
    dialog::add_action([nextpos] () {
      if(nextpos >= isize(tilings))
        spos = 0;
      else spos = nextpos;
      });
    dialog::addItem(XLAT("previous page"), '=');
    dialog::add_action([] () {
      spos -= 10;
      if(spos < 0) spos = 0;
      });
    
    if(archimedean) {
      dialog::addSelItem(XLAT("size of the world"), current.world_size(), 0);
      dialog::addSelItem(XLAT("edge length"), current.get_class() == gcEuclid ? (fts(current.edgelength) + XLAT(" (arbitrary)")) : fts(current.edgelength), 0);

      dialog::addItem(XLAT("color by symmetries"), 't');
      dialog::add_action(setcanvas('A'));
      }
    else {
      dialog::addBreak(100);
      dialog::addBreak(100);
      dialog::addBreak(100);
      }

    if(true) {
      dialog::addItem(XLAT("color by sides"), 'u');
      dialog::add_action(setcanvas('B'));
      }
    
    if(geosupport_threecolor() == 2) {
      dialog::addItem(XLAT("three colors"), 'w');
      dialog::add_action(setcanvas('T'));
      }
    else if(geosupport_football() == 2) {
      dialog::addItem(XLAT("football"), 'w');
      dialog::add_action(setcanvas('F'));
      }
    else if(geosupport_chessboard()) {
      dialog::addItem(XLAT("chessboard"), 'w');
      dialog::add_action(setcanvas('c'));
      }
    else dialog::addBreak(100);

    if(archimedean) {    
      dialog::addSelItem(XLAT("variations"), gp::operation_name(), 'v');
      dialog::add_action(next_variation);
      }
    else dialog::addBreak(100);
    }

  dialog::addHelp();
  dialog::addBack();
  dialog::display();

  keyhandler = [] (int sym, int uni) {
    if(symbol_editing && sym == SDLK_RETURN) sym = uni = '/';
    dialog::handleNavigation(sym, uni);
    if(symbol_editing && dialog::handle_edit_string(sym, uni)) {
      edited.parse(edited.symbol);
      return;
      }
    if(doexiton(sym, uni)) popScreen();
    };
  }

void archimedean_tiling::get_nom_denom(int& anom, int& adenom) {
  int nom = 2 - N, denom = 2;
  for(int f: faces) {
    int g = gcd(denom, f);
    nom = (nom * f + denom) / g;
    denom = denom / g * f;
    }
  anom = 0, adenom = 1;
  if(BITRUNCATED || DUAL) anom = 1, adenom = 1;
  if(!DUAL) for(int f: faces) {
    int g = gcd(adenom, f);
    anom = (anom * f + adenom) / g;
    adenom = adenom / g * f;
    }
  anom *= 2 * denom, adenom *= nom;
  int g = gcd(anom, adenom);
  if(g != 0) {
    anom /= g; adenom /= g;
    }
  if(adenom < 0) anom = -anom, adenom = -adenom;
  }

string archimedean_tiling::world_size() {
  if(get_class() == gcEuclid) return "∞";
  
  int anom, adenom;
  get_nom_denom(anom, adenom);

  string s;
  bool hyp = (anom < 0);
  if(hyp) anom = -anom;
  if(adenom != 1) 
    s += its(anom) + "/" + its(adenom);
  else
    s += its(anom);
  if(hyp) s += " exp(∞)";
  return s;
  }

EX int degree(heptagon *h) {
  return isize(current.adjacent[id_of(h)]);
  }

EX bool is_vertex(heptagon *h) {
  return id_of(h) >= 2 * current.N;
  }

bool archimedean_tiling::get_step_values(int& steps, int& single_step) {

  int nom = -2;
  int denom = 1;
  
  for(int f: arcm::current.faces) {
    if(int(denom*f)/f != denom) { steps = 0; single_step = 0; return false; }
    int g = gcd(denom, f);
    nom = nom * (f/g) + (f-2) * (denom/g);
    denom = denom/g * f;
    }
    
  steps = 2 * abs(denom);
  single_step = abs(nom);
  if(steps/2 != abs(denom)) return false;
  return (2 * denom) % nom == 0;  
  }

EX int valence() {
  if(PURE) return arcm::current.N;
  if(BITRUNCATED) return 3;
  // in DUAL, usually valence would depend on the vertex.
  // 3 is the most interesting, as it allows us to kill hedgehog warriors
  int total = 0;
  for(int i: current.faces) {
    if(i == 3) return 3;
    total += i;
    }
  return total / isize(current.faces);
  }
 
#endif

EX map<int, cdata>& get_cdata() { return ((arcm::hrmap_archimedean*) (currentmap))->eucdata; }
  }

}
