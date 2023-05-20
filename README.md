# json
A command that turns unreadable JSON, as produced by many commands and APIs these daya,
into a much more human-friendly format.

json will read from standard input or from a file if specified

    ugly-json-producer | json

    json ugly.json

You can output portions of the input by including the name
of the objects as arguments.
Use a - as the file name to read standard input.

    ugly-json-producer | json - author.firstname 'author["surname"]'

    json ugly.json author.firstname 'author["surname"]'

You can access arrays in two different ways: explicit reference or all elements.

Accessing array elemens explicitly

    ugly-json-producer | json - 'author.books[0].title'

Accessing all elements of an array

    ugly-json-producer | json - 'author.books[].title'

Here are the examples above applied to the ugly.json file found in this repo.

    $ cat ugly.json
    {"author":{"firstname":"Fred","surname":"Bloggs","books":[{"title":"War and Peace","date":1869},{"title":"Zen and the Art of Motorcycle Maintenance","date":1974}]}}
    $ json ugly.json 
    {
      "author": {
        "firstname": "Fred",
        "surname": "Bloggs",
        "books": [
          {
            "title": "War and Peace",
            "date": 1869
          },
          {
            "title": "Zen and the Art of Motorcycle Maintenance",
            "date": 1974
          }
        ]
      }
    }
    $ json ugly.json author.firstname 'author["surname"]'
    "author.firstname": "Fred"
    "author["surname"]": "Bloggs"
    $ json ugly.json 'author.books[0].title'
    "author.books[0].title": "War and Peace"
    $ json ugly.json 'author.books[].title'
    "author.books[].title": "War and Peace"
    "author.books[].title": "Zen and the Art of Motorcycle Maintenance"
    $ 
