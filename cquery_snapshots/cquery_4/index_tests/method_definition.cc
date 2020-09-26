class Foo {
  void foo() const;
};

void Foo::foo() const {}

/*
OUTPUT:
{
  "includes": [],
  "skipped_by_preprocessor": [],
  "types": [{
      "id": 0,
      "usr": 15041163540773201510,
      "detailed_name": "Foo",
      "short_name": "Foo",
      "kind": 7,
      "spell": "1:7-1:10|-1|1|2",
      "extent": "1:1-3:2|-1|1|0",
      "parents": [],
      "derived": [],
      "types": [],
      "funcs": [0],
      "vars": [],
      "instances": [],
      "uses": ["5:6-5:9|-1|1|4"]
    }],
  "funcs": [{
      "id": 0,
      "usr": 6446764306530590711,
      "detailed_name": "void Foo::foo() const",
      "short_name": "foo",
      "kind": 16,
      "storage": 1,
      "declarations": [{
          "spelling": "2:8-2:11",
          "extent": "2:3-2:19",
          "content": "void foo() const",
          "param_spellings": []
        }],
      "spell": "5:11-5:14|0|2|2",
      "extent": "5:1-5:25|-1|1|0",
      "declaring_type": 0,
      "base": [],
      "derived": [],
      "locals": [],
      "uses": [],
      "callees": []
    }],
  "vars": []
}
*/
