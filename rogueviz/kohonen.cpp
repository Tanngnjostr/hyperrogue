// Hyperbolic Rogue
// Copyright (C) 2011-2018 Zeno and Tehora Rogue, see 'hyper.cpp' for details

// Kohonen's self-organizing maps. 
// This is a part of RogueViz, not a part of HyperRogue.

namespace rogueviz { namespace kohonen {

int columns;

typedef vector<double> kohvec;

struct sample {
  kohvec val;
  string name;
  };

vector<sample> data;

map<int, int> sample_vdata_id;

int whattodraw[3] = {-2,-2,-2};

int min_group = 10, max_group = 10;

struct neuron {
  kohvec net;
  cell *where;
  double udist;
  int lpbak;
  color_t col;
  int allsamples, drawn_samples, csample, bestsample, max_group_here;
  neuron() { drawn_samples = allsamples = bestsample = 0; max_group_here = max_group; }
  };

vector<string> colnames;

kohvec weights;

vector<neuron> net;

int neuronId(neuron& n) { return &n - &(net[0]); }

void alloc(kohvec& k) { k.resize(columns); }

bool neurons_indexed = false;

int samples;

template<class T> T sqr(T x) { return x*x; }

vector<neuron*> whowon;

void normalize() {
  alloc(weights);
  for(int k=0; k<columns; k++) {
    double sum = 0, sqsum = 0;
    for(sample& s: data)
      sum += s.val[k],
      sqsum += s.val[k] * s.val[k];
    double variance = sqsum/samples - sqr(sum/samples);
    weights[k] = 1 / sqrt(variance);
    }
  }

double vnorm(kohvec& a, kohvec& b) {
  double diff = 0;
  for(int k=0; k<columns; k++) diff += sqr((a[k]-b[k]) * weights[k]);
  return diff;
  }

void sominit(int, bool load_compressed = false);
void uninit(int);

bool noshow = false;

vector<int> samples_to_show;

void loadsamples(const string& fname) {
  fhstream f(fname, "rt");
  if(!f.f) {
    fprintf(stderr, "Could not load samples: %s\n", fname.c_str());
    return;
    }
  if(!scan(f, columns)) { 
    printf("Bad format: %s\n", fname.c_str());
    return; 
    }
  printf("Loading samples: %s\n", fname.c_str());
  while(true) {
    sample s;
    bool shown = false;
    alloc(s.val);
    if(feof(f.f)) break;
    for(int i=0; i<columns; i++)
      if(!scan(f, s.val[i])) { goto bigbreak; }
    fgetc(f.f);
    while(true) {
      int c = fgetc(f.f);
      if(c == -1 || c == 10 || c == 13) break;
      if(c == '!' && s.name == "") shown = true;
      else if(c != 32 && c != 9) s.name += c;
      }
    data.push_back(move(s));
    if(shown) 
      samples_to_show.push_back(isize(data)-1);
    }
  bigbreak:
  samples = isize(data);
  normalize();
  colnames.resize(columns);
  for(int i=0; i<columns; i++) colnames[i] = "Column " + its(i);
  uninit(0); sominit(1);
  }
                  
int tmax = 30000;
double distmul = 1;
double learning_factor = .1;

int qpct = 100;

int t, lpct, cells;
double maxdist;

neuron& winner(int id) {
  double bdiff = HUGE_VAL;
  neuron *bcell = NULL;
  for(neuron& n: net) {
    double diff = vnorm(n.net, data[id].val);
    if(diff < bdiff) bdiff = diff, bcell = &n;
    }
  return *bcell;
  }

void setindex(bool b) {
  if(b == neurons_indexed) return;
  neurons_indexed = b;
  if(b) {
    for(neuron& n: net) n.lpbak = n.where->landparam, n.where->landparam = neuronId(n);
    }
  else {
    for(neuron& n: net) n.where->landparam = n.lpbak;
    }
  }

neuron *getNeuron(cell *c) {
  if(!c) return NULL;
  setindex(true);
  if(c->landparam < 0 || c->landparam >= cells) return NULL;
  neuron& ret = net[c->landparam];
  if(ret.where != c) return NULL;
  return &ret;
  }

neuron *getNeuronSlow(cell *c) {
  if(neurons_indexed) return getNeuron(c);
  for(neuron& n: net) if(n.where == c) return &n;
  return NULL;
  }

double maxudist;

neuron *distfrom;

void coloring() {
  if(noshow) return;
  setindex(false);
  
  bool besttofind = true;

  for(int pid=0; pid<3; pid++) {
    int c = whattodraw[pid];
    
    if(c == -5) {
      if(besttofind) {
        besttofind = false;
        for(neuron& n: net) {
          double bdiff = 1e20;
          for(auto p: sample_vdata_id) {
            double diff = vnorm(n.net, data[p.first].val);
            if(diff < bdiff) bdiff = diff, n.bestsample = p.second;
            }
          }
        }

      for(int i=0; i<cells; i++) {
        part(net[i].where->landparam_color, pid) = part(vdata[net[i].bestsample].cp.color1, pid+1);
        }
      }

    else {
      vector<double> listing;
      for(neuron& n: net) switch(c) {
        case -4:
          listing.push_back(log(5+n.allsamples));
          break;
          
        case -3:
          if(distfrom) 
            listing.push_back(vnorm(n.net, distfrom->net));
          else
            listing.push_back(0);
          break;
        
        case -2:
          listing.push_back(n.udist);
          break;
        
        case -1: 
          listing.push_back(-n.udist);
          break;
        
        default:
          listing.push_back(n.net[c]);
          break;
        }
      
      double minl = listing[0], maxl = listing[0];
      for(double& d: listing) minl = min(minl, d), maxl = max(maxl, d);
      if(maxl-minl < 1e-3) maxl = minl+1e-3;
      
      for(int i=0; i<cells; i++) 
        part(net[i].where->landparam_color, pid) = (255 * (listing[i] - minl)) / (maxl - minl);
      }
    }
  }

void distribute_neurons() {
  whowon.resize(samples);
  
  for(neuron& n: net) n.drawn_samples = 0, n.csample = 0;
  
  for(auto p: sample_vdata_id) {
    int s = p.first;
    auto& w = winner(s);
    whowon[s] = &w;
    w.drawn_samples++;
    }
  
  ld rad = .25 * cgi.scalefactor;
  
  for(auto p: sample_vdata_id) {
    int id = p.second;
    int s = p.first;
    auto& w = *whowon[s];
    vdata[id].m->base = w.where;
    vdata[id].m->at = 
      spin(2*M_PI*w.csample / w.drawn_samples) * xpush(rad * (w.drawn_samples-1) / w.drawn_samples);
    w.csample++;
    }
  
  shmup::fixStorage();  
  setindex(false);
  }

void analyze() {

  setindex(true);
  
  maxudist = 0;
  for(neuron& n: net) {
    int qty = 0;
    double total = 0;
    forCellEx(c2, n.where) {
      neuron *n2 = getNeuron(c2);
      if(!n2) continue;
      qty++;
      total += sqrt(vnorm(n.net, n2->net));
      }
    n.udist = total / qty;
    maxudist = max(maxudist, n.udist);
    }
    
  if(!noshow) distribute_neurons();
  
  coloring();
  }

// traditionally Gaussian blur is used in the Kohonen algoritm
// but it does not seem to make much sense in hyperbolic geometry
// especially wrapped one.
// GAUSSIAN==1: use the Gaussian blur, on celldistance
// GAUSSIAN==2: use the Gaussian blur, on true distance
// GAUSSIAN==0: simulate the dispersion on our network

int gaussian = 0;

double mydistance(cell *c1, cell *c2) {
  if(gaussian == 2) return hdist(tC0(ggmatrix(c1)), tC0(ggmatrix(c2)));
  else return celldistance(c1, c2);
  }

struct cellcrawler {

