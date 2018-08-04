#include <new>

#include <assert.h>
#include <errno.h>
#include <new>
#include <string.h>
#include <utility>

#include "refalrts-dynamic.h"
#include "refalrts-commands.h"

//FROM refalrts-functions
#include "refalrts-functions.h"
//FROM refalrts-platform-specific
#include "refalrts-platform-specific.h"


//==============================================================================
// Модуль
//==============================================================================

refalrts::Module::Module(Domain *domain, NativeModule *native)
  : m_unresolved_func_tables()
  , m_funcs_table()
  , m_tables()
  , m_native_identifiers()
  , m_native_externals()
  , m_native(native)
  , m_global_variables()
  , m_domain(domain)
  , m_unresolved_native_functions()
  , m_error_message()
  , m_unresolved_externals()
{
  /* пусто */
}

refalrts::Module::~Module() {
  while (m_funcs_table.size() > 0) {
    FuncsMap::iterator p = m_funcs_table.begin();
    RefalFunction *function = p->second;
    m_funcs_table.erase(p);
    delete function;
  }
}

refalrts::Module *
refalrts::Module::load_main_module(
  refalrts::Domain *domain, refalrts::NativeModule *native,
  refalrts::Module::Error& error
) {
  error = cSuccess;

  Module *module = new Module(domain, native);

  char module_name[api::cModuleNameBufferLen];

  bool successed = api::get_main_module_name(module_name);
  if (! successed) {
    error = cObtainMainModuleNameError;
    return module;
  }

  try {
    Loader loader(module, module_name);
    loader.enumerate_blocks();
    module->load_native_identifiers();
    module->alloc_global_variables();
  } catch (LoadModuleError& e) {
    error = cReadRaslError;
    module->m_error_message = e.message;
  } catch (AllocIdentifierError&) {
    error = cIdentAllocError;
  } catch (std::bad_alloc& e) {
    error = cMemoryAllocError;
    module->m_error_message = e.what();
  }

  if (error == cSuccess) {
    module->find_unresolved_externals();

    if (! module->m_unresolved_externals.empty()) {
      error = cUnresolvedExternal;
    }
  }

  if (error == cSuccess) {
    module->resolve_native_functions();

    if (! module->m_unresolved_native_functions.empty()) {
      error = cUnresolvedNative;
    }
  }

  return module;
}

refalrts::Module *
refalrts::Module::load_main_module_and_report_error(
  refalrts::Domain *domain, refalrts::NativeModule *native,
  refalrts::Module::Error& error
) {
  Module *module = load_main_module(domain, native, error);

  switch (error) {
    case cSuccess:
      return module;

    case cObtainMainModuleNameError:
      fprintf(stderr, "INTERNAL ERROR: can't obtain name of main executable\n");
      exit(155);

    case cReadRaslError:
      fprintf(stderr, "INTERNAL ERROR: %s\n", module->m_error_message.c_str());
      exit(155);

    case cMemoryAllocError:
      fprintf(
        stderr, "INTERNAL ERROR: out of memory while loading module, %s\n",
        module->m_error_message.c_str()
      );
      exit(155);

    case cIdentAllocError:
#ifdef IDENTS_LIMIT
      fprintf(
        stderr, "INTERNAL ERROR: Identifiers table overflows (max %ld)\n",
        static_cast<unsigned long>(IDENTS_LIMIT)
      );
#else
      fprintf(stderr, "INTERNAL ERROR: can't allocate identifier\n");
#endif
      exit(154);

    case cUnresolvedExternal:
      fprintf(
        stderr, "INTERNAL ERROR: found %u unresolved externals:\n",
        (unsigned) module->m_unresolved_externals.size()
      );
      for (
        NameList::iterator p = module->m_unresolved_externals.begin();
        p != module->m_unresolved_externals.end();
        ++p
      ) {
        fprintf(stderr, "  * %s#%u:%u\n", p->name, p->cookie1, p->cookie2);
      }
      break;

    case cUnresolvedNative:
      fprintf(
        stderr, "INTERNAL ERROR: found %u unresolved native functions:\n",
        (unsigned) module->m_unresolved_externals.size()
      );
      for (
        NativeList::iterator p = module->m_unresolved_native_functions.begin();
        p != module->m_unresolved_native_functions.end();
        ++p
      ) {
        RefalFuncName& n = (*p)->name;
        fprintf(stderr, "  * %s#%u:%u\n", n.name, n.cookie1, n.cookie2);
      }
      break;

    default:
      refalrts_switch_default_violation(error);
  }

  return module;
}


