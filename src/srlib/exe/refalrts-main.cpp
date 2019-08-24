#include <exception>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "refalrts-diagnostic-config.h"
#include "refalrts-native-module.h"


//FROM refalrts
#include "refalrts.h"
//FROM refalrts-dynamic
#include "refalrts-dynamic.h"
//FROM refalrts-profiler
#include "refalrts-profiler.h"
//FROM refalrts-vm
#include "refalrts-vm.h"


static void load_native_module_report_error(
  refalrts::ModuleLoadingError error,
  refalrts::ModuleLoadingErrorDetail *detail,
  void *callback_data
) {
  using refalrts::DiagnosticConfig;
  DiagnosticConfig *diagnostic_config =
    static_cast<DiagnosticConfig*>(callback_data);

  switch (error) {
    case refalrts::cModuleLoadingError_ModuleNotFound:
      fprintf(stderr, "INTERNAL ERROR: can't load module %s\n", detail->message);
      exit(155);

    case refalrts::cModuleLoadingError_CantObtainModuleName:
      fprintf(stderr, "INTERNAL ERROR: can't obtain name of main executable\n");
      exit(155);

    case refalrts::cModuleLoadingError_InvalidRasl:
      fprintf(stderr, "INTERNAL ERROR: %s\n", detail->message);
      exit(155);

    case refalrts::cModuleLoadingError_CantAllocMemory:
      fprintf(
        stderr, "INTERNAL ERROR: out of memory while loading module, %s\n",
        detail->message
      );
      exit(155);

    case refalrts::cModuleLoadingError_CantAllocIdent:
      if (diagnostic_config->idents_limit != DiagnosticConfig::NO_LIMIT) {
        fprintf(
          stderr, "INTERNAL ERROR: Identifiers table overflows (max %ld)\n",
          diagnostic_config->idents_limit
        );
      } else {
        fprintf(stderr, "INTERNAL ERROR: can't allocate identifier\n");
      }
      exit(154);

    case refalrts::cModuleLoadingError_UnresolvedExternal:
      fprintf(
        stderr, "INTERNAL ERROR: unresolved external: %s#%u:%u\n",
        detail->func_name.name,
        detail->func_name.cookie1,
        detail->func_name.cookie2
      );
      break;

    case refalrts::cModuleLoadingError_UnresolvedNative:
      fprintf(
        stderr, "INTERNAL ERROR: unresolved native: %s#%u:%u\n",
        detail->func_name.name,
        detail->func_name.cookie1,
        detail->func_name.cookie2
      );
      break;

    case refalrts::cModuleLoadingError_FunctionIsRedeclared:
      fprintf(
        stderr, "INTERNAL ERROR: function is redeclared: %s#%u:%u\n",
        detail->func_name.name,
        detail->func_name.cookie1,
        detail->func_name.cookie2
      );
      exit(156);

    default:
      refalrts_switch_default_violation(error);
  }
}

refalrts::InitDiagnosticConfig refalrts::g_init_diagnostic_config;

namespace {

bool unload_module_rep(
  refalrts::VM *vm, refalrts::Iter pos, refalrts::RefalFunction *module_rep,
  refalrts::FnResult& result
) {
  refalrts::ModuleRepresentant *rep =
    dynamic_cast<refalrts::ModuleRepresentant*>(module_rep);
  if (rep != 0 && rep->module != 0) {
    refalrts::unload_module(vm, pos, rep->module, result);
    return true;
  } else {
    return false;
  }
}

refalrts::Module *module_from_function_rep(refalrts::RefalFunction *module_rep) {
  refalrts::ModuleRepresentant *rep =
    dynamic_cast<refalrts::ModuleRepresentant*>(module_rep);
  return rep != 0 ? rep->module : 0;
}

void read_counters(refalrts::VM *vm, double counters[]) {
  vm->profiler()->read_counters(counters);
  vm->read_counters(counters);
  vm->domain()->read_counters(counters);
}

void profiler_start_generated_function(refalrts::VM *vm) {
  vm->profiler()->start_generated_function();
}

void profiler_stop_sentence(refalrts::VM *vm) {
  vm->profiler()->stop_sentence();
}

void profiler_start_e_loop(refalrts::VM *vm) {
  vm->profiler()->start_e_loop();
}

const refalrts::VMapi api = {
  unload_module_rep,
  module_from_function_rep,
  read_counters,
  profiler_start_generated_function,
  profiler_stop_sentence,
  profiler_start_e_loop,
};

}  // unnamed namespace

int main(int argc, char **argv) {
  refalrts::DiagnosticConfig diagnostic_config;

  if (refalrts::g_init_diagnostic_config) {
    refalrts::g_init_diagnostic_config(&diagnostic_config, &argc, argv);
  }

  refalrts::Profiler profiler(&diagnostic_config);
  refalrts::Domain domain(&diagnostic_config);
  refalrts::VM vm(&api, &profiler, &domain, &diagnostic_config);

  vm.set_debugger_factory(diagnostic_config.debugger_factory);
  vm.set_args(argc, argv);

  refalrts::FnResult res;
  try {
    bool succeeded = domain.load_native_module(
      &vm, &g_module, load_native_module_report_error, &diagnostic_config, res
    );

    if (! succeeded) {
      domain.unload(&vm, res);
      return 157;
    }

    if (res == refalrts::cSuccess) {
      profiler.start_profiler();

      refalrts::RefalFunction *go = domain.lookup_function(0, 0, "GO");

      if (! go) {
        go = domain.lookup_function(0, 0, "Go");
      }

      if (! go) {
        fprintf(stderr, "INTERNAL ERROR: entry point (Go or GO) is not found\n");
        return 158;
      }

      res = vm.execute_zero_arity_function(go);
    }
    fflush(stderr);
    fflush(stdout);
  } catch (std::exception& e) {
    fprintf(stderr, "INTERNAL ERROR: std::exception %s\n", e.what());
    return 152;
  } catch (...) {
    fprintf(stderr, "INTERNAL ERROR: unknown exception\n");
    return 153;
  }

  // TODO: правильный порядок финализации
  profiler.end_profiler();
  refalrts::FnResult res_unload;
  domain.unload(&vm, res_unload);
  if (res == refalrts::cSuccess) {
    res = res_unload;
  }
  vm.free_view_field();
  vm.free_states_stack();
  domain.free_domain_memory();

  fflush(stdout);

  switch(res) {
    case refalrts::cSuccess:
      return 0;

    case refalrts::cRecognitionImpossible:
      return 101;

    case refalrts::cNoMemory:
      return 102;

    case refalrts::cExit:
      return vm.get_return_code();

    case refalrts::cStepLimit:
      return 103;

    case refalrts::cIdentTableLimit:
      return 104;

    default:
      fprintf(stderr, "INTERNAL ERROR: check switch in main (res = %d)\n", res);
      return 155;
  }
}