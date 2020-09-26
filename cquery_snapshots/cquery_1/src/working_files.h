#pragma once

#include "language_server_api.h"
#include "utils.h"

#include <clang-c/Index.h>
#include <optional.h>

#include <mutex>
#include <string>

using std::experimental::optional;
using std::experimental::nullopt;

struct WorkingFile {
  int version = 0;
  std::string filename;

  std::string buffer_content;
  // Note: This assumes 0-based lines (1-based lines are normally assumed).
  std::vector<std::string> index_lines;
  // Note: This assumes 0-based lines (1-based lines are normally assumed).
  std::vector<std::string> all_buffer_lines;
  // This map goes from disk-line -> indicies+1 in index_lines.
  // Note: The items in the value entry are 1-based lines.
  std::unordered_map<std::string, std::vector<int>> index_lines_lookup;
  // This map goes from buffer-line -> indices+1 in all_buffer_lines.
  // Note: The items in the value entry are 1-based liness.
  std::unordered_map<std::string, std::vector<int>> all_buffer_lines_lookup;

  WorkingFile(const std::string& filename, const std::string& buffer_content);

  // This should be called when the indexed content has changed.
  void SetIndexContent(const std::string& index_content);
  // This should be called whenever |buffer_content| has changed.
  void OnBufferContentUpdated();

  // Find the buffer-line which should be shown for |indexed_line|. This
  // accepts and returns 1-based lines.
  optional<int> GetBufferLineFromIndexLine(int indexed_line) const;
  // Find the indexed-line which should be shown for |buffer_line|. This
  // accepts and returns 1-based lines.
  optional<int> GetIndexLineFromBufferLine(int buffer_line) const;

  // Finds the closest 'callable' name prior to position. This is used for
  // signature help to filter code completion results.
  //
  // |completion_position| will be point to a good code completion location to
  // for fetching signatures.
  std::string FindClosestCallNameInBuffer(lsPosition position, int* active_parameter, lsPosition* completion_position = nullptr) const;

  CXUnsavedFile AsUnsavedFile() const;
};

struct WorkingFiles {
  //
  // :: IMPORTANT :: All methods in this class are guarded by a single lock.
  //

  // Find the file with the given filename.
  WorkingFile* GetFileByFilename(const std::string& filename);
  WorkingFile* GetFileByFilenameNoLock(const std::string& filename);

  WorkingFile* OnOpen(const Ipc_TextDocumentDidOpen::Params& open);
  void OnChange(const Ipc_TextDocumentDidChange::Params& change);
  void OnClose(const Ipc_TextDocumentDidClose::Params& close);

  std::vector<CXUnsavedFile> AsUnsavedFiles();

  // Use unique_ptrs so we can handout WorkingFile ptrs and not have them
  // invalidated if we resize files.
  std::vector<std::unique_ptr<WorkingFile>> files;
  std::mutex files_mutex; // Protects |files|.
};