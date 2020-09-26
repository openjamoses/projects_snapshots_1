#include "lex_utils.h"
#include "message_handler.h"
#include "queue_manager.h"
#include "working_files.h"

#include <loguru.hpp>

namespace {
struct Ipc_TextDocumentDocumentLink
    : public RequestMessage<Ipc_TextDocumentDocumentLink> {
  const static IpcId kIpcId = IpcId::TextDocumentDocumentLink;
  struct Params {
    // The document to provide document links for.
    lsTextDocumentIdentifier textDocument;
  };
  Params params;
};
MAKE_REFLECT_STRUCT(Ipc_TextDocumentDocumentLink::Params, textDocument);
MAKE_REFLECT_STRUCT(Ipc_TextDocumentDocumentLink, id, params);
REGISTER_IPC_MESSAGE(Ipc_TextDocumentDocumentLink);

// A document link is a range in a text document that links to an internal or
// external resource, like another text document or a web site.
struct lsDocumentLink {
  // The range this link applies to.
  lsRange range;
  // The uri this link points to. If missing a resolve request is sent later.
  optional<lsDocumentUri> target;
};
MAKE_REFLECT_STRUCT(lsDocumentLink, range, target);

struct Out_TextDocumentDocumentLink
    : public lsOutMessage<Out_TextDocumentDocumentLink> {
  lsRequestId id;
  std::vector<lsDocumentLink> result;
};
MAKE_REFLECT_STRUCT(Out_TextDocumentDocumentLink, jsonrpc, id, result);

struct TextDocumentDocumentLinkHandler
    : BaseMessageHandler<Ipc_TextDocumentDocumentLink> {
  void Run(Ipc_TextDocumentDocumentLink* request) override {
    Out_TextDocumentDocumentLink out;
    out.id = request->id;

    if (config->showDocumentLinksOnIncludes) {
      QueryFile* file;
      if (!FindFileOrFail(db, project, request->id,
                          request->params.textDocument.uri.GetPath(), &file)) {
        return;
      }

      WorkingFile* working_file = working_files->GetFileByFilename(
          request->params.textDocument.uri.GetPath());
      if (!working_file) {
        LOG_S(WARNING) << "Unable to find working file "
                       << request->params.textDocument.uri.GetPath();
        return;
      }
      for (const IndexInclude& include : file->def->includes) {
        optional<int> buffer_line = working_file->GetBufferPosFromIndexPos(
            include.line, nullptr, false);
        if (!buffer_line)
          continue;

        // Subtract 1 from line because querydb stores 1-based lines but
        // vscode expects 0-based lines.
        optional<lsRange> between_quotes = ExtractQuotedRange(
            *buffer_line, working_file->buffer_lines[*buffer_line]);
        if (!between_quotes)
          continue;

        lsDocumentLink link;
        link.target = lsDocumentUri::FromPath(include.resolved_path);
        link.range = *between_quotes;
        out.result.push_back(link);
      }
    }

    QueueManager::WriteStdout(IpcId::TextDocumentDocumentLink, out);
  }
};
REGISTER_MESSAGE_HANDLER(TextDocumentDocumentLinkHandler);
}  // namespace
