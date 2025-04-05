// See https://github.com/d-krupke/cpsat-primer for info on the cpsat solver.

#include <iostream>
#include <ortools/sat/cp_model.h>
#include <ortools/sat/cp_model.pb.h>
#include <ortools/sat/cp_model_solver.h>
#include <sstream>
#include <stdlib.h>
#include <time.h>

#include "cj/cj-csp.h"
#include "cj/cj-csp-io.h"
#include "io.h"

using namespace operations_research;
using namespace operations_research::sat;
using namespace std;

vector<IntVar> genDomains(const CjCsp& csp, CpModelBuilder& cpModel) {
  vector<IntVar> vars;
  for (int iVar = 0; iVar < csp.vars.size; ++iVar) {
    const CjDomain& dom = csp.domains[csp.vars.data[iVar]];
    if (dom.type == CjDomain::CJ_DOMAIN_VALUES) {
      vector<int64_t> values;
      for (int iVal = 0; iVal < dom.values.size - 1; ++iVal) {
        values.push_back(dom.values.data[iVal]);
      }
      stringstream ss;
      ss << "x" << iVar;
      vars.push_back(cpModel.NewIntVar(Domain::FromValues(values)).WithName(ss.str()));
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

void genConstraints(const CjCsp& csp, CpModelBuilder& cpModel, vector<IntVar>& vars) {
  // Constraints
  for (int iConstraint = 0; iConstraint < csp.constraintsSize; ++iConstraint) {
    CjConstraint& constraint = csp.constraints[iConstraint];
    CjConstraintDef& cDef = csp.constraintDefs[constraint.id];
    if (cDef.type == CjConstraintDef::CJ_CONSTRAINT_DEF_NO_GOODS) {
      assert(constraint.vars.size == 2); // TODO: make this a stronger check

      TableConstraint table = cpModel.AddForbiddenAssignments({
        vars[constraint.vars.data[0]],
        vars[constraint.vars.data[1]]});

      for (int iTuple = 0; iTuple < cDef.noGoods.size; ++iTuple) {
        table.AddTuple({
          cDef.noGoods.data[iTuple * cDef.noGoods.arity + 0],
          cDef.noGoods.data[iTuple * cDef.noGoods.arity + 1]});
      }
    }
    else {
      assert(false);
    }
  }
}

void printUsage() {
  fprintf(stderr, "Usage: cj-solve-or-tools --csp INSTANCE_FILENAME\n");
}

void solve(CjCsp& csp, CpModelBuilder& cpModel, vector<IntVar>& vars) {
  srand(time(NULL));
  int seed = rand();
  cerr << "seed:" << seed << endl;

  Model model;
  SatParameters pams;
  pams.set_num_workers(1);
  pams.set_random_seed(seed);
  model.Add(NewSatParameters(pams));

  auto startTime = std::chrono::high_resolution_clock::now();
  const CpSolverResponse response = SolveCpModel(cpModel.Build(), &model);
  auto endTime = std::chrono::high_resolution_clock::now();
  auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

  string solution = "null";
  if (response.status() == CpSolverStatus::OPTIMAL ||
      response.status() == CpSolverStatus::FEASIBLE) {
    stringstream ss;
    ss << "[";
    for (size_t iVar = 0; iVar < vars.size(); ++iVar) {
      if (iVar > 0) { ss << ","; }
      ss << SolutionIntegerValue(response, vars[iVar]);
    }
    ss << "]";
    solution = ss.str();
  }
  printf("{\"solution\": %s, \"timeMs\": %lld}\n", solution.c_str(), timeMs);
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

  CpModelBuilder cpModel;
  vector<IntVar> vars = genDomains(csp, cpModel);
  genConstraints(csp, cpModel, vars);
  solve(csp, cpModel, vars);

  return 0;
}

