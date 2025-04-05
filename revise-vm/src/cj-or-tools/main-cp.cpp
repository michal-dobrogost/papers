#include <iostream>
#include <ortools/constraint_solver/constraint_solver.h>
#include <sstream>

#include "cj/cj-csp.h"
#include "cj/cj-csp-io.h"
#include "io.h"

using namespace operations_research;
using namespace std;

vector<IntVar*> genDomains(const CjCsp& csp, Solver& solver) {
  vector<IntVar*> vars;
  for (int iVar = 0; iVar < csp.vars.size; ++iVar) {
    const CjDomain& dom = csp.domains[csp.vars.data[iVar]];
    if (dom.type == CjDomain::CJ_DOMAIN_VALUES) {
      vector<int64_t> values;
      for (int iVal = 0; iVal < dom.values.size - 1; ++iVal) {
        values.push_back(dom.values.data[iVal]);
      }
      stringstream ss;
      ss << "x" << iVar;
      vars.push_back(solver.MakeIntVar(values, ss.str().c_str()));
      // Can also usage the Domain class:
      //   Domain non_zero_digit(1, kBase - 1);
      //   IntVar c = cp_model.NewIntVar(non_zero_digit).WithName("C");
    }
    else {
      assert(false); // TODO: handle
    }
  }
  return vars;
}

int maxDomValue(const CjCsp& csp) {
  int maxVal = 0;
  for (int iDom = 0; iDom < csp.domainsSize; ++iDom) {
    CjDomain& dom = csp.domains[iDom];
    if (dom.type == CjDomain::CJ_DOMAIN_VALUES) {
      for (int iVal = 0; iVal < dom.values.size; ++iVal) {
        maxVal = std::max(maxVal, dom.values.data[iVal]);
      }
    }
    else {
      throw std::string("ERROR: unsupported domain type.");
    }
  }
  return maxVal;
}

void genConstraints(const CjCsp& csp, Solver& solver, vector<IntVar*>& vars) {
  // TODO: The correct way to get these is by looking at all the domains of the variables involved in a constraint.
  int domMinVal = 0;
  int domMaxVal = maxDomValue(csp);

  // Constraint Definitions
  vector<IntTupleSet> tupleSets;
  for (int iCDef = 0; iCDef < csp.constraintDefsSize; ++iCDef) {
    CjConstraintDef& cDef = csp.constraintDefs[iCDef];
    if (cDef.type == CjConstraintDef::CJ_CONSTRAINT_DEF_NO_GOODS) {

      typedef set<pair<int, int> > Tuples;
      Tuples tuples;
      for (int xVal = domMinVal; xVal <= domMaxVal; ++xVal) {
        for (int yVal = domMinVal; yVal <= domMaxVal; ++yVal) {
          tuples.insert(pair<int, int>(xVal, yVal));
        }
      }
      for (int iTuple = 0; iTuple < cDef.noGoods.size; ++iTuple) {
        tuples.erase(pair<int, int>(
          cDef.noGoods.data[iTuple * cDef.noGoods.arity + 0],
          cDef.noGoods.data[iTuple * cDef.noGoods.arity + 1]));
      }


      const int arity = 2;
      tupleSets.push_back(IntTupleSet(arity));
      for (Tuples::const_iterator it = tuples.begin(); it != tuples.end(); ++it) {
        tupleSets.back().Insert2(it->first, it->second);
      }
    }
    else {
      assert(false);
    }
  }

  // Constraints
  for (int iConstraint = 0; iConstraint < csp.constraintsSize; ++iConstraint) {
    CjConstraint& constraint = csp.constraints[iConstraint];
    CjConstraintDef& cDef = csp.constraintDefs[constraint.id];
    if (cDef.type == CjConstraintDef::CJ_CONSTRAINT_DEF_NO_GOODS) {
      assert(constraint.vars.size == 2); // TODO: make this a stronger check
      std::vector<IntVar*> cvars;
      cvars.push_back(vars[constraint.vars.data[0]]);
      cvars.push_back(vars[constraint.vars.data[1]]);
      solver.AddConstraint(solver.MakeAllowedAssignments(cvars, tupleSets[constraint.id]));
    }
    else {
      assert(false);
    }
  }
}

void printUsage() {
  fprintf(stderr, "Usage: cj-solve-or-tools --csp INSTANCE_FILENAME\n");
}

void solve(CjCsp& csp, Solver& solver, vector<IntVar*>& vars) {
  DecisionBuilder* const db = solver.MakePhase(
    vars, Solver::CHOOSE_FIRST_UNBOUND, Solver::ASSIGN_MIN_VALUE);

  auto startTime = std::chrono::high_resolution_clock::now();
  solver.NewSearch(db);
  while (solver.NextSolution()) {
    auto endTime = std::chrono::high_resolution_clock::now();
    auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    stringstream ss;
    ss << "[";
    for (size_t iVar = 0; iVar < vars.size(); ++iVar) {
      if (iVar > 0) { ss << ","; }
      ss << vars[iVar]->Value();
    }
    ss << "]";

    printf("{\"solution\": %s, \"timeMs\": %lld}\n", ss.str().c_str(), timeMs);
    solver.EndSearch();
    return;
  }
  auto endTime = std::chrono::high_resolution_clock::now();
  auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
  printf("{\"solution\": null, \"timeMs\": %lld}\n", timeMs);
  solver.EndSearch();
}

int main(int argc, char** argv) {
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
  if (CJ_ERROR_OK != (err = readAll(cspInstanceFile, &cspJson, &cspJsonLen))) {
    fprintf(stderr, "ERROR(%d): failed to read csp instance file.", err);
    return 1;
  }

  CjCsp csp = cjCspInit();
  if (CJ_ERROR_OK != (err = cjCspJsonParse(cspJson, cspJsonLen, &csp))) {
    fprintf(stderr, "ERROR(%d): failed to parse csp instance file.", err);
    return 1;
  }

  if (CJ_ERROR_OK != (err = cjCspValidate(&csp))) {
    fprintf(stderr, "ERROR(%d): failed to validate the csp instance.", err);
    return 1;
  }

  Solver solver("csp-json-or-tools-cp");
  vector<IntVar*> vars = genDomains(csp, solver);
  genConstraints(csp, solver, vars);
  solve(csp, solver, vars);

  return 0;
}