  struct cellcrawlerdata {
    cellwalker orig;
    int from, spin, dist;
    cellwalker target;
    cellcrawlerdata(const cellwalker& o, int fr, int sp) : orig(o), from(fr), spin(sp) {}
    };
  
  vector<cellcrawlerdata> data;
  
  void store(const cellwalker& o, int from, int spin, manual_celllister& cl) {
    if(!cl.add(o.at)) return;
    data.emplace_back(o, from, spin);
    }
  
  void build(const cellwalker& start) {
    data.clear();
    manual_celllister cl;
    store(start, 0, 0, cl);
    for(int i=0; i<isize(data); i++) {
      cellwalker cw0 = data[i].orig;
      for(int j=0; j<cw0.at->type; j++) {
        cellwalker cw = cw0 + j + wstep;
        if(!getNeuron(cw.at)) continue;
        store(cw, i, j, cl);
        }
      }
    if(gaussian) for(cellcrawlerdata& s: data)
      s.dist = mydistance(s.orig.at, start.at);
    }
  
  void sprawl(const cellwalker& start) {
    data[0].target = start;
    
    for(int i=1; i<isize(data); i++) {
      cellcrawlerdata& s = data[i];
      s.target = data[s.from].target;
      if(!s.target.at) continue;
      s.target += s.spin;
      if(!s.target.peek()) s.target.at = NULL;
      else s.target += wstep;
      }
    }

