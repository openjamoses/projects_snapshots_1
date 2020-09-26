# Notice

cquery is not yet production ready. I use it day-to-day, but there are still a number of significant issues and unimplemented features.

# cquery

[![Join the chat at https://gitter.im/cquery-project/Lobby](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/cquery-project/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

cquery is a low-latency language server for C++. It is extremely scalable and
has been designed for and tested on large code bases like
[Chromium](https://chromium.googlesource.com/chromium/src/). It's primary goal
is to make working on large code bases much faster by providing accurate and
fast semantic analysis.

![Demo](/images/demo.png?raw=true)

There are rough edges (especially when editing), but it is already possible to
be productive with cquery. Here's a list of implemented features:
  * code completion (with both signature help and snippets)
  * references
  * type hierarchy
  * calls to functions, calls to base and derived functions
  * rename
  * goto definition, goto base method
  * document symbol search
  * global symbol search
  * hover
  * diagnostics

# Setup - build cquery, install extension, setup project

There are three steps to get cquery up and running. Eventually, cquery will be
published in the vscode extension marketplace which will reduce these three
steps to only project setup.

## Build cquery

Building cquery is simple. The external dependencies are few:

- clang (3.4 or greater)
- python
- git

```bash
$ git clone https://github.com/jacobdufault/cquery --recursive
$ cd cquery
$ ./waf configure
$ ./waf build
```

## Install extension

cquery includes a vscode extension; it is part of the repository. Launch vscode
and install the `vscode-extension.tsix` extension. To do this:

- Hit `F1`; execute the command `Install from VSIX`.
- Select `vscode-extension.tsix` in the file chooser.

**IMPORTANT:** Please reinstall the extension when you sync the code base - it is
still being developed.

If you run into issues, you can view debug output by running the
(`F1`) `View: Toggle Output` command and opening the `cquery` output section.

## Project setup (system includes, clang configuration)

### Part 1: System includes

cquery will likely fail to resolve system includes like `<vector>` unless the include path is updated to point to them. Add the system include paths to `cquery.extraClangArguments`. For example,

```js
{
  // ...
  "cquery.extraClangArguments": [
    // Generated by running the following in a Chrome checkout:
    // $ ./third_party/llvm-build/Release+Asserts/bin/clang++ -v ash/debug.cc
    "-isystem/usr/lib/gcc/x86_64-linux-gnu/4.8/../../../../include/c++/4.8",
    "-isystem/usr/lib/gcc/x86_64-linux-gnu/4.8/../../../../include/x86_64-linux-gnu/c++/4.8",
    "-isystem/usr/lib/gcc/x86_64-linux-gnu/4.8/../../../../include/c++/4.8/backward",
    "-isystem/usr/local/include",
    "-isystem/work/chrome/src/third_party/llvm-build/Release+Asserts/lib/clang/5.0.0/include",
    "-isystem/usr/include/x86_64-linux-gnu",
    "-isystem/usr/include",
  ],
  // ...
}
```

### Part 2: Clang configuration

#### compile_commands.json (Best)

To get the most accurate index possible, you can give cquery a compilation
database emitted from your build system of choice. For example, here's how to
generate one in ninja. When you sync your code you should regenerate this file.

```bash
$ ninja -C out/Release -t compdb cxx cc > compile_commands.json
```

The `compile_commands.json` file should be in the top-level workspace directory.

#### cquery.extraClangArguments

If for whatever reason you cannot generate a `compile_commands.json` file, you
can add the flags to the `cquery.extraClangArguments` configuration option.

#### clang_args

If for whatever reason you cannot generate a `compile_commands.json` file, you
can add the flags to a file called `clang_args` located in the top-level
workspace directory.

Each argument in that file is separated by a newline. Lines starting with `#`
are skipped. Here's an example:

```
# Language
-xc++
-std=c++11

# Includes
-I/work/cquery/third_party
```

# Building extension

If you wish to modify the vscode extension, you will need to build it locally.
Luckily, it is pretty easy - the only dependency is npm.

```bash
# Build extension
$ cd vscode-client
$ npm install
$ code .
```

When VSCode is running, you can hit `F5` to build and launch the extension
locally.

# Limitations

cquery is able to respond to queries quickly because it caches a huge amount of
information. When a request comes in, cquery just looks it up in the cache
without running many computations. As a result, there's a large memory overhead.
For example, a full index of Chrome will take about 10gb of memory. If you
exclude v8, webkit, and third_party, it goes down to about 6.5gb.

# License

MIT