//------------------------------------------------------------------------------
// Идентификаторы
//------------------------------------------------------------------------------

void refalrts::Module::load_native_identifiers() {
  m_native_identifiers.resize(m_native->next_ident_id);

  for (IdentReference *p = m_native->list_idents; p != 0; p = p->next) {
    assert(p->id < m_native->next_ident_id);
    RefalIdentifier ident = ident_implode(m_domain, p->name);
    if (! ident) {
      throw AllocIdentifierError();
    }
    m_native_identifiers[p->id] = ident;
  }
}

//------------------------------------------------------------------------------
// Функции
//------------------------------------------------------------------------------

refalrts::RefalFunction *
refalrts::Module::lookup_function(const refalrts::RefalFuncName& name) {
  FuncsMap::iterator p = m_funcs_table.find(name);

  if (p != m_funcs_table.end()) {
    return p->second;
  } else {
    return 0;
  }
}

bool refalrts::Module::register_function(refalrts::RefalFunction *func) {
  FuncsMap::value_type new_value(func->name, func);
  std::pair<FuncsMap::iterator, bool> res = m_funcs_table.insert(new_value);
  return res.second;
}

void refalrts::Module::find_unresolved_externals() {
  while (! m_unresolved_func_tables.empty()) {
    ConstTable *table = m_unresolved_func_tables.front();
    std::vector<FunctionTableItem>& items = table->externals;
    for (size_t i = 0; i < items.size(); ++i) {
      const char *str_name = items[i].func_name;
      char type = *str_name;

      UInt32 cookie1, cookie2;
      if (type == '*') {
        cookie1 = 0;
        cookie2 = 0;
      } else if (type == '#') {
        cookie1 = table->cookie1;
        cookie2 = table->cookie2;
      } else {
        throw LoadModuleError(
          std::string("Bad external functin name '")
          + str_name + "', name must start from '*' or '#'"
        );
      }

      RefalFuncName name(str_name + 1, cookie1, cookie2);
      RefalFunction *function = lookup_function(name);
      items[i].function = function;
      if (! function) {
        m_unresolved_externals.push_back(name);
      }
    }

    m_unresolved_func_tables.pop_front();
  }

  m_native_externals.resize(m_native->next_external_id);
  for (
    const ExternalReference *er = m_native->list_externals;
    er != 0;
    er = er->next
  ) {
    RefalFuncName name(er->name, er->cookie1, er->cookie2);
    RefalFunction *function = lookup_function(name);
    assert(er->id < m_native->next_external_id);
    m_native_externals[er->id] = function;
    if (! function) {
      m_unresolved_externals.push_back(name);
    }
  }
}

void refalrts::Module::resolve_native_functions() {
  NativeList::iterator p = m_unresolved_native_functions.begin();
  while (p != m_unresolved_native_functions.end()) {
    RefalNativeFunction *func = m_unresolved_native_functions.front();
    assert(func->ptr == 0);

    NativeReference *ref = m_native->native_references;
    while (ref != 0 && ref->refal_func_name() != func->name) {
      ref = ref->next;
    }

    if (ref != 0) {
      func->ptr = ref->code;
      p = m_unresolved_native_functions.erase(p);
    } else {
      ++p;
    }
  }
}

//------------------------------------------------------------------------------
// Загружаемый модуль
//------------------------------------------------------------------------------

