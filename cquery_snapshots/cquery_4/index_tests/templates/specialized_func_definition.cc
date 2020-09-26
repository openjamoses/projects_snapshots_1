template<class T>
class Template {
  void Foo();
};

template<class T>
void Template<T>::Foo() {}

template<>
void Template<void>::Foo() {}


/*
// TODO: usage information on Template is bad.
// TODO: Foo() should have multiple definitions.

EXTRA_FLAGS:
-fms-compatibility
-fdelayed-template-parsing

OUTPUT:
{
  "includes": [],
  "skipped_by_preprocessor": [],
  "types": [{
      "id": 0,
      "usr": 17107291254533526269,
      "detailed_name": "Template",
      "short_name": "Template",
      "kind": 7,
      "spell": "2:7-2:15|-1|1|2",
      "extent": "2:1-4:2|-1|1|0",
      "parents": [],
      "derived": [],
      "types": [],
      "funcs": [0],
      "vars": [],
      "instances": [],
      "uses": ["7:6-7:14|-1|1|4", "10:6-10:14|-1|1|4"]
    }, {
      "id": 1,
      "usr": 17649312483543982122,
      "detailed_name": "",
      "short_name": "",
      "kind": 0,
      "parents": [],
      "derived": [],
      "types": [],
      "funcs": [],
      "vars": [],
      "instances": [],
      "uses": []
    }],
  "funcs": [{
      "id": 0,
      "usr": 11994188353303124840,
      "detailed_name": "void Template::Foo()",
      "short_name": "Foo",
      "kind": 16,
      "storage": 1,
      "declarations": [{
          "spelling": "3:8-3:11",
          "extent": "3:3-3:13",
          "content": "void Foo()",
          "param_spellings": []
        }, {
          "spelling": "10:22-10:25",
          "extent": "9:1-10:30",
          "content": "template<>\nvoid Template<void>::Foo() {}",
          "param_spellings": []
        }],
      "spell": "7:19-7:22|0|2|2",
      "extent": "6:1-7:24|-1|1|0",
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
