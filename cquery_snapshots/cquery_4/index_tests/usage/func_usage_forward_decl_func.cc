void foo();

void usage() {
  foo();
}
/*
OUTPUT:
{
  "includes": [],
  "skipped_by_preprocessor": [],
  "types": [],
  "funcs": [{
      "id": 0,
      "usr": 4259594751088586730,
      "detailed_name": "void foo()",
      "short_name": "foo",
      "kind": 12,
      "storage": 1,
      "declarations": [{
          "spelling": "1:6-1:9",
          "extent": "1:1-1:11",
          "content": "void foo()",
          "param_spellings": []
        }],
      "base": [],
      "derived": [],
      "locals": [],
      "uses": ["4:3-4:6|1|3|32"],
      "callees": []
    }, {
      "id": 1,
      "usr": 6767773193109753523,
      "detailed_name": "void usage()",
      "short_name": "usage",
      "kind": 12,
      "storage": 1,
      "declarations": [],
      "spell": "3:6-3:11|-1|1|2",
      "extent": "3:1-5:2|-1|1|0",
      "base": [],
      "derived": [],
      "locals": [],
      "uses": [],
      "callees": ["4:3-4:6|0|3|32"]
    }],
  "vars": []
}
*/
