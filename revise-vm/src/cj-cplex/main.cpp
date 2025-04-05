// https://www.ibm.com/docs/en/icos/22.1.1?topic=optimizer-testing-your-installation-simple-application
// https://www.ibm.com/docs/en/icos/20.1.0?topic=cplex-setting-up-gnulinuxmacos
// https://www.ibm.com/docs/en/icos/20.1.0?topic=manual-optimconcertcpoptimizer
// https://www.ibm.com/docs/en/icos/22.1.1?topic=f-iloforbiddenassignments

#include <ilcp/cp.h>
#include <iostream>
#include <sstream>
#include <vector>

#include "cj/cj-csp.h"
#include "cj/cj-csp-io.h"
#include "io.h"

using namespace std;

IloIntVarArray genDomains(const CjCsp& csp, IloEnv& env) {
  IloIntVarArray vars(env);
  for (int iVar = 0; iVar < csp.vars.size; ++iVar) {
    const CjDomain& dom = csp.domains[csp.vars.data[iVar]];
    if (dom.type == CjDomain::CJ_DOMAIN_VALUES) {
      IloIntArray values(env);
      for (int iVal = 0; iVal < dom.values.size - 1; ++iVal) {
        values.add(dom.values.data[iVal]);
      }
      stringstream ss;
      ss << "x" << iVar;
      vars.add(IloIntVar(env, values, ss.str().c_str()));
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

void genConstraints(const CjCsp& csp, IloEnv& env, IloModel& model, IloIntVarArray& vars) {
  // TODO: The correct way to get these is by looking at all the domains of the variables involved in a constraint.
  int domMinVal = 0;
  int domMaxVal = maxDomValue(csp);

  // Constraint Definitions
  vector<IloIntTupleSet> tupleSets;
  for (int iCDef = 0; iCDef < csp.constraintDefsSize; ++iCDef) {
    CjConstraintDef& cDef = csp.constraintDefs[iCDef];
    if (cDef.type == CjConstraintDef::CJ_CONSTRAINT_DEF_NO_GOODS) {

      const int arity = 2;
      tupleSets.push_back(IloIntTupleSet(env, arity));
      for (int iTuple = 0; iTuple < cDef.noGoods.size; ++iTuple) {
        tupleSets.back().add(
          IloIntArray(
            env,
            2,
            cDef.noGoods.data[iTuple * cDef.noGoods.arity + 0],
            cDef.noGoods.data[iTuple * cDef.noGoods.arity + 1]));
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
      model.add(
        IloForbiddenAssignments(
          env,
          vars[constraint.vars.data[0]],
          vars[constraint.vars.data[1]],
          tupleSets[constraint.id]));
      std::vector<IloIntVar> cvars;
    }
    else {
      assert(false);
    }
  }
}

void printUsage() {
  fprintf(stderr, "Usage: cj-solve-or-tools --csp INSTANCE_FILENAME\n");
}

void solve(IloEnv& env, IloModel& model, IloIntVarArray& vars) {
  auto startTime = std::chrono::high_resolution_clock::now();

  IloCP cp(model);
  cp.setParameter(IloCP::Workers, 1);
  cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
#if 0
  cp.setParameter(IloCP::LogPeriod, 1);
#else
  cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
#endif
  cp.setParameter(IloCP::DefaultInferenceLevel, IloCP::Medium); // IloCP::Low, IloCP::Basic, IloCP::Medium, and IloCP::Extended. The default value is IloCP::Basic.
  IloIntVarChooser varChooser = IloSelectSmallest(IloVarIndex(env, vars));
  IloIntValueChooser valChooser = IloSelectSmallest(IloValue(env));
  IloSearchPhase phase(env, vars, varChooser, valChooser);
  cp.setSearchPhases(phase);

  cp.propagate();
  if (cp.solve()) {
    auto endTime = std::chrono::high_resolution_clock::now();
    auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    stringstream ss;
    ss << "[";
    for (size_t iVar = 0; iVar < vars.getSize(); ++iVar) {
      if (iVar > 0) { ss << ","; }
      ss << cp.getValue(vars[iVar]);
    }
    ss << "]";

    printf("{\"solution\": %s, \"timeMs\": %lld}\n", ss.str().c_str(), timeMs);
    return;
  }
  auto endTime = std::chrono::high_resolution_clock::now();
  auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
  printf("{\"solution\": null, \"timeMs\": %lld}\n", timeMs);
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

  IloEnv env;
  try {
    IloIntVarArray vars = genDomains(csp, env);
    IloModel model(env);
    genConstraints(csp, env, model, vars);
    solve(env,model, vars);
  }
  catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
  return 0;


  return 0;
}


#if 0
int main(int argc, const char * argv[]){
  IloEnv env;
  try {
    IloModel model(env);
    IloIntVar x(env, 5, 12, "x");
    IloIntVar y(env, 2, 17, "y");
    model.add(x + y == 17);
    model.add(x - y == 5);
    IloCP cp(model);
    cp.propagate();
    cp.out() << std::endl << "Propagate:" << std::endl;
    cp.out() << "x in " << cp.domain(x) << std::endl;
    cp.out() << "y in " << cp.domain(y) << std::endl << std::endl;
    if (cp.solve()){
      cp.out() << std::endl << "Solution:" << std::endl;
      cp.out() << "x = " << cp.getValue(x) << std::endl;
      cp.out() << "y = " << cp.getValue(y) << std::endl;
    }
  }
  catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}
#endif
