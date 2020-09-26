#include "indexer.h"

#include "libclangmm/Cursor.h"
#include "libclangmm/Index.h"
#include "libclangmm/TranslationUnit.h"
#include "libclangmm/Utility.h"
#include "platform.h"
#include "serializer.h"
#include "timer.h"

#include <algorithm>
#include <chrono>

namespace {

const bool kIndexStdDeclarations = true;

void AddFuncRef(std::vector<IndexFuncRef>* result, IndexFuncRef ref) {
  if (!result->empty() && (*result)[result->size() - 1] == ref)
    return;
  result->push_back(ref);
}


Range Resolve(const CXSourceRange& range) {
  CXSourceLocation start = clang_getRangeStart(range);
  CXSourceLocation end = clang_getRangeEnd(range);

  unsigned int start_line, start_column;
  clang_getSpellingLocation(start, nullptr, &start_line, &start_column, nullptr);
  unsigned int end_line, end_column;
  clang_getSpellingLocation(end, nullptr, &end_line, &end_column, nullptr);

  return Range(
    Position(start_line, start_column) /*start*/,
    Position(end_line, end_column) /*end*/);
}

Range ResolveSpelling(const CXCursor& cx_cursor) {
  CXSourceRange cx_range = clang_Cursor_getSpellingNameRange(cx_cursor, 0, 0);
  return Resolve(cx_range);
}

Range ResolveExtent(const CXCursor& cx_cursor) {
  CXSourceRange cx_range = clang_getCursorExtent(cx_cursor);
  return Resolve(cx_range);
}

}  // namespace


IndexFile::IndexFile(const std::string& path) : id_cache(path), path(path) {
  // TODO: Reconsider if we should still be reusing the same id_cache.
  // Preallocate any existing resolved ids.
  for (const auto& entry : id_cache.usr_to_type_id)
    types.push_back(IndexType(entry.second, entry.first));
  for (const auto& entry : id_cache.usr_to_func_id)
    funcs.push_back(IndexFunc(entry.second, entry.first));
  for (const auto& entry : id_cache.usr_to_var_id)
    vars.push_back(IndexVar(entry.second, entry.first));
}

// TODO: Optimize for const char*?
IndexTypeId IndexFile::ToTypeId(const std::string& usr) {
  auto it = id_cache.usr_to_type_id.find(usr);
  if (it != id_cache.usr_to_type_id.end())
    return it->second;

  IndexTypeId id(types.size());
  types.push_back(IndexType(id, usr));
  id_cache.usr_to_type_id[usr] = id;
  id_cache.type_id_to_usr[id] = usr;
  return id;
}
IndexFuncId IndexFile::ToFuncId(const std::string& usr) {
  auto it = id_cache.usr_to_func_id.find(usr);
  if (it != id_cache.usr_to_func_id.end())
    return it->second;

  IndexFuncId id(funcs.size());
  funcs.push_back(IndexFunc(id, usr));
  id_cache.usr_to_func_id[usr] = id;
  id_cache.func_id_to_usr[id] = usr;
  return id;
}
IndexVarId IndexFile::ToVarId(const std::string& usr) {
  auto it = id_cache.usr_to_var_id.find(usr);
  if (it != id_cache.usr_to_var_id.end())
    return it->second;

  IndexVarId id(vars.size());
  vars.push_back(IndexVar(id, usr));
  id_cache.usr_to_var_id[usr] = id;
  id_cache.var_id_to_usr[id] = usr;
  return id;
}

IndexTypeId IndexFile::ToTypeId(const CXCursor& cursor) {
  return ToTypeId(clang::Cursor(cursor).get_usr());
}

IndexFuncId IndexFile::ToFuncId(const CXCursor& cursor) {
  return ToFuncId(clang::Cursor(cursor).get_usr());
}

IndexVarId IndexFile::ToVarId(const CXCursor& cursor) {
  return ToVarId(clang::Cursor(cursor).get_usr());
}

IndexType* IndexFile::Resolve(IndexTypeId id) {
  return &types[id.id];
}
IndexFunc* IndexFile::Resolve(IndexFuncId id) {
  return &funcs[id.id];
}
IndexVar* IndexFile::Resolve(IndexVarId id) {
  return &vars[id.id];
}

std::string IndexFile::ToString() {
  return Serialize(*this);
}

IndexType::IndexType(IndexTypeId id, const std::string& usr)
    : def(usr), id(id) {
  assert(usr.size() > 0);
  // std::cerr << "Creating type with usr " << usr << std::endl;
}

void RemoveItem(std::vector<Range>& ranges, Range to_remove) {
  auto it = std::find(ranges.begin(), ranges.end(), to_remove);
  if (it != ranges.end())
    ranges.erase(it);
}

void UniqueAdd(std::vector<IndexFuncRef>& refs, IndexFuncRef ref) {
  if (std::find(refs.begin(), refs.end(), ref) != refs.end())
    refs.push_back(ref);
}

void UniqueAdd(std::vector<Range>& uses, Range loc) {
  if (std::find(uses.begin(), uses.end(), loc) == uses.end())
    uses.push_back(loc);
}

IdCache::IdCache(const std::string& primary_file)
  : primary_file(primary_file) {}

template <typename T>
bool Contains(const std::vector<T>& vec, const T& element) {
  for (const T& entry : vec) {
    if (entry == element)
      return true;
  }
  return false;
}

struct NamespaceHelper {
  std::unordered_map<std::string, std::string> container_usr_to_qualified_name;

  void RegisterQualifiedName(std::string usr,
                             const CXIdxContainerInfo* container,
                             std::string qualified_name) {
    if (container) {
      std::string container_usr = clang::Cursor(container->cursor).get_usr();
      auto it = container_usr_to_qualified_name.find(container_usr);
      if (it != container_usr_to_qualified_name.end()) {
        container_usr_to_qualified_name[usr] =
            it->second + qualified_name + "::";
        return;
      }
    }

    container_usr_to_qualified_name[usr] = qualified_name + "::";
  }