  vector<vector<ld>> dispersion;
  };

double dispersion_end_at = 1.5;

double dispersion_precision = .0001;
int dispersion_each = 1;

int dispersion_count;

void buildcellcrawler(cell *c, cellcrawler& cr, int dir) {
  cr.build(cellwalker(c,dir));

  if(!gaussian) {
    vector<ld> curtemp;
    vector<ld> newtemp;
    vector<int> qty;
    vector<pair<ld*, ld*> > pairs;
    int N = isize(net);
    
    curtemp.resize(N, 0);
    newtemp.resize(N, 0);
    qty.resize(N, 0);
  
    for(int i=0; i<N; i++) 
    forCellEx(c2, net[i].where) {
      neuron *nj = getNeuron(c2);
      if(nj) {
        pairs.emplace_back(&curtemp[i], &newtemp[neuronId(*nj)]);
        qty[i]++;
        }
      }
    
    curtemp[neuronId(*getNeuron(c))] = 1;
  
    ld vmin = 0, vmax = 1;
    int iter;
    
    auto &d = cr.dispersion;
    
    d.clear();
  
    printf("Building dispersion...\n");
    
    for(iter=0; dispersion_count ? true : vmax > vmin * dispersion_end_at; iter++) {
      if(iter % dispersion_each == 0) {
        d.emplace_back(N);
        auto& dispvec = d.back();
        for(int i=0; i<N; i++) dispvec[i] = curtemp[neuronId(*getNeuron(cr.data[i].orig.at))] / vmax;
        if(isize(d) == dispersion_count) break;
        }
      double df = dispersion_precision * (iter+1);
      double df0 = df / ceil(df);
      for(int i=0; i<df; i++) {
        for(auto& p: pairs) 
          *p.second += *p.first;
        for(int i=0; i<N; i++) {
          curtemp[i] += (newtemp[i] / qty[i] - curtemp[i]) * df0;
          newtemp[i] = 0;
          }
        }
      vmin = vmax = curtemp[0];
      for(int i=0; i<N; i++) 
        if(curtemp[i] < vmin) vmin = curtemp[i];
        else if(curtemp[i] > vmax) vmax = curtemp[i];
      }
  
    dispersion_count = isize(d);
    printf("Dispersion count = %d\n", dispersion_count);
    }
  }

map<int, cellcrawler> scc;

pair<int, int> get_cellcrawler_id(cell *c) {
  if(among(geometry, gZebraQuotient, gMinimal, gField435, gField534) || (euwrap && !fulltorus) || IRREGULAR || (GDIM == 3 && sphere)) {
    // Zebra Quotient does exhibit some symmetries,
    // but these are so small anyway that it is safer to just build
    // a crawler for every neuron
    return make_pair(neuronId(*getNeuronSlow(c)), 0);
    // not yet implemented for cylinder
    }
  if(fulltorus && (torusconfig::tmflags() & torusconfig::TF_KLEIN))
    return make_pair(cell_to_pair(c).second * 2 + ctof(c), 0);
  int id = 0, dir = 0;
#if CAP_GP
  if(GOLDBERG) {
    gp::local_info li = gp::get_local_info(c);
    id = (li.relative.first & 15) + (li.relative.second & 15) * 16 + gmod(li.total_dir, S6) * 256;
    // ld = li.last_dir;
    }
#else
  if(0) ;
#endif
  else {
    id = c->type == S7;
    // if(id == 0) ld = c->c.spin(0);
    }
  /* if(geometry == gZebraQuotient) {
    id = 8*id + ld;
    id = 64 * id + c->master->zebraval;
    return make_pair(id, 0);
    } */
  return make_pair(id, dir);
  }

/* unit test: do the crawlers work correctly? */

bool verify_crawler(cellcrawler& cc, cellwalker cw) {
  cc.sprawl(cw);
  for(auto& d: cc.data) if(celldistance(cw.at, d.target.at) != d.dist) 
    return false;
  vector<int> cellcounter(cells, 0);
  for(auto& d: cc.data) cellcounter[d.target.at->landparam]++;
  for(int i=0; i<cells; i++) if(cellcounter[i] != 1) return false;
  return true;
  }

void verify_crawlers() {

  setindex(false);
  gaussian = 1;
  auto& allcells = currentmap->allcells();
  cells = isize(allcells);
  net.resize(cells);
  for(int i=0; i<cells; i++) net[i].where = allcells[i];
  setindex(true);
  map<int, cellcrawler> allcrawlers;
  
  int uniq = 0, failures = 0;
  
  printf("Verifying crawlers...\n");  
  for(cell *c: allcells) {
    auto id = get_cellcrawler_id(c);
    if(allcrawlers.count(id.first)) {
      bool b = verify_crawler(allcrawlers[id.first], cellwalker(c, id.second));
      if(!b) {
        printf("cell %p: type = %d id = %d dir = %d / earlier crawler failed\n", c, c->type, id.first, id.second);
        failures++;
        }
      }
    else {
      for(int i=0; i<c->type; i++)
      for(auto& cc: allcrawlers) if(verify_crawler(cc.second, cellwalker(c, i))) {
        printf("cell %p: type = %d id = %d dir = %d / also works id %d in direction %d\n", c, c->type, id.first, id.second, cc.first, i);
        uniq--;
        goto breakcheck;
        }
      breakcheck:
      cellcrawler cr;
      cr.build(cellwalker(c, id.second));
      allcrawlers[id.first] = move(cr);
      uniq++;
      }
    }
  printf("Crawlers constructed: %d (%d unique, %d failures)\n", isize(allcrawlers), uniq, failures);
  setindex(false);
  if(failures) exit(1);
  }
  
bool finished() { return t == 0; }

int krad;

double ttpower = 1;

void step() {

  if(t == 0) return;
  sominit(2);
  
  double tt = (t-1.) / tmax;
  tt = pow(tt, ttpower);

  double sigma = maxdist * tt;
  int dispid = int(dispersion_count * tt);

  if(qpct) {
    int pct = (int) ((qpct * (t+.0)) / tmax);
    if(pct != lpct) {
      printf("pct %d lpct %d\n", pct, lpct);
      lpct = pct;
      analyze();
      
      if(gaussian)
        printf("t = %6d/%6d %3d%% sigma=%10.7lf maxudist=%10.7lf\n", t, tmax, pct, sigma, maxudist);
      else
        printf("t = %6d/%6d %3d%% dispid=%5d maxudist=%10.7lf\n", t, tmax, pct, dispid, maxudist);
      }
    }
  int id = hrand(samples);
  neuron& n = winner(id);
  whowon.resize(samples);
  whowon[id] = &n;
    
  /* 
  for(neuron& n2: net) {
    int d = celldistance(n.where, n2.where);
    double nu = learning_factor; 
//  nu *= exp(-t*(double)maxdist/perdist);
//  nu *= exp(-t/t2);
    nu *= exp(-sqr(d/sigma));
    for(int k=0; k<columns; k++)
      n2.net[k] += nu * (irisdata[id][k] - n2.net[k]);
    } */
    
  auto cid = get_cellcrawler_id(n.where);
  cellcrawler& s = scc[cid.first];
  s.sprawl(cellwalker(n.where, cid.second));

  vector<double> fake(1,1);
  auto it = gaussian ? fake.begin() : s.dispersion[dispid].begin();

  for(auto& sd: s.data) {
    neuron *n2 = getNeuron(sd.target.at);
    if(!n2) continue;
    double nu = learning_factor;
    
    if(gaussian)
      nu *= exp(-sqr(sd.dist/sigma));
    else
      nu *= *(it++);

    for(int k=0; k<columns; k++)
      n2->net[k] += nu * (data[id].val[k] - n2->net[k]);
    }
  
  t--;
  if(t == 0) analyze();
  }

int initdiv = 1;

int inited = 0;

void uninit(int initto) {
  if(inited > initto) inited = initto;
  }

vector<double> bdiffs;
vector<unsigned short> bids;
vector<double> bdiffn;

int showsample(int id) {
  if(sample_vdata_id.count(id))
    return sample_vdata_id[id];
  if(bids.size()) {
    if(net[bids[id]].drawn_samples >= net[bids[id]].max_group_here) {
      ld bdist = 1e18;
      int whichid = -1;
      for(auto p: sample_vdata_id) {
        int s = p.first;
        if(bids[s] == bids[id]) {
          ld cdist = vnorm(data[s].val, data[id].val);
          if(cdist < bdist) bdist = cdist, whichid = p.second;
          }
        }
      return whichid;
      }
    net[bids[id]].drawn_samples++;
    }
  int i = vdata.size();
  sample_vdata_id[id] = i;
  vdata.emplace_back();
  auto& v = vdata.back();
  v.name = data[id].name;
  v.cp = dftcolor;
  createViz(i, bids.size() ? net[bids[id]].where : cwt.at, Id);
  v.m->store();
  return i;
  }

int showsample(string s) {
  if(s == "") return -1;
  int ret = -1;
  for(int i=0; i<samples; i++) {
    if(s[0] != '*' && data[i].name == s)
      ret = showsample(i);
    if(s[0] == '*' && data[i].name.find(s.substr(1)) != string::npos)
      ret = showsample(i);
    }
  return ret;
  }

void showbestsamples() {
  vector<int> samplesbak;
  for(auto& n: net) 
    if(n.allsamples)
      showsample(n.bestsample);
  analyze();
  }

int kohrestrict = 1000000;
  
void sominit(int initto, bool load_compressed) {

  if(inited < 1 && initto >= 1) {
    inited = 1;
    if(!samples && !load_compressed) {
      fprintf(stderr, "Error: SOM without samples\n");
      exit(1);
      }
    
    init(); kind = kKohonen;
    
    printf("Initializing SOM (1)\n");
  
    vector<cell*> allcells;
    
    if(krad) {
      celllister cl(cwt.at, krad, 1000000, NULL);
      allcells = cl.lst;
      }
    else allcells = currentmap->allcells();
        
    if(isize(allcells) > kohrestrict) {
      map<cell*, int> clindex;
      for(int i=0; i<isize(allcells); i++) clindex[allcells[i]] = i;
      sort(allcells.begin(), allcells.end(), [&clindex] (cell *c1, cell *c2) { 
        return make_pair(hdist0(tC0(ggmatrix(c1))), clindex[c1]) < 
               make_pair(hdist0(tC0(ggmatrix(c2))), clindex[c2]); 
        });
      int at = kohrestrict;
      ld dist = hdist0(tC0(ggmatrix(allcells[at-1])));
      while(at < isize(allcells) && hdist0(tC0(ggmatrix(allcells[at]))) < dist + 1e-6) at++;
      int at1 = kohrestrict;
      while(at1 > 0 && hdist0(tC0(ggmatrix(allcells[at1-1]))) > dist - 1e-6) at1--;
      printf("Cells numbered [%d,%d) are in the same distance\n", at1, at);
      allcells.resize(kohrestrict);
      }
  
    cells = isize(allcells);
    net.resize(cells);
    for(int i=0; i<cells; i++) net[i].where = allcells[i], allcells[i]->landparam = i;
    for(int i=0; i<cells; i++) {
      net[i].where->land = laCanvas;
      alloc(net[i].net);
  
      if(samples)
      for(int k=0; k<columns; k++)
      for(int z=0; z<initdiv; z++)
        net[i].net[k] += data[hrand(samples)].val[k] / initdiv;
      }
      
    for(neuron& n: net) for(int d=BARLEV; d>=7; d--) setdist(n.where, d, NULL);
  
    printf("samples = %d (%d) cells = %d\n", samples, isize(sample_vdata_id), cells);

    if(!noshow) for(int s: samples_to_show) {
      int vdid = isize(vdata);
      sample_vdata_id[s] = vdid;
      vdata.emplace_back();
      auto &vd = vdata.back();
      vd.name = data[s].name;
      vd.cp = dftcolor;
      createViz(vdid, cwt.at, Id);
      storeall(vdid);
      }
    
    samples_to_show.clear();

    analyze();
    }
  
  if(inited < 2 && initto >= 2) {
    inited = 2;

    DEBB(DF_LOG, ("Initializing SOM (2)"));

    if(gaussian) {
      DEBB(DF_LOG, ("dist = ", fts(mydistance(net[0].where, net[1].where))));
      cell *c1 = net[cells/2].where;
      vector<double> mapdist;
      for(neuron &n2: net) mapdist.push_back(mydistance(c1,n2.where));
      sort(mapdist.begin(), mapdist.end());
      maxdist = mapdist[isize(mapdist)*5/6] * distmul;
      DEBB(DF_LOG, ("maxdist = ", fts(maxdist)));
      }
      
    dispersion_count = 0;  
    
    scc.clear();
    for(int i=0; i<cells; i++) {
      cell *c = net[i].where;
      auto cid = get_cellcrawler_id(c);
      if(!scc.count(cid.first)) {
        DEBB(DF_LOG, ("Building cellcrawler id = ", itsh(cid.first)));
        buildcellcrawler(c, scc[cid.first], cid.second);
        }
      }
  
    lpct = -46130;
    }
  }

void describe_cell(cell *c) {
  if(cmode & sm::HELP) return;
  if(kind != kKohonen) return;
  neuron *n = getNeuronSlow(c);
  if(!n) return;
  string h;
  h += "cell number: " + its(neuronId(*n)) + " (" + its(n->allsamples) + ")\n";
  h += "parameters:"; for(int k=0; k<columns; k++) h += " " + fts(n->net[k]); 
  h += ", u-matrix = " + fts(n->udist);
  h += "\n";
  vector<pair<double, int>> v;
  for(int s=0; s<samples; s++) if(whowon[s] == n) v.emplace_back(vnorm(n->net, data[s].val), s);
  for(int i=1; i<isize(v); i++) swap(v[i], v[rand() % (i+1)]);
  sort(v.begin(), v.end(), [] (pair<double,int> a, pair<double,int> b) { return a.first < b.first; });
  
  for(int i=0; i<isize(v) && i<20; i++) {
    int s = v[i].second;
    h += "sample "+its(s)+":"; 
    for(int k=0; k<columns; k++) h += " " + fts(data[s].val[k]); 
    h += " "; h += data[s].name; h += "\n";
    }
  appendHelp(h);
  }

namespace levelline {