bool refalrts::Module::Loader::seek_rasl_signature() {
  int seek_res = fseek(m_stream, 0L, SEEK_END);
  if (seek_res != 0) {
    throw LoadModuleError("can't seek in module");
  }

  long int file_size = ftell(m_stream);
  if (file_size == -1L) {
    throw LoadModuleError(
      std::string("filesize obtaining error") + strerror(errno)
    );
  }

  long int next_offset = 0L;
  bool found = false;
  while (next_offset < file_size && ! found) {
    seek_res = fseek(m_stream, next_offset, SEEK_SET);
    if (seek_res != 0) {
      throw LoadModuleError("can't seek in module");
    }

    static char sample_sig[] = {
      '\x00', '\x08', '\0', '\0', '\0', 'R', 'A', 'S', 'L', 'C', 'O', 'D', 'E'
    };
    sample_sig[0] = cBlockTypeStart;
    char actual_sig[sizeof(sample_sig)];
    size_t read = fread(actual_sig, sizeof(actual_sig), 1);
    if (read != 1) {
      throw LoadModuleError("can't read bytes from module");
    }

    found = memcmp(sample_sig, actual_sig, sizeof(sample_sig)) == 0;
    next_offset += 4096;
  }

  /*
    Позиция в файле после чтения сигнатуры будет в начале следующего блока,
    что вполне нормально для функции enumerate_blocks().
  */
  return found;
}

/*
  Функция также добавляет дополнительный \0 в конец строки.
*/
std::string refalrts::Module::Loader::read_asciiz() {
  std::string result;

  int byte;
  do {
    byte = fgetc(m_stream);
    if (byte != EOF) {
      result += (char) byte;
    }
  } while (byte != EOF && byte != '\0');

  if (byte == EOF) {
    throw LoadModuleError("can't read ASCIIZ string - EOF is reached");
  }

  return result;
}

refalrts::RefalFuncName
refalrts::Module::ConstTable::make_name(const std::string& name) const {
  char type = name[0];
  const char *proper_name = name.data() + 1;

  if (type == '#') {
    return RefalFuncName(proper_name, cookie1, cookie2);
  } else if (type == '*') {
    return RefalFuncName(proper_name, 0, 0);
  } else {
    throw LoadModuleError("name must start from '*' or '#', but got " + name);
  }
}

refalrts::Module::Loader::Loader(refalrts::Module *module, const char *filename)
  : m_module(module)
  , m_stream(fopen(filename, "rb"))
{
  if (! m_stream) {
    throw LoadModuleError("can't open main executable for read");
  }
}

refalrts::Module::Loader::~Loader() {
  if (m_stream) {
    fclose(m_stream);
  }
}

#define PARSE_ASSERT(condition, message) \
  if (! (condition)) { \
    throw LoadModuleError( \
      std::string("RASL invariant error: ") + (message) \
      + " (failed " + #condition + ")" \
    ); \
  }

void refalrts::Module::Loader::read_start_block(size_t datalen) {
  static const char sample[8] = {
    'R', 'A', 'S', 'L', 'C', 'O', 'D', 'E'
  };
  PARSE_ASSERT(sizeof(sample) == datalen, "invalid START block size");

  char signature[sizeof(sample)];
  size_t read = fread(signature, 1, sizeof(signature));
  PARSE_ASSERT(sizeof(signature) == read, "can't read START signature");
  PARSE_ASSERT(
    memcmp(sample, signature, sizeof(signature)) == 0,
    "invalid signature in START, " + std::string(signature, read)
  );
}