  std::string QualifiedName(const CXIdxContainerInfo* container,
                            std::string unqualified_name) {
    if (container) {
      std::string container_usr = clang::Cursor(container->cursor).get_usr();
      auto it = container_usr_to_qualified_name.find(container_usr);
      if (it != container_usr_to_qualified_name.end())
        return it->second + unqualified_name;

      // Anonymous namespaces are not processed by indexDeclaration. If we
      // encounter one insert it into map.
      if (container->cursor.kind == CXCursor_Namespace) {
        // assert(clang::Cursor(container->cursor).get_spelling() == "");
        container_usr_to_qualified_name[container_usr] = "::";
        return "::" + unqualified_name;
      }
    }
    return unqualified_name;
  }
};

struct IndexParam {
  // Only use this when strictly needed (ie, primary translation unit is
  // needed). Most logic should get the IndexFile instance via
  // |file_consumer|.
  //
  // This can be null if we're not generating an index for the primary
  // translation unit.
  IndexFile* primary_file;

  FileConsumer* file_consumer;
  NamespaceHelper ns;

  IndexParam(FileConsumer* file_consumer) : file_consumer(file_consumer) {}
};

int abortQuery(CXClientData client_data, void* reserved) {
  // 0 -> continue
  return 0;
}
void diagnostic(CXClientData client_data,
                CXDiagnosticSet diagnostics,
                void* reserved) {
  IndexParam* param = static_cast<IndexParam*>(client_data);

  // Print any diagnostics to std::cerr
  for (unsigned i = 0; i < clang_getNumDiagnosticsInSet(diagnostics); ++i) {
    CXDiagnostic diagnostic = clang_getDiagnosticInSet(diagnostics, i);

    // Skip diagnostics in system headers.
    CXSourceLocation diag_loc = clang_getDiagnosticLocation(diagnostic);
    if (clang_Location_isInSystemHeader(diag_loc))
      continue;

    // Get db so we can attribute diagnostic to the right indexed file.
    CXFile file;
    unsigned int line, column;
    clang_getSpellingLocation(diag_loc, &file, &line, &column, nullptr);
    bool is_first_time_visiting_file = false;
    IndexFile* db = param->file_consumer->TryConsumeFile(file, &is_first_time_visiting_file);
    if (!db)
      continue;
    if (is_first_time_visiting_file && param->primary_file)
      param->primary_file->dependencies.push_back(db->path);

    // Build diagnostic.

    lsDiagnostic ls_diagnostic;

    // TODO: ls_diagnostic.range is lsRange, we have Range. We should only be
    // storing Range types when inside the indexer so that index <-> buffer
    // remapping logic is applied.
    ls_diagnostic.range = lsRange(lsPosition(line - 1, column), lsPosition(line - 1, column));

    ls_diagnostic.message = clang::ToString(clang_getDiagnosticSpelling(diagnostic));

    // Append the flag that enables this diagnostic, ie, [-Wswitch]
    std::string enabling_flag = clang::ToString(clang_getDiagnosticOption(diagnostic, nullptr));
    if (!enabling_flag.empty())
      ls_diagnostic.message += " [" + enabling_flag + "]";

    ls_diagnostic.code = clang_getDiagnosticCategory(diagnostic);

    switch (clang_getDiagnosticSeverity(diagnostic)) {
      case CXDiagnostic_Ignored:
      case CXDiagnostic_Note:
        ls_diagnostic.severity = lsDiagnosticSeverity::Information;
        break;
      case CXDiagnostic_Warning:
        ls_diagnostic.severity = lsDiagnosticSeverity::Warning;
        break;
      case CXDiagnostic_Error:
      case CXDiagnostic_Fatal:
        ls_diagnostic.severity = lsDiagnosticSeverity::Error;
        break;
    }

    // TODO: integrate FixIts (ie, textDocument/codeAction)

    clang_disposeDiagnostic(diagnostic);

    db->diagnostics.push_back(ls_diagnostic);
  }
}

CXIdxClientFile enteredMainFile(CXClientData client_data,
                                CXFile mainFile,
                                void* reserved) {
  return nullptr;
}

CXIdxClientFile ppIncludedFile(CXClientData client_data,
                               const CXIdxIncludedFileInfo* file) {
  // Clang include logic is broken. This function is never
  // called and clang_findIncludesInFile doesn't work.
  return nullptr;
}

CXIdxClientASTFile importedASTFile(CXClientData client_data,
                                   const CXIdxImportedASTFileInfo*) {
  return nullptr;
}

CXIdxClientContainer startedTranslationUnit(CXClientData client_data,
                                            void* reserved) {
  return nullptr;
}

clang::VisiterResult DumpVisitor(clang::Cursor cursor,
                                 clang::Cursor parent,
                                 int* level) {
  for (int i = 0; i < *level; ++i)
    std::cerr << "  ";
  std::cerr << clang::ToString(cursor.get_kind()) << " "
            << cursor.get_spelling() << std::endl;

  *level += 1;
  cursor.VisitChildren(&DumpVisitor, level);
  *level -= 1;

  return clang::VisiterResult::Continue;
}

void Dump(clang::Cursor cursor) {
  int level = 0;
  cursor.VisitChildren(&DumpVisitor, &level);
}

struct FindChildOfKindParam {
  CXCursorKind target_kind;
  optional<clang::Cursor> result;

  FindChildOfKindParam(CXCursorKind target_kind) : target_kind(target_kind) {}
};

clang::VisiterResult FindChildOfKindVisitor(clang::Cursor cursor,
                                            clang::Cursor parent,
                                            FindChildOfKindParam* param) {
  if (cursor.get_kind() == param->target_kind) {
    param->result = cursor;
    return clang::VisiterResult::Break;
  }

  return clang::VisiterResult::Recurse;
}

optional<clang::Cursor> FindChildOfKind(clang::Cursor cursor,
                                        CXCursorKind kind) {
  FindChildOfKindParam param(kind);
  cursor.VisitChildren(&FindChildOfKindVisitor, &param);
  return param.result;
}

clang::VisiterResult FindTypeVisitor(clang::Cursor cursor,
                                     clang::Cursor parent,
                                     optional<clang::Cursor>* result) {
  switch (cursor.get_kind()) {
    case CXCursor_TypeRef:
    case CXCursor_TemplateRef:
      *result = cursor;
      return clang::VisiterResult::Break;
  }

  return clang::VisiterResult::Recurse;
}