  struct levelline {
    int column, qty;
    color_t color;
    vector<double> values;
    bool modified;
    };
  
  vector<levelline> levellines;
  
  bool on;
  
  void create() {
    int xlalpha = part(default_edgetype.color, 0);
    for(int i=0; i<columns; i++) {
      levellines.emplace_back();
      levelline& lv = levellines.back();
      lv.column = i;    
      lv.color = ((hrandpos() & 0xFFFFFF) << 8) | xlalpha;
      lv.qty = 0;
      }
    }
  
  void build() {
    if(levellines.size() == 0) create();
    on = false;
    for(auto& lv: levellines) {
      if(!lv.qty) { lv.values.clear(); continue; }
      on = true;
      if(!lv.modified) continue;
      lv.modified = false;
      vector<double> sample;
      for(int j=0; j<=1024; j++) sample.push_back(data[hrand(samples)].val[lv.column]);
      sort(sample.begin(), sample.end());
      lv.values.clear();
      lv.values.push_back(-1e10);
      for(int j=0; j<=1024; j+=1024 >> (lv.qty)) lv.values.push_back(sample[j]);
      lv.values.push_back(1e10);
      }
    }
  
  void draw() {
    if(!on) return;
    for(auto& g: gmatrix) {
      cell *c1 = g.first;
      transmatrix T = g.second;
      neuron *n1 = getNeuron(c1);
      if(!n1) continue;
      for(int i=0; i<c1->type; i++) {
        cell *c2 = c1->move(i);
        if(!c2) continue;
        cell *c3 = c1->modmove(i-1);
        if(!c3) continue;
  
        if(!gmatrix.count(c2)) continue;
        if(!gmatrix.count(c3)) continue;
        double d2 = hdist(tC0(T), tC0(gmatrix[c2]));
        double d3 = hdist(tC0(T), tC0(gmatrix[c3]));
        
        neuron *n2 = getNeuron(c2);
        if(!n2) continue;
        neuron *n3 = getNeuron(c3);
        if(!n3) continue;
              
        for(auto& l: levellines) {
          auto val1 = n1->net[l.column];
          auto val2 = n2->net[l.column];
          auto val3 = n3->net[l.column];
          auto v1 = lower_bound(l.values.begin(), l.values.end(), val1);
          auto v2 = lower_bound(l.values.begin(), l.values.end(), val2);
          auto v3 = lower_bound(l.values.begin(), l.values.end(), val3);
          auto draw = [&] () {
            auto vmid = *v1;
            queueline(
              (T * ddspin(c1,i) * xpush0(d2 * (vmid-val1) / (val2-val1))), 
              (T * ddspin(c1,i-1) * xpush0(d3 * (vmid-val1) / (val3-val1))), 
              l.color, vid.linequality);
            };
          while(v1 < v2 && v1 < v3) {
            draw();
            v1++;
            }
          while(v1 > v2 && v1 > v3) {
            v1--;
            draw();
            }
          }
        }      
      }
    setindex(false);
    }

