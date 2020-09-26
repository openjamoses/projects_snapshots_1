#pragma once

#include "language_server_api.h"

#include <string_view.h>

#include <regex>
#include <string>
#include <tuple>

// Utility method to map |position| to an offset inside of |content|.
int GetOffsetForPosition(lsPosition position, std::string_view content);
// Utility method to find a position for the given character.
lsPosition CharPos(std::string_view search,
                   char character,
                   int character_offset = 0);

struct ParseIncludeLineResult
{
  bool ok;
  std::string text; // include the "include" part
  std::smatch match;
};

ParseIncludeLineResult ParseIncludeLine(const std::string& line);

void DecorateIncludePaths(const std::smatch& match,
                          std::vector<lsCompletionItem>* items);

// TODO: eliminate |line_number| param.
optional<lsRange> ExtractQuotedRange(int line_number, const std::string& line);

void LexFunctionDeclaration(const std::string& buffer_content,
                            lsPosition declaration_spelling,
                            optional<std::string> type_name,
                            std::string* insert_text,
                            int* newlines_after_name);

std::string LexWordAroundPos(lsPosition position, const std::string& content);

// Case-insensitive subsequence matching.
bool SubsequenceMatch(std::string_view search, std::string_view content);

std::tuple<bool, int> SubsequenceCountSkip(std::string_view search,
                                           std::string_view content);