optional<clang::Cursor> FindType(clang::Cursor cursor) {
  optional<clang::Cursor> result;
  cursor.VisitChildren(&FindTypeVisitor, &result);
  return result;
}

bool IsTypeDefinition(const CXIdxContainerInfo* container) {
  if (!container)
    return false;

  switch (container->cursor.kind) {
    case CXCursor_EnumDecl:
    case CXCursor_UnionDecl:
    case CXCursor_StructDecl:
    case CXCursor_ClassDecl:
      return true;
    default:
      return false;
  }
}

struct VisitDeclForTypeUsageParam {
  IndexFile* db;
  int has_processed_any = false;
  optional<clang::Cursor> previous_cursor;
  optional<IndexTypeId> initial_type;

  VisitDeclForTypeUsageParam(IndexFile* db) : db(db) {}
};

void VisitDeclForTypeUsageVisitorHandler(clang::Cursor cursor,
                                         VisitDeclForTypeUsageParam* param) {
  param->has_processed_any = true;
  IndexFile* db = param->db;

  std::string referenced_usr =
      cursor.get_referenced()
          .template_specialization_to_template_definition()
          .get_usr();
  // TODO: things in STL cause this to be empty. Figure out why and document it.
  if (referenced_usr == "")
    return;

  IndexTypeId ref_type_id = db->ToTypeId(referenced_usr);

  if (!param->initial_type)
    param->initial_type = ref_type_id;

  IndexType* ref_type_def = db->Resolve(ref_type_id);
  // TODO: Should we even be visiting this if the file is not from the main
  // def? Try adding assert on |loc| later.
  Range loc = ResolveSpelling(cursor.cx_cursor);
  UniqueAdd(ref_type_def->uses, loc);
}

clang::VisiterResult VisitDeclForTypeUsageVisitor(
    clang::Cursor cursor,
    clang::Cursor parent,
    VisitDeclForTypeUsageParam* param) {
  switch (cursor.get_kind()) {
    case CXCursor_TemplateRef:
    case CXCursor_TypeRef:
      if (param->previous_cursor) {
        VisitDeclForTypeUsageVisitorHandler(param->previous_cursor.value(),
                                            param);
      }

      param->previous_cursor = cursor;
      break;

    // We do not want to recurse for everything, since if we do that we will end
    // up visiting method definition bodies/etc. Instead, we only recurse for
    // things that can logically appear as part of an inline variable
    // initializer,
    // ie,
    //
    //  class Foo {
    //   int x = (Foo)3;
    //  }
    case CXCursor_CallExpr:
    case CXCursor_CStyleCastExpr:
    case CXCursor_CXXStaticCastExpr:
    case CXCursor_CXXReinterpretCastExpr:
      return clang::VisiterResult::Recurse;
  }

  return clang::VisiterResult::Continue;
}

// Finds the cursor associated with the declaration type of |cursor|. This
// strips
// qualifies from |cursor| (ie, Foo* => Foo) and removes template arguments
// (ie, Foo<A,B> => Foo<*,*>).
optional<IndexTypeId> ResolveToDeclarationType(IndexFile* db,
                                               clang::Cursor cursor) {
  clang::Cursor declaration =
      cursor.get_type().strip_qualifiers().get_declaration();
  declaration = declaration.template_specialization_to_template_definition();
  std::string usr = declaration.get_usr();
  if (usr != "")
    return db->ToTypeId(usr);
  return nullopt;
}

// Add usages to any seen TypeRef or TemplateRef under the given |decl_cursor|.
// This returns the first seen TypeRef or TemplateRef value, which can be
// useful if trying to figure out ie, what a using statement refers to. If
// trying to generally resolve a cursor to a type, use
// ResolveToDeclarationType, which works in more scenarios.
optional<IndexTypeId> AddDeclTypeUsages(
    IndexFile* db,
    clang::Cursor decl_cursor,
    const CXIdxContainerInfo* semantic_container,
    const CXIdxContainerInfo* lexical_container) {

  // std::cerr << std::endl << "AddDeclUsages " << decl_cursor.get_spelling() <<
  // std::endl;
  // Dump(decl_cursor);

  //
  // The general AST format for definitions follows this pattern:
  //
  //  template<typename A, typename B>
  //  struct Container;
  //
  //  struct S1;
  //  struct S2;
  //
  //  Container<Container<S1, S2>, S2> foo;
  //
  //  =>
  //
  //  VarDecl
  //    TemplateRef Container
  //    TemplateRef Container
  //    TypeRef struct S1
  //    TypeRef struct S2
  //    TypeRef struct S2
  //
  //
  // Here is another example:
  //
  //  enum A {};
  //  enum B {};
  //
  //  template<typename T>
  //  struct Foo {
  //    struct Inner {};
  //  };
  //
  //  Foo<A>::Inner a;
  //  Foo<B> b;
  //
  //  =>
  //
  //  EnumDecl A
  //  EnumDecl B
  //  ClassTemplate Foo
  //    TemplateTypeParameter T
  //    StructDecl Inner
  //  VarDecl a
  //    TemplateRef Foo
  //    TypeRef enum A
  //    TypeRef struct Foo<enum A>::Inner
  //    CallExpr Inner
  //  VarDecl b
  //    TemplateRef Foo
  //    TypeRef enum B
  //    CallExpr Foo
  //
  //
  // Determining the actual type of the variable/declaration from just the
  // children is tricky. Doing so would require looking up the template
  // definition associated with a TemplateRef, figuring out how many children
  // it has, and then skipping that many TypeRef values. This also has to work
  // with the example below (skipping the last TypeRef). As a result, we
  // determine variable types using |ResolveToDeclarationType|.
  //
  //
  // We skip the last type reference for methods/variables which are defined
  // out-of-line w.r.t. the parent type.
  //
  //  S1* Foo::foo() {}
  //
  // The above example looks like this in the AST:
  //
  //  CXXMethod foo
  //    TypeRef struct S1
  //    TypeRef class Foo
  //    CompoundStmt
  //      ...
  //
  //  The second TypeRef is an uninteresting usage.
  bool process_last_type_ref = true;
  if (IsTypeDefinition(semantic_container) &&
      !IsTypeDefinition(lexical_container)) {
    //
    // In some code, such as the following example, we receive a cursor which is
    // not
    // a definition and is not associated with a definition due to an error
    // condition.
    // In this case, it is the Foo::Foo constructor.
    //
    //  struct Foo {};
    //
    //  template<class T>
    //  Foo::Foo() {}
    //
    if (!decl_cursor.is_definition()) {
      // TODO: I don't think this resolution ever works.
      clang::Cursor def = decl_cursor.get_definition();
      if (def.get_kind() != CXCursor_FirstInvalid) {
        std::cerr << "Successful resolution of decl usage to definition"
                  << std::endl;
        decl_cursor = def;
      }
    }
    process_last_type_ref = false;
  }

  VisitDeclForTypeUsageParam param(db);
  decl_cursor.VisitChildren(&VisitDeclForTypeUsageVisitor, &param);

  // VisitDeclForTypeUsageVisitor guarantees that if there are multiple TypeRef
  // children, the first one will always be visited.
  if (param.previous_cursor && process_last_type_ref) {
    VisitDeclForTypeUsageVisitorHandler(param.previous_cursor.value(), &param);
  } else {
    // If we are not processing the last type ref, it *must* be a TypeRef or
    // TemplateRef.
    //
    // We will not visit every child if the is_interseting is false, so
    // previous_cursor
    // may not point to the last TemplateRef.
    assert(param.previous_cursor.has_value() == false ||
           (param.previous_cursor.value().get_kind() == CXCursor_TypeRef ||
            param.previous_cursor.value().get_kind() == CXCursor_TemplateRef));
  }

  return param.initial_type;
}

