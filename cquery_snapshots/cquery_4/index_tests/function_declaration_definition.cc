void foo();

void foo() {}

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
      "spell": "3:6-3:9|-1|1|2",
      "extent": "3:1-3:14|-1|1|0",
      "base": [],
      "derived": [],
      "locals": [],
      "uses": [],
      "callees": []
    }],
  "vars": []
}
*/