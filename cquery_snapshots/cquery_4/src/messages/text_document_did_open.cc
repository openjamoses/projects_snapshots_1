#include "cache_manager.h"
#include "clang_complete.h"
#include "include_complete.h"
#include "message_handler.h"
#include "project.h"
#include "queue_manager.h"
#include "timer.h"
#include "working_files.h"

namespace {
// Open, view, change, close file
struct Ipc_TextDocumentDidOpen
    : public NotificationMessage<Ipc_TextDocumentDidOpen> {
  const static IpcId kIpcId = IpcId::TextDocumentDidOpen;
  struct Params {
    lsTextDocumentItem textDocument;
  };
  Params params;
};
MAKE_REFLECT_STRUCT(Ipc_TextDocumentDidOpen::Params, textDocument);
MAKE_REFLECT_STRUCT(Ipc_TextDocumentDidOpen, params);
REGISTER_IPC_MESSAGE(Ipc_TextDocumentDidOpen);

struct TextDocumentDidOpenHandler
    : BaseMessageHandler<Ipc_TextDocumentDidOpen> {
  void Run(Ipc_TextDocumentDidOpen* request) override {
    // NOTE: This function blocks code lens. If it starts taking a long time
    // we will need to find a way to unblock the code lens request.

    Timer time;
    std::string path = request->params.textDocument.uri.GetPath();
    if (ShouldIgnoreFileForIndexing(path))
      return;

    std::shared_ptr<ICacheManager> cache_manager = ICacheManager::Make(config);
    WorkingFile* working_file =
        working_files->OnOpen(request->params.textDocument);
    optional<std::string> cached_file_contents =
        cache_manager->LoadCachedFileContents(path);
    if (cached_file_contents)
      working_file->SetIndexContent(*cached_file_contents);

    QueryFile* file = nullptr;
    FindFileOrFail(db, project, nullopt, path, &file);
    if (file && file->def) {
      EmitInactiveLines(working_file, file->def->inactive_regions);
      EmitSemanticHighlighting(db, semantic_cache, working_file, file);
    }

    time.ResetAndPrint(
        "[querydb] Loading cached index file for DidOpen (blocks "
        "CodeLens)");

    include_complete->AddFile(working_file->filename);
    clang_complete->NotifyView(path);

    // Submit new index request.
    const Project::Entry& entry = project->FindCompilationEntryForFile(path);
    QueueManager::instance()->index_request.PushBack(
        Index_Request(entry.filename, entry.args, true /*is_interactive*/,
                      request->params.textDocument.text, cache_manager),
        true /* priority */);
  }
};
REGISTER_MESSAGE_HANDLER(TextDocumentDidOpenHandler);
}  // namespace