// Various versions of LLVM (ie, 4.0) will not visit inline variable references
// for template arguments.
clang::VisiterResult AddDeclInitializerUsagesVisitor(clang::Cursor cursor,
                                                     clang::Cursor parent,
                                                     IndexFile* db) {
  /*
    We need to index the |DeclRefExpr| below (ie, |var| inside of
    Foo<int>::var).

      template<typename T>
      struct Foo {
        static constexpr int var = 3;
      };

      int a = Foo<int>::var;

      =>

      VarDecl a
        UnexposedExpr var
          DeclRefExpr var
            TemplateRef Foo

  */

  switch (cursor.get_kind()) {
    case CXCursor_DeclRefExpr:
      CXCursorKind referenced_kind = cursor.get_referenced().get_kind();
      if (cursor.get_referenced().get_kind() != CXCursor_VarDecl)
        break;

      // TODO: when we resolve the template type to the definition, we get a
      // different USR.

      // clang::Cursor ref =
      // cursor.get_referenced().template_specialization_to_template_definition().get_type().strip_qualifiers().get_usr();
      // std::string ref_usr =
      // cursor.get_referenced().template_specialization_to_template_definition().get_type().strip_qualifiers().get_usr();
      std::string ref_usr =
          cursor.get_referenced()
              .template_specialization_to_template_definition()
              .get_usr();
      // std::string ref_usr = ref.get_usr();
      if (ref_usr == "")
        break;

      Range loc = ResolveSpelling(cursor.cx_cursor);
      // std::cerr << "Adding usage to id=" << ref_id.id << " usr=" << ref_usr
      // << " at " << loc.ToString() << std::endl;
      IndexVarId ref_id = db->ToVarId(ref_usr);
      IndexVar* ref_def = db->Resolve(ref_id);
      UniqueAdd(ref_def->uses, loc);
      break;
  }

  return clang::VisiterResult::Recurse;
}

void AddDeclInitializerUsages(IndexFile* db, clang::Cursor decl_cursor) {
  decl_cursor.VisitChildren(&AddDeclInitializerUsagesVisitor, db);
}

bool AreEqualLocations(CXIdxLoc loc, CXCursor cursor) {
  // clang_getCursorExtent
  // clang_Cursor_getSpellingNameRange

  return clang_equalLocations(
    clang_indexLoc_getCXSourceLocation(loc),
    //clang_getRangeStart(clang_getCursorExtent(cursor)));
    clang_getRangeStart(clang_Cursor_getSpellingNameRange(cursor, 0, 0)));
}

























































