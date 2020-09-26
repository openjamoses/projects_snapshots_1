#include "cache_manager.h"
#include "match.h"
#include "message_handler.h"
#include "platform.h"
#include "project.h"
#include "queue_manager.h"
#include "timestamp_manager.h"
#include "working_files.h"

#include <loguru.hpp>
#include <queue>
#include <unordered_set>

namespace {
struct Ipc_CqueryFreshenIndex
    : public NotificationMessage<Ipc_CqueryFreshenIndex> {
  const static IpcId kIpcId = IpcId::CqueryFreshenIndex;
  struct Params {
    bool dependencies = true;
    std::vector<std::string> whitelist;
    std::vector<std::string> blacklist;
  };
  Params params;
};
MAKE_REFLECT_STRUCT(Ipc_CqueryFreshenIndex::Params,
                    dependencies,
                    whitelist,
                    blacklist);
MAKE_REFLECT_STRUCT(Ipc_CqueryFreshenIndex, params);
REGISTER_IPC_MESSAGE(Ipc_CqueryFreshenIndex);

struct CqueryFreshenIndexHandler : BaseMessageHandler<Ipc_CqueryFreshenIndex> {
  void Run(Ipc_CqueryFreshenIndex* request) override {
    LOG_S(INFO) << "Freshening " << project->entries.size() << " files";

    // TODO: think about this flow and test it more.
    GroupMatch matcher(request->params.whitelist, request->params.blacklist);

    // Unmark all files whose timestamp has changed.
    std::shared_ptr<ICacheManager> cache_manager = ICacheManager::Make(config);

    std::queue<const QueryFile*> q;
    // |need_index| stores every filename ever enqueued.
    std::unordered_set<std::string> need_index;
    // Reverse dependency graph.
    std::unordered_map<std::string, std::vector<std::string>> graph;
    // filename -> QueryFile mapping for files haven't enqueued.
    std::unordered_map<std::string, const QueryFile*> path_to_file;

    for (const auto& file : db->files)
      if (file.def) {
        if (matcher.IsMatch(file.def->path))
          q.push(&file);
        else
          path_to_file[file.def->path] = &file;
        for (const std::string& dependency : file.def->dependencies)
          graph[dependency].push_back(file.def->path);
      }

    while (!q.empty()) {
      const QueryFile* file = q.front();
      q.pop();
      need_index.insert(file->def->path);

      optional<int64_t> modification_timestamp =
          GetLastModificationTime(file->def->path);
      if (!modification_timestamp)
        continue;
      optional<int64_t> cached_modification =
          timestamp_manager->GetLastCachedModificationTime(cache_manager.get(),
                                                           file->def->path);
      if (modification_timestamp != cached_modification)
        file_consumer_shared->Reset(file->def->path);

      if (request->params.dependencies)
        for (const std::string& path : graph[file->def->path]) {
          auto it = path_to_file.find(path);
          if (it != path_to_file.end()) {
            q.push(it->second);
            path_to_file.erase(it);
          }
        }
    }

    auto* queue = QueueManager::instance();

    // Send index requests for every file.
    project->ForAllFilteredFiles(
        config, [&](int i, const Project::Entry& entry) {
          if (!need_index.count(entry.filename))
            return;
          optional<std::string> content = ReadContent(entry.filename);
          if (!content) {
            LOG_S(ERROR) << "When freshening index, cannot read file "
                         << entry.filename;
            return;
          }
          bool is_interactive =
              working_files->GetFileByFilename(entry.filename) != nullptr;
          queue->index_request.PushBack(
              Index_Request(entry.filename, entry.args, is_interactive,
                            *content, ICacheManager::Make(config)));
        });
  }
};
REGISTER_MESSAGE_HANDLER(CqueryFreshenIndexHandler);
}  // namespace