  void show() {
    if(levellines.size() == 0) create();
    gamescreen(0);
    cmode = sm::SIDE | sm::MAYDARK;
    dialog::init("level lines");
    char nx = 'a';
    for(auto &l : levellines) {
      dialog::addSelItem(colnames[l.column], its(l.qty), nx++);
      dialog::lastItem().colorv = l.color >> 8;
      }
    dialog::addItem("exit menu", '0');
    dialog::addItem("shift+letter to change color", 0);
    dialog::display();
    keyhandler = [] (int sym, int uni) {
      dialog::handleNavigation(sym, uni);
      if(uni >= 'a' && uni - 'a' + isize(levellines)) {
        auto& l = levellines[uni - 'a'];
        dialog::editNumber(l.qty, 0, 10, 1, 0, colnames[l.column], 
          XLAT("Controls the number of level lines."));
        dialog::reaction = [&l] () {
          l.modified = true;
          build();
          };
        }
      else if(uni >= 'A' && uni - 'A' + isize(levellines)) {
        auto& l = levellines[uni - 'A'];
        dialog::openColorDialog(l.color, NULL);
        dialog::dialogflags |= sm::MAYDARK | sm::SIDE;
        }
      else if(doexiton(sym, uni)) popScreen();
      };
    }

  
  }

void ksave(const string& fname) {
  sominit(1);
  FILE *f = fopen(fname.c_str(), "wt");
  if(!f) {
    fprintf(stderr, "Could not save the network\n");
    return;
    }
  fprintf(f, "%d %d\n", cells, t);
  for(neuron& n: net) {
    for(int k=0; k<columns; k++)
      fprintf(f, "%.9lf ", n.net[k]);
    fprintf(f, "\n");
    }
  fclose(f);
  }

void kload(const string& fname) {
  sominit(1);
  int xcells;
  fhstream f(fname.c_str(), "rt");
  if(!f.f) {
    fprintf(stderr, "Could not load the network: %s\n", fname.c_str());
    return;
    }
  if(!scan(f, xcells, t)) {
    fprintf(stderr, "Bad network format: %s\n", fname.c_str());
    return;
    }
  printf("Loading the network %s...\n", fname.c_str());
  if(xcells != cells) {
    fprintf(stderr, "Error: bad number of cells (x=%d c=%d)\n", xcells, cells);
    exit(1);
    }
  for(neuron& n: net) {
    for(int k=0; k<columns; k++) if(!scan(f, n.net[k])) return;
    }
  analyze();
  }

void ksavew(const string& fname) {
  sominit(1);
  FILE *f = fopen(fname.c_str(), "wt");
  if(!f) {
    fprintf(stderr, "Could not save the weights: %s\n", fname.c_str());
    return;
    }
  printf("Saving the network to %s...\n", fname.c_str());
  for(int i=0; i<columns; i++)
    fprintf(f, "%s=%.9lf\n", colnames[i].c_str(), weights[i]);
  fclose(f);
  }

void kloadw(const string& fname) {
  sominit(1);
  FILE *f = fopen(fname.c_str(), "rt");
  if(!f) {
    fprintf(stderr, "Could not load the weights\n");
    return;
    }
  for(int i=0; i<columns; i++) {
    string s1, s2;
    char kind = 0;
    while(true) {
      int c = fgetc(f);
      if(c == 10 || c == 13 || c == -1) {
        if(s1 == "" && !kind && c != -1) continue;
        if(s1 != "") colnames[i] = s1;
        if(kind == '=') weights[i] = atof(s2.c_str());
        if(kind == '*') weights[i] *= atof(s2.c_str());
        if(kind == '/') weights[i] /= atof(s2.c_str());
        if(c == -1) break;
        goto nexti;
        }
      else if(c == '=' || c == '/' || c == '*') kind = c;
      else (kind?s2:s1) += c;
      }
    nexti: ;
    }
  fclose(f);
  analyze();
  }


unsigned lastprogress;
void progress(string s) {
  if(SDL_GetTicks() >= lastprogress + (noGUI ? 500 : 100)) {
    if(noGUI)
      printf("%s\n", s.c_str());
    else {
      clearMessages();
      addMessage(s);
      mainloopiter();
      }
    lastprogress = SDL_GetTicks();
    } 
  }

template<class T> void save_raw(string fname, const vector<T>& v) {
  FILE *f = fopen(fname.c_str(), "wb");
  fwrite(&v[0], sizeof(v[0]), v.size(), f);
  fclose(f);
  }

template<class T> void load_raw(string fname, vector<T>& v) {
  FILE *f = fopen(fname.c_str(), "rb");
  if(!f) { fprintf(stderr, "file does not exist: %s\n", fname.c_str()); exit(1); }
  fseek(f, 0, SEEK_END);
  auto s = ftell(f);
  rewind(f);
  v.resize(s / sizeof(v[0]));
  hr::ignore(fread(&v[0], sizeof(v[0]), v.size(), f));
  fclose(f);
  }

bool groupsizes_known = false;

void do_classify() {
  sominit(1);
  if(bids.empty()) {
    printf("Classifying...\n");
    bids.resize(samples, 0);
    bdiffs.resize(samples, 1e20);
    for(int s=0; s<samples; s++) {
      for(int n=0; n<cells; n++) {
        double diff = vnorm(net[n].net, data[s].val);
        if(diff < bdiffs[s]) bdiffs[s] = diff, bids[s] = n, whowon[s] = &net[n];
        }
      if(!(s % 128))
        progress("Classifying: " + its(s) + "/" + its(samples));
      }
    }
  if(bdiffs.empty()) {
    printf("Computing distances...\n");
    bdiffs.resize(samples, 1e20);
    for(int s=0; s<samples; s++) 
      bdiffs[s] = vnorm(net[bids[s]].net, data[s].val);
    }
  if(bdiffn.empty()) {
    printf("Finding samples...\n");
    bdiffn.resize(cells, 1e20);
    for(int s=0; s<samples; s++) {
      int n = bids[s];
      double diff = bdiffs[s];
      if(diff < bdiffn[n]) bdiffn[n] = diff, net[n].bestsample = s;
      }  
    }
  whowon.resize(samples);  
  for(int i=0; i<samples; i++) whowon[i] = &net[bids[i]];
  for(neuron& n: net) n.allsamples = 0;
  for(int sn: bids) net[sn].allsamples++;
  
  if(!groupsizes_known) {
    groupsizes_known = true;
    
    vector<int> neurons_to_sort;
    for(int i=0; i<cells; i++) neurons_to_sort.push_back(i);
    sort(neurons_to_sort.begin(), neurons_to_sort.end(), [] (int i, int j) { return net[i].allsamples < net[j].allsamples; });
    int last = 0;
    int lastfirst = 0, lastlast = 0;
    for(int i=0; i<cells; i++) {
      int ngroup = min_group + ((max_group - min_group) * i + (cells/2)) / (cells-1);
      int as = net[neurons_to_sort[i]].allsamples;
      if(ngroup != last) {  
        if(last) printf("%d: %d - %d\n", last, lastfirst, lastlast);
        last = ngroup; lastfirst = as;
        }
      net[neurons_to_sort[i]].max_group_here = ngroup;
      lastlast = as;
      }
    if(last) printf("%d: %d - %d\n", last, lastfirst, lastlast);
    }
  
  coloring();
  }

void fillgroups() {
  do_classify();
  vector<int> samples_to_sort;
  for(int i=0; i<samples; i++) samples_to_sort.push_back(i);
  hrandom_shuffle(&samples_to_sort[0], samples);
  for(int i=0; i<samples; i++) if(net[bids[i]].drawn_samples < net[bids[i]].max_group_here)
    showsample(i);
  distribute_neurons();
  }

void kclassify(const string& fname_classify) {

  do_classify();

  if(fname_classify != "") {  
    printf("Listing classification to %s...\n", fname_classify.c_str());
    FILE *f = fopen(fname_classify.c_str(), "wt");  
    if(!f) {
      printf("Failed to open file\n");
      }
    else {
      for(int s=0; s<samples; s++)
        fprintf(f, "%s;%d\n", data[s].name.c_str(), bids[s]);
      fclose(f);
      }
    }
  }

void kclassify_save_raw(const string& fname_classify) {
  printf("Saving raw classify to %s...\n", fname_classify.c_str());
  do_classify();
  save_raw(fname_classify, bids);
  }

void kclassify_load_raw(const string& fname_classify) {
  printf("Loading raw classify from %s...\n", fname_classify.c_str());
  load_raw(fname_classify, bids);
  do_classify();
  }

void load_edges(const string& fname_edges, string edgename, int pick = 0) {
  do_classify();
  auto t = add_edgetype(edgename);
  vector<pair<int, int>> edgedata;
  load_raw(fname_edges, edgedata);
  int N = isize(edgedata);
  if(pick > 0 && pick < N) {
    for(int i=1; i<N; i++) swap(edgedata[i], edgedata[hrand(i+1)]);
    edgedata.resize(N = pick);    
    }
  t->visible_from = 1. / N;
  vector<pair<int, int>> edgedata2;
  for(auto p: edgedata)
    edgedata2.emplace_back(showsample(p.first), showsample(p.second));
  distribute_neurons();
  int i = 0;
  for(auto p: edgedata2)
    if(p.first >= 0 && p.second >= 0)
      addedge(p.first, p.second, 1 / (i+++.5), true, t);
    else {
      printf("error reading graph\n");
      exit(1);
      }
  }

void random_edges(int q) {
  auto t = add_edgetype("random");
  vector<int> ssamp;
  for(auto p: sample_vdata_id) ssamp.push_back(p.second);
  for(int i=0; i<q; i++)
    addedge(ssamp[hrand(isize(ssamp))], ssamp[hrand(isize(ssamp))], 0, true, t);
  }

void klistsamples(const string& fname_samples, bool best, bool colorformat) {
  if(fname_samples != "") { 
    printf("Listing samples...\n");
    FILE *f = fopen(fname_samples.c_str(), "wt");  
    if(!f) {
      printf("Failed to open file\n");
      }
    else {
      auto klistsample = [f, colorformat] (int id, int neu) {
        if(colorformat) {
          fprintf(f, "%s;+#%d\n", data[id].name.c_str(), neu);
          }
        else {
          for(int k=0; k<columns; k++)
            fprintf(f, "%.4lf ", data[id].val[k]);
          fprintf(f, "!%s\n", data[id].name.c_str());
          }
        };
      if(!colorformat) fprintf(f, "%d\n", columns);
      if(best)
        for(int n=0; n<cells; n++) {
          if(!net[n].allsamples && !net[n].drawn_samples) { if(!colorformat) fprintf(f, "\n"); continue; }
          klistsample(net[n].bestsample, n);
          }
      else
        for(auto p: sample_vdata_id) {
          int id = p.first;
          klistsample(id, neuronId(*(whowon[id])));
          }
      fclose(f);
      }
    }
  }

void neurondisttable(const string &name) {
  FILE *f = fopen(name.c_str(), "wt");
  if(!f) {
    printf("Could not open file: %s\n", name.c_str());
    return;
    }
  int neurons = isize(net);
  fprintf(f, "%d\n", neurons);
  for(int i=0; i<neurons; i++) {
    for(int j=0; j<neurons; j++) fprintf(f, "%3d", celldistance(net[i].where, net[j].where));
    // todo: build the table correctly for gaussian=2
    fprintf(f, "\n");
    }
  fclose(f);
  }

void steps() {
  if(!kohonen::finished()) {
    unsigned int t = SDL_GetTicks();
    while(SDL_GetTicks() < t+20) kohonen::step();
    setindex(false);
    }
  }

void showMenu() {
  string parts[3] = {"red", "green", "blue"};
  for(int i=0; i<3; i++) {
    string c;
    if(whattodraw[i] == -1) c = "u-matrix";
    else if(whattodraw[i] == -2) c = "u-matrix reversed";
    else if(whattodraw[i] == -3) c = "distance from marked ('m')";
    else if(whattodraw[i] == -4) c = "number of samples";
    else if(whattodraw[i] == -5) c = "best sample's color";
    else if(whattodraw[i] == -6) c = "sample names to colors";
    else c = colnames[whattodraw[i]];
    dialog::addSelItem(XLAT("coloring (%1)", parts[i]), c, '1'+i);
    }
  dialog::addItem("coloring (all)", '0');
  dialog::addItem("level lines", '4');
  }

bool handleMenu(int sym, int uni) {
  if(uni >= '1' && uni <= '3') {
    int i = uni - '1';
    whattodraw[i]++;
    if(whattodraw[i] == columns) whattodraw[i] = -5;
    coloring();
    return true;
    }
  if(uni == '0') {
    for(char x: {'1','2','3'}) handleMenu(x, x);
    return true;
    }
  if(uni == '4') {
    pushScreen(levelline::show);
    return true;
    }
  return false;
  }

void save_compressed(string name) {
  // save everything in compressed form
  fhstream f(name, "wb");
  if(!f.f) {
    printf("failed to open for save_compressed: %s\n", name.c_str());
    return;
    }
  // save columns
  f.write(columns);
  for(int i=0; i<columns; i++) f.write(colnames[i]);
  for(int i=0; i<columns; i++) hwrite_raw<float>(f, weights[i]);
  // save neurons
  f.write<int>(isize(net));
  for(int i=0; i<isize(net); i++)
    for(int j=0; j<columns; j++) hwrite_raw<float>(f, net[i].net[j]);
  // save shown samples
  map<int, int> saved_id;
  f.write<int>(isize(sample_vdata_id));
  int index = 0;
  for(auto p: sample_vdata_id) {
    int i = p.first;
    for(int j=0; j<columns; j++) hwrite_raw<float>(f, data[i].val[j]);
    f.write(data[i].name);
    int id = p.second;
    saved_id[id] = index++;
    auto& vd = vdata[id];
    struct colorpair_old { color_t color1, color2; char shade; } cpo;
    cpo.color1 = vd.cp.color1;
    cpo.color2 = vd.cp.color2;
    cpo.shade = vd.cp.shade;
    hwrite_raw(f, cpo);
    }
  // save edge types
  f.write<int>(isize(edgetypes));
  for(auto&et: edgetypes) {
    f.write(et->name);
    hwrite_raw<float>(f, et->visible_from);
    f.write(et->color);
    }
  // save edge infos
  f.write<int>(isize(edgeinfos));
  for(auto& ei: edgeinfos) {
    for(int x=0; x<isize(edgetypes); x++)
      if(ei->type == &*edgetypes[x]) f.write_char(x);
    f.write(saved_id[ei->i]);
    f.write(saved_id[ei->j]);
    hwrite_raw<float>(f, ei->weight);
    }
  }

void load_compressed(string name) {
  // save everything in compressed form
  fhstream f(name, "rb");
  if(!f.f) {
    printf("failed to open for load_compressed: %s\n", name.c_str());
    return;
    }
  // load columns
  f.read(columns);
  colnames.resize(columns);
  for(int i=0; i<columns; i++) f.read(colnames[i]);
  alloc(weights);
  for(int i=0; i<columns; i++) weights[i] = f.get_raw<float>();
  samples = 0; sominit(1, true);
  // load neurons
  int N = f.get<int>();
  if(cells != N) {
    fprintf(stderr, "Error: bad number of cells (N=%d c=%d)\n", N, cells);
    exit(1);
    }
  for(neuron& n: net)
    for(int k=0; k<columns; k++) 
      n.net[k] = f.get_raw<float>();
  // load data
  samples = f.get<int>();
  data.resize(samples);
  int id = 0;
  for(auto& d: data) {
    alloc(d.val);
    for(int j=0; j<columns; j++)
      d.val[j] = f.get_raw<float>();
    f.read(d.name);
    int i = vdata.size();
    sample_vdata_id[id] = i;
    vdata.emplace_back();
    auto& v = vdata.back();
    v.name = data[i].name;
    struct colorpair_old { color_t color1, color2; char shade; } cpo;
    hread_raw(f, cpo);
    v.cp.color1 = cpo.color1;
    v.cp.color2 = cpo.color2;
    v.cp.shade = cpo.shade;
    createViz(i, cwt.at, Id);
    v.m->store();
    id++;
    }
  // load edge types
  int qet = f.get<int>();
  for(int i=0; i<qet; i++) {
    auto et = add_edgetype(f.get<string>());
    et->visible_from = f.get_raw<float>();
    f.read(et->color);
    }
  // load edge infos
  int qei = f.get<int>();
  for(int i=0; i<qei; i++) {
    auto t = edgetypes[f.read_char()];
    int ei = f.get<int>();
    int ej = f.get<int>();
    float w = f.get_raw<float>();
    addedge(ei, ej, w, true, &*t);
    }
  analyze();
  }

#if CAP_COMMANDLINE
int readArgs() {
  using namespace arg;

  // #1: load the samples
  
  if(argis("-som")) {
    PHASE(3);
    shift(); kohonen::loadsamples(args());
    }

  // #2: set parameters

  else if(argis("-somkrad")) {
    gaussian = 0; uninit(0);
    }
  else if(argis("-somskrad")) {
    shift(); krad = argi();
    }
  else if(argis("-somsim")) {
    gaussian = 0; uninit(1);
    }
  else if(argis("-somcgauss")) {
    gaussian = 1; uninit(1);
    }
  else if(argis("-somggauss")) {
    gaussian = 2; uninit(1);
    }
  else if(argis("-sompct")) {
    shift(); qpct = argi();
    }
  else if(argis("-sompower")) {
    shift_arg_formula(ttpower);
    }
  else if(argis("-somparam")) {
    shift_arg_formula((gaussian ? distmul : dispersion_end_at));
    if(dispersion_end_at <= 1) {
      fprintf(stderr, "Dispersion parameter illegal\n");
      dispersion_end_at = 1.5;
      }
    uninit(1);
    }
  else if(argis("-sominitdiv")) {
    shift(); initdiv = argi(); uninit(0);
    }
  else if(argis("-somtmax")) {
    shift(); t = (t*1./tmax) * argi();
    tmax = argi();
    }
  else if(argis("-somlearn")) {
    // this one can be changed at any moment
    shift_arg_formula(learning_factor);
    }

  else if(argis("-somrun")) {
    t = tmax; sominit(1);
    }

  // #3: load the neuron data (usually without #2)
  else if(argis("-somload")) {
    PHASE(3);
    shift(); kohonen::kload(args());
    }

  // #4: run, stop etc.
  else if(argis("-somrunto")) {
    int i = argi();
    shift(); while(t > i) {
      if(t % 128 == 0) progress("Steps left: " + its(t));
      kohonen::step();
      }
    }
  else if(argis("-somstop")) {
    t = 0;
    }
  else if(argis("-somnoshow")) {
    noshow = true;
    }
  else if(argis("-somfinish")) {
    while(!finished()) {
      kohonen::step();
      if(t % 128 == 0) progress("Steps left: " + its(t));
      }
    }

  // #5 save data, classify etc.
  else if(argis("-somsave")) {
    PHASE(3);
    shift(); kohonen::ksave(args());
    }
  else if(argis("-somsavew")) {
    PHASE(3);
    shift(); kohonen::ksavew(args());
    }
  else if(argis("-somloadw")) {
    PHASE(3);
    shift(); kohonen::kloadw(args());
    }
  else if(argis("-somclassify0")) {
    PHASE(3);
    shift(); kohonen::do_classify();
    }
  else if(argis("-somclassify")) {
    PHASE(3);
    shift(); kohonen::kclassify(args());
    }
  else if(argis("-somclassify-sr")) {
    PHASE(3);
    shift(); kohonen::kclassify_save_raw(args());
    }
  else if(argis("-somclassify-lr")) {
    PHASE(3);
    shift(); kohonen::kclassify_load_raw(args());
    }
  else if(argis("-somlistshown")) {
    PHASE(3);
    shift(); kohonen::klistsamples(args(), false, false);
    }
  else if(argis("-somlistbest")) {
    PHASE(3);
    shift(); kohonen::klistsamples(args(), true, false);
    }
  else if(argis("-somlistbestc")) {
    PHASE(3);
    shift(); kohonen::klistsamples(args(), true, true);
    }
  else if(argis("-somndist")) {
    PHASE(3);
    shift(); kohonen::neurondisttable(args());
    }
  else if(argis("-somshowbest")) {
    showbestsamples();
    }
  else if(argis("-somverify")) {
    start_game();
    verify_crawlers();
    }
  else if(argis("-somrestrict")) {
    shift(); kohrestrict = argi();
    }
  else if(argis("-som-maxgroup")) {
    shift(); max_group = argi();
    }
  else if(argis("-som-mingroup")) {
    shift(); min_group = argi();
    }
  else if(argis("-som-fillgroups")) {
    fillgroups();
    }
  else if(argis("-som-load-edges")) {
    shift(); string edgename = args();
    shift(); kohonen::load_edges(args(), edgename,  0);
    }
  else if(argis("-som-random-edges")) {
    shift();
    random_edges(argi());
    }
  else if(argis("-som-load-n-edges")) {
    shift(); string edgename = args();
    shift(); int n = argi();
    shift(); kohonen::load_edges(args(), edgename, n);
    }
  else if(argis("-less-edges")) {
    shift(); double d = argf();
    for(auto t: edgetypes) t->visible_from *= d;
    }
  else if(argis("-som-save-compressed")) {
    shift(); 
    save_compressed(args());
    }
  else if(argis("-som-load-compressed")) {
    shift(); 
    load_compressed(args());
    }

  else return 1;
  return 0;
  }

auto hooks = addHook(hooks_args, 100, readArgs);
#endif

auto hooks2 = addHook(hooks_frame, 50, levelline::draw)
  + addHook(hooks_mouseover, 100, describe_cell);

void clear() {
  printf("clearing Kohonen...\n");
  data.clear();
  sample_vdata_id.clear();
  colnames.clear();
  weights.clear();
  net.clear();
  whowon.clear();
  samples_to_show.clear();
  scc.clear();
  bdiffs.clear();
  bids.clear();
  bdiffn.clear();
  }

}}

namespace rogueviz {
  void mark(cell *c) {
    using namespace kohonen;
    if(kind == kKohonen && inited >= 1) {
      distfrom = getNeuronSlow(c);
      coloring();
      }
    }}