void indexDeclaration(CXClientData client_data, const CXIdxDeclInfo* decl) {
  if (!kIndexStdDeclarations && clang_Location_isInSystemHeader(clang_indexLoc_getCXSourceLocation(decl->loc)))
    return;

  assert(AreEqualLocations(decl->loc, decl->cursor));

  CXFile file;
  clang_getSpellingLocation(clang_indexLoc_getCXSourceLocation(decl->loc), &file, nullptr, nullptr, nullptr);
  IndexParam* param = static_cast<IndexParam*>(client_data);
  bool is_first_time_visiting_file = false;
  IndexFile* db = param->file_consumer->TryConsumeFile(file, &is_first_time_visiting_file);
  if (!db)
    return;

  if (is_first_time_visiting_file && param->primary_file)
    param->primary_file->dependencies.push_back(db->path);

  NamespaceHelper* ns = &param->ns;


  //std::cerr << "DECL kind=" << decl->entityInfo->kind << " at " << db->id_cache.Resolve(decl->cursor, false).ToPrettyString(&db->id_cache) << std::endl;

  switch (decl->entityInfo->kind) {
    case CXIdxEntity_CXXNamespace: {
      ns->RegisterQualifiedName(decl->entityInfo->USR, decl->semanticContainer,
                                decl->entityInfo->name);
      break;
    }

    case CXIdxEntity_EnumConstant:
    case CXIdxEntity_Field:
    case CXIdxEntity_Variable:
    case CXIdxEntity_CXXStaticVariable: {
      Range decl_loc_spelling = ResolveSpelling(decl->cursor);

      clang::Cursor decl_cursor = decl->cursor;

      // Do not index implicit template instantiations.
      if (decl_cursor !=
          decl_cursor.template_specialization_to_template_definition())
        break;

      std::string decl_usr = decl_cursor.get_usr();

      IndexVarId var_id = db->ToVarId(decl->entityInfo->USR);
      IndexVar* var_def = db->Resolve(var_id);

      // TODO: Eventually run with this if. Right now I want to iron out bugs
      // this may shadow.
      // TODO: Verify this gets called multiple times
      // if (!decl->isRedeclaration) {
      var_def->def.short_name = decl->entityInfo->name;

      std::string type_name = clang::ToString(clang_getTypeSpelling(clang_getCursorType(decl->cursor)));
      var_def->def.detailed_name = type_name + " " + ns->QualifiedName(decl->semanticContainer, var_def->def.short_name);
      //}

      if (decl->isDefinition) {
        var_def->def.definition_spelling = ResolveSpelling(decl->cursor);
        var_def->def.definition_extent = ResolveExtent(decl->cursor);;
      }
      else {
        var_def->def.declaration = ResolveSpelling(decl->cursor);
      }
      UniqueAdd(var_def->uses, decl_loc_spelling);

      // std::cerr << std::endl << "Visiting declaration" << std::endl;
      // Dump(decl_cursor);

      AddDeclInitializerUsages(db, decl_cursor);
      var_def = db->Resolve(var_id);

      // Declaring variable type information. Note that we do not insert an
      // interesting reference for parameter declarations - that is handled when
      // the function declaration is encountered since we won't receive ParmDecl
      // declarations for unnamed parameters.
      // TODO: See if we can remove this function call.
      AddDeclTypeUsages(
          db, decl_cursor,
          decl->semanticContainer, decl->lexicalContainer);

      // We don't need to assign declaring type multiple times if this variable
      // has already been seen.
      if (!decl->isRedeclaration) {
        optional<IndexTypeId> var_type = ResolveToDeclarationType(db, decl_cursor);
        if (var_type.has_value()) {
          // Don't treat enum definition variables as instantiations.
          bool is_enum_member = decl->semanticContainer && decl->semanticContainer->cursor.kind == CXCursor_EnumDecl;
          if (!is_enum_member)
            db->Resolve(var_type.value())->instances.push_back(var_id);

          var_def->def.variable_type = var_type.value();
        }
      }

      // TODO: Refactor handlers so more things are under 'if (!decl->isRedeclaration)'
      if (decl->isDefinition && IsTypeDefinition(decl->semanticContainer)) {
        IndexTypeId declaring_type_id =
          db->ToTypeId(decl->semanticContainer->cursor);
        IndexType* declaring_type_def = db->Resolve(declaring_type_id);
        var_def->def.declaring_type = declaring_type_id;
        declaring_type_def->def.vars.push_back(var_id);
      }

      break;
    }

    case CXIdxEntity_Function:
    case CXIdxEntity_CXXConstructor:
    case CXIdxEntity_CXXDestructor:
    case CXIdxEntity_CXXInstanceMethod:
    case CXIdxEntity_CXXStaticMethod:
    case CXIdxEntity_CXXConversionFunction: {
      Range decl_loc_spelling = ResolveSpelling(decl->cursor);

      clang::Cursor decl_cursor = decl->cursor;
      clang::Cursor resolved =
          decl_cursor.template_specialization_to_template_definition();

      IndexFuncId func_id = db->ToFuncId(resolved.cx_cursor);
      IndexFunc* func_def = db->Resolve(func_id);

      // We don't actually need to know the return type, but we need to mark it
      // as an interesting usage.
      AddDeclTypeUsages(db, decl_cursor,
                        decl->semanticContainer, decl->lexicalContainer);

      // TODO: support multiple definitions per function; right now we are
      // hacking the 'declarations' field by
      // adding a definition when we really don't have one.
      if (decl->isDefinition && !func_def->def.definition_extent.has_value()) {
        func_def->def.definition_spelling = ResolveSpelling(decl->cursor);
        func_def->def.definition_extent = ResolveExtent(decl->cursor);

        RemoveItem(func_def->declarations, *func_def->def.definition_spelling);
      }
      else {
        Range decl_spelling = ResolveSpelling(decl->cursor);
        // Only add the declaration if it's not already a definition.
        if (!func_def->def.definition_spelling || *func_def->def.definition_spelling != decl_spelling)
          UniqueAdd(func_def->declarations, decl_spelling);
      }

      // If decl_cursor != resolved, then decl_cursor is a template
      // specialization. We
      // don't want to override a lot of the function definition information in
      // that
      // scenario.
      if (decl_cursor == resolved) {
        // TODO: Eventually run with this if. Right now I want to iron out bugs
        // this may shadow.
        // if (!decl->isRedeclaration) {
        func_def->def.short_name = decl->entityInfo->name;

        // Build detailed name. The type desc looks like void (void *). We
        // insert the qualified name before the first '('.
        std::string qualified_name = ns->QualifiedName(decl->semanticContainer, func_def->def.short_name);
        std::string type_desc = decl_cursor.get_type_description();
        size_t offset = type_desc.find('(');
        type_desc.insert(offset, qualified_name);
        func_def->def.detailed_name = type_desc;
        //}

        // TODO: return type
        //decl_cursor.get_type_description()
        //func_def->def.return_type =

        bool is_pure_virtual = clang_CXXMethod_isPureVirtual(decl->cursor);
        bool is_ctor_or_dtor =
            decl->entityInfo->kind == CXIdxEntity_CXXConstructor ||
            decl->entityInfo->kind == CXIdxEntity_CXXDestructor;
        // bool process_declaring_type = is_pure_virtual || is_ctor_or_dtor;

        // Add function usage information. We only want to do it once per
        // definition/declaration. Do it on definition since there should only
        // ever
        // be one of those in the entire program.
        if (IsTypeDefinition(decl->semanticContainer)) {
          IndexTypeId declaring_type_id =
              db->ToTypeId(decl->semanticContainer->cursor);
          IndexType* declaring_type_def = db->Resolve(declaring_type_id);
          func_def->def.declaring_type = declaring_type_id;

          // Mark a type reference at the ctor/dtor location.
          if (is_ctor_or_dtor) {
            Range type_usage_loc = decl_loc_spelling;
            UniqueAdd(declaring_type_def->uses, type_usage_loc);
          }

          // Register function in declaring type if it hasn't been registered
          // yet.
          if (!Contains(declaring_type_def->def.funcs, func_id))
            declaring_type_def->def.funcs.push_back(func_id);
        }

        if (decl->isDefinition || is_pure_virtual) {
          // Mark type usage for parameters as interesting. We handle this here
          // instead of inside var declaration because clang will not emit a var
          // declaration for an unnamed parameter, but we still want to mark the
          // usage as interesting.
          // TODO: Do a similar thing for function decl parameter usages. Mark
          //       prototype params as interesting type usages but also relate
          //       mark
          //       them as as usages on the primary variable - requires USR to
          //       be
          //       the same. We can work around it by declaring which variables
          //       a
          //       parameter has declared and update the USR in the definition.
          clang::Cursor cursor = decl->cursor;
          for (clang::Cursor arg : cursor.get_arguments()) {
            switch (arg.get_kind()) {
              case CXCursor_ParmDecl:
                // We don't need to know the arg type, but we do want to mark it
                // as
                // an interesting usage. Note that we use semanticContainer
                // twice
                // because a parameter is not really part of the lexical
                // container.
                AddDeclTypeUsages(db, arg,
                                  decl->semanticContainer,
                                  decl->semanticContainer);

                // TypeResolution arg_type = ResolveToType(db, arg.get_type());
                // if (arg_type.resolved_type)
                //  AddInterestingUsageToType(db, arg_type,
                //  FindLocationOfTypeSpecifier(arg));
                break;
            }
          }

          // Process inheritance.
          // void clang_getOverriddenCursors(CXCursor cursor, CXCursor
          // **overridden, unsigned *num_overridden);
          // void clang_disposeOverriddenCursors(CXCursor *overridden);
          if (clang_CXXMethod_isVirtual(decl->cursor)) {
            CXCursor* overridden;
            unsigned int num_overridden;
            clang_getOverriddenCursors(decl->cursor, &overridden,
                                       &num_overridden);

            // TODO: How to handle multiple parent overrides??
            for (unsigned int i = 0; i < num_overridden; ++i) {
              clang::Cursor parent = overridden[i];
              IndexFuncId parent_id = db->ToFuncId(parent.get_usr());
              IndexFunc* parent_def = db->Resolve(parent_id);
              func_def = db->Resolve(func_id);  // ToFuncId invalidated func_def

              func_def->def.base = parent_id;
              parent_def->derived.push_back(func_id);
            }

            clang_disposeOverriddenCursors(overridden);
          }
        }
      }

      /*
      optional<FuncId> base;
      std::vector<FuncId> derived;
      std::vector<VarId> locals;
      std::vector<FuncRef> callers;
      std::vector<FuncRef> callees;
      std::vector<Location> uses;
      */
      break;
    }

    case CXIdxEntity_Typedef:
    case CXIdxEntity_CXXTypeAlias: {
      Range decl_loc_spelling = ResolveSpelling(decl->cursor);

      // Note we want to fetch the first TypeRef. Running
      // ResolveCursorType(decl->cursor) would return
      // the type of the typedef/using, not the type of the referenced type.
      optional<IndexTypeId> alias_of =
          AddDeclTypeUsages(db, decl->cursor,
                            decl->semanticContainer, decl->lexicalContainer);

      IndexTypeId type_id = db->ToTypeId(decl->entityInfo->USR);
      IndexType* type_def = db->Resolve(type_id);

      if (alias_of)
        type_def->def.alias_of = alias_of.value();

      type_def->def.short_name = decl->entityInfo->name;
      type_def->def.detailed_name =
          ns->QualifiedName(decl->semanticContainer, type_def->def.short_name);

      type_def->def.definition_spelling = ResolveSpelling(decl->cursor);
      type_def->def.definition_extent = ResolveExtent(decl->cursor);
      UniqueAdd(type_def->uses, decl_loc_spelling);
      break;
    }

    case CXIdxEntity_Enum:
    case CXIdxEntity_Union:
    case CXIdxEntity_Struct:
    case CXIdxEntity_CXXClass: {
      Range decl_loc_spelling = ResolveSpelling(decl->cursor);

      IndexTypeId type_id = db->ToTypeId(decl->entityInfo->USR);
      IndexType* type_def = db->Resolve(type_id);

      // TODO: Eventually run with this if. Right now I want to iron out bugs
      // this may shadow.
      // TODO: For type section, verify if this ever runs for non definitions?
      // if (!decl->isRedeclaration) {

      // name can be null in an anonymous struct (see
      // tests/types/anonymous_struct.cc).
      if (decl->entityInfo->name) {
        ns->RegisterQualifiedName(decl->entityInfo->USR,
                                  decl->semanticContainer,
                                  decl->entityInfo->name);
        type_def->def.short_name = decl->entityInfo->name;
      } else {
        type_def->def.short_name = "<anonymous>";
      }

      type_def->def.detailed_name =
          ns->QualifiedName(decl->semanticContainer, type_def->def.short_name);

      // }

      assert(decl->isDefinition);
      type_def->def.definition_spelling = ResolveSpelling(decl->cursor);
      type_def->def.definition_extent = ResolveExtent(decl->cursor);
      UniqueAdd(type_def->uses, decl_loc_spelling);

      // type_def->alias_of
      // type_def->funcs
      // type_def->types
      // type_def->uses
      // type_def->vars

      // Add type-level inheritance information.
      CXIdxCXXClassDeclInfo const* class_info =
          clang_index_getCXXClassDeclInfo(decl);
      if (class_info) {
        for (unsigned int i = 0; i < class_info->numBases; ++i) {
          const CXIdxBaseClassInfo* base_class = class_info->bases[i];

          AddDeclTypeUsages(db, base_class->cursor,
                            decl->semanticContainer, decl->lexicalContainer);
          optional<IndexTypeId> parent_type_id =
              ResolveToDeclarationType(db, base_class->cursor);
          // type_def ptr could be invalidated by ResolveToDeclarationType.
          IndexType* type_def = db->Resolve(type_id);
          if (parent_type_id) {
            IndexType* parent_type_def =
                db->Resolve(parent_type_id.value());
            parent_type_def->derived.push_back(type_id);
            type_def->def.parents.push_back(parent_type_id.value());
          }
        }
      }
      break;
    }

    default:
      std::cerr
          << "!! Unhandled indexDeclaration:     "
          << clang::Cursor(decl->cursor).ToString() << " at "
          << ResolveSpelling(decl->cursor).start.ToString()
          << std::endl;
      std::cerr << "     entityInfo->kind  = " << decl->entityInfo->kind
                << std::endl;
      std::cerr << "     entityInfo->USR   = " << decl->entityInfo->USR
                << std::endl;
      if (decl->declAsContainer)
        std::cerr << "     declAsContainer   = "
                  << clang::Cursor(decl->declAsContainer->cursor).ToString()
                  << std::endl;
      if (decl->semanticContainer)
        std::cerr << "     semanticContainer = "
                  << clang::Cursor(decl->semanticContainer->cursor).ToString()
                  << std::endl;
      if (decl->lexicalContainer)
        std::cerr << "     lexicalContainer  = "
                  << clang::Cursor(decl->lexicalContainer->cursor).get_usr()
                  << std::endl;
      break;
  }
}