refalrts::Module::ConstTable *
refalrts::Module::Loader::read_const_table() {
  struct {
    UInt32 cookie1;
    UInt32 cookie2;
    UInt32 external_count;
    UInt32 ident_count;
    UInt32 number_count;
    UInt32 string_count;
    UInt32 rasl_length;
    UInt32 external_size;
    UInt32 ident_size;
    UInt32 string_size;
  } fixed_part;

  size_t read = fread(&fixed_part, sizeof(fixed_part), 1);
  PARSE_ASSERT(read == 1, "can't read fixed part of CONST_TABLE");

  m_module->m_tables.push_back(ConstTable());
  ConstTable *new_table = &m_module->m_tables.back();

  new_table->cookie1 = fixed_part.cookie1;
  new_table->cookie2 = fixed_part.cookie2;

  new_table->externals.resize(fixed_part.external_count);
  new_table->external_memory.resize(fixed_part.external_size);
  read = fread(&new_table->external_memory[0], 1, fixed_part.external_size);
  PARSE_ASSERT(
    read == fixed_part.external_size,
    "can't read externals list in CONST_TABLE"
  );
  const char *next_external_name = &new_table->external_memory[0];
  for (size_t i = 0; i < fixed_part.external_count; ++i) {
    new_table->externals[i].func_name = next_external_name;
    // TODO: нужна проверка за выход из границ
    next_external_name += strlen(next_external_name) + 1;
  }
  m_module->m_unresolved_func_tables.push_front(new_table);

  new_table->idents.resize(fixed_part.ident_count);
  new_table->idents_memory.resize(fixed_part.ident_size);
  read = fread(&new_table->idents_memory[0], 1, fixed_part.ident_size);
  PARSE_ASSERT(
    read == fixed_part.ident_size,
    "can't read idents list in CONST_TABLE"
  );
  const char *next_ident_name = &new_table->idents_memory[0];
  for (size_t i = 0; i < fixed_part.ident_count; ++i) {
    RefalIdentifier ident = ident_implode(m_module->m_domain, next_ident_name);
    if (! ident) {
      throw AllocIdentifierError();
    }
    new_table->idents[i] = ident;
    // TODO: нужна проверка за выход из границ
    next_ident_name += strlen(next_ident_name) + 1;
  }

  new_table->numbers.resize(fixed_part.number_count);
  read = fread(
    &new_table->numbers[0], sizeof(RefalNumber), fixed_part.number_count
  );
  PARSE_ASSERT(
    read == fixed_part.number_count,
    "can't read numbers list in CONST_TABLE"
  );

  new_table->strings.resize(fixed_part.string_count);
  new_table->strings_memory.resize(fixed_part.string_size);
  char *string_target = &new_table->strings_memory[0];
  for (size_t i = 0; i < fixed_part.string_count; ++i) {
    UInt32 length;
    read = fread(&length, sizeof(length), 1);
    PARSE_ASSERT(read == 1, "can't read STRING size in CONST_TABLE");
    read = fread(string_target, 1, length);
    PARSE_ASSERT(read == length, "can't read STRING chars in CONST_TABLE");
    new_table->strings[i].string = string_target;
    new_table->strings[i].string_len = length;
    string_target += length;
  }

  new_table->rasl.resize(fixed_part.rasl_length);
  read = fread(&new_table->rasl[0], sizeof(RASLCommand), fixed_part.rasl_length);
  PARSE_ASSERT(
    read == fixed_part.rasl_length, "can't read rasl in CONST_TABLE"
  );

  return new_table;
}

void refalrts::Module::Loader::read_refal_function(
  refalrts::Module::ConstTable *table
) {
  std::string name = read_asciiz();

  UInt32 offset;
  size_t read = fread(&offset, sizeof(offset), 1);
  PARSE_ASSERT(read == 1, "can't read offset in REFAL_FUNCTION");

  new RASLFunction(
    table->make_name(name),
    &table->rasl[offset],
    &table->externals[0],
    &table->idents[0],
    &table->numbers[0],
    &table->strings[0],
    "filename.sref",
    m_module
  );
}

void refalrts::Module::Loader::enumerate_blocks() {
  ConstTable *table = 0;

  bool successed = seek_rasl_signature();
  if (! successed) {
    throw LoadModuleError("can't find signature in executable\n");
  }

  unsigned char type;
  while (fread(&type, sizeof(type), 1) == 1) {
    UInt32 datalen;

    size_t read = fread(&datalen, sizeof(datalen), 1);
    PARSE_ASSERT(read == 1, "can't read block size");

    switch (type) {
      case cBlockTypeStart:
        read_start_block(datalen);
        break;

      case cBlockTypeConstTable:
        table = read_const_table();
        break;

      case cBlockTypeRefalFunction:
        read_refal_function(table);
        break;

      case cBlockTypeNativeFunction:
        m_module->m_unresolved_native_functions.push_back(
          new RefalNativeFunction(0, table->make_name(read_asciiz()), m_module)
        );
        break;

      case cBlockTypeEmptyFunction:
        new RefalEmptyFunction(table->make_name(read_asciiz()), m_module);
        break;

      case cBlockTypeSwap:
        new RefalSwap(table->make_name(read_asciiz()), m_module);
        break;

      case cBlockTypeReference:
        refalrts_switch_default_violation(type);

      case cBlockTypeConditionRasl:
        new RefalCondFunctionRasl(table->make_name(read_asciiz()), m_module);
        break;

      case cBlockTypeConditionNative:
        new RefalCondFunctionNative(table->make_name(read_asciiz()), m_module);
        break;

      default:
        refalrts_switch_default_violation(type);
    }
  }
}

