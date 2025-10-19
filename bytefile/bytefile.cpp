#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "bytefile.h"

#include <fstream>
#include <ios>
#include <iosfwd>

#include "../runtime/runtime.h"

/* Gets a string from a string table by an index */
char *get_string(bytefile *f, int pos)
{
  return &f->string_ptr[pos];
}

/* Gets a name for a public symbol */
char *get_public_name(bytefile *f, int i)
{
  return get_string(f, f->public_ptr[i * 2]);
}

/* Gets an offset for a public symbol */
int get_public_offset(bytefile *f, int i)
{
  return f->public_ptr[i * 2 + 1];
}

/* Reads a binary bytecode file by name and unpacks it */
bytefile *read_file(char *fname)
{
  auto file = std::ifstream(fname, std::ios::binary);
  if (!file.is_open() || file.fail()) failure("%s\n", strerror(errno));

  file.seekg(0, std::ios::end);
  auto fileSize = file.tellg();
  file.seekg(0, std::ios::beg);
  if (file.fail()) failure("%s\n", strerror(errno));

  auto byte_file_memory = reinterpret_cast<char *>(malloc(static_cast<size_t>(sizeof(void *) * 4 + fileSize)));
  if (byte_file_memory == nullptr) failure("*** FAILURE: unable to allocate memory.\n");

  auto byte_file = reinterpret_cast<bytefile *>(byte_file_memory);

  file.read(const_cast<char *>(reinterpret_cast<const char *>(&byte_file->stringtab_size)),
            static_cast<int>(fileSize));
  if (file.fail()) failure("%s\n", strerror(errno));

  byte_file->string_ptr = byte_file->buffer + byte_file->public_symbols_number * 2 * sizeof(int);
  byte_file->public_ptr = reinterpret_cast<int *>(byte_file->buffer);
  byte_file->code_ptr = byte_file->string_ptr + byte_file->stringtab_size;

  return byte_file;
}