bool IsFunctionCallContext(CXCursorKind kind) {
  switch (kind) {
    case CXCursor_FunctionDecl:
    case CXCursor_CXXMethod:
    case CXCursor_Constructor:
    case CXCursor_Destructor:
    case CXCursor_ConversionFunction:
    case CXCursor_FunctionTemplate:
    case CXCursor_OverloadedDeclRef:
    // TODO: we need to test lambdas
    case CXCursor_LambdaExpr:
      return true;
  }

  return false;
}
















































void indexEntityReference(CXClientData client_data,
                          const CXIdxEntityRefInfo* ref) {
  // Don't index references from or to system headers.
  if (clang_Location_isInSystemHeader(clang_indexLoc_getCXSourceLocation(ref->loc)) ||
      clang_Location_isInSystemHeader(clang_getCursorLocation(ref->referencedEntity->cursor)))
    return;

  //assert(AreEqualLocations(ref->loc, ref->cursor));

//  if (clang_Location_isInSystemHeader(clang_getCursorLocation(ref->cursor)) ||
//      clang_Location_isInSystemHeader(
//          clang_getCursorLocation(ref->referencedEntity->cursor)))
//    return;

  // TODO: Use clang_getFileUniqueID
  CXFile file;
  clang_getSpellingLocation(clang_indexLoc_getCXSourceLocation(ref->loc), &file, nullptr, nullptr, nullptr);
  IndexParam* param = static_cast<IndexParam*>(client_data);
  bool is_first_time_visiting_file = false;
  IndexFile* db = param->file_consumer->TryConsumeFile(file, &is_first_time_visiting_file);
  if (!db)
    return;

  if (is_first_time_visiting_file && param->primary_file)
    param->primary_file->dependencies.push_back(db->path);

  // ref->cursor mainFile=0
  // ref->loc mainFile=1
  // ref->referencedEntity mainFile=1
  //
  // Regardless, we need to do more advanced location processing to handle multiple output IndexFile instances.
  //bool mainFile = clang_Location_isFromMainFile(clang_indexLoc_getCXSourceLocation(ref->loc));
  //Range loc_spelling = param->db->id_cache.ForceResolveSpelling(ref->cursor, false /*interesting*/);
  //std::cerr << "mainFile: " << mainFile << ", loc: " << loc_spelling.ToString() << std::endl;

  // Don't index references that are not from the main file.
  //if (!clang_Location_isFromMainFile(clang_getCursorLocation(ref->cursor)))
  //  return;



  clang::Cursor cursor(ref->cursor);

  // std::cerr << "REF kind=" << ref->referencedEntity->kind << " at " <<
  // db->id_cache.Resolve(cursor, false).ToPrettyString(&db->id_cache) <<
  // std::endl;

  switch (ref->referencedEntity->kind) {
    case CXIdxEntity_CXXNamespace: {
      // We don't index namespace usages.
      break;
    }

    case CXIdxEntity_EnumConstant:
    case CXIdxEntity_CXXStaticVariable:
    case CXIdxEntity_Variable:
    case CXIdxEntity_Field: {
      Range loc_spelling = ResolveSpelling(ref->cursor);

      clang::Cursor referenced = ref->referencedEntity->cursor;
      referenced = referenced.template_specialization_to_template_definition();

      IndexVarId var_id = db->ToVarId(referenced.get_usr());
      IndexVar* var_def = db->Resolve(var_id);
      UniqueAdd(var_def->uses, loc_spelling);
      break;
    }

    case CXIdxEntity_CXXConversionFunction:
    case CXIdxEntity_CXXStaticMethod:
    case CXIdxEntity_CXXInstanceMethod:
    case CXIdxEntity_Function:
    case CXIdxEntity_CXXConstructor:
    case CXIdxEntity_CXXDestructor: {
      // TODO: Redirect container to constructor for the following example, ie,
      //       we should be inserting an outgoing function call from the Foo
      //       ctor.
      //
      //  int Gen() { return 5; }
      //  class Foo {
      //    int x = Gen();
      //  }

      // TODO: search full history?
      Range loc_spelling = ResolveSpelling(ref->cursor);

      // Note: be careful, calling db->ToFuncId invalidates the FuncDef* ptrs.
      IndexFuncId called_id = db->ToFuncId(ref->referencedEntity->USR);
      if (IsFunctionCallContext(ref->container->cursor.kind)) {
        IndexFuncId caller_id = db->ToFuncId(ref->container->cursor);
        IndexFunc* caller_def = db->Resolve(caller_id);
        IndexFunc* called_def = db->Resolve(called_id);

        AddFuncRef(&caller_def->def.callees, IndexFuncRef(called_id, loc_spelling));
        AddFuncRef(&called_def->callers, IndexFuncRef(caller_id, loc_spelling));
      } else {
        IndexFunc* called_def = db->Resolve(called_id);
        AddFuncRef(&called_def->callers, IndexFuncRef(loc_spelling));
      }

      break;
    }

    case CXIdxEntity_Typedef:
    case CXIdxEntity_CXXTypeAlias:
    case CXIdxEntity_Enum:
    case CXIdxEntity_Union:
    case CXIdxEntity_Struct:
    case CXIdxEntity_CXXClass: {
      Range loc_spelling = ResolveSpelling(ref->cursor);

      clang::Cursor referenced = ref->referencedEntity->cursor;
      referenced = referenced.template_specialization_to_template_definition();
      IndexTypeId referenced_id = db->ToTypeId(referenced.get_usr());

      IndexType* referenced_def = db->Resolve(referenced_id);

      //
      // The following will generate two TypeRefs to Foo, both located at the
      // same spot (line 3, column 3). One of the parents will be set to
      // CXIdxEntity_Variable, the other will be CXIdxEntity_Function. There
      // does
      // not appear to be a good way to disambiguate these references, as using
      // parent type alone breaks other indexing tasks.
      //
      // To work around this, we check to see if the usage location has been
      // inserted into all_uses previously.
      //
      //  struct Foo {};
      //  void Make() {
      //    Foo f;
      //  }
      //
      UniqueAdd(referenced_def->uses, loc_spelling);
      break;
    }

    default:
      std::cerr
          << "!! Unhandled indexEntityReference: " << cursor.ToString()
          << " at "
          << ResolveSpelling(ref->cursor).start.ToString()
          << std::endl;
      std::cerr << "     ref->referencedEntity->kind = "
                << ref->referencedEntity->kind << std::endl;
      if (ref->parentEntity)
        std::cerr << "     ref->parentEntity->kind = "
                  << ref->parentEntity->kind << std::endl;
      std::cerr
          << "     ref->loc          = "
          << ResolveSpelling(ref->cursor).start.ToString()
          << std::endl;
      std::cerr << "     ref->kind         = " << ref->kind << std::endl;
      if (ref->parentEntity)
        std::cerr << "     parentEntity      = "
                  << clang::Cursor(ref->parentEntity->cursor).ToString()
                  << std::endl;
      if (ref->referencedEntity)
        std::cerr << "     referencedEntity  = "
                  << clang::Cursor(ref->referencedEntity->cursor).ToString()
                  << std::endl;
      if (ref->container)
        std::cerr << "     container         = "
                  << clang::Cursor(ref->container->cursor).ToString()
                  << std::endl;
      break;
  }
}