void refalrts::Module::alloc_global_variables() {
  m_global_variables.assign(m_native->global_variables_memory, '\0');
}


//==============================================================================
// Домен
//==============================================================================

refalrts::Domain::Domain()
  : m_idents_table()
  , m_at_exit_list(0)
  , m_module(0)
{
  /* пусто */
}

bool refalrts::Domain::load_native_module(NativeModule *main_module) {
  assert(m_module == 0);

  Module::Error error;
  m_module = Module::load_main_module_and_report_error(this, main_module, error);

  return error == Module::cSuccess;
}

refalrts::Module *
refalrts::Domain::load_module(refalrts::VM * /*vm*/, const char * /*name*/) {
  // assert(this == vm->domain());
  return 0;
}

void refalrts::Domain::unload_module(
  refalrts::VM * /*vm*/, refalrts::Module * /*module*/
) {
  //assert(this == vm->domain());
  //assert(this == module->domain());

  /* пока пусто */
}

void refalrts::Domain::unload() {
  free_idents_table();
  delete m_module;
}

//------------------------------------------------------------------------------
// Идентификаторы
//------------------------------------------------------------------------------

size_t refalrts::Domain::idents_count() {
  return m_idents_table.size();
}

void refalrts::Domain::free_idents_table() {
#ifndef DONT_PRINT_STATISTICS
  fprintf(
    stderr, "Identifiers allocated: %lu\n",
    static_cast<unsigned long>(idents_count())
  );
#endif // ifndef DONT_PRINT_STATISTICS

  while (m_idents_table.size() > 0) {
    IdentsMap::iterator p = m_idents_table.begin();
    RefalIdentifier ident = p->second;
    m_idents_table.erase(p);
    delete ident;
  }
}


refalrts::RefalIdentifier
refalrts::Domain::lookup_ident(const char *name) {
  IdentsMap::iterator p = m_idents_table.find(StringRef(name));

  if (p != m_idents_table.end()) {
    return p->second;
  } else {
    return 0;
  }
}

bool refalrts::Domain::register_ident(RefalIdentifier ident) {
  try {
#ifdef IDENTS_LIMIT
    if (idents_count() >= IDENTS_LIMIT) {
      return false;
    }
#endif // ifdef IDENTS_LIMIT

    IdentsMap::value_type new_value(StringRef(ident->name()), ident);
    std::pair<IdentsMap::iterator, bool> res = m_idents_table.insert(new_value);
    assert(res.second);
    return res.second;
  } catch (std::bad_alloc&) {
    return false;
  }
}

void refalrts::Domain::read_counters(unsigned long counters[]) {
  counters[cPerformanceCounter_IdentsAllocated] =
    static_cast<unsigned long>(idents_count());
}

void refalrts::Domain::at_exit(refalrts::AtExitCB callback, void *data) {
  AtExitListNode *p = m_at_exit_list;
  while (p != 0 && (p->callback != callback || p->data != data)) {
    p = p->next;
  }

  if (p == 0) {
    new AtExitListNode(callback, data, this);
  }
}

void refalrts::Domain::perform_at_exit() {
  while (m_at_exit_list != 0) {
    AtExitListNode *current = m_at_exit_list;
    m_at_exit_list = m_at_exit_list->next;
    current->call(this);
    delete current;
  }
}
