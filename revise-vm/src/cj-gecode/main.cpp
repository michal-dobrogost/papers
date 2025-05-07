#include <gecode/int.hh>
#include <gecode/search.hh>
#include "cj/cj-csp.h"
#include "cj/cj-csp-io.h"
#include "io.h"

using namespace Gecode;
using std::vector;

class SpaceCJ : public Space {
protected:
  IntVarArray xs;
public:
  SpaceCJ(CjCsp& csp) : xs(*this, csp.vars.size) {

    // Domains
    std::vector<IntSet> domains;
    for (int iDom = 0; iDom < csp.domainsSize; ++iDom) {
      if (csp.domains[iDom].type == CjDomain::CJ_DOMAIN_VALUES) {
        assert(csp.domains[iDom].values.arity == -1); // TODO
        IntSet values(csp.domains[iDom].values.data, csp.domains[iDom].values.size);
        domains.push_back(values);
      }
      else {
        assert(false); // TODO: handle
      }
    }

    // Variables
    assert(csp.vars.arity == -1); // TODO: make hard error
    for (int iVar = 0; iVar < csp.vars.size; ++iVar) {
      xs[iVar] = IntVar(*this, domains[csp.vars.data[iVar]]);
    }

    // Constraint Definitions
    vector<TupleSet> constraintDefs;
    for (int iCDef = 0; iCDef < csp.constraintDefsSize; ++iCDef) {
      CjConstraintDef& cDef = csp.constraintDefs[iCDef];
      if (cDef.type == CjConstraintDef::CJ_CONSTRAINT_DEF_NO_GOODS) {
        TupleSet tuples(cDef.noGoods.arity);
        for (int iTuple = 0; iTuple < cDef.noGoods.size; ++iTuple) {
          tuples.add(
            vector<int>(
              &cDef.noGoods.data[iTuple * cDef.noGoods.arity],
              &cDef.noGoods.data[(iTuple + 1) * cDef.noGoods.arity]));
        }
        tuples.finalize();
        constraintDefs.push_back(tuples);
      }
      else {
        assert(false);
      }
    }

    // Constraints
    for (int iConstraint = 0; iConstraint < csp.constraintsSize; ++iConstraint) {
      CjConstraint& constraint = csp.constraints[iConstraint];
      CjConstraintDef& cDef = csp.constraintDefs[constraint.id];
      TupleSet& tuples = constraintDefs[constraint.id];
      if (cDef.type == CjConstraintDef::CJ_CONSTRAINT_DEF_NO_GOODS) {
        assert(constraint.vars.arity == -1);
        vector<IntVar> vars(constraint.vars.size);
        for (int iVar = 0; iVar < constraint.vars.size; ++iVar) {
          vars[iVar] = xs[constraint.vars.data[iVar]];
        }
        IntVarArgs varArgs(vars);
        const bool posneg = false;
        extensional(*this, varArgs, tuples, posneg);
      }
      else {
        assert(false);
      }
    }

    // post branching
    branch(*this, xs, INT_VAR_NONE(), INT_VAL_MIN());
  }
  // search support
  SpaceCJ(SpaceCJ& s) : Space(s) {
    xs.update(*this, s.xs);
  }
  virtual Space* copy(void) {
    return new SpaceCJ(*this);
  }
  // print solution
  void print(long long int timeMs, const Search::Statistics& stats) const {
    printf("{\"solution\": [");
    for (int iVar = 0; iVar < xs.size(); ++iVar) {
      if (iVar > 0) { printf(","); }
      printf("%d", xs[iVar].val());
    }
    printf("], \"timeMs\": %lld, \"nodes\": %lu, \"fails\": %lu, \"depthMax\": %lu}\n", timeMs, stats.node, stats.fail, stats.depth);
  }
};

void printUsage() {
  fprintf(stderr, "Usage: csp-solve-gecode --csp INSTANCE_FILENAME\n");
}

int main(int argc, char** argv) {
  fprintf(stderr, "gecode version: %d.%d.%d\n",
      GECODE_VERSION_NUMBER / 100000,
      (GECODE_VERSION_NUMBER / 100) % 1000,
      GECODE_VERSION_NUMBER % 100);

  int err = 0;
  if (argc != 3) {
    fprintf(stderr, "ERROR: number of command line parameters.\n\n");
    printUsage();
    return 1;
  }
  else if (strcmp(argv[1], "--csp") != 0) {
    fprintf(stderr, "ERROR: missing --csp flag.\n\n");
    printUsage();
    return 1;
  }
  char* cspInstanceFilename = argv[2];

  FILE* cspInstanceFile = fopen(cspInstanceFilename, "r");
  if (! cspInstanceFile) {
    fprintf(stderr, "ERROR: failed to open csp instance file.\n");
    return 1;
  }

  char* cspJson = NULL;
  size_t cspJsonLen = 0;
  if (0 != (err = readAll(cspInstanceFile, &cspJson, &cspJsonLen))) {
    fprintf(stderr, "ERROR(%d): failed to read csp instance file.", err);
    return 1;
  }

  CjCsp csp = cjCspInit();
  if (0 != (err = cjCspJsonParse(cspJson, cspJsonLen, &csp))) {
    fprintf(stderr, "ERROR(%d): failed to parse csp instance file.", err);
    return 1;
  }


  auto startTime = std::chrono::high_resolution_clock::now();
  long int startTimeSec = std::chrono::duration_cast<std::chrono::seconds>(startTime.time_since_epoch()).count();

  // create model and search engine
  SpaceCJ* m = new SpaceCJ(csp);
  DFS<SpaceCJ> e(m);
  delete m;
  // search and print all solutions
  while (SpaceCJ* s = e.next()) {
    auto endTime = std::chrono::high_resolution_clock::now();
    long int endTimeSec = std::chrono::duration_cast<std::chrono::seconds>(endTime.time_since_epoch()).count();
    auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    s->print(timeMs, e.statistics()); delete s;
    return 0; // TODO
  }
  auto endTime = std::chrono::high_resolution_clock::now();
  long int endTimeSec = std::chrono::duration_cast<std::chrono::seconds>(endTime.time_since_epoch()).count();
  auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
  Search::Statistics stats = e.statistics();
  printf("{\"solution\": null, \"timeMs\": %lld, \"nodes\": %lu, \"fails\": %lu, \"depthMax\": %lu}\n", timeMs, stats.node, stats.fail, stats.depth);
  return 0;
}