std::vector<std::unique_ptr<IndexFile>> Parse(
    IndexerConfig* config, FileConsumer::SharedState* file_consumer_shared,
    std::string file,
    std::vector<std::string> args,
    const std::string& file_contents_path,
    const optional<std::string>& file_contents,
    PerformanceImportFile* perf,
    bool dump_ast) {

  if (!config->enableIndexing)
    return {};

  file = NormalizePath(file);

  Timer timer;

  clang::Index index(0 /*excludeDeclarationsFromPCH*/,
                     0 /*displayDiagnostics*/);
  std::vector<CXUnsavedFile> unsaved_files;
  if (file_contents) {
    CXUnsavedFile unsaved;
    unsaved.Filename = file_contents_path.c_str();
    unsaved.Contents = file_contents->c_str();
    unsaved.Length = file_contents->size();
    unsaved_files.push_back(unsaved);
  }
  clang::TranslationUnit tu(index, file, args, unsaved_files, CXTranslationUnit_KeepGoing);

  perf->index_parse = timer.ElapsedMicrosecondsAndReset();

  if (dump_ast)
    Dump(tu.document_cursor());


  IndexerCallbacks callbacks[] = {
      {&abortQuery, &diagnostic, &enteredMainFile, &ppIncludedFile,
       &importedASTFile, &startedTranslationUnit, &indexDeclaration,
       &indexEntityReference}
  };

  FileConsumer file_consumer(file_consumer_shared);
  IndexParam param(&file_consumer);

  CXFile cx_file = clang_getFile(tu.cx_tu, file.c_str());
  bool is_first_ownership;
  param.primary_file = file_consumer.TryConsumeFile(cx_file, &is_first_ownership);

  //std::cerr << "!! [START] Indexing " << file << std::endl;
  CXIndexAction index_action = clang_IndexAction_create(index.cx_index);
  clang_indexTranslationUnit(index_action, &param, callbacks, sizeof(callbacks),
                             CXIndexOpt_IndexFunctionLocalSymbols | CXIndexOpt_SkipParsedBodiesInSession | CXIndexOpt_IndexImplicitTemplateInstantiations,
                             tu.cx_tu);
  clang_IndexAction_dispose(index_action);
  //std::cerr << "!! [END] Indexing " << file << std::endl;

  perf->index_build = timer.ElapsedMicrosecondsAndReset();

  auto result = param.file_consumer->TakeLocalState();
  for (auto& entry : result) {
    // TODO: only store the path on one of these.
    // TODO: These NormalizePath call should be not needed.
    assert(entry->path == NormalizePath(entry->path));
    assert(entry->id_cache.primary_file == entry->path);
    entry->path = NormalizePath(entry->path);
    entry->id_cache.primary_file = entry->path;

    entry->last_modification_time = GetLastModificationTime(entry->path);
    entry->import_file = file;
    entry->args = args;
  }

  // TODO: Fix interesting checks.
  for (auto& entry : result) {
    for (auto& type : entry->types) {
      if (!type.HasInterestingState()) {
        std::cerr << "!!!! NO INTERESTING STATE FOR " << entry->path << " of !!! " << file << std::endl;
        std::cerr << "!!!! USR " << type.def.usr << std::endl;
        assert(false);
      }
    }
    for (auto& func : entry->funcs) {
      if (!func.HasInterestingState()) {
        std::cerr << "!!!! NO INTERESTING STATE FOR " << entry->path << " of !!! " << file << std::endl;
        std::cerr << "!!!! USR " << func.def.usr << std::endl;
        assert(false);
      }
    }
    for (auto& var : entry->vars) {
      if (!var.HasInterestingState()) {
        std::cerr << "!!!! NO INTERESTING STATE FOR " << entry->path << " of !!! " << file << std::endl;
        std::cerr << "!!!! USR " << var.def.usr << std::endl;
        assert(false);
      }
    }
  }

  return result;
}


void IndexInit() {
  clang_enableStackTraces();
  clang_toggleCrashRecovery(1);
}
